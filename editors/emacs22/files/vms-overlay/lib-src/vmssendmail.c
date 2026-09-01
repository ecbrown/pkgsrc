/* Help Emacs send mail on VMS.
   Copyright (C) 1993, 2003 Free Software Foundation.

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
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Authors comment:

 VMSSendMail was written for use by sendmail.el in GNU Emacs. It's
 primary purpose is to allow support of Cc and Bcc fields which the
 regular VMS mail program does not support from the command line
 interface.

 Written by:

 James A. McLaughlin                        Schlumberger, Austin Systems Center
 mclaughlin@asc.slb.com                     8311 North FM 620 Road
 512-331-3000 (voice)                       P.O. Box 200015
 512-331-3127 (fax)                         Austin, Texas  78720-0015  */

#ifdef HAVE_CONFIG_H		/* ttn */
#include <config.h>
#endif

#include <stdio.h>
#include <ctype.h>
/* #include <descrip.h> */	/* ttn */
#include <ssdef.h>
#include <maildef.h>
#include <namdef.h>
#include <mail$routines.h>	/* ttn */

/* DECC/VAXC define this in stdio.h, but GNUC doesn't (should it?) */
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

char *xmalloc (), *xrealloc ();
void error ();
void fatal ();

/* Name this program was invoked with.  */
char *progname;

/* Don't trust stdlib.h completely.  */
#ifdef EXIT_SUCCESS
#undef EXIT_SUCCESS
#endif
#ifdef EXIT_FAILURE
#undef EXIT_FAILURE
#endif
#define EXIT_SUCCESS 1
#define EXIT_FAILURE 4

/* #define DEBUG */

#define ADDR_SEPERATOR		    ", "

#define END_OF_ITEM_LIST_MARKER	    { 0, 0, 0, 0 }

#define BCC_CMND    	    	    "BCC: "
#define CC_CMND			    "CC: "
#define FILE_CMND		    "File: "
#define SUBJ_CMND		    "Subject: "
#define TO_CMND			    "To: "
#define FROM_CMND                   "From: "

typedef char			    Boolean_t;

typedef char const *		    ConstString_t;

typedef char			    DisplayLine_t[NAM$C_MAXRSS];

typedef char *			    String_t;

typedef void			    (*ExecFunc_t)(ConstString_t);

typedef const struct
{
    ConstString_t 		    String;
    size_t			    Length;
    ExecFunc_t			    Execute;
} CmndDesc_t;

typedef char			    CmndLine_t[NAM$C_MAXRSS];
typedef char const		    ConstCmndLine_t[NAM$C_MAXRSS];

typedef struct
{
    unsigned short		    BufferLength;
    unsigned short		    ItemCode;
    void *			    BufferAddress;
    void *			    ReturnLengthAddress;
} ItemList_t;

void
AddAddress(
    ConstString_t		    Address);

void
AddAddrToDisplay(
    DisplayLine_t		    DisplayLine,
    ConstString_t		    Address);

void
AddCCAddress(
    ConstString_t		    Address);

void
AddInvisibleAddress(
    ConstString_t		    Address);

void
AddMsgFile(
    ConstString_t		    FileName);

void
AddSubject(
    ConstString_t		    Subject);

void
AddToAddress(
    ConstString_t		    Address);

void
AddFromAddress(
    ConstString_t		    Address);

void
ComposeMessage(void);

void
ParseCmndLine(
    ConstCmndLine_t		    CmndLine);

void
ParseMsgDesc(
     FILE *			    File);

void
ReadCmndLine(
    FILE *			    File,
    CmndLine_t			    CmndLine);

DisplayLine_t			    CCDisplay;

DisplayLine_t			    ToDisplay;

DisplayLine_t			    FromDisplay;

DisplayLine_t			    MsgSubject;

char				    MsgFile[NAM$C_MAXRSS];

Boolean_t			    NoErrors = TRUE;

ItemList_t			    NullItemList[] = { {0, 0, 0, 0} };

volatile Boolean_t		    OKToComposeAndSend = TRUE;

int				    SendContext = 0;

