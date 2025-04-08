#include "iqSDR.h"
#include "WarningBatch.h"
#include <string.h>
#include <vector>
#include <string>

void winout(const char *fmt, ...);

int copyl(char *p1,char *p2,long n);

std::string ProgramVersion="iqSDR-1355";

void *cMalloc(unsigned long r, int tag);

void checkall(void);

char WarningBuff[256];

GLuint vboId = 0;                   // ID of VBO for vertex arrays


StartWindow *startIt;

soundClass *s;

extern std::vector<Sweep *> sgrabList;


std::vector<ApplFrame *> grabList;

static BasicPane *pBasicPane;
static Spectrum *pSpectrum;
static WaterFall *pWaterFall;
static TopPane *pTopPane;
static ostringstream outs;
extern  BasicPane2 *pBasicPane2;

int sendAudio(sdrClass *sdr,int short *data,int length)
{
	if(sdr->gBasicPane)sdr->gBasicPane->sendAudio(data,length);
	if(sdr->gBasicPane2)sdr->gBasicPane2->sendAudio(data,length);
	return 0;
}


class MyApp: public wxApp
{
    virtual bool OnInit();
    
    //wxFrame *frame;
   // wxFrame *frame2;
   // Spectrum2 *glPane2;
public:
    
};

 
bool MyApp::OnInit()
{

/*
    if ( !wxApp::OnInit() )
        return false;
*/
      
    SoapySDR_setLogLevel(SOAPY_SDR_NOTICE);
        
    s=new soundClass;
  	s->soundRun=0;   
    std::thread(&soundClass::startSound,s).detach(); 
    while(s->soundRun == 0){
    	//winout("s->soundRun  %d\n",s->soundRun);
   		 Sleep2(5);
    }
    
    

   
    // testEM();
        
 	wxFrame *frame2 = new wxFrame(NULL,wxID_ANY,ProgramVersion);
	startIt=new StartWindow(frame2, "Controls");
	frame2->SetSize(wxDefaultCoord,wxDefaultCoord,155,300);
	frame2->Show();
	
	//winout("startIt %p\n",startIt);
	
	startIt->openArgcArgv(argc,argv);
	
    return true;
} 

IMPLEMENT_APP(MyApp)

BEGIN_EVENT_TABLE(StartWaveFile, wxWindow)
EVT_LEFT_DOWN(StartWaveFile::mouseDown)
EVT_SLIDER(SCROLL_TIME2,StartWaveFile::OnScroll)
EVT_SLIDER(SCROLL_GAIN2,StartWaveFile::OnScroll)
EVT_CHECKBOX(ID_PAUSE, StartWaveFile::Pause)
EVT_TIMER(TIMER_ID2,StartWaveFile::OnIdle)
END_EVENT_TABLE()

void StartWaveFile::Pause(wxCommandEvent& event) 
{    
	event.Skip();

   //int flag=event.GetValue();
       
    iPause=pbox->GetValue();

//   winout("Pause iPause %d\n",iPause);
}
void StartWaveFile::OnIdle(wxTimerEvent &event)
{
	if(iPause)return;
	
	if(sliderTime){
		if(s->bS != bS)return;
		if(sfinfo.samplerate > 0){
			sliderTime->SetValue(CurrentFrame/sfinfo.samplerate);
		}else{
			sliderTime->SetValue(0);
		}
	}
	sliderTime->Refresh();
}


void StartWaveFile::OnScroll(wxCommandEvent &event)
{
	event.Skip();
	
	float value=event.GetSelection();

	if(event.GetId() == SCROLL_GAIN2){
		//winout("OnScroll value %g %d \n",value,event.GetId());
		gain=0.01*value;
		s->gain=gain;
		return;
	}else if(event.GetId() == SCROLL_TIME2){
		// winout("OnScroll value %g %d \n",value,event.GetId());
		sf_count_t ret=sf_seek(infile, value*sfinfo.samplerate, SEEK_SET);
        if(ret < 0)winout("sf_seek error\n");
		CurrentFrame=value*sfinfo.samplerate;
		return;
	}
	
	
}

StartWaveFile::StartWaveFile(wxWindow *frame,const wxString& title)
    : wxWindow(frame,32000), m_timer(this,TIMER_ID2)
{
	m_timer.Start(2000);


	CurrentFrame=0;
	
	gain=1.0;
	
	iPause=0;
	
	 wxStaticBox *box = new wxStaticBox(this, wxID_ANY, "&Volume",wxPoint(40,10), wxSize(270, 100),wxBORDER_SUNKEN );
	 //box->SetToolTip(wxT("This is tool tip") );

    wxSlider *sliderGain=new wxSlider(box,SCROLL_GAIN2,100,0,100,wxPoint(20,10),wxSize(240,-1),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);
    sliderGain->SetValue(100);
    
 	box = new wxStaticBox(this, wxID_ANY, "&Time",wxPoint(40,120), wxSize(270, 100),wxBORDER_SUNKEN );
	 //box->SetToolTip(wxT("This is tool tip") );
   
    sliderTime=new wxSlider(box,SCROLL_TIME2,5,0,100,wxPoint(20,10),wxSize(240,-1),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);
    sliderTime->SetValue(0);
    
 	pbox=new wxCheckBox(this,ID_PAUSE, "&Pause",wxPoint(40,230), wxSize(130, 30));
	pbox->SetValue(0);	
   
  	const char *file=title;
  	
   if ((infile = sf_open (file, SFM_READ, &sfinfo))  == NULL)
    {
        fprintf (stderr,"Not able to open input file %s\n", file) ;
        infile=NULL;
        return;
    }
    
	s->soundRun = -1;
	while(s->soundRun == -1){
		//winout("Wait soundRun %d\n",s->soundRun);
		Sleep2(10);
	}
    
	if(sfinfo.samplerate > 0){
		sliderTime->SetMax(sfinfo.frames/sfinfo.samplerate); 
	}else{
		sliderTime->SetMax(100); 
	}
    sliderTime->SetValue(0);

	s->faudio=sfinfo.samplerate;
    
	//winout("sfinfo.samplerate %d sfinfo.channels %d sfinfo.frames %lld\n",sfinfo.samplerate,sfinfo.channels,sfinfo.frames);

	bS=new cStack;

	bS->setBuff(2000000,sfinfo.samplerate);

	nReadBlock=(int)(sfinfo.samplerate/s->ncut);

	s->bS=bS;

	wavFlag=2;

	std::thread(&StartWaveFile::wavBuffer,this).detach();
	
	std::thread(&soundClass::startSound,s).detach();

	
}

int StartWaveFile::wavBuffer()
{
	extern soundClass *s;

	s->audioSync=0;
	long int audioOut=0;
	while(wavFlag == 2){
		while(s->audioSync == 1 && wavFlag == 2)Sleep2(5);
		if(iPause){
			Sleep2(5);
			continue;
		}
		bS->mutexo.lock();
		short *data=bS->buffa[audioOut++ % NUM_ABUFF2];
		bS->mutexo.unlock();
	 	int readcount = (int)sf_readf_short(infile,data,nReadBlock);
	 	if(readcount <= 0){
	 		sf_seek(infile, 0, SEEK_SET);
	 		CurrentFrame=0;
	 		continue;
	 	}
	 	if(sfinfo.channels == 2){
	 		for(int n=0;n<readcount;++n){
	 			data[n]=data[2*n];
	 		}
	 	}
	 	
	 	CurrentFrame += readcount;
	 	 	
	 //	sendAudio(data,readcount);

		bS->pushBuffa(audioOut-1);
	//	winout("audioOut %ld readcount %d\n",audioOut,readcount);
		if(s->bS == bS)s->audioSync=1;

	}

	wavFlag=1;
	
	return 0;
}

void StartWaveFile::mouseDown(wxMouseEvent& event) 
{
	extern soundClass *s;
	event.Skip();
	s->gain=gain;
	s->bS=bS;
}

StartWaveFile::~StartWaveFile()
{
	extern soundClass *s;

	s->bS=NULL;
	
	wavFlag=0;
	
	while(wavFlag == 0)Sleep2(5);

	if(infile)sf_close(infile) ;
	infile=NULL;
	
	if(bS)delete bS;
	bS=NULL;
	
	//winout("exit StartWaveFile %p\n",this);

}

void StartWindow::openArgcArgv(int argc,char **argv)
{
	double fc=101.5;
	double samplerate=2e6;
	FILE *outFile=NULL;
	FILE *inFile=NULL;
	int tcp_ip=0;
	int ipipe=0;
	int debug=0;
	char *name;
	int type= -1;
	int udp=0;
	int pipeout=0;
	int cat=0;

	if(argc < 2)return;

	for(int n=1;n<argc;++n){
		wxString name=argv[n];
		if(name == "-pipe"){
		    ipipe=1;
			winout("pipe input\n");
		}else if(name == "-pipeout"){
			pipeout=1;
		}else if(name == "-float"){
			type=TYPE_FLOAT;
		}else if(name == "-short"){
			type=TYPE_SHORT;
		}else if(name == "-unsignedshort"){
			type=TYPE_UNSIGNEDSHORT;
		}else if(name == "-signed"){
			type=TYPE_SIGNED;
		}else if(name == "-unsigned"){
			type=TYPE_UNSIGNED;
		}else if(name == "-debug"){
			debug=1;
			winout("debug %d\n",debug);
		}else if(name == "-udp"){
			udp=1;
			//winout("tcp_ip %d input\n",tcp_ip);
		}else if(name == "-tcp"){
			tcp_ip=1;
			//winout("tcp_ip %d input\n",tcp_ip);
		}else if(name == "-cat"){
			cat=1;
			//winout("tcp_ip %d input\n",tcp_ip);
		}else if(name == "-fc"){		
			fc=atof(argv[++n]);	
		}else if(name == "-samplerate"){		
			samplerate=atof(argv[++n]);	
	    }else if(name == "-outFile"){
	    	name=argv[++n];
	    	if(name == "-"){
	    		outFile=stdout;
	    		winout("outFile=stdout\n");
	    	}else{
				 outFile=fopen(argv[n],"wb");
				 if(outFile == NULL){
					 winout("Could Not Open %s to Write\n",argv[n]);
				 }
	         }
	    }else if(name == "-inFile"){
	         inFile=fopen(argv[++n],"rb");
	         if(inFile == NULL){
	             winout("Could Not Open %s to Write\n",argv[n]);
	         }
		}
	
	}
	
	fc *= 1e6;
		
	//winout("fc %g MHZ samplerate %g\n",fc/1e6,samplerate);
	
    class sdrClass *sdrIn= new sdrClass;
	
	name=(char *)"Piped Data";
		
	sdrIn->f=fc;
	sdrIn->fc=fc;
	sdrIn->fw=fc;
	sdrIn->samplerate=samplerate;
	sdrIn->samplewidth=samplerate;
	sdrIn->deviceNumber=0;
	sdrIn->decodemode=MODE_FM;
	sdrIn->outFile=outFile;
	sdrIn->Debug=debug;
	sdrIn->data_type=type;
	if(inFile){
		sdrIn->inFile=inFile;
		sdrIn->inData=IN_FILE;
	}else if(udp){
		sdrIn->inData=IN_UDP;			
		sdrIn->l=new Listen(LISTEN_UDP);
		sdrIn->l->Debug=debug;
		sdrIn->l->samplerate=samplerate;
		sdrIn->l->binary=pipeout;
		sdrIn->l->cat=cat;
		sdrIn->l->data_type=type;
		copyl(argv[1],sdrIn->l->name,(long)strlen(argv[1])+1);
		std::thread(&Listen::WaitFor,sdrIn->l).detach();
		name=(char *)"UDP Data";
		if(cat)return;
	}else if(tcp_ip){
		sdrIn->inData=IN_TCPIP;			
		sdrIn->l=new Listen(LISTEN_TCP);
		sdrIn->l->Debug=debug;
		sdrIn->l->samplerate=samplerate;
		sdrIn->l->binary=pipeout;
		sdrIn->l->cat=cat;
		sdrIn->l->data_type=type;
		copyl(argv[1],sdrIn->l->name,(long)strlen(argv[1])+1);
		std::thread(&Listen::WaitFor,sdrIn->l).detach();
		name=(char *)"TCP/IP Data";
		if(cat)return;
	}else if(ipipe){
		sdrIn->inData=IN_PIPE;
		sdrIn->inFilenum=fileno(stdin);
	}

	
/*
	sdrIn->inFile=fopen(file,"rb");
	if(sdrIn->inFile == NULL){
	    winout("Could Not Open %s to Read\n",file);
	    return;
	}
*/


	sdrIn->startPlay();

	ApplFrame *grab=new ApplFrame(NULL,name,sdrIn);
	grab->Show();

	grabList.push_back(grab);

	gBasicPane=pBasicPane;
	

	grab->SetLabel(name);

	gBasicPane->gSpectrum->iWait=1;

	gBasicPane->gSpectrum->Spectrum::startRadio2();   	

	Sleep2(50);

	gBasicPane->gSpectrum->iWait=0;
	
	sdrIn->gBasicPane=gBasicPane;

	
	//winout("samplerate %g samplewidth %g\n",sdrIn->samplerate,sdrIn->samplewidth);
	
}

StartWindow::StartWindow(wxWindow *frame, const wxString &title)
    : wxWindow(frame,32000)
{

	 wxStaticBox *box = new wxStaticBox(this, wxID_ANY, "&Start Options",wxPoint(10,10), wxSize(135, 175),wxBORDER_SUNKEN );
	 box->SetToolTip(wxT("This is tool tip") );

  	new wxButton(box,ID_ABOUT,wxT("About"),wxPoint(20,25));
  	
  	new wxButton(box,ID_RADIO,wxT("Radio"),wxPoint(20,55));
  	
    new wxButton(box,ID_FILE,wxT("File"),wxPoint(20,85));
   	
    new wxButton (box,ID_QUIT,wxT("Quit"),wxPoint(20,115));
      
    wxStaticBox *box2 = new wxStaticBox(this, wxID_ANY, "&Device String",wxPoint(10,190), wxSize(135, 75),wxBORDER_SUNKEN );
	box2->SetToolTip(wxT("Enter Device String") );

    textDevice=new wxTextCtrl(box2,ID_TEXTCTRL,wxT(""),
          wxPoint(5,15), wxSize(120, 30));
                    
    grab=NULL;

   //   wxString value=text->GetValue();

}

StartWindow::~StartWindow()
{
	//winout("exit StartWindow %p\n",this);
}
BEGIN_EVENT_TABLE(StartWindow, wxWindow)
EVT_BUTTON(ID_ABOUT, StartWindow::OnAbout)
EVT_BUTTON(ID_RADIO, StartWindow::OnRadio)
EVT_BUTTON(ID_QUIT, StartWindow::OnQuit)
EVT_BUTTON(ID_FILE, StartWindow::OnFile)
//EVT_BUTTON(ID_TEST, StartWindow::OnTest)
EVT_IDLE(StartWindow::OnIdle)
END_EVENT_TABLE()

void StartWindow::OnIdle(wxIdleEvent& event)
{
	extern int WarningBatchHoldDump(void);
	
	static long long ip=0;
	
	if(++ip > 100)WarningBatchHoldDump();
	
	event.Skip();
}


void StartWindow::openWindows(char *name)
{
	wxString title=name;
		
	grab=new ApplFrame(NULL,name,NULL);
	grab->Show();
	
	grabList.push_back(grab);
	
}
#define POS(r, c)        wxGBPosition(r,c)
#define SPAN(r, c)       wxGBSpan(r,c)


wxBEGIN_EVENT_TABLE(ApplFrame, wxFrame)
EVT_MENU_RANGE(INPUT_MENU,INPUT_MENU+99,ApplFrame::OnInputSelect)
EVT_MENU_RANGE(OUTPUT_MENU,OUTPUT_MENU+99,ApplFrame::OnOuputSelect)
EVT_MENU_RANGE(ID_PALETTE,ID_PALETTE+99,ApplFrame::OnPaletteSelected)
EVT_MENU_RANGE(ID_OPTIONS,ID_OPTIONS+99,ApplFrame::OnOptionsSelected)
EVT_MENU_RANGE(ID_DIRECT,ID_DIRECT+99,ApplFrame::OnDirectSelected)
EVT_MENU_RANGE(ID_BAND,ID_BAND+99,ApplFrame::OnBandSelected)
EVT_MENU_RANGE(ID_SAMPLERATE,ID_SAMPLERATE+99,ApplFrame::OnSampleRateSelected)
EVT_SIZE(ApplFrame::resized)
EVT_MENU(wxID_ABOUT, ApplFrame::About)
EVT_MENU(ID_EXIT, ApplFrame::About)
wxEND_EVENT_TABLE()
void ApplFrame::About(wxCommandEvent &event)
{

	int item=event.GetId();
    if(item == wxID_ABOUT)wxMessageBox(ProgramVersion+"(c) 2025 Dale Ranta");
    if(item == ID_EXIT)startIt->OnQuit(event);
}

