$NetBSD$

Use ncurses' native QIO wrapper for character-at-a-time terminal input and
restore the terminal before reporting input failure or Ctrl-C.

Validate terminal capabilities and mode changes.  OpenVMS provides vfork(2),
but not fork(2), so enable safe failed-exec handling, request raw VMS wait
statuses in this translation unit, and distinguish image-not-found safely.

--- term.c.orig
+++ term.c
@@ -94,6 +94,15 @@
  *
  */
 
+#ifdef __VMS
+#ifndef _POSIX_EXIT
+#define _POSIX_EXIT 1
+#endif
+#ifndef _VMS_WAIT
+#define _VMS_WAIT 1
+#endif
+#endif
+
 #include "config.h"
 #include "ispell.h"
 #include "proto.h"
@@ -108,9 +117,21 @@
 #endif
 #endif
 #include <signal.h>
+#include <errno.h>
 #include <sys/ioctl.h>
 #include <sys/wait.h>
 
+#ifdef __VMS
+#include <climsgdef.h>
+#include <unixlib.h>
+#ifdef SIGTSTP
+#undef SIGTSTP
+#endif
+extern int tcgetattr (int, struct termios *);
+extern int tcsetattr (int, int, const struct termios *);
+extern ssize_t _nc_vms_read (int, void *, size_t);
+#endif
+
 void		ierase P ((void));
 void		imove P ((int row, int col));
 void		inverse P ((void));
@@ -158,12 +179,14 @@
 
 void inverse ()
     {
-    tputs (so, 10, iputch);
+    if (so)
+	tputs (so, 10, iputch);
     }
 
 void normal ()
     {
-    tputs (se, 10, iputch);
+    if (se)
+	tputs (se, 10, iputch);
     }
 
 void backup ()
@@ -181,6 +204,27 @@
     return putchar (c);
     }
 
+#ifdef __VMS
+/* Read one terminal byte through ncurses' native OpenVMS QIO wrapper.  The
+ * C RTL read path remains record-oriented in pass-through terminal mode. */
+int vms_get_keystroke ()
+    {
+    unsigned char ch;
+    ssize_t nread;
+
+    nread = _nc_vms_read (0, &ch, 1);
+    if (nread == 1)
+	{
+	if (ch == 3)
+	    (void) raise (SIGINT);
+	return (int) ch;
+	}
+    perror (TERM_C_READ);
+    done (1);
+    return EOF;			/* NOTREACHED */
+    }
+#endif
+
 #if defined(TERMIOS)
 static struct termios	sbuf;
 static struct termios	osbuf;
