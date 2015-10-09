(apply #'assemble-files "tests.bin" '("tests.asm"))                                                                                                             
(defun c-binary (file)
  (apply #'+ (c-list (@ #'string (@ #'char-code (string-list (fetch-file file)))) :brackets :curly)))

(with-output-file o "tests.bin.c"
  (format o "byte tests_image[] = ~A;~%" (c-binary "tests.bin")))

(quit)
