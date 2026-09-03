$NetBSD$

Use mktemp safely with empty TMPDIR values and a fallback that preserves the
created path in the calling shell.  Copy debug snapshots because hard links
are unreliable on OpenVMS and symlinks would dangle after cleanup.  Use
standardized grep forms instead of egrep.

Keep debug trees only when explicitly requested, and stage/check final output
so failed tools or a full output device cannot be reported as success.

--- munchlist.X.orig
+++ munchlist.X
@@ -181,19 +181,71 @@
 #
 
 LIBDIR=!!LIBDIR!!
-TDIR=${TMPDIR-/tmp}
-MUNCHDIR=`mktemp -d ${TDIR}/munchXXXXXXXXXX 2>/dev/null`  ||  (umask 077; mkdir "$TDIR/munch$$"  ||  (echo "Can't create temp directory: ${TDIR}/munch$$" 1>&2; exit 1); MUNCHDIR="$TDIR/munch$$")
+TDIR=${TMPDIR:-/tmp}
+MUNCHDIR=`mktemp -d "${TDIR}/munchXXXXXXXXXX" 2>/dev/null` || MUNCHDIR=
+if [ -z "$MUNCHDIR" ]
+then
+    umask 077
+    MUNCHDIR="$TDIR/munch$$"
+    mkdir "$MUNCHDIR" || {
+	echo "Can't create temp directory: $MUNCHDIR" 1>&2
+	exit 1
+    }
+fi
 TMP=${MUNCHDIR}/munch.
 MAILDEBUGDIR=${MUNCHDIR-/tmp}
 if [ "X$MUNCHMAIL" != X ]
 then
-    exec 2> ${MAILDEBUGDIR}/munchlist.mail
+    exec 2> "${MAILDEBUGDIR}/munchlist.mail"
     echo "munchlist $*" 1>&2
     set -vx
 fi
-SORTTMP="-T ${TDIR}"			# !!SORTTMP!!
+SORTTMP="-T"				# !!SORTTMP!!
+sort_with_tmp ()
+{
+    if [ -n "$SORTTMP" ]
+    then
+	sort "$SORTTMP" "$TDIR" "$@"
+    else
+	sort "$@"
+    fi
+}
 
-DBDIR=${MUNCHDEBUGDIR-$MAILDEBUGDIR}
+DBDIR=${MUNCHDEBUGDIR:-$MAILDEBUGDIR}
+debug_copy=no
+
+# Preserve the traditional live hard-link snapshots when possible.  If the
+# debug directory is on another volume (or links are unavailable), later
+# checkpoints refresh ordinary copies instead.
+debug_alias ()
+{
+    if [ "$debug_copy" = yes ]
+    then
+	cp "$1" "$2"  ||  {
+	    echo "Can't save debug file: $2" 1>&2
+	    rm -rf "$MUNCHDIR"
+	    exit 1
+	}
+    else
+	ln "$1" "$2" 2>/dev/null  ||  {
+	    echo "Can't save debug file: $2" 1>&2
+	    rm -rf "$MUNCHDIR"
+	    exit 1
+	}
+    fi
+}
+
+debug_save ()
+{
+    if [ "$debug" = yes ]  &&  [ "$debug_copy" = yes ]  &&  [ -f "$1" ]
+    then
+	cp "$1" "$2"  ||  {
+	    echo "Can't save debug file: $2" 1>&2
+	    rm -rf "$MUNCHDIR"
+	    exit 1
+	}
+    fi
+}
 
 # Detect MS-DOS systems and arrange to use their silly suffix system
 if [ -z "$COMSPEC$ComSpec" ]
@@ -295,6 +347,7 @@
 export LC_MESSAGES LC_MONETARY LC_NUMERIC LC_TIME
 
 debug=no
+keep_debug=no
 dictopt=
 langtabs=${LIBDIR}/!!DEFLANG!!
 convtabs=
@@ -325,7 +378,7 @@
 	    if [ ! -r "$langtabs" ]
 	    then
 		echo "Can't open language table '$2'" 1>&2
-		rm -rf $MUNCHDIR
+		rm -rf "$MUNCHDIR"
 		exit 1
 	    fi
 	    shift
@@ -339,7 +392,7 @@
 		convtabs="${LIBDIR}/$2"
 	    else
 		echo "Can't open conversion language table '$2'" 1>&2
-		rm -rf $MUNCHDIR
+		rm -rf "$MUNCHDIR"
 		exit 1
 	    fi
 	    shift
@@ -351,6 +404,7 @@
 	    ;;
 	-D)
 	    debug=yes
+	    keep_debug=yes
 	    ;;
 	-T)
 	    icflags="-T $2"
@@ -373,7 +427,7 @@
 	-*)
 	    echo 'Usage: munchlist [-l lang] [-c lang] [-T suff] [-s hashfile] [-D] [-w chars] [-v] [file] ...' \
 	      1>&2
-	    rm -rf $MUNCHDIR
+	    rm -rf "$MUNCHDIR"
 	    exit 2
 	    ;;
 	*)
