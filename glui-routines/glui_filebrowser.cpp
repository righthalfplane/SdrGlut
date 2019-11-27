/****************************************************************************
  
  GLUI User Interface Toolkit
  ---------------------------

     glui_filebrowser.cpp - GLUI_FileBrowser control class


          --------------------------------------------------

  Copyright (c) 1998 Paul Rademacher

  This software is provided 'as-is', without any express or implied 
  warranty. In no event will the authors be held liable for any damages 
  arising from the use of this software. 

  Permission is granted to anyone to use this software for any purpose, 
  including commercial applications, and to alter it and redistribute it 
  freely, subject to the following restrictions: 

  1. The origin of this software must not be misrepresented; you must not 
  claim that you wrote the original software. If you use this software 
  in a product, an acknowledgment in the product documentation would be 
  appreciated but is not required. 
  2. Altered source versions must be plainly marked as such, and must not be 
  misrepresented as being the original software. 
  3. This notice may not be removed or altered from any source distribution. 


*****************************************************************************/

#ifdef _WIN32
#include <windows.h>
#endif

#include "GL/glui.h"
#include "glui_internal.h"
#include <sys/types.h>

#ifdef __GNUC__
#include <dirent.h>
#include <unistd.h>
#endif


#include <sys/stat.h>

GLUI_FileBrowser::GLUI_FileBrowser( GLUI_Node *parent, 
                                    const char *name,
                                    int type,
                                    int id,
                                    GLUI_CB cb)
{
  common_init();

  set_name( name );
  user_id    = id;
  int_val    = type;
  callback   = cb;
  fillbox    = NULL;
  parent->add_control( this );
  list = new GLUI_List(this, true, 1);
  list->set_object_callback( GLUI_FileBrowser::dir_list_callback, this );
  list->set_click_type(GLUI_DOUBLE_CLICK);
  this->fbreaddir(this->current_dir.c_str());
}

/****************************** GLUI_FileBrowser::draw() **********/
#ifdef __GNUC__
extern "C" int goCD(char *name);
#endif
void GLUI_FileBrowser::dir_list_callback(GLUI_Control *glui_object) {
  GLUI_List *list = dynamic_cast<GLUI_List*>(glui_object);
  if (!list) 
    return;
  GLUI_FileBrowser* me = dynamic_cast<GLUI_FileBrowser*>(list->associated_object);
  if (!me)
    return;
  int this_item;
  const char *selected;
  this_item = list->get_current_item();
  if (this_item > 0) { /* file or directory selected */
    selected = list->get_item_ptr( this_item )->text.c_str();
    if (selected[0] == '/' || selected[0] == '\\') {
     if (me->allow_change_dir) {
#ifdef __GNUC__
		if(selected[1] == '.' || selected[2] == '.')
		{
			 goCD((char *)"../");
		}
		else
		{
		     goCD((char *)(selected+1));
		}
#endif
#ifdef _WIN32
        SetCurrentDirectory(selected+1);
#endif
        me->fbreaddir(".");
      }
    } else {
      me->file = selected;
      me->execute_callback();
    }
  }
}

char *strsave(const char *name)
{
	long n=(long)strlen(name);
	char *p=(char *)malloc(n+1);
	for(long i=0;i<n;++i)p[i]=name[i];
	p[n]=0;
	return p;
}
int goUpper(char *a,int len)
{
	if(!a)return 1;
	
	while(*a && (--len >= 0)){
		*a=toupper(*a);
		++a;
	}
	return 0;
}
int mstrncpy(char *out,char *in,long n)
{
	if(!out || !in || (n <= 0))return 1;
	
	while(n-- > 0){
		if(*in == 0){
			*out = 0;
			break;
		}else{
			*out++ = *in++;
		}
	}
	
	return 0;
}
int intcmp(const void *xx,const  void *yy)
{
    char xb[4024],yb[4024];
	char **x=(char **)xx;
	char **y=(char **)yy;
	mstrncpy(xb,*x,4024);
	goUpper(xb,4024);
	mstrncpy(yb,*y,4024);
	goUpper(yb,4024);
	return strcmp(xb,yb);
	/* return strcmp(*x,*y); */
}	
void GLUI_FileBrowser::fbreaddir(const char *d) {
  GLUI_String item;
  int i = 0;
	
	if (!d)
    return;

#ifdef _WIN32

  WIN32_FIND_DATA FN;
  HANDLE hFind;
  //char search_arg[MAX_PATH], new_file_path[MAX_PATH];
  //sprintf(search_arg, "%s\\*.*", path_name);
  
  hFind = FindFirstFile("*.*", &FN);
  if (list) {
    if (hFind != INVALID_HANDLE_VALUE) {
	  list->delete_all();
      do {
        int len = (int)strlen(FN.cFileName);
        if (FN.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
          item = '\\';
          item += FN.cFileName;
        } else {
          item = FN.cFileName;
        }
        list->add_item(i,item.c_str());
        i++;
      } while (FindNextFile(hFind, &FN) != 0);
      
		list->set_start_line(0);
		
      if (GetLastError() == ERROR_NO_MORE_FILES)
        FindClose(hFind);
      else
        perror("fbreaddir");
    }
  }

#elif defined(__GNUC__)

  DIR *dir;
  struct dirent *dirp;
  struct stat dr;

  if (list) {
    if ((dir = opendir(d)) == NULL)
	{
      perror("fbreaddir:");
    }else {
		
	  int count = 0;
		
	  while ((dirp = readdir(dir)) != NULL) {
		++count;
	  }
		
	  if(count == 0)return;
		
	  rewinddir(dir);
		
	 char **listNames= new char*[count];
		
	if(listNames == NULL)
	{
		closedir(dir);
		return;
	}
		
	count=0;
		
	list->delete_all();
		
     while ((dirp = readdir(dir)) != NULL)   /* open directory     */
      { 
        if (!lstat(dirp->d_name,&dr) && S_ISDIR(dr.st_mode)) /* dir is directory   */
		{
		  if(dirp->d_name[0] == '.')
		  {
			  if(strlen(dirp->d_name) > 1)
			  {
				  if(dirp->d_name[1] != '.')continue;
			  }
		  }
          item = GLUI_String("/") + dirp->d_name;
		}
        else
		{
		  if(dirp->d_name[0] == '.')continue;
          item = dirp->d_name;
		}

		listNames[count]=strsave(item.c_str());
		  
		if(listNames[count] == NULL)break;
		  
		++count;
		  
      }
      closedir(dir);
		
	  qsort((char *)listNames, count, sizeof(char *), intcmp);
		
		for(i=0;i<count;++i)
		{
			list->add_item(i,listNames[i]);
			free(listNames[i]);
		}
		
	  delete[] listNames;
		
	  list->set_start_line(0);
    }
  }
#endif
   if(fillbox)(*fillbox)();
}
void ProcessFiles(const char *path_name)
{	

}


void GLUI_FileBrowser::set_w(int w) 
{ 
  if (list) list->set_w(w);
}

void GLUI_FileBrowser::set_h(int h) 
{
  if (list) list->set_h(h);
}
