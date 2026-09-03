$NetBSD$

Declare the OpenVMS single-keystroke input helper used by GETKEYSTROKE.

--- proto.h.orig
+++ proto.h
@@ -117,6 +117,10 @@
 
 #include "ispell.h"		/* For definition of P */
 
+#ifdef __VMS
+struct stat;
+#endif
+
 extern int	addvheader P ((struct dent * ent));
 extern void	askmode P ((void));
 extern void	backup P ((void));
@@ -197,9 +201,34 @@
 extern void	toutent P ((FILE * outfile, struct dent * hent,
 		  int onlykeep));
 extern void	treeinit P ((char * persdict, char * LibDict));
-extern void	treeinsert P ((unsigned char * word, int wordlen, int keep));
+extern int	treeinsert P ((unsigned char * word, int wordlen, int keep));
+#ifdef __VMS
+extern int	vms_char_escaped P ((const char * start,
+		  const char * character));
+extern char *	vms_find_last_unescaped P ((char * name,
+		  const char * characters));
+extern char *	vms_path_separator P ((char * name));
+extern int	vms_path_matches P ((char * path,
+		  struct stat * expected));
+extern int	vms_recovery_link P ((char * source, char * recovery,
+		  size_t recovery_size, struct stat * expected));
+extern int	vms_recovery_matches P ((char * recovery,
+		  struct stat * expected));
+extern int	vms_recovery_discard P ((char * recovery,
+		  struct stat * expected));
+extern int	vms_unlink_if_matches P ((char * path,
+		  struct stat * expected));
+extern int	vms_restore_recovery P ((char * original, char * target,
+		  char * recovery, struct stat * expected, char * replacement,
+		  struct stat * replacement_expected,
+		  struct stat * previous_current));
+extern char *	vms_version_field P ((char * name));
+#endif
 extern struct dent * treelookup P ((ichar_t * word));
-extern void	treeoutput P ((void));
+extern int	treeoutput P ((void));
+#ifdef __VMS
+extern int	vms_get_keystroke P ((void));
+#endif
 extern void	upcase P ((ichar_t * string));
 extern long	whatcap P ((ichar_t * word));
 extern char *	xgets P ((char * string, int size, FILE * stream));
