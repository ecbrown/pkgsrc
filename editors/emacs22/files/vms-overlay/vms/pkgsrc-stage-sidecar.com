$! Move the newest staged file version to a package-owned sidecar and remove
$! every older version that would otherwise resurface through the POSIX name.
$! P1 = native source file specification, P2 = native sidecar specification.
$ save_verify = f$verify(0)
$ save_parse_style = f$getjpi("","PARSE_STYLE_PERM")
$ set process/parse_style=extended
$ sidecar_status = 1
$ on control_y then goto failed
$ on error then goto failed
$ source_file = f$edit(P1,"TRIM")
$ sidecar_file = f$edit(P2,"TRIM")
$ if source_file .eqs. "" .or. sidecar_file .eqs. "" then goto invalid
$ if f$edit(source_file,"UPCASE") .eqs. f$edit(sidecar_file,"UPCASE") -
	then goto invalid
$ if f$search(source_file) .eqs. "" then goto missing_source
$ if f$search(sidecar_file) .nes. "" then delete/nolog 'sidecar_file';*
$ if f$search(sidecar_file) .nes. "" then goto residual_sidecar
$ rename/nolog 'source_file' 'sidecar_file'
$ sidecar_status = $status
$ if .not. sidecar_status then goto done
$ if f$search(source_file) .nes. "" then delete/nolog 'source_file';*
$ if f$search(source_file) .nes. "" then goto residual_source
$ if f$search(sidecar_file) .eqs. "" then goto missing_sidecar
$ sidecar_status = 1
$ goto done
$missing_source:
$ write sys$error "%PKGSRC-E-NOSOURCE, staged source is missing: ",source_file
$ sidecar_status = 2
$ goto done
$residual_source:
$ write sys$error "%PKGSRC-E-OLDSOURCE, old staged versions remain: ",source_file
$ sidecar_status = 2
$ goto done
$residual_sidecar:
$ write sys$error "%PKGSRC-E-OLDSIDECAR, old sidecar versions remain: ",sidecar_file
$ sidecar_status = 2
$ goto done
$missing_sidecar:
$ write sys$error "%PKGSRC-E-NOSIDECAR, staged sidecar is missing: ",sidecar_file
$ sidecar_status = 2
$ goto done
$invalid:
$ write sys$error "%PKGSRC-E-BADSIDECAR, invalid sidecar move"
$ sidecar_status = 2
$ goto done
$failed:
$ sidecar_status = $status
$done:
$ set process/parse_style='save_parse_style'
$ exit sidecar_status + 0*f$verify(save_verify)