void ApplFrame::resized(wxSizeEvent& evt)
{
 	evt.Skip();

	//const wxSize size = evt.GetSize() * GetContentScaleFactor();

	//winout("ApplFrame::resized %d %d %f\n", size.x,size.y,GetContentScaleFactor());
	Refresh();

}


ApplFrame::ApplFrame(wxFrame* parent,wxString title,class sdrClass *sdrIn)
    : wxFrame(parent, wxID_ANY, "wxGridBagSizer Test Frame")
{
	sdr=sdrIn;
	
    wxMenu *menuFile = new wxMenu;
    //menuFile->Append(wxID_OPEN, _T("&Information Audio...\tCtrl-O"), _T("Infomation"));
    
    menuFile->Append(ID_EXIT, _T("Exit"), _T("Quit2 this program"));
    
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, _T("&About...\tCtrl-A"), _T("Show about dialog"));
	
	wxMenu *inputMenu = new wxMenu;
	//winout("s->inputNames.size() %lld\n",(long long)s->inputNames.size());
    for(unsigned long int n=0;n<s->inputNames.size();++n){
        wxMenu *subMenu = new wxMenu;
        inputMenu->AppendSubMenu(subMenu, s->inputNames[n].name, wxT("Description?"));
       // winout("s->inputNames[n].name %s\n",s->inputNames[n].name.c_str());
       //int deviceID=inputNames[n].deviceID;
        for(unsigned long int k=0;k<s->inputNames[n].sampleRate.size();++k){
            std::stringstream srateName;
            srateName << ((float) (s->inputNames[n].sampleRate[k]) / 1000.0f) << "kHz";
            itm = subMenu->AppendRadioItem(INPUT_MENU + 10*n + k, srateName.str(), wxT("Description?"));
        }
    }
   
    wxMenu *outputMenu = new wxMenu;
 	//winout("s->outputNames.size() %lld\n",(long long)s->outputNames.size());
	for(unsigned long int n=0;n<s->outputNames.size();++n){
        wxMenu *subMenu = new wxMenu;
    	outputMenu->AppendSubMenu(subMenu, s->outputNames[n].name, wxT("Description?"));
  //  	winout("s->outputNames[n].name %s\n",s->outputNames[n].name.c_str());
       // int deviceID=s->outputNames[n].deviceID;
        for(unsigned long int k=0;k<s->outputNames[n].sampleRate.size();++k){
            std::stringstream srateName;
            srateName << ((float) (s->outputNames[n].sampleRate[k]) / 1000.0f) << "kHz";
            itm = subMenu->AppendRadioItem(OUTPUT_MENU + 10*n + k, srateName.str(), wxT("Description?"));
        }
  	}
  	
	
	std::vector<std::string> names;
	
	getPaletteNames(names);
	
	//winout("names.size() %ld\n",(long)names.size());
	
	actionMenu = new wxMenu;
	paletteMenu = new wxMenu;
	actionMenu->AppendSubMenu(paletteMenu,"Palette", wxT("Description?"));
	for(unsigned long int n=0;n<names.size();++n){
		paletteMenu->AppendCheckItem(ID_PALETTE+n,names[n], wxT("Description?"));
	}
  	paletteMenu->Check(ID_PALETTE+12,1);
  	
  	
  	if(sdr->device){
  		int directSampleMode=0;
		SoapySDR::ArgInfoList flags;
		flags = sdr->device->getSettingInfo();
//		winout("flags.size() %ld\n",(long)flags.size());
		if(flags.size()){
			int count=0;
			for(size_t k=0;k<flags.size();++k){
			// winout("k %d %s %s type %d\n",(int)k,flags[k].key.c_str(),flags[k].value.c_str(),(int)flags[k].type);
				if(flags[k].key == "direct_samp")directSampleMode=1;
				if(flags[k].type == flags[k].BOOL){
					//wxMenu *subMenu = new wxMenu;
					actionMenu->AppendCheckItem(ID_OPTIONS+count,flags[k].key.c_str(), wxT("Description?"));
				   // flagsmenu[k]=glutCreateMenu(doBiasMode);

					if(sdr->device->readSetting(flags[k].key) == "true"){
						actionMenu->Check(ID_OPTIONS+count,1);
					}else{
						actionMenu->Check(ID_OPTIONS+count,0);
				   }
					++count;
				}
			}
		}
		if(directSampleMode){
			directMenu = new wxMenu;
			actionMenu->AppendSubMenu(directMenu,"directSampleMode", wxT("Description?"));
			directMenu->AppendCheckItem(ID_DIRECT+0,"0", wxT("Description?"));
			directMenu->AppendCheckItem(ID_DIRECT+1,"1", wxT("Description?"));
			directMenu->AppendCheckItem(ID_DIRECT+2,"2", wxT("Description?"));
			directMenu->Check(ID_DIRECT,1);
			directMenu->Check(ID_DIRECT+1,0);
			directMenu->Check(ID_DIRECT+2,0);
		}
		if(sdr->band.size() > 0){
			bandMenu = new wxMenu;
			actionMenu->AppendSubMenu(bandMenu,"Bandwidth", wxT("Description?"));
			for(unsigned long int n=0;n<sdr->band.size();++n){		
				char level[256];
				snprintf(level, sizeof(level), "%.0f",sdr->band[n]);
				bandMenu->AppendCheckItem(ID_BAND+n,level, wxT("Description?"));
				bandMenu->Check(ID_BAND+n,0);
			}
		}
		
		
		if(sdr->rate.size() > 0){
			sampleRateMenu = new wxMenu;
			actionMenu->AppendSubMenu(sampleRateMenu,"SampleRate", wxT("Description?"));
			for(unsigned long n=0;n<sdr->rate.size();++n){
				char level[256];
				snprintf(level, sizeof(level), "%.0f",sdr->rate[n]);
				sampleRateMenu->AppendCheckItem(ID_SAMPLERATE+n,level, wxT("Description?"));
				sampleRateMenu->Check(ID_SAMPLERATE+n,0);
			}
		}
		
		
		
		
		
 	}
 	
 /*

    sdr->rx->device->writeSetting("direct_samp",value);
 */

    wxMenuBar *menuBar = new wxMenuBar();
    
    menuBar->Append(menuFile, _T("&File"));
    
    menuBar->Append(inputMenu, _T("Input Devices"));
    
    menuBar->Append(outputMenu, _T("Output Devices"));
    
    menuBar->Append(actionMenu, _T("Action"));
    
    menuBar->Append(helpMenu, _T("&Help"));
    
    SetMenuBar(menuBar);
    
    
    m_gbs = new wxGridBagSizer();
    
    
    //wxFrame *frame7=new wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxSize(600,200));
    
    startIt->frame7=this;

 	int args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};
 	

//    m_gbs->Add(  new wxTextCtrl(frame7, wxID_ANY, "POS(0,0)"),   POS(0,0) , SPAN(1,3), wxEXPAND  );
    
    m_gbs->Add(new TopPane(this, "TopPane"),   POS(0,0) , SPAN(1,3), wxEXPAND  );
    
    m_gbs->Add(new Spectrum(this, args),   POS(1,1), SPAN(2,2), wxEXPAND  );
    
    m_gbs->Add(new WaterFall(this, args),   POS(3,1) , SPAN(2,2), wxEXPAND  );

    m_gbs->Add(new BasicPane(this, "Controls", sdrIn),   POS(1,0) , SPAN(4,1), wxEXPAND  );
    
    gWaterFall=pWaterFall;
    
    gBasicPane=pBasicPane;
    
    pBasicPane->gApplFrame=this;

    m_gbs->AddGrowableCol(1);
    m_gbs->AddGrowableCol(2);
    m_gbs->AddGrowableRow(1);
    m_gbs->AddGrowableRow(2);
    m_gbs->AddGrowableRow(3);
    m_gbs->AddGrowableRow(4);

    this->SetSizerAndFit(m_gbs);
    SetClientSize(this->GetSize());

    pSpectrum->iWait=1;
 
    //frame7->Show();
}

void ApplFrame::OnSampleRateSelected(wxCommandEvent& event)
{
	event.Skip();

	//int item=event.GetId()-ID_SAMPLERATE;
	
	wxString whatEvent=sampleRateMenu->GetLabelText(event.GetId());
	
	const char *what=whatEvent.c_str();

   	//winout("OnBandSelected %d ID_BAND %d what %s\n",item,ID_BAND,what);
   	
   	for(int n=0;n<(int)sdr->rate.size();++n){
		sampleRateMenu->Check(ID_SAMPLERATE+n,0);
	}
	
	sampleRateMenu->Check(event.GetId(),1);

   	s->bS=NULL;
   	
   	sdr->waitPointer("iqToAudio(8)",&sdr->iqToAudio,0);

	sdr->waitPointer("frame(8)",&sdr->frame,0);

	sdr->waitPointer("doWhat(8)",&sdr->doWhat,0);
	
	sdr->samplerate=atof(what);
	
	sdr->samplewidth=sdr->samplerate;
	
	sdr->startPlay();
	
	std::thread(&sdrClass::run,sdr).detach();

}

void ApplFrame::OnBandSelected(wxCommandEvent& event)
{
	event.Skip();

	//int item=event.GetId()-ID_BAND;
	
	wxString whatEvent=bandMenu->GetLabelText(event.GetId());
	
	const char *what=whatEvent.c_str();

   	//winout("OnBandSelected %d ID_BAND %d what %s\n",item,ID_BAND,what);
   	
   	for(int n=0;n<(int)sdr->band.size();++n){
		bandMenu->Check(ID_BAND+n,0);
	}
	
	bandMenu->Check(event.GetId(),1);

   	s->bS=NULL;
   
   	sdr->waitPointer("iqToAudio(8)",&sdr->iqToAudio,0);

	sdr->waitPointer("frame(8)",&sdr->frame,0);

	sdr->waitPointer("doWhat(8)",&sdr->doWhat,0);
	
	sdr->bandwidth=atof(what);
	
	sdr->startPlay();
	
	std::thread(&sdrClass::run,sdr).detach();



}
void ApplFrame::OnOptionsSelected(wxCommandEvent& event){
	event.Skip();

	//int item=event.GetId()-ID_OPTIONS;
		
	wxString whatEvent=actionMenu->GetLabelText(event.GetId());
	
	const char *what=whatEvent.c_str();

   // winout("OnOptionsSelected %d INPUT_MENU %d checked %d what %s\n",item,ID_OPTIONS,actionMenu->IsChecked(event.GetId()),what);
    
    
    sdr->device->writeSetting(what,actionMenu->IsChecked(event.GetId()));
    
}

void ApplFrame::OnDirectSelected(wxCommandEvent& event){
	event.Skip();

	//int item=event.GetId()-ID_DIRECT;
	
	wxString whatEvent=directMenu->GetLabelText(event.GetId());
	
	const char *what=whatEvent.c_str();

	
  //  winout("OnDirectSelected %d ID_DIRECT %d what %s\n",item,ID_DIRECT,what);
    
    for(int n=0;n<3;++n){
		directMenu->Check(ID_DIRECT+n,0);
	}
	
	directMenu->Check(event.GetId(),1);
	
	sdr->device->writeSetting("direct_samp",what);

}

void ApplFrame::OnPaletteSelected(wxCommandEvent& event){
	event.Skip();

	int item=event.GetId()-ID_PALETTE;
//	winout("OnPaletteSelected %d INPUT_MENU %d\n",item,ID_PALETTE);
	
	char name[512];

	double pal[3*256];

	getPalette(item,name,pal);

	//winout("name %s\n",name);
	
	for(int n=0;n<256;++n){
		gWaterFall->pd.palette[3*n]=pal[3*n]*255;
		gWaterFall->pd.palette[3*n+1]=pal[3*n+1]*255;
		gWaterFall->pd.palette[3*n+2]=pal[3*n+2]*255;
	}
	
	for(int n=0;n<27;++n){
		paletteMenu->Check(ID_PALETTE+n,0);
	}
	
	paletteMenu->Check(event.GetId(),1);

	Refresh();
}

void ApplFrame::OnInputSelect(wxCommandEvent& event){
	int item=event.GetId()-INPUT_MENU;
	//winout("OnInputSelect %d INPUT_MENU %d\n",item,INPUT_MENU);
	int n=item/10;
	s->inputID=s->inputNames[n].deviceID;
	int sound=item-10*n;
	//winout("%s sampleRate %d\n",s->inputNames[n].name.c_str(),s->inputNames[n].sampleRate[sound]);
	SampleFrequency=s->inputNames[n].sampleRate[sound];
	//startAudio();
}

void ApplFrame::OnOuputSelect(wxCommandEvent& event){
	int item=event.GetId()-OUTPUT_MENU;
	//winout("OnOuputSelect %d OUTPUT_MENU %d\n",item,OUTPUT_MENU);
	int n=item/10;
	s->outputID=s->outputNames[n].deviceID;
	int sound=item-10*n;
	//winout("%s sampleRate %d\n",s->outputNames[n].name.c_str(),s->outputNames[n].sampleRate[sound]);
	s->faudio=s->outputNames[n].sampleRate[sound];
	s->soundRun = -1;
	while(s->soundRun == -1){
		//winout("Wait soundRun %d\n",s->soundRun);
		Sleep2(10);
	}
	std::thread(&soundClass::startSound,s).detach();
	
	s->bS=NULL;
	
   	sdr->waitPointer("iqToAudio(8)",&sdr->iqToAudio,0);

	sdr->waitPointer("frame(8)",&sdr->frame,0);

	sdr->waitPointer("doWhat(8)",&sdr->doWhat,0);
		
	sdr->faudio=s->faudio;
	
	sdr->startPlay();
			
	std::thread(&sdrClass::run,sdr).detach();

}

ApplFrame::~ApplFrame()
{
//	winout("ApplFrame::~ApplFrame\n");
	
	if(grabList.size() > 0){
		for(std::vector<ApplFrame *>::size_type k=0;k<grabList.size();++k){
			ApplFrame *grab=grabList[k];
			if(grab == this){
			    grabList[k]=NULL;
//			    winout("ApplFrame remove from list\n");
			}
		}
	}

	
	//winout("exit ApplFrame %p\n",this);

}

void StartWindow::OnAbout(wxCommandEvent& event)
{
	event.Skip();
	
	wxMessageBox(ProgramVersion+"(c) 2025 Dale Ranta");

}
void StartWindow::OnRadio(wxCommandEvent& event)
{	
	event.Skip();

	wxFrame *frame= new wxFrame(NULL, wxID_ANY, wxT("Device Select"), wxDefaultPosition, wxSize(400,500));
		
	new SelectionWindow(frame, "Device",textDevice);
	
	frame->Show();
		
}
void StartWindow::OnFile(wxCommandEvent& event)
{
	event.Skip();

	OpenFile();
}
void StartWindow::OnQuit(wxCommandEvent& event)
{
	doQuit();
}
void StartWindow::doQuit()
{

	if(grabList.size() > 0){
		for(std::vector<ApplFrame *>::size_type k=0;k<grabList.size();++k){
			grab=grabList[k];
			if(grab)delete grab;
		}
	}
	
	checkall();

	wxWindow *parent=GetParent();
	
	parent->Destroy();
}
void StartWindow::openWavFile(const char *file)
{
	
 	wxFrame *frame2 = new wxFrame(NULL,wxID_ANY,file);
	new StartWaveFile(frame2,file);
	frame2->SetSize(wxDefaultCoord,wxDefaultCoord,340,300);
	frame2->Show();
	
	
}
void StartWindow::openIQFile(const char *file)
{
	char name[512];
	
	mstrncpy(name,(char *)file,sizeof(name));
	
	//winout("openIQFile %s name %s\n",file,name);
	
	double samplerate=0;
	double fc=0;
	
	
    char *end=strrchr(name,'_');
    if(end){
        samplerate=0;
        end--;
        unsigned long num=1;
        while(isdigit(*end)){
            samplerate += num*(*end-'0');
            num *= 10;
            --end;
        }
        fc=0;
        end--;
        num=1;
        while(isdigit(*end)){
            fc += num*(*end-'0');
            num *= 10;
            --end;
        }
        
    }else{
        samplerate=200000;
    	fc=90000000;
    }
    
    class sdrClass *sdrIn= new sdrClass;
	
	sdrIn->f=fc;
	sdrIn->fc=fc;
	sdrIn->fw=fc;
	sdrIn->samplerate=samplerate;
	sdrIn->samplewidth=samplerate;
	sdrIn->deviceNumber=0;
	sdrIn->decodemode=MODE_FM;
	sdrIn->inData=IN_FILE;
	sdrIn->inFile=fopen(file,"rb");
	if(sdrIn->inFile == NULL){
	    winout("Could Not Open %s to Read\n",file);
	    return;
	}
	sdrIn->startPlay();
	
	ApplFrame *grab=new ApplFrame(NULL,file,sdrIn);
	grab->Show();
	
	grabList.push_back(grab);
	
	gBasicPane=pBasicPane;
	
	grab->SetLabel(file);

     gBasicPane->gSpectrum->iWait=1;
	
     gBasicPane->gSpectrum->Spectrum::startRadio2();   	
     
     Sleep2(50);
     
     gBasicPane->gSpectrum->iWait=0;
     
     sdrIn->gBasicPane=gBasicPane;


}

