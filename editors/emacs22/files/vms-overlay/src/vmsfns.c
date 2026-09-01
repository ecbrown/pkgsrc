/* Miscellaneous VMS functions.
   Copyright (C) 1987, 1988 Free Software Foundation, Inc.

This file is part of GNU Emacs.

GNU Emacs is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Emacs is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Emacs; see the file COPYING.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* Written by Mukesh Prasad.  */

#include <stdio.h>
#include <ctype.h>
#undef NULL

#include <config.h>
#include "lisp.h"
#include "keyboard.h"
#include <descrip.h>
#include <dvidef.h>
#include <prvdef.h>
#include <iodef.h>
#include <ssdef.h>
#include <lnmdef.h>
#include <errno.h>
#include <lib$routines.h>
#include <starlet.h>

#include <jpidef.h>		/* this may need a configure check
					   for old systems --ttn */

/* VSI C for OpenVMS x86-64 gives the access wrapper introduced by
   <unistd.h> this external name.  Modules that saw the C RTL prototype
   already use it; this older module did not.  */
extern int __long_gid_sys_access (const char *, int);

/* #include <syidef.h> */

#define	CLI$M_NOWAIT	1	/* clidef.h is missing from C library */
#define	SYI$_VERSION	4096	/* syidef.h is missing from C library */
#define	JPI$_CLINAME	522	/* JPI$_CLINAME is missing from jpidef.h */
#define	JPI$_MASTER_PID	805	/* JPI$_MASTER_PID missing from jpidef.h */
#define	LIB$_NOSUCHSYM	1409892 /* libclidef.h missing */

#ifndef PRV$V_ACNT

/* these defines added as hack for VMS 5.1-1.   SJones, 8-17-89 */
/* this is _really_ nasty and needs to be changed ASAP - should see about
   using the union defined in SYS$LIBRARY:PRVDEF.H under v5 */

#define PRV$V_ACNT	0x09
#define PRV$V_ALLSPOOL  0x04
#define PRV$V_ALTPRI	0x0D
#define PRV$V_BUGCHK    0x17
#define PRV$V_BYPASS    0x1D
#define PRV$V_CMEXEC    0x01
#define PRV$V_CMKRNL    0x00
#define PRV$V_DETACH    0x05
#define PRV$V_DIAGNOSE  0x06
#define PRV$V_DOWNGRADE 0x21
#define PRV$V_EXQUOTA   0x13
#define PRV$V_GROUP     0x08
#define PRV$V_GRPNAM    0x03
#define PRV$V_GRPPRV 	0x22
#define PRV$V_LOG_IO    0x07
#define PRV$V_MOUNT     0x11
#define PRV$V_NETMBX    0x14
#define PRV$V_NOACNT    0x09
#define PRV$V_OPER      0x12
#define PRV$V_PFNMAP    0x1A
#define PRV$V_PHY_IO    0x16
#define PRV$V_PRMCEB    0x0A
#define PRV$V_PRMGBL    0x18
#define PRV$V_PRMJNL 	0x25
#define PRV$V_PRMMBX    0x0B
#define PRV$V_PSWAPM    0x0C
#define PRV$V_READALL 	0x23
#define PRV$V_SECURITY 	0x26
#define PRV$V_SETPRI    0x0D
#define PRV$V_SETPRV    0x0E
#define PRV$V_SHARE 	0x1F
#define PRV$V_SHMEM     0x1B
#define PRV$V_SYSGBL    0x19
#define PRV$V_SYSLCK	0x1E
#define PRV$V_SYSNAM    0x02
#define PRV$V_SYSPRV    0x1C
#define PRV$V_TMPJNL 	0x24
#define PRV$V_TMPMBX    0x0F
#define PRV$V_UPGRADE 	0x20
#define PRV$V_VOLPRO    0x15
#define PRV$V_WORLD     0x10
#endif /* not PRV$V_ACNT */

/* Structure for privilege list.  */
struct privilege_list
{
  char * name;
  int  mask;
};

