$NetBSD$

Use ncurses' normal setup and attribute paths on OpenVMS when Lynx is linked
with ncurses rather than the C RTL's legacy curses implementation.  Retain the
native terminal I/O setup and avoid colliding with ncurses' typeahead function.

--- src/LYCurses.c.orig	2024-04-16 21:43:44.000000000 +0000
+++ src/LYCurses.c
@@ -222,7 +222,7 @@ void lynx_setup_colors(void)
 
 #ifdef FANCY_CURSES
 
-#ifndef VMS
+#if !defined(VMS) || defined(NCURSES)
 /* *INDENT-OFF* */
 /* definitions for the mono attributes we can use */
 static struct {
@@ -251,7 +251,7 @@ static int lookup_color_attr(int code)
     }
     return 0;
 }
-#endif /* VMS */
+#endif /* !VMS || NCURSES */
 
 #ifdef USE_COLOR_STYLE
 static char *attr_to_string(int code)
@@ -306,7 +306,7 @@ void LYbox(WINDOW *win, int formfield)
 		   (unsigned) win->height,
 		   (unsigned) win->width + 4);
 #else
-#ifdef VMS
+#if defined(VMS) && !defined(NCURSES)
     /*
      * This should work for VAX-C and DEC-C, since they both have the same
      * win._max_y and win._max_x members -TD
@@ -331,7 +331,7 @@ void LYbox(WINDOW *win, int formfield)
     for (i = 1; i < win->_max_x; i++)
 	waddch(win, 'q');
     waddstr(win, "j\017");
-#else /* !VMS */
+#else /* !VMS || NCURSES */
     int boxvert, boxhori;
 
     UCSetBoxChars(current_char_set, &boxvert, &boxhori, BOXVERT, BOXHORI);
@@ -374,7 +374,7 @@ void LYbox(WINDOW *win, int formfield)
     if (formfield)
 	wcurses_css(win, "frame", ABS_OFF);
 #endif
-#endif /* VMS */
+#endif /* VMS && !NCURSES */
     wrefresh(win);
 #endif /* USE_SLANG */
 }
@@ -913,7 +913,7 @@ static BOOL create_fake_win(void)
 #define delete_fake_win()	/* nothing */
 #endif
 
-#if !defined(VMS) && !defined(USE_SLANG)
+#if (!defined(VMS) || defined(NCURSES)) && !defined(USE_SLANG)
 #if defined(NCURSES) && defined(HAVE_RESIZETERM)
 
 static SCREEN *LYscreen = NULL;
@@ -960,7 +960,7 @@ static void LYDELSCR(void)
 #define LYDELSCR()		/* nothing */
 #endif /* HAVE_NEWTERM   */
 
-#else /* !defined(VMS) && !defined(USE_SLANG) */
+#else /* (defined(VMS) && !defined(NCURSES)) || defined(USE_SLANG) */
 
     /*
      * Provide last recourse definitions of LYscreen and LYDELSCR for
@@ -969,7 +969,7 @@ static void LYDELSCR(void)
      */
 #define LYscreen TRUE
 #define LYDELSCR()		/* nothing */
-#endif /* !defined(VMS) && !defined(USE_SLANG) */
+#endif /* (!defined(VMS) || defined(NCURSES)) && !defined(USE_SLANG) */
 
 #if defined(PDCURSES) && defined(PDC_BUILD) && PDC_BUILD >= 2401
 int saved_scrsize_x = 0;
@@ -1310,13 +1310,13 @@ void start_curses(void)
 #else /* USE_SLANG; Now using curses: */
     int keypad_on = 0;
 
-#ifdef VMS
+#if defined(VMS) && !defined(NCURSES)
     /*
      * If we are VMS then do initscr() every time start_curses() is called!
      */
     CTRACE((tfp, "Screen size: initscr()\n"));
     initscr();			/* start curses */
-#else /* Unix: */
+#else /* !VMS || NCURSES: */
 
 #if defined(HAVE_TTYNAME)
     if (isatty(fileno(stdout)) && LYReopenInput() < 0) {
@@ -1537,9 +1537,9 @@ void start_curses(void)
 #ifdef __DJGPP__
     _eth_init();
 #endif /* __DJGPP__ */
-#endif /* not VMS */
+#endif /* VMS && !NCURSES */
 
-#ifdef VMS
+#if defined(VMS) && !defined(NCURSES)
     crmode();
     raw();
 #else
@@ -1549,7 +1549,7 @@ void start_curses(void)
     crmode();
 #endif /* HAVE_CBREAK */
     signal(SIGINT, cleanup_sig);
-#endif /* VMS */
+#endif /* VMS && !NCURSES */
 
     noecho();
 
@@ -1847,7 +1847,7 @@ static void size_change(int sig GCC_UNUS
 #endif /* !VMS */
 }
 
-#ifdef VMS
+#if defined(VMS) && !defined(NCURSES)
 
 #ifdef USE_SLANG
 extern void longname(char *, char *);
@@ -1916,7 +1916,7 @@ BOOLEAN setup(char *terminal)
     return (TRUE);
 }
 
-#else /* Not VMS: */
+#else /* !VMS || NCURSES: */
 
 /*
  * Check terminal type, start curses & setup terminal.
@@ -1966,6 +1966,9 @@ BOOLEAN setup(char *terminal)
 	LYSleepMsg();
     }
 
+#ifdef VMS
+    ttopen();
+#endif
     start_curses();
 
 #ifdef HAVE_TTYTYPE
@@ -2039,7 +2042,7 @@ BOOLEAN setup(char *terminal)
 #endif /* USE_COLOR_TABLE */
 #endif /* !USE_COLOR_STYLE */
 #endif /* FANCY_CURSES */
-#endif /* VMS */
+#endif /* VMS && !NCURSES */
 
 /* Use this rather than the 'wprintw()' function to write a blank-padded
  * string to the given window, since someone's asserted that printw doesn't
@@ -2684,7 +2687,7 @@ static int waiting_for_input(int fd)
  *		Check whether a keystroke has been entered, and return
  *		 it, or -1 if none was entered.
  */
-int typeahead(void)
+int lynx_typeahead(void)
 {
     int status;
     unsigned short iosb[4];