void StartWindow::OpenFile()
{

	static int index=0;
    
    wxFileDialog filedlg(this, _("Open File"), "", "",
                       "I/Q files (*.raw)|*.raw|"
                       "Wav files (*.wav)|*.wav|"
                       "TexT files (*.txt)|*.txt" 
                       , wxFD_OPEN|wxFD_FILE_MUST_EXIST);
                         
    filedlg.SetFilterIndex(index);

    if (filedlg.ShowModal() == wxID_OK)
    {
    
        
        wxString name = filedlg.GetPath();
        
        const char *file=name.ToUTF8().data();
        
        int Index=filedlg.GetFilterIndex();
        
        winout("OpenFile file %s Index %d\n",file,Index);
        
        if(Index == 0)
        {     
    		openIQFile(file);
    	} 
    	else if(Index == 1)
    	{
    		openWavFile(file);
    	}
    	else if(Index == 2)
    	{
    		//getSlices((const char *)psz);
    	}
    	else if(Index == 3)
    	{
    		//getText((const char *)psz);
    	}
    	else if(Index == 4)
    	{
    		//VtkFrame::GetNCSASlices((const char *)psz);
    	}
    	else if(Index == 5)
    	{
    		//VtkFrame::GetVtkSlices((const char *)psz);
    	}
    	
    }
}


SelectionWindow::SelectionWindow(wxWindow *frame, const wxString &title,wxTextCtrl *textDevice)
    : wxWindow(frame,32000)
{

	gBasicPane=NULL;
			
	wxString s=textDevice->GetValue();
	
	const char *textDevicePointer=s;
	
	if(resultsEnumerate.size() < 1){
		if(doEnumerate((char *)textDevicePointer))return;
	}
	
	size_t length;
	
	std::vector<SoapySDR::Kwargs> results=resultsEnumerate;

    length=results.size();
    
	if(length == 0){
		winout("Error: enumerate Found No Devices - Try Again !\n");
		return;
	}
	
	pSweep=new wxCheckBox(this,ID_SWEEP, "&Frequency Sweep",wxPoint(20,5), wxSize(230, 25));
	wxStaticBox *box2 = new wxStaticBox(this, wxID_ANY, "&Parameters",wxPoint(10,35), wxSize(300, 100),wxBORDER_SUNKEN );
	box2->SetToolTip(wxT("Device Setup Parameters") );
	new wxStaticText(box2,wxID_STATIC,wxT("Frequency(MHZ)"),wxPoint(20,15), wxDefaultSize,wxALIGN_LEFT);
	text=new wxTextCtrl(box2,ID_TEXTCTRL,wxT("101.5"),wxPoint(130,10), wxSize(80, 25));
	new wxStaticText(box2,wxID_STATIC,wxT("Mode :"),wxPoint(20,43), wxDefaultSize,wxALIGN_LEFT);
	
	wxArrayString strings;
	
	strings.Add("AM");
	strings.Add("NAM");
	strings.Add("FM");
	strings.Add("NBFM");
	strings.Add("USB");
	strings.Add("LSB");
	strings.Add("CW");
	
	boxMode=new wxComboBox(box2,ID_COMBOMODE,wxT("Mode"),wxPoint(130,40),wxDefaultSize,
	                   strings,wxCB_DROPDOWN);
	boxMode->SetSelection(2);
	wxStaticBox *box = new wxStaticBox(this, wxID_ANY, "&Device",wxPoint(10,120), wxSize(300, 60+2*length*45),wxBORDER_SUNKEN );
	box->SetToolTip(wxT("Select The Device") );

    int outset1=15;

    SoapySDR::Kwargs deviceArgs;
    
    deviceNames.clear();
        
     for(size_t k=0;k<length;++k){
        std::string name;
        
        name="";
        
 
        try {

            deviceArgs = results[k];
            for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
               // winout("%s=%s\n",it->first.c_str(),it->second.c_str());
                if (it->first == "driver") {
                    if(it->second == "audio")break;
                    if(it->second == "redpitaya"){
                        name=it->second;
                    }
                } else if (it->first == "device") {
                    if(it->second == "HackRF One")continue;
                    name=it->second;
                } else if (it->first == "label") {
                    name=it->second;
                }
            }
	  		deviceNames.push_back(name);
            if(name != ""){
                
				SoapySDR::Device *devicer = SoapySDR::Device::make(deviceArgs);
	  			
	  			
				new wxButton(box,ID_DEVICE+k,name,wxPoint(20,5+outset1));
				outset1 += 40;
				strings.Clear();

                std::vector<double> rate=devicer->listSampleRates(SOAPY_SDR_RX,0);
                for (size_t j = 0; j < rate.size(); j++)
                {
                    char data[256];
                    unsigned long long irate=rate[j];
                    sprintf(data,"%llu",irate);
                    strings.Add(data);

                }
                
               
				new wxStaticText(box,wxID_STATIC,wxT("Sample Rate :"),wxPoint(20,outset1+15), wxDefaultSize,wxALIGN_LEFT);

				boxList[k]=new wxComboBox(box,ID_FREQUENCY+k,wxT("Mode"),wxPoint(110,outset1+11),wxDefaultSize,
				   strings,wxCB_DROPDOWN);
				boxList[k]->SetSelection(0);

				outset1 += 45;

				SoapySDR::Device::unmake(devicer); 
            }
            
        } catch(const std::exception &e) {
            std::string streamExceptionStr = e.what();
            winout("doRadioOpen Error: %s\n",streamExceptionStr.c_str());
        }
   }
 
}
SelectionWindow::~SelectionWindow()
{
	//delete m_context;
	//winout("SelectionWindow::~SelectionWindow\n");
	//winout("exit SelectionWindow %p\n",this);

}

BEGIN_EVENT_TABLE(SelectionWindow, wxWindow)
EVT_COMMAND_RANGE(ID_DEVICE,ID_DEVICE+199,wxEVT_BUTTON,SelectionWindow::OnDevice)
END_EVENT_TABLE()

void SelectionWindow::killMe() 
{
		
	wxWindow *parent=GetParent();
	if(parent)parent->Destroy();
	
}
void SelectionWindow::OnDevice(wxCommandEvent& event) 
{

	event.Skip();
    
	int id=event.GetId();
	
	wxString frequency=boxList[id-ID_DEVICE]->GetValue();
	
	const char *modeFrequency=frequency;

	wxString value=text->GetValue();
	
	const char *modeFC=value;
	
	double fc=atof(modeFC);
	fc *= 1e6;

	class sdrClass *sdrIn= new sdrClass;
	
	sdrIn->f=fc;
	sdrIn->fc=fc;
	sdrIn->fw=fc;
	sdrIn->samplerate=atof(modeFrequency);
	sdrIn->samplewidth=sdrIn->samplerate;
	sdrIn->deviceNumber=id-ID_DEVICE;
	sdrIn->decodemode=boxMode->GetSelection();
	sdrIn->startPlay();
	
	winout("OnDevice sdrIn->samplerate %g Sweep %d\n",sdrIn->samplerate,pSweep->GetValue());
	
	wxString name=deviceNames[id-ID_DEVICE];
	
	if(pSweep->GetValue()){
		extern BasicPane2 *pBasicPane2;
	
		Sweep *sgrab=new Sweep(NULL,name,sdrIn);
		sgrab->Show();
		sgrabList.push_back(sgrab);
	
		sgrab->SetLabel(deviceNames[id-ID_DEVICE]);
				
		pBasicPane2->gSpectrum2->Spectrum2::startRadio2();
	
		Sleep2(50);

		pBasicPane2->gSpectrum2->iWait=0;
		pBasicPane2->iRefresh=1;
		
		sdrIn->gBasicPane2=pBasicPane2;

	
	}else{
	
		ApplFrame *grab=new ApplFrame(NULL,name,sdrIn);
		grab->Show();
	
		grabList.push_back(grab);
	
		gBasicPane=pBasicPane;
	
		grab->SetLabel(deviceNames[id-ID_DEVICE]);
			
		gBasicPane->gSpectrum->Spectrum::startRadio2();
	
		Sleep2(50);

		gBasicPane->gSpectrum->iWait=0;
		gBasicPane->iRefresh=1;
		
		sdrIn->gBasicPane=pBasicPane;

	}

	killMe();

//	winout("OnDevice  11\n");
   
    
}

BEGIN_EVENT_TABLE(TopPane, wxWindow)
EVT_MOTION(TopPane::mouseMoved)
EVT_PAINT(TopPane::render)
EVT_LEFT_DOWN(TopPane::mouseDown)
EVT_CHECKBOX(ID_RECORD, TopPane::Record)
EVT_CHECKBOX(ID_FC, TopPane::Record)
EVT_CHAR(TopPane::OnChar)    
END_EVENT_TABLE()

void TopPane::OnChar(wxKeyEvent& event) 
{
	event.Skip();
	
	int keycode=event.GetKeyCode();
	
	//winout("TopPane::OnChar %d\n",keycode);
	
    if(keycode == 'f'){
        gSpectrum->iFreeze = !gSpectrum->iFreeze;
     //   winout("iWait %d\n",iWait);
    }
	

    if(keycode == 'n'){
        sdr->iWait = !sdr->iWait;
    //    winout("sdr->iWait %d\n",sdr->iWait);
    }
    
    if(keycode == 'm' || keycode == 'M' ){
		if(s->bS){
		   s->bS=NULL;
		}else{
		   s->bS=sdr->bS;
		}
    }
}

void TopPane::Record(wxCommandEvent& event) 
{    
	event.Skip();
	
	if(event.GetId() == ID_FC){
		idoFC=rbox2->GetValue();
		//winout("ID_FC idoFC %d\n",idoFC);
		Refresh();
		return;
	}

   int flag=rbox->GetValue();
   
   if(flag == 1){
   
   		char fname[256];
   		
		struct tm today;

   		time_t now;

    	time(&now);  /* get current time; same as: now = time(NULL)  */
    
    	today = *localtime(&now);
    	
    	sprintf(fname,"iqSDR_%4d%02d%02d_%02d%02d%02d.wav%c\n",
    					today.tm_year+1900,today.tm_mon+1,today.tm_mday,
    					today.tm_hour,today.tm_min,today.tm_sec,'\0');
   
   		s->startRecord(fname);
   }else{
   		s->startRecord(NULL);
   }
     
}

