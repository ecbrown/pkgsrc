# pkg_add on OpenVMS cannot create this top-level entry when the package is
# installed below a logical prefix.  The canonical lib/perl5/bin/perllink
# entry remains packaged and usable.
$0 == "bin/perllink" {
	next
}
{
	if ($0 ~ /^@/) {
		print_entry($0)
		next
	}
	if (!seen[$0]++) {
		print_entry($0)
	}
	next
}