/* Structure for finding VMS related information.  */
struct vms_objlist
{
  char * name;			/* Name of object */
  Lisp_Object (* objfn)();	/* Function to retrieve VMS object */
};

/* List of privilege names and mask offsets.  */
static struct privilege_list priv_list[] =
  {
    { "ACNT",		PRV$V_ACNT },
    { "ALLSPOOL",	PRV$V_ALLSPOOL },
    { "ALTPRI",		PRV$V_ALTPRI },
    { "BUGCHK",		PRV$V_BUGCHK },
    { "BYPASS",		PRV$V_BYPASS },
    { "CMEXEC",		PRV$V_CMEXEC },
    { "CMKRNL",		PRV$V_CMKRNL },
    { "DETACH",		PRV$V_DETACH },
    { "DIAGNOSE",	PRV$V_DIAGNOSE },
    { "DOWNGRADE",	PRV$V_DOWNGRADE }, /* Isn't VMS as low as you can go?  */
    { "EXQUOTA",	PRV$V_EXQUOTA },
    { "GRPPRV",		PRV$V_GRPPRV },
    { "GROUP",		PRV$V_GROUP },
    { "GRPNAM",		PRV$V_GRPNAM },
    { "LOG_IO",		PRV$V_LOG_IO },
    { "MOUNT",		PRV$V_MOUNT },
    { "NETMBX",		PRV$V_NETMBX },
    { "NOACNT",		PRV$V_NOACNT },
    { "OPER",		PRV$V_OPER },
    { "PFNMAP",		PRV$V_PFNMAP },
    { "PHY_IO",		PRV$V_PHY_IO },
    { "PRMCEB",		PRV$V_PRMCEB },
    { "PRMGBL",		PRV$V_PRMGBL },
    { "PRMJNL",		PRV$V_PRMJNL },
    { "PRMMBX",		PRV$V_PRMMBX },
    { "PSWAPM",		PRV$V_PSWAPM },
    { "READALL",	PRV$V_READALL },
    { "SECURITY",	PRV$V_SECURITY },
    { "SETPRI",		PRV$V_SETPRI },
    { "SETPRV",		PRV$V_SETPRV },
    { "SHARE", 		PRV$V_SHARE },
    { "SHMEM",		PRV$V_SHMEM },
    { "SYSGBL",		PRV$V_SYSGBL },
    { "SYSLCK",		PRV$V_SYSLCK },
    { "SYSNAM",		PRV$V_SYSNAM },
    { "SYSPRV",		PRV$V_SYSPRV },
    { "TMPJNL",		PRV$V_TMPJNL },
    { "TMPMBX",		PRV$V_TMPMBX },
    { "UPGRADE",	PRV$V_UPGRADE },
    { "VOLPRO",		PRV$V_VOLPRO },
    { "WORLD",		PRV$V_WORLD },
  };

static Lisp_Object
          vms_account(), vms_cliname(), vms_owner(), vms_grp(), vms_image(),
          vms_parent(), vms_pid(), vms_prcnam(), vms_terminal(), vms_uic_int(),
	  vms_uic_str(), vms_username(), vms_version_fn(), vms_trnlog(),
	  vms_symbol(), vms_proclist();

/* Table of arguments to Fvms_system_info, and their data-retrieval funcs.  */

static struct vms_objlist vms_object [] =
  {
    { "ACCOUNT",    vms_account },	/* account name as a string */
    { "CLINAME",    vms_cliname },	/* CLI name (string) */
    { "OWNER",      vms_owner },	/* owner process's PID (int) */
    { "GRP",        vms_grp },		/* group number of UIC (int) */
    { "IMAGE",      vms_image },	/* executing image (string) */
    { "PARENT",     vms_parent },	/* parent proc's PID (int) */
    { "PID",        vms_pid },		/* process's PID (int) */
    { "PRCNAM",     vms_prcnam },	/* process's name (string) */
    { "TERMINAL",   vms_terminal },	/* terminal name (string) */
    { "UIC",        vms_uic_int },	/* UIC as integer */
    { "UICGRP",     vms_uic_str },	/* UIC as string */
    { "USERNAME",   vms_username },	/* username (string) */
    { "VERSION",    vms_version_fn },	/* VMS version (string) */
    { "LOGICAL",    vms_trnlog },	/* VMS logical name translation */
    { "DCL-SYMBOL", vms_symbol },	/* DCL symbol translation */
    { "PROCLIST",   vms_proclist },	/* list of all PIDs on system */
  };
   
