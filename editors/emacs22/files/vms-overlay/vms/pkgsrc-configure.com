$! Configure the Emacs VMS tree from a pkgsrc staging build.
$! P1 = staged prefix, P2 = NO for the terminal-only flavor.
$
$ save_default = f$environment("DEFAULT")
$ root = f$environment("DEFAULT")
$ src = root - "]" + ".]"
$ stage = p1
$ startup = p1 - "]" + ".STARTUP]"
$ vue = p1 - "]" + ".VUE]"
$ define/job EMACS22_SRC 'src'
$ define/job EMACS22_STAGE 'stage'
$ define/job EMACS22_STARTUP 'startup'
$ define/job EMACS22_VUE 'vue'
$ set default 'root'
$ if p2 .eqs. "NO" then goto no_x
$ @configure.com --FORCE --SRCDIR=EMACS22_SRC:[000000] --WITH-TCPIP=YES --WITH-X=YES --WITH-X-TOOLKIT=NO --PREFIX=EMACS22_STAGE: --STARTUPDIR=EMACS22_STARTUP: --VUELIBDIR=EMACS22_VUE:
$ configure_status = $status
$ goto done
$no_x:
$ @configure.com --FORCE --SRCDIR=EMACS22_SRC:[000000] --WITH-TCPIP=YES --WITH-X=NO --WITH-X-TOOLKIT=NO --PREFIX=EMACS22_STAGE: --STARTUPDIR=EMACS22_STARTUP: --VUELIBDIR=EMACS22_VUE:
$ configure_status = $status
$done:
$ set default 'save_default'
$ exit configure_status
