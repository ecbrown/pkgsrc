/* VAX/VMS getmail---Read new messages from user's mailbox.
   Copyright (C) 1993 Roland B. Roberts.

getmail is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

getmail is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with getmail; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
 * The following is not yet implemented (should it be?):
 *   o Read a someone else's mail
 *
 * Modification History
 *   24 Aug 94 - RBR -
 *      + read directly from external file if there is one.  This prevents
 *        long messages from being truncated anad give significant I/O
 *        speed improvement
 *      + all-round better error handling.
 *   22 Aug 94 - RBR -
 *      + fixed exception handler to recognize most of the mail error
 *        messages we are interested in
 *      + changed main loop to continue until MAIL$_NOMOREMSG is returned
 */

char *version_string = "GNU VMS getmail version 1.0";

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <ssdef.h>
#include <stsdef.h>
#include <descrip.h>
#include <unixio.h>
#include <file.h>
#ifdef __GNUC__
# define variant_struct struct
# define variant_union union
#endif
/* #ifdef VMS5_5 */
#include <maildef.h>
/* #else
#include "maildef.h"
#endif */

#include <mailmsgdef.h>
#include "mail.h"
#include "getopt.h"

#include <mail$routines.h>	/* ttn */
#include <sys/time.h>		/* ttn */
#include <starlet.h>		/* ttn */

char *xmalloc (), *xrealloc ();
void error ();
void fatal ();

/* Name this program was invoked with.  */
char *progname;

/* Buffer size for output fields.
 * VMS MAIL$... routines want no more than 255 bytes
 * We add onee for the null */
#define BUFLEN 256

/* Buffer size we use for file I/O.
 * This gets used for messages with external file IDs */
#define FILEBUFLEN 8192

#ifndef TRUE
# define TRUE  (1)
# define FALSE (0)
#endif

#define VSTRSIZE 256

/* VMS Variable string format, suitable for use with dsc$descriptor_vs */
struct Vstring
{
  short length;
  char body[VSTRSIZE];
};

#if 0 /* ttn */
/* Shortened version of flagsdef */
union mail$flags
{
  unsigned short int mail$w_flags; /* Flags word */
  struct
    {
      unsigned mail$v_newmsg  : 1; /* This is a new message */
      unsigned mail$v_replied : 1; /* This message has been replied to */
      unsigned mail$v_del     : 1; /* This message is deleted */
      unsigned mail$v_extmsg  : 1; /* Message text in external file */
      unsigned mail$v_extfnf  : 1; /* External message file not found */
      unsigned mail$v_sysmsg  : 1; /* Message text in system file */
      unsigned mail$v_extnstd : 1; /* External file is not var-seq file */
      unsigned mail$v_marked  : 1; /* This message has been marked */
      unsigned mail$v_recmode : 1; /* This message should be read in record mode */
      unsigned mail$v_fill_2  : 7;
  } field;
};
#endif /* 0 */ /* ttn */

#ifdef VAXC
# define inline
#endif

/* The program name, as called in argv[0], will go here. */
static char *program;

getmail_help ()
{
  printf ("Usage: %s [-adnvV] [-f file] [-F folder] [-u user]\n\n\
  Options:\n\
    --append  -a         append to FILE (specified by --file)\n\
    --count   -c         return the mail count only\n\
    --delete  -d         delete messages after reading\n\
    --file    -f FILE    output to FILE\n\
    --folder  -F FOLDER  select mail from FOLDER, e.g., -F \"MAIL\"\n\
    --help    -h         display brief help message and exit\n\
    --keep    -k         keep deleted messages, do NOT purge wastebasket\n\
    --newmail -n         select new messages\n\
    --peek    -p         do not clear \"new\" bit\n\
    --reset   -r         reset the new mail count to the correct value\n\
    --user    -u USER    read mail for USER (not implemented!)\n\
    --version -v         return version information\n\
    --verbose -V         display progress messages\n\n\
Folder names should be enclosed in double-quotes to preserve case.\n",
	  program);
}

getmail_usage ()
{
  fprintf (stderr, "Usage: %s %s\n%s %s %s",
	   program, "[-acdnpvV] [-f file] [-F folder] [-u user]",
	   "For help, type:", program, "-h\n");
  exit (SS$_ABORT);
}