CmndDesc_t			    ValidCmnds[] =
{
    { TO_CMND,   sizeof TO_CMND - 1,   AddToAddress },
    { CC_CMND,   sizeof CC_CMND - 1,   AddCCAddress },
    { BCC_CMND,  sizeof BCC_CMND - 1,  AddInvisibleAddress },
    { SUBJ_CMND, sizeof SUBJ_CMND - 1, AddSubject },
    { FILE_CMND, sizeof FILE_CMND - 1, AddMsgFile },
    { FROM_CMND, sizeof FROM_CMND - 1, AddFromAddress },
    { NULL,      0,                NULL } /* Terminates the list */
};

/*
** This function is called when one of the VMS mail utilities fail and
** raise an exception. It gets called as a result of specifying it with
** the VAXC$ESTABLISH routine.
**
** If a routine was not established, then a traceback would be printed
** when the mail utilities failed.
**
*/
int
ExcHandler(
    void			    *sigarr,
    void			    *mecharr)
{
    OKToComposeAndSend = FALSE;

    return SS$_NORMAL;
}

int
main(
     int			    argc,
     char *			    argv[])
{
    FILE *			    File;
    char *			    MessageDescFileSpec;
    int				    Status;

    progname = "vmssendmail";

    --argc;
    ++argv;
    if (argc != 1)
    {
	printf("%s: Wrong number of arguments.\n", progname);
	return EXIT_FAILURE;
    }

    MessageDescFileSpec = argv[0];
    File = fopen(MessageDescFileSpec, "r");
    if (File != NULL)
    {
	VAXC$ESTABLISH(ExcHandler);
	if ((Status = mail$send_begin(&SendContext, &NullItemList,
				      &NullItemList)) == SS$_NORMAL)
	{
	    ParseMsgDesc(File);
	    if (OKToComposeAndSend && NoErrors)
	    {
		ComposeMessage();
		if (OKToComposeAndSend)
		    if ((Status = mail$send_message(&SendContext,
						    &NullItemList,
						    &NullItemList))
			!= SS$_NORMAL)
		    {
			printf("%s: Problem sending message\n", progname);
			printf("             MAIL$SEND_MESSAGE returned %d\n",
			       Status);
			return EXIT_FAILURE;
		    }
	    }
	    if ((Status = mail$send_end(&SendContext, NullItemList,
					NullItemList)) != SS$_NORMAL)
	    {
	        printf("VMSENDMAIL: Problem sending message\n");
		printf("            MAIL$SEND_END returned %d\n",
		       Status);
		return EXIT_FAILURE;
	    }
	}
	else
	{
	    printf("%s: Could not compose message\n", progname);
	    printf("             MAIL$SEND_BEGIN returned %d\n",
		   Status);
	    return EXIT_FAILURE;
	}
    }
    
    return NoErrors && OKToComposeAndSend ? EXIT_SUCCESS : EXIT_FAILURE;
}

void
ParseMsgDesc(
     FILE *			    File)
{

    CmndLine_t			    CmndLine;

    while (!feof(File) && NoErrors)
    {
	ReadCmndLine(File, CmndLine);
	if ((CmndLine[0] != '\0') && NoErrors)
	    ParseCmndLine(CmndLine);
    }
}

	
void
ReadCmndLine(
    FILE *			    File,
    CmndLine_t			    CmndLine)
{
    Boolean_t			    GetMore;
    char *			    NewlineCharP;
    static CmndLine_t		    NextLine = { '\0' };

    GetMore = TRUE;
    if (NextLine[0] != '\0')
	strcpy(CmndLine, NextLine);
    else
	CmndLine[0] = '\0';

    while (GetMore)
    {
	if ((fgets(NextLine, sizeof(CmndLine_t), File) == NULL) &&
	    !feof(File))
	{
	    if (CmndLine[0] == '\0') /* cmnd line is empty */
		NoErrors = FALSE;
	    GetMore = FALSE;
	}
	else if (CmndLine[0] == '\0')
	{
	    strcpy(CmndLine, NextLine);
	    *NextLine = '\0';
	}
	else if ((NextLine[0] == ' ') || (NextLine[0] == '\t'))
	    { /* lines beginning with space or tab are part of current cmnd */
		if (strlen(CmndLine) + strlen(NextLine) >=
		    sizeof(CmndLine_t))
		{
		    printf("%s: Line to long\n", progname);
		    GetMore = FALSE;
		    NoErrors = FALSE;
		}
		else
		{
		    NewlineCharP = strchr(CmndLine, '\n');
		    if (NewlineCharP != NULL)
			strcpy(NewlineCharP, "\n\r");
		    strcat(CmndLine, NextLine);
		    NextLine[0] = '\0';
		}
	    }
	else
	    GetMore = FALSE;
    };
    
    NewlineCharP = strrchr(CmndLine, '\n');
    if (NewlineCharP != NULL)
	*NewlineCharP = '\0';
}

