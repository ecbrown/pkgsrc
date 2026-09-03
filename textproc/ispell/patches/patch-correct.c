$NetBSD$

Fix multibyte-character output and guard malformed dictionary variant
chains.  Avoid readonly, which is a reserved type qualifier in VSI C.

Never replace the original file when an external deformatter emits a
different number of bytes than its source.

--- correct.c.orig
+++ correct.c
@@ -389,10 +389,16 @@
 	    {
 	    if (sourcefile != NULL)
 		{
-		while (fgets ((char *) contextbufs[0], sizeof contextbufs[0],
-		    sourcefile)
-		  != NULL)
-		    (void) fputs ((char *) contextbufs[0], outfile);
+		ch = getc (sourcefile);
+		if (ch != EOF  ||  ferror (sourcefile))
+		    {
+		    (void) fprintf (stderr, CORR_C_SHORT_FILTER,
+		      MAYBE_CR (stderr));
+		    (void) sleep ((unsigned) 2);
+		    filter_error = 1;
+		    changes = 0;		/* Never replace with partial data */
+		    xflag = 0;		/* Preserve file backup */
+		    }
 		}
 	    break;
 	    }
@@ -432,6 +438,8 @@
 		(void) fprintf (stderr, CORR_C_SHORT_SOURCE,
 		  MAYBE_CR (stderr));
 		(void) sleep ((unsigned) 2);
+		filter_error = 1;
+		changes = 0;		/* Never replace with partial data */
 		xflag = 0;		/* Preserve file backup */
 		break;
 		}
@@ -474,7 +482,7 @@
     (void) printf ("    %s", (char *) ctok);
     if (currentfile)
 	(void) printf (CORR_C_FILE_LABEL, currentfile);
-    if (readonly)
+    if (ispell_readonly)
 	(void) printf (" %s", CORR_C_READONLY);
     (void) printf ("\r\n\r\n");
 
@@ -627,7 +635,7 @@
 		}
 	    case 'r': case 'R':
 		imove (li - 1, 0);
-		if (readonly)
+		if (ispell_readonly)
 		    {
 		    (void) putchar (7);
 		    (void) printf ("%s ", CORR_C_READONLY);
@@ -681,7 +689,7 @@
 		      filteredbuf + (begintoken - contextbufs[0]),
 		      ctok, curchar, 1);
 		    ierase ();
-		    if (readonly)
+		    if (ispell_readonly)
 			{
 			imove (li - 1, 0);
 			(void) putchar (7);
@@ -769,11 +777,14 @@
 	ichar = SET_SIZE + laststringch;
     else
 	ichar = chartoichar (ch);
-    if (!vflag  &&  iswordch (ichar)  &&  len == 1)
+    if (!vflag  &&  iswordch (ichar)  &&  len >= 1)
 	{
-	if (output)
-	    (void) putchar (ch);
-	(*cp)++;
+	for (i = 0; i < len; ++i)
+	    {
+		if (output)
+			(void) putchar (**cp);
+		(*cp)++;
+	    }
 	return 1;
 	}
     if (ch == '\t')
@@ -1536,7 +1547,7 @@
 		return;
 		}
 	    }
-	while (dent->flagfield & MOREVARIANTS)
+	while ((dent->flagfield & MOREVARIANTS) && dent->next != NULL)
 	    {
 	    dent = dent->next;
 	    if (captype (dent->flagfield) == FOLLOWCASE
@@ -1577,7 +1588,7 @@
     len = icharlen (p);
     if (dent->flagfield & MOREVARIANTS)
 	dent = dent->next;	/* Skip place-holder entry */
-    for (  ;  ;  )
+    for (  ; dent != NULL ;  )
 	{
 	if (flagsareok (dent))
 	    {
@@ -1799,7 +1810,8 @@
 		}
 	    else if (filteredbuf[0] == '#')
 		{
-		treeoutput ();
+		if (treeoutput () != 0)
+		    done (1);
 		insidehtml = 0;
 		math_mode = 0;
 		LaTeX_Mode = 'P';
@@ -1932,6 +1944,13 @@
     g = grepstr;
     for (s = string; *s != '\0'; s++)
 	{
+	if (g + (*s == '*' ? 2 : 1) >= grepstr + sizeof grepstr)
+	    {
+	    (void) fprintf (stderr, CORR_C_LOOKUP_TOO_LONG,
+	      MAYBE_CR (stderr));
+	    (void) sleep ((unsigned) 2);
+	    return;
+	    }
 	if (*s == '*')
 	    {
 #ifndef REGEX_LOOKUP
@@ -1954,6 +1973,13 @@
 	if (!wild && look)
 	    {
 	    /* no wild and look(1) is possibly available */
+	    if (strlen (LOOK) + strlen (grepstr) + strlen (WORDS) + 3
+	      >= sizeof cmd)
+		{
+		(void) fprintf (stderr, CORR_C_LOOKUP_TOO_LONG,
+		  MAYBE_CR (stderr));
+		return;
+		}
 	    (void) sprintf (cmd, "%s %s %s", LOOK, grepstr, WORDS);
 	    if (shellescape (cmd))
 		return;
@@ -1963,7 +1989,22 @@
 #endif /* LOOK */
 	/* string has wild card chars or look not avail */
 	if (!wild)
+	    {
+	    if (strlen (grepstr) + 2 >= sizeof grepstr)
+		{
+		(void) fprintf (stderr, CORR_C_LOOKUP_TOO_LONG,
+		  MAYBE_CR (stderr));
+		return;
+		}
 	    (void) strcat (grepstr, ".*");	/* work like look */
+	    }
+	if (strlen (EGREPCMD) + strlen (grepstr) + strlen (WORDS) + 5
+	  >= sizeof cmd)
+	    {
+	    (void) fprintf (stderr, CORR_C_LOOKUP_TOO_LONG,
+	      MAYBE_CR (stderr));
+	    return;
+	    }
 	(void) sprintf (cmd, "%s ^%s$ %s", EGREPCMD, grepstr, WORDS);
 	(void) shellescape (cmd);
 #endif /* REGEX_LOOKUP */
