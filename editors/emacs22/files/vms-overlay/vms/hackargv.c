/*
 * Copyright (C) 1994 the Free Software Foundation, Inc.
 *
 * Author: Roland B. Roberts (roberts@nsrl.rochester.edu)
 *
 * This file is a part of GNU VMSLIB, the GNU library for porting GNU
 * software to VMS.
 *
 * GNU VMSLIB is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GNU VMSLIB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU VMSLIB; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

static char *hackargv_version = "VMS hackargv version 1.1";

/*
 * hackargv (int *pargc, char ***pargv, int start)
 *
 * Summary:
 *   Emulate C shell I/O redirection.
 *   Bourne shell I/O forms are *NOT* supported (except, of course,
 *     where they are identical to C-shell forms).
 *   Given pointers to argc and argv, handle redirection and globbing.
 *
 * Argument list:
 *   int *pargc     (modify) argument count
 *   char ***pargv  (modify) argument list
 *   int start      (optional, read) start globbing at (*pargv)[start],
 *
 * Description:
 *   `Fix' (*pargv)[0] so it is a valid VMS filename.  This is a noop unless
 *     the path involves a concealed device.
 *   If background processing is requested ('&' is last character in argv),
 *     do it immediately --- skip all globbing and redirection.  PID of
 *     background process is printed on stdout, and the program exits.
 *   Starting with (*pargv)[start], glob anything that contains a wildcard and
 *     doesn't begin with `-'.
 *   Stop globbing when (*pargv)[i] contains a pipe command, `|'.  Create a
 *     process and pass it the remainder of the command line.  Redirect I/O
 *     so stdout goes into stdin of the new process.
 *
 *   Handle the I/O redirections:
 *     1. "cmd < file"    stdin from file
 *     2. "cmd > file"    stdout to file
 *     3. "cmd >> file"   stdout append to file
 *
 *     Appending an ampersand, "&" to the redirection means send both stdout
 *     and stderr to the file.
 *
 *     The "clobber" forms (e.g., ">!") are recognized but treated identically
 *     to the ordinary forms.  VMS version control makes this the (generally)
 *     correct thing to do.  The only exception would be for files with
 *     version_limit==1.
 *
 *     Appending a "+" to the redirection means open the output file with VMS
 *     text file attributes.  This is recognized for the append forms, but
 *     does *NOT* change the file attributes.  The "+" should be the last
 *     character in the I/O redirection string, i.e., ">&+", not ">+&".
 *     
 *     Bourne shell I/O redirection forms are not recognized.
 *
 *   Pipes (not yet implemented!)
 *     1. "cmd1 | cmd2"   stdout piped into cmd
 *     2. "cmd1 |& cmd2"  stdout and stderr piped into cmd2
 *
 *   Special characters
 *     1. any\Nany  substitute octal character 0N
 *     2. any\xNany substitute hexadecimal character 0xN
 *     3. any\Cany  substitute special character '\C' where '\C' is
 *                  taken from K&R 2nd ed.  Any unrecognized
 *                  character will be replaced by itself, i.e.,
 *                  '\*' --> '*'
 *     4. any^Cany  substitute control character 'C' by masking
 *                  with 0x1F.
 *
 *   Wildcard exansion (globbing)
 *     Recognizes '*', '%' and '?' as wildcards.  '?' is mapped to '%'
 *     before expanding.
 *
 *     Any string with a quoted wildcard (e.g., 'foo\*.txt' will not be
 *     expanded, even if it also contains non-quoted wildcards.  Any string
 *     beginning with a dash, '-', will also not be expanded under the
 *     assumption it is an option, not a filename (VMS filenames can't
 *     start with a dash anyway).
 *
 * Bugs
 *   In order to prevent creating empty output files, an exit handler is
 *   declared if there is any I/O redirection.  On exit, the handler uses
 *   stat() to check the file size.  If it is zero, the file is deleted.
 *   This may not always be desireable.
 *
 *   [ Actually, that's not at all desirable, if you wnt to retain some
 *     kind of compatibility with Unix functionality.  I've removed that
 *     "feature".  -- Richard Levitte ]
 *
 * Author:
 *   Roland B. Roberts (roberts@nsrl.rochester.edu)
 *   March 1994
 *
 */

