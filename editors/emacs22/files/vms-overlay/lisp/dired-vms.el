;;; dired-vms.el --- VMS functions and variables for direr.

;; Copyright (C) 1993 Free Software Foundation, Inc.

;; Author: Richard Levitte <levitte@e.kth.se>
;; Version: 1

;; This file is part of GNU Emacs.

;; GNU Emacs is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.

;; GNU Emacs is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU Emacs; see the file COPYING.  If not, write to
;; the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

;;; Commentary:

;; This is a redefinition of some of the functions and variables in
;; dired.el.  They will be documented in the Emacs manual.

;;; Code:

(defconst dired-vms-filename-regexp
  "\\([^\r\n]+;[0-9]+\\)"
  "Regular expression matching an ODS-2 or ODS-5 filename in Dired output.")

(defconst dired-vms-show-all-version t
  "When non-nil, the dired buffer will show all versions of all files.
When nil, only the highest version will be shown.")

;; Read in a new dired buffer

(defun dired-insert-headerline (dir));; DIR already puts on header

;; Reverting a dired buffer


;; dired mode key bindings and initialization


;; Make menu bar items.


;; Idiosyncratic dired commands that don't deal with marks.

(defun dired-next-dirline (arg &optional opoint)
  "Goto ARG'th next directory file line."
  (interactive "p")
  (or opoint (setq opoint (point)))
  (let ((case-fold-search t))
    (end-of-line)
    (if (if (> arg 0)
	    (search-forward ".DIR;" nil t arg)
	  (beginning-of-line)
	  (search-backward ".DIR;" nil t (- arg)))
	(dired-move-to-filename)	; user may type `i' or `f'
      (goto-char opoint)
      (error "No more subdirectories"))))

;;(defun dired-up-directory ()
;;  "Run dired on parent directory of current directory.
;;Find the parent directory either in this buffer or another buffer.
;;Creates a buffer if necessary."
;;  (interactive)
;;  (setq dir default-directory)
;;  (dired (concat (substring dir 0 (1- (length dir))) ".-]")))

(defun dired-find-file-subcommand (func)
  ;; Patched 6/22/94 by snyder@ricks.enet.dec.com to add sans-version BREAKS if not newest version
  (let* ((fn (dired-get-filename))
	 (sans-version (file-name-sans-versions fn))
	 (fn2 (file-name-nondirectory sans-version))
	 (dr2 (file-name-directory fn))
	 (versions (and dr2
			(condition-case nil
			    (file-name-all-versions fn2 dr2)
			  (file-error nil))))
	 (newest (car (last versions))))
    (if (or (not (equal system-type 'vax-vms))
	    (string= fn sans-version)
	    (and (stringp newest)
		 (string= (upcase (file-name-nondirectory fn))
			  (upcase newest))))
	(funcall func (file-name-sans-versions fn t))
      (progn
	(message (if newest
		     "Warning: Editing old version of file."
		   "Warning: Could not determine the newest file version."))
	(funcall func fn)))))

(defun dired-find-file ()
  "In dired, visit the file or directory named on this line.  If the file
represents the most recent VMS version of the file, the version number is
ignored."
  (interactive)
  (dired-find-file-subcommand 'find-file))

(defun dired-find-file-other-window ()
  "In dired, visit the file or directory named on this line.  If the file
represents the most recent VMS version of the file, the version number is
ignored."
  (interactive)
  (dired-find-file-subcommand 'find-file-other-window))

;;; Functions for extracting and manipulating file names in dired buffers.

(defun dired-get-filename (&optional localp no-error-if-not-filep)
  "In dired, return name of file mentioned on this line.
Value returned normally includes the directory name.
Optional arg LOCALP with value `no-dir' means don't include directory
name in result.  A value of `verbatim' means to return the name exactly as
it occurs in the buffer, and a value of t means construct name relative to
`default-directory', which still may contain slashes if in a subdirectory.
Optional arg NO-ERROR-IF-NOT-FILEP means treat `.' and `..' as
regular filenames and return nil if no filename on this line.
Otherwise, an error occurs in these cases."
  (let (case-fold-search file p1 p2 already-absolute)
    (save-excursion
      (if (setq p1 (dired-move-to-filename (not no-error-if-not-filep)))
	  (setq p2 (dired-move-to-end-of-filename no-error-if-not-filep))))
    ;; nil if no file on this line, but no-error-if-not-filep is t:
    (if (setq file (and p1 p2 (buffer-substring p1 p2)))
	(progn
	  ;; Get rid of the mouse-face property that file names have.
	  (set-text-properties 0 (length file) nil file)
	  ;; Unquote names quoted by directory listing code.  This is the
	  ;; same decoding performed by the Emacs 22 implementation in
	  ;; dired.el and preserves control and non-ASCII characters.
	  (setq file
		(read
		 (concat "\""
			 (or (dired-string-replace-match
			      "\\([^\\]\\|\\`\\)\"" file "\\1\\\\\"" nil t)
			     file)
			 "\"")))
	  (if (and enable-multibyte-characters
		   (not (multibyte-string-p file)))
	      (setq file (string-to-multibyte file)))))
    (and file (file-name-absolute-p file)
	 ;; A relative file name can start with ~.
	 (not (eq (aref file 0) ?~))
	 (setq already-absolute t))
    (cond
     ((null file)
      nil)
     ((eq localp 'verbatim)
      file)
     ((and (not no-error-if-not-filep)
	   (member file '("." "..")))
      (error "Cannot operate on `.' or `..'"))
     ((and (eq localp 'no-dir) already-absolute)
      (file-name-nondirectory file))
     (already-absolute
      (let ((handler (find-file-name-handler file nil)))
	(if (and handler (not (get handler 'safe-magic)))
	    (concat "/:" file)
	  file)))
     ((eq localp 'no-dir)
      file)
     ((equal (dired-current-directory) "/")
      (setq file (concat (dired-current-directory localp) file))
      (let ((handler (find-file-name-handler file nil)))
	(if (and handler (not (get handler 'safe-magic)))
	    (concat "/:" file)
	  file)))
     (t
      (concat (dired-current-directory localp) file)))))

(defun dired-make-absolute (file &optional dir)
  "Convert FILE (a pathname relative to DIR) to an absolute pathname."
  ;; This uses a dirty trick you can make with file-name-as-directory...
  (if (string-match "\\[" file)
      (concat
       (file-name-as-directory (concat
				(or dir default-directory)
				(substring (file-name-directory file)
					   (1+ (match-beginning 0))
					   -1))))
    (concat (or dir default-directory) file)))

(defun dired-make-relative (file &optional dir no-error)
  ;;"Convert FILE (an absolute pathname) to a pathname relative to DIR.
  ;; Else error (unless NO-ERROR is non-nil, then FILE is returned unchanged)
  ;;DIR defaults to default-directory."
  ;; DIR must be file-name-as-directory, as with all directory args in
  ;; Emacs Lisp code.
  (setq dir (expand-file-name (or dir default-directory)))
  (if (or (string-match "\\[000000.000000\\(\\]\\)$" dir)
	  (string-match "\\[\\(000000\\]\\)$" dir)
	  (string-match "\\(\\]\\)$" dir))
      (if (string-match (concat "^"
				(regexp-quote (substring dir
							 0
							 (match-beginning 1))))
			file)
	  (progn
	    (setq file (concat "[" (substring file (match-end 0))))
	    (if (string-match "^\\[\\]" file)
		(substring file (match-end 0))
	      file)))
    (if no-error
	file
      (error "%s: not in directory tree growing at %s" file dir))))

;;; Functions for finding the file name in a dired buffer line.

;; Move to first char of filename on this line.
;; Returns position (point) or nil if no filename on this line.
(defun dired-move-to-filename (&optional raise-error eol)
  (or eol
      (setq eol (progn (end-of-line) (point))))
  (beginning-of-line)
  (if (and (<= (+ (point) 2) eol)
	   (progn (forward-char 2)
		  (looking-at dired-vms-filename-regexp)))
      (point)
    (if raise-error
	(error "No file on this line")
      nil)))

(defun dired-move-to-end-of-filename (&optional no-error)
  (let ((eol (progn (end-of-line) (point))))
    (beginning-of-line)
    (if (and (<= (+ (point) 2) eol)
	     (progn (forward-char 2)
		    (looking-at dired-vms-filename-regexp)))
	(match-end 1)
      (if no-error
	  nil
	(error "No file on this line")))))

;; Keeping Dired buffers in sync with the filesystem and with each other


;; Tree Dired

;;; utility functions

(defun dired-in-this-tree (file dir)
  ;;"Is FILE part of the directory tree starting at DIR?"
  (let ((case-fold-search t))
    (if (string-match "\\(\\]\\|>\\)$" dir)
	(string-match (concat "^"
			      (regexp-quote (substring dir
						       0
						       (match-beginning 0))))
		      (directory-file-name file)))))

;; This is basically a copy of the one in dired.el, with a few tweaks. --ttn
(defun dired-build-subdir-alist (&optional switches)
  "Build `dired-subdir-alist' by parsing the buffer.
Returns the new value of the alist.
If optional arg SWITCHES is non-nil, use its value
instead of `dired-actual-switches'."
  (dired-clear-alist)
  (save-excursion
    (let* ((count 0)
	   (buffer-read-only nil)
	   (switches (or switches dired-actual-switches))
	   new-dir-name
	   (R-ftp-base-dir-regex
	    ;; Used to expand subdirectory names correctly in recursive
	    ;; ange-ftp listings.  Disabled for VMS.
	    (and 
		 nil			; ttn
		 (string-match "R" switches)
		 (string-match "\\`/.*:\\(/.*\\)" default-directory)
		 (concat "\\`" (match-string 1 default-directory)))))
      (goto-char (point-min))
      (setq dired-subdir-alist nil)
      (while (and (re-search-forward
		   ;; dired-subdir-regexp	; ttn
		   "Directory \\(.*\\[.*\\]\\)\\(.*\\)[\n]"
		   nil t)
;;		  ;; Avoid taking a file name ending in a colon
;;		  ;; as a subdir name.
;;		  (not (save-excursion
;;			 (goto-char (match-beginning 0))
;;			 (beginning-of-line)
;;			 (forward-char 2)
;;			 (save-match-data (looking-at dired-re-perms))))
		  )
	(save-excursion
	  (goto-char (match-beginning 1))
	  (setq new-dir-name
		(buffer-substring-no-properties (point) (match-end 1))
		new-dir-name
		(save-match-data
		  (if (and R-ftp-base-dir-regex
			   (not (string= new-dir-name default-directory))
			   (string-match R-ftp-base-dir-regex new-dir-name))
		      (concat default-directory
			      (substring new-dir-name (match-end 0)))
		    (expand-file-name new-dir-name))))
	  (delete-region (point) (match-end 1))
	  (insert new-dir-name))
	(setq count (1+ count))
	(dired-alist-add-1 new-dir-name
			   ;; Place a sub directory boundary between lines.
			   (save-excursion
			     (goto-char (match-beginning 0))
			     (beginning-of-line)
			     (point-marker))))
      (if (> count 1)
	  (message "Buffer includes %d directories" count))
      ;; We don't need to sort it because it is in buffer order per
      ;; constructionem.  Return new alist:
      dired-subdir-alist)))


;; These are hooks which make tree dired work.
;; They are in this file because other parts of dired need to call them.
;; But they don't call the rest of tree dired unless there are subdirs loaded.

;;(defun dired-current-directory (&optional localp)
;;  "Return the name of the subdirectory to which this line belongs.
;;This returns a string with trailing slash, like `default-directory'.
;;Optional argument means return a file name relative to `default-directory'."
;;  (let* ((dir default-directory))))


;;(defun dired-goto-file (file)
;;  "Go to file line of FILE in this dired buffer."
;;  ;; Return value of point on success, else nil.
;;  ;; FILE must be an absolute pathname.
;;  ;; Loses if FILE contains control chars like "\007" for which ls
;;  ;; either inserts "?" or "\\007" into the buffer, so we won't find
;;  ;; it in the buffer.
;;  (interactive
;;   (prog1				; let push-mark display its message
;;       (list (expand-file-name
;;	      (read-file-name "Goto file: "
;;			      (dired-current-directory))))
;;     (push-mark)))
;;  (setq file (directory-file-name file)) ; does no harm if no directory
;;  (let (found case-fold-search dir)
;;    (setq dir (or (file-name-directory file)
;;		  (error "Need absolute pathname for %s" file)))
;;    (save-excursion
;;	  (goto-char (point-min))
;;	    (while (and (and (< (point) (point-max)) (not found)))
;;			;; filenames are preceded by SPC, this makes
;;			;; the search faster (e.g. for the filename "-"!).
;; 	      (if (search-forward file (end-of-line) t)
;;		  (setq found (dired-move-to-filename))
;;		;; If this isn't the right line, move forward to avoid
;;		;; trying this line again.
;;		(forward-line 1)))
;;    (and found
;;	 ;; return value of point (i.e., FOUND):
;;	 (goto-char found)))))

;; Deleting files


;; Confirmation


;; Commands to mark or flag file(s) at or near current line.

;; Need to avoid empty lines or continuation line in DIR output
(defun dired-mark (arg)
  "Mark the current (or next ARG) files.
If on a subdir headerline, mark all its files except `.' and `..'.

Use \\[dired-unmark-all-files] to remove all marks
and \\[dired-unmark] on a subdir to remove the marks in
this subdir."
  (interactive "P")
  (if (and (cdr dired-subdir-alist) (dired-get-subdir))
      (save-excursion (dired-mark-subdir-files))
    (let (buffer-read-only)
      (dired-repeat-over-lines
       (prefix-numeric-value arg)
       (function (lambda () (if (not (or (equal ?\011 (following-char))
			          (equal ?\012 (following-char))))
		(progn (delete-char 1) (insert dired-marker-char)))))))))

;;; Commands to mark or flag files based on their characteristics or names.

(defun dired-flag-auto-save-files (&optional unflag-p)
  "Flag for deletion files whose names suggest they are auto save files.
A prefix argument says to unflag those files instead."
  (interactive "P")
  (let ((dired-marker-char (if unflag-p ?\040 dired-del-marker)))
    (dired-mark-if
     (let ((fn (dired-get-filename t t)))
       (if fn
	   (auto-save-file-name-p (file-name-nondirectory fn))))
			      
     "auto save file")))

;; Logging failures operating on files, and showing the results.


;;; Sorting

(defconst dired-vms-date-regexp
  (concat "[0-9] +"
          "\\([0-9]+\\)-\\([A-Z][A-Z][A-Z]\\)-\\([0-9]+\\)"
          " "
          "\\([0-9]+\\):\\([0-9]+\\):\\([0-9]+\\)")
  "Regexp used by `dired-sort-toggle-or-edit' to recognize dates.")
          
(defun dired-sort-toggle-or-edit (&optional arg)
  "Toggle between sort by date/name and refresh the dired buffer.
The prefix arg is ignored under VMS for the time being."
  (interactive "P")
  (let ((buffer-read-only nil)
        order count p mtime key)
    (unless (setq order (get-text-property (point-min) :dired-order))
      (goto-char (point-min))
      (search-forward "Directory")
      (forward-line 1)
      (setq count 0 p (point))
      (while (re-search-forward dired-vms-date-regexp (point-max) t)
        (setq mtime (encode-time
                     (string-to-number (match-string 6))
                     (string-to-number (match-string 5))
                     (string-to-number (match-string 4))
                     (string-to-number (match-string 1))
                     (cadr (member (match-string 2)
                                   '("JAN" 1 "FEB" 2 "MAR" 3
                                     "APR" 4 "MAY" 5 "JUN" 6
                                     "JUL" 7 "AUG" 8 "SEP" 9
                                     "OCT" 10 "NOV" 11 "DEC" 12)))
                     (string-to-number (match-string 3))
                     t))
        (forward-line 1)
        (add-text-properties
         p (point) (list :dired-vms-lexical (format "%03X" count)
                         :dired-vms-date (mapconcat (lambda (n)
                                                      (format "%05X" n))
                                                    mtime "")))
        (setq count (1+ count) p (point)))
      (put-text-property (point-min) (point-max)
                         :dired-order :dired-vms-lexical))
    (goto-char (point-min))
    (search-forward "Directory")
    (forward-line 1)
    (setq order (if (or (not order) (eq :dired-vms-lexical order))
                    :dired-vms-date
                  :dired-vms-lexical)
          key nil)
    (sort-subr (eq order :dired-vms-date)
               (lambda ()
                 (unless (setq key (get-text-property (point) order))
                   (goto-char (point-max))))
               (lambda ()
                 (forward-line 1))
               (lambda ()
                 (or key (get-text-property (point) :dired-vms-date))))
    (put-text-property (point-min) (point-max) :dired-order order)
    (goto-char (point-min))
    (re-search-forward "Directory.*\n[ ]+")
    (dired-sort-set-modeline)))

(defun dired-sort-set-modeline ()
  ;; Set modeline display according to VMS-specific text properties.
  ;; Modeline display of "by name" or "by date" guarantees the user a
  ;; match with the corresponding regexps.  Non-matching switches are
  ;; shown literally.
  (setq mode-name
        (let ((order (get-text-property (point-min) :dired-order)))
          (cond ((or (not order) (eq :dired-vms-lexical order))
                 "Dired by name")
                ((eq :dired-vms-date order)
                 "Dired by date")
                (t
                 (concat "Dired " dired-actual-switches)))))
  (force-mode-line-update))

;;; AUX
;;(defun dired-clean-directory (keep)
;;  "Flag numerical backups for deletion.
;;Spares `dired-kept-versions' latest versions, and `kept-old-versions' oldest.
;;Positive prefix arg KEEP overrides `dired-kept-versions';
;;Negative prefix arg KEEP overrides `kept-old-versions' with KEEP made positive.
;;To clear the flags on these files, you can use \\[dired-flag-backup-files]
;;with a prefix argument."
;;  (interactive "P")
;;    (message "Cleaning backups not implemented."))

;; After rename/copy, reread directories that might be changed 
;;(defun dired-do-create-files (op-symbol file-creator operation arg
;;					     &optional marker-char op1
;;					     how-to)
;;  ;; Create a new file for each marked file.
;;  ;; Prompts user for target, which is a directory in which to create
;;  ;;   the new files.  Target may be a plain file if only one marked
;;  ;;   file exists.
;;  ;; OP-SYMBOL is the symbol for the operation.  Function `dired-mark-pop-up'
;;  ;;   will determine whether pop-ups are appropriate for this OP-SYMBOL.
;;  ;; FILE-CREATOR and OPERATION as in dired-create-files.
;;  ;; ARG as in dired-get-marked-files.
;;  ;; Optional arg OP1 is an alternate form for OPERATION if there is
;;  ;;   only one file.
;;  ;; Optional arg MARKER-CHAR as in dired-create-files.
;;  ;; Optional arg HOW-TO determines how to treat target:
;;  ;;   If HOW-TO is not given (or nil), and target is a directory, the
;;  ;;     file(s) are created inside the target directory.  If target
;;  ;;     is not a directory, there must be exactly one marked file,
;;  ;;     else error.
;;  ;;   If HOW-TO is t, then target is not modified.  There must be
;;  ;;     exactly one marked file, else error.
;;  ;; Else HOW-TO is assumed to be a function of one argument, target,
;;  ;;     that looks at target and returns a value for the into-dir
;;  ;;     variable.  The function dired-into-dir-with-symlinks is provided
;;  ;;     for the case (common when creating symlinks) that symbolic
;;  ;;     links to directories are not to be considered as directories
;;  ;;     (as file-directory-p would if HOW-TO had been nil).
;;  (or op1 (setq op1 operation))
;;  (let* ((fn-list (dired-get-marked-files nil arg))
;;	 (fn-count (length fn-list))
;;	 (target (expand-file-name
;;		   (dired-mark-read-file-name
;;		    (concat (if (= 1 fn-count) op1 operation) " %s to: ")
;;		    (dired-dwim-target-directory)
;;		    op-symbol arg (mapcar (function dired-make-relative) fn-list))))
;;	 (into-dir (cond ((null how-to) (file-directory-p target))
;;			 ((eq how-to t) nil)
;;			 (t (funcall how-to target)))))
;;    (if (and (> fn-count 1)
;;	     (not into-dir))
;;	(error "Marked %s: target must be a directory: %s" operation target))
;;    ;; rename-file bombs when moving directories unless we do this:
;;    (or into-dir (setq target (directory-file-name target)))
;;    (dired-create-files
;;     file-creator operation fn-list
;;     (if into-dir			; target is a directory
;;	 ;; This function uses fluid vars into-dir and target when called
;;	 ;; inside dired-create-files:
;;	 (function (lambda (from)
;;		     (expand-file-name (file-name-nondirectory from) target)))
;;       (function (lambda (from) target)))
;;     marker-char)
;;     (revert-buffer)
;;     (setq target (downcase (file-name-nondirectory (directory-file-name 
;;       (file-name-directory target)))))
;;     (if (get-buffer target)
;;       (if (not (eq (get-buffer target) (current-buffer)))
;;         (save-excursion (set-buffer target) (revert-buffer))))))

;;(defun dired-create-directory (directory)
;;  "Create a directory called DIRECTORY."
;;  (interactive
;;   (list (read-file-name "Create directory: " (dired-current-directory))))
;;    (make-directory directory)
;;     (revert-buffer))


;;; dired-vms.el ends here
