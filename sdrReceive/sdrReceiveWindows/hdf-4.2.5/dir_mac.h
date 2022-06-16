/****************************************************************************************
 *
 *	File:		dirent.h
 *	Created:	7/3/93		By:	George T. Talbot
 *	Purpose:	Implements UNIX-like directory reading for the Macintosh.
 *				This file describes Filesystem-independent directory information.
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

/* $Id: dir_mac.h 3341 1998-02-02 21:38:48Z smitchel $ */

#ifndef	__dirent_h
#define	__dirent_h

#if defined(MAC) || defined(macintosh) || defined(SYMANTEC_C)
/*
 * Added to HDF core library for directory reading on the Macintosh
 *
 * 1996/03/01 - GeorgeV.
 */

#include <Files.h>

/*	Maximum path length for opendir()	*/
#define	MAXPATHLEN	255

/*
 * Definitions for library routines operating on directories.
 */
typedef	struct __dirdesc {
	/*	PRIVATE FIELDS.  Use the fields & defines below PUBLIC */
	FSSpec	dir_fsp;
	long	dd_off;		/* Current offset (ioWDIndex) in dir (for telldir) */
	int		dd_cached;	/* true if dd_cache_hint points to the next dir to be read	*/

	struct dirent	**dd_cache_hint;
	struct dirent	**dd_buf;	/* directory data buffer */

	/*	PUBLIC	*/
	long	dd_fd;							/* file descriptor (dirID) of this dir	*/

#define	dd_parent	dir_fsp.parID			/* dirID of parent	*/
#define	dd_bsize	1						/* amount of entries read at a time */
#define	dd_size		sizeof(struct dirent)	/* amount of valid data in buffer */
#define	dd_loc		1
#define	dd_name		dir_fsp.name
#define	dd_volume	dir_fsp.vRefNum

	long	dd_numents;	/* Number of files/directories in this directory	*/
} DIR;

/*	See dirent.c for descriptions of these routines	*/
extern	DIR	*opendir(char *dirname);
extern	struct dirent *readdir(DIR *dirp);
extern	int closedir(DIR *dirp);
extern	void seekdir(DIR *dirp, long loc);
extern	long telldir(DIR *dirp);

#ifndef	lint
	#define	rewinddir(dirp)	seekdir((dirp), (long)0)
#else
	extern	void rewinddir(DIR *dirp);
#endif

/*	Convenient routines	*/
extern char	*getwd(char *path);
extern int	chdir(char *path);
extern char	*pathdir(DIR *dirp, char *path);

/*	Any errors in above routines (and hopendir()) are put here	*/
extern OSErr	dd_errno;
extern char		*dd_separator;	/*	If you're feeling brave, change this to "/"	*/
extern int		dd_xform_seps;

/*	In case you like errno instead	*/
#ifdef UNCOMMENT_ME_IF_YOUVE_GOT_TO_HAVE_ERRNO
#define	errno	dd_errno
#endif

/*	This routine is Mac-specific, but very convenient in certain situations	*/
OSErr	hgetwd(short vRefNum, long startDirID, char *path, int max_path_len, char *sep);

/*	You probably don't want to call this.	*/
extern	DIR	*hopendir(char *dirname, short vRefNum, long dirID);

#include "sys_dir_mac.h"

#endif /* defined(MAC) || defined(macintosh) || defined(SYMANTEC_C)  */
#endif	/* !__dirent_h */