/* Because we probably don't compile with /PREFIX=ALL, but still want
   to use these routines...   */
#ifdef __DECC
#define delete decc$delete
#define fgetname decc$fgetname
#if __VMS_VER < 70000000
#define fstat decc$fstat
#define stat decc$stat
#endif
#endif

/* If you absolutelly want to remove empty files as described in the bugs
   section, define the macro ENABLE_HACKARGV_REMOVE_EMPTY */
/*#define ENABLE_HACKARGV_REMOVE_EMPTY*/

#include <stdio.h>
#include <stdlib.h>
#include <ssdef.h>
#include <stsdef.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stat.h>
#include <unixio.h>
#include <lib$routines.h>
#include <starlet.h>

/* In newer versions of DEC C, va_count is defined again, althought the
   comments tell us it's an extension (and a damn usefull one, I'd say),
   so let's check if it's already defined before we defined it...
   -- Richard Levitte  */
#ifndef va_count
/* In stdarg.h, there is no va_count.  It is, however, quite possible
   to use it on a VMS computer.  */
#ifdef __ALPHA
unsigned int __VA_COUNT_BUILTIN(void);
#define va_count(count)         (count = __VA_COUNT_BUILTIN())
#else
#if defined(VAXC) || defined (__GNUC__)
void vaxc$va_count( int *__count );
#define va_count(count)         vaxc$va_count (&count)
#else
void decc$va_count( int *__count );
#define va_count(count)         decc$va_count (&count)
#endif
#endif
#endif

#ifndef MEMINC
#define MEMINC 10
#else
#if MEMINC < 3
#undef MEMINC
#define MEMINC 10
#endif
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* Forward declarations */
char **glob (char *);
int    esctrans (char *dst, char *src);
char  *downcase (char *);
#ifdef ENABLE_HACKARGV_REMOVE_EMPTY
int    chkfile (FILE *stream);
#endif
int    fixpath (char *);
int    background (char *);
char  *argvconcat (int, char **);
FILE  *popen (const char *, const char *);
int    pclose (FILE *);
void   fatal (char *);
void  *xmalloc (int);
void  *xrealloc (void *, int);
#if defined(__DECC) && __VMS_VER < 70000000
char  *strdup (char *);
#endif

/* Append exactly LENGTH bytes without relying on spare capacity in the
   destination returned by strdup.  The old parser used strcat/strncat here,
   which overwrote the heap whenever an argument was split around an embedded
   redirection character.  */
static void
append_argument (destination, source, length)
     char **destination;
     const char *source;
     size_t length;
{
  size_t old_length = *destination ? strlen (*destination) : 0;
  size_t new_size;

  if (old_length >= (size_t) INT_MAX
      || length > (size_t) INT_MAX - old_length - 1)
    fatal ("hackargv: argument too long");
  new_size = old_length + length + 1;
  *destination = (char *) xrealloc (*destination, (int) new_size);
  memcpy (*destination + old_length, source, length);
  (*destination)[old_length + length] = '\0';
}

enum redirection_t
{ /* Type of redirection to be done */
  NONE   = 0x000,
  STDIN  = 0x001,
  STDOUT = 0x002,
  STDERR = 0x004,
  VMSFMT = 0x010,
  CONCAT = 0x020,
  OVRWRT = 0x040
};