/* Actually truncating the message requires some RMS hacks.
 * This will let the user know about the problem but the truncated message
 * will still be there.
 * We declare folder and user as file global so we can put them in the
 * message header. */

static struct Vstring folder = { 0, "" }; /* VMS Mail folder name */
static struct Vstring user = { 0, "" };

static inline
ftrunc (out, pos)
  FILE *out;
  fpos_t pos;
{
  char timestamp[24];
  $DESCRIPTOR (timedsc, timestamp);
  short len;

  if (out != stdout)
    {
      sys$asctim (&len, &timedsc, 0, 0);
      timestamp[len] = 0;
      vaxc$errno = errno = 0;
      if (fsetpos (out, &pos) != 0)
	{
	  printf ("\nCouldn't set postion on file!\n");
	  perror ("fsetpos");
	}
    }
  fprintf (out,
	   "\f\nFrom:\t%s\t%s %s\nTo:\t%s\nCC:\t%s\nSubj:\t*** %s %s ***\n\n",
	   program, "\"VMS getmail\"", timestamp, user.body,
	   "", "Message truncated due to errors, original in", folder.body);
}

/* This is a simple "report the message and die" macro.
 * It is only suitable for early errors which requiree no cleanup */
#define errchk(status) if (status != SS$_NORMAL) \
{ fprintf(stderr, "\n%s: %s\n\tFile %s, Line %d\n", \
	  program, "error exit being forced", __FILE__, __LINE__); \
    exit (status); }


