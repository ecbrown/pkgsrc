$NetBSD$

Use VMS-native paths for recursive extension builds.  MMS needs source-relative
paths and command definitions that remain valid after descending into nested
extension directories.

--- make_ext.pl.orig
+++ make_ext.pl
@@ -283,6 +283,7 @@ sub build_extension {
     my $up = $ext_dir;
     $up =~ s![^/]+!..!g;
 
+    $perl = VMS::Filespec::vmsify("$up/miniperl.exe") if IS_VMS;
     $perl ||= "$up/miniperl";
     my $return_dir = $up;
     my $lib_dir = "$up/lib";
@@ -518,13 +519,28 @@ sub build_extension {
 		'INSTALLMAN3DIR=none';
 	}
 	push @args, @$pass_through;
-	push @args, 'PERL=' . $perl if $perl; # use miniperl to run the Makefile later
+	my $perl_for_make = IS_VMS ? 'Sys$Disk:[-.-]miniperl.exe' : $perl;
+	push @args, 'PERL=' . $perl_for_make if $perl_for_make; # use miniperl to run the Makefile later
 	_quote_args(\@args) if IS_VMS;
 	print join(' ', $perl, @args), "\n" if $verbose;
 	my $code = do {
 	   local $ENV{PERL_MM_USE_DEFAULT} = 1;
 	    system $perl, @args;
 	};
+	if (IS_VMS && -f $makefile) {
+	    open my $mms_fh, q(<), $makefile
+		or die "Cannot read $makefile: $!";
+	    local $/;
+	    my $mms = <$mms_fh>;
+	    close $mms_fh;
+	    $mms =~ s/^PERL\s*=.*$/PERL = MCR \x24(PERL_SRC)miniperl.exe/m;
+	    $mms =~ s/^FULLPERL\s*=.*$/FULLPERL = MCR \x24(PERL_SRC)perl.exe/m;
+	    $mms =~ s/^XSUBPPARGS\s*=.*$/XSUBPPARGS = -typemap "\x24(XSUBPPDIR)typemap"/m;
+	    open my $mms_out, q(>), $makefile
+		or die "Cannot write $makefile: $!";
+	    print {$mms_out} $mms;
+	    close $mms_out;
+	}
 	if($code != 0){
 	    #make sure next build attempt/run of make_ext.pl doesn't succeed
 	    _unlink($makefile);
