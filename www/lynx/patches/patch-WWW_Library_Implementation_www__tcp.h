$NetBSD$

Use the TCP/IP Services socket entry point for reads.  The C RTL read()
entry point raises an access violation on the first socket read on VSI
OpenVMS x86-64, while this block already uses send() for socket writes.
Also override NETREAD: with UNIX intentionally undefined, its generic
definition recursively calls HTDoRead until the stack guard is reached.

--- WWW/Library/Implementation/www_tcp.h.orig	2024-03-17 23:04:27.000000000 +0000
+++ WWW/Library/Implementation/www_tcp.h
@@ -419,6 +419,13 @@ VAX/VMS
 
 #ifdef TCPIP_SERVICES
 /*
+ * Use socket I/O rather than the C RTL file-descriptor entry points.
+ */
+#undef SOCKET_READ
+#define SOCKET_READ(s,b,l) recv((s),(char *)(b),(l),0)
+#undef NETREAD
+#define NETREAD(s,b,l) recv((s),(char *)(b),(l),0)
+/*
  * TCPIP Services has all of the entrypoints including ioctl().
  */
 #undef NETWRITE