DEFUN ("setprv", Fsetprv, Ssetprv, 1, 3, 0,
  "Modify or examine a VMS privilege PRIV.\n\
Optional second arg VAL is t or nil, indicating whether PRIV is\n\
to be set or reset.  Return t if successful, nil otherwise.\n\
Optional third arg EXAMINE-ONLY, if non-nil, means do not modify\n\
privileges, instead returning t or nil depending upon whether or\n\
not PRIV is currently enabled.")
  (priv, value, getprv)
     Lisp_Object priv, value, getprv;
{
  int prvmask[2], prvlen, newmask[2];
  char * prvname;
  int found, i;
  struct privilege_list * ptr;

  CHECK_STRING (priv);
  priv = Fupcase (priv);
  prvname = XSTRING (priv)->data;
  prvlen = XSTRING (priv)->size;
  found = 0;
  prvmask[0] = 0;
  prvmask[1] = 0;
  for (i = 0; i < sizeof (priv_list) / sizeof (priv_list[0]); i++)
    {
      ptr = &priv_list[i];
      if (prvlen == strlen (ptr->name) &&
	  bcmp (prvname, ptr->name, prvlen) == 0)
	{
	  if (ptr->mask >= 32)
	    prvmask[1] = 1UL << (ptr->mask % 32);
	  else
	    prvmask[0] = 1UL << ptr->mask;
	  found = 1;
	  break;
	}
    }
  if (! found)
    error ("Unknown privilege name %s", XSTRING (priv)->data);
  if (NILP (getprv))
    {
      if (sys$setprv (NILP (value) ? 0 : 1, prvmask, 0, 0) == SS$_NORMAL)
	return Qt;
      return Qnil;
    }
  /* Get old priv value.  */
  if (sys$setprv (0, 0, 0, newmask) != SS$_NORMAL)
    return Qnil;
  if ((newmask[0] & prvmask[0])
      || (newmask[1] & prvmask[1]))
    return Qt;
  return Qnil;
}

/* Retrieve VMS system information.  */

DEFUN ("vms-system-info", Fvms_system_info, Svms_system_info, 1, 3, 0,
  "Retrieve VMS process and system information.\n\
The first argument (a string) specifies the type of information desired.\n\
The other arguments depend on the type you select.\n\
For information about a process, the second argument is a process ID\n\
or a process name, with the current process as a default.\n\
These are the possibilities for the first arg (upper or lower case ok):\n\
    account     account name\n\
    cliname     CLI name\n\
    owner       owner process's PID\n\
    grp         group number\n\
    parent      parent process's PID\n\
    pid         process's PID\n\
    prcnam      process's name\n\
    terminal    terminal name\n\
    uic         UIC number\n\
    uicgrp      formatted [UIC,GRP]\n\
    username    username\n\
    version     VMS version\n\
    logical     VMS logical name (second argument) translation\n\
    dcl-symbol  DCL symbol (second argument) translation\n\
    proclist    list of all PIDs on system (need WORLD privilege)." )
  (type, arg1, arg2)
     Lisp_Object type, arg1, arg2;
{
  int i, typelen;
  char * typename;
  struct vms_objlist * ptr;

  CHECK_STRING (type);
  type = Fupcase (type);
  typename = XSTRING (type)->data;
  typelen = XSTRING (type)->size;
  for (i = 0; i < sizeof (vms_object) / sizeof (vms_object[0]); i++)
    {
      ptr = &vms_object[i];
      if (typelen == strlen (ptr->name)
	  && bcmp (typename, ptr->name, typelen) == 0)
	return (* ptr->objfn)(arg1, arg2);
    }
  error ("Unknown object type %s", typename);
  return Qnil;
}