@@ -387,7 +441,7 @@
     verbose=true
     debug=yes
 fi
-trap "rm -rf $MUNCHDIR; exit 1" 1 2 13 15
+trap 'rm -rf "$MUNCHDIR"; exit 1' 1 2 13 15
 #
 # Names of temporary files.  This is just to make the code a little easier
 # to read.
@@ -409,36 +463,61 @@
 # Ispell insists that hash files have a "!!HASHSUFFIX!!" suffix
 FAKEHASH=${TMP}o!!HASHSUFFIX!!
 AWKSCRIPT=${TMP}p
+FINALUNSORTED=${TMP}q
+FINALOUTPUT=${TMP}r
+legal_base_saved=no
 if [ "$debug" = yes ]
 then
-    touch $EXPANDEDINPUT $STRIPPEDINPUT $CRUNCHEDINPUT $PRODUCTLIST \
-      $EXPANDEDPAIRS $LEGALFLAGLIST $JOINEDPAIRS $MINIMALAFFIXES \
-      $CROSSROOTS $CROSSEXPANDED $CROSSPAIRS $CROSSILLEGAL $ILLEGALCOMBOS \
-      $FAKEDICT $FAKEHASH $AWKSCRIPT
-    rm -f ${DBDIR}/EXPANDEDINPUT ${DBDIR}/STRIPPEDINPUT \
-      ${DBDIR}/CRUNCHEDINPUT ${DBDIR}/PRODUCTLIST ${DBDIR}/EXPANDEDPAIRS \
-      ${DBDIR}/LEGALFLAGLIST ${DBDIR}/JOINEDPAIRS ${DBDIR}/MINIMALAFFIXES \
-      ${DBDIR}/CROSSROOTS ${DBDIR}/CROSSEXPANDED ${DBDIR}/CROSSPAIRS \
-      ${DBDIR}/CROSSILLEGAL ${DBDIR}/ILLEGALCOMBOS ${DBDIR}/FAKEDICT \
-      ${DBDIR}/FAKEHASH!!HASHSUFFIX!! ${DBDIR}/AWKSCRIPT \
-      ${DBDIR}/CROSSROOTS.[0-9]* ${DBDIR}/CROSSEXP.[0-9]* \
-      ${DBDIR}/CROSSPAIRS.[0-9]* ${DBDIR}/CROSSILLEGAL.[0-9]*
-    ln $EXPANDEDINPUT ${DBDIR}/EXPANDEDINPUT
-    ln $STRIPPEDINPUT ${DBDIR}/STRIPPEDINPUT
-    ln $CRUNCHEDINPUT ${DBDIR}/CRUNCHEDINPUT
-    ln $PRODUCTLIST ${DBDIR}/PRODUCTLIST
-    ln $EXPANDEDPAIRS ${DBDIR}/EXPANDEDPAIRS
-    ln $LEGALFLAGLIST ${DBDIR}/LEGALFLAGLIST
-    ln $JOINEDPAIRS ${DBDIR}/JOINEDPAIRS
-    ln $MINIMALAFFIXES ${DBDIR}/MINIMALAFFIXES
-    ln $CROSSROOTS ${DBDIR}/CROSSROOTS
-    ln $CROSSEXPANDED ${DBDIR}/CROSSEXPANDED
-    ln $CROSSPAIRS ${DBDIR}/CROSSPAIRS
-    ln $CROSSILLEGAL ${DBDIR}/CROSSILLEGAL
-    ln $ILLEGALCOMBOS ${DBDIR}/ILLEGALCOMBOS
-    ln $FAKEDICT ${DBDIR}/FAKEDICT
-    ln $FAKEHASH ${DBDIR}/FAKEHASH!!HASHSUFFIX!!
-    ln $AWKSCRIPT ${DBDIR}/AWKSCRIPT
+    if [ ! -d "$DBDIR" ]
+    then
+	echo "Debug directory does not exist: $DBDIR" 1>&2
+	rm -rf "$MUNCHDIR"
+	exit 1
+    fi
+    touch "$EXPANDEDINPUT" "$STRIPPEDINPUT" "$CRUNCHEDINPUT" \
+      "$PRODUCTLIST" "$EXPANDEDPAIRS" "$LEGALFLAGLIST" "$JOINEDPAIRS" \
+      "$MINIMALAFFIXES" "$CROSSROOTS" "$CROSSEXPANDED" "$CROSSPAIRS" \
+      "$CROSSILLEGAL" "$ILLEGALCOMBOS" "$FAKEDICT" "$FAKEHASH" \
+      "$AWKSCRIPT"
+    linksource="${MUNCHDIR}/.munchlist-link-source.$$"
+    linktest="${DBDIR}/.munchlist-link-test.$$"
+    rm -f "$linksource" "$linktest"
+    : > "$linksource"
+    if ln "$linksource" "$linktest" 2>/dev/null \
+      &&  echo link-test > "$linksource" \
+      &&  [ "`cat "$linktest" 2>/dev/null`" = link-test ]
+    then
+	rm -f "$linksource" "$linktest"
+    else
+	rm -f "$linksource" "$linktest"
+	debug_copy=yes
+    fi
+    rm -f "${DBDIR}/EXPANDEDINPUT" "${DBDIR}/STRIPPEDINPUT" \
+      "${DBDIR}/CRUNCHEDINPUT" "${DBDIR}/PRODUCTLIST" \
+      "${DBDIR}/EXPANDEDPAIRS" "${DBDIR}/LEGALFLAGLIST" \
+      "${DBDIR}/JOINEDPAIRS" "${DBDIR}/MINIMALAFFIXES" \
+      "${DBDIR}/CROSSROOTS" "${DBDIR}/CROSSEXPANDED" \
+      "${DBDIR}/CROSSPAIRS" "${DBDIR}/CROSSILLEGAL" \
+      "${DBDIR}/ILLEGALCOMBOS" "${DBDIR}/FAKEDICT" \
+      "${DBDIR}/FAKEHASH!!HASHSUFFIX!!" "${DBDIR}/AWKSCRIPT" \
+      "${DBDIR}"/CROSSROOTS.[0-9]* "${DBDIR}"/CROSSEXP.[0-9]* \
+      "${DBDIR}"/CROSSPAIRS.[0-9]* "${DBDIR}"/CROSSILLEGAL.[0-9]*
+    debug_alias "$EXPANDEDINPUT" "${DBDIR}/EXPANDEDINPUT"
+    debug_alias "$STRIPPEDINPUT" "${DBDIR}/STRIPPEDINPUT"
+    debug_alias "$CRUNCHEDINPUT" "${DBDIR}/CRUNCHEDINPUT"
+    debug_alias "$PRODUCTLIST" "${DBDIR}/PRODUCTLIST"
+    debug_alias "$EXPANDEDPAIRS" "${DBDIR}/EXPANDEDPAIRS"
+    debug_alias "$LEGALFLAGLIST" "${DBDIR}/LEGALFLAGLIST"
+    debug_alias "$JOINEDPAIRS" "${DBDIR}/JOINEDPAIRS"
+    debug_alias "$MINIMALAFFIXES" "${DBDIR}/MINIMALAFFIXES"
+    debug_alias "$CROSSROOTS" "${DBDIR}/CROSSROOTS"
+    debug_alias "$CROSSEXPANDED" "${DBDIR}/CROSSEXPANDED"
+    debug_alias "$CROSSPAIRS" "${DBDIR}/CROSSPAIRS"
+    debug_alias "$CROSSILLEGAL" "${DBDIR}/CROSSILLEGAL"
+    debug_alias "$ILLEGALCOMBOS" "${DBDIR}/ILLEGALCOMBOS"
+    debug_alias "$FAKEDICT" "${DBDIR}/FAKEDICT"
+    debug_alias "$FAKEHASH" "${DBDIR}/FAKEHASH!!HASHSUFFIX!!"
+    debug_alias "$AWKSCRIPT" "${DBDIR}/AWKSCRIPT"
 fi
 #
 # Create a dummy dictionary to hold a compiled copy of the language
