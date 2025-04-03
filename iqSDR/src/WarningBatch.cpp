#include "WarningBatch.h"
#include <mutex>

#if wxUSE_FONTDLG
    #include "wx/fontdlg.h"
#endif // wxUSE_FONTDLG


using namespace std;

void WarningBatch(const char *message);
void BatchWarning(ostringstream& outs);

#define POINT_INCREMENT 10

static struct dataStructHold BatchHold;
static int checkBatchHold(struct dataStructHold *plane);

std::mutex mutexb;

extern void *cRealloc(char *p,unsigned long r,int tag);

enum
{
    // menu items
    Minimal_Quit = 1,
    Minimal_Test,
    Minimal_Test2,
    Minimal_About,
    DIALOGS_CHOOSE_FONT,

};

BEGIN_EVENT_TABLE(BatchWindow, wxFrame)
    EVT_MENU(wxID_EXIT, BatchWindow::OnQuit)
    EVT_MENU(wxID_OPEN, BatchWindow::OnOpen)
    EVT_MENU(wxID_CLOSE, BatchWindow::OnClose)
    EVT_CLOSE( BatchWindow::OnClose2)
    EVT_MENU(wxID_SAVE, BatchWindow::OnFileSave)
//    EVT_IDLE(BatchWindow::OnIdle)
#if wxUSE_FONTDLG
    EVT_MENU(DIALOGS_CHOOSE_FONT, BatchWindow::ChooseFont)
#endif // USE_FONTDLG_GENERIC
    EVT_MENU(wxID_ABOUT, BatchWindow::OnAbout)
END_EVENT_TABLE()

void BatchWindow::OnIdle(wxIdleEvent& event)
{
	fprintf(stderr,"OnIdle\n");
	event.Skip();
}

 BatchWindow::BatchWindow():
      wxFrame(NULL, wxID_ANY, wxT("BatchPrint"), wxDefaultPosition, wxSize(650,500))
 {
#ifdef __WXMAC__
    // we need this in order to allow the about menu relocation, since ABOUT is
    // not the default id of the about menu
    wxApp::s_macAboutMenuItemId = wxID_ABOUT;
#endif
    // create a menu bar
    wxMenu *menuFile = new wxMenu(_T(""), wxMENU_TEAROFF);
    menuFile->Append(wxID_OPEN, _T("&Open...\tCtrl-O"), _T("Open New File"));
    menuFile->Append(wxID_SAVE, _T("&Save...\tCtrl-S"), _T("Save Text"));
    menuFile->Append(wxID_CLOSE, _T("Close\tCtrl-W"), _T("Close Window"));
    menuFile->Append(wxID_EXIT, _T("E&xit\tAlt-X"), _T("Quit this program"));
    
    
    
    wxMenu *menuEdit = new wxMenu(_T(""), wxMENU_TEAROFF);
#if wxUSE_FONTDLG
    menuEdit->Append(DIALOGS_CHOOSE_FONT, _T("Choose &font"));
    menuEdit->AppendSeparator();
#endif // USE_FONTDLG_GENERIC
    menuEdit->Append(wxID_SELECTALL, _T("&Select All...\tCtrl-A"), _T("Select All"));
    menuEdit->Append(wxID_CUT, _T("&Cut...\tCtrl-X"), _T("Cut Text"));
    menuEdit->Append(wxID_COPY, _T("&Copy...\tCtrl-C"), _T("Copy Text"));
    menuEdit->Append(wxID_PASTE, _T("&Paste...\tCtrl-V"), _T("Paste Text"));
    
    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, _T("&About...\tCtrl-A"), _T("Show about dialog"));
    
     // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(menuFile, _T("&File"));
    menuBar->Append(menuEdit, _T("&Edit"));
    menuBar->Append(helpMenu, _T("&Help"));
    
   
    SetMenuBar(menuBar);

 }
 
 void BatchWindow::OnQuit(wxCommandEvent&(event))
 {
     event.Skip();
      extern StartWindow *startIt;
	  startIt->doQuit();

 }
 void BatchWindow::OnOpen(wxCommandEvent&(event))
 {
     event.Skip();

 //		VtkFrame *vtl=new VtkFrame();
 		
 //	  	vtl->OpenFile();
 	  	
 //	  	delete vtl;
 }


 void BatchWindow::OnClose(wxCommandEvent&(event))
 {
    event.Skip();
 	Destroy();
 }
 
 void BatchWindow::OnClose2(wxCloseEvent&(event))
 {
 
     event.Skip();
     Destroy();
 	
 }
