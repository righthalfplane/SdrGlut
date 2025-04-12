/****************************************************************************************
 *
 *	File:		dirent.c
 *	Created:	7/3/93		By:	George T. Talbot
 *	Purpose:	Implements UNIX-like directory reading for the Macintosh.
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

#if defined(MAC) || defined(macintosh) || defined(SYMANTEC_C)

/*
 * Added to HDF core library for directory reading on the Macintosh
 *
 * 1996/03/01 - GeorgeV.
 */

#ifdef RCSID
static char RcsId[] = "@(#)$Revision: 3780 $";
#endif

/* $Id: dir_mac.c 3780 1999-05-26 16:39:47Z epourmal $ */

#include "dir_mac.h"
#if 0
#include <pascal.h>
#endif
#include <string.h>
#include <ctype.h>
#include <Errors.h>

OSErr	dd_errno;				/*	Global errno to check after calls to dirent routines	*/
char	*dd_separator = ":";	/*	If you're feeling brave, change this to "/"	*/
int		dd_xform_seps = false;

/****************************************************************************************
 *
 *	This function, given a Macintosh-style pathname, will open a directory to that path.
 *	NOTES:	1)	passing in nil will get you the current directory.
 *			2)	".:", "..:" & "Å:" are supported at the beginning of paths ONLY
 *				by this routine.
 *			3)	"/" will be turned into ":" by this routine.
 *
 *	Calls:			PBHGetVol(), PBHGetCatInfo(), PBHSetVol(), hopendir(), CtoPstr()
 *	Called By:		<general purpose>
 *	Globals Used:	dd_errno
 *	Parameters:		pointer to C-string pathname or nil for current directory
 *	Returns:		pointer to directory management block or nil & dd_errno will be set
 *
 ****************************************************************************************/

DIR	*
opendir(char *dirname)
{
	WDPBRec			pb;
	CInfoPBRec		cpb;
	short			vRefNum;
	long			dirID;
	char			*dname;
	DIR				*temp = NULL;
	char			path_temp[MAXPATHLEN+1];	/*	Temporary area for building pathname	*/

	/*	Save the current path	*/
	pb.ioCompletion	= nil;
	pb.ioNamePtr	= nil;
	
	if ((dd_errno = PBHGetVolSync(&pb)) != noErr)
		return nil;

	vRefNum	= pb.ioWDVRefNum;
	dirID	= pb.ioWDDirID;

	/*	dname points to the desired pathname	*/
	dname	= dirname;

	/*	If no pathname was passed in, or there are no ".", ".." or "Å" special directory
	 *	names, then handle the pathname as normal.
	 */
	if (dirname == nil)
		goto opendir_fallthrough;

	/*	If there's not '.', '..' or 'Å', fall through	*/
	if ((dirname[0] != '.') && (dirname[0] != 'Å'))
		goto opendir_fallthrough;

	/*	If there's a 'Å', treat it like '..'	*/
	if (dirname[0] == 'Å')
      {
          dname = &(dirname[1]);
          goto path_dotdot;
      }

	/*	If the pathname has "." (current directory) in front of it...	*/
	if (dirname[1] != '.')
      {
          /*	Skip over the "." and fall through	*/
          dname	= &(dirname[1]);
          goto opendir_fallthrough;
      }

	/*	Skip over the ".."	*/
	dname	= &(dirname[2]);

  path_dotdot:
	/*	If we get here, the directory has ".." in front of it...	*/
	
	/*	First, get the directory info on the current directory.  We do this so
	 *	that we can get the directory's parent
	 */
	cpb.dirInfo.ioCompletion	= nil;
	cpb.dirInfo.ioNamePtr		= (unsigned char *)path_temp;	/* Unused, but must be set because of
                                                                 * bug in Apple File Sharing.
                                                                 */
	cpb.dirInfo.ioVRefNum		= vRefNum;
	cpb.dirInfo.ioFDirIndex		= -1;
	cpb.dirInfo.ioDrDirID		= dirID;

	if ((dd_errno = PBGetCatInfoSync(&cpb)) != noErr)
		return nil;

	/*	Temporarily CD to the parent directory	*/
	pb.ioCompletion				= nil;
	pb.ioNamePtr				= nil;
	pb.ioVRefNum				= pb.ioWDVRefNum;
	pb.ioWDDirID				= cpb.dirInfo.ioDrParID;
	
	if ((dd_errno = PBHSetVolSync(&pb)) != noErr)
		return nil;

	/*	This is the common code for all three cases above	*/
  opendir_fallthrough:
	/*	If the pathname is too long (this is a Macintosh FS constraint), then return	*/
	if (strlen(dname) > MAXPATHLEN)
      {
          /*	Set the error	*/
          dd_errno	= bdNamErr; /* -37 */
          temp		= nil;
		
          /*	Go to the common exit, where we CD back to the saved WD	*/
          goto opendir_exit;
      }

	/*	If this call was passed a pathname	*/
	if (dname != nil)
      {
          /*	Copy the pathname into a temp	*/
          strcpy(path_temp, dname);
		
          /*	Turn it into a Pascal string for the Mac FS	*/
          c2pstr(path_temp);

          /*	Change any "/" to ":" for the Mac FS	*/
          if (dd_xform_seps)
			{
                int i;
			
                for (i=1; i<= path_temp[0]; ++i)
                    if (path_temp[i] == '/')
                        path_temp[i] = ':';
			}

          /*	Try and open the directory	*/
          if ((vRefNum != 0) && (dirID != 0))		{
	          temp = hopendir(path_temp, vRefNum, dirID);
	      }
	      if (temp == NULL)
	          temp = hopendir(path_temp, 0, 0);
      }
	else
		/*	If this call wasn't passed a pathname, then we call hopendir() with nil to
		 *	tell it to open the current working directory.
		 */
		temp = hopendir(nil, 0, 0);

	/*	This is the common exit code which restores the current WD	*/
  opendir_exit:
	pb.ioCompletion				= nil;
	pb.ioNamePtr				= nil;
	pb.ioVRefNum				= vRefNum;
	pb.ioWDDirID				= dirID;
	
	if ((dd_errno = PBHSetVolSync(&pb)) != noErr)
      {
          /*	If this call failed, then get rid of the structures created by hopendir()	*/
          closedir(temp);
          return nil;
      }

	return temp;
}