void
ParseCmndLine(
    ConstCmndLine_t		    CmndLine)
{
    CmndDesc_t *		    Cmnd;
    Boolean_t			    Found;

    Found = FALSE;
    Cmnd = ValidCmnds;
    while ((Cmnd->String != NULL) && !Found)
    {
	if (strncmp(CmndLine, Cmnd->String, Cmnd->Length) == 0)
	    Found = TRUE;
	else
	    ++Cmnd;
    }

    if (Found)
    {
	ConstString_t		    RestOfArgs;

	RestOfArgs = &CmndLine[Cmnd->Length];
	Cmnd->Execute(RestOfArgs);
    }
    else
    {
	printf("%s: Unknown command - %s\n", progname, CmndLine);
	NoErrors = FALSE;
    }
}

void
AddAddress(
    ConstString_t		    Address)
{
    ItemList_t AddrItemList[] = 
    {
	{ 0, MAIL$_SEND_USERNAME, 0, 0 },
	END_OF_ITEM_LIST_MARKER
    };

    while (isspace(*Address))
	Address++;
    if (Address[0]!='@')
    {
	AddrItemList[0].BufferLength = strlen(Address);
	AddrItemList[0].BufferAddress = (void *) Address; /* ttn */

	if (mail$send_add_address(&SendContext, AddrItemList,
				  NullItemList) != SS$_NORMAL)
	{
	    printf("%s: Problem with address %s\n", progname, Address);
	}
    }
    else
    {	/* if it starts with a "@" it's a distribution list */
	FILE *distfile = fopen(++Address,"r");  /* open the file and read addresses */

	if (distfile != NULL)
	{
	    char DistAddress[257];

	    do {
		char *tempchar;
		char *firstnonspace;
		char *lastnonspace;
		register instring = 0;

		fgets(DistAddress,256,distfile);
#ifdef DEBUG
		printf("Trimming distribution file item `%s'...\n",
		       DistAddress);
#endif
		tempchar = DistAddress;
		while (*tempchar != '\0' && strchr(" \t\f",*tempchar) != NULL)
		    tempchar++;
		firstnonspace = lastnonspace = tempchar;
		while (*tempchar != '\0')
		{
		    if (*tempchar == '\n')
			break;  /* fgets includes the '\n'  */
		    else
		    {
			if (!instring && *tempchar == '!')
#ifdef DEBUG
			{
			    printf("\t`!' found, instring = %d, so we break\n",
				   instring);
#endif
			    break;
#ifdef DEBUG
			}
#endif
			if (*tempchar == '"')
#ifdef DEBUG
			{
#endif
			    instring = !instring;
#ifdef DEBUG
			    printf("\t`\"' found, so we set instring to %d\n",
				   instring);
			}
#endif

			/* Yes, I want lastnonspace to point at the character
			   AFTER the last non-space character */
			if (strchr(" \t\f",*tempchar++) == NULL || instring)
			    lastnonspace = tempchar;
		    }
		}
		*lastnonspace = '\0';
#ifdef DEBUG
		printf("\t-> `%s'", firstnonspace);
#endif
		if (*firstnonspace != '\0')
		{
#ifdef DEBUG
		    printf("\n");
#endif

		    AddrItemList[0].BufferLength = strlen(firstnonspace);
		    AddrItemList[0].BufferAddress = firstnonspace;
           
		    if (mail$send_add_address(&SendContext, AddrItemList,
					      NullItemList) != SS$_NORMAL)
		    {
			printf("%s: Problem with address %s\n", progname,
			       firstnonspace);
		    }
		}
#ifdef DEBUG
		else
		    printf(" (not used, since it's empty)\n");
#endif
	    }
	    while (!feof(distfile));
	    fclose(distfile);
	}
	else
	{
	    printf("%s:problems with distribution file %s\n",
		   progname, Address);
	}
    }
}

void
AddFromAddress(
	       ConstString_t		    Address)
{
  AddAddrToDisplay(FromDisplay, Address);
}


void
AddToAddress(
    ConstString_t		    Address)
{
    AddAddress(Address);

    AddAddrToDisplay(ToDisplay, Address);
}

