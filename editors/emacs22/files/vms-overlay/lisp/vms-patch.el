;;; vms-patch.el --- override parts of files.el for VMS

;; Copyright (C) 1986, 1992, 2001, 2002, 2003, 2004,
;;   2005, 2006, 2007, 2008 Free Software Foundation, Inc.

;; Maintainer: FSF
;; Keywords: vms

;; This file is part of GNU Emacs.

;; GNU Emacs is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3, or (at your option)
;; any later version.

;; GNU Emacs is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU Emacs; see the file COPYING.  If not, write to the
;; Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
;; Boston, MA 02110-1301, USA.

;;; Commentary:

;;; Code:

(add-to-list 'auto-mode-alist '("\\.com\\'" . dcl-mode))

;;; Functions that need redefinition

;;; VMS file names are upper case, but buffer names are more
;;; convenient in lower case.

(defun create-file-buffer (filename)
  "Create a suitably named buffer for visiting FILENAME, and return it.
FILENAME (sans directory) is used unchanged if that name is free;
otherwise a string <2> or <3> or ... is appended to get an unused name."
  (generate-new-buffer (downcase (file-name-nondirectory filename))))

;;; Given a string FN, return a similar name which is a legal VMS filename.
;;; This is used to avoid invalid auto save file names.
(defun make-legal-file-name (fn)
  (setq fn (copy-sequence fn))
  (let ((dot nil) (indx 0) (len (length fn)) chr)
    (while (< indx len)
      (setq chr (aref fn indx))
      (cond
       ((eq chr ?.) (if dot (aset fn indx ?_) (setq dot t)))
       ((not (or (and (>= chr ?a) (<= chr ?z)) (and (>= chr ?A) (<= chr ?Z))
		 (and (>= chr ?0) (<= chr ?9))
		 (eq chr ?$) (eq chr ?_) (and (eq chr ?-) (> indx 0))))
	(aset fn indx ?_)))
      (setq indx (1+ indx))))
  fn)

;;; Auto save filesnames start with _$ and end with $.

(defun make-auto-save-file-name ()
  "Return file name to use for auto-saves of current buffer.
This function does not consider `auto-save-visited-file-name';
the caller should check that before calling this function.
This is a separate function so that your `.emacs' file or the site's
`site-init.el' can redefine it.
See also `auto-save-file-name-p'."
  (if buffer-file-name
      (let ((name (file-name-sans-versions buffer-file-name)))
	(concat (file-name-directory name)
		"_$"
		(file-name-nondirectory name)
		"$"))
    (expand-file-name (concat "_$_" (make-legal-file-name (buffer-name)) "$"))))

(defun auto-save-file-name-p (filename)
  "Return t if FILENAME can be yielded by `make-auto-save-file-name'.
FILENAME should lack slashes.
This is a separate function so that your `.emacs' file or the site's
`site-init.el' can redefine it."
  (string-match "^_\\$.*\\$\\'" filename))