TopPane::TopPane(wxWindow *frame, const wxString &title)
    : wxWindow(frame,32000)
{
	
	//font= new wxFont(35,wxFONTFAMILY_MODERN,wxNORMAL,wxNORMAL);

	font = new wxFont(35, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	
	fontSize = font->GetPixelSize();
	if(fontSize.x <= 0)fontSize.x=28;
		
	//outs << "fontSize : "  << " x "  << " " << fontSize.x  << " " << " y " << fontSize.y  << endl ;
	//BatchWarning(outs);

	nchar = -1;
	
	idoFC=0;
	
	pTopPane=this;
		
	rbox=new wxCheckBox(this,ID_RECORD, "&record", wxPoint(20,0), wxSize(80, 40));
	rbox->SetValue(0);	

	rbox2=new wxCheckBox(this, ID_FC, "&fc", wxPoint(90,0), wxSize(60, 40));
	rbox2->SetValue(0);	

	wxWindow::SetSize(wxDefaultCoord,wxDefaultCoord,400,40);
}
TopPane::~TopPane()
{
	//winout("TopPane::~TopPane\n");
	//winout("exit TopPane %p\n",this);

}


void TopPane::mouseDown(wxMouseEvent& event)
{
	event.Skip();

	wxPoint p = event.GetLogicalPosition(wxClientDC(this));
	int yh=fontSize.GetHeight()/2-10;
	int diff=p.x-pt.x;
	nchar=(diff)/fontSize.x;
	if(diff >= 0 && nchar >=0 && nchar <= 14){
		if(nchar == 3 || nchar == 7 || nchar == 11)return;
		int up=1;
		if(p.y > yh)up=0;
		//winout("p.y %d up %d yh %d\n",p.y,up,yh);
		
		long long fl=(long long)sdr->f;
		if(idoFC) fl=(long long)sdr->fc;
		
		int k=nchar;
		if(nchar >= 3)k -= 1;
		if(nchar >= 7)k -= 1;
		if(nchar >= 11)k -= 1;
		double value=pow(10.0,11-k);
		fl=(long long)fl/value;
		fl=(long long)fl*value;
		if(up == 1){
			fl += value;
		}else{
			fl -= value;
		}
		
		if(idoFC){
			sdr->setFrequencyFC((double)fl);
		}else{
			sdr->setFrequency((double)fl);
		}
		
//		winout("TopPane::mouseDown f %lld k %d nchar %d up %d %f\n",fl,k,nchar,up,value);
		Refresh();

	}
}


void TopPane::mouseMoved(wxMouseEvent& event)
{
	event.Skip();

	wxPoint p = event.GetLogicalPosition(wxClientDC(this));
	
	nchar=(p.x-pt.x)/fontSize.x;

	//winout("TopPane::mouseMoved diff %d character %d\n",p.x-pt.x,nchar);
	
	Refresh();
	
}

#include <wx/graphics.h>

void TopPane::render( wxPaintEvent& evt )
{
	evt.Skip();
	
	wxPaintDC dc(this);
	
	dc.SetFont(*font);
	dc.SetBackgroundMode(wxTRANSPARENT);
	dc.SetTextForeground(*wxBLACK);
	dc.SetTextBackground(*wxWHITE);
	
	//winout("TopPane f %g \n",sdr->f);

	if(sdr){
		char freq[50];
		double   f=sdr->f;
		if(idoFC)f=sdr->fc;
		int i1,i2,i3,i4;
		i1=(int)(f/1e9);
		i2=(int)((f-i1*1e9)/1e6);
		i3=(int)((f-i1*1e9-i2*1e6)/1e3);
		i4=(int)(f-i1*1e9-i2*1e6-i3*1e3);
		snprintf(freq, sizeof(freq), "%03d.%03d.%03d.%03d",i1,i2,i3,i4);
		dc.DrawText(freq,pt);	
	}else{
		dc.DrawText("No Freq",pt);
	}
	
	if(nchar >= 0 && nchar <= 14){

		wxGraphicsContext* gc = wxGraphicsContext::Create(dc);

		if (gc)
		{
			gc->SetPen(*wxGREEN);
			wxGraphicsPath path = gc->CreatePath();
			path.AddRectangle((int)(pt.x + nchar * fontSize.x), 0, fontSize.x, fontSize.y-8);
			gc->StrokePath(path);
			delete gc;
		}
	}

	
//	winout("TopPane:render\n");
}

BEGIN_EVENT_TABLE(BasicPane, wxWindow)
EVT_RADIOBOX(ID_DATATYPE,BasicPane::dataType)
EVT_RADIOBOX(ID_DATAMODE,BasicPane::dataMode)
//EVT_MOTION(Spectrum2::mouseMoved)
EVT_PAINT(BasicPane::render)
EVT_TIMER(TIMER_ID,BasicPane::OnTimer)
EVT_LEFT_DOWN(BasicPane::mouseDown)
EVT_BUTTON(ID_STARTRADIO, BasicPane::startRadio)
EVT_RADIOBOX(ID_RADIOBOX, BasicPane::radioBox)
EVT_COMBOBOX(ID_COMBOBOX,BasicPane::OnCombo)
EVT_COMBOBOX(ID_COMBOMODE,BasicPane::OnCombo)
EVT_COMBOBOX(ID_COMBOFILTER,BasicPane::OnComboFilter)
EVT_COMBOBOX(ID_COMBOANTENNA,BasicPane::OnComboAntenna)
EVT_COMBOBOX(ID_COMBOSAMPLERATE,BasicPane::OnComboSampleRate)
EVT_COMBOBOX(ID_COMBOBANDWIDTH,BasicPane::setBandwidth)
//EVT_TEXT(ID_COMBOSAMPLERATE, BasicPane::OnText)
EVT_TEXT_ENTER(ID_COMBOSAMPLERATE, BasicPane::OnText)
EVT_TEXT_ENTER(ID_COMBOBANDWIDTH, BasicPane::OnTextBandWidth)
EVT_BUTTON(ID_COMBOBUTTON, BasicPane::setSampleRate)
EVT_SIZE(BasicPane::resized)
EVT_SLIDER(SCROLL_GAIN,BasicPane::OnScroll)
EVT_SLIDER(ID_SAMPLEWIDTH,BasicPane::setSampleWidth)
EVT_SLIDER(ID_ROTATEDATA,BasicPane::setDataRotate)
EVT_SLIDER(ID_MININUM,BasicPane::OnMinimun)
EVT_SLIDER(ID_MAXINUM,BasicPane::OnMaximun)
EVT_COMMAND_RANGE(ID_RXGAIN,ID_RXGAIN+99,wxEVT_SLIDER,BasicPane::setRxGain)
EVT_CHECKBOX(ID_CHECKAUTO,BasicPane::OnCheckAuto)
EVT_CHECKBOX(ID_SWAPIQ,BasicPane::OnCheckAuto)
EVT_CHECKBOX(ID_OSCILLOSCOPE,BasicPane::OnCheckAuto)
EVT_CHECKBOX(ID_SOFTAUTOGAIN,BasicPane::OnCheckAuto)
EVT_CHECKBOX(ID_SETGAIN,BasicPane::OnCheckAuto)
EVT_BUTTON(ID_STARTSEND, BasicPane::startSend)
EVT_BUTTON(ID_STOPSEND, BasicPane::stopSend)
EVT_BUTTON(ID_ALPHA, BasicPane::stopSend)
EVT_DATAVIEW_ITEM_ACTIVATED(ID_VIEWSELECTED, BasicPane::OnViewSelected)
EVT_DATAVIEW_SELECTION_CHANGED(ID_VIEWSELECTED, BasicPane::OnViewSelected)
END_EVENT_TABLE()
void dummpy11(){;}
void BasicPane::OnViewSelected(wxDataViewEvent& event) 
{    
	event.Skip();
	
	long int nRow=(long)listctrlFreq->GetSelectedRow();
	
	//winout("nRow %ld\n",nRow);
	
	wxString nn=listctrlFreq->GetTextValue(nRow,1);

	//winout("%s\n",static_cast<const char*>(nn));
		
	const char *freq=nn;
		
	sdr->setFrequencyFC(atof(freq)*1e6);
	
	gTopPane->Refresh();

}
void BasicPane::startSend(wxCommandEvent& event) 
{    
	event.Skip();
	
	
	wxString value=sendAddress->GetValue();
	
	const char *Address=value;
	
	if(sendFlag){
		winout("Send Already Running\n");
		return;
	}else{	
	    sendFlag=1;
		sdr->fillBuffer=1;
//		winout("startSend Type %d Mode %d Address %s\n",sendTypeBox->GetSelection(),sendModeBox->GetSelection(),Address);	
		SendStart((char *)Address,sendTypeBox->GetSelection(),sendModeBox->GetSelection());
	}

   // int id=event.GetId();
}
void BasicPane::stopSend(wxCommandEvent& event) 
{    
	event.Skip();
	
	if(event.GetId() == ID_ALPHA){
	
		wxString value=textAlpha->GetValue();
	
		const char *alpha=value;

		lineAlpha=atof(alpha);
		
		if(lineAlpha < 0.0)lineAlpha=0.0;
		
		if(lineAlpha > 1.0)lineAlpha=1.0;
		
		gSpectrum->lineAlpha=lineAlpha;
		
		wxString value1=rangeMin->GetValue();
	
		const char *alpha1=value1;

		gWaterFall->pd.sPmin=atof(alpha1);
	
		wxString value2=rangeMax->GetValue();
	
		const char *alpha2=value2;

		gWaterFall->pd.sPmax=atof(alpha2);
		
		//winout("UsePlotScales %d sPmin %g sPmax %g\n",gWaterFall->pd.UsePlotScales,gWaterFall->pd.sPmin,gWaterFall->pd.sPmax);
			
		return;
	}
	
	sendFlag=0;
	
	sdr->fillBuffer=0;
	
	//winout("stopSend \n");

   // int id=event.GetId();
}

int BasicPane::SetScrolledWindow()
{

	if(ScrolledWindow)ScrolledWindow->Destroy();
	
	ScrolledWindow = new wxScrolledWindow(gBasicPane,ID_SCROLLED,wxPoint(0,0),
	 	                                   wxSize(340,200),wxVSCROLL ); // wxHSCROLL
 	int pixX=10;
 	int pixY=10;
 	int ux=200;
 	int uy=200;
 	
 	ScrolledWindow->SetScrollbars(pixX,pixY,ux,uy);
 	
	int yloc=0;

	wxStaticBox *box = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Set Volume ",wxPoint(20,yloc), wxSize(230, 80),wxBORDER_SUNKEN );
	box->SetToolTip(wxT("Set Volume") );

	new wxSlider(box,SCROLL_GAIN,100,0,100,wxPoint(10,15),wxSize(210,30),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);

	yloc += 85;   
	
	wxCheckBox *cbox;
	if(sdr->hasGainMode){
		cbox=new wxCheckBox(ScrolledWindow,ID_CHECKAUTO, "&Hardware AGC",wxPoint(20,yloc), wxSize(230, 25));
		cbox->SetValue(1);	
		yloc += 25;   
	}
	
	cbox=new wxCheckBox(ScrolledWindow,ID_SWAPIQ, "&I/Q Swap",wxPoint(20,yloc), wxSize(100, 25));
	cbox->SetValue(0);	
	cbox=new wxCheckBox(ScrolledWindow,ID_OSCILLOSCOPE, "&Oscilloscope",wxPoint(120,yloc), wxSize(120, 25));
	cbox->SetValue(scrolledWindowFlag);	
	yloc += 25;   

	cbox=new wxCheckBox(ScrolledWindow,ID_SOFTAUTOGAIN, "&Software AGC",wxPoint(20,yloc), wxSize(230, 25));
	cbox->SetValue(1);	
	yloc += 25;   
	
	if(sdr->rxGainRangeList.size()){
		for(size_t n=0;n<sdr->rxGainRangeList.size();++n){
			box = new wxStaticBox(ScrolledWindow, wxID_ANY,sdr->rxGainNames[n],wxPoint(20,yloc), wxSize(230, 80),wxBORDER_SUNKEN );
			//box->SetToolTip(wxT("This is tool tip") );
			
			SoapySDR::Range rxGainRange=sdr->rxGainRangeList[n];
			
			double rmid=0.5*(rxGainRange.maximum()+rxGainRange.minimum());

			new wxSlider(box,ID_RXGAIN+n,rmid,rxGainRange.minimum(),rxGainRange.maximum(),wxPoint(10,15),wxSize(210,-1),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);

			yloc += 85;   
		}
	
	}
	

	box = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Zoom",wxPoint(20,yloc), wxSize(230, 80),wxBORDER_SUNKEN );
	box->SetToolTip(wxT("Zoom Data") );
	new wxSlider(box,ID_SAMPLEWIDTH,200,0,200,wxPoint(10,15),wxSize(210,-1),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);

	yloc += 85;   
	
	
	box = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Rotate Oscilloscope Data",wxPoint(20,yloc), wxSize(230, 80),wxBORDER_SUNKEN );
	box->SetToolTip(wxT("Rotate Oscilloscope Data") );
	new wxSlider(box,ID_ROTATEDATA,200,0,200,wxPoint(10,15),wxSize(210,-1),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);

	yloc += 85;   
	
	
	
	box = new wxStaticBox(ScrolledWindow, wxID_ANY, "Minimum Value ",wxPoint(20,yloc), wxSize(230, 80),wxBORDER_SUNKEN );
	box->SetToolTip(wxT("This is tool tip") );

	new wxSlider(box,ID_MININUM,-130,-200,100,wxPoint(10,15),wxSize(210,-1),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);

	yloc += 85;

	box = new wxStaticBox(ScrolledWindow, wxID_ANY, "Maximum Value",wxPoint(20,yloc), wxSize(230, 80),wxBORDER_SUNKEN );

	new wxSlider(box,ID_MAXINUM,-30,-200,100,wxPoint(10,15),wxSize(210,-1),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);
  	
	yloc += 85;

	wxArrayString strings;
	
	wxPanel *panel22=NULL;
		
    if(sdr->rate.size() > 0){
    	int yloc2;
      	if(sdr->band.size() > 0){
			yloc2=45;
  		}else{
  			yloc2=0;
  		}
    
    	//box = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Sample Rate ",wxPoint(20,yloc), wxSize(230, 100),wxBORDER_SUNKEN );

    	panel22 = new wxPanel(ScrolledWindow,wxID_ANY, wxPoint(20,yloc), wxSize(230, 45+yloc2),wxBORDER_SUNKEN | wxFULL_REPAINT_ON_RESIZE,wxT("Control2"));
		strings.Clear();
		for(size_t n=0;n<sdr->rate.size();++n){
			char level[256];
			snprintf(level, sizeof(level), "%.0f",sdr->rate[n]);
			strings.Add(level);
		}
				
		new wxStaticText(panel22,wxID_STATIC,wxT("Sample Rate:"),wxPoint(10,12), wxDefaultSize,wxALIGN_LEFT);

		sampleRateCombo=new wxComboBox(panel22,ID_COMBOSAMPLERATE,wxT("SAMPLES"),wxPoint(95,10),wxDefaultSize,
	                   strings,wxCB_DROPDOWN | wxTE_PROCESS_ENTER);
	                   
		sampleRateCombo->SetSelection(0);
		
		yloc += 45;

    }
    
    
    
    if(sdr->band.size() > 0){
    	int yloc2;
      	if(!panel22){
      		panel22 = new wxPanel(ScrolledWindow,wxID_ANY, wxPoint(20,yloc), wxSize(230, 45),wxBORDER_SUNKEN | wxFULL_REPAINT_ON_RESIZE,wxT("Control2"));
			yloc2=0;
  		}else{
  			yloc2=45;
  		}
    	new wxStaticText(panel22,wxID_STATIC,wxT("Bandwidth:"),wxPoint(10,12+yloc2), wxDefaultSize,wxALIGN_LEFT);
		
		strings.Clear();
		for(size_t n=0;n<sdr->band.size();++n){
			char level[256];
			snprintf(level, sizeof(level), "%.0f",sdr->band[n]);
			strings.Add(level);
		}
		
		bandwidthCombo=new wxComboBox(panel22,ID_COMBOBANDWIDTH,wxT("BANDWIDTH"),wxPoint(95,10+yloc2),wxDefaultSize,
	                   strings,wxCB_DROPDOWN | wxTE_PROCESS_ENTER);
	                   
	  //  new wxButton (panel22,ID_COMBOBUTTON,wxT("Set Frequency"),wxPoint(90,40));

	
		bandwidthCombo->SetSelection(0);
		
		yloc += 45;

	}
	
 	wxPanel *panel1 = new wxPanel(ScrolledWindow,wxID_ANY, wxPoint(20,yloc), wxSize(230, 200),wxBORDER_SUNKEN | wxFULL_REPAINT_ON_RESIZE,wxT("Control2"));
	
	     wxString computers1[] =
      { "AM", "NAM","FM","NBFM","USB","LSB","CW" };
	
	
	modeBox=new wxRadioBox(panel1, ID_RADIOBOX,
		"Choose Mode", wxPoint(20,10), wxDefaultSize,
		 7, computers1, 0, wxRA_SPECIFY_ROWS);
		 
	modeBox->SetSelection(sdr->decodemode);

	yloc += 205;
	
  	
 	wxPanel *panel2 = new wxPanel(ScrolledWindow,wxID_ANY, wxPoint(20,yloc), wxSize(230, 100),wxBORDER_SUNKEN | wxFULL_REPAINT_ON_RESIZE,wxT("Control2"));
	panel2->SetToolTip(wxT("This is tool tip2") );
    
	strings.Clear();
	
	strings.Add("4096");
	strings.Add("8192");
	strings.Add("16384");
	strings.Add("32768");
	strings.Add("65536");
	strings.Add("131072");
/*
	strings.Add("262144");
	strings.Add("524288");
	strings.Add("1048576");
*/
	new wxStaticText(panel2,wxID_STATIC,wxT("FFT size:"),wxPoint(10,12), wxDefaultSize,wxALIGN_LEFT);

	fftCombo=new wxComboBox(panel2,ID_COMBOBOX,wxT("FFT SIZE"),wxPoint(65,10),wxDefaultSize,
	                   strings,wxCB_DROPDOWN);
	fftCombo->SetSelection(1);
	

	
	strings.Clear();
	strings.Add("RECTANGULAR");
	strings.Add("HANN");
	strings.Add("HAMMING");
	strings.Add("FLATTOP");
	strings.Add("BLACKMANHARRIS");
	strings.Add("BLACKMANHARRIS7");
	
	new wxStaticText(panel2,wxID_STATIC,wxT("Window:"),wxPoint(10,42), wxDefaultSize,wxALIGN_LEFT);

	filterCombo=new wxComboBox(panel2,ID_COMBOFILTER,wxT("FILTER"),wxPoint(60,40),wxDefaultSize,
	                   strings,wxCB_DROPDOWN);
	filterCombo->SetSelection(5);
	
//	winout("Ant %p count %d\n",sdr->antenna,sdr->antennaCount);
	
	if(sdr->antennaNames.size() > 0){
 		strings.Clear();
       for (size_t i=0;i<sdr->antennaNames.size();++i){
       		strings.Add(sdr->antennaNames[i]);
        }
		new wxStaticText(panel2,wxID_STATIC,wxT("Antenna:"),wxPoint(10,72), wxDefaultSize,wxALIGN_LEFT);

		antennaCombo=new wxComboBox(panel2,ID_COMBOANTENNA,wxT("FILTER"),wxPoint(60,70),wxDefaultSize,
	                   strings,wxCB_DROPDOWN);
		antennaCombo->SetSelection(0);
        

	}

    yloc += 110;
    

       wxPanel *panel3 = new wxPanel(ScrolledWindow,wxID_ANY, wxPoint(20,yloc), wxSize(230, 270),wxBORDER_SUNKEN | wxFULL_REPAINT_ON_RESIZE,wxT("Control2"));

      wxString computers2[] =
      { "Float", "Short int","Signed char","Unsigned char" };
      
	  sendTypeBox=new wxRadioBox(panel3, ID_DATATYPE,
		"Data Type", wxPoint(2,10), wxSize(120, 140),
		 4, computers2, 0, wxRA_SPECIFY_ROWS);
		 
	sendTypeBox->SetSelection(1);

      wxString computers3[] =
      { "Listen","TCP/IP","UDP","Speakers","Pipe" };
      
	  sendModeBox=new wxRadioBox(panel3, ID_DATAMODE,
		"Send Mode", wxPoint(125,10), wxSize(120, 140),
		 5, computers3, 0, wxRA_SPECIFY_ROWS);
		 
	sendModeBox->SetSelection(1);
	
	wxStaticBox *box22 = new wxStaticBox(panel3, wxID_ANY, "&Net-Address",wxPoint(15,170), wxSize(210, 60),wxBORDER_SUNKEN );

    sendAddress=new wxTextCtrl(box22,ID_TEXTCTRL,wxT("192.168.1.3:3500"),
          wxPoint(5,15), wxSize(190, 30));

  	new wxButton(panel3,ID_STARTSEND,wxT("Start"),wxPoint(20,235));
  	
  	new wxButton(panel3,ID_STOPSEND,wxT("Stop"),wxPoint(140,235));

 	yloc += 285;
	
	wxStaticBox *box33 = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Alpha",wxPoint(20,yloc), wxSize(225, 220),wxBORDER_SUNKEN );
	box33->SetToolTip(wxT("Click Apply To Activate") );

    textAlpha=new wxTextCtrl(box33,ID_TEXTCTRL,wxT("0.1"),
          wxPoint(5,15), wxSize(220, 30));
	textAlpha->SetToolTip(wxT("Click Apply To Activate") );
          
    cbox=new wxCheckBox(box33,ID_SETGAIN, "&Power Range",wxPoint(20,50), wxSize(230, 25));
	cbox->SetValue(0);	


	new wxStaticText(box33,wxID_STATIC,wxT("pmin:"),wxPoint(20,94), wxSize(50, 30),wxALIGN_LEFT);

          
    rangeMin=new wxTextCtrl(box33,ID_TEXTCTRL,wxT("-120"),
          wxPoint(75,90), wxSize(100, 30));
          
  	new wxStaticText(box33,wxID_STATIC,wxT("pmax:"),wxPoint(20,124), wxSize(50, 30),wxALIGN_LEFT);
        
          
    rangeMax=new wxTextCtrl(box33,ID_TEXTCTRL,wxT("-30"),
          wxPoint(75,120), wxSize(100, 30));
          
    
  	new wxButton(box33,ID_ALPHA,wxT("Apply"),wxPoint(10,160));
	
	yloc += 210;
	
	listctrlFreq = new wxDataViewListCtrl( ScrolledWindow, ID_VIEWSELECTED,wxPoint(20,yloc),wxSize(225, 190));
	listctrlFreq->AppendTextColumn( "Channel" );
	listctrlFreq->AppendTextColumn( "Freq" );
	
	wxVector<wxVariant> data;
	for(unsigned int n=0;n<sizeof(computers31)/sizeof(wxString);++n){
		data.clear();
		data.push_back(computers31[n]);
		data.push_back(computers32[n]);
		listctrlFreq->AppendItem( data );
		yloc += 30;
	}

    wxWindow::SetSize(wxDefaultCoord,wxDefaultCoord,280,100);
    
    Refresh();
    
    iRefresh=1;
    
  //  winout("SetScrolledWindow Done yloc %d\n",yloc);

	return 0;
}

BasicPane::BasicPane(wxWindow *frame, const wxString &title,class sdrClass *sdrIn)
    : wxWindow(frame,32000)
    , m_timer(this,TIMER_ID)
{
	m_timer.Start(50);
	
	zerol((char *)sx,sizeof(sxs));
	
	sendFlag=0;
	
	lineAlpha=0.1;
	
	softAutoGain=1;
	
	sdr=sdrIn;
	
	pBasicPane=this;
	
	gBasicPane=pBasicPane;
	
	gTopPane=pTopPane;
	
	gTopPane->sdr=sdr;
	
	gTopPane->gSpectrum=pSpectrum;
	
	gSpectrum=pSpectrum;
	
	gSpectrum->sdr=sdr;
	
	gSpectrum->gTopPane=pTopPane;
	
	gSpectrum->gBasicPane=pBasicPane;
	
	gWaterFall=pWaterFall;

	gWaterFall->sdr=sdr;
	
	gWaterFall->gTopPane=pTopPane;
	
	gWaterFall->gSpectrum=gSpectrum;
	
	gWaterFall->gBasicPane=pBasicPane;
	
	scrolledWindowFlag=0;
	
	ScrolledWindow=NULL;
	
    SetScrolledWindow();
}

void BasicPane::dataType(wxCommandEvent &event)
{
	event.Skip();
	
	//wxString name=event.GetString();
	
	//const char *mode=name.ToUTF8().data();

	//winout("dataType %s GetSelection %d\n",mode,event.GetSelection());
	
}
void BasicPane::dataMode(wxCommandEvent &event)
{
	event.Skip();
	
	//wxString name=event.GetString();
	
	//const char *mode=name.ToUTF8().data();

	//winout("dataMode %s GetSelection %d\n",mode,event.GetSelection());
	
}
void BasicPane::OnCheckAuto(wxCommandEvent &event)
{
	event.Skip();
	
	int id=event.GetId();
		
	if(id == ID_SWAPIQ){
		sdr->IQSwap=event.GetSelection();
		return;
	}else if(id == ID_SOFTAUTOGAIN){
		softAutoGain=event.GetSelection();
		gSpectrum->softAutoGain=softAutoGain;
		return;
	}else if(id == ID_SETGAIN){
		int flag=event.GetSelection();
		if(flag){
		    char value[256];
			gWaterFall->pd.UsePlotScales=1;
		    sprintf(value,"%g",gWaterFall->pmin);
		    rangeMin->SetValue(value);
		    sprintf(value,"%g",gWaterFall->pmax);
		    rangeMax->SetValue(value);
		}else{
			gWaterFall->pd.UsePlotScales=0;	
		}
		return;
	}else if(id == ID_OSCILLOSCOPE){
		int flag=event.GetSelection();
		//winout("ID_OSCILLOSCOPE flag %d\n",flag);
		gSpectrum->oscilloscope=flag;
		scrolledWindowFlag=flag;
		SetScrolledWindow();
		wxSize size = gApplFrame->GetClientSize();
		size.x += 4;
		size.y += 4;
		gApplFrame->SetClientSize(size);
		size.x -= 4;
		size.y -= 4;
		gApplFrame->SetClientSize(size);
		sdr->initPlay();
		if(flag == 1){		
		    gSpectrum->sdr->bS2->mutex1.lock();
			gSpectrum->sdr->bS2->bufftop=0;
			gSpectrum->sdr->witch=0;
		    gSpectrum->sdr->bS2->mutex1.unlock();
		    //fprintf(stderr,"Clear Buffer\n");
		}
		return;
	}
	
	if(sdr->inData != IN_RADIO)return;
	
	
	if(id == ID_CHECKAUTO){
		
		bool gainMode=sdr->device->getGainMode(SOAPY_SDR_RX, sdr->channel);
		
		//winout("1 OnCheckAuto id %d gainMode %d\n",id,gainMode);
	
		gainMode = !gainMode;
	
		//winout("1a OnCheckAuto id %d gainMode %d\n",id,gainMode);
		
		sdr->device->setGainMode(SOAPY_SDR_RX, sdr->channel, gainMode);

		//winout("2 OnCheckAuto id %d gainMode %d\n",id,gainMode);
		
		//gainMode=sdr->device->getGainMode(SOAPY_SDR_RX, sdr->channel);

		//winout("2a OnCheckAuto id %d gainMode %d\n",id,gainMode);
		
	}

}


void BasicPane::setRxGain(wxCommandEvent &event)
{
	event.Skip();
	
	int id=event.GetId()-ID_RXGAIN;
	
	//winout("setRxGain id %d\n",id);
	
	double rxgain=event.GetSelection();
	
	if(id == 0){
		sdr->device->setGain(SOAPY_SDR_RX, sdr->channel,rxgain);
	}else{
		sdr->device->setGain(SOAPY_SDR_RX, sdr->channel, sdr->rxGainNames[id], rxgain);
	}
	
	
	//sdr->setSampleWidth(samplewidth);	
}
void BasicPane::setSampleWidth(wxCommandEvent &event)
{
	event.Skip();
	
	double samplewidth=event.GetSelection();
	
	sdr->setSampleWidth(samplewidth);	
}
void BasicPane::setDataRotate(wxCommandEvent &event)
{
	event.Skip();
	
	sampleDataRotate=event.GetSelection();
	
	sampleDataRotate=(200-sampleDataRotate)/200;
	
	Sleep2(100);
	
	//fprintf(stderr,"sampleDataRotate %g\n",sampleDataRotate);
	
}
void BasicPane::OnTextBandWidth(wxCommandEvent &event)
{
	event.Skip();
	
	//winout("OnTextBandWidth\n");
	
	wxString bandwidth=event.GetString();
	
	if(!bandwidth)return;
	
	s->bS=NULL;

	sdr->waitPointer("iqToAudio(11)",&sdr->iqToAudio,0);

	sdr->waitPointer("frame(11)",&sdr->frame,0);

	sdr->waitPointer("doWhat(11)",&sdr->doWhat,0);
	
	sdr->bandwidth=atof(bandwidth);
	
	sdr->startPlay();
	
	std::thread(&sdrClass::run,sdr).detach();
	
}
void BasicPane::setBandwidth(wxCommandEvent &event)
{
	event.Skip();
	
	//winout("setBandwidth\n");
	
	//winout("Filter %d\n",bandwidthCombo->GetSelection());
	
	
	wxString bandWidths=event.GetString();

	//const char *bandWidth=bandWidths.c_str();

   //winout("bandWidth %s len %d\n",bandWidth,(int)strlen(bandWidth));
   
    s->bS=NULL;

	sdr->waitPointer("iqToAudio(10)",&sdr->iqToAudio,0);

	sdr->waitPointer("frame(10)",&sdr->frame,0);

	sdr->waitPointer("doWhat(10)",&sdr->doWhat,0);
	
	sdr->bandwidth=atof(bandWidths);
	
	sdr->startPlay();
	
	std::thread(&sdrClass::run,sdr).detach();

}
void BasicPane::OnMinimun(wxCommandEvent &event)
{
	event.Skip();

	float gain=event.GetSelection();
	gSpectrum->verticalMinimum=gain;
	//winout("min gain %g\n",gain);
	if(gain+20 > gSpectrum->verticalMaximum)gSpectrum->verticalMaximum=gain+20;

}
void BasicPane::OnMaximun(wxCommandEvent &event)
{
	event.Skip();
	float gain=event.GetSelection();
	gSpectrum->verticalMaximum=gain;
	//winout("max gain %g\n",gain);
	if(gain-20 < gSpectrum->verticalMinimum)gSpectrum->verticalMinimum=gain-20;

}

void BasicPane::setSampleRate(wxCommandEvent &event)
{
	event.Skip();
	winout("setSampleRate\n");
	
	winout("Filter %d\n",sampleRateCombo->GetSelection());
	

	wxString name=sampleRateCombo->GetValue();

	winout("S1\n");

	
	if(!name)return;
	
	winout("S2\n");
	
	const char *mode=name.ToUTF8().data();
	
	std::string number=mode;
	
	winout("Filter %s\n",number.c_str());


}

void BasicPane::OnText(wxCommandEvent &event)
{
	event.Skip();
		
	wxString rates=event.GetString();
	
	if(!rates)return;
	
	s->bS=NULL;

	sdr->waitPointer("iqToAudio(7)",&sdr->iqToAudio,0);

	sdr->waitPointer("frame(7)",&sdr->frame,0);

	sdr->waitPointer("doWhat(7)",&sdr->doWhat,0);
	
	sdr->samplerate=atof(rates);
	sdr->samplewidth=sdr->samplerate;
	
	//fprintf(stderr,"samplerate %g\n",sdr->samplerate);
	
	sdr->startPlay();
	
	std::thread(&sdrClass::run,sdr).detach();
	
}

void BasicPane::OnScroll(wxCommandEvent &event)
{
	event.Skip();
	//int which=event.GetId();
	float gain=event.GetSelection();
	if(event.GetId() == SCROLL_GAIN){
	//	winout("OnScroll gain %g %d \n",gain,event.GetId());
		s->gain=0.01*gain;
//		gain=value;
		return;
	}
	
	//setgain(value,f.p[which]);
}

 void BasicPane::OnComboFilter(wxCommandEvent& event){

	event.Skip();
	
	wxString name=event.GetString();

	const char *mode=name.ToUTF8().data();

	std::string number=mode;
	
	winout("Filter %s %d\n",number.c_str(),filterCombo->GetSelection());
	
	gSpectrum->filterType=filterCombo->GetSelection();


}
 
 void BasicPane::render( wxPaintEvent& evt )
{
	evt.Skip();
	//winout("BasicPane:render sdr->decodemode %d\n",sdr->decodemode);
	modeBox->SetSelection(sdr->decodemode);

}

 void BasicPane::resized(wxSizeEvent& evt)
{
//	wxGLCanvas::OnSize(evt);
	//winout("BasicPane::resized %d %d\n",getWidth(),getHeight());
	ScrolledWindow->SetSize(0,0,getWidth(),getHeight());
	evt.Skip();
   	Refresh();
}

int BasicPane::getWidth()
{
    return GetSize().x;
}
 
int BasicPane::getHeight()
{
    return GetSize().y;
}

void BasicPane::mouseMoved(wxMouseEvent& event) {	event.Skip();}

void BasicPane::mouseDown(wxMouseEvent& event) {	event.Skip(); winout("BasicPane::mouseDown\n");}

/*
void BasicPane::OnButton(wxCommandEvent& event) 
{
//    event.Skip();

	wxWindow *parent=GetParent();
	wxWindow *parent2=parent->GetParent();
	if(parent2)parent2->Destroy();
//	parent->Destroy();

}
*/

void BasicPane::startRadio(wxCommandEvent& event) 
{    
	event.Skip();
	
}

void BasicPane::OnTimer(wxTimerEvent &event){
		event.Skip();

	if(!gSpectrum){
		Sleep2(10);
		return;
	}
	//static long count;
	
	//winout("BasicPane::OnTimer count %ld gSpectrum->iWait %d\n",count++,gSpectrum->iWait);
	
	if(gSpectrum->iWait){
		iRefresh=1;
		Sleep2(10);
		return;
	}
	
	//winout("BasicPane::OnTimer gSpectrum->iWait %d gSpectrum->sdr %p\n",gSpectrum->iWait,gSpectrum->sdr);
	
	//auto t1 = std::chrono::high_resolution_clock::now();

	
	if(gSpectrum->sdr->saveFlag){
		
		for(int n=0;n<gSpectrum->buffSendLength;++n){
			gSpectrum->buffSend2[2*n]=gSpectrum->sdr->saveBuff[2*n];
			gSpectrum->buffSend2[2*n+1]=gSpectrum->sdr->saveBuff[2*n+1];
			//if(fabs(buff[2*n]) > amax)amax=fabs(buff[2*n]);
		}
		
		//winout("sdr->saveFlag %d buffSendLength %d buffFlag %d\n",sdr->saveFlag,buffSendLength,buffFlag);
		
		gSpectrum->buffFlag=1;
		
	    gSpectrum->sdr->saveFlag=0;
	}else{
		//winout("Save Not Set\n");
	}
	
	if(iRefresh){
		iRefresh=0;
		gTopPane->Refresh();
		Refresh();
		if(ScrolledWindow){
			ScrolledWindow->Refresh();
		}
		//winout("gTopPane->Refresh called f %g sdr->decodemode %d\n",sdr->f,sdr->decodemode);
	}
	
	if(gSpectrum->buffFlag){
		gSpectrum->Refresh();
		gWaterFall->Refresh();
	}else{
	 // winout("OnTimer Skip call\n");
	}
	
	//auto t2 = std::chrono::high_resolution_clock::now();
	//std::chrono::duration<double> difference = t2 - t1;
	//winout("Time in OnTimer %g\n",difference.count());

	
	
}



void BasicPane::OnComboSampleRate(wxCommandEvent& event)
{
	event.Skip();

	wxString rates=event.GetString();

//	const char *rate=rates.c_str();

//	winout("rate %s len %d\n",rate,(int)strlen(rate));

	//winout("Start Stop\n");
	
	s->bS=NULL;

	sdr->waitPointer("iqToAudio(7)",&sdr->iqToAudio,0);

	sdr->waitPointer("frame(7)",&sdr->frame,0);

	sdr->waitPointer("doWhat(7)",&sdr->doWhat,0);

	
	sdr->samplerate=atof(rates);
	sdr->samplewidth=0;
	
	Sleep2(50);
	
	
	//winout("Stop Stop sdr->samplerate %g\n",sdr->samplerate);

	
	sdr->startPlay();
	
	std::thread(&sdrClass::run,sdr).detach();
}

void BasicPane::OnComboAntenna(wxCommandEvent& event)
{
		event.Skip();
		
		wxString antennas=event.GetString();
		
		const char *antenna=antennas.c_str();
		
		//winout("antenna %s len %d\n",antenna,(int)strlen(antenna));
		
		sdr->device->setAntenna(SOAPY_SDR_RX, sdr->channel, antenna);

}

void BasicPane::OnCombo(wxCommandEvent& event){
	//int ngroup=event.GetSelection();
//	winout("OnCombo %d IsChecked %d\n",item,event.IsChecked());
//	noteCheckBox=event.IsChecked();

		event.Skip();
		
		
	//winout("BasicPane::OnCombo\n");

	wxString number=event.GetString();
	
	double value;
	
	number.ToCDouble(&value);
	

    if(!gSpectrum->buffSend10){
    	winout("Radio Not Running\n");
    	return;
	}
	
    gSpectrum->iWait=1;
	Sleep2(20);
	gSpectrum->sdr->saveCall=0;
	Sleep2(20);
	gSpectrum->buffSendLength=0;
	gSpectrum->buffFlag=1;
	
	int np=(int)value;
	

	if(gSpectrum->buffSend)cFree((char *)gSpectrum->buffSend);
	gSpectrum->buffSend=(float *)cMalloc(sizeof(float)*8*np,9588);
	
	if(gSpectrum->buffSend2)cFree((char *)gSpectrum->buffSend2);
	gSpectrum->buffSend2=(float *)cMalloc(sizeof(float)*8*np,95889);
	
	if(gSpectrum->buffSend10)cFree((char *)gSpectrum->buffSend10);
	gSpectrum->buffSend10=(float *)cMalloc(sizeof(float)*8*np,9889);

	
	memset(gSpectrum->buffSend10, 0, sizeof(float)*8*np);
		
	
	gSpectrum->p1 = fftwf_plan_dft_1d(np,(fftwf_complex *)gSpectrum->buffSend2, (fftwf_complex *)gSpectrum->buffSend, FFTW_FORWARD, FFTW_ESTIMATE);


	gSpectrum->sdr->setDataSave(np);
	

	gSpectrum->buffFlag=0;
	
	gSpectrum->iWait=0;
	
	gSpectrum->buffSendLength=np;


//	winout("np %d iWait %d saveCall %d iWait %p\n",np,gSpectrum->iWait,gSpectrum->sdr->saveCall,&gSpectrum->iWait);
	
}


void BasicPane::OnButton2(wxCommandEvent& event) 
{    
	event.Skip();

}
void BasicPane::radioBox(wxCommandEvent& event) 
{
	event.Skip();
	
	wxString name=event.GetString();
	
	const char *mode=name.ToUTF8().data();
	
	sdr->setMode(mode);
	
}


BasicPane::~BasicPane()
{
	//winout("exit1 BasicPane %p thread %llx\n",this,(long long)std::this_thread::get_id);
	if(sdr)delete sdr;
	sdr=NULL;
	//winout("exit2 BasicPane %p thread %llx\n",this,(long long)std::this_thread::get_id);
}



BEGIN_EVENT_TABLE(WaterFall, wxGLCanvas)
EVT_MOTION(WaterFall::mouseMoved)
EVT_PAINT(WaterFall::render)
EVT_SIZE(WaterFall::resized)
EVT_LEFT_DOWN(WaterFall::mouseDown)
EVT_MOUSEWHEEL(WaterFall::mouseWheelMoved)
EVT_CHAR(WaterFall::OnChar)    

END_EVENT_TABLE()

void WaterFall::OnChar(wxKeyEvent& event) 
{
	extern soundClass *s;

	event.Skip();
	
    int keycode=event.GetKeyCode();
    
   // winout("WaterFall::OnChar %d\n",keycode);
    
    if(keycode == 'w'){
        iWait = !iWait;
       // winout("iWait %d\n",iWait);
    }

    if(keycode == 'n'){
        sdr->iWait = !sdr->iWait;
       // winout("sdr->iWait %d\n",sdr->iWait);
    }

    if(keycode == 'm' || keycode == 'M' ){
		if(s->bS){
		   s->bS=NULL;
		}else{
		   s->bS=sdr->bS;
		}
    }
	
}

void WaterFall::mouseWheelMoved(wxMouseEvent& event)
{
	event.Skip();
	if(sdr){
		int rot=event.GetWheelRotation();
		if(rot > 0){
			rot=1;
		}else if(rot < 0){
			rot = -1;
		}
		
		double frequency=sdr->f+rot*0.5*sdr->bw;		
		if(sdr->samplescale < 0.98){
			sdr->setCenterFrequency(frequency);
		}else{
			sdr->setFrequency(frequency);
		}
		gTopPane->Refresh();
//		winout("bw %g %ld\n",sdr->bw,(long)event.ControlDown());
	}
}
void WaterFall::mouseDown(wxMouseEvent& event) 
{
	extern soundClass *s;
	event.Skip();
	wxPoint pp3 = event.GetLogicalPosition(wxClientDC(this))*scaleFactor;
	if(sdr){
		double fx=sdr->fw-0.5*sdr->samplewidth + sdr->samplewidth*(pp3.x)/((double)getWidth());
		//winout("mouseDown x %d y %d sdr->fc %g sdr->f %g sdr->samplerate %g fx %g width %d\n",p1.x,p1.y,sdr->fc,sdr->f,sdr->samplerate,fx,getWidth());
		//winout(" fx %g\n",fx);
		if(sdr->samplescale < 0.98){
			sdr->setCenterFrequency(fx);
		}else{
			sdr->setFrequency(fx);
		}
		s->bS=sdr->bS;
		gTopPane->Refresh();

	}
}
// some useful events to use
void WaterFall::mouseMoved(wxMouseEvent& event) {
	//winout("mouseMoved \n");
		event.Skip();

}
WaterFall::~WaterFall()
{
	if(m_context)delete m_context;
	
	if(water.data)cFree((char *)water.data);
	water.data=NULL;

	//winout("exit WaterFall %p\n",this);
}

int WaterFall::getWidth()
{
    return (int)(GetSize().x*scaleFactor);
}
 
int WaterFall::getHeight()
{
    return (int)(GetSize().y*scaleFactor);
}
void WaterFall::resized(wxSizeEvent& evt)
{
	evt.Skip();
	
	scaleFactor=GetContentScaleFactor();

	SetWindow();
	
   	Refresh();
}
int WaterFall::ftox(double frequency){

	int x;
	
	x=(int)(getWidth()*(frequency-sdr->fw+0.5*sdr->samplewidth)/sdr->samplewidth);

	return x;
}

void WaterFall::render( wxPaintEvent& evt )
{
	evt.Skip();
//   winout("WaterFall::render\n");

    if(!IsShown()) return;
    
    if(iWait)return;
    
    float *magnitude=gSpectrum->buffSend10;
    
    int length=gSpectrum->buffSendLength;
    
    //auto t1 = chrono::high_resolution_clock::now();
    

   // winout("WaterFall render 2\n");
        
    wxGLCanvas::SetCurrent(*m_context);
    wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
    //wxClientDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
    
 //   const wxSize ClientSize = GetClientSize();
    
   
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    // ------------- draw some 2D ----------------
    prepare2DViewport(0,0,getWidth(), getHeight());
    glLoadIdentity();
 
    // white background
    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);
    glVertex3f(0,0,0);
    glVertex3f(getWidth(),0,0);
    glVertex3f(getWidth(),getHeight(),0);
    glVertex3f(0,getHeight(),0);
    glEnd();
    

	double *range = new double[length];

    pmin =  1e33;
    pmax = -1e33;

    double rmin=sdr->fw-0.5*sdr->samplewidth;
    double rmax=sdr->fw+0.5*sdr->samplewidth;
    
    double ddx=(double)sdr->samplerate/(double)(length);
    for(int n=0;n<length;++n){
        double r;
        r=sdr->fc-0.5*sdr->samplerate+n*ddx;
        range[n]=r;
        double v=magnitude[n];
        if(v < pmin)pmin=v;
        if(v > pmax)pmax=v;
    }    

    if(!pd.UsePlotScales){
    	pd.dmin=pmin;
    	pd.dmax=pmax;
    	pd.sPmin=pmin;
    	pd.sPmax=pmax;
    }
    
//   winout("rmin %g rmax %g amin %g amax %g\n",rmin,rmax,amin,amax);
    
    
    if(water.nline >= water.ysize)water.nline=0;
    
    unsigned char *wateric= new unsigned char[length*3];
    
    FloatToImage(magnitude,length,&pd,wateric);
/*
    int ns1=3*water.xsize*(water.ysize-water.nline-1);
    int ns2=3*water.xsize*water.ysize+3*water.xsize*(water.ysize-1-water.nline++);
*/
    int ns1=3*water.xsize*(water.nline+water.ysize);
    int ns2=3*water.xsize*(water.nline++);

    int nmin,nmax;
    nmin=length-1;
    nmax=0;
    for(int n=0;n<length;++n){
        if(range[n] <= rmin)nmin=n;
        if(range[n] <= rmax)nmax=n;
    }
    

    double dxn = -1;
    if(nmax-nmin){
        dxn=(double)(nmax-nmin)/(double)(length-1);
    }else{
        nmin=0;
        dxn = 1;
    }
    
    
    //winout("nmin %d nmax %d dxn %g\n",nmin,nmax,dxn);
   
    
    double dxw=(double)(water.xsize-1)/(double)(length-1);
   
    
    int ics=wateric[(int)(2*dxn+nmin)];
    
    for(int nnn=0;nnn<length;++nnn){
        int ic;
        
        int n=nnn*dxn+nmin+0.5;
        
        int next=(nnn+1)*dxn+nmin+0.5;
        
        ic=wateric[n];
 
        int nn=nnn*dxw+0.5;
        
        int nn2=next*dxw+0.5;
        
        

//            winout("nn %d nn2 %d nnn %d n %d next %d ic %d ics %d\n",nn,nn2,nnn,n,next,ic,ics);

        if(ic > ics)ics=ic;
        
        if(nn == nn2){
        	//winout("2 nn %d nn2 %d nnn %d\n",nn,nn2,nnn);
           continue;
        }
        ic=ics;
        
        ics=wateric[next];
        
        if(nn < 0 || nn >= water.SRect.xsize){
            winout("nn %d nn2 %d nnn %d n %d next %d ic %d ics %d\n",nn,nn2,nnn,n,next,ic,ics);
			exit(1);
        }
                
        water.data[ns1+3*nn]=pd.palette[3*ic];
        water.data[ns1+3*nn+1]=pd.palette[3*ic+1];
        water.data[ns1+3*nn+2]=pd.palette[3*ic+2];
        
        water.data[ns2+3*nn]=pd.palette[3*ic];
        water.data[ns2+3*nn+1]=pd.palette[3*ic+1];
        water.data[ns2+3*nn+2]=pd.palette[3*ic+2];
        
    }    
    
    water.SRect.y=water.ysize+1-water.nline;
    
   // winout("x %d y %d xsize %d ysize %d \n",water.SRect.x,water.SRect.y,water.SRect.xsize,water.SRect.ysize);

    int xsize=water.SRect.xsize;
    
    int ysize=water.SRect.ysize;
    
    int nline=water.nline;
         
    glDrawPixels(xsize,ysize,GL_RGB, GL_UNSIGNED_BYTE, &water.data[nline*xsize*3]);

    glColor4f(0, 0, 0, 1);
	int xs=ftox(sdr->f-sdr->bw/2.0);
	DrawLine(xs, 0, xs, getHeight());
	xs=ftox(sdr->f+sdr->bw/2.0);
	DrawLine(xs, 0, xs, getHeight());
	
//	winout("xs %d iWait %d\n",xs,iWait);

/*
 		
 		//winout(" left %d right %d center %d xsize %d bw %g\n",ftox(sdr->f-sdr->bw/2.0),ftox(sdr->f+sdr->bw/2.0),ftox(sdr->f),box.xsize,sdr->bw);
 		
 		DrawBox(&box,0);
*/

//	winout("Waterfall f %p\n",&sdr->f);

    	if(range)delete [] range;
    	range=NULL;
    	if(wateric)delete [] wateric;
    	wateric=NULL;

    	glFlush();
    	SwapBuffers();
    }
