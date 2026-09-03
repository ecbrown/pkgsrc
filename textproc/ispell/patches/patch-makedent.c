$NetBSD$

Provide exact OpenVMS version parsing and file-identity helpers used by the
transactional main-file and personal-dictionary writers.  Recovery aliases
are removed only after the expected old and replacement versions are proven.

--- makedent.c.orig
+++ makedent.c
@@ -123,6 +123,13 @@
 #include "proto.h"
 #include "msgs.h"
 #include <ctype.h>
+#ifdef __VMS
+#include <errno.h>
+#include <nam.h>
+#include <string.h>
+#include <sys/stat.h>
+#include <unistd.h>
+#endif
 
 int		makedent P ((unsigned char * lbuf, int lbuflen,
 		  struct dent * ent));
@@ -164,6 +171,264 @@
 
 static int  	has_marker;
 
+#ifdef __VMS
+int vms_char_escaped (start, character)
+    const char *	start;
+    const char *	character;
+    {
+    int			carets = 0;
+
+    while (character > start  &&  character[-1] == '^')
+	{
+	carets++;
+	character--;
+	}
+    return carets & 1;
+    }
+
+char * vms_find_last_unescaped (name, characters)
+    char *		name;
+    const char *	characters;
+    {
+    char *		last = NULL;
+    char *		cp;
+
+    for (cp = name;  *cp != '\0';  cp++)
+	if (strchr (characters, *cp) != NULL
+	  &&  !vms_char_escaped (name, cp))
+	    last = cp;
+    return last;
+    }
+
+char * vms_path_separator (name)
+    char *		name;
+    {
+    char *		native;
+    char *		slash;
+
+    native = vms_find_last_unescaped (name, "]>:");
+    slash = rindex (name, '/');
+    if (native == NULL  ||  (slash != NULL  &&  slash > native))
+	native = slash;
+    return native;
+    }
+
+char * vms_version_field (name)
+    char *		name;
+    {
+    char *		semicolon;
+    char *		cp;
+
+    semicolon = vms_find_last_unescaped (name, ";");
+    if (semicolon == NULL)
+	return NULL;
+    cp = semicolon + 1;
+    if (*cp == '\0')
+	return semicolon;
+    if (*cp == '-')
+	cp++;
+    if (*cp == '*')
+	return cp[1] == '\0' ? semicolon : (char *) NULL;
+    if (!isdigit ((unsigned char) *cp))
+	return NULL;
+    do
+	cp++;
+    while (isdigit ((unsigned char) *cp));
+    return *cp == '\0' ? semicolon : (char *) NULL;
+    }
+
+static int vms_same_file (left, right)
+    struct stat *	left;
+    struct stat *	right;
+    {
+    return left->st_dev == right->st_dev
+      &&  memcmp (&left->st_ino, &right->st_ino,
+	    sizeof left->st_ino) == 0;
+    }
+
+int vms_path_matches (path, expected)
+    char *		path;
+    struct stat *	expected;
+    {
+    struct stat		actual;
+
+    return stat (path, &actual) == 0  &&  vms_same_file (&actual, expected);
+    }
+
+int vms_recovery_matches (recovery, expected)
+    char *		recovery;
+    struct stat *	expected;
+    {
+    return expected != NULL  &&  vms_path_matches (recovery, expected);
+    }
+
+int vms_recovery_discard (recovery, expected)
+    char *		recovery;
+    struct stat *	expected;
+    {
+    if (!vms_recovery_matches (recovery, expected))
+	return 0;
+    return unlink (recovery) == 0;
+    }
+
+int vms_unlink_if_matches (path, expected)
+    char *		path;
+    struct stat *	expected;
+    {
+    if (expected == NULL  ||  !vms_path_matches (path, expected))
+	return 0;
+    return unlink (path) == 0;
+    }
+
+int vms_recovery_link (source, recovery, recovery_size, expected)
+    char *		source;
+    char *		recovery;
+    size_t		recovery_size;
+    struct stat *	expected;
+    {
+    unsigned int	attempt;
+    int			length;
+    char *		separator;
+    size_t		prefix_length;
+    char *		version;
+
+    separator = vms_path_separator (source);
+    prefix_length = separator == NULL ? 0 : (size_t) (separator - source + 1);
+    version = vms_version_field (source);
+    for (attempt = 0;  attempt < 1000;  attempt++)
+	{
+	length = snprintf (recovery, recovery_size,
+	      "%.*sISPELL$RECOVERY_%lu_%u.TMP%s", (int) prefix_length,
+	      source, (unsigned long) getpid (), attempt,
+	      version == NULL ? "" : version);
+	if (length < 0  ||  (size_t) length >= recovery_size)
+	    {
+	    errno = ENAMETOOLONG;
+	    return 0;
+	    }
+	if (link (source, recovery) == 0)
+	    {
+	    if (expected == NULL  ||  vms_path_matches (recovery, expected))
+		return 1;
+	    (void) unlink (recovery);
+	    errno = EAGAIN;
+	    return 0;
+	    }
+	if (errno != EEXIST)
+	    return 0;
+	}
+    errno = EEXIST;
+    return 0;
+    }
+
+int vms_restore_recovery (original, target, recovery, expected,
+  replacement, replacement_expected, previous_current)
+    char *		original;
+    char *		target;
+    char *		recovery;
+    struct stat *	expected;
+    char *		replacement;
+    struct stat *	replacement_expected;
+    struct stat *	previous_current;
+    {
+    int			link_errno;
+    int			replacement_status;
+    char			restore_target[NAML$C_MAXRSS + 1];
+    struct stat		current;
+
+    if (expected == NULL  ||  previous_current == NULL
+      ||  !vms_path_matches (recovery, expected))
+	return 0;
+
+    if (stat (original, &current) == 0)
+	{
+	if (!vms_same_file (&current, expected))
+	    return 0;
+	if (replacement != NULL)
+	    {
+	    if (replacement_expected == NULL)
+		return 0;
+	    replacement_status = stat (replacement, &current);
+	    if (replacement_status == 0)
+		{
+		if (!vms_same_file (&current, replacement_expected)
+		  ||  !vms_path_matches (target, replacement_expected)
+		  ||  unlink (replacement) != 0)
+		    return 0;
+		}
+	    else if (errno != ENOENT)
+		return 0;
+	    }
+	if (!vms_path_matches (original, expected)
+	  ||  !vms_path_matches (target, previous_current)
+	  ||  !vms_path_matches (recovery, expected))
+	    return 0;
+	return unlink (recovery) == 0;
+	}
+    if (errno != ENOENT  ||  replacement == NULL
+      ||  replacement_expected == NULL)
+	return 0;
+
+    /* The old entry was consumed by VERSION_LIMIT.  First try to promote
+    ** the recovery object while the failed version still keeps the filename
+    ** family (and therefore its version limit) alive.  If ;-1 already names
+    ** a surviving prior version, remove only the validated failed version
+    ** and restore the recovery link under its original exact version.
+    */
+    if (!vms_path_matches (replacement, replacement_expected)
+      ||  !vms_path_matches (target, replacement_expected)
+      ||  strlen (target) + sizeof ";-1" > sizeof restore_target)
+	return 0;
+    (void) strcpy (restore_target, target);
+    (void) strcat (restore_target, ";-1");
+    if (link (recovery, restore_target) != 0)
+	{
+	link_errno = errno;
+	if (link_errno != EEXIST
+	  ||  !vms_path_matches (replacement, replacement_expected)
+	  ||  !vms_path_matches (target, replacement_expected)
+	  ||  unlink (replacement) != 0
+	  ||  !vms_path_matches (target, previous_current)
+	  ||  link (recovery, original) != 0
+	  ||  !vms_path_matches (original, expected)
+	  ||  !vms_path_matches (target, previous_current)
+	  ||  !vms_path_matches (recovery, expected))
+	    return 0;
+	return unlink (recovery) == 0;
+	}
+    if (!vms_path_matches (target, expected))
+	return 0;
+
+    replacement_status = stat (replacement, &current);
+    if (replacement_status == 0)
+	{
+	if (!vms_same_file (&current, replacement_expected)
+	  ||  unlink (replacement) != 0)
+	    return 0;
+	}
+    else if (errno != ENOENT)
+	return 0;
+    if (!vms_path_matches (target, expected)
+	  ||  !vms_path_matches (recovery, expected))
+	return 0;
+    return unlink (recovery) == 0;
+    }
+
+static int vms_name_equal (left, right, length)
+    char *		left;
+    char *		right;
+    size_t		length;
+    {
+    size_t		i;
+
+    for (i = 0;  i < length;  i++)
+	if (tolower ((unsigned char) left[i])
+	  != tolower ((unsigned char) right[i]))
+	    return 0;
+    return 1;
+    }
+#endif
+
 /*
  * Fill in a directory entry, including setting the capitalization flags, and
  * allocate and initialize memory for the d->word field.  Returns -1
@@ -1107,7 +1372,14 @@
 	{
 	for (i = 0;  i < hashheader.nstrchartype;  i++)
 	    {
-	    if (strcmp (name, (char *) chartypes[i].name) == 0)
+	    if (
+#ifdef __VMS
+	      len == (int) strlen ((char *) chartypes[i].name)
+	      &&  vms_name_equal (name, (char *) chartypes[i].name, len)
+#else
+	      strcmp (name, (char *) chartypes[i].name) == 0
+#endif
+	      )
 		{
 		if (deformatter != NULL)
 		    {
@@ -1124,12 +1396,27 @@
 		}
 	    }
 	}
+#ifdef __VMS
+    {
+    char *version = vms_version_field (name);
+    if (version != NULL)
+	len = version - name;
+    }
+#endif
     for (i = 0;  i < hashheader.nstrchartype;  i++)
 	{
 	for (cp = chartypes[i].suffixes;  *cp != '\0';  cp += cplen + 1)
 	    {
 	    cplen = strlen (cp);
-	    if (len >= cplen  &&  strcmp (&name[len - cplen], cp) == 0)
+	    if (len >= cplen
+#ifdef __VMS
+	      &&  (cp[0] != '.'
+	        ||  vms_find_last_unescaped (name, ".") == &name[len - cplen])
+	      &&  vms_name_equal (&name[len - cplen], cp, cplen)
+#else
+	      &&  strcmp (&name[len - cplen], cp) == 0
+#endif
+	      )
 		{
 		if (deformatter != NULL)
 		    {
