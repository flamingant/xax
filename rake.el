(cond 
 ((eq system-type 'windows-nt)
  (setq rake-args `(
		    (build-dir . ".bwg")
		    ))
  )
 ((eq system-type 'gnu/linux)
  (setq rake-args `(
		    (build-dir . ".bln")
		    ))
  )
 )