int WaterFall::SetWindow()
{
    
    //CheckSceneBuffer(scene);
    
    int xsize=getWidth();
    
    int ysize=getHeight();
    
    
    water.DRect.x=0;
    water.DRect.y=0;
    water.DRect.xsize=xsize;
    water.DRect.ysize=ysize;
    
    //xsize=(int)rx->FFTcount;
    
    if(ysize == water.ysize && xsize == water.xsize && water.data)return 0;
    
    water.amin=1e60;
    water.amax=-1e60;
    
    water.SRect.x=0;
    water.SRect.y=0;
    water.SRect.xsize=xsize;
    water.SRect.ysize=ysize;
    
    water.nline=0;
    
    if(water.data)cFree((char *)water.data);
    
    water.data=(unsigned char *)cMalloc(2*xsize*ysize*3,9999);
    
  //  winout("WaterFall::SetWindow\n");
    
    if(!water.data)return 1;
    
    water.ysize=ysize;
    water.xsize=xsize;
    
    for(int y=0;y<ysize*2;++y)
    {
        for(int x=0;x<xsize;++x){
            int nn=y*(xsize*3)+x*3;
            water.data[nn]=255;
            water.data[nn+1]=255;
            water.data[nn+2]=255;
        }
    }
    
    return 0;
}

 int WaterFall::FloatToImage(float *d,long length,struct paletteDraw *pd,unsigned char *bp)
{
        double dmin;
        double dmax;
        double mmax;
        double dx;
        double a,b;
        double r;
        long n;
        int dolog;
        int v;

        if(!d || !pd)return 1;


        dmin=pd->sPmin;
        dmax=pd->sPmax;
        
        dolog = 0;

        if(pd->sType == 0){

            dmin=pd->dmin;
			dmax=pd->dmax;
	
			dx=dmax-dmin;
	
			mmax=maxmy(fabs(dmin),fabs(dmax));
	
			if(dx <= 0.0 || dx < 1e-5*mmax){
				dmax=dmin+1.;
				--dmin;
			}
        }else if(pd->sType == 1){
            dolog = 1;
        }
        
        if(!bp){
            bp=(unsigned char *)d;
        }

        if(dolog){
            if(dmin <= 0.0)return 1;

            a=(254.0-2.0)/(log10(dmax)-log10(dmin));
            b=2.0-a*log10(dmin);

                for(n=0;n<length;++n){
                    r=d[n];
                    if(r == FLOAT_NOT_SET){
                        *bp++ = 0;
                        continue;
                    }else if(r < dmin){
                       r = dmin;
                    }else if(r > dmax){
                       r = dmax;
                    }

                    r=log10(r);
                    
                    v=(int)(a*r+b);
                    
                    if(v < 2){
                        v=2;
                    }else if(v > 254){
                        v=254;
                    }
                    
                    *bp++ = (unsigned char)(v);
                }

        }else{
            a=(254.0-2.0)/(dmax-dmin);
            b=2.0-a*dmin;
                for(n=0;n<length;++n){
                    r=d[n];
                    if(r == FLOAT_NOT_SET){
                        *bp++ = 0;
                        continue;
                    }

                    v=(int)(a*r+b);
                    

                    if(v < 2){
                        v=2;
                    }else if(v > 254){
                        v=254;
                    }

                    *bp++ = (unsigned char)(v);
                }
        }

        pd->dmin=dmin;
        pd->dmax=dmax;

        return 0;
}   

