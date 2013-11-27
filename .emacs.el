(bake-tool-select 'gcc)

(qb-define (kbd "C-a C-m") "main.c" t)
(qb-define (kbd "C-a C-u") "uf.c" t)
(qb-define (kbd "C-u C-h") "uf.h" t)
(qb-define (kbd "C-a C-a") "ma.c" t)
(qb-define (kbd "C-a C-h") "http.c" t)
(qb-define (kbd "C-a C-t") "ht_tc.c" t)
(qb-define (kbd "C-a C-e") ".emacs.el" t)
(qb-define (kbd "C-a C-s") "stimer.c" t)
(qb-define (kbd "C-a C-q") "tc_sql.c" t)
(qb-define (kbd "C-a C-i") "ma_irc.c" t)
(qb-define (kbd "C-a C-j") "inserver.c" t)
(qb-define (kbd "C-a C-d") "dht.c" t)
(qb-define (kbd "C-a C-n") "dht_session.c" t)

(qb-define (kbd "C-s C-a") "arg.c" t)

(qb-define (kbd "C-h C-h") "htdocs/index.html" t)

(qb-define (kbd "C-d C-d") "e:/_/D/dingoo/dev/sdl/a.1" t)

(define-key global-map (read-kbd-macro "H-5 H-5") 'help)
(define-key help-map (read-kbd-macro "H-5") 'describe-function)
(define-key help-map (read-kbd-macro "H-4") 'describe-variable)
(define-key help-map (read-kbd-macro "H-6") 'where-is)
(define-key help-map (read-kbd-macro "H-8") 'describe-key-briefly)

(setq safe-local-variable-values (cons '(first-change-hook . (not-this-file)) safe-local-variable-values))

(add-hook 'org-mode-hook 'turn-on-font-lock)

(defun c-other-file-gen (&optional name)
  "The 'gen' linked file"
  (or name (setq name (buffer-file-name)))
  (cond (name
	 (let* ((file (file-name-nondirectory name))
		(dir (file-name-directory name)))
	   (or (file-if-exists (format ".gen/%s" file))
	       )))
	)
  )

(file-class-linked-file-add 'c-mode '((gen . c-other-file-gen)))

(qb-define (control-key-vector ?l ?g)
	   '(linked-file 'gen)
	   )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defmacro cfs (&rest stuff)
  `(let ((case-fold-search t) (case-replace t))
     ,@stuff)
  )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define-key global-map "\C-x\C-v" vc-prefix-map)
(setq vc-extended-map (make-sparse-keymap))
(define-key vc-prefix-map "\C-v" vc-extended-map)

(defun git-commit-amend () (interactive)
  (vc-git--call nil "commit" "-a" "--amend" "--reuse-message=HEAD" )
  )

(defun git-commit-repeat () (interactive)
  (vc-git--call nil "commit" "-a" "--reuse-message=HEAD" )
  )

(define-key vc-extended-map "\C-v" 'git-commit-repeat)
(define-key vc-extended-map "\C-a" 'git-commit-amend)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defun win-boo-path-set () (interactive)
  (path-set
   `(
     "."
     "d:\\E\\Emacs\\emacs\\bin"
     "d:\\bin"
     "d:\\p\\perl\\bin"
     "d:\\cygwin\\bin"
;     "c:\\MinGW\\bin"
;     "c:\\MinGW\\msys\\1.0\\bin"
     "d:\\G\\Git\\bin"
     "C:\\WINDOWS\\system32"
     "C:\\WINDOWS"
     "C:\\WINDOWS\\system32\\WindowsPowerShell\\v1.0"
     "d:\\p\\postgreSQL\\9.2\\bin"
     )))

(defun gdb-qb (d)
  (qb-define (kbd "C-d C-b") d t)
  (qb-define (kbd "C-m C-m") (format "%s/makefile" d) t)
  (qb-define (kbd "C-g C-e") (format "%s/.gdb.el" d) t)
  (qb-define (kbd "C-g C-d") (format "%s/.gdb" d) t)
  (qb-define (kbd "C-g C-a") (format "%s/.gdb/a" d) t)
  )

(defun boo-win-init () 
  (gdb-qb ".bwg")
  (win-boo-path-set)
  )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defun boo-gnu-init () 
  (gdb-qb ".bln")
  )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(cond 
 ((eq system-type 'windows-nt)
  (boo-win-init)
  )
 ((eq system-type 'gnu/linux)
  (boo-gnu-init)
  )
 )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(top-level)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(find-file ".bwg/.gdb.el")
(find-file "e:/emacs/vc-x.el")




(top-level)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(define-key global-map (read-kbd-macro "H-h H-f") 'describe-function)

(cfs (query-replace-regexp "tline" "tanno"))

(setq grep-command "grep")

