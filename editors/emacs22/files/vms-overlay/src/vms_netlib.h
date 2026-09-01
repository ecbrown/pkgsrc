/* VMS_NETLIB.H */

#ifndef VMS_NETLIB_H
#include <descrip.h>

/* Assigns a network channel for subsequent operations.  "ctx" will be
   set to a context value to be passed to other routines on successful return.
*/
unsigned net_assign (void *ctx);

/* Binds a local socket for the specified protocol (NET_K_TCP or NET_K_UDP).
   If the port number is specified, the connection is assumed to be "passive"
   (i.e., for a server), with the maximum number of incoming connections
   to be handled by the server specified by the "threads" argument.  This
   can be overriden by passing in a 1 for "notpass".

   If the port number is not given, the connection is assumed to be "active"
   and the port number will be assigned at random.

   NET_K_TCP = 1, NET_K_UDP = 2.
*/
unsigned net_bind (void *ctx, int protocol,
		   /*OPTIONAL*/ int port, /*OPTIONAL*/ int threads,
		   /*OPTIONAL*/ int notpass);

/* Returns a list of IP addresses for the given host name (passed in by
   descriptor).  The array "alist" should have enough room for "alsize"
   addresses; "alcount" will be set to the number of addresses returned.
*/
unsigned net_get_address (void *ctx, struct dsc$descriptor *host, int alsize,
			  unsigned int alist[], int *alcount);

/* Returns the host name for a given IP address.  "name" must point to
    a dynamic string descriptor.
*/
unsigned net_addr_to_name (void *ctx, unsigned int addr,
			   struct dsc$descriptor *name);

/* Deassigns a network channel.  */
unsigned net_deassign (void *ctx);

/* Gets address & port information about a connection.  */
unsigned net_get_info (void *ctx, unsigned int *remadr,
		       /*OPTIONAL*/ int *remport,
		       /*OPTIONAL*/ unsigned int *lcladr,
		       /*OPTIONAL*/ int *lclprt);

/* Gets the local host name.  "name" can point to any string descriptor;
   the length of the name is returned if "length" is specified.
*/
unsigned net_get_hostname (struct dsc$descriptor *name,
			   /*OPTIONAL*/ int *length);

/* Establishes a TCP connection to the specified node and port number.  */
unsigned tcp_connect (void *ctx, struct dsc$descriptor *node, int port);

/* Establishes a TCP connection to the specified IP address (passed by
   reference) and port number.
*/
unsigned tcp_connect_addr (void *ctx, unsigned int *addr, int port);

/* Accepts an incoming connection.  It will not complete until a connection
   comes in for it to accept.  Intend for use by servers only.  The
   call will be completed asynchronously if iosb, astadr, and astprm
   are specified.
*/
unsigned tcp_accept (void *lsnr, void *ctx,
		     /*OPTIONAL*/ unsigned short iosb[4],
		     /*OPTIONAL*/ unsigned int (*astadr)(),
		     /*OPTIONAL*/ unsigned int astprm);

/* Closes a TCP connection.  */
unsigned tcp_disconnect (void *ctx);

/* Sends a line of text to the remote sequence.  It will be terminated
   automatically with a CR/LF sequence, unless bit 1 of "flags" is set
   (which would be flags == 2).  The call will complete asynchronously if
   iosb, astadr, and astprm are specified.
*/
unsigned tcp_send (void *ctx, struct dsc$descriptor *str,
		   /*OPTIONAL*/ int flags,
		   /*OPTIONAL*/ unsigned short iosb[4],
		   /*OPTIONAL*/ unsigned int (*astadr)(),
		   /*OPTIONAL*/ int astprm);

/* Receives data over a TCP connection. "str" must be a dynamic string
   descriptor. Call will complete asynchronously if iosb, astadr, and astprm
   are specified.  You can specify a timeout for the receive with "timeout"
   (in VMS quadword time format).
*/
unsigned tcp_receive (void *ctx, struct dsc$descriptor *str,
		      /*OPTIONAL*/ unsigned short iosb[4],
		      /*OPTIONAL*/ unsigned int (*astadr)(),
		      /*OPTIONAL*/ int astprm,
		      /*OPTIONAL*/ unsigned int timeout[2]);

/* Receives a line of text over a TCP connection (a line of text being a
   string terminated with a CR/LF sequence). "str" must be a dynamic string
   descriptor (the CR/LF will be stripped from the received line on
   completion). Call will complete asynchronously if iosb, astadr, and astprm
   are specified.  You can specify a timeout for the receive with "timeout"
   (in VMS quadword time format).
*/
unsigned tcp_get_line (void *ctx, struct dsc$descriptor *str,
		       /*OPTIONAL*/ unsigned short iosb[4],
		       /*OPTIONAL*/ unsigned int (*astadr)(),
		       /*OPTIONAL*/ int astprm,
		       /*OPTIONAL*/ unsigned int timeout[2]);

/* Sends a UDP datagram to the specified address & port.  */
unsigned udp_send (void *ctx, unsigned int addr, unsigned short port,
		   void *bufptr, unsigned short buflen);

/* Receives a UDP datagram.  Source address and port of the datagram
   can also be returned.  You can specify a timeout value (in VMS quadword
   time format) with the "timeout" argument.
*/
unsigned udp_receive (void *ctx, void *bufptr, unsigned short bufsize,
		      unsigned short *buflen, unsigned int *srcaddr,
		      unsigned int *srcprt, unsigned int timeout[2]);

#define VMS_NETLIB_H
#endif
