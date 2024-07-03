#!/bin/env guile
!#

(define compile-flags
  '("-g3"
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wdouble-promotion"
    "-Wno-unused-parameter"
    "-Wno-unused-function"
    "-Wno-sign-conversion"
    "-fsanitize=undefined"))

(define source-files
  '((format . "tests/format_tests.c")
    (string . "tests/string_tests.c")
    (hashmap . "tests/hashmap_tests.c")
    (lists . "tests/pair_tests.c")))

(define cwd (string-append (getcwd) "/"))

(define run-test
  (lambda (args)
    (let* ((keyword (string->symbol (list-ref args 2)))
	   (flags (string-join compile-flags " "))
	   (command (format #f "cc ~a ~a~a -o ~a && ./~a"
			    flags
			    cwd
			    (assoc-ref source-files keyword)
			    (symbol->string keyword)
			    (symbol->string keyword))))
      (display command) (newline)
      (system command))))

(define command-switch
  (lambda (args)
    (case (string->symbol (list-ref args 1))
      ((test) (run-test args))
      ((help) (format #t "this is a placeholder for helptext\n")))))

(define main
  (lambda (args)
    (command-switch args)))

(main (command-line))
