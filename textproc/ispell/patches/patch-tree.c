$NetBSD$

Treat empty HOME and WORDLIST values as unset.  Initialize the secondary
dictionary pathname before inspecting it, use a safe relative personal
dictionary when no home directory is available, and bounds-check every
personal-dictionary pathname.

--- tree.c.orig
+++ tree.c
@@ -94,6 +94,14 @@
 
 #include <ctype.h>
 #include <errno.h>
+#include <signal.h>
+#include <sys/stat.h>
+#include <unistd.h>
+#ifdef __VMS
+#include <fcntl.h>
+#include <nam.h>
+#include <unixio.h>
+#endif
 #include "config.h"
 #include "ispell.h"
 #include "proto.h"
@@ -102,14 +110,21 @@
 void		treeinit P ((char * p, char * LibDict));
 static FILE *	trydict P ((char * dictname, char * home,
 		  char * prefix, char * suffix));
-static void	treeload P ((FILE * dictf));
-void		treeinsert P ((unsigned char * word, int wordlen, int keep));
+static int	make_dict_path P ((char * filename, char * home,
+		  char * prefix, char * suffix));
+#ifndef __VMS
+static int	make_temp_dict_path P ((char * filename, size_t filename_size,
+		  char * target));
+#endif
+static int	treeload P ((FILE * dictf));
+static int	load_dict P ((FILE * dictf, char * dictname));
+int		treeinsert P ((unsigned char * word, int wordlen, int keep));
 static struct dent * tinsert P ((struct dent * proto));
 struct dent *	treelookup P ((ichar_t * word));
 #if SORTPERSONAL != 0
 static int	pdictcmp P ((struct dent ** enta, struct dent **entb));
 #endif /* SORTPERSONAL != 0 */
-void		treeoutput P ((void));
+int		treeoutput P ((void));
 VOID *		mymalloc P ((unsigned int size));
 void		myfree P ((VOID * ptr));
 #ifdef REGEX_LOOKUP
@@ -133,8 +148,31 @@
     };
 
 static char		personaldict[MAXPATHLEN];
+#ifdef __VMS
+static char		personalexact[NAML$C_MAXRSS + 1];
+static struct stat	personalstat;
+static int		personalstat_valid = 0;
+#endif
 static FILE *		dictf;
 static int		newwords = 0;
+static int		personal_load_error = 0;
+
+#ifndef __VMS
+static int make_temp_dict_path (filename, filename_size, target)
+    char *		filename;
+    size_t		filename_size;
+    char *		target;
+    {
+    static char		tempsuffix[] = ".tmpXXXXXX";
+    size_t		base_length;
+    base_length = strlen (target);
+    if (base_length + sizeof tempsuffix > filename_size)
+	return 0;
+    (void) memcpy (filename, target, base_length);
+    (void) memcpy (filename + base_length, tempsuffix, sizeof tempsuffix);
+    return 1;
+    }
+#endif
 
 void treeinit (p, LibDict)
     char *		p;		/* Value specified in -p switch */
@@ -145,6 +183,14 @@
     char		seconddict[MAXPATHLEN]; /* Name of secondary dict */
     FILE *		secondf;	/* Access to second dict file */
 
+    seconddict[0] = '\0';
+    secondf = NULL;
+    personal_load_error = 0;
+#ifdef __VMS
+    personalexact[0] = '\0';
+    personalstat_valid = 0;
+#endif
+
     /*
     ** If -p was not specified, try to get a default name from the
     ** environment.  After this point, if p is null, the the value in
@@ -154,16 +200,27 @@
     */
     if (p == NULL)
 	p = getenv (PDICTVAR);
+    if (p != NULL  &&  *p == '\0')
+	p = NULL;
+    if (p != NULL  &&  strlen (p) >= sizeof personaldict)
+	{
+	(void) fprintf (stderr, TREE_C_PATH_TOO_LONG, MAYBE_CR (stderr));
+	return;
+	}
     /*
     ** Figure out the user's home.  If HOME is unset (which can happen
     ** if ispell is invoked unusually, such as from a daemon), we
     ** won't look for any HOME personal dictionaries.  Exception:  If
     ** PDICTHOME is set, we use it as a default for HOME.
     */
