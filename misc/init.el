
;; GNU EMACS ALBLABLA'S
;; Author: Antonio Barreiros
;; Description: This is my personal emacs config file, keep in mind I am a total noob in emacs/elisp

; sets directory close to init.el
(cd "D:/projects")

; puts these emacs backup files elsewhere
(setq backup-directory-alist '((".*" . "~/.emacs.d/emacs-backup")))

;compilation
(setq casey-aquamacs (featurep 'aquamacs))
(setq casey-linux (featurep 'x))
(setq casey-win32 (not (or casey-aquamacs casey-linux)))

(setq compilation-directory-locked nil)
(scroll-bar-mode -1)
;(setq shift-select-mode nil)
(setq enable-local-variables nil)
(setq casey-font "outline-DejaVu Sans Mono")

(when casey-win32 
  (setq casey-makescript "build.bat")
  (setq casey-font "outline-Liberation Mono")
  )

(load-library "view")
(require 'cc-mode)
(require 'ido)
(require 'compile)
;(ido-mode t)



;(define-key global-map "\ef" 'find-file)
(define-key global-map "\eF" 'find-file-other-window)
(global-set-key (read-kbd-macro "\eb")  'ido-switch-buffer)
(global-set-key (read-kbd-macro "\eB")  'ido-switch-buffer-other-window)

(defun casey-ediff-setup-windows (buffer-A buffer-B buffer-C control-buffer)
  (ediff-setup-windows-plain buffer-A buffer-B buffer-C control-buffer)
)
(setq ediff-window-setup-function 'casey-ediff-setup-windows)
(setq ediff-split-window-function 'split-window-horizontally)

(defun casey-big-fun-compilation-hook ()
  (make-local-variable 'truncate-lines)
  (setq truncate-lines nil)
)

(add-hook 'compilation-mode-hook 'casey-big-fun-compilation-hook)
(defun w32-restore-frame ()
    "Restore a minimized frame"
     (interactive)
     (w32-send-sys-command 61728))

(defun maximize-frame ()
    "Maximize the current frame"
     (interactive)
     (when casey-aquamacs (aquamacs-toggle-full-frame))
     (when casey-win32 (w32-send-sys-command 61488)))

(define-key global-map "\ep" 'maximize-frame)
(define-key global-map "\ew" 'other-window)
(setq compilation-context-lines 0)
(setq compilation-error-regexp-alist
    (cons '("^\\([0-9]+>\\)?\\(\\(?:[a-zA-Z]:\\)?[^:(\t\n]+\\)(\\([0-9]+\\)) : \\(?:fatal error\\|warnin\\(g\\)\\) C[0-9]+:" 2 3 nil (4))
     compilation-error-regexp-alist))

(defun find-project-directory-recursive ()
  "Recursively search for a makefile."
  (interactive)
  (if (file-exists-p casey-makescript) t
      (cd "../")
      (find-project-directory-recursive)))

(defun lock-compilation-directory ()
  "The compilation process should NOT hunt for a makefile"
  (interactive)
  (setq compilation-directory-locked t)
  (message "Compilation directory is locked."))

(defun unlock-compilation-directory ()
  "The compilation process SHOULD hunt for a makefile"
  (interactive)
  (setq compilation-directory-locked nil)
  (message "Compilation directory is roaming."))

(defun find-project-directory ()
  "Find the project directory."
  (interactive)
  (setq find-project-from-directory default-directory)
  (switch-to-buffer-other-window "*compilation*")
  (if compilation-directory-locked (cd last-compilation-directory)
  (cd find-project-from-directory)
  (find-project-directory-recursive)
  (setq last-compilation-directory default-directory)))

(defun make-without-asking ()
  "Make the current build."
  (interactive)
  (if (find-project-directory) (compile casey-makescript))
  (other-window 1))
(define-key global-map "\em" 'make-without-asking)

; these noises are ridiculous
(setq visible-bell t)
(setq ring-bell-function 'ignore)

; increases undo buffer to a lot.
(setq undo-limit 20000000)
(setq undo-strong-limit 40000000)

; dracula
(add-to-list 'custom-theme-load-path "~/.emacs.d/themes")

;albos
(load-theme 'albos t)

; sets CUA cuz im a windows user :X
(cua-mode t)
(transient-mark-mode 1)
(add-to-list 'default-frame-alist '(font . "Liberation Mono-12.0") '(height . 160))

; disable menu, tool and scroll bar
(menu-bar-mode 0)
(tool-bar-mode 0)
(scroll-bar-mode 0)

; enables pair mode
(electric-pair-mode 1)

; startup configuration
(setq truncate-partial-width-windows nil)
(setq next-line-add-newlines nil)
(split-window-horizontally)
(global-set-key "\C-x\ w" 'toggle-truncate-lines)

; code from Casey Muratori, highlights TODO and NOTE
(setq fixme-modes '(c++-mode c-mode emacs-lisp-mode python-mode))
(make-face 'font-lock-fixme-face)
(make-face 'font-lock-note-face)
(mapc (lambda(mode)
	(font-lock-add-keywords
	 mode
	 '(("\\<\\(TODO\\)" 1 'font-lock-fixme-face t)
	   ("\\<\\(NOTE\\)" 1 'font-lock-note-face t))))
      fixme-modes)
(modify-face 'font-lock-fixme-face "Red" nil nil t nil t nil nil)
(modify-face 'font-lock-note-face "orange red" nil nil t nil t nil nil)

; increases tab width
(setq tab-width 4
      indent-tabs-mode nil)
(windmove-default-keybindings 'meta)
