#ifdef vms
#undef vms
#endif

/* GNU libc requires ORDINARY_LINK so that its own crt0 is used.
   Linux is an exception because it uses a funny variant of GNU libc.  */
#ifdef __GNU_LIBRARY__
#ifndef LINUX
#define ORDINARY_LINK
#endif
#endif

/* Some machines don't find the standard C libraries in the usual place.  */
#ifndef ORDINARY_LINK
#ifndef LIB_STANDARD
#define LIB_STANDARD -lc
#endif
#else
#ifndef LIB_STANDARD
#define LIB_STANDARD
#endif
#endif

/* Unless inhibited or changed, use -lg to link for debugging.  */
#ifndef LIBS_DEBUG
#define LIBS_DEBUG
#endif

/* Some s/*.h files define this to request special libraries.  */
#ifndef LIBS_SYSTEM
#define LIBS_SYSTEM
#endif

/* Some m/*.h files define this to request special libraries.  */
#ifndef LIBS_MACHINE
#define LIBS_MACHINE
#endif

#ifndef LIB_MATH
# ifdef LISP_FLOAT_TYPE
#  define LIB_MATH -lm
# else /* ! defined (LISP_FLOAT_TYPE) */
#  define LIB_MATH
# endif /* ! defined (LISP_FLOAT_TYPE) */
#endif /* LIB_MATH */

#ifndef LIBX10_MACHINE
#define LIBX10_MACHINE
#endif

#ifndef LIBX11_MACHINE
#define LIBX11_MACHINE
#endif

#ifndef LIBX10_SYSTEM
#define LIBX10_SYSTEM
#endif

#ifndef LIBX11_SYSTEM
#define LIBX11_SYSTEM
#endif

#ifndef LIB_X11_LIB
#define LIB_X11_LIB -lX11
#endif

#ifdef HAVE_X_WINDOWS
#ifdef HAVE_X_MENU

/* Include xmenu.obj in the list of X object files.  */
#define XOBJ ,xterm.obj, xfns.obj, xfaces.obj, xmenu.obj, xselect.obj, xrdb.obj

/* The X Menu stuff is present in the X10 distribution, but missing
   from X11.  If we have X10, just use the installed library;
   otherwise, use our own copy.  */
#ifdef HAVE_X11
#ifdef USE_X_TOOLKIT
#define OLDXMENU [-.lwlib]liblw.olb
#define LIBXMENU OLDXMENU/lib
#else /* not USE_X_TOOLKIT */
#define OLDXMENU [-.oldXMenu]libXMenu11.olb
#define LIBXMENU OLDXMENU/lib
#endif /* not USE_X_TOOLKIT */
#else /* not HAVE_X11 */
#define LIBXMENU -lXMenu
#endif /* not HAVE_X11 */

#else /* not HAVE_X_MENU */

/* Otherwise, omit xmenu.obj from the list of X object files, and
   don't worry about the menu library at all.  */
#define XOBJ , xterm.obj, xfns.obj, xfaces.obj, xselect.obj, xrdb.obj
#define LIBXMENU
#endif /* ! defined (HAVE_X_MENU) */

#ifdef USE_X_TOOLKIT
#define @X_TOOLKIT_TYPE@
#if defined (LUCID) || defined (ATHENA)
#define LIBW -lXaw
#endif
#ifdef MOTIF
#ifdef LIB_MOTIF
#define LIBW LIB_MOTIF
#else
#define LIBW -lXm
#endif
#endif
#ifdef OPEN_LOOK
#define LIBW -lXol
#endif

#ifdef HAVE_X11XTR6
#define LIBXTR6 -lSM -lICE
#endif

#define LIBXT $(LIBW) -lXmu -lXt $(LIBXTR6) -lXext
#else
#define LIBXT
#endif