-    if ((h = getenv (HOME)) == NULL)
+    h = getenv (HOME);
+    if (h == NULL  ||  *h == '\0')
 	{
+	h = NULL;
 #ifdef PDICTHOME
 	h = PDICTHOME;
+	if (*h == '\0')
+	    h = NULL;
 #endif /* PDICTHOME */
 	}
 
@@ -215,19 +272,33 @@
 	    {
 	    if (seconddict[0] != '\0')
 		(void) strcpy (personaldict, seconddict);
+	    else if (h != NULL)
+		{
+		if (!make_dict_path (personaldict, h, DEFPDICT, LibDict))
+		    {
+		    (void) fprintf (stderr, TREE_C_PATH_TOO_LONG,
+		      MAYBE_CR (stderr));
+		    return;
+		    }
+		}
 	    else
-		(void) sprintf (personaldict, "%s/%s%s", h == NULL ? "" : h,
-		  DEFPDICT, LibDict);
+		{
+		if (!make_dict_path (personaldict, (char *) NULL,
+		  DEFPDICT, LibDict))
+		    {
+		    (void) fprintf (stderr, TREE_C_PATH_TOO_LONG,
+		      MAYBE_CR (stderr));
+		    return;
+		    }
+		}
 	    }
 	if (dictf != NULL)
 	    {
-	    treeload (dictf);
-	    (void) fclose (dictf);
+	    (void) load_dict (dictf, personaldict);
 	    }
 	if (secondf != NULL)
 	    {
-	    treeload (secondf);
-	    (void) fclose (secondf);
+	    (void) load_dict (secondf, seconddict);
 	    }
 	}
     else
@@ -239,6 +310,9 @@
 	*/
 	abspath = IS_SLASH (*p)  ||  strncmp (p, "./", 2) == 0
 	  ||  strncmp (p, "../", 3) == 0;
+#ifdef __VMS
+	abspath |= vms_find_last_unescaped (p, ":[<") != NULL;
+#endif
 #ifdef MSDOS
 	/*
 	** DTRT with drive-letter braindamage and with backslashes.
@@ -252,8 +326,7 @@
 	    (void) strcpy (personaldict, p);
 	    if ((dictf = fopen (personaldict, "r")) != NULL)
 		{
-		treeload (dictf);
-		(void) fclose (dictf);
+		(void) load_dict (dictf, personaldict);
 		}
 	    }
 	else
@@ -269,17 +342,20 @@
 	    (void) strcpy (personaldict, p);
 	    if ((dictf = fopen (personaldict, "r")) != NULL)
 		{
-		treeload (dictf);
-		(void) fclose (dictf);
+		(void) load_dict (dictf, personaldict);
 		}
 	    else if (!abspath  &&  h != NULL)
 		{
 		/* Try the home */
-		(void) sprintf (personaldict, "%s/%s", h, p);
+		if (!make_dict_path (personaldict, h, "", p))
+		    {
+		    (void) fprintf (stderr, TREE_C_PATH_TOO_LONG,
+		      MAYBE_CR (stderr));
+		    return;
+		    }
 		if ((dictf = fopen (personaldict, "r")) != NULL)
 		    {
-		    treeload (dictf);
-		    (void) fclose (dictf);
+		    (void) load_dict (dictf, personaldict);
 		    }
 		}
 	    /*
@@ -309,35 +385,116 @@
  * name in "filename" if one is found, and leaves a null string there
  * otherwise.
  */
-static FILE * trydict (filename, home, prefix, suffix)
+static int make_dict_path (filename, home, prefix, suffix)
     char *		filename;	/* Where to store the file name */
