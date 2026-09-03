$NetBSD$

Avoid readonly, which is a reserved type qualifier in VSI C.

Close external deformatter pipes with pclose(3), release the unfiltered input
and duplicated descriptors on every path, reject failed filters, and never
install edits made from incomplete filtered data.

Decode native OpenVMS and C RTL POSIX completion statuses without treating an
odd encoded nonzero POSIX exit as success.  Flush replacement output before
close, and update versioned files transactionally with file-identity checks.

Treat empty temporary-directory variables like unset ones, bounds-check the
temporary pathname, and only unlink a temporary file owned by this process.
Do not probe a root-relative log directory when HOME is absent.

Limit OpenVMS stdout buffering to a mailbox-safe write size.  GNV stream
pipes reject the traditional BUFSIZ-sized flush and would otherwise lose
output at the end of large list and crunch operations.

--- ispell.c.orig
+++ ispell.c
@@ -5,6 +5,14 @@
 
 #define MAIN
 
+/* Ask pclose() for the native condition value so successful DCL filters can
+ * be distinguished from encoded POSIX child exits on OpenVMS. */
+#ifdef __VMS
+#ifndef _VMS_WAIT
+#define _VMS_WAIT 1
+#endif
+#endif
+
 /*
  * ispell.c - An interactive spelling corrector.
  *
@@ -244,18 +252,189 @@
 #include <fcntl.h>
 #endif /* NO_FCNTL_H */
 #include <sys/stat.h>
+#include <signal.h>
+#ifdef __VMS
+#include <nam.h>
+#include <unixio.h>
+#endif
 
 static void	usage P ((void));
 int		main P ((int argc, char * argv[]));
 static void	dofile P ((char * filename));
 static FILE *	setupdefmt P ((char * filename, struct stat * statbuf));
-static void	update_file P ((char * filename, struct stat * statbuf));
+static int	closedefmt P ((void));
+static int	update_file P ((char * filename, struct stat * statbuf,
+		  char * originalfile));
+#ifdef __VMS
+static int	update_file_vms P ((char * filename, struct stat * statbuf,
+		  char * originalfile));
+#endif
+static int	restore_backup P ((char * filename, char * bakfile,
+		  int replacement_created));
+static int	make_hash_path P ((char * result, size_t result_size,
+		  char * directory, char * name));
+static int	append_hash_suffix P ((char * result, size_t result_size));
+static char *	find_hash_suffix P ((char * name));
+static int	filename_suffix_equal P ((char * name, char * suffix));
+static int	filename_numeric_suffix P ((char * name));
 static void	expandmode P ((int printorig));
 char *		last_slash P ((char * file));
 
 static char *	Cmd;
 static char *	LibDict = NULL;		/* Pointer to name of $(LIBDIR)/dict */
 
