$NetBSD$

The Emacs 22 distfile includes stale bytecode for files.el, so native VMS
builds must explicitly load the corrected source while producing the dump.
Also canonicalize default-directory before the dump's executable-version
scan; the MMS startup environment can expose its directory-file spelling.

--- lisp/loadup.el.orig	2008-09-05 07:10:57.000000000 +0000
+++ lisp/loadup.el
@@ -67,7 +67,7 @@
 (load "format")
 (load "bindings")
 (setq load-source-file-function 'load-with-code-conversion)
-(load "files")
+(load (if (eq system-type 'vax-vms) "files.el" "files"))
 
 (load "cus-face")
 (load "faces")  ; after here, `defface' may be used.
@@ -236,6 +236,11 @@
 ;; for the sake of the next call to precompute-menubar-bindings.
 (setq define-key-rebound-commands nil)
 
+;; Keep the VMS dump's version scan on directory syntax even when the
+;; startup environment supplied the current directory in file syntax.
+(if (eq system-type 'vax-vms)
+    (setq default-directory (file-name-as-directory default-directory)))
+
 ;; Determine which last version number to use
 ;; based on the executables that now exist.
 (if (and (or (equal (nth 3 command-line-args) "dump")