/****************************************************************************************
 *
 *	This function actually opens the directory.  If you feel brave, you can call it.
 *	If you pass in a dirname, then set vRefNum and dirID to 0.  All named opens are
 *	relative to the current WD.  If you pass in vRefNum and dirID, then don't bother
 *	passing in a name.  This routine WILL CHANGE YOUR CURRENT WORKING DIRECTORY!
 *
 *	Calls:			NewHandle(), PBHGetCatInfo(), PBHSetVol(), PtoCstr(), BlockMove(),
 *					DisposeHandle(), MoveHHi(), HLock(), MemError()
 *	Called By:		opendir(), and you if you feel brave.
 *	Globals Used:	dd_errno
 *	Parameters:		pointer to Pascal-string pathname, vRefNum, dirID of desired
 *					directory.  If you pass in a WDRefNum as the vRefNum, set dirID to 0
 *	Returns:		pointer to directory management block or nil & dd_errno will be set
 *
 ****************************************************************************************/

DIR	*
hopendir(char *dirname, short vRefNum, long dirID)
{
	DIR				**curh, *cur;
	CInfoPBRec		cpb;
	WDPBRec			pb;
	Str63			name;

	/*	Get memory for the directory structure	*/
	curh	= (DIR **) NewHandle(sizeof(DIR));

	/*	Did we get it?	*/
	if (curh == nil)
      {
          dd_errno	= MemError();
          return nil;
      }

	/*	Move it high and lock it	*/
	MoveHHi((char **)curh);
	HLock((char **)curh);
	cur		= *curh;

	/*	If we're supposed to open anything but the current directory, set the current
	 *	working directory to the desired directory.
	 */
	if ((dirname != nil) || (vRefNum != 0) || (dirID != 0))
      {
          pb.ioCompletion				= nil;
          pb.ioNamePtr				= (unsigned char *)dirname;
          pb.ioVRefNum				= vRefNum;
          pb.ioWDDirID				= dirID;
		
          if ((dd_errno = PBHSetVolSync(&pb)) != noErr)		{
              /* a trailing colon could cause trouble */
	          if (dirname[dirname[0]] == ':')		{
	              dirname[0]--;
		          dd_errno = PBHSetVolSync(&pb);
		          dirname[0]++;
		      }
		  }

          if (dd_errno == noErr)	
            {  /* make vRefNum and dirId valid & up-to-date */
            	pb.ioNamePtr = NULL;
         		dd_errno = PBHGetVolSync(&pb);
                vRefNum = pb.ioVRefNum;
                dirID = pb.ioWDDirID;
            }
      }


	cur->dd_buf	= nil;
	
	/*	Get info on the desired directory (its name, etc.)	*/
	cpb.dirInfo.ioCompletion	= nil;
	cpb.dirInfo.ioNamePtr		= name;
	cpb.dirInfo.ioVRefNum		= vRefNum;
	cpb.dirInfo.ioFDirIndex		= -1;
	cpb.dirInfo.ioDrDirID		= dirID;
	
	if ((dd_errno = PBGetCatInfoSync(&cpb)) != noErr)
		goto failure_exit;

	/*	Save the directory info	*/
	cur->dir_fsp.vRefNum	= vRefNum;
	cur->dd_fd				= cpb.dirInfo.ioDrDirID;
	cur->dd_parent			= cpb.dirInfo.ioDrParID;

	BlockMove(name, cur->dir_fsp.name, sizeof(Str63));

	/*	Convert the name to a C-style string	*/
	p2cstr(cur->dir_fsp.name);

	/*	Set up our directory structure to read the first entry	*/
	cur->dd_off				= 1;
	cur->dd_numents			= cpb.dirInfo.ioDrNmFls;
	cur->dd_cached			= false;

	return cur;

	/*	This code is branched-to in case of error.  It frees up the memory and returns.	*/
  failure_exit:
	DisposeHandle((char **) curh);
	return nil;
}

