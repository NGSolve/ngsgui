

(require 'epc)
(when noninteractive
  (load "subr")
  (load "byte-run"))
(eval-when-compile (require 'cl))

(message "Start EPC")

(setq frame-resize-pixelwise t)

(defvar pyepc-ngsolve-epc
  (epc:start-epc "python3" `(,(concat "-c \rprint(" (number-to-string portnumber) ")")))
  "EPC manager object for GTK server example.")

(epc:define-method pyepc-ngsolve-epc
                   'message
                   (lambda (&rest args) (message "%S" args)))

(epc:define-method pyepc-ngsolve-epc
                   'set-width
                   (lambda (arg) (set-frame-width (selected-frame) arg nil t)))

(epc:define-method pyepc-ngsolve-epc
                   'set-height
                   (lambda (arg) (set-frame-height (selected-frame) arg nil t)))

(defun pyepc-ngsolve-run ()
  "Run buffer"
  (interactive)
  (deferred:nextc
    (epc:call-deferred pyepc-ngsolve-epc 'run (list buffer-file-name))))

(require 'python)
(define-key python-mode-map (kbd "C-c r") 'pyepc-ngsolve-run)