+static int make_hash_path (result, result_size, directory, name)
+    char *		result;
+    size_t		result_size;
+    char *		directory;
+    char *		name;
+    {
+    size_t		length;
+    size_t		used = 0;
+
+    if (directory != NULL  &&  *directory != '\0')
+	{
+	length = strlen (directory);
+	if (length >= result_size)
+	    return 0;
+	(void) memcpy (result, directory, length);
+	used = length;
+	if (
+#ifdef __VMS
+	  vms_path_separator (directory) != directory + length - 1
+#else
+	  result[used - 1] != '/'
+#endif
+	  )
+	    {
+	    if (used + 1 >= result_size)
+		return 0;
+	    result[used++] = '/';
+	    }
+	}
+    length = strlen (name);
+    if (length >= result_size - used)
+	return 0;
+    (void) memcpy (result + used, name, length + 1);
+    return 1;
+    }
+
+static int append_hash_suffix (result, result_size)
+    char *		result;
+    size_t		result_size;
+    {
+    size_t		length;
+    size_t		suffix_length = strlen (HASHSUFFIX);
+#ifdef __VMS
+    char *		version = vms_version_field (result);
+    size_t		version_length = version == NULL ? 0 : strlen (version);
+
+    length = version == NULL ? strlen (result) : (size_t) (version - result);
+    if (length + suffix_length + version_length >= result_size)
+	return 0;
+    if (version != NULL)
+	(void) memmove (result + length + suffix_length, version,
+	  version_length + 1);
+    else
+	result[length + suffix_length] = '\0';
+    (void) memcpy (result + length, HASHSUFFIX, suffix_length);
+#else
+
+    length = strlen (result);
+
+    if (suffix_length >= result_size - length)
+	return 0;
+    (void) memcpy (result + length, HASHSUFFIX, suffix_length + 1);
+#endif
+    return 1;
+    }
+
+static char * find_hash_suffix (name)
+    char *		name;
+    {
+    size_t		name_length = strlen (name);
+    size_t		suffix_length = strlen (HASHSUFFIX);
+    char *		suffix;
+#ifdef __VMS
+    char *		version = vms_version_field (name);
+    char *		dot;
+    size_t		i;
+
+    if (version != NULL)
+	name_length = (size_t) (version - name);
+#endif
+    if (name_length < suffix_length)
+	return NULL;
+    suffix = name + name_length - suffix_length;
+#ifdef __VMS
+    dot = vms_find_last_unescaped (name, ".");
+    if (HASHSUFFIX[0] == '.'  &&  dot != suffix)
+	return NULL;
+    for (i = 0;  i < suffix_length;  i++)
+	if (tolower ((unsigned char) suffix[i])
+	  != tolower ((unsigned char) HASHSUFFIX[i]))
+	    return NULL;
+    return suffix;
+#else
+    return strcmp (suffix, HASHSUFFIX) == 0 ? suffix : (char *) NULL;
+#endif
+    }
+
+static int filename_suffix_equal (name, suffix)
+    char *		name;
+    char *		suffix;
+    {
+    size_t		name_length = strlen (name);
+    size_t		suffix_length = strlen (suffix);
+    char *		tail;
+#ifdef __VMS
+    char *		version = vms_version_field (name);
+    char *		dot;
+    size_t		i;
+
+    if (version != NULL)
+	name_length = (size_t) (version - name);
+#endif
+    if (name_length < suffix_length)
+	return 0;
+    tail = name + name_length - suffix_length;
+#ifdef __VMS
+    dot = vms_find_last_unescaped (name, ".");
+    if (suffix[0] == '.'  &&  dot != tail)
+	return 0;
+    for (i = 0;  i < suffix_length;  i++)
+	if (tolower ((unsigned char) tail[i])
+	  != tolower ((unsigned char) suffix[i]))
+	    return 0;
+    return 1;
+#else
+    return strcmp (tail, suffix) == 0;
+#endif
+    }
+
+static int filename_numeric_suffix (name)
+    char *		name;
+    {
+    size_t		name_length = strlen (name);
+    char *		dot;
+    char *		separator;
+#ifdef __VMS
+    char *		version = vms_version_field (name);
+
+    if (version != NULL)
+	name_length = (size_t) (version - name);
+#endif
+#ifdef __VMS
+    dot = vms_find_last_unescaped (name, ".");
+#else
+    dot = rindex (name, '.');
+#endif
+    separator = last_slash (name);
+    return dot != NULL  &&  (separator == NULL  ||  dot > separator)
+	&&  dot + 2 == name + name_length
+	&&  isdigit ((unsigned char) dot[1]);
+    }
+
 static void usage ()
     {
 
@@ -281,48 +460,66 @@
     char **	versionp;
     char *	wchars = NULL;
     char *	preftype = NULL;
-    static char	libdictname[sizeof DEFHASH];
+    static char	libdictname[MAXPATHLEN];
     char	logfilename[MAXPATHLEN];
     static char	outbuf[BUFSIZ];
     int		argno;
     int		arglen;
     int		i;
+    char *	suffixp;
 
     Cmd = *argv;
 
     Trynum = 0;
 
     p = getenv (LIBRARYVAR);
-    if (p == NULL)
-	(void) strcpy (libdir, LIBDIR);
-    else
+    if (p == NULL  ||  *p == '\0')
+	p = LIBDIR;
+    if (strlen (p) >= sizeof libdir)
 	{
-	(void) strncpy (libdir, p, sizeof libdir);
-	libdir[sizeof libdir - 1] = '\0';
+	(void) fprintf (stderr, ISPELL_C_PATH_TOO_LONG, p, MAYBE_CR (stderr));
+	return 1;
 	}
+    (void) strcpy (libdir, p);
 
     p = getenv (DICTIONARYVAR);
-    if (p != NULL)
+    if (p != NULL  &&  *p != '\0')
 	{
-	if (last_slash (p) != NULL)
-	    (void) strcpy (hashname, p);
-	else
-	    (void) sprintf (hashname, "%s/%s", libdir, p);
+	if (!make_hash_path (hashname, sizeof hashname,
+	      last_slash (p) == NULL ? libdir : (char *) NULL, p)
+	  ||  strlen (p) >= sizeof libdictname)
+	    {
+	    (void) fprintf (stderr, ISPELL_C_PATH_TOO_LONG, p,
+	      MAYBE_CR (stderr));
+	    return 1;
+	    }
 	(void) strcpy (libdictname, p);
-	p = rindex (p, '.');
-	if (p == NULL  ||  strcmp (p, HASHSUFFIX) != 0)
-	    (void) strcat (hashname, HASHSUFFIX);
+	suffixp = find_hash_suffix (p);
+	if (suffixp == NULL
+	  &&  !append_hash_suffix (hashname, sizeof hashname))
+	    {
+	    (void) fprintf (stderr, ISPELL_C_PATH_TOO_LONG, p,
+	      MAYBE_CR (stderr));
+	    return 1;
+	    }
 	LibDict = last_slash (libdictname);
 	if (LibDict != NULL)
 	    LibDict++;
 	else
 	    LibDict = libdictname;
-	p = rindex (LibDict, '.');
+	p = find_hash_suffix (LibDict);
 	if (p != NULL)
 	    *p = '\0';
 	}
    else
-	(void) sprintf (hashname, "%s/%s", libdir, DEFHASH);
+	{
+	if (!make_hash_path (hashname, sizeof hashname, libdir, DEFHASH))
+	    {
+	    (void) fprintf (stderr, ISPELL_C_PATH_TOO_LONG, DEFHASH,
+	      MAYBE_CR (stderr));
+	    return 1;
+	    }
+	}
 
     cpd = NULL;
 
@@ -477,7 +674,9 @@
 		    (void) printf ("\tLINT = \"%s\"\n", LINT);
 		    (void) printf ("\tLINTFLAGS = \"%s\"\n", LINTFLAGS);
 #ifndef REGEX_LOOKUP
-		    (void) printf ("\tLOOK = \"%s\"\n", LOOK);
+#ifdef LOOK
+			    (void) printf ("\tLOOK = \"%s\"\n", LOOK);
+#endif /* LOOK */
 #endif /* REGEX_LOOKUP */
 		    (void) printf ("\tLOOK_XREF = \"%s\"\n", LOOK_XREF);
 		    (void) printf ("\tMAKE_SORTTMP = \"%s\"\n", MAKE_SORTTMP);
@@ -803,7 +1002,7 @@
 		    }
 		LibDict = NULL;
 		break;
-	    case 'd':
+	case 'd':
 		p = argv[argno] + 2;
 		if (*p == '\0')
 		    {
@@ -812,22 +1011,45 @@
 			usage ();
 		    p = argv[argno];
 		    }
-		if (last_slash (p) != NULL)
-		    (void) strcpy (hashname, p);
-		else
-		    (void) sprintf (hashname, "%s/%s", libdir, p);
+		if (!make_hash_path (hashname, sizeof hashname,
+		      last_slash (p) == NULL ? libdir : (char *) NULL, p))
+		    {
+		    (void) fprintf (stderr, ISPELL_C_PATH_TOO_LONG, p,
+		      MAYBE_CR (stderr));
+		    return 1;
+		    }
 		if (cpd == NULL  &&  *p != '\0')