@@ -449,9 +528,9 @@
 	convtabs="$langtabs"
 	;;
 esac
-echo 'QQQQQQQQ' > $FAKEDICT
-$BUILDHASH -s $FAKEDICT $convtabs $FAKEHASH \
-  ||  (echo "Couldn't create fake hash file" 1>&2; rm -rf $MUNCHDIR; exit 1) \
+echo 'QQQQQQQQ' > "$FAKEDICT"
+$BUILDHASH -s "$FAKEDICT" $convtabs "$FAKEHASH" \
+  ||  (echo "Couldn't create fake hash file" 1>&2; rm -rf "$MUNCHDIR"; exit 1) \
   ||  exit 1
 #
 # Figure out how 'sort' sorts signed fields, for arguments to ijoin.
@@ -492,15 +571,15 @@
 fi
 if [ $# -eq 0 ]
 then
-    $ISPELL "$wchars" -e1 -d $FAKEHASH -p /dev/null | tr " " '
+    $ISPELL "$wchars" -e1 -d "$FAKEHASH" -p /dev/null | tr " " '
 '
 else
-    cat "$@" | $ISPELL "$wchars" -e1 -d $FAKEHASH -p /dev/null | tr " " '
+    cat "$@" | $ISPELL "$wchars" -e1 -d "$FAKEHASH" -p /dev/null | tr " " '
 '
 fi \
-  | sort $SORTTMP -u $sortopts \
+  | sort_with_tmp -u $sortopts \
   | $COMBINE $icflags "$wchars" $langtabs \
-  | sort $SORTTMP -u > $EXPANDEDINPUT
+  | sort_with_tmp -u > "$EXPANDEDINPUT"
 #
 # If a conversion table existed, recreate the fake hash file with the
 # "real" language table.
@@ -509,13 +588,14 @@
     $langtabs)
 	;;
     *)
