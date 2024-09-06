#!/bin/env guile
!#

(define compile-flags-default
  '("-g3"
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wdouble-promotion"
    "-Wno-unused-parameter"
    "-Wno-unused-function"
    "-Wno-sign-conversion"
    "-fsanitize=undefined,address"
    "-lxml2"
    "-I/usr/include/libxml2"))

(define compile-flags-valgrind
  '("-g3"
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Wdouble-promotion"
    "-Wno-unused-parameter"
    "-Wno-unused-function"
    "-Wno-sign-conversion"
    "-fsanitize=undefined"
    "-lxml2"
    "-I/usr/include/libxml2"))

(define source-files
  '((format . "tests/format_tests.c")
    (string . "tests/string_tests.c")
    (hashmap . "tests/hashmap_tests.c")
    (lists . "tests/pair_tests.c")
    (custom . "tests/custom_tests.c")
    (xml . "tests/lxml2_tests.c")))

(define cwd (string-append (getcwd) "/"))

(define run-test
  (lambda (args)
    (let* ((keyword (string->symbol (list-ref args 2)))
	   (flags-default (string-join compile-flags-default " "))
	   (command-default (format #f "cc ~a ~a~a -o ~a && ./~a"
			    flags-default
			    cwd
			    (assoc-ref source-files keyword)
			    (symbol->string keyword)
			    (symbol->string keyword)))
	   (flags-valgrind (string-join compile-flags-valgrind " "))
	   (delete-command (format #f "rm ./~a" (symbol->string keyword)))
	   (command-valgrind (format #f "cc ~a ~a~a -o ~a && valgrind ./~a"
			    flags-valgrind
			    cwd
			    (assoc-ref source-files keyword)
			    (symbol->string keyword)
			    (symbol->string keyword))))
      (display command-default) (newline)
      (system command-default)
      (system delete-command)
      (newline) (newline)
      (display command-valgrind) (newline)
      (system command-valgrind))))

(define command-switch
  (lambda (args)
    (case (string->symbol (list-ref args 1))
      ((test) (run-test args))
      ((help) (format #t "this is a placeholder for helptext\n"))
      (else (format #t "this is a placeholder for helptext\n")))))

(define main
  (lambda (args)
    (command-switch args)))

(main (command-line))
