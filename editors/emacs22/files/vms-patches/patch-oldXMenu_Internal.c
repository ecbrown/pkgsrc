$NetBSD$

Give oldXMenu helpers that only update caller-owned state their actual void
return type.  Their historical implicit-int definitions fall off the end and
conflict with the corrected internal declarations on current VSI C.

--- oldXMenu/Internal.c.orig	2008-01-20 01:49:24.000000000 +0000
+++ oldXMenu/Internal.c
@@ -106,6 +106,7 @@ int (*_XMEventHandler)() = NULL;
  * _XMWinQueInit - Internal routine to initialize the window
  *		   queue.
  */
+void
 _XMWinQueInit()
 {
     /*
@@ -418,6 +419,7 @@ _XMGetSelectionPtr(p_ptr, s_num)
  * _XMRecomputeGlobals - Internal subroutine to recompute menu wide
  *			 global values.
  */
+void
 _XMRecomputeGlobals(display, menu)
     register Display *display; /*X11 display variable. */
     register XMenu *menu;	/* Menu object to compute from. */
@@ -813,6 +815,7 @@ _XMRecomputeSelection(display, menu, s_p
  *			recomputed before calling this routine or
  *			unpredictable results will follow.
  */
+void
 _XMTransToOrigin(display, menu, p_ptr, s_ptr, x_pos, y_pos, orig_x, orig_y)
     Display *display;		/* Not used. Included for consistency. */
     register XMenu *menu;	/* Menu being computed against. */
@@ -873,6 +876,7 @@ _XMTransToOrigin(display, menu, p_ptr, s
  * _XMRefreshPane - Internal subroutine to completely refresh
  *		    the contents of a pane.
  */
+void
 _XMRefreshPane(display, menu, pane)
     register Display *display;
     register XMenu *menu;
@@ -943,6 +947,7 @@ _XMRefreshPane(display, menu, pane)
  * _XMRefreshSelection - Internal subroutine that refreshes
  *			 a single selection window.
  */
+void
 _XMRefreshSelection(display, menu, select)
     register Display *display;
     register XMenu *menu;
