#ifndef EMACS_VMS_TYPES_H
#define EMACS_VMS_TYPES_H 1

#include <iledef.h>
#include <iosbdef.h>

/* The generated headers expose uppercase typedefs with __NEW_STARLET and
   lowercase typedefs with the compatibility declarations.  Their structure
   tags are stable in both modes, so use local names that do not depend on
   that compiler switch.  */
typedef struct _ile3 EMACS_ILE3;
typedef struct _iosb EMACS_IOSB;

/*
 * Miscellaneous VMS types that are not normally defined
 * in any consistent fashion.
 */

/* VMS Lock status block with value block */
struct LOCK
{
  unsigned short status, reserved;
  unsigned int lockid;
  unsigned int value[4];
};

/* VMS Exit Handler Control block */
struct EXHCB
{
  unsigned int exh$l_link;
  void (*exh$a_routine)();
  unsigned int exh$l_argcount;
  unsigned int *exh$a_status;
  unsigned int exh$l_status;
};

#endif /* EMACS_VMS_TYPES_H */
