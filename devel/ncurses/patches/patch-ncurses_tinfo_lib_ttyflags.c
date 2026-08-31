$NetBSD$

VSI OpenVMS provides the POSIX termios structures and flags but explicitly
does not implement its terminal access functions.  Supply the subset ncurses
uses with native sense/set/read and typeahead QIOs.

--- ncurses/tinfo/lib_ttyflags.c.orig	2025-12-27 12:33:34.000000000 +0000
+++ ncurses/tinfo/lib_ttyflags.c
@@ -38,12 +38,269 @@
 
 #include <curses.priv.h>
 
+#ifdef __VMS
+#ifndef __NEW_STARLET
+#define __NEW_STARLET 1
+#endif
+#include <descrip.h>
+#include <iodef.h>
+#include <ssdef.h>
+#include <starlet.h>
+#include <stsdef.h>
+#include <ttdef.h>
+#include <tt2def.h>
+#include <unixio.h>
+#endif
+
 #ifndef CUR
 #define CUR SP_TERMTYPE
 #endif
 
 MODULE_ID("$Id: lib_ttyflags.c,v 1.40 2025/12/27 12:33:34 tom Exp $")
 
+#ifdef __VMS
+/*
+ * OpenVMS provides the POSIX termios types and flags, but not its terminal
+ * access routines.  Translate the subset ncurses needs to native terminal
+ * sense/set QIOs.  Input itself uses a QIO because the C RTL read() remains
+ * record-oriented even after the terminal is put into pass-through mode.
+ */
+typedef struct {
+    unsigned short status;
+    unsigned char txspeed;
+    unsigned char rxspeed;
+    unsigned char cr_fill;
+    unsigned char lf_fill;
+    unsigned short framing;
+} VMS_TERM_IOSB;
+
+typedef struct {
+    unsigned char class;
+    unsigned char type;
+    unsigned short page_width;
+    TTDEF ttdef;
+    TT2DEF tt2def;
+} VMS_TERM_CHARS;
+
+typedef struct {
+    unsigned short status;
+    unsigned short count;
+    unsigned short terminator;
+    unsigned short terminator_count;
+} VMS_READ_IOSB;
+
+#pragma member_alignment save
+#pragma nomember_alignment longword
+typedef struct {
+    unsigned short count;
+    unsigned char first;
+    unsigned char reserved;
+    unsigned int reserved2;
+} VMS_TYPEAHEAD;
+#pragma member_alignment restore
+
+#define VMS_IOSB(pointer) ((struct _iosb *) (void *) (pointer))
+
+static int
+vms_channel_for_fd(int fd, unsigned short *channel)
+{
+    char device[256];
+    char *name;
+    struct dsc$descriptor_s descriptor;
+    unsigned int status;
+
+    name = getname(fd, device, 1);
+    if (name == NULL) {
+	errno = EBADF;
+	return -1;
+    }
+    descriptor.dsc$w_length = (unsigned short) strlen(device);
+    descriptor.dsc$b_dtype = DSC$K_DTYPE_T;
+    descriptor.dsc$b_class = DSC$K_CLASS_S;
+    descriptor.dsc$a_pointer = device;
+    status = sys$assign(&descriptor, channel, 0, 0, 0);
+    if (!$VMS_STATUS_SUCCESS(status)) {
+	errno = EIO;
+	return -1;
+    }
+    return 0;
+}
+
+NCURSES_EXPORT(ssize_t)
+_nc_vms_read(int fd, void *buffer, size_t length)
+{
+    unsigned short channel;
+    unsigned int status;
+    VMS_READ_IOSB iosb;
+
+    if (!isatty(fd))
+	return read(fd, buffer, length);
+    if (vms_channel_for_fd(fd, &channel) < 0)
+	return -1;
+    memset(&iosb, 0, sizeof(iosb));
+    status = sys$qiow(0, channel,
+	IO$_READVBLK | IO$M_NOECHO | IO$M_NOFILTR,
+	VMS_IOSB(&iosb), 0, 0, buffer, length, 0, 0, 0, 0);
+    (void) sys$dassgn(channel);
+    if (!$VMS_STATUS_SUCCESS(status) || !$VMS_STATUS_SUCCESS(iosb.status)) {
+	errno = EIO;
+	return -1;
+    }
+    return (ssize_t) (iosb.count + iosb.terminator_count);
+}
+
+NCURSES_EXPORT(int)
+_nc_vms_typeahead(int fd)
+{
+    unsigned short channel;
+    unsigned int status;
+    VMS_TERM_IOSB iosb;
+    VMS_TYPEAHEAD typeahead;
+
+    if (!isatty(fd))
+	return -1;
+    if (vms_channel_for_fd(fd, &channel) < 0)
+	return -1;
+    memset(&iosb, 0, sizeof(iosb));
+    memset(&typeahead, 0, sizeof(typeahead));
+    status = sys$qiow(0, channel, IO$_SENSEMODE | IO$M_TYPEAHDCNT,
+	VMS_IOSB(&iosb), 0, 0, &typeahead, sizeof(typeahead), 0, 0, 0, 0);
+    (void) sys$dassgn(channel);
+    if (!$VMS_STATUS_SUCCESS(status) || !$VMS_STATUS_SUCCESS(iosb.status)) {
+	errno = EIO;
+	return -1;
+    }
+    return (int) typeahead.count;
+}
+
+NCURSES_EXPORT(int)
+tcgetattr(int fd, struct termios *term)
+{
+    unsigned short channel;
+    unsigned int status;
+    VMS_TERM_IOSB iosb;
+    VMS_TERM_CHARS chars;
+
+    if (term == NULL || !isatty(fd)) {
+	errno = EINVAL;
+	return -1;
+    }
+    if (vms_channel_for_fd(fd, &channel) < 0)
+	return -1;
+    memset(&iosb, 0, sizeof(iosb));
+    memset(&chars, 0, sizeof(chars));
+    status = sys$qiow(0, channel, IO$_SENSEMODE, VMS_IOSB(&iosb), 0, 0,
+	&chars, sizeof(chars), 0, 0, 0, 0);
+    (void) sys$dassgn(channel);
+    if (!$VMS_STATUS_SUCCESS(status) || !$VMS_STATUS_SUCCESS(iosb.status)) {
+	errno = EIO;
+	return -1;
+    }
+
+    memset(term, 0, sizeof(*term));
+    term->c_iflag = IGNBRK | ICRNL;
+    if (!chars.ttdef.tt$v_eightbit)
+	term->c_iflag |= ISTRIP;
+    if (chars.ttdef.tt$v_ttsync)
+	term->c_iflag |= IXON;
+    if (chars.ttdef.tt$v_hostsync)
+	term->c_iflag |= IXOFF;
+    term->c_oflag = OPOST | ONLCR;
+    term->c_cflag = CS8 | CREAD;
+    if (!chars.ttdef.tt$v_modem)
+	term->c_cflag |= CLOCAL;
+    if (!chars.ttdef.tt$v_noecho)
+	term->c_lflag |= ECHO;
+    if (!chars.ttdef.tt$v_passall && !chars.tt2def.tt2$v_pasthru)
+	term->c_lflag |= ISIG | ICANON | IEXTEN;
+    term->c_lflag |= ECHOE | ECHOK;
+    term->c_cc[VEOF] = 26;
+    term->c_cc[VERASE] = 127;
+    term->c_cc[VKILL] = 21;
+    term->c_cc[VINTR] = 3;
+    term->c_cc[VSTART] = 17;
+    term->c_cc[VSTOP] = 19;
+    term->c_ispeed = B9600;
+    term->c_ospeed = B9600;
+    return 0;
+}
+
+NCURSES_EXPORT(int)
+tcsetattr(int fd, int action, const struct termios *term)
+{
+    unsigned short channel;
+    unsigned int status;
+    VMS_TERM_IOSB iosb;
+    VMS_TERM_CHARS chars;
+
+    (void) action;
+    if (term == NULL || !isatty(fd)) {
+	errno = EINVAL;
+	return -1;
+    }
+    if (vms_channel_for_fd(fd, &channel) < 0)
+	return -1;
+    memset(&iosb, 0, sizeof(iosb));
+    memset(&chars, 0, sizeof(chars));
+    status = sys$qiow(0, channel, IO$_SENSEMODE, VMS_IOSB(&iosb), 0, 0,
+	&chars, sizeof(chars), 0, 0, 0, 0);
+    if (!$VMS_STATUS_SUCCESS(status) || !$VMS_STATUS_SUCCESS(iosb.status)) {
+	(void) sys$dassgn(channel);
+	errno = EIO;
+	return -1;
+    }
+    chars.ttdef.tt$v_eightbit = (term->c_iflag & ISTRIP) == 0;
+    chars.ttdef.tt$v_ttsync = (term->c_iflag & IXON) != 0;
+    chars.ttdef.tt$v_hostsync = (term->c_iflag & IXOFF) != 0;
+    chars.ttdef.tt$v_noecho = (term->c_lflag & ECHO) == 0;
+    chars.tt2def.tt2$v_pasthru =
+	(term->c_lflag & (ISIG | ICANON | IEXTEN)) == 0;
+    memset(&iosb, 0, sizeof(iosb));
+    status = sys$qiow(0, channel, IO$_SETMODE, VMS_IOSB(&iosb), 0, 0,
+	&chars, sizeof(chars), 0, 0, 0, 0);
+    (void) sys$dassgn(channel);
+    if (!$VMS_STATUS_SUCCESS(status) || !$VMS_STATUS_SUCCESS(iosb.status)) {
+	errno = EIO;
+	return -1;
+    }
+    return 0;
+}
+
+NCURSES_EXPORT(speed_t)
+cfgetispeed(const struct termios *term)
+{
+    return term->c_ispeed;
+}
+
+NCURSES_EXPORT(speed_t)
+cfgetospeed(const struct termios *term)
+{
+    return term->c_ospeed;
+}
+
+NCURSES_EXPORT(int)
+cfsetispeed(struct termios *term, speed_t speed)
+{
+    term->c_ispeed = speed;
+    return 0;
+}
+
+NCURSES_EXPORT(int)
+cfsetospeed(struct termios *term, speed_t speed)
+{
+    term->c_ospeed = speed;
+    return 0;
+}
+
+NCURSES_EXPORT(int)
+tcflush(int fd, int queue)
+{
+    (void) fd;
+    (void) queue;
+    return 0;
+}
+#endif /* __VMS */
+
 NCURSES_EXPORT(int)
 NCURSES_SP_NAME(_nc_get_tty_mode) (NCURSES_SP_DCLx TTY * buf)
 {