#ifdef HAVE_X11
#define LIBX LIBXMENU LD_SWITCH_X_SITE LD_SWITCH_X_DEFAULT LIBXT LIB_X11_LIB LIBX11_MACHINE LIBX11_SYSTEM
#else /* ! defined (HAVE_X11) */
#define LIBX LIBXMENU LD_SWITCH_X_SITE -lX10 LIBX10_MACHINE LIBX10_SYSTEM
#endif /* ! defined (HAVE_X11) */
#else /* ! defined (HAVE_X_WINDOWS) */
#define XOBJ
#define LIBX
#endif /* ! defined (HAVE_X_WINDOWS) */

#define VMSLIB [-.vms]vmslib.olb
#define LIB_VMSLIB VMSLIB/lib

/* Construct full set of libraries to be linked.
   Note that SunOS needs -lm to come before -lc; otherwise, you get
   duplicated symbols.  If the standard libraries were compiled
   with GCC, we might need gnulib again after them.  */
#define LIBES LIB_VMSLIB LIBX LIBS_SYSTEM LIBS_MACHINE LIBS_TERMCAP \
   LIBS_DEBUG LIB_MATH LIB_STANDARD

#ifdef USE_TEXT_PROPERTIES
#define INTERVAL_OBJ ,intervals.obj, textprop.obj
#else
#define INTERVAL_OBJ
#endif

#ifdef HAVE_GETLOADAVG
#define GETLOADAVG_OBJ
#else
#define GETLOADAVG_OBJ ,getloadavg.obj
#endif

#ifdef TERMINFO
/* Used to be -ltermcap here.  If your machine needs that,
   define LIBS_TERMCAP in the m/*.h file.  */
#ifndef LIBS_TERMCAP
#define LIBS_TERMCAP -lcurses
#endif /* LIBS_TERMCAP */
#define termcapobj terminfo.obj
#else /* ! defined (TERMINFO) */
#ifndef LIBS_TERMCAP
#define LIBS_TERMCAP
#define termcapobj termcap.obj, tparam.obj
#else /* LIBS_TERMCAP */
#define termcapobj tparam.obj
#endif /* LIBS_TERMCAP */
#endif /* ! defined (TERMINFO) */

#ifndef SYSTEM_MALLOC

#ifdef GNU_MALLOC  /* New GNU malloc */
#ifdef REL_ALLOC
#define mallocobj ,vmsgmalloc.obj, ralloc.obj, vm-limit.obj
#else /* ! defined (REL_ALLOC) */
#define mallocobj ,vmsgmalloc.obj, vm-limit.obj
#endif /* ! defined (REL_ALLOC) */
#else /* Old GNU malloc */
#define mallocobj ,malloc.obj
#endif /* Old GNU malloc */

#else /* ! SYSTEM_MALLOC */
#define mallocobj
#endif /* SYSTEM_MALLOC */

#ifndef HAVE_ALLOCA
#define allocaobj ,alloca.obj
#else
#define allocaobj
#endif

#ifdef USE_X_TOOLKIT
#define widgetobj ,widget.obj
#else /* not USE_X_TOOLKIT */
#define widgetobj
#endif /* not USE_X_TOOLKIT */

#ifdef LISP_FLOAT_TYPE
#define FLOAT_SUPPORT float-sup.elc
#else
#define FLOAT_SUPPORT
#endif

#ifdef MULTI_FRAME
#define FRAME_SUPPORT frame.elc menu-bar.elc mouse.elc select.elc \
  scroll-bar.elc faces.elc
#define FRAME_SUPPORT_PRE44 frame.elc menu_bar.elc mouse.elc select.elc \
  scroll_bar.elc faces.elc
#else
#define FRAME_SUPPORT
#define FRAME_SUPPORT_PRE44
#endif

#ifdef VMS
#define VMS_SUPPORT vmsproc.elc vms-patch.elc vms-date.elc
#define VMS_SUPPORT_PRE44 vmsproc.elc vms_patch.elc vms_date.elc
#else
#define VMS_SUPPORT
#define VMS_SUPPORT_PRE44
#endif
