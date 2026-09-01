/* localshared.h --- reduced-functionality stub definitions  */

/* Copyright (C) 2003 Free Software Foundation */

/* send complaints to the author: <ttn@gnu.org> */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>

#define _(x) x
#define PARAMS(x) x
#define VPARAMS(x) x
#define VA_START(VA_LIST, VAR)  va_start(VA_LIST, VAR)
#define VA_OPEN(AP, VAR)        { va_list AP; va_start(AP, VAR); { struct Qdmy
#define VA_CLOSE(AP)            } va_end(AP); }
#define VA_FIXEDARG(AP, T, N)   struct Qdmy
#define ATTRIBUTE_UNUSED
#define ATTRIBUTE_PRINTF(x,y)
#define ATTRIBUTE_PRINTF_1
#define ATTRIBUTE_PRINTF_2
#define ATTRIBUTE_NORETURN
/* #define USER_LABEL_PREFIX "L" */
#define version_string "3.3.2ttn1"
#ifdef VMS
# define SUCCESS_EXIT_CODE 1
# define FATAL_EXIT_CODE 4
#else /* !VMS */
# define SUCCESS_EXIT_CODE 0
# define FATAL_EXIT_CODE 1
#endif /* !VMS */
#define PTR void *
#define xmalloc malloc
#define xstrdup strdup
#define xcalloc calloc
#define xrealloc realloc
#define ISDIGIT isdigit
#define ISIDNUM(c) (isalnum(c)||(c)=='_')
#define ISPRINT isprint
#define ISSPACE isspace
#define IS_NVSPACE(c) ((c)==' '||(c)=='\t'||(c)=='\f'||(c)=='\v'||(c)==0)
#define ISIDST(c) (((c)>='A'&&(c)<='Z')||((c)>='a'&&(c)<='z')||(c)=='_')
#ifdef VMS
# define IS_ABSOLUTE_PATHNAME(name) (NULL != index ((const char *)(name),']'))
#else /* !VMS */
# define IS_ABSOLUTE_PATHNAME(name) ('/' == (name)[0])
#endif /* !VMS */
#ifdef VMS
# define _dollar_ok(x) ((x)=='$')
#else /* !VMS */
# define _dollar_ok(x) ((x)=='$' && 0)
#endif /* !VMS */
/* #define is_idchar(x)    (ISIDNUM(x) || _dollar_ok(x)) */
/* #define is_idstart(x)   (ISIDST(x) || _dollar_ok(x)) */
#define CHAR_TYPE_SIZE 8
#define BITS_PER_UNIT 8
#ifndef TARGET_BELL
#  define TARGET_BELL 007
#  define TARGET_BS 010
#  define TARGET_TAB 011
#  define TARGET_NEWLINE 012
#  define TARGET_VT 013
#  define TARGET_FF 014
#  define TARGET_CR 015
#  define TARGET_ESC 033
#endif
#define ISUPPER(c) ((c)>='A'&&(c)<='Z')
#define TOLOWER(c) (ISUPPER(c)?((c)+0x20):(c))
#define ISXDIGIT(c) (((c)>='0'&&(c)<='9')||((c)>='A'&&(c)<='F')||((c)>='a'&&(c)<='f'))

/* hex character manipulation routines */

#define _hex_array_size 256
#define _hex_bad        99
extern const char _hex_value[_hex_array_size];
extern void hex_init PARAMS ((void));
#define hex_p(c)        (hex_value (c) != _hex_bad)
/* If you change this, note well: Some code relies on side effects in
   the argument being performed exactly once.  */
#define hex_value(c)    (_hex_value[(unsigned char) (c)])


#ifdef VMS
#ifndef alloca
extern void *alloca (int);
#endif
#endif /* VMS */

/* extern int compare_defs (DEFINITION *d1, DEFINITION *d2); */


/* set everything up */
extern void initialize_random_junk (void);

/* localshared.h ends here */