;;;
;;; This goes along with kepteditor.com which defines these logicals.
;;; Indexed EMACS_COMMAND_ARG_n logicals preserve argument boundaries and
;;; case.  EMACS_COMMAND_ARGS remains supported for older command files.
;;; If command arguments are defined, they supersede EMACS_FILE_NAME,
;;; which is probably set up incorrectly anyway.
;;; The function command-line-again is a kludge, but it does the job.
;;;
(defun vms-kept-editor-command-line ()
  "Return kept-editor command data as (DIRECTORY . ARGUMENTS), or nil.
New versions of kepteditor.com define one logical name per argument.  Fall
back to the historical, space-separated EMACS_COMMAND_ARGS logical name so
an updated Emacs can still attach when invoked by an older command file."
  (let ((count-text
	 (vms-system-info "LOGICAL" "EMACS_COMMAND_ARG_COUNT")))
    (if (and (stringp count-text) (> (length count-text) 0))
	(progn
	  (unless (string-match "\\`[0-9]+\\'" count-text)
	    (error "Invalid EMACS_COMMAND_ARG_COUNT value %S" count-text))
	  (let ((count (string-to-number count-text))
		(index 1)
		arguments
		directory)
	    (while (<= index count)
	      (let ((argument
		     (vms-system-info
		      "LOGICAL"
		      (concat "EMACS_COMMAND_ARG_" (number-to-string index)))))
		(unless (stringp argument)
		  (error "Missing kept-editor argument %d" index))
		(setq arguments (cons argument arguments)))
	      (setq index (1+ index)))
	    (setq directory
		  (vms-system-info "LOGICAL" "EMACS_COMMAND_DIRECTORY"))
	    (unless (and (stringp directory) (> (length directory) 0))
	      (error "Missing kept-editor command directory"))
	    (cons directory (nreverse arguments))))
      (let ((legacy (vms-system-info "LOGICAL" "EMACS_COMMAND_ARGS")))
	(when (and (stringp legacy) (> (length legacy) 0))
	  ;; Replace control and non-ASCII bytes with spaces before parsing.
	  (setq legacy (copy-sequence legacy))
	  (let ((index 0))
	    (while (< index (length legacy))
	      (let ((character (aref legacy index)))
		(when (or (< character 33) (> character 126))
		  (aset legacy index ?\s)))
	      (setq index (1+ index))))
	  (let ((arguments (split-string-and-unquote legacy)))
	    (and arguments (cons (car arguments) (cdr arguments)))))))))

(defun vms-suspend-resume-hook ()
  "Process kept-editor arguments when resuming a suspended Emacs.
If there are no command arguments and `EMACS_FILE_NAME' is defined,
`find-file' that file."

  (let ((file (vms-system-info "LOGICAL" "EMACS_FILE_NAME"))
	(command-data (vms-kept-editor-command-line))
	(line (vms-system-info "LOGICAL" "EMACS_FILE_LINE")))

    (if (not command-data)
	(if file
	    (progn (find-file file)
		   (if line (goto-line (string-to-number line)))))
      (vms-command-line-again command-data))))