main (argc, argv)
  int argc;
  char **argv;
{
/* getopt_long options struct */
  static const struct option getmail_options[] =
    {
      {"append",   no_argument,       0, 'a' },
      {"count",    no_argument,       0, 'c' },
      {"delete",   no_argument,       0, 'd' },
      {"file",     required_argument, 0, 'f' },
      {"folder",   required_argument, 0, 'F' },
      {"help",     no_argument,       0, 'h' },
      {"keep",     no_argument,       0, 'k' },
      {"newmail",  no_argument,       0, 'n' },
      {"peek",     no_argument,       0, 'p' },
      {"reset",    no_argument,       0, 'r' },
      {"user",     required_argument, 0, 'u' },
      {"version",  no_argument,       0, 'v' },
      {"verbose",  no_argument,       0, 'V' },
      { 0, 0, 0, 0 }
    };

  /* Force most options to be explictly requested */
  int append  = FALSE;		/* Append to file, create if needed */
  int count   = FALSE;		/* Just give the mail count */
  int delmsg  = FALSE;		/* Delete the message after reading */
  int help    = FALSE;		/* Display help message and exit  */
  int keep    = FALSE;		/* Keep deleted messages, don't purge */
  int newmail = FALSE;		/* Only select new messages */
  int peek    = FALSE;		/* Don't clear new message bit */
  int reset   = FALSE;		/* Reset/fix new mail count */
  int verbose = FALSE;		/* Display progress messages */
  int version = FALSE;		/* Display version and exit */
  int badargs = FALSE;		/* Some conflicting arguments */
				/*  */
  unsigned filectx;		/* Context for mail$file... routines */
  unsigned msgctx;		/* Context for mail$message... routines */
  unsigned userctx;		/* Context for mail$user... routines */
  short rectype;		/* Mail message record type */
  short newcount;		/* New message count */
  long msgid;			/* Current message id */
  long *message;		/* Messages successfully processed */
  int Nmsgs;			/* Size of message array */

  char CC[BUFLEN];		/* VMS Mail CC header contents */
  char Date[BUFLEN];		/* VMS Mail CC Date contents */
  char From[BUFLEN];		/* VMS Mail From header contents */
  char Reply[BUFLEN];		/* VMS Mail Reply header contents */
  char Sender[BUFLEN];		/* VMS Mail Sender header contents */
  char Subject[BUFLEN];		/* VMS Mail Subject header contents */
  char To[BUFLEN];		/* VMS Mail To header contents */
  char ExternalID[BUFLEN];	/* Message External ID */
  char MailDirectory[BUFLEN];	/* Mail directory name */
  /* Corresponding lengths for the above */
  size_t lCC, lDate, lFrom, lReply, lSender, lSubject, lTo;
  size_t lExternalID, lMailDirectory;

  char *Buffer;			/* Message body record */
  int BufferSize = 0;		/* Allocated size of Buffer */
  size_t lBuffer;		/* Message body length */

  /* The Convienient shorthand definitions */
  /* Retrieve the message header information */
  struct itm$list3 headerlst[] = {
    { sizeof(msgid), MAIL$_MESSAGE_CURRENT_ID, &msgid, 0 },
    { BUFLEN-1, MAIL$_MESSAGE_CC, CC, &lCC },
    { BUFLEN-1, MAIL$_MESSAGE_DATE, Date, &lDate },
    { BUFLEN-1, MAIL$_MESSAGE_FROM, From, &lFrom },
    { BUFLEN-1, MAIL$_MESSAGE_REPLY_PATH, Reply, &lReply },
    { BUFLEN-1, MAIL$_MESSAGE_SENDER, Sender, &lSender },
    { BUFLEN-1, MAIL$_MESSAGE_SUBJECT, Subject, &lSubject },
    { BUFLEN-1, MAIL$_MESSAGE_TO, To, &lTo },
    { BUFLEN-1, MAIL$_MESSAGE_EXTID, ExternalID, &lExternalID },
    { 0, 0, 0, 0 } };

  /* Get the next record from the message.
   * Note: this will have to be re-initialized when Buffer is allocated */
  struct itm$list3 messagelst[] = {
    { BUFLEN-1, MAIL$_MESSAGE_RECORD, 0, &lBuffer },
    { sizeof(rectype), MAIL$_MESSAGE_RECORD_TYPE, &rectype, 0 },
    { 0, 0, 0, 0 } };

  /* Start the next message */
  struct itm$list3 nextitm[] = {
    { 0, MAIL$_MESSAGE_NEXT, 0, 0 },
    { 0, MAIL$_NOSIGNAL, 0, 0 },
    /* This doesn't work, why not?
     * { 0, MAIL$_MESSAGE_AUTO_NEWMAIL, 0, 0 }, */
    { 0, 0, 0, 0 } };

  /* Continue with the curent message */
  struct itm$list3 sameitm[] = {
    { 0, MAIL$_MESSAGE_CONTINUE, 0, 0 },
    { 0, MAIL$_NOSIGNAL, 0, 0 },
    { 0, 0, 0, 0 } };

  /* A null item */
  struct itm$list3 nullitm[] = { { 0, 0, 0, 0 } };

  struct itm$list3 inlst[32];	/* General purpose itemlist */
  struct itm$list3 outlst[32];	/* General purpose itemlist */

  char *file = NULL;		/* Output file name (for message contents) */
  FILE *out = stdout;		/* Output file pointer */
  fpos_t lastpos;		/* Last "safe" position in output file */
  long status;			/* VMS return status */
  long selected;		/* Number of messages selected/processed */

  struct flagsdef mailflags;	/* Message flags */
  long autopurge;		/* User has done SET AUTO_PURGE */

  int i, cnt;			/* Local counters */
  int opt, option_index;	/* getopt variables */

  /* Initialize here. */
  Buffer = (char *) malloc (sizeof(char) * (BufferSize=FILEBUFLEN));

  strcpy (user.body, getenv("USER"));
  user.length = strlen(user.body);
  
  /* Hack argv[0] to be "pretty", use `program' to point to the result. */
  program = strrchr(*argv,']');
  if (program)
    {
      program++;
      *strchr(program,'.') = 0;    
    }
  else
    program = *argv;
  
  *argv = program;
  
  progname = program;

  /* Handle the command line args */
  while (1)
    {
      opt = getopt_long (argc, argv, "acdf:npu:vDF:V",
		       getmail_options, &option_index);
      if (opt == EOF)
	break;
      switch (opt)
	{
	case 'a':
	  append = TRUE;
	  break;
	case 'c':
	  count = TRUE;
	  break;
	case 'd':
	  delmsg = TRUE;
	  break;
	case 'f':
	  file = optarg;
	  break;
	case 'h':
	  help = TRUE;
	  break;
	case 'k':
	  keep = TRUE;
	  break;
	case 'n':
	  newmail = TRUE;
	  break;
	case 'p':
	  peek = TRUE;
	  break;
	case 'u':
	  fprintf (stderr, "%s: --user not implemented\n", program);
	  strcpy (user.body, optarg);
	  user.length = strlen(user.body);
	  badargs = TRUE;
	  break;
	case 'v':
	  printf ("%s", version_string);
	  break;
	case 'F':
	  strcpy (folder.body, optarg);
	  folder.length = strlen(folder.body);
	  break;
	case 'V':
	  verbose = TRUE;
	  break;
	case 0:
	case '?':
	default:
	  badargs = TRUE;
	}
    }

  /* Now we check for conflicting arguments */

  /* Only the folder "NEWMAIL" is allowed for --newmail --folder FOLDER
   * Note: the folder specification is *required* in MAIL$MESSAGE_SELECT */
  if (newmail && folder.length != 0 && strcmp(folder.body,"NEWMAIL") != 0)
    {
      fprintf (stderr, "%s: conflicting options --newmail and --folder %s\n",
	       program, folder.body);
      badargs = TRUE;
    }
  
  if (!newmail && folder.length == 0)
    {
      fprintf (stderr, "%s: no folder selected, use --newmail or --folder FOLDER\n",
	       program);
      badargs = TRUE;
    }

  if (peek && delmsg)
    {
      fprintf (stderr, "%s: conflicting options --peek and --delete\n",
	       program);
      badargs = TRUE;
    }

  /* Display help information then exit */
  if (help || badargs)
    {
      getmail_help ();
      if (badargs)
	exit (SS$_ABORT);
      else
	exit (SS$_NORMAL);
    }

  /* Get some general information from the user profile */
  userctx = 0;
  $SETITM3 (inlst[0], user.length, MAIL$_USER_USERNAME, user.body, 0);
  $CLRITM3 (inlst[1]);
  $SETITM3 (outlst[0], sizeof(newcount), MAIL$_USER_NEW_MESSAGES, &newcount, 0);
  $SETITM3 (outlst[1], BUFLEN-1, MAIL$_USER_FULL_DIRECTORY, MailDirectory, &lMailDirectory);
  $SETITM3 (outlst[2], sizeof(autopurge), MAIL$_USER_AUTO_PURGE, &autopurge, 0);
  $CLRITM3 (outlst[3]);
  status = mail$user_begin (&userctx, nullitm, nullitm);
  errchk (status);
  status = mail$user_get_info (&userctx, inlst, outlst);
  errchk (status);
  status = mail$user_end (&userctx, nullitm, nullitm);
  errchk (status);
  
  /* For some reason, the return value is -2 if autopurge is not set,
   * and -1 if it is set. */
  autopurge = ~autopurge ? 0 : 1;
  MailDirectory[lMailDirectory] = 0;
  if (verbose)
    {
      printf ("\nNew mail count for %s is %d", user.body, newcount);
      printf ("\nAuto purge is %s", autopurge ? "on" : "off");
      printf ("\nMail directory is %s", MailDirectory);
    }

  /* If --count --newmail was specified, just give the count and exit.
   * If --folder "NEWMAIL" was also specified, defer until the folder is open. */
  if (count && newmail && folder.length != 0)
    {
      printf ("\nYou have %d new mail message%s", newcount,
	      ((newcount > 1 || newcount == 0) ? "s" : ""));
      exit (SS$_NORMAL);
    }
  
  /* Actually open the mail file and prepare to read messages */
  filectx = 0;
  status = mail$mailfile_begin (&filectx, nullitm, nullitm);
  errchk (status);
  status = mail$mailfile_open (&filectx, nullitm , nullitm);
  errchk (status);

  msgctx = 0;
  $SETITM3 (inlst[0], sizeof(filectx), MAIL$_MESSAGE_FILE_CTX, &filectx, 0);
  $CLRITM3 (inlst[1]);
  status = mail$message_begin (&msgctx, inlst, nullitm);
  errchk (status);

  /* If --newmail was specified, force the folder to be "NEWMAIL" */
  if (newmail)
    {
      strcpy(folder.body, "NEWMAIL");
      folder.length = strlen(folder.body);
    }
  
  if (verbose)
    printf ("\nSelecting folder %s", folder.body);
  
  /* This will complain if the selected folder does not exist.
   * I suppose that's as good a way as any to let the user know.... */
  $SETITM3 (inlst[0], folder.length, MAIL$_MESSAGE_FOLDER, folder.body, 0);
  $SETITM3 (inlst[1], 0, MAIL$_NOSIGNAL, 0, 0);
  if (newmail)
    {
#ifdef __GNUC__
      mailflags.mail$w_flags = 0;
      mailflags.mail$r_fill_0.mail$r_fill_0.mail$v_newmsg = 1;
#else /* VAXC */
      mailflags.mail$w_flags = 0;
      mailflags.mail$v_newmsg = 1;
#endif
      $SETITM3 (inlst[2], sizeof(mailflags), MAIL$_MESSAGE_FLAGS, &mailflags, 0);
      $CLRITM3 (inlst[3]);
      if (verbose)
	printf ("\nSelecting new messages");
    }
  else
    $CLRITM3 (inlst[2]);
  
  $SETITM3 (outlst[0], sizeof(selected), MAIL$_MESSAGE_SELECTED, &selected, 0);
  $CLRITM3 (outlst[1]);
  status = mail$message_select (&msgctx, inlst, outlst);
  if ((status != MAIL$_NOTEXIST)) /* && (strcmp(folder.body,"NEWMAIL") != 0))*/
    { errchk (status); }
  else if (count || verbose)
    {
      printf ("\nFolder %s contains %d message%s", folder.body, selected,
	      ((selected > 1 || selected == 0) ? "s" : ""));
      if (count)
	exit (SS$_NORMAL);
    }

  /* If selected == 0, quit now. */
  if (selected == 0)
    if (newmail)
      exit (SS$_NORMAL);
    else
      { errchk (status); }
  
  /* Allocate some space for keeping message id's */
  message = (long *) malloc (sizeof(long) * (Nmsgs=2*selected));

  /* If no file was specified, skip this.
   * Note: RMS may fail to open the file.  The most likely error is
   * "RMS-E-CRE, ACP file create failed", probably caused by lack of quota. */
  if (file != NULL)
    {
      errno = 0;
      if (append)
	out = fopen (file, "a+");
      else
	out = fopen (file, "w+");
      if (!out)
	{
	  fprintf (stderr, "%s: %s %s",
		   program, "can't open file", file);
	  exit (SS$_ABORT);
	}
      if (verbose)
	printf ("\nFile %s opened for %s", file,
		(append) ? "append" : "write");
    }

  errno = 0;
  if (file && fgetpos (out, &lastpos) != 0)
    {
      printf ("\nCouldn't get postion on file!\n");
      perror ("fgetpos");
      abort();
    }
  /* mail$message_get signals an error we there are no more messages.
     Our error handler will catch that, but we need to reset the
     selected to be in agreement with how many messages we actually
     read, which may be more or less than selected.  (For example, the
     new mail count may be screwed up.) */
  for (cnt = 0; ; cnt++)
    {
      status = mail$message_get (&msgctx, nextitm, headerlst);
      if (status == MAIL$_NOMOREMSG)
	{
	  if (verbose)
	    printf ("\nNo more messages");
	  selected = cnt;
	  break;
	}
      if (verbose)
	printf ("\nProcessing message %d...", cnt+1);
      /* Null terminate the returned values. */
      CC[lCC] = 0;
      Date[lDate] = 0;
      From[lFrom] = 0;
      Reply[lReply] = 0;
      Sender[lSender] = 0;
      Subject[lSubject] = 0;
      To[lTo] = 0;
      ExternalID[lExternalID] = 0;

      if (verbose)
	if (lExternalID)
	  printf ("\nExternal file is %s%s", MailDirectory, ExternalID);
	else
	  printf ("\nNo external mail file");

      /* Print out the message headers just like VMS Mail does.
       *
       * If write fails, it is most like a quota problem, so just abort.
       * This means the output file may be truncated in mid-message.
       * Also, I *think* output can fail once but you can still write more
       * until you exceed your overdraft.  I'm not sure how to try that....
       */
      sprintf (Buffer, "\f\nFrom:\t%s %s\nTo:\t%s\nCC:\t%s\nSubj:\t%s\n\n",
	       From, Date, To, CC, Subject);
      vaxc$errno = errno = 0;
      if (fwrite(Buffer, strlen(Buffer), 1, out) != 1)
	{
	  fprintf (stderr, "\n%s: %s\n",
		   program,
		   "incomplete fwrite on message header");
	  selected = cnt;
	  /* Until we have a real ftrunc(), just do the cleanup */
	  /* ftrunc (out, lastpos); */
	  if (file)
	    fclose (out);
	  goto cleanup;
	}

      /* If the message is in an external file, then open that file directly
       * and retrieve the message.  If we attempt to get it through via
       * mail$message_get, the buffer may be silently truncated */
      if (lExternalID)
	{
	  FILE *ext;
	  sprintf (Buffer, "%s%s", MailDirectory, ExternalID);
	  errno = 0;
	  ext = fopen (Buffer, "r", "ctx=rec");
	  if (!ext)
	    {
	      fprintf (stderr, "\n%s: %s%s%s\n",
		       program, "couldn't open ",
		       MailDirectory, ExternalID);
	      fclose (ext);
	      selected = cnt;
	      ftrunc (out, lastpos);
	      if (file)
		fclose (out);
	      goto cleanup;
	    }
	  lBuffer = fread (Buffer, sizeof(char), BufferSize, ext);
	  while (lBuffer)
	    {
	      if (fwrite (Buffer, sizeof(char)*lBuffer, 1, out) != 1)
		{
		  fprintf (stderr, "\n%s: %s%s%s\n",
			   program, "incomplete fwrite from external file ",
			   MailDirectory, ExternalID);
		  selected = cnt;
		  ftrunc (out, lastpos);
		  if (file)
		    fclose (out);
		  goto cleanup;
		}
	      lBuffer = fread (Buffer, sizeof(char), BufferSize, ext);
	    }
	  fclose (ext);
	}
      else
	/* Message is not in an external file */
	{
	  $SETITM3 (messagelst[0], BUFLEN-1, MAIL$_MESSAGE_RECORD, Buffer, &lBuffer);
	  status = mail$message_get (&msgctx, sameitm, messagelst);
	  while (status != MAIL$_NOMOREREC)
	    {
	      if (rectype == MAIL$_MESSAGE_TEXT)
		{
		  /* Same as with headers except for the message body now */
		  Buffer[lBuffer++] = '\n';
		  Buffer[lBuffer]   = 0;
		  errno = 0;
 		  if (fwrite (Buffer, lBuffer, 1, out) != 1)
		    {
		      fprintf (stderr, "\n%s: %s%s%s\n",
			       program, "incomplete fwrite from message ",
			       MailDirectory, ExternalID);
		      selected = cnt;
		      ftrunc (out, lastpos);
		      if (file)
			fclose (out);
		      goto cleanup;
		    }
		}
	      status = mail$message_get (&msgctx, sameitm, messagelst);
	    }
	}
      /* Flush the message to disk */
      fflush (out);
      errno = 0;
      if (file && fgetpos (out, &lastpos) != 0)
	{
	  printf ("\nCouldn't get postion on file!\n");
	  perror ("fsetpos");
	  abort();
	}
      if (Nmsgs <= cnt)
	if (!realloc (message, sizeof(long) * (Nmsgs*=2)))
	  {
	    fprintf (stderr, "%s: %s", program,
		     "memory exhausted while allocating message id space\n");
	    selected = cnt;
	    ftrunc (out, lastpos);
	    if (file)
	      fclose (out);
	    goto cleanup;
	  }
      message[cnt] = msgid;
      if (verbose)
	printf (".  Done.");
    }
  
  /* Close the output before deleting any messages, but don't close stdout */
  if (file)
    fclose (out);
  
  /* This is where we end up if there was an error while reading or
   * writing a message */
 cleanup:

  /* Decrement the new mail count */
  if (newmail)
    {
      if (verbose)
	printf ("\nDecrementing new count...");
      userctx = 0;
      $SETITM3 (inlst[0], user.length, MAIL$_USER_USERNAME, user.body, 0);
      $CLRITM3 (inlst[1]);
      $SETITM3 (outlst[0], sizeof(newcount), MAIL$_USER_NEW_MESSAGES, &newcount, 0);
      $CLRITM3 (outlst[1]);
      mail$user_begin (&userctx, nullitm, nullitm);
      mail$user_get_info (&userctx, inlst, outlst);
      newcount -= selected;
      if (newcount < 0)
	newcount = 0;
      $SETITM3 (inlst[0], sizeof(newcount), MAIL$_USER_SET_NEW_MESSAGES, &newcount, 0);
      $CLRITM3 (inlst[1]);
      mail$user_set_info (&userctx, inlst, nullitm);
      mail$user_end (&userctx, nullitm, nullitm);
      if (verbose)
	printf ("to %d", newcount);
    }
  
  /* Now we reprocess the messages and move them out of NEWMAIL
   * MAIL$_MESSAGE_AUTO_NEWMAIL seems to be broken, so we have to do this
   * manually */
  if (newmail)
    for (i = 0; i < selected; i++)
      {
	if (verbose)
	  printf ("\nMoving message id=%d out of NEWMAIL", message[i]);
	$SETITM3 (inlst[0], sizeof(message[i]), MAIL$_MESSAGE_ID, &message[i], 0);
	$CLRITM3 (inlst[1]);
	$SETITM3 (outlst[0], sizeof(mailflags), MAIL$_MESSAGE_RETURN_FLAGS, &mailflags, 0);
	$CLRITM3 (outlst[1]);
	status = mail$message_get (&msgctx, inlst, outlst);

#ifdef __GNUC__
	mailflags.mail$r_fill_0.mail$r_fill_0.mail$v_newmsg = 0;
#else /* VAXC */
	mailflags.mail$v_newmsg = 0;
#endif
	$SETITM3 (inlst[1], sizeof(mailflags), MAIL$_MESSAGE_FLAGS, &mailflags, 0);
	$CLRITM3 (inlst[2]);
	status = mail$message_modify (&msgctx, inlst, nullitm);

	if (delmsg)
	  /* Either delete or move */
	  {
	    if (verbose)
	      printf ("\nDeleting message id=%d out of NEWMAIL", message[i]);
	    $CLRITM3 (inlst[1]);
	    mail$message_delete (&msgctx, inlst, nullitm);
	  }
	else
	  {
	    $SETITM3 (inlst[1], 4, MAIL$_MESSAGE_FOLDER, "MAIL", 0);
	    $SETITM3 (inlst[2], 0, MAIL$_MESSAGE_DELETE, 0, 0);
	    $CLRITM3 (inlst[3]);
	    mail$message_copy (&msgctx, inlst, nullitm);
	  }
      }
  else if (delmsg)
    {
      for (i = 0; i < selected; i++)
	{
	  if (verbose)
	    printf ("\nDeleting message id=%d out of NEWMAIL", message[i]);
	  
	  $SETITM3 (inlst[0],sizeof(message[i]), MAIL$_MESSAGE_ID, &message[i], 0);
	  $CLRITM3 (inlst[1]);
	  mail$message_delete (&msgctx, inlst, nullitm);
	}
    }
  
  /* We are done with all message processing */
  mail$message_end (&msgctx, nullitm, nullitm);
  
  /* Close everything and exit.  Check for autopurge */
  if (autopurge && !keep)
    {
      if (verbose)
	printf ("\nPurging wastebasket.");
      mail$mailfile_purge_waste (&filectx, nullitm, nullitm);
    }

  status = mail$mailfile_close (&filectx, nullitm, nullitm);
  status = mail$mailfile_end (&filectx, nullitm, nullitm);
  
  if (verbose)
    /* printf ("\nAll done!", newcount); */
    printf ("\nAll done!");	/* ttn */
  
  exit (SS$_NORMAL);
}

/* Exit codes for success and failure.  */
#ifdef VMS
#define	GOOD	1
#define BAD	0
#else
#define	GOOD	0
#define	BAD	1
#endif

/* Print error message and exit.  */

/* VARARGS1 */
void
fatal (s1, s2)
     char *s1, *s2;
{
  error (s1, s2);
  exit (BAD);
}

/* Print error message.  `s1' is printf control string, `s2' is arg for it. */

/* VARARGS1 */
void
error (s1, s2)
     char *s1, *s2;
{
  fprintf (stderr, "%s: ", progname);
  fprintf (stderr, s1, s2);
  fprintf (stderr, "\n");
}

/* Like malloc but get fatal error if memory is exhausted.  */

char *
xmalloc (size)
     unsigned int size;
{
  char *result = (char *) malloc (size);
  if (result == NULL)
    fatal ("virtual memory exhausted", 0);
  return result;
}

char *
xrealloc (ptr, size)
     char *ptr;
     unsigned int size;
{
  char *result = (char *) realloc (ptr, size);
  if (result == NULL)
    fatal ("virtual memory exhausted", 0); /* ttn */
  return result;
}

/* vmsgetmail.c ends here */
