;; ##generated## from <.gdb.g.el>

;;
;; HOST: moga
;;

(setq gdb-exec-dir default-directory)
(setq gdb-exec-file "tc")

(setq gdb-debugger-file "gdb")
(setq gdb-debugger-args "--annotate=3")

(defun gdb-quick (&optional arg) (interactive "P")
  (if arg (call-interactively 'gdb)
    (if (not (and (boundp 'gud-comint-buffer)
		  gud-comint-buffer
		  (buffer-name gud-comint-buffer)))
	(if gdb-exec-file
	    (let ((cmd (format "%s %s %s" gdb-debugger-file gdb-debugger-args
			       (filename-concat gdb-exec-dir gdb-exec-file))))
	      (gdb cmd)
	      )
	  (call-interactively 'gdb))))
  (switch-to-buffer gud-comint-buffer)
  (eob)
  (sit-for 5)
  )

(setq gud-gdb-history nil)
(setq gud-gdb-history
  (cons (format "%s %s %s" gdb-debugger-file gdb-debugger-args gdb-exec-file)
	gud-gdb-history))

(define-key global-map [C-f11] 'gdb-quick)

(defun gud-mode-map-extend ()
  (define-key gud-mode-map "\M-n" 'gud-next)
  (define-key gud-mode-map "\M-s" 'gud-step)
  (define-key gud-mode-map "\M-f" 'gud-finish)
  (define-key gud-mode-map "\M-u" 'gud-up)
  (define-key gud-mode-map "\M-d" 'gud-down)
  )

(defun gud-minor-mode-map-extend ()
  (define-key gud-minor-mode-map "\M-n" 'gud-next)
  (define-key gud-minor-mode-map "\M-s" 'gud-step)
  (define-key gud-minor-mode-map "\M-f" 'gud-finish)
  (define-key gud-minor-mode-map "\M-u" 'gud-up)
  (define-key gud-minor-mode-map "\M-d" 'gud-down)
  (define-key gud-minor-mode-map "\M-c" 'gud-cont)
  (define-key gud-minor-mode-map "\M-r" 'gud-run)
  )

(top-level)
(gud-mode-map-extend)

;; Local Variables:
;; first-change-hook : (not-this-file)
;; End:

