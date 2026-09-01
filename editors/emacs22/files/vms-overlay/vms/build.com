$ ! VMS command file to run `temacs.exe' and dump the data file `temacs.dump'.
$ old_dump = f$search("temacs.dump;0")
$ temacs :== $emacs_library:[vms]temacs -batch
$ temacs -l [-.lisp]loadup.el dump
$ build_status = $status
$ if .not. build_status
$ then
$   new_dump = f$search("temacs.dump;0")
$   if new_dump .nes. "" .and. new_dump .nes. old_dump then delete 'new_dump'
$ endif
$ exit build_status