-    char *		home;		/* Home directory */
+    char *		home;		/* Home directory, or NULL */
     char *		prefix;		/* Prefix for dictionary */
     char *		suffix;		/* Suffix for dictionary */
     {
-    FILE *		dictf;		/* Access to dictionary file */
+    size_t		length;
+    size_t		home_length = 0;
+    int			need_separator = 0;
 
+    length = strlen (prefix) + strlen (suffix) + 1;
+    if (home != NULL)
+	{
+	home_length = strlen (home);
+	need_separator = home_length != 0
+	  &&  !IS_SLASH (home[home_length - 1]);
+#ifdef __VMS
+	if (need_separator
+	  &&  vms_path_separator (home) == home + home_length - 1)
+	    need_separator = 0;
+#endif
+	length += home_length + need_separator;
+	}
+    if (length > MAXPATHLEN)
+	{
+	filename[0] = '\0';
+	errno = ENAMETOOLONG;
+	return 0;
+	}
     if (home == NULL)
 	(void) sprintf (filename, "%s%s", prefix, suffix);
     else
-	(void) sprintf (filename, "%s/%s%s", home, prefix, suffix);
+	(void) sprintf (filename, "%s%s%s%s", home,
+	  need_separator ? "/" : "", prefix, suffix);
+    return 1;
+    }
+
+static FILE * trydict (filename, home, prefix, suffix)
+    char *		filename;	/* Where to store the file name */
+    char *		home;		/* Home directory */
+    char *		prefix;		/* Prefix for dictionary */
+    char *		suffix;		/* Suffix for dictionary */
+    {
+    FILE *		dictf;		/* Access to dictionary file */
+
+    if (!make_dict_path (filename, home, prefix, suffix))
+	return NULL;
     dictf = fopen (filename, "r");
     if (dictf == NULL)
 	filename[0] = '\0';
     return dictf;
     }
 
-static void treeload (loadfile)
+static int treeload (loadfile)
     register FILE *	loadfile;	/* File to load words from */
     {
     char		buf[BUFSIZ];	/* Buffer for reading pers dict */
+    int			c;
+    size_t		length;
+    int			status = 0;
 
     while (fgets (buf, sizeof buf, loadfile) != NULL)
-	treeinsert ((unsigned char *) buf, sizeof buf, 1);
+	{
+	length = strlen (buf);
+	if (length == sizeof buf - 1  &&  buf[length - 1] != '\n')
+	    {
+	    while ((c = getc (loadfile)) != EOF  &&  c != '\n')
+		;
+	    status = -1;
+	    continue;
+	    }
+	if (treeinsert ((unsigned char *) buf, sizeof buf, 1) != 0)
+	    status = -1;
+	}
     newwords = 0;
+    return ferror (loadfile) ? -1 : status;
+    }
+
+static int load_dict (loadfile, dictname)
+    FILE *		loadfile;
+    char *		dictname;
+    {
+    int			status = 0;
+
+#ifdef __VMS
+    if (strcmp (dictname, personaldict) == 0
+      &&  (fgetname (loadfile, personalexact, 1) == NULL
+        ||  fstat (fileno (loadfile), &personalstat) != 0))
+	{
+	personalexact[0] = '\0';
+	personalstat_valid = 0;
+	status = -1;
+	}
+    else if (strcmp (dictname, personaldict) == 0)
+	personalstat_valid = 1;
+#endif
+    if (treeload (loadfile) != 0)
+	status = -1;
+    if (fclose (loadfile) != 0)
+	status = -1;
+    if (status != 0)
+	{
+	personal_load_error = 1;
+	(void) fprintf (stderr, TREE_C_CANT_READ, dictname,
+	  MAYBE_CR (stderr));
+	}
+    return status;
     }
 
-void treeinsert (word, wordlen, keep)
+int treeinsert (word, wordlen, keep)
     unsigned char *	word;	/* Word to insert - must be canonical */
     int			wordlen; /* Length of the word buffer */
     int			keep;
@@ -433,7 +590,7 @@
     ** entry for the word.
     */
     if (makedent (word, wordlen, &wordent) < 0)
-	return;			/* Word must be too big or something */
+	return -1;		/* Word must be too big or something */
     if (keep)
 	wordent.flagfield |= KEEP;
     /*
@@ -448,7 +605,7 @@
 	if (combinecaps (dp, &wordent) < 0)
 	    {
 	    free (wordent.word);
-	    return;
+	    return -1;
 	    }
 	}
     else
@@ -456,9 +613,11 @@
 	/* It's new. Insert the word. */
 	dp = tinsert (&wordent);
 	if (captype (dp->flagfield) == FOLLOWCASE)
-	   (void) addvheader (dp);
+	   if (addvheader (dp) != 0)
+	       return -1;
 	}
     newwords |= keep;
+    return 0;
     }
 
 static struct dent * tinsert (proto)
@@ -537,7 +696,73 @@
     }
 #endif
 
