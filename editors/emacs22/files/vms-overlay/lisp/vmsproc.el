;;; vmsproc.el --- run asynchronous VMS subprocesses under Emacs

;; Copyright (C) 1986, 2001, 2002, 2003, 2004, 2005,
;;   2006, 2007, 2008 Free Software Foundation, Inc.

;; Author: Mukesh Prasad
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

(defvar display-subprocess-window nil
  "If non-nil, the subprocess window is displayed whenever input is received.")

(defvar command-prefix-string "$ "
  "String to insert to distinguish commands entered by user.")

(defvar subprocess-running nil
  "The asynchronous DCL process used by `subprocess-command', or nil.")

(defvar subprocess-buf nil
  "Buffer associated with `subprocess-running'.")

(defvar command-mode-map
  (let ((map (make-sparse-keymap)))
    (define-key map "\C-m" 'command-send-input)
    (define-key map "\C-u" 'command-kill-line)
    map))

(defun subprocess-input (process string)
  "Handle STRING received from the DCL subprocess PROCESS."
  (let ((buffer (process-buffer process))
	(mark (process-mark process)))
    (when (buffer-live-p buffer)
      (with-current-buffer buffer
	(let ((move-point (= (point) (marker-position mark))))
	  (save-restriction
	    (widen)
	    (save-excursion
	      (goto-char mark)
	      (insert string)
	      (set-marker mark (point))))
	  (when move-point
	    (goto-char mark))))
      (if display-subprocess-window
	  (display-buffer buffer)))))

(defun subprocess-exit (process event)
  "Notice that DCL subprocess PROCESS has exited.
EVENT is the process status message supplied by Emacs."
  (when (and (eq process subprocess-running)
	     (memq (process-status process) '(exit signal closed failed)))
    (setq subprocess-running nil)))

(defun vms-subprocess-live-p ()
  "Return non-nil if `subprocess-running' can accept commands."
  (and (processp subprocess-running)
       (memq (process-status subprocess-running) '(run stop))))

(defun start-subprocess ()
  "Spawn an asynchronous subprocess with output redirected to
the buffer *COMMAND*.  Within this buffer, use C-m to send
the last line to the subprocess or to bring another line to
the end."
  (if (vms-subprocess-live-p)
      subprocess-running
    (setq subprocess-buf (get-buffer-create "*COMMAND*")
	  subprocess-running nil)
    (with-current-buffer subprocess-buf
      (use-local-map command-mode-map))
    ;; The empty argument keeps the VMS *dcl* launcher in interactive
    ;; mode; only a leading "-c" requests a one-command subprocess.
    (setq subprocess-running
	  (start-process "DCL" subprocess-buf "*dcl*" ""))
    (set-process-filter subprocess-running 'subprocess-input)
    (set-process-sentinel subprocess-running 'subprocess-exit)
    subprocess-running))

(defun subprocess-command-to-buffer (command buffer)
  "Execute COMMAND and redirect output into BUFFER."
  ;; The VMS `call-process' implementation recognizes `*dcl* -c' and
  ;; passes the following string to DCL as one complete command line.
  ;; Treating the command verb as PROGRAM instead incorrectly appends
  ;; "*dcl*" as its first parameter and makes commands such as DIRECTORY
  ;; fail with DCL-W-MAXPARM.
  (call-process "*dcl*" nil buffer nil "-c" command))

(defun subprocess-command ()
  "Start asynchronous subprocess if not running and switch to its window."
  (interactive)
  (if (not (vms-subprocess-live-p))
      (start-subprocess))
  (and (vms-subprocess-live-p)
       (progn (pop-to-buffer subprocess-buf) (goto-char (point-max)))))

(defun command-send-input ()
  "If at last line of buffer, send the current line to
the spawned subprocess.  Otherwise bring back current
line to the last line for resubmission."
  (interactive)
  (beginning-of-line)
  (let* ((current-line (buffer-substring (point) (line-end-position)))
	 (prefixed (and command-prefix-string
			(<= (length command-prefix-string) (length current-line))
			(string-equal command-prefix-string
				      (substring current-line 0
						 (length command-prefix-string)))))
	 (command-line (if prefixed
			   (substring current-line (length command-prefix-string))
			 current-line)))
    (if (= (line-end-position) (point-max))
	(progn
	  (if (not (vms-subprocess-live-p))
	      (start-subprocess))
	  (if (vms-subprocess-live-p)
	      (progn
		(beginning-of-line)
		(if (and command-prefix-string (not prefixed))
		    (progn (beginning-of-line) (insert command-prefix-string)))
		(goto-char (point-max))
		(insert ?\n)
		(set-marker (process-mark subprocess-running) (point))
		(process-send-string subprocess-running
				     (concat command-line "\n")))))
      ;; else -- if not at last line in buffer
      (goto-char (point-max))
      (if (not (bolp))
	  (insert ?\n))
      (insert
	command-line))))

(defun command-kill-line ()
  "Kill the current line.  Used in command mode."
  (interactive)
  (beginning-of-line)
  (kill-line))

(define-key esc-map "$" 'subprocess-command)

;; arch-tag: 600b2512-f903-4887-bcd2-e76b306f5b66
;;; vmsproc.el ends here