-		    LibDict = p;
-		p = rindex (p, '.');
-		if (p != NULL  &&  strcmp (p, HASHSUFFIX) == 0)
-		    *p = '\0';	/* Don't want ext. in LibDict */
-		else
-		    (void) strcat (hashname, HASHSUFFIX);
+		    {
+		    if (strlen (p) >= sizeof libdictname)
+			{
+			(void) fprintf (stderr, ISPELL_C_PATH_TOO_LONG, p,
+			  MAYBE_CR (stderr));
+			return 1;
+			}
+		    (void) strcpy (libdictname, p);
+		    LibDict = libdictname;
+		    }
+		suffixp = find_hash_suffix (p);
+		if (suffixp != NULL)
+		    {
+		    if (LibDict != NULL)
+			{
+			suffixp = find_hash_suffix (LibDict);
+			if (suffixp != NULL)
+			    *suffixp = '\0';
+			}
+		    }
+		else if (!append_hash_suffix (hashname, sizeof hashname))
+		    {
+		    (void) fprintf (stderr, ISPELL_C_PATH_TOO_LONG, p,
+		      MAYBE_CR (stderr));
+		    return 1;
+		    }
 		if (LibDict != NULL)
 		    {
-		    p = last_slash (LibDict);
-		    if (p != NULL)
-			LibDict = p + 1;
+		    suffixp = last_slash (LibDict);
+		    if (suffixp != NULL)
+			LibDict = suffixp + 1;
 		    }
 		break;
 	    case 'V':		/* Display 8-bit characters as M-xxx */
@@ -1001,8 +1223,8 @@
 	{
 	(void) strcpy (libdictname, DEFHASH);
 	LibDict = libdictname;
-	p = rindex (libdictname, '.');
-	if (p != NULL  &&  strcmp (p, HASHSUFFIX) == 0)
+	p = find_hash_suffix (libdictname);
+	if (p != NULL)
 	    *p = '\0';	/* Don't want ext. in LibDict */
 	}
     if (!nodictflag)
@@ -1011,8 +1233,7 @@
     if (aflag)
 	{
 	askmode ();
-	treeoutput ();
-	exit (0);
+	exit (treeoutput () != 0);
 	}
     else if (eflag)
 	{
@@ -1026,13 +1247,48 @@
 	}
 
 #ifndef __bsdi__
+#ifdef __VMS
+    /* GNV stream-pipe mailboxes reject BUFSIZ-sized writes. */
+    if (setvbuf (stdout, outbuf, _IOFBF, 512) != 0)
+	(void) setvbuf (stdout, (char *) NULL, _IONBF, 0);
+#else
     setbuf (stdout, outbuf);
+#endif
 #endif /* __bsdi__ */
     if (lflag)
 	{
+	int input_error;
+	int filter_status;
+	int output_error;
+
 	infile = setupdefmt(NULL, NULL);
+	if (infile == NULL)
+	    {
+	    (void) fprintf (stderr, CANT_OPEN,
+	      defmtpgm == NULL ? "standard input" : defmtpgm,
+	      MAYBE_CR (stderr));
+	    exit (1);
+	    }
 	outfile = stdout;
 	checkfile ();
+	input_error = ferror (infile);
+	filter_status = closedefmt ();
+	output_error = ferror (stdout);
+	if (fflush (stdout) == EOF)
+	    output_error = 1;
+	if (filter_status != 0)
+	    {
+	    (void) fprintf (stderr, ISPELL_C_FILTER_FAILED, defmtpgm,
+	      MAYBE_CR (stderr));
+	    }
+	if (input_error)
+	    (void) fprintf (stderr, ISPELL_C_IO_FAILED, "standard input",
+	      MAYBE_CR (stderr));
+	if (output_error)
+	    (void) fprintf (stderr, ISPELL_C_IO_FAILED, "standard output",
+	      MAYBE_CR (stderr));
+	if (filter_status != 0  ||  input_error  ||  output_error)
+	    exit (1);
 	exit (0);
 	}
 
@@ -1040,10 +1296,16 @@
      * If there is a log directory, open a log file.  If the open
      * fails, we just won't log.
      */
-    (void) sprintf (logfilename, "%s/%s/%s",
-      getenv ("HOME") == NULL ? "" : getenv ("HOME"),
-      DEFLOGDIR, LibDict);
-    logfile = fopen (logfilename, "a");
+    p = getenv (HOME);
+    if (p != NULL  &&  *p != '\0'
+      &&  strlen (p) + strlen (DEFLOGDIR) + strlen (LibDict) + 3
+        <= sizeof logfilename)
+	{
+	(void) sprintf (logfilename, "%s/%s/%s", p, DEFLOGDIR, LibDict);
+	logfile = fopen (logfilename, "a");
+	}
+    else
+	logfile = NULL;
 
     terminit ();
 