-	$BUILDHASH -s $FAKEDICT $langtabs $FAKEHASH \
+	$BUILDHASH -s "$FAKEDICT" $langtabs "$FAKEHASH" \
 	  ||  (echo "Couldn't create fake hash file" 1>&2; \
-		rm -rf $MUNCHDIR; exit 1) \
+		rm -rf "$MUNCHDIR"; exit 1) \
 	  ||  exit 1
 	;;
 esac
-rm -f ${FAKEDICT}*
+debug_save "$FAKEDICT" "${DBDIR}/FAKEDICT"
+rm -f "${FAKEDICT}"*
 #
 # If the -s (strip) option was specified, remove all
 # expanded words that are covered by the dictionary.  This produces
@@ -524,23 +604,24 @@
 #
 if [ "X$strip" = "Xno" ]
 then
-    rm -f $STRIPPEDINPUT
-    ln $EXPANDEDINPUT $STRIPPEDINPUT
+    rm -f "$STRIPPEDINPUT"
+    cp "$EXPANDEDINPUT" "$STRIPPEDINPUT"
     if [ "$debug" = yes ]
     then
-	rm -f ${DBDIR}/STRIPPEDINPUT
-	ln $STRIPPEDINPUT ${DBDIR}/STRIPPEDINPUT
+	rm -f "${DBDIR}/STRIPPEDINPUT"
+	debug_alias "$STRIPPEDINPUT" "${DBDIR}/STRIPPEDINPUT"
     fi
 else
     $verbose  &&  echo "Stripping words already in the dictionary." 1>&2
-    $ISPELL "$wchars" -l $dictopt -p /dev/null < $EXPANDEDINPUT \
-      > $STRIPPEDINPUT
+    $ISPELL "$wchars" -l $dictopt -p /dev/null < "$EXPANDEDINPUT" \
+      > "$STRIPPEDINPUT"
 fi
+debug_save "$STRIPPEDINPUT" "${DBDIR}/STRIPPEDINPUT"
 #
 # Figure out what the flag-marking character is.
 #
 $verbose  &&  echo "Finding flag marker." 1>&2