/****************************************************************************************
 *
 *	This function returns the index of the directory entry to be next read.
 *
 *	Calls:			nothing
 *	Called By:		<general purpose>
 *	Globals Used:	none
 *	Parameters:		pointer to the directory management block
 *	Returns:		index of the next directory entry to be read.
 *
 ****************************************************************************************/

long	
telldir(DIR *dirp)
{
	if (dirp->dd_off > dirp->dd_numents)
		return -1;
	else
		return dirp->dd_off-1;	/* The -1 is because Macs start at 1 & not 0 for dir index,
								 * and this is a little more POSIX.
								 */
}

/****************************************************************************************
 *
 *	This function closes the directory opened with opendir() or hopendir()
 *
 *	Calls:			DisposeHandle(), RecoverHandle()
 *	Called By:		<general purpose>
 *	Globals Used:	none
 *	Parameters:		pointer to the directory management block
 *	Returns:		0 (always successful)
 *
 ****************************************************************************************/

int	
closedir(DIR *dirp)
{
	struct dirent	**cur;
	
	/*	Dispose of any directory entries read in.	*/
	cur	= dirp->dd_buf;
	
	dd_errno	= noErr;

	while (cur)
      {
          struct dirent	**next;
		
          next	= (*cur)->next;
		
          DisposeHandle((Handle) cur);
		
          if (dd_errno == noErr)
              dd_errno	= MemError();

          cur		= next;
      }

	/*	Dispose of the directory managment block	*/
	DisposeHandle(RecoverHandle((Ptr) dirp));

	if (dd_errno == noErr)
		dd_errno	= MemError();

	return dd_errno?-1:0;
}

/****************************************************************************************
 *
 *	This function sets the index of the next-read directory entry.  It will also search
 *	the list of read entries so that an entry won't be read from disk more than once.
 *
 *	Calls:			nothing
 *	Called By:		<general purpose>
 *	Globals Used:	none
 *	Parameters:		pointer to the directory management block, index of directory
 *	Returns:		nothing
 *
 ****************************************************************************************/

void	
seekdir(DIR *dirp, long loc)
{
	struct dirent	**cur;
	
	dirp->dd_off		= loc+1;	/* The +1 is because the Mac indexes directories
									 * from 1 and not 0 and we want to be a little bit
									 * POSIX
									 */

	/*	Search through the entries that we've read already	*/
	cur	= dirp->dd_buf;
	
	while (cur)
      {
          /*	If we find the entry that we've seeked to, set up so that readdir() will
           *	return this one instead of reading a new one.
           */
          if (loc == (*cur)->d_off)
			{
                dirp->dd_cached		= true;
                dirp->dd_cache_hint	= cur;

                return;
			}

          cur	= (*cur)->next;
      }

	/*	If we didn't find it, then tell readdir() to get the entry from the FS	*/
	dirp->dd_cached	= false;
}

