(load "c/c.el")

(defun csym (sym) (string-substitute sym ?- ?_))

(defun unlo (s) (cond ((string-match "^lo_" s) (substring s 3)) (s)))

(defun args (fun) (cf-args x))

(defun lsym (sym)
  (let* ((s (string-substitute sym ?_ ?-))
	 )
    (case-match s
      ("[QVSF]$" (sms "\\(.*\\)[QVSF]+" s 1))
      ("^[QVSF]" (sms "\\(QV\\|QF\\|[QVSF]\\|\\)\\(.*\\)" s 2))
      ("." s)
      )
    )
  )

(defun lname (i)
  (or (car (cdr (assoc 'name i))) (lsym (symbol-name (cdecl-name i))))
  )

(defun new-lname (i &optional k)
  (or (car (cdr (assoc 'name i)))
      (let* ((name (string-substitute (symbol-name (cdecl-name i)) ?_ ?-))
	     (k (or k "[QVSF]"))
	     (post (format "%s$" k))
	     (pre (format "^%s" k)))
	(cond
	 ((string-match pre name) (substring name 1))
	 ((string-match post name) (substring name 0 -1))
	 (name)
	 )
	)
      )
  )

(defmacro grob (pat &optional no yes)
  (`
   (cond ((sx (bob) (rsf (, pat) search-limit t))
	  (, (or yes '(ms 0))))
	 ((, no))
	 ))
  )

(defun cg-subset (tag)
  (subset-if '(lambda (x) (assoc tag x)) cdefuns))

(defun sym-names (x)
  (list 'sym (intern (format "Q%s" (cdecl-name x))) (assoc 'name x))
  )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defmacro cg-start (&rest props)
  (setq search-limit (sx (rsf (regexp-quote "(cg-end)"))))
  (setq cg-file (downcase (file-name-nondirectory (basename))))
  (setq cdefuns nil)  
  (setq Qlist nil)
  (setq Slist nil)
  (setq Vlist nil)
  (setq Tlist nil)
  (setq Xlist nil)
  (setq Klist nil)
  nil
  )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defmacro X (&rest args) (setq Xlist (append Xlist args)) nil)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defmacro F (&optional flags &rest props)
  (mapcar '(lambda (x) (case x
			 (k (setq props (cons '(keys) props)))
			 (E (setq props (cons '(side-effects) props)))
			 (A (setq props (cons '(alloc) props)))
			 (l (setq props (cons '(lisp) props)))
			 (s (setq props (cons '(symbolic) props)))))
	  flags)
  (rsf "[{;]")      ; is this required ? possibly just makes trouble
  (setq cdefuns (cons (append (cdecl-fun) props) cdefuns))
  (message "function %s" (cadar cdefuns))
  nil
  )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defmacro K (&rest args)
  (sx
   (backward-sexp 2)
   (let ((v (cons (readc) args)))
     (message "keyword %s" v)
     (setq Klist (cons (cons 'sym v) Klist)))
   nil
   ))
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defmacro Q (&rest args)
  (sx
   (backward-sexp 2)
   (let ((v (cons (readc) args)))
     (message "symbol %s" v)
     (setq Qlist (cons (cons 'sym v) Qlist)))
   nil
   ))
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defmacro S (&rest args)
  (sx
   (backward-sexp 2)
   (let ((v (cons (readc) args)))
     (setq Slist (cons (cons 'sym v) Slist)))
   nil
   ))
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defmacro V (&rest args)
  (sx
   (backward-sexp 3)
   (let ((v (nreverse (list (readc) (readc)))))
     (message "variable %s" (car v))
     (setq Vlist (cons (cons 'var (append v args)) Vlist)))
   nil
   ))
  
(defun Vdef (list)
  (cat (mapcar '(lambda (i)
    (let* ((type (symbol-name (cdecl-type i)))
	   (ltype (cond ((string-match "^lo_" type) (substring type 3))
			(type)))
	   (ctype (cond ((string-match "^lo_" type) "lo")
			(type)))
	   (name (symbol-name (cdecl-name i)))
	   (init (car (cdr (assoc 'init i))))
	   (local (memq 'local i))
	   )
      (if local
	  (format "\t%s = deflocal(\"%s\",%s) ;\n"
		  name (lname i) (or init "Qnil"))
	(format "\tdefvar_%s(Q%s,&%s,%s) ;\n"
		  ctype name name (or init "Qnil"))))
    ) list)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defun buffer-find (s)
  (sx (bob) (rsf s))
  )

(defun function-defined (name)
  (sx (bob) (rsf (concat name "(")))
  )

(defun cg-init-fun (name &optional nullf)
  (cond
   ((function-defined name) name)
   ((or nullf "return_void")))
  )

(defun cg-def-mod ()
  (format "\nstruct mod mod_%s[] = {{\"%s\",mod_mimf,%s,%s}} ;\n"
	  (csym cg-file)
	  (csym cg-file)
	  (cond (Qlist "sym_mod") (t "0"))
	  (cond (cdefuns-lisp "sub_mod") (t "0"))
	  )
  )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defun cdefun-sub (i)
  (let* ((args (cf-args i))
	 (argc (or (cdr (assoc 'argc i)) (make-list 2 (length args))))
	 (cname (symbol-name (cdecl-name i)))
	 (lname (lname i))
	 (doc (or (car (cdr (assoc 'doc i))) ""))
	 )

    (format "    {%s%s,%s,0,{{0,%s,%s,%s}},%s,\"%s\"},\n"
	    (format "\"%s\",(lfun) %s,subret_%s," lname cname (unlo (cdecl-type i)))
	    (nth 0 argc)
	    (or (nth 1 argc) (length args))
	    (if (assoc 'side-effects i) 1 0)
	    (if (assoc 'alloc i) 1 0)
	    (if (assoc 'command i) 1 0)
	    (if (assoc 'keys i) (format "K%s" cname) "0")
	    doc
	    )
    )
  )

(defun mod-def-sub (list)
  (and list
       (concat
	"static lo_sub sub_mod[] = {\n"
	(cat (mapcar 'cdefun-sub list))
	"    {0}} ;\n\n")))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defun mod-def-key (list)
  (and list
       (concat
	"\nstatic struct sym_init key_mod[] = {\n"
	(cat (doprin x list "    {&%s,\"%s\"},\n" (cdecl-name x) (new-lname x "K")))
	"    {0}} ;\n")))

(defun mod-def-sym (list)
  (and list
       (concat
	"\nstatic struct sym_init sym_mod[] = {\n"
	(cat (doprin x list "    {&%s,\"%s\"},\n" (cdecl-name x) (lname x)))
	"    {0}} ;\n")))

(defun mod-def-tsym (list)
  (and list
       (concat
	"\nstatic struct sym_init tsym_mod[] = {\n"
	(cat (doprin x list "    {&%s,\"%s\"},\n" (cdecl-name x) (lname x)))
	"    {0}} ;\n")))

(defun cg-mimf ()
  (let ((sym (csym cg-file))
	(funs cdefuns-lisp))
    (concat
     "\nstatic int mod_mimf(int level)\n{\n    int r = 0 ;\n"
     (and (buffer-find "\\bpre__mimf(int") "    r |= pre__mimf(level) ;\n")
     (and (buffer-find "\\b__mimf(int") "    r |= __mimf(level) ;\n")
     "    switch(level) {\n"
     (cond (Tlist
	    (concat "    case MOD_TOINIT:"
		    (cat (doprin x Tlist "\n\tltd_static_register(%s_ltd) ;" (car x)))
		    "\n\tbreak ;\n"
		    )
	    ))
     "    case MOD_SUBINIT:"
     (and cdefuns-lisp "\n\tdefsubs(sub_mod) ;")
     "\n\tbreak ;\n"
     "    case MOD_SYMINIT:"
     (and Qlist "\n\tdefsyms(sym_mod) ;")
     (and Klist "\n\tdefkeys(key_mod) ;")
     (and Slist "\n\tdeftsyms(tsym_mod) ;")
     "\n\tbreak ;\n"
     "    case MOD_VARINIT:\n"
     (Vdef Vlist)
     "\tbreak ;\n"
     "    }\n"
     (and (buffer-find "\\bpost__mimf(int") "    r |= post__mimf(level) ;\n")
     "    return(r) ;\n    }\n"
     )
    )
  )

(defun cg-def-morsel ()
  (concat 
   (format "\nstatic int morsel_fun(int i,void *a)\n{\n" (csym cg-file))
   (format "    switch(i) {
    case MOM_LISPSTART: lisp_mod_add(mod_%s) ; break ;
}\n    return(0) ;\n}\n\n" (csym cg-file))
   (format "MORSEL morsel_%s[] = {morsel_fun} ;\n\n" (csym cg-file))
   )
  )

(defun cg-defs ()
 (concat
  "\n"
  (to-out)
  (mod-def-sub cdefuns-lisp)
  (cat (doprin x (append cdefuns-sym Vlist) "lo Q%s ;\n" (cdecl-name x)))
  (mod-def-sym Qlist)
  (mod-def-key Klist)
  (mod-def-tsym Slist)
  "\n"
  (cg-mimf)
  (cg-def-mod)
  (cg-def-morsel)
  ))

(defun cg-header ()
  (let ((fun '(lambda (list body)
		(and list (concat
			   (cat (mapcar '(lambda (i) (eval body)) list))
			   "\n")))))
    (concat
     (funcall fun Qlist '(format "extern lo %s ;\n" (cdecl-name i)))
     (funcall fun Klist '(format "extern lo %s ;\n" (cdecl-name i)))
;     (funcall fun Vlist '(format "extern lo Q%s ;\n" (cdecl-name i)))
     (funcall fun Slist '(format "extern lo %s ;\n" (cdecl-name i)))
     (funcall fun Vlist '(format "extern %s %s ;\n" (cdecl-type i)
				 (cdecl-name i)))
;     (funcall fun cdefuns-sym '(format "extern lo Q%s ;\n" (cdecl-name i)))
     "\n\n"
     )
    )
  )

(defun prin-cdecl-fun-proto- (fun print &optional class)
  (let ((args (cf-args fun)))
    (concat
     (and (or class (setq class (cdecl-class fun))) (prin class "%s "))
     (format "%s(%s)" (prin-cdecl-id-s fun)
	     (cond
	      (args (cat (mapcar print args) ","))
	      ("void")
	      )
	     )
     )))

(defun prin-cdecl-fun-proto (fun &optional class)
  (prin-cdecl-fun-proto- fun 'prin-cdecl-id-s class))

(defun cg-end ()
  (setq cdefuns-sym (cg-subset 'symbolic))
  (setq cdefuns-lisp (nreverse (cg-subset 'lisp)))
  (setq cdefuns-ext (cg-subset 'extern))
  (setq Qlist (append Qlist (mapcar 'sym-names Vlist)))
  (setq Qlist (append Qlist (mapcar 'sym-names cdefuns-sym)))
  (make-file (format "ext/%s.h" cg-file)
      (concat
       (format "/*#~= ##generated## (modf) */\n\n")
       (format "#ifndef __ext_%s\n#define __ext_%s\n\n" cg-file cg-file)
       (cat (mapcar 'eval Xlist) "\n") "\n\n"
       (cat (mapcar 'prin-cdecl-fun-proto cdefuns-ext) "" "%s ;\n")
       (cg-header)
       "#endif\n\n"
       ))
  (repr (rtor "\n" "^/\\*\\|\\'") (cg-defs))
;  (save-buffer)
  )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defmacro T (&rest args)
  (setq Tlist (cons args Tlist))
  nil)

(setq to-funs '(
		(gp lo)
		(funcall lo)
		(destroy void)
		(claim void)
		(lread lo)
		(eval lo)
		(print void)
		(format lo)
		(copy lo)
		(eq lo)
		(equal lo)
		(length lo_int)
		(pplist lo*)
		(aref lo)
		(aset lo)
		(sf_get int)
		(sf_set lo)
		(symget lo)
		(symset lo)
		(init void)
		(ox_int void)
		(ox_str void)
		(cmp int)
		(pprel void)
		(mread lo)
		(mwrite lo)
		(mdelete lo)
		(create lo)
		(next lo)
		(prev lo)
		(this lo)
		))

(defun tof-name (fun type)
  (format "%s__%s" type fun))

(defun to-find-funs (name)
  (sx
   (let ((tags (mapcar
		'(lambda (x)
		   (cons x (format "\\b%s\\b" (tof-name (car x) name))))
		       to-funs)))
     (mapcar
      '(lambda (x) (bob)
	 (reverse
	  (cons (if (rsf (cdr x) search-limit t) name 'tog) (car x))))
      tags)
     )))
    
(defun to-out ()
  (setq Tlist (nreverse Tlist))
  (cat (mapcar 'to-out-one Tlist))
  )

(defun to-flags (to)
  (let ((flags (or (delq nil (list
			      (cond ((assoc 'gcpro to) "tf_gcpro"))
			      (cond ((assoc 'destroy to) "tf_destroy"))

			      (cond ((assoc 'nopointer to) "tf_nopointer"))
			      (cond ((assoc 'link to) "tf_self_link"))

			      (cond ((assoc 'number to) "tf_number"))
			      (cond ((assoc 'integer to) "tf_integer"))
			      (cond ((assoc 'symbol to) "tf_symbol"))
			      (cond ((assoc 'string to) "tf_string"))
			      )) '("0"))))
    (concat "    " (mconcat flags " | ") ",\n")
    )
  )

(defun to-out-one (to)
  (let* ((name (car to))
	 (funs (cdr (assoc 'fun to)))
	 (to-funs (to-find-funs name))
	 )
    (concat
     (and (assoc 'alloc to) (setq funs (append '(a f) funs)) nil)
     (format "\nLTD %s_ltd[] = {{\n    \"%s\",'%s',\n"
	      name name (car (cdr (assoc 'tag to))) name)
     (to-flags to)
     (to-alloc-ref to)
     "    "
     (grob (format "%s__fields\\[]" name) "0," (format "%s__fields," name))
     "\n"
     (format "    %s,\n" (grob (format "%s__gs_name\\>" name) "gs__none"))
     (format "    %s,\n" (grob (format "%s__gs_cvx\\>" name) "gs__none"))
     (cat (mapcar '(lambda (x)
		     (format "    %s,\n" (apply 'tof-name (cdr x)))) to-funs))
     "    }} ;\n\n"
     )
    ))

(defun to-alloc-ref (to)
  (let* ((name (car to))
	 (z (cdr (assoc 'alloc to))))
    (case (car z)
      (f
       (format "    {0,nnc_nil, %s,sizeof(lo_%s),0,  0,0,0},\n"
	      (cadr z) name))
      (v
       (format "    {0,nnc_nil, %s,sizeof(lo_%s),0,  %s,%s,%s},\n"
	       (cadr z) name (nth 4 z) (nth 3 z) (nth 2 z)))
      (t "    {0},\n")
      )
    ))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defmacro modules (&rest list)
  (setq cg-modules list)
  nil)

(defmacro types (&rest list)
  (setq cg-types list)
  nil)

