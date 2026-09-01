$! VMS command file to make definitions so that you can run
$! `temacs.exe' with the data file just dumped by build.com.
$!
$ scrdir := @srcdir@
$ proc_dir = f$parse("A.;0",f$environment("PROCEDURE")) - "A.;0"
$ runtemacs :== @'proc_dir'runtemacs """"""
$ runtemacs_d :== @'proc_dir'runtemacs """_d"""
$ temacs :== @'srcdir'kepteditor temacs
$ temacs_d :== @'srcdir'kepteditor temacs_d
$ exit