-flagmarker=`$ISPELL -D -d $FAKEHASH \
+flagmarker=`$ISPELL -D -d "$FAKEHASH" \
   | sed -n -e '/^flagmarker/s/flagmarker //p'`
 case "$flagmarker" in
     \\*)
@@ -549,7 +630,7 @@
 esac    
 #
 # Munch the input to generate roots and affixes ($ISPELL -c).  We are
-# only interested in words that have at least one affix (egrep $flagmarker);
+# only interested in words that have at least one affix (grep -E $flagmarker);
 # the next step will pick up the rest.  Some of the roots are illegal.  We
 # use join to restrict the output to those root words that are found
 # in the original dictionary.
@@ -561,11 +642,11 @@
 else
     sortopts='+0 -1 +1'
 fi
-$ISPELL "$wchars" -c -W0 -d $FAKEHASH -p /dev/null < $STRIPPEDINPUT \
+$ISPELL "$wchars" -c -W0 -d "$FAKEHASH" -p /dev/null < "$STRIPPEDINPUT" \
   | tr " " '
 ' \
-  | egrep "$flagmarker" | sort $SORTTMP -u "-t$flagmarker" $sortopts \
-  | $JOIN $SIGNED "-t$flagmarker" - $EXPANDEDINPUT > $CRUNCHEDINPUT
+  | grep -E "$flagmarker" | sort_with_tmp -u "-t$flagmarker" $sortopts \
+  | $JOIN $SIGNED "-t$flagmarker" - "$EXPANDEDINPUT" > "$CRUNCHEDINPUT"
 #
 # We now have a list of legal roots, and of affixes that apply to the
 # root words.  However, it is possible for some affix flags to generate more
@@ -595,8 +676,8 @@
 else
     sortopts='+1'
 fi
-$ISPELL "$wchars" -e3 -d $FAKEHASH -p /dev/null < $CRUNCHEDINPUT \
-  | sort $SORTTMP $sortopts > $EXPANDEDPAIRS
+$ISPELL "$wchars" -e3 -d "$FAKEHASH" -p /dev/null < "$CRUNCHEDINPUT" \
+  | sort_with_tmp $sortopts > "$EXPANDEDPAIRS"
 #
 # Now we want to extract the lines in EXPANDEDPAIRS in which the second field
 # is *not* listed in the original dictionary EXPANDEDINPUT;  these illegal
@@ -613,11 +694,14 @@
 # BTW, I thought of using cut for the sed step (on systems that have it),
 # but it turns out that sed is faster!
 #
-$JOIN -j1 2 -o 1.1 1.2 $SIGNED $EXPANDEDPAIRS $EXPANDEDINPUT \
-  | sort $SORTTMP -u > $JOINEDPAIRS
+$JOIN -j1 2 -o 1.1 1.2 $SIGNED "$EXPANDEDPAIRS" "$EXPANDEDINPUT" \
+  | sort_with_tmp -u > "$JOINEDPAIRS"
 
-sort $SORTTMP -o $EXPANDEDPAIRS $EXPANDEDPAIRS
-sort $SORTTMP -o $CRUNCHEDINPUT $CRUNCHEDINPUT
+sort_with_tmp -o "$EXPANDEDPAIRS" "$EXPANDEDPAIRS"
+sort_with_tmp -o "$CRUNCHEDINPUT" "$CRUNCHEDINPUT"
+debug_save "$JOINEDPAIRS" "${DBDIR}/JOINEDPAIRS"
+debug_save "$EXPANDEDPAIRS" "${DBDIR}/EXPANDEDPAIRS"
+debug_save "$CRUNCHEDINPUT" "${DBDIR}/CRUNCHEDINPUT"
 
 $verbose  &&  echo 'Creating list of legal roots/flags.' 1>&2
 if $CRETIN_SORT
@@ -626,12 +710,12 @@
 else
     sortopts='+0f -1 +0'
 fi
-comm -13 $JOINEDPAIRS $EXPANDEDPAIRS \
-  | (sed -e 's; .*$;;' ; rm -f $JOINEDPAIRS $EXPANDEDPAIRS) \
+comm -13 "$JOINEDPAIRS" "$EXPANDEDPAIRS" \
+  | (sed -e 's; .*$;;' ; rm -f "$JOINEDPAIRS" "$EXPANDEDPAIRS") \
   | uniq \
-  | (comm -13 - $CRUNCHEDINPUT ; rm -f $CRUNCHEDINPUT) \
-  | sort $SORTTMP -u "-t$flagmarker" $sortopts \
-  | $COMBINE "$wchars" $langtabs > $LEGALFLAGLIST
+  | (comm -13 - "$CRUNCHEDINPUT" ; rm -f "$CRUNCHEDINPUT") \
+  | sort_with_tmp -u "-t$flagmarker" $sortopts \
+  | $COMBINE "$wchars" $langtabs > "$LEGALFLAGLIST"
 
 #
 # LEGALFLAGLIST now contains root/flag combinations that, when expanded,
@@ -658,31 +742,31 @@
 else
     sortopts='+0rn -1 +2'
 fi
-$ISPELL -D -d $FAKEHASH \
+$ISPELL -D -d "$FAKEHASH" \
   | sed -n -e '1,$s/:.*$//
     /^flagmarker/d
     /^prefixes/,/^suffixes/s/^  flag \*/p /p
     /^suffixes/,$s/^  flag \*/s /p' \
-  | sort $SORTTMP \
+  | sort_with_tmp \
   | uniq -c \
   | tr '	' ' ' \
-  | sort $SORTTMP $sortopts > $PRODUCTLIST
+  | sort_with_tmp $sortopts > "$PRODUCTLIST"
 
-if [ `egrep ' p ' $PRODUCTLIST | wc -l` -gt 0 \
-  -a `egrep ' s ' $PRODUCTLIST | wc -l` -gt 0 ]
+if [ `grep -F ' p ' "$PRODUCTLIST" | wc -l` -gt 0 \
+  -a `grep -F ' s ' "$PRODUCTLIST" | wc -l` -gt 0 ]
 then
     #
     # The language tables allow cross products.  See if LEGALFLAGLIST has
     # any roots with multiple cross-product flags.  Put them in CROSSROOTS.
     #
     $verbose  &&  echo 'Finding prefix and suffix flags.' 1>&2
-    preflags=`sed -n -e 's/^[ 0-9]*p //p' $PRODUCTLIST | tr -d '
+    preflags=`sed -n -e 's/^[ 0-9]*p //p' "$PRODUCTLIST" | tr -d '
 '`
-    sufflags=`sed -n -e 's/^[ 0-9]*s //p' $PRODUCTLIST | tr -d '
+    sufflags=`sed -n -e 's/^[ 0-9]*s //p' "$PRODUCTLIST" | tr -d '
 '`
-    egrep "$flagmarker.*[$preflags].*[$sufflags]|$flagmarker.*[$sufflags].*[$preflags]" \
-      $LEGALFLAGLIST \
-      > $CROSSROOTS
+    grep -E "$flagmarker.*[$preflags].*[$sufflags]|$flagmarker.*[$sufflags].*[$preflags]" \
+      "$LEGALFLAGLIST" \
+      > "$CROSSROOTS"
 
     #
     # We will need an awk script;  it's so big that it core-dumps my shell
@@ -699,15 +783,16 @@
 	    ;;
     esac
     $verbose  &&  echo 'Creating awk script.' 1>&2
+    MUNCH_ILLEGALCOMBOS="$ILLEGALCOMBOS"
+    export MUNCH_ILLEGALCOMBOS
     sed -e "s/PREFLAGS/$preflags/" -e "s/SUFFLAGS/$sufflags/" \
-      -e "s;ILLEGALCOMBOS;$ILLEGALCOMBOS;" \
       -e "s${sedchar}FLAGMARKER${sedchar}$flagmarker${sedchar}" \