#if wxUSE_FONTDLG
void BatchWindow::ChooseFont(wxCommandEvent&(event) )
{
    event.Skip();

    wxFontData data;
    data.SetInitialFont(text->GetFont());
   
    wxFontDialog dialog(this, data);
    if (dialog.ShowModal() == wxID_OK)
    {
        wxFontData retData = dialog.GetFontData();
        text->SetFont(retData.GetChosenFont());        
        text->Refresh();
    }
    
}
#endif // USE_FONTDLG_GENERIC
 
void BatchWindow::OnFileSave(wxCommandEvent&(event))
{

    event.Skip();

	static const wxChar* psz=_("BatchPrint.txt");

    static Utilities name=Utilities();

 
    wxFileDialog filedlg(this,
                         _("Save File :"),
                         wxEmptyString,
                         psz,
                         wxEmptyString,
                         wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (filedlg.ShowModal() == wxID_OK)
    {
    	
    	wxString names = filedlg.GetPath();
        
        const char *file=names.ToUTF8().data();
                  
      //	int Index=filedlg.GetFilterIndex();
       
        if(text->SaveFile(file))
        {
        	;
        }
                        	
        wxTopLevelWindow::SetTitle(file);

    
    }
}
 
 void BatchWindow::OnAbout(wxCommandEvent&(event))
{
     event.Skip();

    wxString msg;
    msg.Printf( _T("This is the iqSDR Application.\n"));

    wxMessageBox(msg, _T("About iqSDR"), wxOK | wxICON_INFORMATION, this);
}

int BatchWindow::print=0;

void BatchWindow::winout(char *buff)
{
		
	if(BatchWindow::print){
		WarningBatchHold(buff);
	}else{
		fprintf(stderr,"%s",buff);
	}

}
void winout(const char *fmt, ...)
{
	char buff[4096];
	va_list arg;
	int ret;
	

    buff[0]=0;
    va_start(arg, fmt);
    ret = vsnprintf((char *)buff, sizeof(buff)-1, fmt, arg);
	if(ret){

	}
    va_end(arg);
	
	BatchWindow::winout(buff);
}

void WarningBatch(const char *message)
{

    wxWindow* win = wxWindow::FindWindowByName(wxT("BatchPrint"));
    if(win)
    {
    	BatchWindow *frame= (BatchWindow *)win;
    	frame->text->AppendText(message);
    }    
	else
	{
		BatchWindow *frame= new BatchWindow();
		frame->text = new wxTextCtrl(frame, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_MULTILINE | wxTE_PROCESS_TAB);
		frame->text->SetFont(*wxSMALL_FONT);
		frame->Show();
		frame->text->AppendText(message);
	}
	
	

}
void BatchWarning(ostringstream& outs)
{
    WarningBatch(outs.str().c_str());
    
    outs.str("");
}

int WarningBatchHoldDump(void)
{
	struct dataStructHold *b;
	int ret=1;
	long n;
	
	mutexb.lock();

	b=&BatchHold;
	
	if(!b->message || (b->count <= 0))goto OutOfHere;
	
	
	for(n=0;n<b->count;++n){
		if(b->message[n]){
		    WarningBatch(b->message[n]);
		    cFree((char *)b->message[n]);
		    b->message[n]=NULL;
		}
	}
	
	cFree((char *)b->message);
	
	b->message=NULL;
	b->countMax=0;
	b->count=0;
	
	ret=0;
	
OutOfHere:
	
	mutexb.unlock();

	return ret;
	
}
int WarningBatchHold(char *buff)
{
	struct dataStructHold *b;
	int ret=1;
	
	if(!buff)return 1;
	
	mutexb.lock();

	b=&BatchHold;
	
	if(checkBatchHold(b))goto OutOfHere;
	
	b->message[b->count]=strsave(buff,1974);
	
	if(!b->message[b->count])goto OutOfHere;
	
	++b->count;
	ret=0;
	
OutOfHere:
	
	mutexb.unlock();

	return ret;
}
static int checkBatchHold(struct dataStructHold *plane)
{
    long countMax;
	char **message;
	if(!plane)return 1;
	
	if(plane->count+1 < plane->countMax)return 0;
	
	countMax=plane->countMax+POINT_INCREMENT;
	
	message=NULL;
	
	if(plane->message){
	    message=(char **)cRealloc((char *)plane->message,countMax*sizeof(char **),7761);
	    if(!message){
	        goto ErrorOut;
	    }
	    zerol((char *)&message[plane->countMax],POINT_INCREMENT*sizeof(char **));
	}else{
	    message=(char **)cMalloc(countMax*sizeof(char **),7452);
	    if(!message){
	        goto ErrorOut;
	    }
	    zerol((char *)message,countMax*sizeof(char **));
	}
	
	plane->countMax=countMax;
	plane->message=message;
	
	return 0;
ErrorOut:
    if(message)cFree((char *)message);
	return 1;
	
}	


