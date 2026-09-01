$! Run MMS with the staging startup/VUE logicals defined in this DCL process.
$! P1 = MMS target, P2 = startup directory, P3 = VUE directory.
$
$ target = P1
$ startup = P2
$ vue = P3
$ define/job EMACS22_STARTUP 'startup'
$ define/job EMACS22_VUE 'vue'
$ MMS/IGNORE=WARNING 'target'
$ exit $status
