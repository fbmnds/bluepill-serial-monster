
;;(import (chicken posix))
(import (chicken io))
(import (chicken port))
(import (chicken process-context))
(import (chicken file))
(import srfi-1)
(import srfi-13)
(import (chicken sort)) 
(import regex)

;; Function to extract #include directives from a file


(define (extract-includes file-path)
  (if (file-exists? file-path)
      (call-with-input-file file-path
        (lambda (port)
          (let loop ((line (read-line port))
                     (includes '()))
            (if (eof-object? line)
                includes
                (let ((match (string-match "#include\\s*[<\"]([^>\"]+)[>\"]" line)))
                  (loop (read-line port)
                        (if match
                            (cons (cadr match) includes)
                            includes)))))))
      '()))

;; Function to get all C files (.c and .h) in a directory
(define (get-c-files dir)
  (find-files dir
              #:test (lambda (f)
                       (or (string-suffix? ".c" f)
                           (string-suffix? ".h" f)))))

;; Function to recursively collect all included headers
(define (collect-all-includes start-files seen)
  (let loop ((files start-files)
             (all-includes '())
             (seen seen))
    (if (null? files)
        (values all-includes seen)
        (let* ((file (car files))
               (includes (extract-includes file)))
          (let ((new-includes (filter (lambda (inc) (not (member inc seen)))
                                     includes)))
            (loop (append new-includes (cdr files))
                  (append new-includes all-includes)
                  (append new-includes seen)))))))

;; Main function
(define (main dir)
  (let ((c-files (get-c-files dir)))
    (if (null? c-files)
        (print "No C files found in directory: " dir)
        (let-values (((includes seen) (collect-all-includes c-files '())))
          (print "Included header files:")
          (for-each (lambda (inc)
                      (print "  " inc))
                    (sort includes string<?))))))

;; Program entry point
(let ((args (command-line-arguments)))
  (if (null? args)
      (print "Usage: " (car (argv)) " <directory>")
      (main (car args))))
