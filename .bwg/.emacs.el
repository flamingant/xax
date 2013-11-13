(qb-define (kbd "C-d C-d") ".gdbinit" t)

(set-default 'c-basic-offset 4)
(set-default 'c-syntactic-indentation t)
(set-default 'c-electric-flag nil)
(setq search-whitespace-regexp nil)

(bake-tool-select 'gcc)
(top-level)
;(gdb "d:/g/gdb-7.2/bin/gdb-python27 -i=mi mi2ly.exe")
(setq gdb-exec-file "c.exe")
(setq gdb-exec-file "tc.exe")