/****************************************************************************************
 *
 *	This function will read the next directory entry from disk.  It will return nil and
 *	set dd_errno to noErr when the end of the directory is reached.  It will avoid
 *	reading directory entries from disk more than once.
 *
 *	Calls:			nothing
 *	Called By:		<general purpose>
 *	Globals Used:	none
 *	Parameters:		pointer to the directory management block
 *	Returns:		pointer to directory entry or nil if an error occurred and dd_errno
 *					will be set.  If the last entry has already been read, this will
 *					return nil and dd_errno will be set to noErr.
 *
 ****************************************************************************************/

struct dirent	*
readdir(DIR *dirp)
{
	CInfoPBRec		cpb;
	struct dirent	**meh, *me;
	
	/*	If the entry has been read already, then return the already present entry	*/
	if (dirp->dd_cached)
		me	= *(dirp->dd_cache_hint);
	else
		/*	Otherwise, read it from disk...	*/
      {
          /*	Past the end of the directory?	*/
          if (dirp->dd_off > dirp->dd_numents)
			{
                dd_errno	= noErr;
                return nil;
			}

          /*	Allocate space for a new entry	*/
          meh	= (struct dirent **) NewHandle(sizeof(struct dirent));
		
          /*	Enough memory?	*/
          if (meh == nil)
			{
                dd_errno	= MemError();
                return nil;
			}

          /*	Lock the entry	*/
          MoveHHi((Handle) meh);
          HLock((Handle) meh);

          me	= *meh;

          /*	Get the entry's info from disk	*/
          me->fsp.name[0]				= 0;

          cpb.dirInfo.ioCompletion	= nil;
          cpb.dirInfo.ioNamePtr		= me->fsp.name;
          cpb.dirInfo.ioVRefNum		= dirp->dir_fsp.vRefNum;
          cpb.dirInfo.ioFDirIndex		= dirp->dd_off;
          cpb.dirInfo.ioDrDirID		= dirp->dd_fd;

          if ((dd_errno = PBGetCatInfoSync(&cpb)) != noErr)
			{
                DisposeHandle((Handle) meh);
                return nil;
			}
	
          /*	Set up the dirent structure	*/
          me->d_off			= dirp->dd_off-1;
          me->fsp.vRefNum		= cpb.dirInfo.ioVRefNum;
          me->d_fileno		= cpb.dirInfo.ioDrDirID;
          me->d_parent		= cpb.dirInfo.ioDrParID;
		
          /*	C strings only!	*/
          p2cstr(me->fsp.name);

          /*	Add it to the list for this directory	*/
          me->next			= dirp->dd_buf;
		
          dirp->dd_buf		= meh;
      }

	/*	Seek to the next entry	*/
	seekdir(dirp, dirp->dd_off);

	/*	Return what we've found	*/
	return me;
}

/****************************************************************************************
 *
 *	This function will give an absolute pathname to a given directory.
 *
 *	Calls:			NewPtr(), DisposePtr(), PBGetCatInfo()
 *	Called By:		<general purpose>
 *	Globals Used:	none
 *	Parameters:		vRefNum and startDirID of desired path, pointer to path name storage,
 *					length of path name storage, pointer to C-string separator.
 *	Returns:		bdNamErr if the path would overflow the storage,
 *					some other error code if something else happened,
 *					or noErr on success.
 *
 ****************************************************************************************/