WaterFall::WaterFall(wxFrame* parent, int* args) :
    //wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxSize(800,600), wxFULL_REPAINT_ON_RESIZE)
   // wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
   // wxGLCanvas(parent, wxID_ANY, args, wxPoint(200,0), wxSize(800,200), wxFULL_REPAINT_ON_RESIZE)
      wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxSize(400,100), wxFULL_REPAINT_ON_RESIZE)
{

	verticalMinimum=-130;
		
	verticalMaximum=-30;
	
	water.data=NULL;

	pWaterFall=this;
	
	scaleFactor=1.0;
	
	m_context = new wxGLContext(this);    
	
    // To avoid flashing on MSW
    
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    
    angle=50;
    
    buffFlag=0;
    
    iWait=0;
    
    startTime = new std::chrono::high_resolution_clock;
    
    memset((char *)&pd,0,sizeof(struct paletteDraw));
    
    memset((char *)&water,0,sizeof(struct WaterFall4));
    
    pd.sType=2;
    pd.sPmin=-120;
    pd.sPmax=-30;
    
	char name[512];

	double pal[3*256];

	getPalette(12,name,pal);

	for(int n=0;n<256;++n){
		pd.palette[3*n]=pal[3*n]*255;
		pd.palette[3*n+1]=pal[3*n+1]*255;
		pd.palette[3*n+2]=pal[3*n+2]*255;
	}


}


BEGIN_EVENT_TABLE(Spectrum, wxGLCanvas)
EVT_MOTION(Spectrum::mouseMoved)
EVT_LEFT_DOWN(Spectrum::mouseDown)
EVT_LEFT_UP(Spectrum::mouseReleased)
EVT_RIGHT_DOWN(Spectrum::rightClick)
EVT_LEAVE_WINDOW(Spectrum::mouseLeftWindow)
EVT_SIZE(Spectrum::resized)
//EVT_KEY_DOWN(Spectrum::keyPressed)
//EVT_KEY_UP(Spectrum::keyReleased)
EVT_MOUSEWHEEL(Spectrum::mouseWheelMoved)
EVT_PAINT(Spectrum::render)
EVT_CHAR(Spectrum::OnChar)    
//EVT_BUTTON(ID_DELETE,Spectrum::DeleteRow )
EVT_IDLE(Spectrum::OnIdle)
END_EVENT_TABLE()

void Spectrum::OnIdle(wxIdleEvent& event)
{	
	event.Skip();
	if(oscilloscope == 1){
	    //fprintf(stderr,"Spectrum::OnIdle\n");
		Refresh();
	}
}

// some useful events to use
void Spectrum::mouseMoved(wxMouseEvent& event) {
	//winout("mouseMoved \n");
	event.Skip();
}
void Spectrum::mouseReleased(wxMouseEvent& event) {event.Skip();}
void Spectrum::rightClick(wxMouseEvent& event) {event.Skip();}
void Spectrum::mouseLeftWindow(wxMouseEvent& event) {event.Skip();}
void Spectrum::keyReleased(wxKeyEvent& event) {event.Skip();}

void Spectrum::mouseWheelMoved(wxMouseEvent& event)
{
	event.Skip();
	if(sdr){
		int rot=event.GetWheelRotation();
		if(rot > 0){
			rot=1;
		}else if(rot < 0){
			rot = -1;
		}		
		double frequency=sdr->f+rot*0.5*sdr->bw;		
		if(sdr->samplescale < 0.98){
			sdr->setCenterFrequency(frequency);
		}else{
			sdr->setFrequency(frequency);
		}
	
		gTopPane->Refresh();
//		winout("bw %g %ld\n",sdr->bw,(long)event.ControlDown());
	}

}

void Spectrum::OnChar(wxKeyEvent& event) 
{
	event.Skip();
	
	int keycode=event.GetKeyCode();
	
	//winout("Spectrum::OnChar %d\n",keycode);
	
    if(keycode == 'f'){
        iFreeze = !iFreeze;
     //   winout("iWait %d\n",iWait);
    }
    
    if(keycode == 'w'){
        iWait = !iWait;
     //   winout("iWait %d\n",iWait);
    }

    if(keycode == 'n'){
        sdr->iWait = !sdr->iWait;
     //   winout("sdr->iWait %d\n",sdr->iWait);
    }
    
    if(keycode == 'm' || keycode == 'M' ){
		if(s->bS){
		   s->bS=NULL;
		}else{
		   s->bS=sdr->bS;
		}
    }
	
	
}
  
int InitGL=0;