/* Given a reference to a VMS process, returns its process id.  */

static int
translate_id (pid, owner)
     Lisp_Object pid;
     int owner;		/* if pid is null/0, return owner.  If this
			 * flag is 0, return self. */
{
  int status, code, id, i, numeric, size;
  char * p;
  struct dsc$descriptor_s prcnam =
    {0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};

  if (NILP (pid)
      || STRINGP (pid) && XSTRING (pid)->size == 0
      || INTEGERP (pid) && XFASTINT (pid) == 0)
    {
      code = owner ? JPI$_OWNER : JPI$_PID;
      status = lib$getjpi (&code, 0, 0, &id);
      if (! (status & 1))
	error ("Cannot find %s: %s",
	       owner ? "owner process" : "process id",
	       vmserrstr (status));
      return (id);
    }
  if (INTEGERP (pid))
    return (XFASTINT (pid));
  CHECK_STRING (pid);
  pid = Fupcase (pid);
  size = SBYTES (pid);
  p = (char *) SDATA (pid);
  numeric = 1;
  id = 0;
  for (i = 0; i < size; i++, p++)
    if (isxdigit (*p))
      {
	id *= 16;
	if (*p >= '0' && *p <= '9')
	  id += *p - '0';
	else
	  id += *p - 'A' + 10;
      }
    else
      {
	numeric = 0;
	break;
      }
  if (numeric)
    return (id);
  prcnam.dsc$w_length = SBYTES (pid);
  prcnam.dsc$a_pointer = (char *) SDATA (pid);
  status = lib$getjpi (&JPI$_PID, 0, &prcnam, &id);
  if (! (status & 1))
    error ("Cannot find process id: %s",
	   vmserrstr (status));
  return (id);
}				/* translate_id */

/* VMS object retrieval functions.  */

static Lisp_Object
getjpi (jpicode, arg, numeric)
     int jpicode;		/* Type of GETJPI information */
     Lisp_Object arg;
     int numeric;		/* 1 if numeric value expected */
{
  int id, status, numval;
  char str[128];
  struct dsc$descriptor_s strdsc =
    {sizeof (str), DSC$K_DTYPE_T, DSC$K_CLASS_S, str};
  unsigned short result_length;

  id = translate_id (arg, 0);
  status = lib$getjpi (&jpicode, &id, 0, &numval, &strdsc,
		       &result_length);
  if (! (status & 1))
    error ("Unable to retrieve information: %s",
	   vmserrstr (status));
  if (numeric)
    return (make_number (numval));
  return (make_string (str, result_length));
}

static Lisp_Object
vms_account (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  return getjpi (JPI$_ACCOUNT, arg1, 0);
}

static Lisp_Object
vms_cliname (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  return getjpi (JPI$_CLINAME, arg1, 0);
}

static Lisp_Object
vms_grp (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  return getjpi (JPI$_GRP, arg1, 1);
}

static Lisp_Object
vms_image (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  return getjpi (JPI$_IMAGNAME, arg1, 0);
}

static Lisp_Object
vms_owner (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  return getjpi (JPI$_OWNER, arg1, 1);
}

static Lisp_Object
vms_parent (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  return getjpi (JPI$_MASTER_PID, arg1, 1);
}

static Lisp_Object
vms_pid (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  return getjpi (JPI$_PID, arg1, 1);
}

static Lisp_Object
vms_prcnam (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  return getjpi (JPI$_PRCNAM, arg1, 0);
}

static Lisp_Object
vms_terminal (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  return getjpi (JPI$_TERMINAL, arg1, 0);
}

