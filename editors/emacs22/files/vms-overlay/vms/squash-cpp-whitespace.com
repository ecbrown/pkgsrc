$! Author: Thien-Thi Nguyen <ttn@gnu.org>
$! Time-stamp: <03/03/11 03:04:20 TTN>
$!
$! Usage: @squash-cpp-whitespace FILE
$!
$! Commentary:
$!
$! Apparently, the C preprocessor on alpha-dec-vms7.3 (at least) inserts
$! whitespace between tokens, so that something simple like:
$!
$!              sys$share:ucx$ipc_shr/share
$! becomes
$!              sys$share : ucx$ipc_shr / share
$!
$! which fubars the rest of the flow (which now considers each of these
$! a separate token and subsequently mishandles the stream).  As of 21.x
$! GNU Emacs still uses the C preprocessor, so rather than do something
$! radical (like bundling and using m4), this program simply goes through
$! the given FILE and converts:
$!
$!              " . " -> "."
$!              " : " -> ":"
$!              " / " -> "/"
$!
$! FILE is then overwritten w/ the changes.
$!
$! Code:
$!
$ on warning then exit
$ on warning then exit
$!
$       __infile = p1
$       open/write out tmp-'__infile'
$       open/read  in  '__infile'
$       linecount = 0
$ loop:
$       read/end=done in foo
$ dot:
$       i = f$locate (" . ", foo)
$       len = f$length (foo)
$       if i .ne. len
$               then
$               foo = f$extract (0, i, foo) + "." + -
                      f$extract (i + 3, len - i, foo)
$               goto dot
$               endif
$ colon:
$       i = f$locate (" : ", foo)
$       len = f$length (foo)
$       if i .ne. len
$               then
$               foo = f$extract (0, i, foo) + ":" + -
                      f$extract (i + 3, len - i, foo)
$               goto colon
$               endif
$ slash:
$       i = f$locate (" / ", foo)
$       len = f$length (foo)
$       if i .ne. len
$               then
$               foo = f$extract (0, i, foo) + "/" + -
                      f$extract (i + 3, len - i, foo)
$               goto slash
$               endif
$
$       write out foo
$       linecount = linecount + 1
$       goto loop
$!
$ done:
$       close in
$       close out
$       write sys$output "(",linecount," lines)"
$!
$       rename tmp-'__infile' '__infile'
$       exit
