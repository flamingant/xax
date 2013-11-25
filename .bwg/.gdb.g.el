#!perl -I../.env

use envmain ;

# (perl-mode)
################################################################
__END__
;; !!ghead()!!

;;
;; HOST: !!$host!!
;;

(setq gdb-exec-dir default-directory)
(setq gdb-exec-file "tc!!$exec_suffix!!")

(setq gdb-debugger-file "!!$gdbexec!!")
(setq gdb-debugger-args "!!$gdbargs!!")

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

(defun gud-send (s &optional wait)
  (let ((process (get-buffer-process gud-comint-buffer)))
    (cond
     (wait)
     (t (setq s (concat s "\n")))
     )
    (comint-send-string process s)
    )
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
  (define-key gud-mode-map (kbd "s-k") '(lambda () (interactive (gud-send "kill"))))
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

(defun gdb-kill ()
  (or (boundp 'gud-comint-buffer) (setq gud-comint-buffer nil))
  (let ((process (get-buffer-process gud-comint-buffer)))
   (cond
    (process
     (save-excursion
      (set-buffer gud-comint-buffer)
      (comint-interrupt-subjob)
      (comint-send-string process "kill\n")
     )
    )
   )
  )
 )

(add-hook 'compilation-mode-hook 'gdb-kill)
;(gdb-kill)

(top-level)
(gud-mode-map-extend)

!!gtail()!!
__ENDEND__

