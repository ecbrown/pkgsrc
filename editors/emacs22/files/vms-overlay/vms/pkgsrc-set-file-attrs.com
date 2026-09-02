$! Apply archive-safe RMS attributes to the staged raw dump.
$! P1 = native file specification.
$ save_verify = f$verify(0)
$ save_parse_style = f$getjpi("","PARSE_STYLE_PERM")
$ attr_status = 1
$ on control_y then goto failed
$ on error then goto failed
$ set process/parse_style=extended
$ set file/attribute=(rfm:udf,rat:none) 'p1'
$ attr_status = $status
$ goto done
$failed:
$ attr_status = $status
$done:
$ set process/parse_style='save_parse_style'
$ exit attr_status + 0*f$verify(save_verify)
