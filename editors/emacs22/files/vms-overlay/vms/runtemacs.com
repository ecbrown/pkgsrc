$! This is a subcommand to testemacs.com.
$! It was automatically generated.
$! Do NOT run this directly.  Instead, run testemacs.com
$! and use the symbols `runtemacs' and `runtemacs_d'.
$ proc_dir = f$parse("A.;0",f$environment("PROCEDURE")) - "A.;0"
$ args = ""
$ if args .nes. "" .or. p8 .nes. "" then args = p8 + " " + args
$ if args .nes. "" .or. p7 .nes. "" then args = p7 + " " + args
$ if args .nes. "" .or. p6 .nes. "" then args = p6 + " " + args
$ if args .nes. "" .or. p5 .nes. "" then args = p5 + " " + args
$ if args .nes. "" .or. p4 .nes. "" then args = p4 + " " + args
$ if args .nes. "" .or. p3 .nes. "" then args = p3 + " " + args
$ if args .nes. "" .or. p2 .nes. "" then args = p2 + " " + args
$ define/user EMACSLOADPATH "AXPA:[TTN.EMACS.EMACS212_3.LISP],AXPA:[TTN.LOCAL.LIB.EMACS.SITE-LISP],AXPA:[TTN.LOCAL.LIB.EMACS.21_2.LISP]"
$ define/user EMACSPATH "AXPA:[TTN.EMACS.EMACS212_3.LIB-SRC]"
$ define/user EMACSDATA "AXPA:[TTN.EMACS.EMACS212_3.ETC]"
$ define/user EMACSDOC "AXPA:[TTN.EMACS.EMACS212_3.ETC]"
$ define/user TERMCAP "AXPA:[TTN.EMACS.EMACS212_3.ETC]TERMCAP.DAT"
$ define/user INFOPATH "AXPA:[TTN.EMACS.EMACS212_3.INFO]"
$ define/user SYS$INPUT SYS$COMMAND
$ mcr 'proc_dir'temacs'p1' -map 'proc_dir'temacs'p1'.dump 'args'
$ exit
