(fn c-binary (file)
  (apply #'+ (c-list (@ #'string (@ #'char-code (string-list (fetch-file file)))) :brackets :curly)))

(fn c-truth (x)
  (? x "TRUE" "FALSE"))

(var *apps* nil)

(fn gen-game-info (&key binary name
                           start load-address
                           (memory-expansion-3k? nil)
                           (memory-expansion 0)
                           needs-paddles?
                           (manual-screen-updates? nil)
                           (frames-per-second 50)
                           (frame-irq? nil))
  (with (id       (+ name "_image")
         outpath  (format nil "examples/~A.c" name))
  (format t "Generating ~A to `~A'…~%" name outpath)
  (push name *apps*)
  (with-output-file o outpath
    (format o "/* Generated by gen-examples.lisp. */~%")
    (format o "#include <stdio.h>~%")
    (format o "#include <string.h>~%")
    (terpri o)
    (format o "#include \"types.h\"~%")
    (format o "#include \"config.h\"~%")
    (format o "#include \"6502.h\"~%")
    (format o "#include \"shadowvic.h\"~%")
    (format o "#include \"joystick.h\"~%")
    (format o "#include \"video.h\"~%")
    (format o "#include \"sync.h\"~%")
    (format o "#include \"debugger.h\"~%")
    (terpri o)
    (format o "byte ~A[] = ~A;~%" id (c-binary binary))
    (terpri o)
    (format o "int~%")
    (format o "main (int argc, char * argv[])~%")
    (format o "{~%")
    (format o "    struct vic20_config config = {~%")
    (format o "        .memory_expansion_3k = ~A,~%" (c-truth memory-expansion-3k?))
    (format o "        .memory_expansion = ~A,~%" memory-expansion)
    (format o "        .use_paddles = FALSE,~%" (c-truth needs-paddles?))
    (format o "        .manual_screen_updates = ~A,~%" (c-truth manual-screen-updates?))
    (format o "        .frames_per_second = ~A,~%" frames-per-second)
    (format o "        .frame_irq = ~A,~%" (c-truth frame-irq?))
    (format o "        .frame_interceptor = NULL~%")
    (format o "    };~%")
    (terpri o)
    (format o "    joystick_open ();~%")
    (format o "    video_open ();~%")
    (format o "    video_map ();~%")
    (format o "    vic20_open (&config);~%")
    (format o "    init_debugger ();~%")
    (format o "    memcpy (&m[~A], ~A, sizeof (~A));~%" load-address id id)
    (format o "    vic20_emulate (~A);~%" start)
    (format o "    vic20_close ();~%")
    (format o "    video_close ();~%")
    (format o "    joystick_close ();~%")
    (terpri o)
    (format o "    return 0;~%")
    (format o "}~%"))))

(gen-game-info
  :binary                   "examples/pulse.bin"
  :name                     "pulse"
  :start                    #x2000
  :load-address             #x2000
  :memory-expansion         1
  :needs-paddles?           nil
  :manual-screen-updates?   t
  :frames-per-second        32)

(gen-game-info
  :binary                   "examples/panicman.bin"
  :name                     "panicman"
  :start                    #x1201
  :load-address             #x11ff
  :memory-expansion         1
  :needs-paddles?           nil
  :manual-screen-updates?   t
  :frames-per-second        30)

(gen-game-info
  :binary                   "examples/arukanoido.bin"
  :name                     "arukanoido"
  :start                    #x1000
  :load-address             #x1000
  :memory-expansion         1
  :needs-paddles?           t
  :manual-screen-updates?   t
  :frame-irq?               nil
  :frames-per-second        50)

(with-output-file o "examples/Makefile.am"
  (format o "~A~%" (apply #'+ "bin_PROGRAMS = tests picovic " (pad *apps* " ")))
  (terpri o)
  (format o "tests_SOURCES = tests.c~%")
  (format o "tests_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include~%")
  (format o "tests_LDADD = ../src/libshadowvic.la~%")
  (terpri o)
  (format o "picovic_SOURCES = picovic.c~%")
  (format o "picovic_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include~%")
  (format o "picovic_LDADD = ../src/libshadowvic.la~%")
  (@ (! *apps*)
    (terpri o)
    (format o "~A_SOURCES = ~A.c~%" ! !)
    (format o "~A_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/include~%" !)
    (format o "~A_LDADD = ../src/libshadowvic.la~%" !)))

(quit)