int hackargv (int *pargc, char ***pargv, ...)
{
  /* Stuff for new argv, argc */
  int ac, sz, istart = 1;
  char **av;
  /* Filenames and flags associated with I/O redirection */
  char *infile, *outfile, *errfile;
  int inflags, outflags, errflags;
  enum redirection_t redirection;
  /* Command line passed to popen() or background() */
  char *cmdline;
  FILE *stdpipe;
  int pid;
  /* Pointer for handling optional arguments */
  va_list vap;
  int nvargs;
  /* Miscellany */
  int i, j;
  char *temp;
  struct stat stbuf;

  inflags = outflags = errflags = 0;
  infile  = outfile  = errfile  = NULL;

  /* Allocate space; adjust size later */
  av = (char **) xmalloc ((sz=MEMINC) * sizeof (char *));

  /* Set up argv[0] before anything else */
  av[0] = *pargv[0];
  fixpath (av[0]);

  /* Check to see if stdin is comming from a pipe.
   * If so, reopen in binary mode.
   */
  fstat (0, &stbuf);
  if (strncmp(stbuf.st_dev,"_MB",3) == 0)
    freopen (stbuf.st_dev, "rb", stdin);
  
  /* If background processing was requested, do it immediately.
   * This is equivalent to matching the regexp "\(.*\)\([|>]?\)&\(.*\)"
   * then asking is \2 == NULL?  If so, it really is a background
   * request.  If \3 != NULL, an error occurs.
   */
  temp = (void *) 0;
  if ((strcmp("&",(*pargv)[*pargc-1]) == 0) ||
      ((temp = strrchr((*pargv)[*pargc-1],'&')) &&
	       (temp == (*pargv)[*pargc-1]
		|| ((*(temp-1) != '|') && (*(temp-1) != '>'))))) {
    if (temp) {
      if (*(temp+1)) {
	fprintf (stderr, "background: junk follows `&'\n");
	abort ();
      }
      *temp = 0;
    }
    else
      --*pargc;
    errno = 0;
    temp  = argvconcat (*pargc, *pargv);
    pid   = background (temp);
    if (errno) {
      perror ("background");
      exit (vaxc$errno | STS$M_INHIB_MSG);
    }
    else {
      printf ("%08x", pid);
      exit (SS$_NORMAL);
    }
  }
  
  /* Check the optional parameter */
  va_count(nvargs);
  if (nvargs > 2) {
    va_start (vap, pargv);
    istart = va_arg (vap, int);
    va_end (vap);
  }
  if (istart < 1)
    istart = 1;

  av[ac=1] = NULL;
  redirection = NONE;
  for (i = istart; i < *pargc; ) {
    int version;
    char **filelist;
    int iglob = 0;
    temp = NULL;
    if ((strcmp("&",(*pargv)[*pargc-1]) == 0) ||
	((temp = (char*)strrchr((*pargv)[*pargc-1],'&')) &&
	 (temp == (*pargv)[*pargc-1]
	  || ((*(temp-1) != '|') && (*(temp-1) != '>'))))) {
      fprintf (stderr, "background: misplaced `&'\n");
      abort ();
    }
    if ((temp = strpbrk((*pargv)[i],"<>|"))) {
      /* Maybe redirected */
      if (temp == (*pargv)[i]) {
	/* Definitely redirected */
	switch (*temp) {
	case '|':		/* stuff down the pipe */
	  switch ((*pargv)[i][1]) {
	  case '&':
	    redirection = STDERR;
	    if ((*pargv)[i][2])
	      (*pargv)[i] += 2;
	    else
	      i++;
	    break;
	  case 0:
	    redirection = STDOUT;
	    i++;
	    break;
	  default:
	    redirection = STDOUT;
	    (*pargv)[i] += 1;
	    break;
	  }
	  cmdline = argvconcat (*pargc-i, &((*pargv)[i]));
	  errno  = 0;
	  stdpipe = popen (cmdline, "w");
	  if (!stdpipe) {
	    perror ("hackargv, popen failed");
	    abort ();
	  }
	  /*
	   * Redefine SYS$ERROR and SYS$OUTPUT.
	   * Priorities are:
	   *   For SYS$ERROR
	   *     1. if errfile == outfile, SYS$ERROR --> SYS$OUTPUT
	   *     2. if errfile, SYS$ERROR --> errfile
	   *     3. if redirection == '|&', SYS$ERROR --> stdpipe
	   *   For SYS$OUTPUT
	   *     1. if outfile, SYS$OUTPUT --> outfile
	   *     2. stdout --> stdpipe
	   * This means the command line
	   *     $ foo >&junk | bar
	   * will result in `bar' reading an immediate EOF on stdin.
	   * Exit handler's are declared to deassign the logical name
	   * definitions at image rundown.
	   */
	  if (outfile) {
	    freopen (outfile, "w", stdout);
#ifdef ENABLE_HACKARGV_REMOVE_EMPTY
	    chkfile (stdout);
#endif
	  }
	  else
	    stdout = stdpipe;
	  if (errfile) {
	    freopen (errfile, "w", stderr);
#ifdef ENABLE_HACKARGV_REMOVE_EMPTY
	    if (errfile != outfile)
	      chkfile (stderr);
#endif
	  }
	  else if (redirection == STDERR)
	    stderr = stdpipe;
	  if (infile)
	    freopen (infile, "r", stdin);
	  /* done --- set the new argc, argv and return */
	  av[ac] = NULL;
	  if (sz > ac)
	    av = (char **) xrealloc (av, (ac+1) * sizeof (char *));
	  *pargc = ac;
	  *pargv = av;
	  return (0);
	  break;
	case '<':		/* setup input redirection and continue */
	  if ((*pargv)[i][1])
	    (*pargv)[i]++;
	  else
	    i++;
	  if (infile) {
	    fprintf (stderr, "stdin already redirected to %s\n", infile);
	    abort();
	  }
	  redirection = STDIN;
	  break;
	case '>':		/* setup output redirection and continue */
	  /* Each check is of the form:
	   * Has the file redirection already been set?
	   * - if so, signal an error
	   * - if not, flag the type of redirection to be done and
	   *   setup for next pass (to parse file name)
	   * Note, the stderr forms do not signal an error if stdout has
	   * already been redirected.  On VMS the line
	   *     $ foo > outfile >& errfile
	   * is equivalent to the C-shell line
	   *     % (foo > outfile) >& errfile
	   * The catenation forms, '>>...' allow the '$' specifier so that
	   * catenation can also create a file.
	   */
	  if (strncmp(">>&!+",(*pargv)[i],j=5) == 0) {
	    if (errfile) {	/* error */ }
	    redirection = STDERR | CONCAT | VMSFMT | OVRWRT;
	    if (!outfile)
	      redirection |= STDOUT;
	  }
	  else if (strncmp(">>&+",(*pargv)[i],j=4) == 0) {
	    if (errfile) {
	      fprintf (stderr, "stderr already redirected to %s\n", errfile);
	      abort ();
	    }
	    redirection = STDERR | CONCAT | VMSFMT;
	    if (!outfile)
	      redirection |= STDOUT;
	  }
	  else if (strncmp(">>&!",(*pargv)[i],j=4) == 0) {
	    if (errfile) {
	      fprintf (stderr, "stderr already redirected to %s\n", errfile);
	      abort ();
	    }
	    redirection = STDERR | CONCAT | OVRWRT;
	    if (!outfile)
	      redirection |= STDOUT;
	  }
	  else if (strncmp(">>!+", (*pargv)[i],j=4) == 0) {
	    if (outfile) {
	      fprintf (stderr, "stdout already redirected to %s\n", outfile);
	      abort ();
	    }
	    redirection = STDOUT | CONCAT | VMSFMT | OVRWRT;
	  }
	  else if (strncmp(">>&", (*pargv)[i],j=3) == 0) {
	    if (errfile) {
	      fprintf (stderr, "stderr already redirected to %s\n", errfile);
	      abort ();
	    }
	    redirection = STDERR | CONCAT;
	    if (!outfile)
	      redirection |= STDOUT;
	  }
	  else if (strncmp(">>+", (*pargv)[i],j=3) == 0) {
	    if (errfile) {
	      fprintf (stderr, "stderr already redirected to %s\n", errfile);
	      abort ();
	    }
	    redirection = STDERR | CONCAT | VMSFMT;
	    if (!outfile)
	      redirection |= STDOUT;
	  }
	  else if (strncmp(">>!", (*pargv)[i],j=3) == 0) {
	    if (outfile) {
	      fprintf (stderr, "stdout already redirected to %s\n", outfile);
	      abort ();
	    }
	    redirection = STDOUT | CONCAT | OVRWRT;
	  }
	  else if (strncmp(">&!+",(*pargv)[i],j=4) == 0) {
	    if (errfile) {
	      fprintf (stderr, "stderr already redirected to %s\n", errfile);
	      abort ();
	    }
	    redirection = STDERR | VMSFMT | OVRWRT;
	    if (!outfile)
	      redirection |= STDOUT;
	  }
	  else if (strncmp(">!+", (*pargv)[i],j=3) == 0) {
	    if (outfile) {
	      fprintf (stderr, "stdout already redirected to %s\n", outfile);
	      abort ();
	    }
	    redirection = STDOUT | VMSFMT | OVRWRT;
	  }
	  else if (strncmp(">&!", (*pargv)[i],j=3) == 0) {
	    if (errfile) {
	      fprintf (stderr, "stderr already redirected to %s\n", errfile);
	      abort ();
	    }
	    redirection = STDERR | OVRWRT;
	    if (!outfile)
	      redirection |= STDOUT;
	  }
	  else if (strncmp(">+",  (*pargv)[i],j=2) == 0) {
	    if (outfile) {
	      fprintf (stderr, "stdout already redirected to %s\n", outfile);
	      abort ();
	    }
	    redirection = STDOUT | VMSFMT;
	    if (!outfile)
	      redirection |= STDOUT;
	  }
	  else if (strncmp(">&",  (*pargv)[i],j=2) == 0) {
	    if (errfile) {
	      fprintf (stderr, "stderr already redirected to %s\n", errfile);
	      abort ();
	    }
	    redirection = STDERR;
	    if (!outfile)
	      redirection |= STDOUT;
	  }
	  else if (strncmp(">!",  (*pargv)[i],j=2) == 0) {
	    if (outfile) {
	      fprintf (stderr, "stdout already redirected to %s\n", outfile);
	      abort ();
	    }
	    redirection = STDOUT | OVRWRT;
	  }
	  else {		/* plain redirection */
	    if (outfile) {
	      fprintf (stderr, "stdout already redirected to %s\n", outfile);
	      abort ();
	    }
	    redirection = STDOUT;
	    j = 1;
	  }
	  if ((*pargv)[i][j])
	    (*pargv)[i] += j;
	  else
	    i++;
	  break;
	default: break;		/* Can't happen */
	}
	goto skip_to_next;
      }
      else if (*(temp-1) == '\\') {
	/* Not really redirected */
	size_t n = temp - 1 - (*pargv)[i];
	if (sz <= ac)
	  av = (char **) xrealloc (av, (sz=ac+MEMINC) * sizeof (char *));
	append_argument (&av[ac], (*pargv)[i], n);
	(*pargv)[i] = temp + 1;
	goto skip_to_next;
      }
      else {
	/* Really redirected, but embedded */
	size_t n = temp - (*pargv)[i];
	if (sz <= ac)
	  av = (char **) xrealloc (av, (sz=ac+MEMINC) * sizeof (char *));
	append_argument (&av[ac], (*pargv)[i], n);
	(*pargv)[i] = temp;
	/* Fall through and handle redirection next time */
	i--;
      }
    }
    else {
      /* Just copy/cat the original */
      if (sz <= ac)
	av = (char **) xrealloc (av, (sz=ac+MEMINC) * sizeof (char *));
      append_argument (&av[ac], (*pargv)[i], strlen ((*pargv)[i]));
    }
    iglob = ac;			/* point to first expansion */
    /* Substitute special characters */
    if (strstr(av[ac],"\\*") ||
	strstr(av[ac],"\\%") ||
	strstr(av[ac],"\\?"))
      /* Wildcard is quoted, can't glob, even if other wildcards
       * are not quoted.
       */
      iglob = 0;
    esctrans (av[ac], av[ac]);
    if (iglob && strpbrk(av[ac],"*%?") && av[ac][0] != '-') {
      /* Expand _only_ if wildcards are present.
       *   Othewise `tar xvf foo.tar' might expand to `tar xvf.txt foo.tar',
       *   i.e., the string "xvf" may match a file name field.
       * Leading '-' looks like an option, and is illegal for VMS filenames.
      */
      temp = strdup (av[ac]);
      version = strchr (temp, ';') != NULL;
      if ((filelist = glob(temp)) != NULL) {
	char **x = filelist;
	while (*x) {
	  char *file_version;
	  if (sz <= ac)
	    av = (char **) xrealloc (av, (sz=ac+MEMINC) * sizeof (char *));
	  file_version = strchr (*x, ';');
	  if (!version && file_version)
	    *file_version = 0;
	  av[ac] = *x;
	  downcase (av[ac]);
	  fixpath (av[ac]);
	  ac++;
	  x++;
	}
	ac--;
	free (filelist);
      }
      else {
	fprintf (stderr, "%s: No match\n", av[ac]);
	exit (SS$_ABORT | STS$M_INHIB_MSG);
      }
      /* Demand unambiguous wildcard match */
      if ((redirection != NONE) && (iglob != ac)) {
	fprintf (stderr, "%s: Ambiguous redirection\n", temp);
	exit (SS$_ABORT | STS$M_INHIB_MSG);
      }
      free (temp);
    }
    if (redirection) {
      /* Make sure wildcard wasn't quoted */
      if (strpbrk(av[ac],"*%?")) {
	fprintf (stderr, "%s: Illegal filename\n", av[ac]);
	exit (SS$_ABORT | STS$M_INHIB_MSG);
      }
      /* Set filename and flags */
      if (redirection & STDOUT) {
	outfile = av[ac];
	outflags = redirection;
      }
      if (redirection & STDERR) {
	errfile = av[ac];
	errflags = redirection;
      }
      if (redirection & STDIN) {
	{
	  FILE *test = fopen (av[ac], "r");
	  if (test == 0)
	    {
	      fprintf (stderr, "%s: No such file\n", av[ac]);
	      exit (SS$_ABORT | STS$M_INHIB_MSG);
	    }
	  fclose (test);
	}
	infile = av[ac];
	inflags = redirection;	/* not used */
      }
      redirection = NONE;
      ac--;
    }
    i++;
    ac++;
    iglob = 0;
    if (sz <= ac)
      av = (char **) xrealloc (av, (sz=ac+MEMINC) * sizeof (char *));
    av[ac] = NULL;
  skip_to_next: ; /* Some compilers absolutelly need a statement here.  */
  }
  /* Set the new argv, argc */
  av[ac] = NULL;
  if (sz > ac)
    av = (char **) xrealloc (av, (ac+1) * sizeof (char *));
  *pargc = ac;
  *pargv = av;
  /* Actually do the redirection */
  if (outfile) {
    freopen (outfile, "w", stdout);
#ifdef ENABLE_HACKARGV_REMOVE_EMPTY
    chkfile (stdout);
#endif
  }
  if (errfile) 
    if (errfile == outfile)
      stderr = stdout;
    else {
      freopen (errfile, "w", stderr);
#ifdef ENABLE_HACKARGV_REMOVE_EMPTY
      chkfile (stderr);
#endif
    }
  if (infile)
    freopen (infile, "r", stdin);
  return (0);
}


