$NetBSD$

VSI C reserves __align as a keyword.

--- lib/malloc/scratch_buffer.h.orig	2026-01-11 19:39:04.000000000 +0000
+++ lib/malloc/scratch_buffer.h
@@ -66,7 +66,7 @@ struct scratch_buffer {
   void *data;    /* Pointer to the beginning of the scratch area.  */
   size_t length; /* Allocated space at the data pointer, in bytes.  */
-  union { max_align_t __align; char __c[1024]; } __space;
+  union { max_align_t __alignment; char __c[1024]; } __space;
 };
 
 /* Initializes *BUFFER so that BUFFER->data points to BUFFER->__space