Spectrum::Spectrum(wxFrame* parent, int* args) :
    //wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxSize(800,600), wxFULL_REPAINT_ON_RESIZE)
   // wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
   // wxGLCanvas(parent, wxID_ANY, args, wxPoint(200,0), wxSize(800,200), wxFULL_REPAINT_ON_RESIZE)
      wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxSize(800,100), wxFULL_REPAINT_ON_RESIZE)
{

	softAutoGain=1;

	verticalMinimum=-130;
		
	verticalMaximum=-30;
	
	amaxGlobal=0;

	m_context = new wxGLContext(this);    
	
	scaleFactor=1.0;
	
	
	lineAlpha=0.1;
	
	pSpectrum=this;
	

    // To avoid flashing on MSW
    
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    

    angle=50;
    
    buffSend=NULL;
    
    buffSend2=NULL;
    
    buffSend10=NULL;
    
    buffSendLength=0;
    
    buffFlag=0;
    
    iWait=0;
        
    decodemode1= -11;
    
    iHaveData=0;
    
    iFreeze=0;
    
    buff1=NULL;
    
    buff2=NULL;
    
    buff3=NULL;
	
	buffSize = -10;

    
    filterType=FILTER_BLACKMANHARRIS7;
    
    startTime = new std::chrono::high_resolution_clock;
    
    lineDumpInterval=0.1;
    lineTime=ftime()+lineDumpInterval;

    
  //  winout("Groups of triangles %ld vender %s\n",triangle,glGetString(GL_VENDOR));
  
	//Bind(wxEVT_CHAR, &Spectrum::OnChar, this);    
}



void Spectrum::InitOpenGl()
{
    if(InitGL) return;
    //
    while ( !IsShown() ) {};  // Force the Shown
    
    SetCurrent(*m_context);

#ifdef GLEW_IN

    GLenum err = glewInit();
    if(err!=GL_NO_ERROR)
    {
        wxMessageBox(
            wxString("GLEW Error: ") +
            wxString(glewGetErrorString(err)),
            _("OpenGl ERROR"),
            wxOK | wxICON_EXCLAMATION
        );
   }
#endif
   InitGL=1;
   
}
double Spectrum::ftime()
{
	//auto t1=startTime->now();
	//std::chrono::time_point<std::chrono::system_clock> tt = std::chrono::system_clock::now();
	auto t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> t5 = t2.time_since_epoch();
	//std::chrono::duration<double> difference = t2-t2;
	return t5.count();
}

void Spectrum::startRadio2() 
{
	//winout("startRadio2 Start sdr %p thread %llx\n",sdr,(long long)std::this_thread::get_id);
	
    
    if(buffSend10){
        return;
    }
        	
	int np=8192;
	
	int npp=131072;
		
	buffSendLength=np;
	
	buffSend=(float *)cMalloc(sizeof(float)*8*npp,9588);
	
	buffSend2=(float *)cMalloc(sizeof(float)*8*npp,95889);
	
	buffSend10=(float *)cMalloc(sizeof(float)*8*npp,9889);
	
	memset(buffSend10, 0, sizeof(float)*8*npp);
		
	p1 = fftwf_plan_dft_1d(np,(fftwf_complex *)buffSend2, (fftwf_complex *)buffSend, FFTW_FORWARD, FFTW_ESTIMATE);
	
	sdr->setDataSave(np);
		
	std::thread(&sdrClass::run,sdr).detach();
		
   //winout("startRadio2 out sdr %p thread %llx \n",sdr,(long long)std::this_thread::get_id);

}


void Spectrum::keyPressed(wxKeyEvent& event) 
{
    wxString key;
    long keycode = event.GetKeyCode();
    
    winout("keycode keyPressed %ld\n",keycode);
    
    if(keycode == 84){
    	doTestSpeed();
    }
        
    if(keycode == 87){
        iWait = !iWait;
        winout("iWait %d\n",iWait);
    }

    if(keycode == 78){
        sdr->iWait = !sdr->iWait;
        winout("sdr->iWait %d\n",sdr->iWait);
    }
    
	event.Skip();
}
int Spectrum::doTestSpeed()
{
    
	double start,end;
	long n;
	double den;
	
	winout("doTestSpeed\n");
	//count=0;
	
	start=rtime();
	
	for(n=0;n<25;++n){
	    //count++;
		//doCommands(NewScene);
		angle += 2;
	}
	
	
	end=rtime();
	den=end-start;
	if(den <= 0)den=1;
	return 0;

}

Spectrum::~Spectrum()
{
	if(m_context)delete m_context;
    
    if(buffSend10)cFree((char *)buffSend10);
    if(buffSend2)cFree((char *)buffSend2);
    if(buffSend)cFree((char *)buffSend);
	buffSend10=NULL;
	buffSend2=NULL;
	buffSend=NULL;
    if(buff1)cFree((char *)buff1);
    if(buff2)cFree((char *)buff2);
    if(buff3)cFree((char *)buff3);
    buff1=NULL;
    buff2=NULL;
    buff3=NULL;
	//winout("exit Spectrum %p\n",this);

}

void Spectrum::resized(wxSizeEvent& evt)
{
//	wxGLCanvas::OnSize(evt);
	//winout("Spectrum::resized %d %d\n",getWidth(),getHeight());
	evt.Skip();
	scaleFactor=GetContentScaleFactor();
	Refresh();
}
 
 
int Spectrum::getWidth()
{
    return (int)(GetSize().x*scaleFactor);
}
 
int Spectrum::getHeight()
{
    return (int)(GetSize().y*scaleFactor);
}
void Spectrum::mouseDown(wxMouseEvent& event) 
{
	extern soundClass *s;

	event.Skip();
	
	wxPoint pp3 = event.GetLogicalPosition(wxClientDC(this))*scaleFactor;

	if(buffSendLength){
		if(sdr){
		    double fx=sdr->fw-0.5*sdr->samplewidth + sdr->samplewidth*(pp3.x)/((double)getWidth());
			//winout("mouseDown x %d y %d sdr->fc %g sdr->f %g sdr->samplerate %g fx %g width %d\n",p1.x,p1.y,sdr->fc,sdr->f,sdr->samplerate,fx,getWidth());
			//winout(" fx %g\n",fx);
			if(sdr->samplescale < 0.98){
				sdr->setCenterFrequency(fx);
			}else{
				sdr->setFrequency(fx);
			}
			s->bS=sdr->bS;
			//winout("Spectrum::mouseDown %p %p\n",s->bS,sdr->bS);
			gTopPane->Refresh();
		}
	}else{
		winout("Spectrum::mouseDown x %d y %d\n",pp3.x,pp3.y);
	}

}

int Spectrum::ftox(double frequency){

	int x;
	
	x=(int)(getWidth()*(frequency-sdr->fw+0.5*sdr->samplewidth)/sdr->samplewidth);

	return x;
}
void Spectrum::render(wxPaintEvent& evt )
{
	if(oscilloscope == 1){
		render1(evt);	
	}else{
		render2(evt);
	}
}

void Spectrum::render1(wxPaintEvent& evt )
{
	evt.Skip();
	
	//winout("Spectrum render nc %lld\n",nc++);

    if(!IsShown()) return;
    
    //auto t1 = chrono::high_resolution_clock::now();
    
        
    wxGLCanvas::SetCurrent(*m_context);
    //wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
    wxClientDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    // ------------- draw some 2D ----------------
    prepare2DViewport(0,0,getWidth(), getHeight());
    glLoadIdentity();
 
    // white background
    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);
    glVertex3f(0,0,0);
    glVertex3f(getWidth(),0,0);
    glVertex3f(getWidth(),getHeight(),0);
    glVertex3f(0,getHeight(),0);
    glEnd();
    
    //winout("render1\n");
    
    int buffSendLength=0;
    
    float *buffSend2;
    
    double amax=0;
    
	if(!iWait){
 	
		glColor4f(0, 0, 1, 1);
		
		//winout("filterType %d\n",filterType);
		
      	if(decodemode1 != sdr->decodemode){
      		decodemode1=sdr->decodemode;
      		double Ratio = (float)(sdr->bw/sdr->samplerate);
      		iqSampler1  = msresamp_crcf_create(Ratio, 60.0f);
      	}
    	
		if(buffSize != sdr->size){
			buffSize=sdr->size;
			if(buff1)cFree((char *)buff1);
			buff1=(float *)cMalloc(sizeof(float)*8*sdr->size,95288);
			if(buff2)cFree((char *)buff2);
			buff2=(float *)cMalloc(sizeof(float)*8*sdr->size,95288);
			if(buff3)cFree((char *)buff3);
			buff3=(float *)cMalloc(sizeof(float)*8*sdr->size,95288);
			iHaveData=0;
		}
		
		int ip = -1;
 	  	if(sdr->bS2)ip=sdr->bS2->popBuff();
      	if(ip < 0){ 	    	
 			//winout("render1 ip %d\n",ip);
      	    return;
      	}
      	
      	
      	int witch=ip % NUM_DATA_BUFF;
		buffSend2=sdr->bS2->buff[witch];
		
		//fprintf(stderr,"witch %d ip %d\n",witch,ip);
		
		double sint,cost;
		
		unsigned int num;
		
		if(iFreeze && iHaveData){
			for (int k = 0 ; k < sdr->size ; k++){
				float r = buff1[k * 2];
				float i = buff1[k * 2 + 1];
				float rr=r;
				float ii=i;
				if(sdr->dt > 0){
					rr = (float)(r*sdr->coso - i*sdr->sino);
					ii = (float)(i*sdr->coso + r*sdr->sino);
					sint=sdr->sino*sdr->cosdt+sdr->coso*sdr->sindt;
					cost=sdr->coso*sdr->cosdt-sdr->sino*sdr->sindt;
					sdr->coso=cost;
					sdr->sino=sint;
				 }else{
					buff2[k * 2] = r;
					buff2[k * 2 + 1] = i;
					continue;
				}       
				if(gBasicPane->sampleDataRotate > 0.0){
					//fprintf(stderr,"gBasicPane->sampleDataRotate \n");
					//gBasicPane->sampleDataRotate=0.5;
					int nn=gBasicPane->sampleDataRotate*(sdr->size-1)+k;
					if(nn > sdr->size-1){
						nn -= sdr->size;
					}
					//fprintf(stderr,"nn %d k %d\n",nn,k);
					buff2[nn * 2] = rr;
					buff2[nn * 2+ 1] = ii;
				}else{
					buff2[k * 2] = rr;
					buff2[k * 2+ 1] = ii;
				}
			}
	
			//if(gBasicPane->sampleDataRotate > 0.0)exit(1);
  
	 
			double r=sqrt(sdr->coso*sdr->coso+sdr->sino*sdr->sino);
		
			if(r > 0.0){
				sdr->coso /= r;	
				sdr->sino /= r;
			}
		
		
			if(isnan(sdr->coso)){
				winout("NaN found\n");
				sdr->initPlay();
				return;
			}		
		
	
			//fprintf(stderr,"1 num %d iFreeze %d\n",num,iFreeze);
		}else{
			for (int k = 0 ; k < sdr->size ; k++){
				float r = buffSend2[k * 2];
				float i = buffSend2[k * 2 + 1];
				float rr=r;
				float ii=i;
				buff1[k * 2] = rr;
				buff1[k * 2+ 1] = ii;
				if(sdr->dt > 0){
					rr = (float)(r*sdr->coso - i*sdr->sino);
					ii = (float)(i*sdr->coso + r*sdr->sino);
					sint=sdr->sino*sdr->cosdt+sdr->coso*sdr->sindt;
					cost=sdr->coso*sdr->cosdt-sdr->sino*sdr->sindt;
					sdr->coso=cost;
					sdr->sino=sint;
				 }else{
					buff2[k * 2] = r;
					buff2[k * 2 + 1] = i;
					continue;
				}       
				if(gBasicPane->sampleDataRotate > 0.0){
					//fprintf(stderr,"gBasicPane->sampleDataRotate \n");
					//gBasicPane->sampleDataRotate=0.5;
					int nn=gBasicPane->sampleDataRotate*(sdr->size-1)+k;
					if(nn > sdr->size-1){
						nn -= sdr->size;
					}
					//fprintf(stderr,"nn %d k %d\n",nn,k);
					buff2[nn * 2] = rr;
					buff2[nn * 2+ 1] = ii;
				}else{
					buff2[k * 2] = rr;
					buff2[k * 2+ 1] = ii;
				}
			}
	
			//if(gBasicPane->sampleDataRotate > 0.0)exit(1);
  
	 
			double r=sqrt(sdr->coso*sdr->coso+sdr->sino*sdr->sino);
		
			if(r > 0.0){
				sdr->coso /= r;	
				sdr->sino /= r;
			}
		
		
			if(isnan(sdr->coso)){
			//	winout("NaN found\n");
				sdr->initPlay();
				return;
			}		
		
			iHaveData=1;
			//fprintf(stderr,"2 num %d iFreeze %d\n",num,iFreeze);
		}
		
		
		//int nmax2=-1;
		//double amax2=0;
/*		
		for(int n=0;n<sdr->size;++n){
			double v=(buff2[2*n]*buff2[2*n]+buff2[2*n+1]*buff2[2*n+1]);
        	if(v > 0.0)v=sqrt(v);
         	if(v > amax2){
         		amax2=v;
         		nmax2=n;
         	}
		}
*/
		buff2[0]=buff2[0];
		buff2[1]=buff2[1];
			
		num=0;
 
 		msresamp_crcf_reset(iqSampler1);
		msresamp_crcf_execute(iqSampler1, (liquid_float_complex *)&buff2[0], sdr->size, (liquid_float_complex *)&buff3[0], &num);  // decimate
		
      	buffSendLength=num;
		
		//int nmax=-1;
		amax=0;
		for(int n=0;n<buffSendLength;++n){
			double v=(buff3[2*n]*buff3[2*n]+buff3[2*n+1]*buff3[2*n+1]);
        	if(v > 0.0)v=sqrt(v);
         	if(v > amax){
         		amax=v;
         	   //nmax=n;
         	}
		}
		
		//winout("amax %g num %d %d amax2 %g nmax2 %d\n",amax,num,nmax,amax2,nmax2);
		//winout("amax %g num %d num %d\n",amax,num,nmax);
		
		if(amaxGlobal == 0.0)amaxGlobal=amax;
        amaxGlobal = 0.9*amaxGlobal+0.1*amax;
		amax=amaxGlobal;
		
		
		//static long int count1;

		//winout("Spectrum render 1 count %ld amax %g ip %d num %d amax2 %g cosdt %g sindt %g  coso %g sino %g\n",count1++,
		//       amax,ip,num,amax2,sdr->cosdt,sdr->sindt,sdr->coso,sdr->sino);
		       			
		buffFlag=0;
		
		double ymin = -amax;
		double ymax =  amax;	
		if(ymin >= ymax)ymin=ymax-40;
		
		double dy=ymax-ymin;
		
		double iymin=0;
		double iymax=getHeight()-20;
		double idy=iymin-iymax;		
		
		double xmin=sdr->fw-0.5*sdr->samplewidth;
		double xmax=sdr->fw+0.5*sdr->samplewidth;
		double dx=xmax-xmin;

		double ixmin=50;
		double ixmax=getWidth();
		double idx=ixmax-ixmin;

		//int ixxmin,ixxmax,iyymin,iyymax;
	
		//ixxmin=100000000;
		//ixxmax= -100000000;
		//iyymin=100000000;
		//iyymax= -100000000;
	
		int ixold=0;
		int iyold=0;
		int iflag=0;
	
		//double xmin2=0;
		//double xmax2=num;
		//double dx2=xmax2-xmin2;
		
		//fprintf(stderr,"buffSendLength %ld lineAlpha %g\n",(long)buffSendLength,lineAlpha);
		
		double beta=sdr->samplewidth/sdr->samplerate;
			
		for(int n=0;n<buffSendLength;++n){
			double v;
			v=buff3[2*n];
			double x=n/((double)buffSendLength);
			double y=v;
			int ix;
			int iy;
			ix=(int)((0.5+x/beta-0.5/beta)*idx+ixmin);
			//fprintf(stderr,"n %d x %g y %g ix %d zoom %g\n",n,x,y,ix,sdr->samplewidth/sdr->samplerate);
			if(ix <= ixmin || ix >= ixmax)continue;
			//if(ix < ixxmin)ixxmin=ix;
			//if(ix > ixxmax)ixxmax=ix;
			iy=(int)((y-ymin)*idy/dy+iymax);
			if(iy <= iymin || iy >= iymax)continue;
			//if(iy < iyymin)iyymin=iy;
			//if(iy > iyymax)iyymax=iy;
			if(iflag == 0){
			  ixold=ix;
			  iyold=iy;
			  DrawLine(ixold, iyold, ix, iy);
			  iflag=1;
			}
			DrawLine(ixold, iyold, ix, iy);
			ixold=ix;
			iyold=iy;
		}
	
		
		//fprintf(stderr,"ixxmin %d ixxmax %d getWidth() %d iyymin %d iyymax %d getHeight() %d\n",ixxmin,ixxmax,getWidth(),iyymin,iyymax,getHeight());

		glColor4f(0, 0, 0, 1);

		
 		{
			double xmnc,xmxc,Large,Small;
			double xmnc2,xmxc2;
			double xmns,xmxs;
			double cmin,cmax;
			double fc=sdr->fw;
			double bw=sdr->samplewidth/(2.0);
			xmnc2=fc-bw;
			xmxc2=fc+bw;
			cmin= -((sdr->fc-xmnc2)/sdr->samplerate)+0.5;
			xmnc=cmin*1.0/(double)sdr->ncut;
			cmax= -((sdr->fc-xmxc2)/sdr->samplerate)+0.5;
			xmxc=cmax*1.0/(double)sdr->ncut;
			xmns=xmnc;
			xmxs=xmxc;
			//fprintf(stderr,"xmnc %g xmxc %g samplewidth %g samplescale %g\n",xmnc,xmxc,sdr->samplewidth,sdr->samplescale);
			GridPlotNeat2(&xmnc,&xmxc,&Large,&Small);
			//fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
			
			dx=xmxs-xmns;
			
			for(double xp=xmnc;xp <= xmxc;xp += Large){
				char cbuff[256];
			    double xx=(xp-xmns)/dx;
			    //fprintf(stderr,"xx %g ",xx);
			    if(xx < 0.0 || xx > 1.0)continue;
			    int ixx=(int)(idx*xx+ixmin);
			    //fprintf(stderr,"ixx %d\n ",ixx);
			    if(ixx < ixmin || ixx > ixmax)continue;
 				DrawLine3(ixx, 0, ixx, getHeight()-15);
 				sprintf(cbuff,"%g",xp);
				DrawString(ixx-10,getHeight()-13, cbuff);
			}
			//winout(" idx %g\n",idx);
			

			xmnc=ymin;
			xmxc=ymax;
			//fprintf(stderr,"xmnc %g xmxc %g\n",xmnc,xmxc);
			GridPlotNeat2(&xmnc,&xmxc,&Large,&Small);
			//fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
			
			for(double xp=xmnc;xp <= xmxc;xp += Large){
				char cbuff[256];
			    double xx=((xp-ymin)/(dy));
			    if(xx < 0.0 || xx > 1.0)continue;
			    int ixx=(int)(iymax+idy*xx);
 				DrawLine3(30, ixx, getWidth(), ixx);
 				sprintf(cbuff,"%g",xp);
				DrawString(5,ixx-8,cbuff);
			}
			//winout(" idy %g\n",idy);
		

		
 		}
		
    	glFlush();
    	SwapBuffers();
    	    
		//auto t2 = chrono::high_resolution_clock::now();
		//std::chrono::duration<double> difference = t2 - t1;
		//std::cout << "Time "<< difference.count() << endl;
		//winout("count %g\n",difference.count());

	}
	
	//static long int count1;

	//winout("Spectrum render 1 count %ld amax %g iWait %d\n",count1++,amax,iWait);

	//fprintf(stderr,"Next 77\n");
		
    
 
}
 
