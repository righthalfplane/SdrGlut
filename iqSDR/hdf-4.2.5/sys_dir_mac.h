/****************************************************************************************
 *
 *	File:		sys_dirent.h
 *	Created:	7/3/93		By:	George T. Talbot
 *	Purpose:	Implements UNIX-like directory reading for the Macintosh.
 *				Filesystem-independent directory information.
 *
 *	Modifications:
 *
 *	Notes:
 *			1) These routines will NOT work under A/UX.
 *			2) WD = working directory
 *			3) CD = change directory
 *			4) FS = file system
 *			5) Mac filesystems allow spaces as part of pathnames!
 *			6) All routines which return a path use the default Macintosh path separator,
 *			   a colon (":").
 *
 ****************************************************************************************/

/* $Id: sys_dir_mac.h 3342 1998-02-02 21:52:53Z smitchel $ */

#ifndef	__sys_dirent_h
#define	__sys_dirent_h

#if defined(MAC) || defined(macintosh) || defined(SYMANTEC_C)

#include <Files.h>

struct	dirent {
	/* PRIVATE FIELDS.  Use fields after PUBLIC	*/
	struct dirent	**next;
	FSSpec			fsp;
	
	/*	PUBLIC.	*/
	long			d_off;					/* index (to seekdir()) of this entry */	
	long			d_fileno;				/* File number (dirID) of this entry	*/
#define	d_parent	fsp.parID				/* File number (dirID) of parent	*/
#define	d_reclen	sizeof(struct dirent)	/* Size of this record	*/
#define	d_namelen	strlen(fsp.name)		/*	Length of the name	*/
#define	d_name		fsp.name				/*	Name	*/
#define	d_volume	fsp.vRefNum
};

#define	DIRSIZ(dp) sizeof(struct dirent)

#endif /* defined(MAC) || defined(macintosh) || defined(SYMANTEC_C)  */
#endif	/* !__sys_dirent_h */