-void treeoutput ()
+#ifdef __VMS
+static int vms_restore_signal_mask (old_signals)
+    sigset_t *		old_signals;
+    {
+    if (sigprocmask (SIG_SETMASK, old_signals, (sigset_t *) NULL) == 0)
+	return 0;
+    (void) fprintf (stderr, ISPELL_C_SIGNAL_RESTORE_FAILED,
+	MAYBE_CR (stderr));
+    return -1;
+    }
+
+static int vms_refresh_personal (target, expected)
+    char *		target;
+    struct stat *	expected;
+    {
+    char		exact[NAML$C_MAXRSS + 1];
+    FILE *		refresh;
+    struct stat	actual;
+    struct stat	linkstat;
+
+    personalstat_valid = 0;
+    refresh = fopen (target, "r");
+    if (refresh == NULL)
+	return 0;
+    if ((fgetname (refresh, exact, 1) == NULL
+        &&  getname (fileno (refresh), exact, 1) == NULL)
+      ||  fstat (fileno (refresh), &actual) != 0
+      ||  lstat (exact, &linkstat) != 0
+      ||  S_ISLNK (linkstat.st_mode)
+      ||  !vms_path_matches (exact, expected)
+      ||  !vms_path_matches (exact, &actual))
+	{
+	(void) fclose (refresh);
+	return 0;
+	}
+    if (fclose (refresh) != 0)
+	return 0;
+    (void) strcpy (personalexact, exact);
+    personalstat = actual;
+    personalstat_valid = 1;
+    return 1;
+    }
+
+static int vms_restore_personal (original, target, recovery, expected,
+  replacement, replacement_expected, previous_current)
+    char *		original;
+    char *		target;
+    char *		recovery;
+    struct stat *	expected;
+    char *		replacement;
+    struct stat *	replacement_expected;
+    struct stat *	previous_current;
+    {
+    if (!vms_restore_recovery (original, target, recovery, expected,
+	  replacement, replacement_expected, previous_current))
+	return 0;
+    if (!vms_refresh_personal (
+	  vms_path_matches (original, expected) ? original : target, expected))
+	{
+	(void) fprintf (stderr, TREE_C_REFRESH_FAILED, MAYBE_CR (stderr));
+	return -1;
+	}
+    return 1;
+    }
+#endif
+
+int treeoutput ()
     {
     register struct dent *	cent;	/* Current entry */
     register struct dent *	lent;	/* Linked entry */
@@ -547,15 +772,299 @@
     register struct dent **	sortptr; /* Handy pointer into sortlist */
 #endif
     register struct dent *	ehtab;	/* End of pershtab, for fast looping */
+#ifdef __VMS
+    char			dicttarget[NAML$C_MAXRSS + 1];
+    char			createddict[NAML$C_MAXRSS + 1];
+    struct stat		createdstat;
+    int			createdstat_valid;
+    int			dictfd;
+    int			first_create_partial;
+    struct stat		linkstat;
+    struct stat		previous_current;
+    char			recoverydict[NAML$C_MAXRSS + 1];
+    int			recovery_created;
+    char *		version;
+    sigset_t		blocked_signals;
+    sigset_t		old_signals;
+#else
+    char			dicttarget[MAXPATHLEN];
+    char			tempdict[MAXPATHLEN];
+    struct stat		dictstat;
+    struct stat		linkstat;
+    int			have_stat;
+    int			tempfd;
+#endif
+    int			write_error;
 
     if (newwords == 0)
-	return;
-
-    if ((dictf = fopen (personaldict, "w")) == NULL)
+	return 0;
+    if (personal_load_error)
+	return -1;
+
+#ifdef __VMS
+    recovery_created = 0;
+    createdstat_valid = 0;
+    createddict[0] = '\0';
+    dictfd = -1;
+    first_create_partial = 0;
+    if (personalexact[0] != '\0')
 	{
-	(void) fprintf (stderr, CANT_CREATE, personaldict, MAYBE_CR (stderr));
-	return;
+	if (strlen (personalexact) >= sizeof dicttarget)
+	    {
+	    (void) fprintf (stderr, TREE_C_PATH_TOO_LONG, MAYBE_CR (stderr));
+	    return -1;
+	    }
+	(void) strcpy (dicttarget, personalexact);
+	}
+    else if (strlen (personaldict) >= sizeof dicttarget)
+	{
+	(void) fprintf (stderr, TREE_C_PATH_TOO_LONG, MAYBE_CR (stderr));
+	return -1;
+	}
+    else
+	(void) strcpy (dicttarget, personaldict);
+    version = vms_version_field (dicttarget);
+    if (version != NULL)
+	*version = '\0';
+    if (personalexact[0] != '\0'
+      &&  (!personalstat_valid
+        ||  lstat (personalexact, &linkstat) != 0
+        ||  S_ISLNK (linkstat.st_mode)
+        ||  !vms_path_matches (personalexact, &personalstat)))
+	{
+	(void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	  MAYBE_CR (stderr));
+	return -1;
+	}
+    errno = 0;
+    if (lstat (dicttarget, &linkstat) == 0)
+	{
+	if (S_ISLNK (linkstat.st_mode))
+	    {
+	    (void) fprintf (stderr, TREE_C_SYMLINK, personaldict,
+	      MAYBE_CR (stderr));
+	    return -1;
+	    }
+	if (personalexact[0] == '\0')
+	    {
+	    (void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	      MAYBE_CR (stderr));
+	    return -1;
+	    }
+	previous_current = linkstat;
+	if (!vms_path_matches (dicttarget, &previous_current))
+	    {
+	    (void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	      MAYBE_CR (stderr));
+	    return -1;
+	    }
+	}
+    else if (errno != ENOENT  ||  personalexact[0] != '\0')
+	{
+	(void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	  MAYBE_CR (stderr));
+	return -1;
+	}
+    (void) sigemptyset (&blocked_signals);
+    (void) sigaddset (&blocked_signals, SIGINT);
+    (void) sigaddset (&blocked_signals, SIGTERM);
+#ifdef SIGHUP
+    (void) sigaddset (&blocked_signals, SIGHUP);
+#endif
+#ifdef SIGQUIT
+    (void) sigaddset (&blocked_signals, SIGQUIT);
+#endif
+#ifdef SIGTSTP
+    (void) sigaddset (&blocked_signals, SIGTSTP);
+#endif
+#ifdef SIGTTIN
+    (void) sigaddset (&blocked_signals, SIGTTIN);
+#endif
+#ifdef SIGTTOU
+    (void) sigaddset (&blocked_signals, SIGTTOU);
+#endif
+    if (sigprocmask (SIG_BLOCK, &blocked_signals, &old_signals) != 0)
+	{
+	(void) fprintf (stderr, ISPELL_C_SIGNAL_BLOCK_FAILED,
+	  MAYBE_CR (stderr));
+	return -1;
+	}
+    if (personalexact[0] != '\0')
+	{
+	if (!personalstat_valid)
+	    {
+	    (void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	      MAYBE_CR (stderr));
+	    (void) vms_restore_signal_mask (&old_signals);
+	    return -1;
+	    }
+	if (!vms_recovery_link (personalexact, recoverydict,
+	      sizeof recoverydict, &personalstat))
+	    {
+	    (void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	      MAYBE_CR (stderr));
+	    (void) vms_restore_signal_mask (&old_signals);
+	    return -1;
+	    }
+	recovery_created = 1;
+	}
+    if (personalexact[0] == '\0')
+	{
+	dictfd = open (dicttarget, O_WRONLY | O_CREAT | O_EXCL, 0600);
+	if (dictfd < 0)
+	    dictf = NULL;
+	else
+	    {
+	    dictf = fdopen (dictfd, "w");
+	    if (dictf == NULL)
+		{
+		if ((getname (dictfd, createddict, 1) == NULL)
+		  ||  fstat (dictfd, &createdstat) != 0)
+		    first_create_partial = 1;
+		(void) close (dictfd);
+		dictfd = -1;
+		if (!first_create_partial
+		  &&  !vms_unlink_if_matches (createddict, &createdstat))
+		    first_create_partial = 1;
+		}
+	    }
+	}
+    else
+	dictf = fopen (dicttarget, "w");
+    if (dictf == NULL)
+	{
+	(void) fprintf (stderr, CANT_CREATE, dicttarget, MAYBE_CR (stderr));
+	if (first_create_partial)
+	    (void) fprintf (stderr, TREE_C_PARTIAL_LEFT,
+	      createddict[0] == '\0' ? dicttarget : createddict,
+	      MAYBE_CR (stderr));
+	if (recovery_created
+	  &&  !vms_restore_personal (personalexact, dicttarget,
+	    recoverydict, &personalstat, (char *) NULL,
+	    (struct stat *) NULL, &previous_current))
+	    (void) fprintf (stderr, TREE_C_RECOVERY_LEFT, recoverydict,
+	      MAYBE_CR (stderr));
+	(void) vms_restore_signal_mask (&old_signals);
+	return -1;
+	}
+    if (fgetname (dictf, createddict, 1) == NULL
+      &&  getname (fileno (dictf), createddict, 1) == NULL)
+	{
+	(void) fclose (dictf);
+	dictf = NULL;
+	if (recovery_created)
+	    (void) fprintf (stderr, TREE_C_RECOVERY_LEFT, recoverydict,
+	      MAYBE_CR (stderr));
+	else
+	    (void) fprintf (stderr, TREE_C_PARTIAL_LEFT, dicttarget,
+	      MAYBE_CR (stderr));
+	(void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	  MAYBE_CR (stderr));
+	(void) vms_restore_signal_mask (&old_signals);
+	return -1;
+	}
+    if (fstat (fileno (dictf), &createdstat) != 0)
+	{
+	(void) fclose (dictf);
+	dictf = NULL;
+	if (recovery_created)
+	    (void) fprintf (stderr, TREE_C_RECOVERY_LEFT, recoverydict,
+	      MAYBE_CR (stderr));
+	else
+	    (void) fprintf (stderr, TREE_C_PARTIAL_LEFT, createddict,
+	      MAYBE_CR (stderr));
+	(void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	  MAYBE_CR (stderr));
+	(void) vms_restore_signal_mask (&old_signals);
+	return -1;
+	}
+    createdstat_valid = 1;
+    if (personalexact[0] != '\0'
+      &&  (strcmp (createddict, personalexact) == 0
+        ||  vms_path_matches (personalexact, &createdstat)))
+	{
+	(void) fclose (dictf);
+	dictf = NULL;
+	if (recovery_created)
+	    (void) fprintf (stderr, TREE_C_RECOVERY_LEFT, recoverydict,
+	      MAYBE_CR (stderr));
+	(void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	  MAYBE_CR (stderr));
+	(void) vms_restore_signal_mask (&old_signals);
+	return -1;
 	}
+    if (lstat (createddict, &linkstat) != 0)
+	{
+	(void) fclose (dictf);
+	dictf = NULL;
+	if (recovery_created)
+	    (void) fprintf (stderr, TREE_C_RECOVERY_LEFT, recoverydict,
+	      MAYBE_CR (stderr));
+	else
+	    (void) fprintf (stderr, TREE_C_PARTIAL_LEFT, createddict,
+	      MAYBE_CR (stderr));
+	(void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	  MAYBE_CR (stderr));
+	(void) vms_restore_signal_mask (&old_signals);
+	return -1;
+	}
+    if (S_ISLNK (linkstat.st_mode)
+      ||  !vms_path_matches (createddict, &createdstat))
+	{
+	(void) fclose (dictf);
+	dictf = NULL;
+	if (S_ISLNK (linkstat.st_mode))
+	    (void) fprintf (stderr, TREE_C_SYMLINK, personaldict,
+	      MAYBE_CR (stderr));
+	if (recovery_created)
+	    (void) fprintf (stderr, TREE_C_RECOVERY_LEFT, recoverydict,
+	      MAYBE_CR (stderr));
+	else if (!S_ISLNK (linkstat.st_mode))
+	    (void) fprintf (stderr, TREE_C_PARTIAL_LEFT, createddict,
+	      MAYBE_CR (stderr));
+	(void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	  MAYBE_CR (stderr));
+	(void) vms_restore_signal_mask (&old_signals);
+	return -1;
+	}
+#else
+    have_stat = stat (personaldict, &dictstat) == 0;
+    if (realpath (personaldict, dicttarget) == NULL)
+	{
+#ifdef S_ISLNK
+	if (lstat (personaldict, &linkstat) == 0
+	  &&  S_ISLNK (linkstat.st_mode))
+	    {
+	    (void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	      MAYBE_CR (stderr));
+	    return -1;
+	    }
+#endif
+	(void) strcpy (dicttarget, personaldict);
+	}
+    if (have_stat  &&  dictstat.st_nlink > 1)
+	{
+	(void) fprintf (stderr, TREE_C_MULTILINK, personaldict,
+	  MAYBE_CR (stderr));
+	return -1;
+	}
+    if (!make_temp_dict_path (tempdict, sizeof tempdict, dicttarget))
+	{
+	(void) fprintf (stderr, TREE_C_PATH_TOO_LONG, MAYBE_CR (stderr));
+	return -1;
+	}
+    tempfd = mkstemp (tempdict);
+    if (tempfd < 0  ||  (dictf = fdopen (tempfd, "w")) == NULL)
+	{
+	if (tempfd >= 0)
+	    {
+	    (void) close (tempfd);
+	    (void) unlink (tempdict);
+	    }
+	(void) fprintf (stderr, CANT_CREATE, tempdict, MAYBE_CR (stderr));
+	return -1;
+	}
+#endif
 
 #if SORTPERSONAL != 0
     /*
@@ -628,7 +1137,7 @@
 		}
 	    }
 #if SORTPERSONAL != 0
-	return;
+	goto output_done;
 	}
     /*
     ** Produce dictionary in sorted order.  We used to do this
@@ -669,9 +1178,85 @@
     free ((char *) sortlist);
 #endif
 
+output_done:
+    write_error = ferror (dictf);
+    if (fflush (dictf) != 0)
+	write_error = 1;
+#ifndef __VMS
+    if (!write_error  &&  have_stat
+      &&  fchmod (fileno (dictf), dictstat.st_mode & 07777) != 0)
+	write_error = 1;
+#endif
+    if (fclose (dictf) != 0)
+	write_error = 1;
+    dictf = NULL;
+    if (write_error)
+	{
+#ifdef __VMS
+	if (recovery_created
+	  &&  (!createdstat_valid
+	    ||  !vms_restore_personal (personalexact, dicttarget,
+	      recoverydict, &personalstat, createddict, &createdstat,
+	      &previous_current)))
+	    (void) fprintf (stderr, TREE_C_RECOVERY_LEFT, recoverydict,
+	      MAYBE_CR (stderr));
+	else if (!recovery_created
+	  &&  (!createdstat_valid
+	    ||  !vms_unlink_if_matches (createddict, &createdstat)))
+	    (void) fprintf (stderr, TREE_C_PARTIAL_LEFT, createddict,
+	      MAYBE_CR (stderr));
+#else
+	(void) unlink (tempdict);
+#endif
+	(void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	  MAYBE_CR (stderr));
+#ifdef __VMS
+	(void) vms_restore_signal_mask (&old_signals);
+#endif
+	return -1;
+	}
+#ifdef __VMS
+    if (!vms_path_matches (createddict, &createdstat)
+      ||  !vms_path_matches (dicttarget, &createdstat))
+	{
+	if (recovery_created
+	  &&  !vms_restore_personal (personalexact, dicttarget,
+	    recoverydict, &personalstat, createddict, &createdstat,
+	    &previous_current))
+	    (void) fprintf (stderr, TREE_C_RECOVERY_LEFT, recoverydict,
+	      MAYBE_CR (stderr));
+	else if (!recovery_created
+	  &&  !vms_unlink_if_matches (createddict, &createdstat))
+	    (void) fprintf (stderr, TREE_C_PARTIAL_LEFT, createddict,
+	      MAYBE_CR (stderr));
+	(void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	  MAYBE_CR (stderr));
+	(void) vms_restore_signal_mask (&old_signals);
+	return -1;
+	}
+    if (recovery_created
+      &&  !vms_recovery_discard (recoverydict, &personalstat))
+	(void) fprintf (stderr, TREE_C_RECOVERY_LEFT, recoverydict,
+	  MAYBE_CR (stderr));
+    (void) strcpy (personalexact, createddict);
+    personalstat = createdstat;
+    personalstat_valid = createdstat_valid;
+#endif
+#ifndef __VMS
+    if (rename (tempdict, dicttarget) != 0)
+	{
+	(void) unlink (tempdict);
+	(void) fprintf (stderr, TREE_C_CANT_UPDATE, personaldict,
+	  MAYBE_CR (stderr));
+	return -1;
+	}
+#endif
     newwords = 0;
-
-    (void) fclose (dictf);
+#ifdef __VMS
+    if (vms_restore_signal_mask (&old_signals) != 0)
+	return -1;
+#endif
+    return 0;
     }
 
 VOID * mymalloc (size)
