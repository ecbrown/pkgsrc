# pkg_create on OpenVMS needs an explicit "./" for a top-level bin entry.
# Keep the installed file while avoiding its VMS relative-path lookup bug.
$0 == "bin/perllink" {
	print "./" $0
	next
}
{ print }
