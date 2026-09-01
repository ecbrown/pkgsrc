$NetBSD$

On OpenVMS the native Configure.com keeps the install root in the
VMS-specific vms_prefix variable.  Honor the prefix selected by -Dprefix
instead of leaving vms_prefix at the built-in perl_root default.

--- Configure.com.orig	2025-03-30 10:35:38.000000000 +0000
+++ Configure.com
@@ -2240,6 +2240,8 @@
 $ ELSE 
 $   prefix = dflt
 $ ENDIF
 $ perl_root = prefix
+$ vms_prefix = prefix
+$ vms_prefixup = F$EDIT(vms_prefix,"UPCASE")
 $!
 $! Check here for pre-existing PERL_ROOT.
 $!  -> ask if removal desired.
