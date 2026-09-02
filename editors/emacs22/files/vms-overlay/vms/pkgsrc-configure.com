$! Configure the Emacs VMS tree from a pkgsrc staging build.
$! P1 = staged prefix, P2 = NO for the terminal-only flavor.
$
$ save_default = f$environment("DEFAULT")
$ root = f$environment("DEFAULT")
$ src = root - "]" + ".]"
$ stage = p1
$ startup = p1 - "]" + ".STARTUP]"
$ vue = p1 - "]" + ".VUE]"
$ proc = f$environment("PROCEDURE")
$ proc_dir = f$parse(proc,,,"NODE") + f$parse(proc,,,"DEVICE") + -
	f$parse(proc,,,"DIRECTORY")
$ @'proc_dir'pkgsrc-define-root.com EMACS22_SRC 'root'
$ if .not. $status then goto root_failed
$ @'proc_dir'pkgsrc-define-root.com EMACS22_STAGE 'stage'
$ if .not. $status then goto root_failed
$ @'proc_dir'pkgsrc-define-root.com EMACS22_STARTUP 'startup'
$ if .not. $status then goto root_failed
$ @'proc_dir'pkgsrc-define-root.com EMACS22_VUE 'vue'
$ if .not. $status then goto root_failed
$ set default 'root'
$ set noon
$ if p2 .eqs. "NO" then goto no_x
$ @configure.com --FORCE --SRCDIR=EMACS22_SRC:[000000] --WITH-TCPIP=YES --WITH-X=YES --WITH-X-TOOLKIT=MOTIF --PREFIX=EMACS22_STAGE:[000000] --STARTUPDIR=EMACS22_STARTUP:[000000] --VUELIBDIR=EMACS22_VUE:[000000]
$ configure_status = $status
$ goto done
$no_x:
$ @configure.com --FORCE --SRCDIR=EMACS22_SRC:[000000] --WITH-TCPIP=YES --WITH-X=NO --WITH-X-TOOLKIT=NO --PREFIX=EMACS22_STAGE:[000000] --STARTUPDIR=EMACS22_STARTUP:[000000] --VUELIBDIR=EMACS22_VUE:[000000]
$ configure_status = $status
$ goto done
$root_failed:
$ configure_status = $status
$done:
$ if .not. configure_status
$  then
$   write sys$error "%PKGSRC-E-CONFIGURE, configure.com failed: ", -
	f$message(configure_status)
$   goto restore_default
$  endif
$ config_file = "descrip.mms"
$ gosub validate_mms
$ if .not. configure_status then goto restore_default
$ config_file = "[.src]descrip.mms"
$ gosub validate_mms
$ if .not. configure_status then goto restore_default
$ if p2 .eqs. "NO" then goto gui_validation_done
$ config_pattern = "USE_MOTIF"
$ gosub validate_generated
$ if .not. configure_status then goto restore_default
$ config_pattern = "DECW$XmuLIBSHRR5.EXE"
$ gosub validate_generated
$ if .not. configure_status then goto restore_default
$gui_validation_done:
$ config_file = "[.lib-src]descrip.mms"
$ gosub validate_mms
$ if .not. configure_status then goto restore_default
$ config_file = "[.oldXMenu]descrip.mms"
$ gosub validate_mms
$ if .not. configure_status then goto restore_default
$ config_file = "[.lwlib]descrip.mms"
$ gosub validate_mms
$ if .not. configure_status then goto restore_default
$ config_file = "[.vms]descrip.mms"
$ gosub validate_mms
$ if .not. configure_status then goto restore_default
$ config_file = "[.vms]emacs_vue.com"
$ config_pattern = "vue$suppress_output_popup"
$ gosub validate_generated
$ if .not. configure_status then goto restore_default
$ config_file = "[.src]config.h"
$ config_pattern = "HAVE_ALLOCA 1"
$ gosub validate_generated
$restore_default:
$ set default 'save_default'
$! CONFIGURE.COM can leave an alternate odd (successful) VMS status.  The GNV
$! DCL launcher only maps canonical SS$_NORMAL reliably to a Unix zero status.
$! Preserve failures verbatim, but normalize every VMS-success completion.
$ if configure_status then configure_status = 1
$ exit configure_status
$
$validate_mms:
$ if f$search(config_file) .eqs. ""
$  then
$   write sys$error "%PKGSRC-E-NOCONFIG, configure did not create ",config_file
$   configure_status = 2
$   return
$  endif
$ search/nolog/output=nla0: 'config_file' "all :"
$ search_severity = $severity
$ if search_severity .ne. 1
$  then
$   write sys$error "%PKGSRC-E-BADCONFIG, ",config_file," has no all target"
$   configure_status = 2
$  endif
$ return
$
$validate_generated:
$ if f$search(config_file) .eqs. ""
$  then
$   write sys$error "%PKGSRC-E-NOCONFIG, configure did not create ",config_file
$   configure_status = 2
$   return
$  endif
$ search/nolog/output=nla0: 'config_file' "''config_pattern'"
$ search_severity = $severity
$ if search_severity .ne. 1
$  then
$   write sys$error "%PKGSRC-E-BADCONFIG, ",config_file," is incomplete"
$   configure_status = 2
$  endif
$ return
