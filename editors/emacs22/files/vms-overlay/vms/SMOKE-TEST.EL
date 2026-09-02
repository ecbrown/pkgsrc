;;; smoke-test.el --- OpenVMS runtime checks for Emacs 22  -*- no-byte-compile: t -*-

(defvar emacs-vms-smoke-failures nil)
(defvar emacs-vms-smoke-use-source-tree t)

;; Exercise the files from the tree under test.  A configured system can
;; legitimately have older installed .el/.elc files earlier in load-path.
(when emacs-vms-smoke-use-source-tree
  (let* ((vms-source-directory (file-name-directory load-file-name))
	 (lisp-source-directory
	  (expand-file-name "[-.lisp]" vms-source-directory))
	 (gnus-source-directory
	  (expand-file-name "[.gnus]" lisp-source-directory)))
    ;; Dired loads dired-vms.el itself, so keep the tree under test ahead of
    ;; an older installed copy for the duration of this run.
    (setq load-path (cons lisp-source-directory
			  (cons gnus-source-directory load-path)))
    (load (expand-file-name "dired-vms.el" lisp-source-directory) nil nil t)
    (load (expand-file-name "gnus-agent.el" gnus-source-directory) nil nil t)))

(defun emacs-vms-smoke-record (name passed &optional detail)
  (princ (format "E22-%s=%s%s\n"
		 (upcase (symbol-name name))
		 (if passed "OK" "FAIL")
		 (if detail (format " %s" detail) "")))
  (unless passed
    (setq emacs-vms-smoke-failures
	  (cons name emacs-vms-smoke-failures))))

(defun emacs-vms-smoke-filespec-tail (name)
  "Return the part of native file specification NAME after its device."
  (let ((index (1- (length name))))
    (while (and (>= index 0) (/= (aref name index) ?:))
      (setq index (1- index)))
    (substring name (1+ index))))

