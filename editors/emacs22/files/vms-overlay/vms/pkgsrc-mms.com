$! Run MMS with short staging logicals defined in this DCL process.
$! P1 = MMS target, P2 = staged installation prefix.
$
$ target = f$edit(P1,"LOWERCASE")
$! MMS target names are case-sensitive with extended parsing, while DCL
$! uppercases unquoted command-procedure parameters.
$ root = f$environment("DEFAULT")
$ stage = P2
$ startup = P2 - "]" + ".STARTUP]"
$ vue = P2 - "]" + ".VUE]"
$ proc = f$environment("PROCEDURE")
$ proc_dir = f$parse(proc,,,"NODE") + f$parse(proc,,,"DEVICE") + -
	f$parse(proc,,,"DIRECTORY")
$ @'proc_dir'pkgsrc-define-root.com EMACS22_SRC 'root'
$ if .not. $status then exit $status
$ @'proc_dir'pkgsrc-define-root.com EMACS22_STAGE 'stage'
$ if .not. $status then exit $status
$ @'proc_dir'pkgsrc-define-root.com EMACS22_STARTUP 'startup'
$ if .not. $status then exit $status
$ @'proc_dir'pkgsrc-define-root.com EMACS22_VUE 'vue'
$ if .not. $status then exit $status
$! Under extended parsing, MMS can otherwise select a differently cased
$! description-file version.  Name the generated pkgsrc file explicitly.
$ MMS/IGNORE=WARNING/DESCRIPTION=descrip.mms 'target'
$ exit $status