static Lisp_Object
vms_uic_int (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  return getjpi (JPI$_UIC, arg1, 1);
}

static Lisp_Object
vms_uic_str (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  return getjpi (JPI$_UIC, arg1, 0);
}

static Lisp_Object
vms_username (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  return getjpi (JPI$_USERNAME, arg1, 0);
}

static Lisp_Object
vms_version_fn (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  char str[40];
  int status;
  struct dsc$descriptor_s strdsc =
    {sizeof (str), DSC$K_DTYPE_T, DSC$K_CLASS_S, str};
  unsigned short result_length;

  status = lib$getsyi (&SYI$_VERSION, 0, &strdsc, &result_length, 0, 0);
  if (! (status & 1))
    error ("Unable to obtain version: %s", vmserrstr (status));
  return (make_string (str, result_length));
}

static Lisp_Object
vms_trnlog (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  char str[256];		/* Max logical translation is 255 bytes.  */
  int status;
  struct dsc$descriptor_s symdsc =
    {0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};
  struct dsc$descriptor_s strdsc =
    {sizeof (str), DSC$K_DTYPE_T, DSC$K_CLASS_S, str};
  unsigned short length;

  CHECK_STRING (arg1);
  symdsc.dsc$w_length = SBYTES (arg1);
  symdsc.dsc$a_pointer = (char *) SDATA (arg1);
  status = lib$sys_trnlog (&symdsc, &length, &strdsc);
  if (! (status & 1))
    error ("Unable to translate logical name: %s", vmserrstr (status));
  if (status == SS$_NOTRAN)
    return (Qnil);
  return (make_string (str, length));
}

static int
vms_trnlnm (name, index, output)
     char *name;
     int index;
     char *output;		/* Trust the user to provide a 256 char str. */
{
  int status, level, attr = LNM$M_CASE_BLIND;
  unsigned short length, levlen;
  struct itemlist {
    short buffer_length;
    short item_code;
    char *buffer_addr;
    unsigned short *return_length;
  } itmlst [] = {
    4,   LNM$_INDEX    , (char *) &index, 0,
    255, LNM$_STRING   , (char *) output, &length,
    4,   LNM$_MAX_INDEX, (char *) &level, &levlen,
    0,   0,   0,  0 };
  struct dsc$descriptor_s lognam =
    { strlen (name), DSC$K_DTYPE_T, DSC$K_CLASS_S, name };
  $DESCRIPTOR (tabnam, "LNM$DCL_LOGICAL");

  status = sys$trnlnm (&attr, &tabnam, &lognam, 0, itmlst);

  if (status != SS$_NORMAL)
    return -1;

  output[length] = '\0';

  return ++level;
}

static Lisp_Object
vms_symbol (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  char str[1025];		/* Max symbol translation is 1024 bytes.  */
  int status;
  struct dsc$descriptor_s symdsc =
    {0, DSC$K_DTYPE_T, DSC$K_CLASS_S, 0};
  struct dsc$descriptor_s strdsc =
    {sizeof (str), DSC$K_DTYPE_T, DSC$K_CLASS_S, str};
  unsigned short length;
  int level;

  CHECK_STRING (arg1);
  symdsc.dsc$w_length = SBYTES (arg1);
  symdsc.dsc$a_pointer = (char *) SDATA (arg1);
  status = lib$get_symbol (&symdsc, &strdsc, &length, &level);
  if (! (status & 1)) {
    if (status == LIB$_NOSUCHSYM)
      return (Qnil);
    else
      error ("Unable to translate symbol: %s", vmserrstr (status));
  }
  return (make_string (str, length));
}

static Lisp_Object
vms_proclist (arg1, arg2)
     Lisp_Object arg1, arg2;
{
  Lisp_Object retval;
  int id, status, pid;

  retval = Qnil;
  pid = -1;
  for (;;)
    {
      status = lib$getjpi (&JPI$_PID, &pid, 0, &id);
      if (status == SS$_NOMOREPROC)
	break;
      if (! (status & 1))
	error ("Unable to get process ID: %s", vmserrstr (status));
      retval = Fcons (make_number (id), retval);
    }
  return (Fsort (retval, intern ("<")));
}