OSErr	
hgetwd(short vRefNum, long startDirID, char *path, int max_path_len, char *sep)
{
	long		curDirID;
	OSErr		err;
	CInfoPBRec	pb;
	Str63		name;
	char		*temp_path;

	/*	Start with an empty path	*/
	path[0]	= 0;

	/*	Get memory for a temporary path	*/
	temp_path	= (char *) NewPtr(max_path_len);
	
	if (temp_path == nil)
		return MemError();

	/*	Start at the given directory	*/
	curDirID	= startDirID;

	do	{
		/*	Get cat info for the current directory	*/
		name[0]	= 0;

		pb.dirInfo.ioCompletion	= nil;
		pb.dirInfo.ioNamePtr	= name;
		pb.dirInfo.ioVRefNum	= vRefNum;
		pb.dirInfo.ioFDirIndex	= -1;
		pb.dirInfo.ioDrDirID	= curDirID;
		
		if ((err = PBGetCatInfoSync(&pb)) != noErr)
          {
              DisposePtr((Ptr) temp_path);
              return err;
          }

		/*	Convert name to a C string	*/
		p2cstr(name);

		/*	Check that we don't overflow storage	*/
		if ((strlen((char const *)name) + strlen(path) + strlen(sep)) >= max_path_len)
          {
              DisposePtr((Ptr) temp_path);
              return bdNamErr; /* -37 */
          }

		/*	Prepend the name and separator	*/
		strcpy(temp_path, path);
		strcpy(path, (char const *)name);
		strcat(path, sep);
		strcat(path, temp_path);

		/*	Move "up" one directory	*/
		curDirID	= pb.dirInfo.ioDrParID;
    }
	/*	Until we hit the root directory	*/
	while (pb.dirInfo.ioDrDirID != fsRtDirID);

	/*	Get rid of our temp storage and return	*/
	DisposePtr((Ptr) temp_path);

	return MemError();
}

/****************************************************************************************
 *
 *	This function will change the current working directory.
 *
 *	Calls:			opendir(), closedir(), PBHSetVol()
 *	Called By:		<general purpose>
 *	Globals Used:	none
 *	Parameters:		C-string pathname.
 *	Returns:		-1 on failure, 0 on success.  Sets dd_errno on failure.
 *
 ****************************************************************************************/

int	
chdir(char *path)
{
	DIR		*d;
	short	vRefNum;
	long	dirID;
	WDPBRec	pb;

	/*	Open the directory	*/
	d	= opendir(path);
	
	if (d == nil)
		return -1;

	/*	Get the Mac FS identification for this directory	*/
	vRefNum	= d->dd_volume;
	dirID	= d->dd_fd;
	
	/*	Close the directory	*/
	closedir(d);

	/*	CD to the new directory	*/
	pb.ioCompletion	= nil;
	pb.ioNamePtr	= nil;
	pb.ioVRefNum	= vRefNum;
	pb.ioWDDirID	= dirID;
	
	dd_errno = PBHSetVolSync(&pb);
	
	return dd_errno?-1:0;
}

/****************************************************************************************
 *
 *	This function will get the current working directory's path.
 *
 *	Calls:			PBHGetVol(), hgetwd()
 *	Called By:		<general purpose>
 *	Globals Used:	none
 *	Parameters:		pointer to a buffer of MAXPATHLEN bytes.
 *	Returns:		pointer to the buffer on success, on failure, nil and dd_errno will
 *					be set.
 *
 ****************************************************************************************/

char *
getwd(char *path)
{
	WDPBRec	pb;

	/*	Get the current working directory	*/
	pb.ioCompletion	= nil;
	pb.ioNamePtr	= nil;
	
	if ((dd_errno = PBHGetVolSync(&pb)) != noErr)
		return nil;

	/*	Transform it into a path	*/
	if ((dd_errno = hgetwd(pb.ioWDVRefNum, pb.ioWDDirID, path, MAXPATHLEN-1, dd_separator)) != noErr)
		return nil;

	return path;
}

/****************************************************************************************
 *
 *	This function will get the path to a given (already opened) directory.
 *
 *	Calls:			hgetwd()
 *	Called By:		<general purpose>
 *	Globals Used:	none
 *	Parameters:		pointer to a buffer of MAXPATHLEN bytes.
 *	Returns:		pointer to the buffer on success, on failure, nil and dd_errno will
 *					be set.
 *
 ****************************************************************************************/

char *
pathdir(DIR *dirp, char *path)
{
	if ((dd_errno = hgetwd(dirp->dd_volume, dirp->dd_fd, path, MAXPATHLEN-1, dd_separator)) != noErr)
		return nil;

	return path;
}

#endif /* defined(MAC) || defined(macintosh) || defined(SYMANTEC_C) */
