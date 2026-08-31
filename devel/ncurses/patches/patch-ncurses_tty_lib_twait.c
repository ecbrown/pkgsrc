$NetBSD$

Poll the OpenVMS terminal driver's typeahead count.  C RTL poll and select do
not report terminal input readiness.

--- ncurses/tty/lib_twait.c.orig	2025-03-01 17:07:19.000000000 +0000
+++ ncurses/tty/lib_twait.c
@@ -177,7 +177,7 @@
     int count;
     int result = TW_NONE;
     TimeType t0;
-#if (USE_FUNC_POLL || HAVE_SELECT)
+#if (USE_FUNC_POLL || HAVE_SELECT) && !defined(__VMS)
     int fd;
 #endif
 
@@ -186,7 +186,8 @@
     int n;
 #endif
 
-#if USE_FUNC_POLL
+#if defined(__VMS)
+#elif USE_FUNC_POLL
 #define MIN_FDS 2
     struct pollfd fd_list[MIN_FDS];
     struct pollfd *fds = fd_list;
@@ -235,7 +236,29 @@
 	evl->result_flags = 0;
 #endif
 
-#if USE_FUNC_POLL
+#if defined(__VMS)
+    result = TW_NONE;
+    if (mode & TW_INPUT) {
+	int remaining = milliseconds;
+	for (;;) {
+	    int pending = _nc_vms_typeahead(sp->_ifd);
+	    int step;
+
+	    if (pending > 0) {
+		result = 1;
+		break;
+	    }
+	    if (remaining == 0)
+		break;
+	    step = (remaining < 0 || remaining > 10) ? 10 : remaining;
+	    (void) usleep((unsigned int) step * 1000U);
+	    if (remaining > 0)
+		remaining -= step;
+	}
+    } else if (milliseconds > 0) {
+	(void) usleep((unsigned int) milliseconds * 1000U);
+    }
+#elif USE_FUNC_POLL
     memset(fd_list, 0, sizeof(fd_list));
 
 #ifdef NCURSES_WGETCH_EVENTS
@@ -486,7 +509,9 @@
     if (result != 0) {
 	if (result > 0) {
 	    result = 0;
-#if USE_FUNC_POLL
+#if defined(__VMS)
+	    result = TW_INPUT;
+#elif USE_FUNC_POLL
 	    for (count = 0; count < MIN_FDS; count++) {
 		if ((mode & (1 << count))
 		    && (fds[count].revents & POLLIN)) {
@@ -512,7 +537,7 @@
 	result |= TW_EVENT;
 #endif
 
-#if USE_FUNC_POLL
+#if USE_FUNC_POLL && !defined(__VMS)
 #ifdef NCURSES_WGETCH_EVENTS
     if (fds != fd_list)
 	free((char *) fds);