DEFUN ("vms-expand-search-paths", Fvms_expand_search_paths, 
       Svms_expand_search_paths, 1, 2, 0,
       "Expands any vms paths to find which entry contains the first\n\
occurrence of the specified file.  The path returned is unambiguous\n\
with regard to VMS paths.  I.e. a file written to the returned\n\
filename will appear in the same directory it was read from.\n\
If the optional second argument is non-nil, the specification will\n\
be completely expanded, so the device part will be a real device, and\n\
not a logical name.")
(filename, expand_completely)
     Lisp_Object filename, expand_completely;
{
  extern Lisp_Object expand_search_paths ();

  CHECK_STRING (filename);
  return expand_search_paths (filename, !NILP (expand_completely));
}

/* Combine one logical-name translation with the suffix from the original
   file specification.  Allocate to fit the actual strings: a logical
   translation is limited to 255 bytes, but the suffix is not.  */
static Lisp_Object
vms_search_path_candidate (translation, suffix, skip_0_dir)
     const char *translation;
     const char *suffix;
     int *skip_0_dir;
{
  size_t size = strlen (translation) + strlen (suffix) + 4;
  char *buf = (char *) xmalloc (size);
  char *bp;
  Lisp_Object candidate;

  strcpy (buf, translation);
  if (!strchr (buf, ':'))
    strcat (buf, ":");

  bp = strpbrk (buf, "]>");
  if (bp)
    {
      switch (*skip_0_dir)
	{
	case -1:
	  strcat (buf, *suffix == '<' ? "<" : "[");
	  *skip_0_dir = 8;
	  break;
	case 8:
	  if (bp > buf && bp[-1] == '.')
	    {
	      bp[-1] = *bp;
	      bp[0] = '\0';
	    }
	  break;
	}
      strcat (buf, suffix + *skip_0_dir);
    }
  else
    strcat (buf, suffix);

  candidate = build_string (buf);
  free (buf);
  return candidate;
}

Lisp_Object
expand_search_paths (file, expand_completely)
     Lisp_Object file;
     int expand_completely;
{
  Lisp_Object tem = file, dev = Qnil, result = file, seen = Qnil;
  Lisp_Object suffix_string = Qnil;
  char translation[256];
  int i, n;
  int new_file = __long_gid_sys_access (SDATA (file), 0);
  struct gcpro gcpro1, gcpro2, gcpro3, gcpro4, gcpro5;

  GCPRO5 (tem, dev, result, seen, suffix_string);

  while (1)
    {
      char *cp = strchr (SDATA (tem), ':');
      char *suffix;
      int skip_0_dir;

      if (!cp)
	{
	  UNGCPRO;
	  return result;
	}

      dev = make_string (SDATA (tem), cp - (char *) SDATA (tem));
      dev = Fupcase (dev);
      if (!NILP (Fmember (dev, seen)))
	error ("Circular logical-name translation involving %s",
	       SDATA (dev));
      seen = Fcons (dev, seen);

      suffix_string = build_string (cp + 1);
      suffix = SDATA (suffix_string);
      skip_0_dir = (strncmp (suffix, "[000000]", 8) == 0
		    || strncmp (suffix, "<000000>", 8) == 0 ? 8
		    : (strncmp (suffix, "[000000.", 8) == 0
		       || strncmp (suffix, "<000000.", 8) == 0 ? -1 : 0));
      n = vms_trnlnm (SDATA (dev), 0, translation);
      if (n <= 0)
	{
	  UNGCPRO;
	  return result; /* No more translations, or a translation error.  */
	}

      for (i = 0; i < n; i++)
	{
	  /* TEM is GCPRO'd; keep the newly allocated candidate there while
	     Fexpand_file_name calls back into Lisp-capable file handlers.  */
	  tem = vms_search_path_candidate (translation, suffix, &skip_0_dir);
	  tem = Fexpand_file_name (tem, Qnil);

	  if (n == 1)
	    {
	      if (expand_completely)
		result = tem;
	      break;		/* There are no more possibilities,
				   so recurse. */
	    }

	  if (new_file || !__long_gid_sys_access (SDATA (tem), 0))
	    {			/* found the translation we want */
	      result = tem;
	      break;
	    }

	  if (i < n - 1
	      && vms_trnlnm (SDATA (dev), i + 1, translation) <= 0)
	    break;
	}
    }
}