(emacs-vms-smoke-record
 'version (string-match "\\`22\\.3" emacs-version) emacs-version)
(emacs-vms-smoke-record 'batch (= (1+ (1+ (1+ 39))) 42))
(emacs-vms-smoke-record 'pure-space (not pure-space-overflow))

(princ (format "E22-DOC-CONTEXT file=%S directory=%S data=%S count=%S\n"
	       internal-doc-file-name doc-directory
	       (not (null (member "data.o" build-files))) (length build-files)))
(let ((doc (documentation 'car)))
  (emacs-vms-smoke-record
   'doc (and (stringp doc) (string-match "\\`Return the car of LIST" doc))
   (format "%S" doc)))

(condition-case err
    (let* ((file (locate-library "simple"))
	   (root (and file (file-name-directory file))))
      (emacs-vms-smoke-record
	'filename (and file (file-readable-p file)) file)
      (emacs-vms-smoke-record
	'truename (and root (stringp (file-truename root)))
	(and root (file-truename root))))
  (error
   (emacs-vms-smoke-record 'filename nil (error-message-string err))
   (emacs-vms-smoke-record 'truename nil (error-message-string err))))

(condition-case err
    (let* ((file (locate-library "simple"))
	   (directory (and file (file-name-directory file)))
	   (directory-file (and directory (directory-file-name directory)))
	   (bare-directory-file
	    (and directory-file
		 (replace-regexp-in-string
		  "\\.DIR\\(?:\\.1\\)?\\'" "" directory-file t t)))
	   (converted (and bare-directory-file
			   (file-name-as-directory bare-directory-file)))
	   (listing (and bare-directory-file
			 (directory-files bare-directory-file nil nil t))))
      (emacs-vms-smoke-record
	'vms-directory-syntax
	(and bare-directory-file
	     (file-directory-p bare-directory-file)
	     (string= (upcase converted) (upcase directory))
	     listing)
	(format "bare=%S converted=%S expected=%S entries=%d"
		bare-directory-file converted directory (length listing))))
  (error
   (emacs-vms-smoke-record
    'vms-directory-syntax nil (error-message-string err))))

(condition-case err
    (let ((buffer (dired-noselect (expand-file-name "~"))))
      (unwind-protect
	  (with-current-buffer buffer
	    (emacs-vms-smoke-record
	     'dired
	     (and (eq major-mode 'dired-mode)
		  dired-subdir-alist
		  (> (count-lines (point-min) (point-max)) 1))
	     (format "directory=%S lines=%d subdirs=%d"
		     dired-directory
		     (count-lines (point-min) (point-max))
		     (length dired-subdir-alist))))
	(when (buffer-live-p buffer)
	  (kill-buffer buffer))))
  (error
   (emacs-vms-smoke-record 'dired nil (error-message-string err))))

(let* ((unix-directory "~/News/drafts/drafts")
	(converted (file-name-as-directory unix-directory)))
  (emacs-vms-smoke-record
   'vms-unix-directory-syntax
   (string= converted (concat unix-directory "/"))
   (format "directory=%S converted=%S" unix-directory converted)))

(let* ((directory-file "SYS$DISK:[000000]FOO^.BAR.DIR;1")
	(expected-directory-file "[000000]FOO^.BAR.DIR.1")
	(directory "SYS$DISK:[000000.FOO^.BAR]")
	(expected-directory "[000000.FOO^.BAR]")
	(as-directory (file-name-as-directory directory-file))
	(as-file (directory-file-name directory)))
  (emacs-vms-smoke-record
   'ods5-escaped-dot-directory
   (and (string= (upcase (emacs-vms-smoke-filespec-tail as-directory))
		 (upcase expected-directory))
	(string= (upcase (emacs-vms-smoke-filespec-tail as-file))
		 (upcase expected-directory-file)))
   (format "as-directory=%S as-file=%S" as-directory as-file)))

(let* ((directory-file "SYS$DISK:[000000]FOO^]BAR.DIR;1")
	(expected-directory-file "[000000]FOO^]BAR.DIR.1")
	(directory "SYS$DISK:[000000.FOO^]BAR]")
	(expected-directory "[000000.FOO^]BAR]")
	(as-directory (file-name-as-directory directory-file))
	(as-file (directory-file-name directory)))
  (emacs-vms-smoke-record
   'ods5-escaped-bracket-directory
   (and (string= (upcase (emacs-vms-smoke-filespec-tail as-directory))
		 (upcase expected-directory))
	(string= (upcase (emacs-vms-smoke-filespec-tail as-file))
		 (upcase expected-directory-file)))
   (format "as-directory=%S as-file=%S" as-directory as-file)))

(dolist (leaf '("A^]B.TXT;1" "A^>B.TXT;1" "A^:B.TXT;1"))
  (let* ((name (concat "SYS$DISK:[BASE]" leaf))
	 (directory (file-name-directory name))
	 (nondirectory (file-name-nondirectory name)))
    (emacs-vms-smoke-record
     (intern (concat "ods5-components-"
		     (number-to-string (aref leaf 2))))
     (and (string= directory "SYS$DISK:[BASE]")
	  (string= nondirectory leaf))
     (format "name=%S directory=%S nondirectory=%S"
	     name directory nondirectory))))

(let* ((even "SYS$DISK:[BASE]A^^]B.TXT;1")
	(odd "SYS$DISK:[BASE]A^^^]B.TXT;1"))
  (emacs-vms-smoke-record
   'ods5-caret-parity
   (and (string= (file-name-directory even) "SYS$DISK:[BASE]A^^]")
	(string= (file-name-nondirectory even) "B.TXT;1")
	(string= (file-name-directory odd) "SYS$DISK:[BASE]")
	(string= (file-name-nondirectory odd) "A^^^]B.TXT;1")
	(string= (directory-file-name "E22^]") "E22^]"))
   (format "even-dir=%S odd-dir=%S"
	   (file-name-directory even) (file-name-directory odd))))

(let* ((name "SYS$DISK:[BASE.FOO^.-]BAR.TXT;1")
	(expanded (expand-file-name name)))
  (emacs-vms-smoke-record
   'ods5-slow-path-escaped-dot
   (string= (upcase expanded) (upcase name))
   (format "name=%S expanded=%S" name expanded)))

(let ((home (expand-file-name "~"))
      (home-with-slash (expand-file-name "~/")))
  (emacs-vms-smoke-record
   'home-directory
   (and (file-directory-p home)
	(string= (upcase home) (upcase home-with-slash)))
   (format "home=%S slash=%S" home home-with-slash)))

(condition-case err
    (let* ((user (user-login-name))
	   (named (expand-file-name (concat "~" user)))
	   (named-with-slash (expand-file-name (concat "~" user "/"))))
      (emacs-vms-smoke-record
	'named-home-directory
	(and (file-directory-p named)
	     (string= (upcase named) (upcase named-with-slash)))
	(format "user=%S named=%S slash=%S" user named named-with-slash)))
  (error
   (emacs-vms-smoke-record
    'named-home-directory nil (error-message-string err))))

(emacs-vms-smoke-record
 'empty-directory-file-name
 (string= (directory-file-name "") ""))

(condition-case err
    (let* ((expected "~/EMACS/AUTO_SAVE_LIST/SAVES-")
	   (directory (file-name-directory auto-save-list-file-prefix))
	   (expanded (and directory (expand-file-name directory))))
      (make-directory directory t)
      (emacs-vms-smoke-record
	'auto-save-directory
	(and (string= auto-save-list-file-prefix expected)
	     (file-directory-p expanded))
	(format "prefix=%S directory=%S expanded=%S"
		auto-save-list-file-prefix directory expanded)))
  (error
   (emacs-vms-smoke-record
    'auto-save-directory nil (error-message-string err))))

(let ((entry (assoc "\\.com\\'" auto-mode-alist)))
  (emacs-vms-smoke-record
   'com-auto-mode
   (eq (cdr entry) 'dcl-mode)
   (format "entry=%S" entry)))

(condition-case err
    (with-temp-buffer
      (setq buffer-file-name
	    (concat (file-name-as-directory (expand-file-name "~"))
		    "SMOKE.TXT;17"))
      (let ((name (make-auto-save-file-name)))
	(emacs-vms-smoke-record
	 'auto-save-version
	 (and (not (string-match ";17" name))
	      (auto-save-file-name-p (file-name-nondirectory name))
	      (not (auto-save-file-name-p
		    (concat (file-name-nondirectory name) "JUNK"))))
	 (format "name=%S" name))))
  (error
   (emacs-vms-smoke-record
    'auto-save-version nil (error-message-string err))))

(condition-case err
    (with-temp-buffer
      (insert "  lowercase^_mixed-name.txt;42  1  27-AUG-2026\n")
      (goto-char (point-min))
      (let ((start (dired-move-to-filename))
	    (end (dired-move-to-end-of-filename)))
	(emacs-vms-smoke-record
	 'ods5-dired-parser
	 (and start end
	      (string= (buffer-substring start end)
		       "lowercase^_mixed-name.txt;42"))
	 (format "start=%S end=%S value=%S"
		 start end (and start end (buffer-substring start end))))))
  (error
   (emacs-vms-smoke-record
   'ods5-dired-parser nil (error-message-string err))))

(condition-case err
    (with-temp-buffer
      (insert "  Mixed Case File.txt;7  1  27-AUG-2026\n")
      (goto-char (point-min))
      (let ((parsed (dired-get-filename 'verbatim t)))
	(emacs-vms-smoke-record
	 'ods5-dired-space
	 (string= parsed "Mixed Case File.txt;7")
	 (format "value=%S" parsed))))
  (error
   (emacs-vms-smoke-record
    'ods5-dired-space nil (error-message-string err))))

(condition-case err
    (with-temp-buffer
      (let ((minibuffer-completing-file-name t))
	(vms-magic-right-square-brace)
	(emacs-vms-smoke-record
	 'empty-magic-bracket (string= (buffer-string) "]")
	 (format "value=%S" (buffer-string)))))
  (error
   (emacs-vms-smoke-record
    'empty-magic-bracket nil (error-message-string err))))

(condition-case err
    (let ((original-info (symbol-function 'vms-system-info))
	  (original-command-line (symbol-function 'command-line))
	  command-line-called)
      (unwind-protect
	  (progn
	    (fset 'command-line (lambda () (setq command-line-called t)))
	    (fset 'vms-system-info (lambda (&rest ignored) nil))
	    (vms-command-line-again)
	    (fset 'vms-system-info (lambda (&rest ignored) ""))
	    (vms-command-line-again)
	    (emacs-vms-smoke-record
	     'empty-resume-command-line (not command-line-called)))
	(fset 'vms-system-info original-info)
	(fset 'command-line original-command-line)))
  (error
   (emacs-vms-smoke-record
    'empty-resume-command-line nil (error-message-string err))))

(condition-case err
    (progn
      (require 'gnus-agent)
      (let ((original-gnus (symbol-function 'gnus))
	    (gnus-plugged t)
	    (gnus-invocation 'not-called))
	(unwind-protect
	    (progn
	      (fset 'gnus
		    (lambda (&rest args)
		      (setq gnus-invocation args)))
	      (gnus-unplugged)
	      (emacs-vms-smoke-record
	       'gnus-unplugged
	       (and (null gnus-plugged)
		    (null gnus-invocation))
	       (format "plugged=%S invocation=%S"
		       gnus-plugged gnus-invocation)))
	  (fset 'gnus original-gnus))))
  (error
   (emacs-vms-smoke-record
    'gnus-unplugged nil (error-message-string err))))

(let ((file (locate-library "calendar")))
  (emacs-vms-smoke-record
   'subdir-library (and file (file-readable-p file))
   (or file (format "load-path=%S" load-path))))

(condition-case err
    (let* ((component (make-string 40 ?a))
	   (relative (mapconcat 'identity (make-list 9 component) "/"))
	   (expanded (expand-file-name relative default-directory)))
      (emacs-vms-smoke-record
	'long-unix-path-translation
	(and (stringp expanded) (> (length expanded) 255))
	(format "relative=%d expanded=%d value=%S"
		(length relative) (length expanded) expanded)))
  (error
   (emacs-vms-smoke-record
    'long-unix-path-translation nil (error-message-string err))))

(condition-case err
    (let ((file (make-temp-file "E22_COPY_SELF_"))
	  (copy (make-temp-file "E22_COPY_DISTINCT_"))
	  (logical (format "E22_COPY_%s" (vms-system-info "PID")))
	  same-file-rejected logical-alias-rejected
	  (versions-left 20))
      (unwind-protect
	  (progn
	    (with-temp-file file (insert "E22-COPY-SELF"))
	    (define-logical-name logical (file-name-directory file))
	    (setq same-file-rejected
		  (condition-case nil
		      (progn (copy-file file file t) nil)
		    (file-error t)))
	    (setq logical-alias-rejected
		  (condition-case nil
		      (progn
			(copy-file file
				   (concat logical ":"
					   (file-name-nondirectory file))
				   t)
			nil)
		    (file-error t)))
	    (copy-file file copy t)
	    (with-temp-buffer
	      (insert-file-contents file)
	      (emacs-vms-smoke-record
	       'copy-file-self
	       (and same-file-rejected logical-alias-rejected
		    (string= (buffer-string) "E22-COPY-SELF")
		    (with-temp-buffer
		      (insert-file-contents copy)
		      (string= (buffer-string) "E22-COPY-SELF")))
	       (format "file=%S direct=%S logical=%S contents=%S"
		       file same-file-rejected logical-alias-rejected
		       (buffer-string)))))
	(condition-case nil (define-logical-name logical nil) (error nil))
	(dolist (cleanup (list file copy))
	  (setq versions-left 20)
	  (while (and (> versions-left 0) (file-exists-p cleanup))
	    (setq versions-left (1- versions-left))
	    (condition-case nil
		(delete-file cleanup)
	      (error (setq versions-left 0)))))))
  (error
   (emacs-vms-smoke-record
    'copy-file-self nil (error-message-string err))))

(condition-case err
    (let* ((directory (make-temp-file "E22_ODS5_" t))
	   (leaf (concat (make-string 120 ?a) ".txt"))
	   (file (expand-file-name leaf directory))
	   dired-buffer)
      (unwind-protect
	  (progn
	    (write-region "ODS5" nil file nil 'silent)
	    (setq dired-buffer (dired-noselect directory))
	    (emacs-vms-smoke-record
	     'ods5-long-filename
	     (and (member (upcase leaf)
			  (mapcar 'upcase (directory-files directory)))
		  (with-current-buffer dired-buffer
		    (let ((case-fold-search t))
		      (save-excursion
			(goto-char (point-min))
			(search-forward leaf nil t)))))
	     (format "length=%d file=%S" (length leaf) file)))
	(when (buffer-live-p dired-buffer)
	  (kill-buffer dired-buffer))
	(when (file-exists-p file)
	  (delete-file file))
	(when (file-directory-p directory)
	  (delete-directory directory))))
  (error
   (emacs-vms-smoke-record
    'ods5-long-filename nil (error-message-string err))))

(condition-case err
    (let* ((parent (make-temp-file "E22_ODS5_DIR_" t))
	   ;; A right bracket in an ODS-5 component is written as ^].
	   (child (expand-file-name "literal^]bracket" parent))
	   directory-name directory-file roundtrip)
      (unwind-protect
	  (progn
	    (make-directory child)
	    (setq directory-name (file-name-as-directory child))
	    (setq directory-file (directory-file-name directory-name))
	    (setq roundtrip (file-name-as-directory directory-file))
	    (emacs-vms-smoke-record
	     'ods5-real-bracket-directory
	     (and (file-directory-p child)
		  (string= (upcase directory-name) (upcase roundtrip)))
	     (format "directory=%S file=%S roundtrip=%S"
		     directory-name directory-file roundtrip)))
	(when (file-directory-p child)
	  (delete-directory child))
	(when (file-directory-p parent)
	  (delete-directory parent))))
  (error
   (emacs-vms-smoke-record
    'ods5-real-bracket-directory nil (error-message-string err))))

(condition-case err
    (let ((file (make-temp-file "E22_WRITE_"))
	  (contents (make-string 65536 ?x))
	  (versions-left 20))
      (unwind-protect
	  (progn
	    (write-region contents nil file nil 'silent)
	    (with-temp-buffer
	      (insert-file-contents file)
	      (emacs-vms-smoke-record
	       'long-unterminated-write
	       (string= contents (buffer-string))
	       (format "bytes=%d" (buffer-size)))))
	(while (and (> versions-left 0) (file-exists-p file))
	  (setq versions-left (1- versions-left))
	  (condition-case nil
	      (delete-file file)
	    (error (setq versions-left 0))))))
  (error
   (emacs-vms-smoke-record
    'long-unterminated-write nil (error-message-string err))))

(condition-case err
    (let ((file (make-temp-file "E22_PROTECT_"))
	  (buffer nil)
	  (make-backup-files nil)
	  before after
	  (versions-left 20))
      (unwind-protect
	  (progn
	    (set-file-modes file #o400)
	    (setq before (file-modes file))
	    (setq buffer (find-file-noselect file))
	    (with-current-buffer buffer
	      ;; Visiting a protected file correctly makes its buffer read-only.
	      ;; Override that UI guard so this test reaches the VMS save path.
	      (setq buffer-read-only nil)
	      (insert "PROTECTION")
	      ;; The write-protection confirmation is useful interactively but
	      ;; must not make the batch smoke test wait on its controlling TTY.
	      (let ((original-yes-or-no-p (symbol-function 'yes-or-no-p)))
		(unwind-protect
		    (progn
		      (fset 'yes-or-no-p (lambda (&rest ignored) t))
		      (save-buffer))
		  (fset 'yes-or-no-p original-yes-or-no-p))))
	    (setq after (file-modes file))
	    (emacs-vms-smoke-record
	     'save-restores-protection
	     (and (integerp before) (integerp after) (= before after))
	     (format "before=%S after=%S file=%S" before after file)))
	(when (buffer-live-p buffer)
	  (with-current-buffer buffer (set-buffer-modified-p nil))
	  (kill-buffer buffer))
	(when (file-exists-p file)
	  (condition-case nil (set-file-modes file #o600) (error nil)))
	(while (and (> versions-left 0) (file-exists-p file))
	  (setq versions-left (1- versions-left))
	  (condition-case nil (delete-file file) (error (setq versions-left 0))))))
  (error
   (emacs-vms-smoke-record
    'save-restores-protection nil (error-message-string err))))

(condition-case err
    (let* ((tag (format "E22_CYCLE_%s_" (vms-system-info "PID")))
	   (first (concat tag "A"))
	   (second (concat tag "B"))
	   cycle-detected)
      (unwind-protect
	  (progn
	    (define-logical-name first (concat second ":"))
	    (define-logical-name second (concat first ":"))
	    (setq cycle-detected
		  (condition-case nil
		      (progn
			(vms-expand-search-paths (concat first ":SMOKE.TXT") t)
			nil)
		    (error t)))
	    (emacs-vms-smoke-record 'logical-cycle cycle-detected))
	(condition-case nil (define-logical-name first nil) (error nil))
	(condition-case nil (define-logical-name second nil) (error nil))))
  (error
   (emacs-vms-smoke-record 'logical-cycle nil (error-message-string err))))

(condition-case err
    (let ((tab-rejected
	   (condition-case nil
	       (progn (vms-dcl-file-argument (concat "A" (string ?\t) "B"))
		      nil)
	     (error t)))
	  (trailing-caret-rejected
	   (condition-case nil
	       (progn (vms-dcl-file-argument "SYS$DISK:[DIR]FILE^") nil)
	     (error t))))
      (emacs-vms-smoke-record
	'dcl-file-argument
	(and tab-rejected trailing-caret-rejected
	     (string= (vms-dcl-file-argument
		       "SYS$DISK:[DIR]A B&!/@.TXT;1")
		      "SYS$DISK:[DIR]A^ B^&^!^/^@.TXT;1")
	     (string= (vms-dcl-file-argument
		       "SYS$DISK:[DIR]A^ B.TXT;1")
		      "SYS$DISK:[DIR]A^ B.TXT;1")
	     (string= (vms-dcl-file-argument
		       "SYS$DISK:[5953,9,0]A?.TXT;*")
		      "SYS$DISK:[5953,9,0]A?.TXT;*")
	     (string= (vms-dcl-file-argument
		       "SYS$DISK:[DIR]A,B.TXT;1")
		      "SYS$DISK:[DIR]A^,B.TXT;1")
	     ;; A raw final hyphen is DCL's command-continuation marker.
	     (string= (vms-dcl-file-argument "SYS$DISK:[DIR]FILE-")
		      "SYS$DISK:[DIR]FILE^-"))))
  (error
   (emacs-vms-smoke-record
    'dcl-file-argument nil (error-message-string err))))

(condition-case err
    (let ((original-call-process (symbol-function 'call-process))
	  (original-file-argument
	   (symbol-function 'vms-dcl-file-argument))
	  captured-command
	  captured-temporary
	  expected-command)
      (unwind-protect
	  (progn
	    (fset 'call-process
		  (lambda (&rest arguments)
		    (setq captured-command (car (last arguments)))
		    1))
	    (fset 'vms-dcl-file-argument
		  (lambda (file)
		    (setq captured-temporary file)
		    (funcall original-file-argument file)))
	    (with-temp-buffer
	      (insert "print smoke")
	      (vms-print-region (point-min) (point-max) "PRINT"
				nil nil nil "/QUEUE=SYS$PRINT"))
	    (setq expected-command
		  (concat "PRINT "
			  (funcall original-file-argument captured-temporary)
			  "/NAME=\"GNUprintbuffer\" /QUEUE=SYS$PRINT"))
	    (emacs-vms-smoke-record
	     'print-temporary-file
	     (and captured-temporary
		  (not (file-exists-p captured-temporary))
		  (string= captured-command expected-command))
	     (format "command=%S expected=%S temporary=%S"
		     captured-command expected-command captured-temporary)))
	(fset 'vms-dcl-file-argument original-file-argument)
	(fset 'call-process original-call-process)))
  (error
   (emacs-vms-smoke-record
    'print-temporary-file nil (error-message-string err))))

(condition-case err
    (let ((timer-fired nil)
	  (timer (run-at-time 0.1 nil (lambda () (setq timer-fired t))))
	  input-error)
      (unwind-protect
	  (progn
	    (setq input-error
		  (condition-case nil
		      (progn
			(call-process "*dcl*" "E22_MISSING_INPUT.FILE" nil nil
				      "-c" "show default")
			nil)
		    (error t)))
	    (sit-for 0.5)
	    (emacs-vms-smoke-record
	     'process-error-restores-timers
	     (and input-error timer-fired)
	     (format "input-error=%S timer-fired=%S" input-error timer-fired)))
	(cancel-timer timer)))
  (error
   (emacs-vms-smoke-record
    'process-error-restores-timers nil (error-message-string err))))

(condition-case err
    (with-temp-buffer
      (let ((status (call-process "*dcl*" nil t nil "-c" "show default")))
	(emacs-vms-smoke-record
	 'call-process
	 (and (integerp status) (= (logand status 1) 1) (> (buffer-size) 0))
	 (format "status=%S output=%S" status (buffer-string)))))
  (error
   (emacs-vms-smoke-record 'call-process nil (error-message-string err))))

(condition-case err
    (with-temp-buffer
      (let ((status
	     (call-process
	      "*dcl*" nil t nil "-c"
	      "write sys$output \"E22-RAW-''f$time()'\"")))
	(emacs-vms-smoke-record
	 'raw-dcl-command
	 (and (integerp status) (= (logand status 1) 1)
	      (string-match "\\(?:\\`\\|[\r\n]\\)E22-RAW-"
			    (buffer-string)))
	 (format "status=%S output=%S" status (buffer-string)))))
  (error
   (emacs-vms-smoke-record
    'raw-dcl-command nil (error-message-string err))))

(condition-case err
    (let ((expected-apostrophes
	   "Adjacent apostrophes are unsafe in VMS DCL argument")
	  (expected-control "Unsafe control character in VMS DCL argument")
	  (safe-quoted
	   (string= (vms-dcl-quote-argument "two \"words\"")
		    "\"two \"\"words\"\"\""))
	  (apostrophes-diagnostic
	   (condition-case diagnostic
	       (progn (vms-dcl-quote-argument "bad''symbol'") nil)
	     (error (error-message-string diagnostic))))
	  (control-diagnostic
	   (condition-case diagnostic
	       (progn
		 (vms-dcl-quote-argument
		  (concat "bad" (char-to-string 10) "argument"))
		 nil)
	     (error (error-message-string diagnostic)))))
      (emacs-vms-smoke-record
	'dcl-quoted-argument-validation
	(and safe-quoted
	     (equal apostrophes-diagnostic expected-apostrophes)
	     (equal control-diagnostic expected-control))
	(format "safe=%S apostrophes=%S control=%S"
		safe-quoted apostrophes-diagnostic control-diagnostic)))
  (error
   (emacs-vms-smoke-record
    'dcl-quoted-argument-validation nil (error-message-string err))))

(condition-case err
    (let* ((image (vms-system-info "IMAGE"))
	   (bad-apostrophes "bad''symbol'")
	   (bad-control (concat "bad" (char-to-string 10) "argument"))
	   (expected-apostrophes
	    "Adjacent apostrophes in OpenVMS process argument")
	   (expected-control "Control character in OpenVMS process command")
	   (expected-command "OpenVMS command line too long")
	   (expected-program "Apostrophe in raw OpenVMS program command")
	   apostrophes-diagnostic async-diagnostics control-diagnostic
	   command-diagnostic program-diagnostic)
	(setq apostrophes-diagnostic
	      (condition-case diagnostic
		  (progn
		    (call-process image nil nil nil "-batch" bad-apostrophes)
		    nil)
		(error (error-message-string diagnostic))))
	(setq async-diagnostics nil)
	(let ((attempt 0)
	      (name "e22-invalid-process-argument"))
	  (while (< attempt 2)
	    (let ((diagnostic
		   (condition-case process-error
		       (progn
			 (start-process name nil image "-batch" bad-apostrophes)
			 nil)
		     (error (error-message-string process-error)))))
	      (when (get-process name)
		(delete-process name)
		(setq diagnostic "Rejected start left a live process"))
	      (setq async-diagnostics
		    (cons diagnostic async-diagnostics)))
	    (setq attempt (1+ attempt))))
	(setq async-diagnostics (nreverse async-diagnostics))
	(setq control-diagnostic
	      (condition-case diagnostic
		  (progn
		    (call-process image nil nil nil "-batch" bad-control)
		    nil)
		(error (error-message-string diagnostic))))
	(setq command-diagnostic
	      (condition-case diagnostic
		  (progn
		    (call-process image nil nil nil "-batch"
				  (make-string 220 ?a) (make-string 220 ?b)
				  (make-string 220 ?c) (make-string 220 ?d)
				  (make-string 220 ?e))
		    nil)
		(error (error-message-string diagnostic))))
	(setq program-diagnostic
	      (condition-case diagnostic
		  (progn (call-process "bad'program" nil nil nil) nil)
		(error (error-message-string diagnostic))))
	(emacs-vms-smoke-record
	 'process-argument-validation
	 (and (equal apostrophes-diagnostic expected-apostrophes)
	      (equal async-diagnostics
		     (list expected-apostrophes expected-apostrophes))
	      (equal control-diagnostic expected-control)
	      (equal command-diagnostic expected-command)
	      (equal program-diagnostic expected-program))
	 (format
	  "apostrophes=%S async=%S control=%S command=%S program=%S"
	  apostrophes-diagnostic async-diagnostics control-diagnostic
	  command-diagnostic program-diagnostic)))
  (error
   (emacs-vms-smoke-record
    'process-argument-validation nil (error-message-string err))))

(condition-case err
    (let ((no-wait-status
	   (call-process "*dcl*" nil 0 nil "-c" "wait 00:00:01")))
      ;; The first child used to share the synchronous child's completion
      ;; event flag and status word.  Let it finish while the second child is
      ;; still waiting, then verify that the second child's late output is not
      ;; mistaken for end-of-file.
      (with-temp-buffer
	(let ((status
	       (call-process
		"*dcl*" nil t nil "-c"
		"pipe wait 00:00:02 ; write sys$output \"E22-SYNC-AFTER-NOWAIT\"")))
	  (emacs-vms-smoke-record
	   'call-process-no-wait-isolation
	   (and (null no-wait-status)
		(integerp status) (= (logand status 1) 1)
		(string-match "E22-SYNC-AFTER-NOWAIT" (buffer-string)))
	   (format "no-wait=%S status=%S output=%S"
		   no-wait-status status (buffer-string))))))
  (error
   (emacs-vms-smoke-record
    'call-process-no-wait-isolation nil (error-message-string err))))

(condition-case err
    (let* ((image (vms-system-info "IMAGE"))
	   (image-without-version (file-name-sans-versions image))
	   (map (concat (file-name-sans-extension image-without-version)
			".DUMP")))
      (with-temp-buffer
	(let ((status
	       (call-process image nil t nil
			     "-map" map "-batch" "-q" "-eval"
			     "(princ \"E22 O'Brien\")")))
	  (emacs-vms-smoke-record
	   'versioned-image-and-quoting
	   (and (integerp status) (= (logand status 1) 1)
		(string-match "E22 O'Brien" (buffer-string)))
	   (format "image=%S map=%S status=%S output=%S"
		   image map status (buffer-string))))))
  (error
   (emacs-vms-smoke-record
    'versioned-image-and-quoting nil (error-message-string err))))

(condition-case err
    (let* ((buffer (get-buffer-create "*e22-async*"))
	   (process nil)
	   (marker "E22OK")
	   (deadline nil)
	   (marker-seen nil))
      (with-current-buffer buffer
	(erase-buffer))
      (unwind-protect
	  (progn
	    (setq process (start-process "e22-async" buffer "*dcl*" ""))
	    ;; One call deliberately exceeds the VMS PTY buffer.  The old
	    ;; one-shot send path silently discarded everything after its first
	    ;; partial write, including the marker at the tail.
	    (process-send-string
	     process
	     (concat (mapconcat 'identity (make-list 600 "! E22 PAD") "\n")
		     "\nWRITE SYS$OUTPUT \"E22OK\"\n"))
	    (setq deadline (+ (float-time) 10.0))
	    (while (and (eq (process-status process) 'run)
			(not
			 (setq marker-seen
			       (with-current-buffer buffer
				 (save-excursion
				   (goto-char (point-min))
				   (search-forward marker nil t)))))
			(< (float-time) deadline))
	      (accept-process-output process 0.25))
	    ;; LIB$SPAWN continues reading its input channel after the command
	    ;; string.  EXIT leaves this top-level spawned command interpreter
	    ;; running; LOGOUT terminates the process.
	    (when (eq (process-status process) 'run)
	      (process-send-string process "LOGOUT\n"))
	    (setq deadline (+ (float-time) 10.0))
	    (while (and (eq (process-status process) 'run)
			(< (float-time) deadline))
	      (accept-process-output process 0.25))
	    (let ((status (process-status process)))
	      (emacs-vms-smoke-record
	       'async-process
	       (and marker-seen (eq status 'exit)
		    (zerop (process-exit-status process)))
	       (format "status=%S exit=%S output=%S"
		       status (process-exit-status process)
		       (with-current-buffer buffer (buffer-string))))))
	(when (and process (eq (process-status process) 'run))
	  (condition-case nil
	      (process-send-string process "LOGOUT\n")
	    (error nil))
	  (accept-process-output process 2)
	  (when (eq (process-status process) 'run)
	    (delete-process process)))
	(when process
	  ;; Release the VMS channel and process slot before testing a second
	  ;; asynchronous child in this same Emacs invocation.
	  (condition-case nil
	      (delete-process process)
	    (error nil)))))
  (error
   (emacs-vms-smoke-record 'async-process nil (error-message-string err))))

(condition-case err
    (let* ((buffer nil)
	   (process nil)
	   (marker "E22VMSPROCOK")
	   (deadline nil)
	   (marker-seen nil))
      ;; Exercise the user-facing OpenVMS command-buffer front end as well as
      ;; the process primitives tested above.
      (unwind-protect
	  (progn
	    (setq process (start-subprocess)
		  buffer (process-buffer process))
	    (setq deadline (+ (float-time) 3.0))
	    (while (and (eq (process-status process) 'run)
			(zerop (with-current-buffer buffer (buffer-size)))
			(< (float-time) deadline))
	      (accept-process-output process 0.25))
	    (with-current-buffer buffer
	      (goto-char (point-max))
	      (insert "WRITE SYS$OUTPUT \"E22VMSPROCOK\"")
	      (command-send-input))
	    (setq deadline (+ (float-time) 10.0))
	    (while (and (eq (process-status process) 'run)
			(not
			 (setq marker-seen
			       (with-current-buffer buffer
				 (save-excursion
				   (goto-char (point-min))
				   (search-forward marker nil t)))))
			(< (float-time) deadline))
	      (accept-process-output process 0.25))
	    (when (eq (process-status process) 'run)
	      (process-send-string process "LOGOUT\n"))
	    (setq deadline (+ (float-time) 10.0))
	    (while (and (eq (process-status process) 'run)
			(< (float-time) deadline))
	      (accept-process-output process 0.25))
	    (emacs-vms-smoke-record
	     'vmsproc-helper
	     (and marker-seen (eq (process-status process) 'exit)
		  (zerop (process-exit-status process))
		  (not (vms-subprocess-live-p)))
	     (format "status=%S exit=%S output=%S"
		     (process-status process) (process-exit-status process)
		     (with-current-buffer buffer (buffer-string)))))
	(when (and process (eq (process-status process) 'run))
	  (condition-case nil
	      (process-send-string process "LOGOUT\n")
	    (error nil))
	  (accept-process-output process 2)
	  (when (eq (process-status process) 'run)
	    (delete-process process)))
	(when process
	  (condition-case nil
	      (delete-process process)
	    (error nil)))
	(when (buffer-live-p buffer)
	  (kill-buffer buffer))))
  (error
   (emacs-vms-smoke-record 'vmsproc-helper nil (error-message-string err))))

(if emacs-vms-smoke-failures
    (progn
      (princ (format "E22-SMOKE-FAILURES=%S\n"
		     (nreverse emacs-vms-smoke-failures)))
      (kill-emacs 1))
  (princ "E22-SMOKE=OK\n")
  (kill-emacs 0))

;;; smoke-test.el ends here
