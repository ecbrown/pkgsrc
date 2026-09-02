#ifndef __MAIL_LOADED
#define __MAIL_LOADED 1
/*
 * MAIL.H, prototypes for callable mail routines.
 *
 * Hand coded by Roland B. Roberts from the VMS Utilities manual
 * descriptions of these routines.
 */

/* Callable Mail uses the legacy 32-bit item-list layout, even in a source file
   that otherwise enables 64-bit pointers.  Its output item lists use a
   longword return-length cell, unlike the word-sized return length used by
   most system-service item lists.  */
#ifdef __INITIAL_POINTER_SIZE
#pragma __required_pointer_size __save
#pragma __required_pointer_size __short
#endif
struct itm$list3
{
  unsigned short buflen;
  unsigned short itemcode;
  void *buffer;
  unsigned int *retlen;
};
#ifdef __INITIAL_POINTER_SIZE
#pragma __required_pointer_size __restore
#endif

/* Set the values of an Item List 3 structure */
#define $SETITM3(item,blen,code,buf,rlen) ( item.buflen = blen, \
					   item.itemcode = code, \
					   item.buffer = buf, \
					   item.retlen = (unsigned int *) rlen )
					   
/* Clear the values of an Item list 3 structure.
   A NULL item is required to terminate the item list. */
#define $CLRITM3(item) ( item.buflen = 0, \
			item.itemcode = 0, \
			item.buffer = (void *) 0, \
			item.retlen = (unsigned int *) 0 )
			
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
