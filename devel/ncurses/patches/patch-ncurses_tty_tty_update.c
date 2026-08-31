$NetBSD$

Use native OpenVMS QIO helpers for terminal reads and readiness checks.

--- ncurses/tty/tty_update.c.orig	2025-12-27 12:34:03.000000000 +0000
+++ ncurses/tty/tty_update.c
@@ -158,7 +158,12 @@
     *(s = buf) = 0;
     do {
 	int ask = sizeof(buf) - 1 - (s - buf);
-	int got = read(0, s, ask);
+	int got;
+#ifdef __VMS
+	got = (int) _nc_vms_read(0, s, (size_t) ask);
+#else
+	got = (int) read(0, s, (size_t) ask);
+#endif
 	if (got == 0)
 	    break;
 	s += got;
@@ -387,7 +392,11 @@
 	return FALSE;
 
     if (SP_PARM->_checkfd >= 0) {
-#if USE_FUNC_POLL
+#if defined(__VMS)
+	if (_nc_vms_typeahead(SP_PARM->_checkfd) > 0) {
+	    have_pending = TRUE;
+	}
+#elif USE_FUNC_POLL
 	struct pollfd fds[1];
 	fds[0].fd = SP_PARM->_checkfd;
 	fds[0].events = POLLIN;
