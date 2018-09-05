

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

(defun pyepc-ngsolve-next-tab ()
  "Next tab"
  (interactive)
  (deferred:nextc
    (epc:call-deferred pyepc-ngsolve-epc 'nextTab nil)))

(defun pyepc-ngsolve-previous-tab ()
  "Previous tab"
  (interactive)
  (deferred:nextc
    (epc:call-deferred pyepc-ngsolve-epc 'previousTab nil)))

(defun pyepc-ngsolve-activate-console ()
  "Activete interactive ipython console"
  (interactive)
  (deferred:nextc
    (epc:call-deferred pyepc-ngsolve-epc 'activateConsole nil)))

(require 'python)
(define-key python-mode-map (kbd "C-c r") 'pyepc-ngsolve-run)
(global-set-key (kbd "C-<left>") 'pyepc-ngsolve-previous-tab)
(global-set-key (kbd "C-<right>") 'pyepc-ngsolve-next-tab)
(global-set-key (kbd "C-c j") 'pyepc-ngsolve-activate-console)