DEFUN ("shrink-to-icon", Fshrink_to_icon, Sshrink_to_icon, 0, 0, 0,
      "If emacs is running in a workstation window, shrink to an icon.")
     ()
{
  static char result[128];
  static $DESCRIPTOR (result_descriptor, result);
  static $DESCRIPTOR (tt_name, "TT:");
  static int chan = 0;
  static int buf = 0x9d + ('2'<<8) + ('2'<<16) + (0x9c<<24);
  static int temp = JPI$_TERMINAL;
  int status;

  status = lib$getjpi (&temp, 0, 0, 0, &result_descriptor, 0);
  if (status != SS$_NORMAL)
    error ("Unable to determine terminal type.");
  /* Make sure this is a workstation.  */
  if (result[0] != 'W' || result[1] != 'T')
    error ("Can't shrink-to-icon on a non workstation terminal");
  /* Assign channel if not assigned.  */
  if (!chan)
    if ((status = sys$assign (&tt_name, &chan, 0, 0)) != SS$_NORMAL)
      error ("Can't assign terminal, %d", status);
  status = sys$qiow (0, chan, IO$_WRITEVBLK + IO$M_BREAKTHRU,
		     0, 0, 0,
		     &buf, 4, 0, 0, 0, 0);
  if (status != SS$_NORMAL)
    error ("Can't shrink-to-icon, %d", status);
  return Qt;
}


/* Logical name support on this page.  */

#include <descrip.h>
#include <lib$routines.h>

static struct dsc$descriptor_s lnmjobdsc =
  {7, DSC$K_DTYPE_T, DSC$K_CLASS_S, "LNM$JOB"};

#define STRING_AS_DSC(s) \
  { SBYTES (s), DSC$K_DTYPE_T, DSC$K_CLASS_S, (char *) SDATA (s) }

DEFUN ("define-logical-name", Fdefine_logical_name, Sdefine_logical_name,
       2, 2, "sDefine logical name: \nsDefine logical name %s as: ",
  "Define the job-wide logical name NAME to have the value STRING.\n\
If STRING is nil or a null string, the logical name NAME is deleted.")
  (name, string)
     Lisp_Object name;
     Lisp_Object string;
{
  int delp = 0;
  int status;

  CHECK_STRING (name);
  delp = NILP (string);

  {
    struct dsc$descriptor_s name_dsc = STRING_AS_DSC (name);

    if (! delp)
      {
	CHECK_STRING (string);
	delp = (XSTRING (string)->size == 0);
      }

    if (delp)
      status = LIB$DELETE_LOGICAL (&name_dsc, &lnmjobdsc);
    else
      {
	struct dsc$descriptor_s string_dsc = STRING_AS_DSC (string);

	status = LIB$SET_LOGICAL (&name_dsc, &string_dsc, &lnmjobdsc, 0, 0);
      }
    if (!(status & 1))
      error ("Unable to %s logical name %s: %s",
	     delp ? "delete" : "define", SDATA (name), vmserrstr (status));
  }

  return string;
}


void
syms_of_vmsfns (void)
{
  defsubr (&Ssetprv);
  defsubr (&Svms_expand_search_paths);
  defsubr (&Svms_system_info);
  defsubr (&Sshrink_to_icon);
  defsubr (&Sdefine_logical_name);
}

/* vmsfns.c ends here */