void
AddCCAddress(
    ConstString_t		    Address)
{

    AddAddress(Address);

    AddAddrToDisplay(CCDisplay, Address);
}

void
AddInvisibleAddress(
    ConstString_t		    Address)
{
    AddAddress(Address);
}

void
AddSubject(
    ConstString_t		    Subject)
{
    strncpy(MsgSubject, Subject, sizeof MsgSubject - 1);
}

void
AddMsgFile(
    ConstString_t		    FileName)
{
    strncpy(MsgFile, FileName, sizeof MsgFile - 1);
}

void
AddAddrToDisplay(
    DisplayLine_t		    DisplayLine,
    ConstString_t		    Address)
{
    size_t			    AddrLength;
    size_t			    AmtToCopy;
    size_t			    CurrentLength;
    const size_t		    MaxLength = sizeof(DisplayLine_t) - 1;
    size_t			    RemainingSpace;

    CurrentLength = strlen(DisplayLine);
    RemainingSpace = MaxLength - CurrentLength;
    AddrLength = strlen(Address);
    
    if (CurrentLength != 0)	/* Add Seperator */
    {
	if (RemainingSpace < (sizeof ADDR_SEPERATOR - 1))
	    AmtToCopy = RemainingSpace;
	else
	    AmtToCopy = sizeof ADDR_SEPERATOR - 1;

	strncat(DisplayLine, ADDR_SEPERATOR, AmtToCopy);

	RemainingSpace = RemainingSpace - AmtToCopy;
    }

    if (RemainingSpace < AddrLength)
	AmtToCopy = RemainingSpace;
    else
	AmtToCopy = AddrLength;
    
    strncat(DisplayLine, Address, AmtToCopy);
}

void
ComposeMessage(void)
{
    char			    ResultSpec[NAM$C_MAXRSS];
    
    ItemList_t AttrItemList[] = 
    {
      { 0, MAIL$_SEND_TO_LINE, ToDisplay,  0 },
      { 0, MAIL$_SEND_FROM_LINE, FromDisplay,  0 },
      { 0, MAIL$_SEND_CC_LINE, CCDisplay,  0 },
      { 0, MAIL$_SEND_SUBJECT, MsgSubject, 0 },
      END_OF_ITEM_LIST_MARKER
    };

    ItemList_t BodyItemList[] = 
    {
	{ 0, MAIL$_SEND_FILENAME, MsgFile, 0 },
	END_OF_ITEM_LIST_MARKER
    };

    AttrItemList[0].BufferLength = strlen(ToDisplay);
    AttrItemList[1].BufferLength = strlen(FromDisplay);
    AttrItemList[2].BufferLength = strlen(CCDisplay);
    AttrItemList[3].BufferLength = strlen(MsgSubject);

    if (mail$send_add_attribute(&SendContext, AttrItemList,
				&NullItemList) == SS$_NORMAL)
    {
	BodyItemList[0].BufferLength = strlen(MsgFile);

	if (mail$send_add_bodypart(&SendContext, BodyItemList,
				   &NullItemList) != SS$_NORMAL)
	    printf("%s: Problem with message file\n", progname);
    }
    else
	printf("%s: Problem adding attributes\n", progname);
}

/* Exit codes for success and failure.  */
#ifdef VMS
#define	GOOD	1
#define BAD	0
#else
#define	GOOD	0
#define	BAD	1
#endif

/* Print error message and exit.  */

/* VARARGS1 */
void
fatal (s1, s2)
     char *s1, *s2;
{
  error (s1, s2);
  exit (BAD);
}

/* Print error message.  `s1' is printf control string, `s2' is arg for it. */

/* VARARGS1 */
void
error (s1, s2)
     char *s1, *s2;
{
  fprintf (stderr, "%s: ", progname);
  fprintf (stderr, s1, s2);
  fprintf (stderr, "\n");
}

/* Like malloc but get fatal error if memory is exhausted.  */

char *
xmalloc (size)
     unsigned int size;
{
  char *result = (char *) malloc (size);
  if (result == NULL)
    fatal ("virtual memory exhausted", 0);
  return result;
}

char *
xrealloc (ptr, size)
     char *ptr;
     unsigned int size;
{
  char *result = (char *) realloc (ptr, size);
  if (result == NULL)
    fatal ("virtual memory exhausted", 0); /* ttn */
  return result;
}

/* vmssendmail.c ends here */
