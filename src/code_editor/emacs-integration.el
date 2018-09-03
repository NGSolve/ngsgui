

(require 'epc)
(when noninteractive
  (load "subr")
  (load "byte-run"))
(eval-when-compile (require 'cl))

(message "Start EPC")

(setq frame-resize-pixelwise t)

(defvar pyepc-ngsolve-epc
  (epc:start-epc "python3" (list ".printport.py"))
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
    (epc:call-deferred pyepc-ngsolve-epc 'run nil)))

(require 'python)
(define-key python-mode-map (kbd "C-c r") 'pyepc-ngsolve-run)


;; (defun pyepc-sample-gtk-destroy ()
;;   "Close GTK window"
;;   (interactive)
;;   (deferred:nextc
;;     (epc:call-deferred pyepc-sample-gtk-epc 'destroy nil)
;;     (lambda ()
;;       (epc:stop-epc pyepc-sample-gtk-epc)
;;       (message "EPC server is stopped."))))

;; (defun pyepc-sample-gtk-set-button-label (label)
;;   "Change GUI button label."
;;   (interactive "sButton label: ")
;; (epc:call-deferred pyepc-sample-gtk-epc 'set_button_label (list label)))