-      > $AWKSCRIPT << 'ENDOFAWKSCRIPT'
+      > "$AWKSCRIPT" << 'ENDOFAWKSCRIPT'
 	BEGIN \
 	    {
 	    preflags = "PREFLAGS"
 	    sufflags = "SUFFLAGS"
-	    illegalcombos = "ILLEGALCOMBOS"
+	    illegalcombos = ENVIRON["MUNCH_ILLEGALCOMBOS"]
 	    flagmarker = "FLAGMARKER"
 	    pflaglen = length (preflags)
 	    for (i = 1;  i <= pflaglen;  i++)
@@ -768,9 +853,9 @@
 		}
 	    }
 ENDOFAWKSCRIPT
-    : > $ILLEGALCOMBOS
+    : > "$ILLEGALCOMBOS"
     dbnum=1
-    while [ -s $CROSSROOTS ]
+    while [ -s "$CROSSROOTS" ]
     do
 	#
 	# CROSSROOTS contains the roots whose cross-product expansions
@@ -786,31 +871,42 @@
 	else
 	    sortopts='+1'
 	fi
-	$ISPELL "$wchars" -e3 -d $FAKEHASH -p /dev/null < $CROSSROOTS \
-	  | sort $SORTTMP $sortopts > $CROSSEXPANDED
+	$ISPELL "$wchars" -e3 -d "$FAKEHASH" -p /dev/null < "$CROSSROOTS" \
+	  | sort_with_tmp $sortopts > "$CROSSEXPANDED"
 	#
 	# Now we join CROSSEXPANDED against EXPANDEDINPUT to produce
 	# CROSSPAIRS, and then comm that against CROSSEXPANDED to
 	# get CROSSILLEGAL, the list of illegal cross-product flag
 	# combinations.
 	#
-	$JOIN -j1 2 -o 1.1 1.2 $SIGNED $CROSSEXPANDED $EXPANDEDINPUT \
-	  | sort $SORTTMP -u > $CROSSPAIRS
+	$JOIN -j1 2 -o 1.1 1.2 $SIGNED "$CROSSEXPANDED" "$EXPANDEDINPUT" \
+	  | sort_with_tmp -u > "$CROSSPAIRS"
 
-	sort $SORTTMP -u -o $CROSSEXPANDED $CROSSEXPANDED
+	sort_with_tmp -u -o "$CROSSEXPANDED" "$CROSSEXPANDED"
 
 	$verbose \
 	  &&  echo "Finding illegal cross expansions (pass $dbnum)." 1>&2
-	comm -13 $CROSSPAIRS $CROSSEXPANDED \
+	comm -13 "$CROSSPAIRS" "$CROSSEXPANDED" \
 	  | sed -e 's; .*$;;' \
-	  | uniq > $CROSSILLEGAL
+	  | uniq > "$CROSSILLEGAL"
 
 	if [ "$debug" = yes ]
 	then
-	    mv $CROSSROOTS $DBDIR/CROSSROOTS.$dbnum
-	    ln $CROSSEXPANDED $DBDIR/CROSSEXP.$dbnum
-	    ln $CROSSPAIRS $DBDIR/CROSSPAIRS.$dbnum
-	    ln $CROSSILLEGAL $DBDIR/CROSSILLEGAL.$dbnum
+	    if [ "$debug_copy" = yes ]  &&  [ "$dbnum" -eq 1 ]
+	    then
+		debug_save "$CROSSROOTS" "${DBDIR}/CROSSROOTS"
+		debug_save "$CROSSEXPANDED" "${DBDIR}/CROSSEXPANDED"
+		debug_save "$CROSSPAIRS" "${DBDIR}/CROSSPAIRS"
+		debug_save "$CROSSILLEGAL" "${DBDIR}/CROSSILLEGAL"
+	    fi
+	    mv "$CROSSROOTS" "${DBDIR}/CROSSROOTS.$dbnum"  ||  {
+		echo "Can't save debug file: ${DBDIR}/CROSSROOTS.$dbnum" 1>&2
+		rm -rf "$MUNCHDIR"
+		exit 1
+	    }
+	    debug_alias "$CROSSEXPANDED" "${DBDIR}/CROSSEXP.$dbnum"
+	    debug_alias "$CROSSPAIRS" "${DBDIR}/CROSSPAIRS.$dbnum"
+	    debug_alias "$CROSSILLEGAL" "${DBDIR}/CROSSILLEGAL.$dbnum"
 	fi
 	#
 	# Now it is time to try to clear up the illegalities.  For 
@@ -828,10 +924,10 @@
 	# inside the awk script) so they can be removed from LEGALFLAGLIST
 	# later.
 	#
-	awk "-F$flagmarker" -f $AWKSCRIPT $CROSSILLEGAL > $CROSSROOTS
+	awk "-F$flagmarker" -f "$AWKSCRIPT" "$CROSSILLEGAL" > "$CROSSROOTS"
 	if [ "$debug" = yes ]
 	then
-	    rm -f $CROSSEXPANDED $CROSSPAIRS $CROSSILLEGAL
+	    rm -f "$CROSSEXPANDED" "$CROSSPAIRS" "$CROSSILLEGAL"
 	fi
 	dbnum=`expr $dbnum + 1`
 	if [ $dbnum -gt 100 ]
