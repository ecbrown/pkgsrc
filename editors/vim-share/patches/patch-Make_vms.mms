$NetBSD$

Use the pkgsrc installation paths when no VIM or VIMRUNTIME logical names
are defined.

--- src/Make_vms.mms.orig	2026-08-30 21:19:06.000000000 +0000
+++ src/Make_vms.mms
@@ -272,8 +272,8 @@ LDFLAGS   = $(MAP_OPT)
 
 # Predefined VIM directories
 # Please, use $VIM and $VIMRUNTIME logicals instead
-VIMLOC  = ""
-VIMRUN  = ""
+VIMLOC  = "@PREFIX@/share/vim"
+VIMRUN  = "@PREFIX@/share/vim/@VIM_SUBDIR@"
 
 CONFIG_H = os_vms_conf.h
 
