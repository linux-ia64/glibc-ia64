/* Copyright (C) 1993-2025 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.

   As a special exception, if you link the code in this file with
   files compiled with a GNU compiler to produce an executable,
   that does not cause the resulting executable to be covered by
   the GNU Lesser General Public License.  This exception does not
   however invalidate any other reasons why the executable file
   might be covered by the GNU Lesser General Public License.
   This exception applies to code released by its copyright holders
   in files containing the exception.  */

#include "libioP.h"
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <shlib-compat.h>
#include <not-cancel.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <spawn.h>
#include <paths.h>

struct _IO_proc_file
{
  struct _IO_FILE_plus file;
  /* Following fields must match those in class procbuf (procbuf.h) */
  pid_t pid;
  struct _IO_proc_file *next;
};
typedef struct _IO_proc_file _IO_proc_file;

static struct _IO_proc_file *proc_file_chain;

#ifdef _IO_MTSAFE_IO
static _IO_lock_t proc_file_chain_lock = _IO_lock_initializer;

static void
unlock (void *not_used)
{
  _IO_lock_unlock (proc_file_chain_lock);
}
#endif

/* These lock/unlock/resetlock functions are used during fork.  */

void
_IO_proc_file_chain_lock (void)
{
  _IO_lock_lock (proc_file_chain_lock);
}

void
_IO_proc_file_chain_unlock (void)
{
  _IO_lock_unlock (proc_file_chain_lock);
}

void
_IO_proc_file_chain_resetlock (void)
{
  _IO_lock_init (proc_file_chain_lock);
}

/* POSIX states popen shall ensure that any streams from previous popen()
   calls that remain open in the parent process should be closed in the new
   child process.
   To avoid a race-condition between checking which file descriptors need to
   be close (by transversing the proc_file_chain list) and the insertion of a
   new one after a successful posix_spawn this function should be called
   with proc_file_chain_lock acquired.  */
static int
spawn_process (posix_spawn_file_actions_t *fa, FILE *fp, const char *command,
	       int do_cloexec, int pipe_fds[2], int parent_end, int child_end,
	       int child_pipe_fd)
{
  int err = 0;

  for (struct _IO_proc_file *p = proc_file_chain; p; p = p->next)
    {
      int fd = _IO_fileno ((FILE *) p);

      /* If any stream from previous popen() calls has fileno
	 child_pipe_fd, it has been already closed by the adddup2 action
	 above.  */
      if (fd != child_pipe_fd)
	{
	  err = __posix_spawn_file_actions_addclose (fa, fd);
	  if (err != 0)
	    return err;
	}
    }

  err = __posix_spawn (&((_IO_proc_file *) fp)->pid, _PATH_BSHELL, fa, NULL,
		       (char *const[]){ (char*) "sh", (char*) "-c", (char*) "--",
		       (char *) command, NULL }, __environ);
  if (err != 0)
    return err;

  __close_nocancel (pipe_fds[child_end]);

  if (!do_cloexec)
    /* Undo the effects of the pipe2 call which set the
       close-on-exec flag.  */
    __fcntl (pipe_fds[parent_end], F_SETFD, 0);

  _IO_fileno (fp) = pipe_fds[parent_end];

  ((_IO_proc_file *) fp)->next = proc_file_chain;
  proc_file_chain = (_IO_proc_file *) fp;

  return 0;
}

