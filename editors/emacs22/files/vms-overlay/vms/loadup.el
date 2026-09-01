;;; loadup.el --- compatibility entry point for the OpenVMS build

;; Keep one authoritative preload sequence.  Older command procedures load
;; this file from [.VMS]; the real Emacs 22 preload logic lives in [.LISP].

(load "[-.lisp]loadup.el" nil nil t)

;;; loadup.el ends here