(setq suspend-resume-hook 'vms-suspend-resume-hook)

(defun vms-suspend-hook ()
  "Don't allow suspending if logical name `DONT_SUSPEND_EMACS' is defined."
  (if (vms-system-info "LOGICAL" "DONT_SUSPEND_EMACS")
      (error "Can't suspend this emacs"))
  nil)

(setq suspend-hook 'vms-suspend-hook)

;;;
;;; A kludge that allows reprocessing of the command line.  This is mostly
;;;   to allow a spawned VMS mail process to do something reasonable when
;;;   used in conjunction with the modifications to sysdep.c that allow
;;;   Emacs to attach to a "foster" parent.
;;;
(defun vms-command-line-normalize-option (argument)
  "Downcase the option part of ARGUMENT without changing its value.
Kept-editor file names and option values retain their original case.
The case-sensitive Emacs switches `-D', `-L', `-Q', and `-T' must also
retain their case; in particular, `-L' and `-l' have different meanings."
  (if (and (> (length argument) 0)
	   (eq (aref argument 0) ?-)
	   (not (member argument '("-D" "-L" "-Q" "-T"))))
      (let ((equals (string-match "=" argument)))
	(if equals
	    (concat (downcase (substring argument 0 equals))
		    (substring argument equals))
	  (downcase argument)))
    argument))

(defun vms-command-line-normalize-arguments (arguments)
  "Normalize options in ARGUMENTS while preserving operands after `--'."
  (let (normalized just-files)
    (while arguments
      (let ((argument (car arguments)))
	(setq normalized
	      (cons (if just-files
			argument
		      (vms-command-line-normalize-option argument))
		    normalized))
	(when (string= argument "--")
	  (setq just-files t)))
      (setq arguments (cdr arguments)))
    (nreverse normalized)))

(defun vms-command-line-again (&optional command-data)
  "Reprocess command line arguments.  VMS specific.
Command line arguments come from logical names defined by kepteditor.com.
On VMS this allows attaching to a spawned Emacs and doing things like
\"emacs -l MyFile.el -f doit\" without changing the case of file names."
  (let* ((data (or command-data (vms-kept-editor-command-line)))
	 (directory (car-safe data))
	 (arguments (cdr-safe data)))
    (when arguments
      (when (and (stringp directory) (> (length directory) 0))
	(cd directory)
	(setq command-line-default-directory default-directory))
      (let ((command-line-args
	     (cons "emacs"
		   (vms-command-line-normalize-arguments arguments))))
	(command-line)))))

(defun vms-read-directory (dirname switches buffer)
  (let ((status
	 (subprocess-command-to-buffer
	  (concat "DIRECTORY " switches " "
		  (vms-dcl-file-argument dirname))
	  buffer)))
    (unless (and (integerp status) (= (logand status 1) 1))
      (error "VMS DIRECTORY failed for %s with status %S" dirname status))
    (with-current-buffer buffer
      (goto-char (point-min))
      ;; Remove all the trailing blanks.
      (while (search-forward " \n" nil t)
	(forward-char -1)
	(delete-horizontal-space))
      (goto-char (point-min)))))

(setq dired-listing-switches
      "/SIZE/DATE/OWNER/WIDTH=(DISPLAY=512,FILENAME=255,SIZE=12,OWNER=32)")

(defun vms-dcl-quote-argument (string)
  "Quote STRING as one DCL command argument.
Signal an error when DCL cannot represent STRING literally and safely."
  (let ((index 0)
	(previous-apostrophe nil)
	(length (length string)))
    (when (> length 255)
      (error "VMS DCL command element exceeds 255 characters"))
    (while (< index length)
      (let ((character (aref string index)))
	(when (or (< character 32) (= character 127))
	  (error "Unsafe control character in VMS DCL argument"))
	(when (and (= character 39) previous-apostrophe)
	  (error "Adjacent apostrophes are unsafe in VMS DCL argument"))
	(setq previous-apostrophe (= character 39)))
      (setq index (1+ index)))
    (concat "\""
	    (replace-regexp-in-string "\"" "\"\"" string t t)
	    "\"")))

(defun vms-dcl-file-argument (file)
  "Return native FILE suitably escaped for use as one DCL argument.
Do not put double quotes around a complete native file specification.
Commands such as DIRECTORY interpret those quotes as an ODS-5 quoted file
name component, not as shell-style argument quotes.  Preserve existing RMS
circumflex escapes and add one before characters that DCL could otherwise
tokenize."
  (let ((index 0)
	(carets 0)
	(delimiter-depth 0)
	(length (length file))
	(result ""))
    (while (< index length)
      (let ((character (aref file index)))
	(when (or (< character 32)
		  (= character 127))
	  (error "Unsafe control character in VMS file name"))
	(if (= character ?^)
	    (setq carets (1+ carets))
	  (when (and (= (% carets 2) 0)
		     (or (and (= character ?-)
			      (= index (1- length)))
			 (not (or (and (>= character ?A) (<= character ?Z))
				  (and (>= character ?a) (<= character ?z))
				  (and (>= character ?0) (<= character ?9))
				  (>= character 128)
				  (and (= character ?,) (> delimiter-depth 0))
				  (memq character
					'(?$ ?_ ?: ?\[ ?\] ?< ?> ?. ?\; ?- ?* ?% ??))))))
	    (setq result (concat result "^")))
	  (when (= (% carets 2) 0)
	    (cond
	     ((memq character '(?\[ ?<))
	      (setq delimiter-depth (1+ delimiter-depth)))
	     ((and (> delimiter-depth 0) (memq character '(?\] ?>)))
	      (setq delimiter-depth (1- delimiter-depth)))))
	  (setq carets 0))
	(setq result (concat result (char-to-string character))))
      (setq index (1+ index)))
    (when (= (% carets 2) 1)
      (error "Unpaired circumflex escape in VMS file name"))
    result))

(defun vms-print-region (start end command delete-text buffer display
			       &rest switches)
  "Print a region through COMMAND using a private temporary VMS file."
  (let ((temporary (make-temp-file "EMACS_PRINT_"))
	status)
    (unwind-protect
	(progn
	  ;; `make-temp-file' has already created this exact version.  Writing
	  ;; with APPEND at offset zero reuses it; a normal VMS write would make
	  ;; another version and leave the empty placeholder behind at cleanup.
	  (write-region start end temporary 0 'silent)
	  (setq status
		(call-process
		 "*dcl*" nil nil nil "-c"
		 (mapconcat
		  'identity
		  (append
			   (list command
				 (concat (vms-dcl-file-argument temporary)
					 "/NAME=\"GNUprintbuffer\""))
		   switches)
		  " ")))
	  (unless (and (integerp status) (= (logand status 1) 1))
	    (error "VMS print command failed with status %S" status))
	  (when delete-text
	    (delete-region start end))
	  status)
      (when (file-exists-p temporary)
	(delete-file temporary)))))

(setq print-region-function 'vms-print-region)

;;;
;;; Fuctions for using Emacs as a VMS Mail editor
;;;
(autoload 'vms-pmail-setup "vms-pmail"
  "Set up file assuming use by VMS Mail utility.
The buffer is put into text-mode, auto-save is turned off and the
following bindings are established.

\\[vms-pmail-save-and-exit]	vms-pmail-save-and-exit
\\[vms-pmail-abort]	vms-pmail-abort

All other Emacs commands are still available."
  t)

;;;
;;; Filename handling in the minibuffer
;;;
(defun vms-magic-right-square-brace ()
  "\
Insert a right square brace, but do other things first depending on context.
During filename completion, when point is at the end of the line and the
character before is not a right square brace, do one of three things before
inserting the brace:
 - If there are already two left square braces preceding, do nothing special.
 - If there is a previous right-square-brace, convert it to dot.
 - If the character before is dot, delete it.
Additionally, if the preceding chars are right-square-brace followed by
either \"-\" or \"..\", strip one level of directory hierarchy."
  (interactive)
  (when (and minibuffer-completing-file-name
	     (= (point) (point-max))
	     (not (eq ?\] (char-before))))
    (cond
     ;; Avoid clobbering: user:[one.path][another.path
     ((search-backward "[" (field-beginning) t 2))
     ((search-backward "]" (field-beginning) t)
      (delete-char 1)
      (insert ".")
      (goto-char (point-max)))
     ((eq ?. (char-before))
      (delete-char -1)))
    (goto-char (point-max))
    (let ((specs '(".." "-"))
	  (pmax (point-max)))
      (while specs
	(let* ((up (car specs))
	       (len (length up))
	       (cut (- (point) len)))
	  (when (and (> cut (field-beginning))
		     (< (1+ len) pmax)
		     (eq ?. (char-before cut))
		     (string= up (buffer-substring cut (point))))
	    (delete-char (- (1+ len)))
	    (while (and (> (point) (field-beginning))
			(not (memq (char-before) '(?. ?\[))))
	      (delete-char -1))
	    (when (eq ?. (char-before)) (delete-char -1))
	    (setq specs nil)))
	(setq specs (cdr specs)))))
  (insert "]"))

(defun vms-magic-colon ()
  "\
Insert a colon, but do other things first depending on context.
During filename completion, when point is at the end of the line
and the line contains a right square brace, remove all characters
from the beginning of the line up to and including such brace.
This enables one to type a new filespec without having to delete
the old one."
  (interactive)
  (when (and minibuffer-completing-file-name
	     (= (point) (point-max))
             (search-backward "]" (field-beginning) t))
    (delete-region (field-beginning) (1+ (point)))
    (goto-char (point-max)))
  (insert ":"))

(let ((m minibuffer-local-completion-map))
  (define-key m "]" 'vms-magic-right-square-brace)
  (define-key m "/" 'vms-magic-right-square-brace)
  (define-key m ":" 'vms-magic-colon))

;;; arch-tag: c178494e-2c37-4d02-99b7-e47e615656cf
;;; vms-patch.el ends here
