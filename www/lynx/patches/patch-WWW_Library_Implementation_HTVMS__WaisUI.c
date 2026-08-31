$NetBSD$

Declare LYStrNCpy so VSI C applies the same external-name prefix used by its
definition.

--- WWW/Library/Implementation/HTVMS_WaisUI.c.orig	2020-01-21 22:00:50.000000000 +0000
+++ WWW/Library/Implementation/HTVMS_WaisUI.c
@@ -49,6 +49,7 @@
 
 #include <LYexit.h>
 #include <LYLeaks.h>
+#include <LYStrings.h>
 
 void log_write(char *s GCC_UNUSED)
 {