@@ -840,16 +936,17 @@
 	    if [ "X$MUNCHMAIL" != X ]
 	    then
 		(
-		ls -ld ${DBDIR}/[A-Z]*
-		cat ${MAILDEBUGDIR}/munchlist.mail
+		ls -ld "${DBDIR}"/[A-Z]*
+		cat "${MAILDEBUGDIR}/munchlist.mail"
 		) | mail -s 'Munchlist debug output' "$MUNCHMAIL"
-		rm -f ${MAILDEBUGDIR}/munchlist.mail
+		rm -f "${MAILDEBUGDIR}/munchlist.mail"
 	    fi
-	    rm -rf $MUNCHDIR
+	    rm -rf "$MUNCHDIR"
 	    exit 1
 	fi
     done
-    rm -f $CROSSEXPANDED $CROSSPAIRS $CROSSILLEGAL $AWKSCRIPT
+    debug_save "$AWKSCRIPT" "${DBDIR}/AWKSCRIPT"
+    rm -f "$CROSSEXPANDED" "$CROSSPAIRS" "$CROSSILLEGAL" "$AWKSCRIPT"
     #
     # Now we have, in ILLEGALCOMBOS, a list of root/flag combinations
     # that must be removed from LEGALFLAGLIST to get the final list
@@ -859,9 +956,9 @@
     # extraction, to get all flags for a given root on the same line so that
     # cross-products will come out right.
     #
-    if [ -s $ILLEGALCOMBOS ]
+    if [ -s "$ILLEGALCOMBOS" ]
     then
-	sort $SORTTMP -u -o $ILLEGALCOMBOS $ILLEGALCOMBOS
+	sort_with_tmp -u -o "$ILLEGALCOMBOS" "$ILLEGALCOMBOS"
 	$verbose  &&  echo 'Finding roots of cross expansions.' 1>&2
 	if $CRETIN_SORT
 	then
@@ -869,25 +966,33 @@
 	else
 	    sortopts='+0f -1 +0'
 	fi
-	sort $SORTTMP $LEGALFLAGLIST \
+	sort_with_tmp "$LEGALFLAGLIST" \
 	  | sed -e '/\/../{
 	      s;^\(.*\)/\(.\)\(.*\);\1/\2\
 \1/\3;
 	      P
 	      D
 	      }' \
-	  | comm -23 - $ILLEGALCOMBOS \
-	  | sort $SORTTMP -u "-t$flagmarker" $sortopts \
-	  | $COMBINE "$wchars" $langtabs > $CROSSROOTS
-	mv $CROSSROOTS $LEGALFLAGLIST
+	  | comm -23 - "$ILLEGALCOMBOS" \
+	  | sort_with_tmp -u "-t$flagmarker" $sortopts \
+	  | $COMBINE "$wchars" $langtabs > "$CROSSROOTS"
+	debug_save "$LEGALFLAGLIST" "${DBDIR}/LEGALFLAGLIST"
+	if [ "$debug_copy" = yes ]
+	then
+	    legal_base_saved=yes
+	fi
+	mv "$CROSSROOTS" "$LEGALFLAGLIST"
 	if [ "$debug" = yes ]
 	then
-	    rm -f ${DBDIR}/LEGALFLAGLIST1
-	    ln $LEGALFLAGLIST ${DBDIR}/LEGALFLAGLIST1
+	    rm -f "${DBDIR}/LEGALFLAGLIST1"
+	    debug_alias "$LEGALFLAGLIST" "${DBDIR}/LEGALFLAGLIST1"
 	fi
     fi
 fi
-rm -f $PRODUCTLIST $CROSSROOTS $ILLEGALCOMBOS $EXPANDEDINPUT
+debug_save "$PRODUCTLIST" "${DBDIR}/PRODUCTLIST"
+debug_save "$ILLEGALCOMBOS" "${DBDIR}/ILLEGALCOMBOS"
+debug_save "$EXPANDEDINPUT" "${DBDIR}/EXPANDEDINPUT"
+rm -f "$PRODUCTLIST" "$CROSSROOTS" "$ILLEGALCOMBOS" "$EXPANDEDINPUT"
 #
 
 # We now have (in LEGALFLAGLIST) a list of roots and flags which will
@@ -922,12 +1027,16 @@
     sortopts2='+1 -2'
     sortopts3='+0f -1 +0'
 fi
-$ISPELL "$wchars" -e4 -d $FAKEHASH -p /dev/null < $LEGALFLAGLIST \
-  | sort $SORTTMP $sortopts1 \
-  | sort $SORTTMP -um $sortopts2 \
+$ISPELL "$wchars" -e4 -d "$FAKEHASH" -p /dev/null < "$LEGALFLAGLIST" \
+  | sort_with_tmp $sortopts1 \
+  | sort_with_tmp -um $sortopts2 \
   | sed -e 's; .*$;;' \