@@ -1059,9 +1321,17 @@
     char *	filename;
     {
     struct stat	statbuf;
-    char *	cp;
-    int		outfd;			/* Used in opening temp file */
+    int		outfd = -1;		/* Used in opening temp file */
 					/* ..might produce not-used warnings */
+    int		filter_status;
+    int		input_error;
+    int		output_error;
+#ifdef __VMS
+    FILE *		authoritative;
+    struct stat		linkstat;
+    char		originalfile[NAML$C_MAXRSS + 1];
+    int			vms_symlink;
+#endif
 
     currentfile = filename;
 
@@ -1070,19 +1340,18 @@
     if (tflag < 0)
 	{
 	tflag = DEFORMAT_NONE;		/* Default to none */
-	cp = rindex (filename, '.');
-	if (cp != NULL)
-	    {
-	    if (strcmp (cp, ".ms") == 0  ||  strcmp (cp, ".mm") == 0
-	      ||  strcmp (cp, ".me") == 0  ||  strcmp (cp, ".man") == 0
-	      ||  isdigit(*cp))
-		tflag = DEFORMAT_NROFF;
-	    else if (strcmp (cp, ".tex") == 0)
-		tflag = DEFORMAT_TEX;
-	    else if (strcmp (cp, ".html") == 0  ||  strcmp (cp, ".htm") == 0
-	      ||  strcmp (cp, ".shtml") == 0)
-		tflag = DEFORMAT_SGML;
-	    }
+	if (filename_suffix_equal (filename, ".ms")
+	  ||  filename_suffix_equal (filename, ".mm")
+	  ||  filename_suffix_equal (filename, ".me")
+	  ||  filename_suffix_equal (filename, ".man")
+	  ||  filename_numeric_suffix (filename))
+	    tflag = DEFORMAT_NROFF;
+	else if (filename_suffix_equal (filename, ".tex"))
+	    tflag = DEFORMAT_TEX;
+	else if (filename_suffix_equal (filename, ".html")
+	  ||  filename_suffix_equal (filename, ".htm")
+	  ||  filename_suffix_equal (filename, ".shtml"))
+	    tflag = DEFORMAT_SGML;
 	}
     if (prefstringchar < 0)
 	{
@@ -1098,10 +1367,43 @@
 	(void) sleep ((unsigned) 2);
 	return;
 	}
+#ifdef __VMS
+    authoritative = sourcefile != NULL ? sourcefile : infile;
+    if (fstat (fileno (authoritative), &statbuf) != 0
+      ||  (fgetname (authoritative, originalfile, 1) == NULL
+        &&  getname (fileno (authoritative), originalfile, 1) == NULL)
+      ||  lstat (originalfile, &linkstat) != 0)
+	{
+	(void) fprintf (stderr, ISPELL_C_IO_FAILED, filename,
+	  MAYBE_CR (stderr));
+	(void) closedefmt ();
+	(void) sleep ((unsigned) 2);
+	return;
+	}
+    vms_symlink = S_ISLNK (linkstat.st_mode);
+    if (!vms_symlink  &&  !vms_path_matches (originalfile, &statbuf))
+	{
+	(void) fprintf (stderr, ISPELL_C_IO_FAILED, filename,
+	  MAYBE_CR (stderr));
+	(void) closedefmt ();
+	(void) sleep ((unsigned) 2);
+	return;
+	}
+#endif
 
-    readonly = access (filename, W_OK) < 0;
-    if (readonly)
+    ispell_readonly = access (filename, W_OK) < 0
+#ifdef __VMS
+      ||  vms_symlink
+#endif
+      ;
+    if (ispell_readonly)
 	{
+#ifdef __VMS
+	if (vms_symlink)
+	    (void) fprintf (stderr, ISPELL_C_SYMLINK, originalfile,
+	      MAYBE_CR (stderr));
+	else
+#endif
 	(void) fprintf (stderr, ISPELL_C_CANT_WRITE, filename,
 	  MAYBE_CR (stderr));
 	(void) sleep ((unsigned) 2);
@@ -1114,45 +1416,69 @@
      * unfortunately isn't available anywhere).  In other words, don't
      * worry about the security of this hunk of code.
      */
+    tempfile[0] = '\0';
+    tempfile_created = 0;
     if (last_slash (TEMPNAME) != NULL)
-	(void) strcpy (tempfile, TEMPNAME);
+	{
+	if (strlen (TEMPNAME) < sizeof tempfile)
+	    (void) strcpy (tempfile, TEMPNAME);
+	}
     else
 	{
 	char *tmp = getenv ("TMPDIR");
 	int   lastchar;
+	int   need_separator;
 
-	if (tmp == NULL)
+	if (tmp == NULL  ||  *tmp == '\0')
 	    tmp = getenv ("TEMP");
-	if (tmp == NULL)
+	if (tmp == NULL  ||  *tmp == '\0')
 	    tmp = getenv ("TMP");
-	if (tmp == NULL)
+	if (tmp == NULL  ||  *tmp == '\0')
 #ifdef P_tmpdir
 	    tmp = P_tmpdir;
 #else
 	    tmp = "/tmp";
 #endif
+	if (tmp == NULL  ||  *tmp == '\0')
+	    tmp = "/tmp";
 	lastchar = tmp[strlen (tmp) - 1];
-	(void) sprintf (tempfile, "%s%s%s", tmp,
-			IS_SLASH (lastchar) ? "" : "/",
-			TEMPNAME);
+	need_separator = !IS_SLASH (lastchar);
+#ifdef __VMS
+	if (vms_path_separator (tmp) == tmp + strlen (tmp) - 1)
+	    need_separator = 0;
+#endif
+	if (strlen (tmp) + strlen (TEMPNAME) + need_separator + 1
+	  <= sizeof tempfile)
+	    (void) sprintf (tempfile, "%s%s%s", tmp,
+			    need_separator ? "/" : "",
+			    TEMPNAME);
 	}
+    outfile = NULL;
 #ifdef NO_MKSTEMP
-    if (mktemp (tempfile) == NULL  ||  tempfile[0] == '\0'
+    if (tempfile[0] == '\0'  ||  mktemp (tempfile) == NULL
+      ||  tempfile[0] == '\0'
 #ifdef O_EXCL
       ||  (outfd = open (tempfile, O_WRONLY | O_CREAT | O_EXCL, 0600)) < 0
-      ||  (outfile = fdopen (outfd, "w")) == NULL)
+      ||  (tempfile_created = 1, (outfile = fdopen (outfd, "w")) == NULL))
 #else /* O_EXCL */
-      ||  (outfile = fopen (tempfile, "w")) == NULL)
+      ||  (outfile = fopen (tempfile, "w")) == NULL
+      ||  (tempfile_created = 1, 0))
 #endif /* O_EXCL */
 #else /* NO_MKSTEMP */
-    if ((outfd = mkstemp (tempfile)) < 0
-      ||  (outfile = fdopen (outfd, "w")) == NULL)
+    if (tempfile[0] == '\0'  ||  (outfd = mkstemp (tempfile)) < 0
+      ||  (tempfile_created = 1, (outfile = fdopen (outfd, "w")) == NULL))
 #endif /* NO_MKSTEMP */
 	{
 	(void) fprintf (stderr, CANT_CREATE,
-	  (tempfile == NULL  ||  tempfile[0] == '\0')
+	  tempfile[0] == '\0'
 	    ? "temporary file" : tempfile,
 	  MAYBE_CR (stderr));
+	if (outfd >= 0  &&  outfile == NULL)
+	    (void) close (outfd);
+	if (tempfile_created)
+	    (void) unlink (tempfile);
+	tempfile_created = 0;
+	closedefmt ();
 	(void) sleep ((unsigned) 2);
 	return;
 	}
