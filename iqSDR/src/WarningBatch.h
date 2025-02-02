#ifndef WARNINGBATCH_H
#define WARNINGBATCH_H

#include "iqSDR.h"

#define FILE_NAME_SEPERATOR '/'

#define wxUSE_FONTDLG 1

class Utilities
{
public:
	Utilities()
	{
		name=NULL;
		clipedName=NULL;
	}
	
	~Utilities()
	{
		if(name)delete[] name;
		if(clipedName)delete[] clipedName;
	}
	
	char *GetClipedName(char *name)
	{
	    char *fp=strrchr((char *)name,FILE_NAME_SEPERATOR);
	    if(fp)
	    {
	    	name=fp+1;
	    }
	    if(clipedName)delete[] clipedName;
	    clipedName=new char[strlen(name)+1];
	    strcpy(clipedName,name);
		return clipedName;
	}

	char *GetPrunedName(char *name)
	{
	    if(clipedName)delete[] clipedName;
	    clipedName=new char[strlen(name)+1];
	    strcpy(clipedName,name);
	    char *fp=strrchr((char *)clipedName,'.');
	    if(fp)
	    {
	    	*fp=0;
	    }
		return clipedName;
	}

private:

	char *name;
	char *clipedName;

};


class BatchWindow: public wxFrame
{
public:
	BatchWindow();
    void OnQuit(wxCommandEvent& event);
    void OnOpen(wxCommandEvent& event);
    void OnFileSave(wxCommandEvent& event);
    void OnClose(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnClose2(wxCloseEvent& event); 
    
#if wxUSE_FONTDLG
    void ChooseFont(wxCommandEvent& event);
#endif // USE_FONTDLG_GENERIC
	wxTextCtrl *text;
private:
    DECLARE_EVENT_TABLE()
};

void BatchWarning(ostringstream& outs);

void winout(const char *fmt, ...);

#endif