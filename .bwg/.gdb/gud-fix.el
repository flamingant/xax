(defun gud-filter (proc string)
  ;; Here's where the actual buffer insertion is done
  (setq string (replace-regexp-in-string "" "" string))
  (let (output process-window)
    (if (buffer-name (process-buffer proc))
	(if gud-filter-defer-flag
	    ;; If we can't process any text now,
	    ;; save it for later.
	    (setq gud-filter-pending-text
		  (concat (or gud-filter-pending-text "") string))

	  ;; If we have to ask a question during the processing,
	  ;; defer any additional text that comes from the debugger
	  ;; during that time.
	  (let ((gud-filter-defer-flag t))
	    ;; Process now any text we previously saved up.
	    (if gud-filter-pending-text
		(setq string (concat gud-filter-pending-text string)
		      gud-filter-pending-text nil))

	    (with-current-buffer (process-buffer proc)
	      ;; If we have been so requested, delete the debugger prompt.
	      (save-restriction
		(widen)
		(if (marker-buffer gud-delete-prompt-marker)
		    (let ((inhibit-read-only t))
		      (delete-region (process-mark proc)
				     gud-delete-prompt-marker)
		      (comint-update-fence)
		      (set-marker gud-delete-prompt-marker nil)))
		;; Save the process output, checking for source file markers.
		(setq output (gud-marker-filter string))
		;; Check for a filename-and-line number.
		;; Don't display the specified file
		;; unless (1) point is at or after the position where output appears
		;; and (2) this buffer is on the screen.
		(setq process-window
		      (and gud-last-frame
			   (>= (point) (process-mark proc))
			   (get-buffer-window (current-buffer)))))

	      ;; Let the comint filter do the actual insertion.
	      ;; That lets us inherit various comint features.
	      (comint-output-filter proc output))

	    ;; Put the arrow on the source line.
	    ;; This must be outside of the save-excursion
	    ;; in case the source file is our current buffer.
	    (if process-window
		(with-selected-window process-window
		  (gud-display-frame))
	      ;; We have to be in the proper buffer, (process-buffer proc),
	      ;; but not in a save-excursion, because that would restore point.
	      (with-current-buffer (process-buffer proc)
		(gud-display-frame))))

	  ;; If we deferred text that arrived during this processing,
	  ;; handle it now.
	  (if gud-filter-pending-text
	      (gud-filter proc ""))))))

(defun gdb-shell (output-field)
  (let ((gdb-output-sink gdb-output-sink))
    (if (string-match "^[[:blank:]]+" output-field)
	(setq output-field (substring  output-field (match-end 0))))
    (setq gdb-filter-output
          (concat output-field gdb-filter-output))))

(defun gud-gdbmi-marker-filter (string)
  "Filter GDB/MI output."

  ;; Record transactions if logging is enabled.
  (when gdb-enable-debug
    (push (cons 'recv string) gdb-debug-log)
    (if (and gdb-debug-log-max
	     (> (length gdb-debug-log) gdb-debug-log-max))
	(setcdr (nthcdr (1- gdb-debug-log-max) gdb-debug-log) nil)))

  ;; Recall the left over gud-marker-acc from last time
  (setq gud-marker-acc (concat gud-marker-acc string))

  ;; Start accumulating output for the GUD buffer
  (setq gdb-filter-output "")
  (let ((output-record) (output-record-list))

    ;; Process all the complete markers in this chunk.
    (dolist (gdbmi-record gdbmi-record-list)
      (while (string-match (cdr gdbmi-record) gud-marker-acc)
	(push (list (match-beginning 0)
		    (car gdbmi-record)
		    (match-string 1 gud-marker-acc)
		    (match-string 2 gud-marker-acc)
		    (match-end 0))
	      output-record-list)
	(setq gud-marker-acc
	      (concat (substring gud-marker-acc 0 (match-beginning 0))
		      ;; Pad with spaces to preserve position.
		      (make-string (length (match-string 0 gud-marker-acc)) 32)
		      (substring gud-marker-acc (match-end 0))))))

    (setq output-record-list (sort output-record-list 'gdb-car<))

    (dolist (output-record output-record-list)
      (let ((record-type (cadr output-record))
	    (arg1 (nth 2 output-record))
	    (arg2 (nth 3 output-record)))
	(if (eq record-type 'gdb-error)
	    (gdb-done-or-error arg2 arg1 'error)
	  (if (eq record-type 'gdb-done)
	      (gdb-done-or-error arg2 arg1 'done)
	    ;; Suppress "No registers." since GDB 6.8 and earlier duplicates MI
	    ;; error message on internal stream.  Don't print to GUD buffer.
	    (unless (and (eq record-type 'gdb-internals)
		     (string-equal (read arg1) "No registers.\n"))
	      (funcall record-type arg1))))))

    (setq gdb-output-sink 'user)
    ;; Remove padding.
    (string-match "^ *" gud-marker-acc)
    (setq gud-marker-acc (substring gud-marker-acc (match-end 0)))

    gdb-filter-output))