@@ -205,18 +249,34 @@
 
 void terminit ()
     {
-#ifdef TIOCPGRP
-    int			tpgrp;
-#else
-#ifdef TIOCGPGRP
-    int			tpgrp;
+    char *		termname;
+#ifdef __VMS
+    int			feature_index;
 #endif
+#if (defined(TIOCPGRP) || defined(TIOCGPGRP)) && !defined(__VMS)
+    int			tpgrp;
 #endif
 #ifdef TIOCGWINSZ
     struct winsize	wsize;
 #endif /* TIOCGWINSZ */
 
-    tgetent (termcap, getenv ("TERM"));
+    termname = getenv ("TERM");
+    if (termname == NULL  ||  *termname == '\0'
+      ||  tgetent (termcap, termname) <= 0)
+	{
+	(void) fprintf (stderr, TERM_C_BAD_TERM,
+	  termname == NULL  ||  *termname == '\0' ? "(unset)" : termname);
+	exit (1);
+	}
+#ifdef __VMS
+    feature_index = decc$feature_get_index ("DECC$EXIT_AFTER_FAILED_EXEC");
+    if (feature_index < 0
+      ||  decc$feature_set_value (feature_index, 1, 1) < 0)
+	{
+	perror (TERM_C_VFORK_MODE);
+	exit (1);
+	}
+#endif
     termptr = termstr;
     BC = tgetstr ("bc", &termptr);
     cd = tgetstr ("cd", &termptr);
@@ -228,10 +288,21 @@
     se = tgetstr ("se", &termptr);	/* inverse video off */
     if ((sg = tgetnum ("sg")) < 0)	/* space taken by so/se */
 	sg = 0;
+    if (so == NULL  ||  se == NULL)
+	{
+	so = NULL;
+	se = NULL;
+	sg = 0;
+	}
     ti = tgetstr ("ti", &termptr);	/* terminal initialization */
     te = tgetstr ("te", &termptr);	/* terminal termination */
     co = tgetnum ("co");
     li = tgetnum ("li");
+    if (cm == NULL  ||  (cl == NULL  &&  cd == NULL))
+	{
+	(void) fprintf (stderr, TERM_C_MISSING_CAPS, termname);
+	exit (1);
+	}
 #ifdef TIOCGWINSZ
     if (ioctl (0, TIOCGWINSZ, (char *) &wsize) >= 0)
 	{
@@ -250,6 +321,10 @@
 	co = atoi (getenv ("COLUMNS"));
     if (getenv ("LINES") != NULL)
 	li = atoi (getenv ("LINES"));
+    if (co <= 0)
+	co = 80;
+    if (li <= 0)
+	li = 24;
 #if MAX_SCREEN_SIZE > 0
     if (li > MAX_SCREEN_SIZE)
 	li = MAX_SCREEN_SIZE;
@@ -295,8 +370,11 @@
 	(void) fprintf (stderr, TERM_C_NO_BATCH);
 	exit (1);
 	}
-    (void) tcgetattr (0, &osbuf);
-    termchanged = 1;
+    if (tcgetattr (0, &osbuf) < 0)
+	{
+	perror (TERM_C_TTY_MODE);
+	exit (1);
+	}
 
     sbuf = osbuf;
     sbuf.c_lflag &= ~(ECHO | ECHOK | ECHONL | ICANON);
@@ -304,7 +382,12 @@
     sbuf.c_iflag &= ~(INLCR | IGNCR | ICRNL);
     sbuf.c_cc[VMIN] = 1;
     sbuf.c_cc[VTIME] = 1;
-    (void) tcsetattr (0, TCSADRAIN, &sbuf);
+    if (tcsetattr (0, TCSADRAIN, &sbuf) < 0)
+	{
+	perror (TERM_C_TTY_MODE);
+	exit (1);
+	}
+    termchanged = 1;
 
     uerasechar = osbuf.c_cc[VERASE];
     ukillchar = osbuf.c_cc[VKILL];
@@ -336,7 +419,7 @@
     (void) sigsetmask (1<<(SIGTSTP-1) | 1<<(SIGTTIN-1) | 1<<(SIGTTOU-1));
 #endif
 #endif
-#ifdef TIOCGPGRP
+#if defined(TIOCGPGRP) && !defined(__VMS)
     if (ioctl (0, TIOCGPGRP, (char *) &tpgrp) != 0)
 	{
 	(void) fprintf (stderr, TERM_C_NO_BATCH);
@@ -405,8 +488,9 @@
 SIGNAL_TYPE done (signo)
     int		signo;
     {
-    if (tempfile[0] != '\0')
+    if (tempfile_created  &&  tempfile[0] != '\0')
 	(void) unlink (tempfile);
+    tempfile_created = 0;
     if (termchanged)
 	{
 	if (te)
@@ -422,7 +506,7 @@
 #endif
 #endif
 	}
-    exit (0);
+    exit (signo == 0 ? 0 : 1);
     }
 
 #ifdef SIGTSTP
@@ -480,6 +564,11 @@
 
 void stop ()
     {
+#ifdef __VMS
+    /* OpenVMS terminals do not provide Unix-style job control here. */
+    (void) putchar (7);
+    return;
+#else
 #ifdef SIGTSTP
     onstop (SIGTSTP);
 #else
@@ -495,6 +584,7 @@
     shescape ("");
 #endif /* NEED_SHELLESCAPE */
 #endif /* SIGTSTP */
+#endif /* __VMS */
     }
 
 /* Fork and exec a process.  Returns NZ if command found, regardless of
@@ -509,6 +599,8 @@
     char *	cp = buf;
     int		i = 0;
     int		termstat;
+    pid_t	child;
+    pid_t	waited;
 
     /* parse buf to args (destroying it in the process) */
     while (*cp != '\0')
@@ -517,12 +609,16 @@
 	    ++cp;
 	if (*cp == '\0')
 	    break;
+	if (i >= (sizeof argv / sizeof argv[0]) - 1)
+	    return 0;
 	argv[i++] = cp;
 	while (*cp != ' '  &&  *cp != '\t'  &&  *cp != '\0')
 	    ++cp;
 	if (*cp != '\0')
 	    *cp++ = '\0';
 	}
+    if (i == 0)
+	return 0;
     argv[i] = NULL;
 
 #if defined(TERMIOS)
@@ -542,21 +638,45 @@
     (void) signal (SIGTTOU, oldttou);
     (void) signal (SIGTSTP, oldtstp);
 #endif
-    if ((i = fork ()) == 0)
+#ifdef __VMS
+    child = vfork ();
+#else
+    child = fork ();
+#endif
+    if (child == 0)
 	{
 	(void) execvp (argv[0], (char **) argv);
 	_exit (123);		/* Command not found */
 	}
-    else if (i > 0)
+    else if (child > 0)
 	{
-	while (wait (&termstat) != i)
-	    ;
-	termstat = (termstat == (123 << 8)) ? 0 : -1;
+	do
+	    waited = waitpid (child, &termstat, 0);
+	while (waited < 0  &&  errno == EINTR);
+	if (waited != child)
+	    termstat = -1;
+#ifdef __VMS
+	else if (termstat == CLI$_IMAGEFNF)
+	    termstat = 0;
+	else
+	    termstat = -1;
+#else
+	if (waited == child  &&  WIFEXITED (termstat)
+	  &&  WEXITSTATUS (termstat) == 123)
+	    termstat = 0;
+	else
+	    termstat = -1;
+#endif
 	}
     else
 	{
+#ifdef __VMS
+	/* A failed exec returns here with a negative pseudo-PID. */
+	termstat = 0;
+#else
 	(void) printf (TERM_C_CANT_FORK, MAYBE_CR (stderr));
 	termstat = -1;		/* Couldn't fork */
+#endif
 	}
 
     if (oldint != SIG_IGN)
