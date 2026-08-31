$NetBSD$

DEFAULT_TMPDIR already names /sys$scratch/ on OpenVMS.  Do not append a second
sys$scratch device specification to it, and keep the six X characters at the
end as required by mkstemp().

--- src/misc.c.orig	2023-02-20 19:46:52.000000000 +0000
+++ src/misc.c
@@ -573,7 +573,7 @@ umask (mode_t mask)
 #endif
 
 #ifdef VMS
-# define DEFAULT_TMPFILE    "sys$scratch:gnv$make_cmdXXXXXX.com"
+# define DEFAULT_TMPFILE    "gnv_make_cmdXXXXXX"
 #else
 # define DEFAULT_TMPFILE    "GmXXXXXX"
 #endif
