$NetBSD$

Restore the OpenVMS file-name handling carried by the native port.  VMS's
master directory is its own parent in directory-file syntax, so the generic
Unix symlink walk otherwise recurses until file-truename reports a cycle.

Also return usable VMS syntax for temporary directories and stop recursive
directory creation cleanly when a malformed name produces a parent cycle.

--- lisp/files.el.orig	2008-09-05 07:10:57.000000000 +0000
+++ lisp/files.el
@@ -806,6 +806,23 @@ containing it, until no links are left a
 	    (setq filename (or (w32-long-file-name filename) filename))))
 	(setq done t)))
 
+    ;; OpenVMS has no symbolic links, but logical names can denote search
+    ;; lists.  Resolve those here instead of recursively walking VMS's
+    ;; master directory as though it were a Unix directory hierarchy.
+    (when (memq system-type '(vax-vms vms))
+      (let ((result (vms-expand-search-paths filename t)))
+	(unless (file-attributes result)
+	  ;; If the file does not exist, canonicalize as much of its directory
+	  ;; specification as possible and leave the final component intact.
+	  (let ((dir1 (file-name-directory filename)))
+	    (unless (string= dir1 (file-name-directory result))
+	      (setq result
+		    (concat (file-name-as-directory
+			     (file-truename (directory-file-name dir1)))
+			    (file-name-nondirectory result))))))
+	(setq filename result
+	      done t)))
+
     ;; If this file directly leads to a link, process that iteratively
     ;; so that we don't use lots of stack.
     (while (not done)
@@ -924,7 +941,14 @@ If SUFFIX is non-nil, add it to the end 
	    ;; the file was somehow created by someone else between
	    ;; `make-temp-name' and `write-region', let's try again.
	    nil)
-	  file)
+	  ;; A VMS temporary directory made through a directory logical such as
+	  ;; SYS$SCRATCH: is otherwise returned as LOGICAL:NAME.  Treating that
+	  ;; value as DEFAULT-DIRECTORY produces LOGICAL:[NAME], which is only
+	  ;; valid for a concealed rooted logical.  Return canonical directory
+	  ;; syntax so callers can immediately create children in it.
+	  (if (and dir-flag (memq system-type '(vax-vms axp-vms)))
+	      (file-name-as-directory (file-truename file))
+	    file))
       ;; Reset the umask.
       (set-default-file-modes umask))))
 
@@ -4085,10 +4109,19 @@ PARENTS says whether to create parent di
       (if (not parents)
 	  (make-directory-internal dir)
 	(let ((dir (directory-file-name (expand-file-name dir)))
-	      create-list)
+	      create-list parent)
 	  (while (not (file-exists-p dir))
 	    (setq create-list (cons dir create-list)
-		  dir (directory-file-name (file-name-directory dir))))
+		  parent (file-name-directory dir))
+	    (unless parent
+	      (error "Cannot determine parent directory of %s" dir))
+	    (setq parent (directory-file-name parent))
+	    ;; Malformed names can make `file-name-directory' return the
+	    ;; same name (or cycle through names).  Do not cons forever and
+	    ;; eventually report "Memory exhausted".
+	    (when (member parent create-list)
+	      (error "Parent directory cycle while creating %s" dir))
+	    (setq dir parent))
 	  (while create-list
 	    (make-directory-internal (car create-list))
 	    (setq create-list (cdr create-list))))))))
