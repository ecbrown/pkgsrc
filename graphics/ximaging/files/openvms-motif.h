#ifndef XIMAGING_OPENVMS_MOTIF_H
#define XIMAGING_OPENVMS_MOTIF_H

#include <stdlib.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Display.h>
#include <Xm/GmUtilsI.h>
#include <X11/keysym.h>

/* Motif 2 renamed the Motif 1 font-list API to render tables. */
#define XmRenderTable XmFontList
#define XmNrenderTable XmNfontList
#define XmCRenderTable XmCFontList
#define XmRRenderTable XmRFontList
#define XmLABEL_RENDER_TABLE XmLABEL_FONTLIST
#define XmeGetDefaultRenderTable(widget, type) \
    _XmGetDefaultFontList((widget), XmLABEL_FONTLIST)
#define XmRenderTableGetDefaultFontExtents ximaging_font_list_extents
#define _XInitImageFuncPtrs XInitImage
#define XmeReplyToQueryGeometry _XmGMReplyToQueryGeometry
#define XmeNavigChangeManaged _XmNavigChangeManaged
#define XmeDrawShadows _XmDrawShadows
#define XmTRAVERSE_GLOBALLY_FORWARD XmTRAVERSE_NEXT_TAB_GROUP

/*
 * The OpenVMS C RTL realpath(3) rejects some valid absolute Unix paths that
 * begin with a logical name (for example /SYS$SYSDEVICE/...).  XImaging uses
 * realpath throughout its file browser, so retain the original path when it
 * names an existing file and the native canonicalization fails.
 */
char *ximaging_realpath(const char *, char *);
#define realpath ximaging_realpath

/* Match the newer private traversal signature used by PathField. */
#define _XmMgrTraversal ximaging_mgr_traversal
void ximaging_mgr_traversal(Widget, Cardinal);

/* Motif 1.2 predates the XmVaCreate convenience entry points. */
#define XmVaCreateForm(p, n, ...) \
    XtVaCreateWidget((n), xmFormWidgetClass, (p), __VA_ARGS__)
#define XmVaCreateMainWindow(p, n, ...) \
    XtVaCreateWidget((n), xmMainWindowWidgetClass, (p), __VA_ARGS__)
#define XmVaCreateManagedDrawingArea(p, n, ...) \
    XtVaCreateManagedWidget((n), xmDrawingAreaWidgetClass, (p), __VA_ARGS__)
#define XmVaCreateManagedFrame(p, n, ...) \
    XtVaCreateManagedWidget((n), xmFrameWidgetClass, (p), __VA_ARGS__)
#define XmVaCreateManagedLabel(p, n, ...) \
    XtVaCreateManagedWidget((n), xmLabelWidgetClass, (p), __VA_ARGS__)
#define XmVaCreateManagedLabelGadget(p, n, ...) \
    XtVaCreateManagedWidget((n), xmLabelGadgetClass, (p), __VA_ARGS__)
#define XmVaCreateManagedList(p, n, ...) \
    XtVaCreateManagedWidget((n), xmListWidgetClass, (p), __VA_ARGS__)
#define XmVaCreateManagedPanedWindow(p, n, ...) \
    XtVaCreateManagedWidget((n), xmPanedWindowWidgetClass, (p), __VA_ARGS__)
#define XmVaCreateManagedPushButton(p, n, ...) \
    XtVaCreateManagedWidget((n), xmPushButtonWidgetClass, (p), __VA_ARGS__)
#define XmVaCreateManagedScrollBar(p, n, ...) \
    XtVaCreateManagedWidget((n), xmScrollBarWidgetClass, (p), __VA_ARGS__)
#define XmVaCreateManagedScrolledWindow(p, n, ...) \
    XtVaCreateManagedWidget((n), xmScrolledWindowWidgetClass, (p), __VA_ARGS__)
#define XmVaCreateManagedSeparatorGadget(p, n, ...) \
    XtVaCreateManagedWidget((n), xmSeparatorGadgetClass, (p), __VA_ARGS__)
#define XmVaCreateScrolledWindow(p, n, ...) \
    XtVaCreateWidget((n), xmScrolledWindowWidgetClass, (p), __VA_ARGS__)

/* Newer list and toggle resources are cosmetic for this application. */
#ifndef XmNprimaryOwnership
#define XmNprimaryOwnership "primaryOwnership"
#endif
#ifndef XmOWN_NEVER
#define XmOWN_NEVER 0
#endif
#ifndef XmNO_AUTO_SELECT
#define XmNO_AUTO_SELECT 0
#endif
#ifndef XmINDICATOR_NONE
#define XmINDICATOR_NONE False
#endif
#ifndef XmNenableThinThickness
#define XmNenableThinThickness "enableThinThickness"
#endif
#ifndef XmNpathMode
#define XmNpathMode "pathMode"
#endif
#ifndef XmPATH_MODE_FULL
#define XmPATH_MODE_FULL 0
#endif

void ximaging_font_list_extents(XmFontList, int *, int *, int *);
char *ximaging_xmstring_unparse(XmString);
#define XmStringUnparse(value, ...) ximaging_xmstring_unparse(value)
#ifndef XmCHARSET_TEXT
#define XmCHARSET_TEXT 0
#endif
#ifndef XmOUTPUT_ALL
#define XmOUTPUT_ALL 0
#endif

#endif
