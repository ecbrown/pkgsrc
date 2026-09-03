$NetBSD$

Add diagnostics for safe external-filter, personal-dictionary, and terminal
failure handling.

--- languages/english/msgs.h.orig
+++ languages/english/msgs.h
@@ -181,6 +181,9 @@
 #define BHASH_C_COUNTING	"Counting words in dictionary ...\n"
 #define BHASH_C_WORD_COUNT	"\n%d words\n"
 #define BHASH_C_USAGE		"Usage:  buildhash [-s] dict-file aff-file hash-file\n\tbuildhash -c count aff-file\n"
+#define BHASH_C_PATH_TOO_LONG	"buildhash: pathname is too long: %s%s\n"
+#define BHASH_C_WRITE_FAILED	"buildhash: error writing %s%s\n"
+#define BHASH_C_READ_FAILED	"buildhash: error reading %s%s\n"
 
 /*
  * The following strings are used in correct.c:
@@ -223,6 +226,18 @@
 #define CORR_C_BLANK_MORE	"\r           \r"
 #define CORR_C_END_LOOK		"--end--"
 #define CORR_C_SHORT_SOURCE	"ispell:  unexpected EOF on unfiltered version of input%s\n"
+#define CORR_C_SHORT_FILTER	"ispell:  unexpected EOF on filtered version of input%s\n"
+#define CORR_C_LOOKUP_TOO_LONG	"Lookup pattern is too long.%s\n"
+#define ISPELL_C_FILTER_FAILED	"External deformatter \"%s\" failed.%s\n"
+#define ISPELL_C_IO_FAILED	"I/O error while processing \"%s\".%s\n"
+#define ISPELL_C_PATH_TOO_LONG	"Dictionary pathname is too long: %s%s\n"
+#define ISPELL_C_BACKUP_FAILED	"Can't safely create backup file %s%s\n"
+#define ISPELL_C_RESTORE_FAILED	"Can't restore %s from backup %s%s\n"
+#define ISPELL_C_SIGNAL_BLOCK_FAILED "Can't protect file replacement from signals%s\n"
+#define ISPELL_C_SIGNAL_RESTORE_FAILED "Can't restore the process signal mask%s\n"
+#define ISPELL_C_RECOVERY_LEFT	"Recovery link remains at %s%s\n"
+#define ISPELL_C_SYMLINK	"Refusing to replace OpenVMS symbolic link %s%s\n"
+#define TREE_C_PATH_TOO_LONG	"Personal dictionary pathname is too long%s\n"
 
 /*
  * The following strings are used in defmt.c:
@@ -328,6 +343,11 @@
 #define TERM_C_NO_BATCH		"Can't deal with non-interactive use yet.\n"
 #define TERM_C_CANT_FORK	"Couldn't fork, try later.%s\n"
 #define TERM_C_TYPE_SPACE	"\n-- Type space to continue --"
+#define TERM_C_BAD_TERM		"Can't initialize terminal type \"%s\".\n"
+#define TERM_C_MISSING_CAPS	"Terminal type \"%s\" lacks cursor/clear capabilities.\n"
+#define TERM_C_TTY_MODE		"Can't set terminal mode"
+#define TERM_C_VFORK_MODE	"Can't enable safe subprocess handling"
+#define TERM_C_READ		"Can't read from terminal"
 
 /*
  * The following strings are used in tgood.c:
@@ -338,6 +358,12 @@
  * The following strings are used in tree.c:
  */
 #define TREE_C_CANT_UPDATE	"Warning: Cannot update personal dictionary (%s)%s\n"
+#define TREE_C_CANT_READ	"Warning: Cannot completely read personal dictionary (%s)%s\n"
+#define TREE_C_MULTILINK	"Warning: Cannot safely replace multiply-linked personal dictionary (%s)%s\n"
+#define TREE_C_RECOVERY_LEFT	"Warning: Personal dictionary recovery link remains at %s%s\n"
+#define TREE_C_PARTIAL_LEFT	"Warning: Incomplete personal dictionary remains at %s%s\n"
+#define TREE_C_SYMLINK		"Warning: Cannot safely replace symbolic-link personal dictionary (%s)%s\n"
+#define TREE_C_REFRESH_FAILED	"Warning: Cannot refresh the restored personal dictionary identity%s\n"
 #define TREE_C_NO_SPACE		"Ran out of space for personal dictionary%s\n"
 #define TREE_C_TRY_ANYWAY	"Continuing anyway (with reduced performance).%s\n"
 
