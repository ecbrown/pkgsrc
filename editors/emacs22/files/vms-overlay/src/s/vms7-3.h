/* vms7-3.h: NEW FILE --ttn */
#include "vms.h"
#define VMS7_3

#undef NO_HYPHENS_IN_FILENAMES	/* ttn */

#define HAVE_VMS_PTYS
#define HAVE_GETPPID
#define HAVE_TIMEVAL

/* The bug that SHARABLE_LIB_BUG fixes is gone in version 5.5 of VMS.
   And defining it causes lossage because sys_errlist has a different
   number of elements.  */
#undef SHARABLE_LIB_BUG

/* VMS does not implement soft links yet...  */
#ifdef S_IFLNK
#undef S_IFLNK
#endif
#ifdef S_ISLNK
#undef S_ISLNK
#endif

/* These work ok.  TODO: Zonk in favor of HAVE_* in CONFIG.H.  */
#undef tzset
#undef localtime
#undef gmtime

/* Stupid names collide Fmod vs. fmod. So we do this: */
#define Fmod F_mod
#define Ftruncate F_truncate
#define Fabs F_abs
#define Fmin F_min
#define Fmax F_max

#define DECLARE_GETPWUID_WITH_UID_T