@@ -1168,18 +1494,54 @@
 
     quit = 0;
     changes = 0;
+    filter_error = 0;
 
     checkfile ();
 
-    (void) fclose (infile);
-    (void) fclose (outfile);
+	/* X copies the unfiltered remainder.  Drain the filter so pclose does
+	 * not mistake its expected broken pipe for a filter failure. */
+    if (quit  &&  defmtpgm != NULL)
+	while (getc (infile) != EOF)
+	    ;
+    input_error = ferror (infile)
+      ||  (sourcefile != NULL  &&  ferror (sourcefile));
+    filter_status = closedefmt ();
+    output_error = ferror (outfile);
+    if (fflush (outfile) != 0)
+	output_error = 1;
+    if (fclose (outfile) != 0)
+	output_error = 1;
+    outfile = NULL;
+
+    if (filter_status != 0)
+	{
+	filter_error = 1;
+	(void) fprintf (stderr, ISPELL_C_FILTER_FAILED, defmtpgm,
+	  MAYBE_CR (stderr));
+	}
+    if (input_error  ||  output_error)
+	{
+	filter_error = 1;
+	(void) fprintf (stderr, ISPELL_C_IO_FAILED, filename,
+	  MAYBE_CR (stderr));
+	}
 
-    if (!cflag)
-	treeoutput ();
+    if (!filter_error  &&  !cflag  &&  treeoutput () != 0)
+	filter_error = 1;
 
-    if (changes && !readonly)
-	update_file (filename, &statbuf);
+    if (changes  &&  !ispell_readonly  &&  !filter_error)
+	if (update_file (filename, &statbuf,
+#ifdef __VMS
+	      originalfile
+#else
+	      filename
+#endif
+	      ) != 0)
+	    filter_error = 1;
     (void) unlink (tempfile);
+    tempfile_created = 0;
+    if (filter_error)
+	done (1);
     }
 
 /*
@@ -1264,49 +1626,487 @@
 	if (statbuf != NULL  &&  fstat (fileno (sourcefile), statbuf) == -1)
 	    statbuf->st_mode = DEFAULT_FILE_MODE;
 	savedstdin = dup (0);
+	if (savedstdin < 0)
+	    {
+	    (void) fclose (sourcefile);
+	    sourcefile = NULL;
+	    return NULL;
+	    }
 	inputfd = open (filename, 0);
 	if (inputfd < 0)
+	    {
+	    (void) close (savedstdin);
+	    (void) fclose (sourcefile);
+	    sourcefile = NULL;
 	    return NULL;		/* Failed to open the file */
+	    }
 	else if (dup2 (inputfd, 0) != 0)
 	    {
+	    (void) close (inputfd);
+	    (void) close (savedstdin);
+	    (void) fclose (sourcefile);
+	    sourcefile = NULL;
 	    (void) fprintf (stderr, ISPELL_C_UNEXPECTED_FD, filename,
 	      MAYBE_CR (stderr));
 	    exit (1);
 	    }
+	(void) close (inputfd);
 	filteredfile = popen (defmtpgm, "r");
 	if (dup2 (savedstdin, 0) != 0)
 	    {
+	    if (filteredfile != NULL)
+		(void) pclose (filteredfile);
+	    (void) close (savedstdin);
+	    (void) fclose (sourcefile);
+	    sourcefile = NULL;
 	    (void) fprintf (stderr, ISPELL_C_UNEXPECTED_FD, filename,
 	      MAYBE_CR (stderr));
 	    exit (1);
 	    }
-	close (savedstdin);
+	(void) close (savedstdin);
+	if (filteredfile == NULL)
+	    {
+	    (void) fclose (sourcefile);
+	    sourcefile = NULL;
+	    }
 	return filteredfile;
 	}
     }
 