/*
 * int background (char *cmdline)
 *
 * Synopsis:
 *   Spawn a subprocess to execute CMDLINE. 
 *
 * Description:
 *   The subprocess shares SYS$INPUT and SYS$OUTPUT with the parent.
 *   Returns the process id of the subprocess.
 *
 * Author:
 *   Roland B Roberts (roberts@nsrl.rochester.edu)
 *   March 1994
 */
  
#include <clidef.h>
#include <ssdef.h>
#include <errno.h>
#include <descrip.h>

int background (char *cmdline)
{
  int status, flags, pid;
  size_t command_length = strlen (cmdline);
  struct dsc$descriptor_s command = { 0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0 };
  /*
   * We don't know the name by which this program was invoked, so 
   * reinvoke the command using mcr.
   */
  if (command_length > USHRT_MAX - 4)
    {
      errno = E2BIG;
      vaxc$errno = SS$_ABORT;
      return -1;
    }
  command.dsc$a_pointer = xmalloc ((command_length + 5) * sizeof(char));
  sprintf (command.dsc$a_pointer, "mcr %s", cmdline);
  command.dsc$w_length = strlen(command.dsc$a_pointer);
  flags = CLI$M_NOWAIT;
  status = lib$spawn (&command, 0, 0, &flags, 0, &pid, 0, 0, 0, 0, 0, 0);
  free (command.dsc$a_pointer);
  if (status & 1)
    return (pid);
  else {
    errno = EVMSERR;
    vaxc$errno = status;
    return (-1);		/* Is -1 a valid pid on  VMS? */
  }
}