-  | sort $SORTTMP -u "-t$flagmarker" $sortopts3 > $MINIMALAFFIXES
-rm -f $LEGALFLAGLIST
+  | sort_with_tmp -u "-t$flagmarker" $sortopts3 > "$MINIMALAFFIXES"
+if [ "$legal_base_saved" != yes ]
+then
+    debug_save "$LEGALFLAGLIST" "${DBDIR}/LEGALFLAGLIST"
+fi
+rm -f "$LEGALFLAGLIST"
 #
 # Now we're almost done.  MINIMALAFFIXES covers some (with luck, most)
 # of the words in STRIPPEDINPUT.  Now we must create a list of the remaining
@@ -944,32 +1053,78 @@
 else
     sortopts='+0f -1 +0'
 fi
-if [ -s $MINIMALAFFIXES ]
+if [ -s "$MINIMALAFFIXES" ]
 then
-    $BUILDHASH -s $MINIMALAFFIXES $langtabs $FAKEHASH > /dev/null \
+    $BUILDHASH -s "$MINIMALAFFIXES" $langtabs "$FAKEHASH" > /dev/null \
       ||  (echo "Couldn't create intermediate hash file" 1>&2;
-	rm -rf $MUNCHDIR;
+	rm -rf "$MUNCHDIR";
 	exit 1) \
       ||  exit 1
     if [ "$debug" = yes ]
     then
-	rm -f ${DBDIR}/MINIMALAFFIXES!!STATSUFFIX!!
-	ln $MINIMALAFFIXES!!STATSUFFIX!! ${DBDIR}/MINIMALAFFIXES.!!STATSUFFIX!!
+	rm -f "${DBDIR}/MINIMALAFFIXES!!STATSUFFIX!!"
+	debug_alias "${TMP}stat" \
+	  "${DBDIR}/MINIMALAFFIXES!!STATSUFFIX!!"
     fi
-    ($ISPELL "$wchars" -l -d $FAKEHASH -p /dev/null < $STRIPPEDINPUT; \
-	$COMBINE "$wchars" $langtabs < $MINIMALAFFIXES) \
-      | sort $SORTTMP "-t$flagmarker" -u $sortopts
+	output_status=0
+	if $ISPELL "$wchars" -l -d "$FAKEHASH" -p /dev/null \
+	  < "$STRIPPEDINPUT" > "$FINALUNSORTED"
+	then
+	    :
+	else
+	    output_status=1
+	fi
+	if [ "$output_status" -eq 0 ]  && \
+	  $COMBINE "$wchars" $langtabs < "$MINIMALAFFIXES" \
+	  >> "$FINALUNSORTED"
+	then
+	    :
+	else
+	    output_status=1
+	fi
+	if [ "$output_status" -eq 0 ]  && \
+	  sort_with_tmp "-t$flagmarker" -u $sortopts "$FINALUNSORTED" \
+	  > "$FINALOUTPUT"
+	then
+	    :
+	else
+	    output_status=1
+	fi
 else
     # MINIMALAFFIXES is empty;  just produce a sorted version of STRIPPEDINPUT
-    sort $SORTTMP "-t$flagmarker" -u $sortopts $STRIPPEDINPUT
+    if sort_with_tmp "-t$flagmarker" -u $sortopts "$STRIPPEDINPUT" \
+	> "$FINALOUTPUT"
+    then
+	output_status=0
+    else
+	output_status=1
+    fi
+fi
+if [ "$output_status" -eq 0 ]  &&  cat "$FINALOUTPUT"
+then
+    :
+else
+    echo "Couldn't generate complete output word list" 1>&2
+    output_status=1
 fi
+debug_save "$STRIPPEDINPUT" "${DBDIR}/STRIPPEDINPUT"
+debug_save "$CRUNCHEDINPUT" "${DBDIR}/CRUNCHEDINPUT"
+debug_save "$EXPANDEDPAIRS" "${DBDIR}/EXPANDEDPAIRS"
+debug_save "$JOINEDPAIRS" "${DBDIR}/JOINEDPAIRS"
+debug_save "$MINIMALAFFIXES" "${DBDIR}/MINIMALAFFIXES"
+debug_save "$FAKEHASH" "${DBDIR}/FAKEHASH!!HASHSUFFIX!!"
 if [ "X$MUNCHMAIL" != X ]
 then
     (
-    ls -ld ${DBDIR}/[A-Z]*
-    cat ${MAILDEBUGDIR}/munchlist.mail
+    ls -ld "${DBDIR}"/[A-Z]*
+    cat "${MAILDEBUGDIR}/munchlist.mail"
     ) | mail -s 'Munchlist debug output' "$MUNCHMAIL"
-    rm -f ${MAILDEBUGDIR}/munchlist.mail
+    rm -f "${MAILDEBUGDIR}/munchlist.mail"
+fi
+if [ "$keep_debug" = yes ]  &&  [ "$DBDIR" = "$MUNCHDIR" ]
+then
+    echo "Debug files left in $MUNCHDIR" 1>&2
+else
+    rm -rf "$MUNCHDIR"
 fi
-rm -rf $MUNCHDIR
-exit 0
+exit "$output_status"