void Spectrum::render2(wxPaintEvent& evt )
{
	evt.Skip();
	
	//static long long nc=0;
/*
	static int tick=0;
	tick++;
	tick=10;
*/
	//winout("Spectrum render nc %lld\n",nc++);

    if(!IsShown()) return;
    
    //auto t1 = chrono::high_resolution_clock::now();
    
        
    wxGLCanvas::SetCurrent(*m_context);
    //wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
    wxClientDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    // ------------- draw some 2D ----------------
    prepare2DViewport(0,0,getWidth(), getHeight());
    glLoadIdentity();
 
    // white background
    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);
    glVertex3f(0,0,0);
    glVertex3f(getWidth(),0,0);
    glVertex3f(getWidth(),getHeight(),0);
    glVertex3f(0,getHeight(),0);
    glEnd();
 
 	//double	value=sdr->samplewidth/sdr->samplerate;

 	if(buffSendLength && !iWait){
  		uRect box;
	
 		box.x=ftox(sdr->f-sdr->bw/(2.0));
 		box.y=0;
 		box.xsize=ftox(sdr->f+sdr->bw/(2.0))-ftox(sdr->f-sdr->bw/(2.0));
 		box.ysize=getHeight();
 		
 		//winout(" left %d right %d center %d xsize %d bw %g\n",ftox(sdr->f-sdr->bw/2.0),ftox(sdr->f+sdr->bw/2.0),ftox(sdr->f),box.xsize,sdr->bw);
 		
 		DrawBox(&box,0);
 		
		glColor4f(0, 0, 1, 1);
		
		//winout("filterType %d\n",filterType);
		
		doWindow(buffSend2,buffSendLength,filterType);
		
        for(int n=0;n<buffSendLength;++n){
            buffSend2[2*n] *= pow(-1.0,n);
            buffSend2[2*n+1] *= pow(-1.0,n);
        }

		
		//fft(buffSend2,buffSendLength,-1);
		

		fftwf_execute(p1);
		
		double avg=0;
		for(int n=0;n<buffSendLength;++n){
			double v=(buffSend[2*n]*buffSend[2*n]+buffSend[2*n+1]*buffSend[2*n+1]);
        	if(v > 0.0)v=10*log10(v)+5;
        	double mag= (1.0-lineAlpha)*buffSend10[n]+lineAlpha*v;
			avg += mag;
		}
		
		avg /= buffSendLength;
	
		double shift=-100-avg;
		
		
		buffFlag=0;
		
		double dnom=100000000;

		double ymin = verticalMinimum;
		double ymax = verticalMaximum;	
		if(ymin >= ymax)ymin=ymax-40;
		
		double dy=ymax-ymin;
		
		double iymin=0;
		double iymax=getHeight();
		double idy=iymin-iymax;		
		
		double xmin=sdr->fw-0.5*sdr->samplewidth;
		double xmax=sdr->fw+0.5*sdr->samplewidth;
		double dx=xmax-xmin;

		double ixmin=0;
		double ixmax=getWidth();
		double idx=ixmax-ixmin;

		//int ixxmin,ixxmax,iyymin,iyymax;
	
		//ixxmin=100000000;
		//ixxmax= -100000000;
		//iyymin=100000000;
		//iyymax= -100000000;
	
		int ixold=0;
		int iyold=0;
		int iflag=0;
	
		double xmin2=sdr->fc-0.5*sdr->samplerate;
		double xmax2=sdr->fc+0.5*sdr->samplerate;
		double dx2=xmax2-xmin2;
		
		
		//winout("buffSendLength %ld lineAlpha %g\n",(long)buffSendLength,lineAlpha);
			
		for(int n=0;n<buffSendLength;++n){
			double sum=0;
			double v=0;
			if(softAutoGain == 1){
				v=(buffSend[2*n]*buffSend[2*n]+buffSend[2*n+1]*buffSend[2*n+1]);
        		if(v > 0.0)v=10*log10(v)+5;
				sum=(1.0-lineAlpha)*buffSend10[n]+lineAlpha*v+shift;
			}else{
				v=(buffSend[2*n]*buffSend[2*n]+buffSend[2*n+1]*buffSend[2*n+1])/dnom;
        		if(v > 0.0)v=10*log10(v);
				sum=(1.0-lineAlpha)*buffSend10[n]+lineAlpha*v;
			}
			
			buffSend10[n]=sum;
			double x=dx2*n/((double)buffSendLength)+xmin2;
			double y=sum;
			int ix;
			int iy;
			ix=(int)((x-xmin)*idx/dx+ixmin);
			if(ix <= ixmin || ix >= ixmax)continue;
			//if(ix < ixxmin)ixxmin=ix;
			//if(ix > ixxmax)ixxmax=ix;
			iy=(int)((y-ymin)*idy/dy+iymax);
			if(iy <= iymin || iy >= iymax)continue;
			//if(iy < iyymin)iyymin=iy;
			//if(iy > iyymax)iyymax=iy;
			if(iflag == 0){
			  ixold=ix;
			  iyold=iy;
			  DrawLine(ixold, iyold, ix, iy);
			  iflag=1;
			}
			DrawLine(ixold, iyold, ix, iy);
			ixold=ix;
			iyold=iy;
		}
		
		
		glColor4f(0, 0, 0, 1);

		
 		{
			double xmnc,xmxc,Large,Small;
			double fc=sdr->fw;
			double bw=sdr->samplewidth/(2.0);
			xmnc=fc-bw;
			xmxc=fc+bw;
			//winout("xmnc %g xmxc %g value %g samplewidth %g\n",xmnc,xmxc,value,sdr->samplewidth);
			GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
			//winout("xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
			
			for(double xp=xmnc;xp <= xmxc;xp += Large){
				char cbuff[256];
			    double xx=((xp-(fc-bw))/(2.0*bw));
			    if(xx < 0.0 || xx > 1.0)continue;
			    int ixx=(int)(idx*xx);
			    if(ixx < ixmin || ixx > ixmax)continue;
 				DrawLine3(ixx, 0, ixx, getHeight()-15);
 				sprintf(cbuff,"%g",xp/1e6);
				DrawString(ixx-10,getHeight()-13, cbuff);
			}
			//winout(" idx %g\n",idx);
			
			
			xmnc=ymin;
			xmxc=ymax;
			//winout("xmnc %g xmxc %g\n",xmnc,xmxc);
			GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
			//winout("xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
			
			for(double xp=xmnc;xp <= xmxc;xp += Large){
				char cbuff[256];
			    double xx=((xp-ymin)/(dy));
			    if(xx < 0.0 || xx > 1.0)continue;
			    int ixx=(int)(iymax+idy*xx);
 				DrawLine3(30, ixx, getWidth(), ixx);
 				sprintf(cbuff,"%g",xp);
				DrawString(5,ixx-8,cbuff);
			}
			//winout(" idy %g\n",idy);
    		glFlush();
    		SwapBuffers();
		
 		}
		
		
/*		
		if(tick % 40 == 0){
			double dt=1.0;
			winout("plot %d signal\n",buffSendLength);
			for(int n=0;n<buffSendLength;++n){
			//	winout("%f %f\n",n*dt,(buffSend[2*n]*buffSend[2*n]+buffSend[2*n+1]*buffSend[2*n+1]));
				winout("%f %f\n",n*dt,buffSend10[n]);
			}
		}
*/

		
		//winout("ixxmin %d ixxmax %d getWidth() %d iyymin %d iyymax %d getHeight() %d\n",ixxmin,ixxmax,getWidth(),iyymin,iyymax,getHeight());

		//auto t2 = chrono::high_resolution_clock::now();
		//std::chrono::duration<double> difference = t2 - t1;
		//std::cout << "Time2 "<< difference.count() << endl;
		//winout("count %g\n",difference.count());

	}

	//winout("Spectrum done\n");
		
}

void Spectrum::render1a(wxPaintEvent& evt )
{
	evt.Skip();
	
	//winout("Spectrum render nc %lld\n",nc++);

    if(!IsShown()) return;
    
    //auto t1 = chrono::high_resolution_clock::now();
    
        
    wxGLCanvas::SetCurrent(*m_context);
    //wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
    wxClientDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
    // ------------- draw some 2D ----------------
    prepare2DViewport(0,0,getWidth(), getHeight());
    glLoadIdentity();
 
    // white background
    glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);
    glVertex3f(0,0,0);
    glVertex3f(getWidth(),0,0);
    glVertex3f(getWidth(),getHeight(),0);
    glVertex3f(0,getHeight(),0);
    glEnd();
    
    //winout("render1a\n");
    
    int buffSendLength=0;
    
    float *buffSend2;
    
	if(!iWait){
	/*
  		uRect box;
	
 		box.x=ftox(sdr->f-sdr->bw/(2.0));
 		box.y=0;
 		box.xsize=ftox(sdr->f+sdr->bw/(2.0))-ftox(sdr->f-sdr->bw/(2.0));
 		box.ysize=getHeight();
 		 		
 		DrawBox(&box,0);
 	*/
 	
		glColor4f(0, 0, 1, 1);
		
		//winout("filterType %d\n",filterType);
		
		int ip = -1;
 	  	if(sdr->bS2)ip=sdr->bS2->popBuff();
      	if(ip < 0)return;
      	
      	int witch=ip % NUM_DATA_BUFF;
		buffSend2=sdr->bS2->buff[witch];
      	
      	buffSendLength=sdr->size;
		
		double amax=0;
		for(int n=0;n<buffSendLength;++n){
			double v=(buffSend2[2*n]*buffSend2[2*n]+buffSend2[2*n+1]*buffSend2[2*n+1]);
        	if(v > 0.0)v=sqrt(v);
         	if(v > amax)amax=v;
		}
		
			
		buffFlag=0;
		
		double ymin = -amax;
		double ymax =  amax;	
		if(ymin >= ymax)ymin=ymax-40;
		
		double dy=ymax-ymin;
		
		double iymin=0;
		double iymax=getHeight()-20;
		double idy=iymin-iymax;		
		
		double xmin=sdr->fw-0.5*sdr->samplewidth;
		double xmax=sdr->fw+0.5*sdr->samplewidth;
		double dx=xmax-xmin;

		double ixmin=50;
		double ixmax=getWidth();
		double idx=ixmax-ixmin;

		//int ixxmin,ixxmax,iyymin,iyymax;
	
		//ixxmin=100000000;
		//ixxmax= -100000000;
		//iyymin=100000000;
		//iyymax= -100000000;
	
		int ixold=0;
		int iyold=0;
		int iflag=0;
	
		double xmin2=sdr->fc-0.5*sdr->samplerate;
		double xmax2=sdr->fc+0.5*sdr->samplerate;
		double dx2=xmax2-xmin2;
		
		//fprintf(stderr,"buffSendLength %ld lineAlpha %g\n",(long)buffSendLength,lineAlpha);
			
		for(int n=0;n<buffSendLength;++n){
			double v;
			v=buffSend2[2*n];
			double x=dx2*n/((double)buffSendLength)+xmin2;
			double y=v;
			int ix;
			int iy;
			ix=(int)((x-xmin)*idx/dx+ixmin);
			if(ix <= ixmin || ix >= ixmax)continue;
			//if(ix < ixxmin)ixxmin=ix;
			//if(ix > ixxmax)ixxmax=ix;
			iy=(int)((y-ymin)*idy/dy+iymax);
			if(iy <= iymin || iy >= iymax)continue;
			//if(iy < iyymin)iyymin=iy;
			//if(iy > iyymax)iyymax=iy;
			if(iflag == 0){
			  ixold=ix;
			  iyold=iy;
			  DrawLine(ixold, iyold, ix, iy);
			  iflag=1;
			}
			DrawLine(ixold, iyold, ix, iy);
			ixold=ix;
			iyold=iy;
		}
	
		
		//fprintf(stderr,"ixxmin %d ixxmax %d getWidth() %d iyymin %d iyymax %d getHeight() %d\n",ixxmin,ixxmax,getWidth(),iyymin,iyymax,getHeight());

		glColor4f(0, 0, 0, 1);

		
 		{
			double xmnc,xmxc,Large,Small;
			double xmnc2,xmxc2;
			double xmns,xmxs;
			double cmin,cmax;
			double fc=sdr->fw;
			double bw=sdr->samplewidth/(2.0);
			xmnc2=fc-bw;
			xmxc2=fc+bw;
			cmin= -((sdr->fc-xmnc2)/sdr->samplerate)+0.5;
			xmnc=cmin*1.0/(double)sdr->ncut;
			cmax= -((sdr->fc-xmxc2)/sdr->samplerate)+0.5;
			xmxc=cmax*1.0/(double)sdr->ncut;
			xmns=xmnc;
			xmxs=xmxc;
			//fprintf(stderr,"xmnc %g xmxc %g samplewidth %g samplescale %g\n",xmnc,xmxc,sdr->samplewidth,sdr->samplescale);
			GridPlotNeat2(&xmnc,&xmxc,&Large,&Small);
			//fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
			
			dx=xmxs-xmns;
			
			for(double xp=xmnc;xp <= xmxc;xp += Large){
				char cbuff[256];
			    double xx=(xp-xmns)/dx;
			    //fprintf(stderr,"xx %g ",xx);
			    if(xx < 0.0 || xx > 1.0)continue;
			    int ixx=(int)(idx*xx+ixmin);
			    //fprintf(stderr,"ixx %d\n ",ixx);
			    if(ixx < ixmin || ixx > ixmax)continue;
 				DrawLine3(ixx, 0, ixx, getHeight()-15);
 				sprintf(cbuff,"%g",xp);
				DrawString(ixx-10,getHeight()-13, cbuff);
			}
			//winout(" idx %g\n",idx);
			

			xmnc=ymin;
			xmxc=ymax;
			//fprintf(stderr,"xmnc %g xmxc %g\n",xmnc,xmxc);
			GridPlotNeat2(&xmnc,&xmxc,&Large,&Small);
			//fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
			
			for(double xp=xmnc;xp <= xmxc;xp += Large){
				char cbuff[256];
			    double xx=((xp-ymin)/(dy));
			    if(xx < 0.0 || xx > 1.0)continue;
			    int ixx=(int)(iymax+idy*xx);
 				DrawLine3(30, ixx, getWidth(), ixx);
 				sprintf(cbuff,"%g",xp);
				DrawString(5,ixx-8,cbuff);
			}
			//winout(" idy %g\n",idy);
		

		
 		}
		
		

		//auto t2 = chrono::high_resolution_clock::now();
		//std::chrono::duration<double> difference = t2 - t1;
		//std::cout << "Time "<< difference.count() << endl;
		//winout("count %g\n",difference.count());

	}

	//winout("Spectrum done\n");

	//fprintf(stderr,"Next 77\n");
		
    glFlush();
    SwapBuffers();    
    
 
}