#ifdef ENABLE_HACKARGV_REMOVE_EMPTY
/*
 * Code associated with the exit handler.
 */
#include <stsdef.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stat.h>
#include <nam.h>
typedef struct exit_handler_control_block
{
  struct exhcb *link;
  int (*exh_routine)();
  int argcount;
  void *sadd;
  int status; } EXHCB;

/* Forward declaration */
int XHdelete (int *);

static EXHCB deleteXHCB = { 0, &XHdelete, 1, &deleteXHCB.status, 0 };
static int fcount = 0;
static char **fname = NULL;
static FILE **flist = NULL;
static int flsize = 0;

/*
 * Build a list of streams and their associated filenames to be checked
 * at image rundown.  These files will be closed and (if empty) deleted.
 * The exit handler will be declared the first time this is called.
 */
int chkfile (FILE *stream)
{
  char path[NAM$C_MAXRSS+1];
  int status;

  if (flsize == 0) {
    flist  = (FILE **) xmalloc ((flsize=10) * sizeof(FILE *));
    fname  = (char **) xmalloc ((flsize=10) * sizeof(char *));
    status = sys$dclexh (&deleteXHCB);
    if (!(status & STS$M_SUCCESS)) {
      errno = EVMSERR;
      vaxc$errno = status;
      free (flist);
      free (fname);
      flist = NULL;
      fname = NULL;
      flsize = 0;
      return -1;
    }
  }
  else if (flsize <= fcount) {
    int new_size = flsize + 10;
    flist = (FILE **) xrealloc (flist, new_size * sizeof(FILE *));
    fname = (char **) xrealloc (fname, new_size * sizeof(char *));
    flsize = new_size;
  }

  if (!fgetname (stream, path))
    return -1;
  flist[fcount]  = stream;
  fname[fcount] = xmalloc ((1+strlen(path)) * sizeof(char));
  strcpy(fname[fcount], path);
  fcount++;
  
  return 0;
}

/*
 * This is the actual exit handler.
 * This traverses the list of files created by chkfile() and deletes any
 * empty ones.  The idea is to insure that files created because stdin or
 * stdout were reopened aren't left.  This is not what happens on unix,
 * but is what happens if you redefine sys$output or sys$error.
 */
int XHdelete (int *status)
{
  int i;
  struct stat stbuf;

  for (i = 0; i < fcount; i++)
  {
    fclose (flist[i]);
    stat (fname[i], &stbuf);
    if (stbuf.st_size == 0)
      delete (fname[i]);
  }
  return 0;
}
#endif