-static void update_file (filename, statbuf)
+/* Close the stream returned by setupdefmt and its unfiltered companion. */
+static int closedefmt ()
+    {
+    int		status = 0;
+
+    if (sourcefile != NULL  &&  sourcefile != infile)
+	(void) fclose (sourcefile);
+    sourcefile = NULL;
+
+    if (infile != NULL)
+	{
+	if (defmtpgm != NULL)
+	    {
+	    status = pclose (infile);
+#ifdef __VMS
+	    if (status != -1)
+		{
+		unsigned int vms_status = (unsigned int) status;
+
+		/* The C RTL encodes nonzero _POSIX_EXIT values in the C
+		 * facility's A000 block.  Their severity bit is not a success
+		 * test (for example, exit 2 is odd), so decode them before
+		 * accepting ordinary odd-valued OpenVMS success conditions. */
+		if ((vms_status & 0x0ffff800U) == 0x0035a000U)
+		    {
+		    if (((vms_status & 0x7f8U) >> 3) == 0)
+			status = 0;
+		    }
+		else if ((vms_status & 1U) != 0)
+		    status = 0;
+		}
+#endif
+	    }
+	else if (infile != stdin)
+	    (void) fclose (infile);
+	}
+    infile = NULL;
+    return status;
+    }
+
+static int restore_backup (filename, bakfile, replacement_created)
+    char *		filename;
+    char *		bakfile;
+    int			replacement_created;
+    {
+    if (replacement_created)
+	(void) unlink (filename);
+#ifdef HAS_RENAME
+    if (rename (bakfile, filename) == 0)
+	return 0;
+#else /* HAS_RENAME */
+    if (link (bakfile, filename) == 0)
+	return 0;
+#endif /* HAS_RENAME */
+    (void) fprintf (stderr, ISPELL_C_RESTORE_FAILED, filename, bakfile,
+      MAYBE_CR (stderr));
+    return -1;
+    }
+
+static int restore_signal_mask (old_signals)
+    sigset_t *		old_signals;
+    {
+    if (sigprocmask (SIG_SETMASK, old_signals, (sigset_t *) NULL) == 0)
+	return 0;
+    (void) fprintf (stderr, ISPELL_C_SIGNAL_RESTORE_FAILED,
+	MAYBE_CR (stderr));
+    return -1;
+    }
+
+#ifdef __VMS
+static int update_file_vms (filename, statbuf, originalfile)
+    char *		filename;
+    struct stat *	statbuf;
+    char *		originalfile;
+    {
+    char		bakfile[NAML$C_MAXRSS + 1];
+    char		backup_target[NAML$C_MAXRSS + 1];
+    sigset_t		blocked_signals;
+    int			c;
+    int			copy_error = 0;
+    int			new_created = 0;
+    char		newfile[NAML$C_MAXRSS + 1];
+    struct stat		newstat;
+    int			newstat_valid = 0;
+    int			orphan_created = 0;
+    sigset_t		old_signals;
+    char		recoveryfile[NAML$C_MAXRSS + 1];
+    int			recovery_created = 0;
+    struct stat		previous_current;
+    char		target[NAML$C_MAXRSS + 1];
+    char *		version;
+    struct stat		linkstat;
+
+    if ((infile = fopen (tempfile, "r")) == NULL)
+	{
+	(void) fprintf (stderr, ISPELL_C_TEMP_DISAPPEARED, tempfile,
+	  MAYBE_CR (stderr));
+	(void) sleep ((unsigned) 2);
+	return -1;
+	}
+    if (strlen (originalfile) >= sizeof target)
+	{
+	(void) fprintf (stderr, ISPELL_C_PATH_TOO_LONG, filename,
+	  MAYBE_CR (stderr));
+	(void) fclose (infile);
+	infile = NULL;
+	return -1;
+	}
+    (void) strcpy (target, originalfile);
+    version = vms_version_field (target);
+    if (version != NULL)
+	*version = '\0';
+    if (strlen (target) + strlen (BAKEXT) + 2 >= sizeof bakfile)
+	{
+	(void) fprintf (stderr, ISPELL_C_PATH_TOO_LONG, filename,
+	  MAYBE_CR (stderr));
+	(void) fclose (infile);
+	infile = NULL;
+	return -1;
+	}
+    (void) strcpy (bakfile, target);
+    (void) strcat (bakfile, BAKEXT);
+    (void) strcpy (backup_target, bakfile);
+    (void) strcat (backup_target, ";0");
+
+    if (lstat (originalfile, &linkstat) != 0
+      ||  !vms_path_matches (originalfile, statbuf))
+	{
+	(void) fprintf (stderr, ISPELL_C_IO_FAILED, originalfile,
+	  MAYBE_CR (stderr));
+	(void) fclose (infile);
+	infile = NULL;
+	return -1;
+	}
+    if (S_ISLNK (linkstat.st_mode))
+	{
+	(void) fprintf (stderr, ISPELL_C_SYMLINK, originalfile,
+	  MAYBE_CR (stderr));
+	(void) fclose (infile);
+	infile = NULL;
+	return -1;
+	}
+    if (lstat (target, &linkstat) != 0)
+	{
+	(void) fprintf (stderr, ISPELL_C_IO_FAILED, target,
+	  MAYBE_CR (stderr));
+	(void) fclose (infile);
+	infile = NULL;
+	return -1;
+	}
+    if (S_ISLNK (linkstat.st_mode))
+	{
+	(void) fprintf (stderr, ISPELL_C_SYMLINK, target,
+	  MAYBE_CR (stderr));
+	(void) fclose (infile);
+	infile = NULL;
+	return -1;
+	}
+    previous_current = linkstat;
+    if (!vms_path_matches (target, &previous_current))
+	{
+	(void) fprintf (stderr, ISPELL_C_IO_FAILED, target,
+	  MAYBE_CR (stderr));
+	(void) fclose (infile);
+	infile = NULL;
+	return -1;
+	}
+
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
+	(void) fclose (infile);
+	infile = NULL;
+	return -1;
+	}
+
+    /* A recovery hard link keeps the original data reachable even if an
+    ** OpenVMS VERSION_LIMIT removes its old directory entry on creation.
+    */
+    if (!vms_recovery_link (originalfile, recoveryfile,
+          sizeof recoveryfile, statbuf))
+	{
+	(void) fprintf (stderr, ISPELL_C_BACKUP_FAILED, filename,
+	  MAYBE_CR (stderr));
+	(void) fclose (infile);
+	infile = NULL;
+	(void) restore_signal_mask (&old_signals);
+	return -1;
+	}
+    recovery_created = 1;
+
+    /* Keeping the old version in place makes the C RTL inherit its RMS
+    ** record format, maximum record size, carriage control, and protection.
+    */
+    outfile = fopen (target, "w");
+    if (outfile == NULL)
+	{
+	(void) fprintf (stderr, CANT_CREATE, target, MAYBE_CR (stderr));
+	copy_error = 1;
+	goto vms_update_done;
+	}
+    if (fgetname (outfile, newfile, 1) == NULL
+      &&  getname (fileno (outfile), newfile, 1) == NULL)
+	{
+	(void) fprintf (stderr, ISPELL_C_IO_FAILED, target,
+	  MAYBE_CR (stderr));
+	(void) fclose (outfile);
+	outfile = NULL;
+	orphan_created = 1;
+	copy_error = 1;
+	goto vms_update_done;
+	}
+    if (strcmp (newfile, originalfile) == 0)
+	{
+	(void) fprintf (stderr, ISPELL_C_IO_FAILED, target,
+	  MAYBE_CR (stderr));
+	(void) fclose (outfile);
+	outfile = NULL;
+	orphan_created = 1;
+	copy_error = 1;
+	goto vms_update_done;
+	}
+    if (fstat (fileno (outfile), &newstat) != 0)
+	{
+	(void) fprintf (stderr, ISPELL_C_IO_FAILED, target,
+	  MAYBE_CR (stderr));
+	(void) fclose (outfile);
+	outfile = NULL;
+	orphan_created = 1;
+	copy_error = 1;
+	goto vms_update_done;
+	}
+    if (vms_path_matches (originalfile, &newstat))
+	{
+	(void) fprintf (stderr, ISPELL_C_IO_FAILED, target,
+	  MAYBE_CR (stderr));
+	(void) fclose (outfile);
+	outfile = NULL;
+	orphan_created = 1;
+	copy_error = 1;
+	goto vms_update_done;
+	}
+    if (lstat (newfile, &linkstat) != 0)
+	{
+	(void) fprintf (stderr, ISPELL_C_IO_FAILED, target,
+	  MAYBE_CR (stderr));
+	(void) fclose (outfile);
+	outfile = NULL;
+	orphan_created = 1;
+	copy_error = 1;
+	goto vms_update_done;
+	}
+    if (S_ISLNK (linkstat.st_mode)
+      ||  !vms_path_matches (newfile, &newstat))
+	{
+	(void) fprintf (stderr,
+	  S_ISLNK (linkstat.st_mode) ? ISPELL_C_SYMLINK : ISPELL_C_IO_FAILED,
+	  S_ISLNK (linkstat.st_mode) ? newfile : target,
+	  MAYBE_CR (stderr));
+	(void) fclose (outfile);
+	outfile = NULL;
+	orphan_created = 1;
+	copy_error = 1;
+	goto vms_update_done;
+	}
+    new_created = 1;
+    newstat_valid = 1;
+
+    while ((c = getc (infile)) != EOF)
+	if (putc (c, outfile) == EOF)
+	    {
+	    copy_error = 1;
+	    break;
+	    }
+    if (ferror (infile)  ||  ferror (outfile))
+	copy_error = 1;
+    if (fflush (outfile) != 0)
+	copy_error = 1;
+    if (fclose (infile) != 0)
+	copy_error = 1;
+    infile = NULL;
+    if (fclose (outfile) != 0)
+	copy_error = 1;
+    outfile = NULL;
+    if (!copy_error  &&  chmod (newfile, statbuf->st_mode) != 0)
+	copy_error = 1;
+    if (copy_error)
+	{
+	(void) fprintf (stderr, ISPELL_C_IO_FAILED, filename,
+	  MAYBE_CR (stderr));
+	goto vms_update_done;
+	}
+
+    if (!vms_path_matches (newfile, &newstat)
+      ||  !vms_path_matches (target, &newstat))
+	{
+	(void) fprintf (stderr, ISPELL_C_IO_FAILED, filename,
+	  MAYBE_CR (stderr));
+	copy_error = 1;
+	}
+    else if (xflag)
+	{
+	if (!vms_recovery_discard (recoveryfile, statbuf))
+	    copy_error = 1;
+	if (!copy_error)
+	    recovery_created = 0;
+	}
+    else if (!vms_recovery_matches (recoveryfile, statbuf)
+      ||  rename (recoveryfile, backup_target) != 0)
+	{
+	(void) fprintf (stderr, ISPELL_C_BACKUP_FAILED, bakfile,
+	  MAYBE_CR (stderr));
+	copy_error = 1;
+	}
+    else
+	{
+	recovery_created = 0;
+	}
+
+vms_update_done:
+    if (infile != NULL)
+	{
+	(void) fclose (infile);
+	infile = NULL;
+	}
+    if (outfile != NULL)
+	{
+	(void) fclose (outfile);
+	outfile = NULL;
+	}
+    if (copy_error  &&  recovery_created)
+	{
+	if (orphan_created
+	  ||  !vms_restore_recovery (originalfile, target, recoveryfile,
+		statbuf, new_created  &&  newstat_valid ? newfile : (char *) NULL,
+		new_created  &&  newstat_valid ? &newstat : (struct stat *) NULL,
+		&previous_current))
+	    (void) fprintf (stderr, ISPELL_C_RECOVERY_LEFT, recoveryfile,
+	      MAYBE_CR (stderr));
+	else
+	    recovery_created = 0;
+	}
+    if (restore_signal_mask (&old_signals) != 0)
+	copy_error = 1;
+    if (copy_error)
+	{
+	(void) sleep ((unsigned) 2);
+	return -1;
+	}
+    return 0;
+    }
+#endif
+
+static int update_file (filename, statbuf, originalfile)
     char *		filename;
     struct stat *	statbuf;
