#ifndef __MAIL_LOADED
#define __MAIL_LOADED 1
/*
 * MAIL.H, prototypes for callable mail routines.
 *
 * Hand coded by Roland B. Roberts from the VMS Utilities manual
 * descriptions of these routines.
 */

#include <stddef.h>

/* VMS Item List 3 structure */
struct itm$list3
{
  short buflen;
  short itemcode;
  void *buffer;
  size_t *retlen;
};

/* Set the values of an Item List 3 structure */
#define $SETITM3(item,blen,code,buf,rlen) ( item.buflen = blen, \
					   item.itemcode = code, \
					   item.buffer = buf, \
					   item.retlen = (size_t *) rlen )
					   
/* Clear the values of an Item list 3 structure.
   A NULL item is required to terminate the item list. */
#define $CLRITM3(item) ( item.buflen = 0, \
			item.itemcode = 0, \
			item.buffer = (void *) 0, \
			item.retlen = (size_t *) 0 )
			
/* All mail utility functions look the same */
typedef unsigned (MAIL$FUNCTION(unsigned*, struct itm$list3*, struct itm$list3*));

MAIL$FUNCTION mail$mailfile_begin;
MAIL$FUNCTION mail$mailfile_close;
MAIL$FUNCTION mail$mailfile_compress;
MAIL$FUNCTION mail$mailfile_end;
MAIL$FUNCTION mail$mailfile_info_file;
MAIL$FUNCTION mail$mailfile_modify;
MAIL$FUNCTION mail$mailfile_open;
MAIL$FUNCTION mail$mailfile_purge_waste;
MAIL$FUNCTION mail$mailfile_begin;
MAIL$FUNCTION mail$message_copy;
MAIL$FUNCTION mail$message_delete;
MAIL$FUNCTION mail$message_end;
MAIL$FUNCTION mail$message_get;
MAIL$FUNCTION mail$message_info;
MAIL$FUNCTION mail$message_modify;
MAIL$FUNCTION mail$message_select;
MAIL$FUNCTION mail$send_abort;
MAIL$FUNCTION mail$send_add_address;
MAIL$FUNCTION mail$send_add_attribute;
MAIL$FUNCTION mail$send_add_bodypart;
MAIL$FUNCTION mail$send_begin;
MAIL$FUNCTION mail$send_end;
MAIL$FUNCTION mail$send_message;
MAIL$FUNCTION mail$user_begin;
MAIL$FUNCTION mail$user_delete_info;
MAIL$FUNCTION mail$user_end;
MAIL$FUNCTION mail$user_get_info;
MAIL$FUNCTION mail$user_set_info;

#endif /* __MAIL_LOADED */