FILE *
_IO_new_proc_open (FILE *fp, const char *command, const char *mode)
{
  int read_or_write;
  /* These are indexes for pipe_fds.  */
  int parent_end, child_end;
  int pipe_fds[2];
  int child_pipe_fd;
  int err;

  int do_read = 0;
  int do_write = 0;
  int do_cloexec = 0;
  while (*mode != '\0')
    switch (*mode++)
      {
      case 'r':
	do_read = 1;
	break;
      case 'w':
	do_write = 1;
	break;
      case 'e':
	do_cloexec = 1;
	break;
      default:
      errout:
	__set_errno (EINVAL);
	return NULL;
      }

  if ((do_read ^ do_write) == 0)
    goto errout;

  if (_IO_file_is_open (fp))
    return NULL;

  /* Atomically set the O_CLOEXEC flag for the pipe end used by the
     child process (to avoid leaking the file descriptor in case of a
     concurrent fork).  This is later reverted in the child process.
     When popen returns, the parent pipe end can be O_CLOEXEC or not,
     depending on the 'e' open mode, but there is only one flag which
     controls both descriptors.  The parent end is adjusted below,
     after creating the child process.  (In the child process, the
     parent end should be closed on execve, so O_CLOEXEC remains set
     there.)  */
  if (__pipe2 (pipe_fds, O_CLOEXEC) < 0)
    return NULL;

  if (do_read)
    {
      parent_end = 0;
      child_end = 1;
      read_or_write = _IO_NO_WRITES;
      child_pipe_fd = 1;
    }
  else
    {
      parent_end = 1;
      child_end = 0;
      read_or_write = _IO_NO_READS;
      child_pipe_fd = 0;
    }

  posix_spawn_file_actions_t fa;
  /* posix_spawn_file_actions_init does not fail.  */
  __posix_spawn_file_actions_init (&fa);

  /* The descriptor is already the one the child will use.  In this case
     it must be moved to another one otherwise, there is no safe way to
     remove the close-on-exec flag in the child without creating a FD leak
     race in the parent.  */
  if (pipe_fds[child_end] == child_pipe_fd)
    {
      int tmp = __fcntl (child_pipe_fd, F_DUPFD_CLOEXEC, 0);
      if (tmp < 0)
	goto spawn_failure;
      __close_nocancel (pipe_fds[child_end]);
      pipe_fds[child_end] = tmp;
    }

  err = __posix_spawn_file_actions_adddup2 (&fa, pipe_fds[child_end],
					    child_pipe_fd);
  if (err != 0)
    goto spawn_failure;

#ifdef _IO_MTSAFE_IO
  _IO_cleanup_region_start_noarg (unlock);
  _IO_lock_lock (proc_file_chain_lock);
#endif
  err = spawn_process (&fa, fp, command, do_cloexec, pipe_fds, parent_end,
		       child_end, child_pipe_fd);
#ifdef _IO_MTSAFE_IO
  _IO_lock_unlock (proc_file_chain_lock);
  _IO_cleanup_region_end (0);
#endif

  __posix_spawn_file_actions_destroy (&fa);

  if (err != 0)
    {
      __set_errno (err);
    spawn_failure:
      __close_nocancel (pipe_fds[child_end]);
      __close_nocancel (pipe_fds[parent_end]);
      return NULL;
    }

  _IO_mask_flags (fp, read_or_write, _IO_NO_READS|_IO_NO_WRITES);
  return fp;
}

FILE *
_IO_new_popen (const char *command, const char *mode)
{
  struct locked_FILE
  {
    struct _IO_proc_file fpx;
#ifdef _IO_MTSAFE_IO
    _IO_lock_t lock;
#endif
  } *new_f;
  FILE *fp;

  new_f = (struct locked_FILE *) malloc (sizeof (struct locked_FILE));
  if (new_f == NULL)
    return NULL;
#ifdef _IO_MTSAFE_IO
  new_f->fpx.file.file._lock = &new_f->lock;
#endif
  fp = &new_f->fpx.file.file;
  _IO_init_internal (fp, 0);
  _IO_JUMPS (&new_f->fpx.file) = &_IO_proc_jumps;
  _IO_new_file_init_internal (&new_f->fpx.file);
  if (_IO_new_proc_open (fp, command, mode) != NULL)
    return (FILE *) &new_f->fpx.file;
  _IO_un_link (&new_f->fpx.file);
  free (new_f);
  return NULL;
}

int
_IO_new_proc_close (FILE *fp)
{
  /* This is not name-space clean. FIXME! */
  int wstatus;
  _IO_proc_file **ptr = &proc_file_chain;
  pid_t wait_pid;
  int status = -1;

  /* Unlink from proc_file_chain. */
#ifdef _IO_MTSAFE_IO
  _IO_cleanup_region_start_noarg (unlock);
  _IO_lock_lock (proc_file_chain_lock);
#endif
  for ( ; *ptr != NULL; ptr = &(*ptr)->next)
    {
      if (*ptr == (_IO_proc_file *) fp)
	{
	  *ptr = (*ptr)->next;
	  status = 0;
	  break;
	}
    }
#ifdef _IO_MTSAFE_IO
  _IO_lock_unlock (proc_file_chain_lock);
  _IO_cleanup_region_end (0);
#endif

  if (status < 0 || __close_nocancel (_IO_fileno(fp)) < 0)
    return -1;
  /* POSIX.2 Rationale:  "Some historical implementations either block
     or ignore the signals SIGINT, SIGQUIT, and SIGHUP while waiting
     for the child process to terminate.  Since this behavior is not
     described in POSIX.2, such implementations are not conforming." */
  do
    {
      int state;
      __pthread_setcancelstate (PTHREAD_CANCEL_DISABLE, &state);
      wait_pid = __waitpid (((_IO_proc_file *) fp)->pid, &wstatus, 0);
      __pthread_setcancelstate (state, NULL);
    }
  while (wait_pid == -1 && errno == EINTR);
  if (wait_pid == -1)
    return -1;
  return wstatus;
}

strong_alias (_IO_new_popen, __new_popen)
versioned_symbol (libc, _IO_new_popen, _IO_popen, GLIBC_2_1);
versioned_symbol (libc, __new_popen, popen, GLIBC_2_1);
versioned_symbol (libc, _IO_new_proc_open, _IO_proc_open, GLIBC_2_1);
versioned_symbol (libc, _IO_new_proc_close, _IO_proc_close, GLIBC_2_1);