+    char *		originalfile;
     {
+#ifdef __VMS
+    return update_file_vms (filename, statbuf, originalfile);
+#else
     char		bakfile[MAXPATHLEN];
     int			c;
+    int			copy_error;
     char *		pathtail;
+    sigset_t		blocked_signals;
+    sigset_t		old_signals;
 
     if ((infile = fopen (tempfile, "r")) == NULL)
 	{
 	(void) fprintf (stderr, ISPELL_C_TEMP_DISAPPEARED, tempfile,
 	  MAYBE_CR (stderr));
 	(void) sleep ((unsigned) 2);
-	return;
+	return -1;
 	}
 
 #ifdef TRUNCATEBAK
     (void) strncpy (bakfile, filename, sizeof bakfile - 1);
     bakfile[sizeof bakfile - 1] = '\0';
 #else /* TRUNCATEBAK */
-    (void) sprintf (bakfile, "%.*s%s", (int) (sizeof bakfile - sizeof BAKEXT),
-      filename, BAKEXT);
+#ifdef __VMS
+    {
+    char *	version = rindex (filename, ';');
+    size_t	base_length = version == NULL
+			  ? strlen (filename) : (size_t) (version - filename);
+    size_t	version_length = version == NULL ? 0 : strlen (version);
+
+    if (base_length + strlen (BAKEXT) + version_length >= sizeof bakfile)
+	{
+	(void) fprintf (stderr, ISPELL_C_PATH_TOO_LONG, filename,
+	  MAYBE_CR (stderr));
+	(void) fclose (infile);
+	infile = NULL;
+	return -1;
+	}
+    (void) memcpy (bakfile, filename, base_length);
+    (void) memcpy (bakfile + base_length, BAKEXT, strlen (BAKEXT));
+    if (version != NULL)
+	(void) memcpy (bakfile + base_length + strlen (BAKEXT), version,
+	  version_length + 1);
+    else
+	bakfile[base_length + strlen (BAKEXT)] = '\0';
+    }
+#else
+    if (strlen (filename) + strlen (BAKEXT) >= sizeof bakfile)
+	{
+	(void) fprintf (stderr, ISPELL_C_PATH_TOO_LONG, filename,
+	  MAYBE_CR (stderr));
+	(void) fclose (infile);
+	infile = NULL;
+	return -1;
+	}
+    (void) strcpy (bakfile, filename);
+    (void) strcat (bakfile, BAKEXT);
+#endif
 #endif /* TRUNCATEBAK */
     pathtail = last_slash (bakfile);
     if (pathtail == NULL)
@@ -1314,7 +2114,9 @@
     else
 	pathtail++;
 #ifdef TRUNCATEBAK
-    if (strcmp(BAKEXT, filename + strlen(filename) - sizeof BAKEXT + 1) != 0)
+    if (strlen (filename) < sizeof BAKEXT - 1
+      ||  strcmp (BAKEXT,
+	    filename + strlen (filename) - sizeof BAKEXT + 1) != 0)
 	{
 	if (strlen (pathtail) > MAXNAMLEN - sizeof BAKEXT + 1)
 	    pathtail[MAXNAMLEN - sizeof BAKEXT + 1] = '\0';
@@ -1371,23 +2173,63 @@
 	}
 #endif /* MSDOS */
 
-    if (strncmp (filename, bakfile, pathtail - bakfile + MAXNAMLEN) != 0)
-	(void) unlink (bakfile);	/* unlink so we can write a new one. */
+    (void) sigemptyset (&blocked_signals);
+    (void) sigaddset (&blocked_signals, SIGINT);
+    (void) sigaddset (&blocked_signals, SIGTERM);
+#ifdef SIGHUP
+    (void) sigaddset (&blocked_signals, SIGHUP);
+#endif
+#ifdef SIGQUIT
+    (void) sigaddset (&blocked_signals, SIGQUIT);
+#endif
+    if (sigprocmask (SIG_BLOCK, &blocked_signals, &old_signals) != 0)
+	{
+	(void) fprintf (stderr, ISPELL_C_SIGNAL_BLOCK_FAILED,
+	  MAYBE_CR (stderr));
+	(void) fclose (infile);
+	infile = NULL;
+	return -1;
+	}
+
+    if (strncmp (filename, bakfile, pathtail - bakfile + MAXNAMLEN) == 0)
+	{
+	(void) fprintf (stderr, ISPELL_C_BACKUP_FAILED, bakfile,
+	  MAYBE_CR (stderr));
+	(void) fclose (infile);
+	infile = NULL;
+	(void) restore_signal_mask (&old_signals);
+	(void) sleep ((unsigned) 2);
+	return -1;
+	}
+    (void) unlink (bakfile);	/* unlink so we can write a new one. */
 #ifdef HAS_RENAME
-    (void) rename (filename, bakfile);
+    if (rename (filename, bakfile) != 0)
 #else /* HAS_RENAME */
-    if (link (filename, bakfile) == 0)
-	(void) unlink (filename);
+    if (link (filename, bakfile) != 0  ||  unlink (filename) != 0)
 #endif /* HAS_RENAME */
+	{
+	(void) fprintf (stderr, ISPELL_C_BACKUP_FAILED, bakfile,
+	  MAYBE_CR (stderr));
+	(void) fclose (infile);
+	infile = NULL;
+	(void) restore_signal_mask (&old_signals);
+	(void) sleep ((unsigned) 2);
+	return -1;
+	}
 
     /* if we can't write new, preserve .bak regardless of xflag */
     if ((outfile = fopen (filename, "w")) == NULL)
 	{
 	(void) fprintf (stderr, CANT_CREATE, filename, MAYBE_CR (stderr));
+	(void) fclose (infile);
+	infile = NULL;
+	(void) restore_backup (filename, bakfile, 0);
+	(void) restore_signal_mask (&old_signals);
 	(void) sleep ((unsigned) 2);
-	return;
+	return -1;
 	}
 
+    copy_error = 0;
 #ifndef MSDOS
     /*
     ** This is usually a no-op on MS-DOS, but with file-sharing
@@ -1395,18 +2237,43 @@
     ** Apparently, the file-sharing module would close the file when
     ** `chmod' is called.
     */
-    (void) chmod (filename, statbuf->st_mode);
+    if (chmod (filename, statbuf->st_mode & 07777) != 0)
+	copy_error = 1;
 #endif
 
     while ((c = getc (infile)) != EOF)
-	(void) putc (c, outfile);
+	if (putc (c, outfile) == EOF)
+	    {
+	    copy_error = 1;
+	    break;
+	    }
+    if (ferror (infile)  ||  ferror (outfile))
+	copy_error = 1;
+    if (fflush (outfile) != 0)
+	copy_error = 1;
+    if (fclose (infile) != 0)
+	copy_error = 1;
+    infile = NULL;
+    if (fclose (outfile) != 0)
+	copy_error = 1;
+    outfile = NULL;
 
-    (void) fclose (infile);
-    (void) fclose (outfile);
+    if (copy_error)
+	{
+	(void) fprintf (stderr, ISPELL_C_IO_FAILED, filename,
+	  MAYBE_CR (stderr));
+	(void) restore_backup (filename, bakfile, 1);
+	(void) restore_signal_mask (&old_signals);
+	(void) sleep ((unsigned) 2);
+	return -1;
+	}
 
-    if (xflag
-      &&  strncmp (filename, bakfile, pathtail - bakfile + MAXNAMLEN) != 0)
+    if (xflag)
 	(void) unlink (bakfile);
+    if (restore_signal_mask (&old_signals) != 0)
+	return -1;
+    return 0;
+#endif
     }
 
 static void expandmode (option)
@@ -1500,6 +2367,9 @@
 #ifdef MSDOS
     char *		backslash;	/* Position of last backslash */
 #endif /* MSDOS */
+#ifdef __VMS
+    char *		vmssep;		/* Position of native VMS separator */
+#endif
     char *		slash;		/* Position of last slash */
 
     slash = rindex (file, '/');
@@ -1521,5 +2391,11 @@
 	slash = file + 1;
 #endif
 
+#ifdef __VMS
+    vmssep = vms_path_separator (file);
+    if (slash == NULL  ||  (vmssep != NULL  &&  vmssep > slash))
+	slash = vmssep;
+#endif
+
     return slash;
     }
