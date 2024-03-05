#include "iqSDR.h"
#include "sound.h"
#include <string.h>
#include <vector>
#include <string>
#include <wx/cmdline.h>
#include <wx/event.h>

int copyl(char *p1,char *p2,long n);

//c++ -std=c++11 -o iqSDR.x iqSDR.cpp -lGLEW `/usr/local/bin/wx-config --cxxflags --libs --gl-libs` -lGLU -lGL

//c++ -std=c++11 -o iqSDR.x iqSDR.cpp -lGLEW `/usr/bin/wx-config --cxxflags --libs --gl-libs` -lGLU -lGL

//c++ -std=c++11 -o iqSDR.x iqSDR.cpp sdr.cpp cMalloc.cpp mThread.cpp -lGLEW  -lfftw3 -lrtaudio -lSoapySDR -lliquid `/opt/local/Library/Frameworks/wxWidgets.framework/Versions/wxWidgets/3.1/bin/wx-config --cxxflags --libs --gl-libs` -Wno-deprecated-declarations -Wno-return-type-c-linkage

/*
void Buttons::OnMenuFileOpen( wxCommandEvent& WXUNUSED(event) )
{
    wxString filename = wxFileSelector("Choose File", "", "", "",
        "Audio File All files (*.*)|*.*",
        wxFD_OPEN);
    if (!filename.IsEmpty())
    {
    	//OpenFile(filename);
    }
}
*/

#define FILTER_RECTANGULAR     0
#define FILTER_HANN            1
#define FILTER_HAMMING         2
#define FILTER_FLATTOP         3
#define FILTER_BLACKMANHARRIS  4
#define FILTER_BLACKMANHARRIS7 5

static void GridPlotNeat(double *xmnc,double *xmxc,double *Large,double *Small);

int DrawLine3(int x1, int y1, int x2, int y2);

int DrawString(int x, int y, char *out);

int testEM();

int getPalette(int n,char *name,double *pal);

int getPaletteNames(std::vector<std::string>&names);	

int doWindow(float *x,long length,int type);

void *cMalloc(unsigned long r, int tag);

void checkall(void);

char WarningBuff[256];

GLuint vboId = 0;                   // ID of VBO for vertex arrays

int DrawBox(uRect *box,int offset);

startWindow *startIt;
soundClass *s;

std::vector<applFrame *> grabList;

BasicPane *pBasicPane;
Spectrum *pSpectrum;
WaterFall *pWaterFall;
TopPane *pTopPane;

int sendAudio(int short *data,int length)
{
	return pBasicPane->sendAudio(data,length);
}

int DrawLine(int x1, int y1, int x2, int y2)
{
    glBegin(GL_LINES);
    
    glVertex2f((x1),(y1));
    glVertex2f((x2),(y2));
    
    glEnd();
    
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
    	//fprintf(stderr,"s->soundRun  %d\n",s->soundRun);
   		 Sleep2(5);
    }
    
    

   
    // testEM();
        
 	wxFrame *frame2 = new wxFrame(NULL,wxID_ANY,wxT("iqSDR.x"));
	startIt=new startWindow(frame2, "Controls");
	frame2->SetSize(wxDefaultCoord,wxDefaultCoord,155,270);
	frame2->Show();
	
	//fprintf(stderr,"startIt %p\n",startIt);
	
	startIt->openArgcArgv(argc,argv);
	
    return true;
} 

IMPLEMENT_APP(MyApp)

BEGIN_EVENT_TABLE(startWaveFile, wxWindow)
EVT_LEFT_DOWN(startWaveFile::mouseDown)
EVT_SLIDER(SCROLL_TIME2,startWaveFile::OnScroll)
EVT_SLIDER(SCROLL_GAIN2,startWaveFile::OnScroll)
EVT_CHECKBOX(ID_PAUSE, startWaveFile::Pause)
EVT_TIMER(TIMER_ID2,startWaveFile::OnIdle)
END_EVENT_TABLE()

void startWaveFile::Pause(wxCommandEvent& event) 
{    
	event.Skip();

   //int flag=event.GetValue();
       
    iPause=pbox->GetValue();

//   fprintf(stderr,"Pause iPause %d\n",iPause);
}
void startWaveFile::OnIdle(wxTimerEvent &event)
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


void startWaveFile::OnScroll(wxCommandEvent &event)
{
	event.Skip();
	
	float value=event.GetSelection();

	if(event.GetId() == SCROLL_GAIN2){
		//printf("OnScroll value %g %d \n",value,event.GetId());
		gain=0.01*value;
		s->gain=gain;
		return;
	}else if(event.GetId() == SCROLL_TIME2){
		// printf("OnScroll value %g %d \n",value,event.GetId());
		sf_count_t ret=sf_seek(infile, value*sfinfo.samplerate, SEEK_SET);
        if(ret < 0)printf("sf_seek error\n");
		CurrentFrame=value*sfinfo.samplerate;
		return;
	}
	
	
}

startWaveFile::startWaveFile(wxWindow *frame,const wxString& title)
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
		//fprintf(stderr,"Wait soundRun %d\n",s->soundRun);
		Sleep2(10);
	}
    
	if(sfinfo.samplerate > 0){
		sliderTime->SetMax(sfinfo.frames/sfinfo.samplerate); 
	}else{
		sliderTime->SetMax(100); 
	}
    sliderTime->SetValue(0);

	s->faudio=sfinfo.samplerate;
    
//	fprintf(stderr,"sfinfo.samplerate %d sfinfo.channels %d sfinfo.frames %lld\n",sfinfo.samplerate,sfinfo.channels,sfinfo.frames);

	bS=new cStack;

	bS->setBuff(2000000,sfinfo.samplerate);

	nReadBlock=(int)(sfinfo.samplerate/40.0);

	s->bS=bS;

	wavFlag=2;

	std::thread(&startWaveFile::wavBuffer,this).detach();
	
	std::thread(&soundClass::startSound,s).detach();

	
}

int startWaveFile::wavBuffer()
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
		//fprintf(stderr,"audioOut %ld readcount %d\n",audioOut,readcount);
		if(s->bS == bS)s->audioSync=1;

	}

	wavFlag=1;
	
	return 0;
}

void startWaveFile::mouseDown(wxMouseEvent& event) 
{
	extern soundClass *s;
	event.Skip();
	s->gain=gain;
	s->bS=bS;
}

startWaveFile::~startWaveFile()
{
	extern soundClass *s;

	s->bS=NULL;
	
	wavFlag=0;
	
	while(wavFlag == 0)Sleep2(5);

	if(infile)sf_close(infile) ;
	infile=NULL;
	
	if(bS)delete bS;
	bS=NULL;
	
	//fprintf(stderr,"exit startWaveFile %p\n",this);

}

void startWindow::openArgcArgv(int argc,char **argv)
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
			fprintf(stderr,"pipe input\n");
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
			fprintf(stderr,"debug %d\n",debug);
		}else if(name == "-udp"){
			udp=1;
			//fprintf(stderr,"tcp_ip %d input\n",tcp_ip);
		}else if(name == "-tcp"){
			tcp_ip=1;
			//fprintf(stderr,"tcp_ip %d input\n",tcp_ip);
		}else if(name == "-cat"){
			cat=1;
			//fprintf(stderr,"tcp_ip %d input\n",tcp_ip);
		}else if(name == "-fc"){		
			fc=atof(argv[++n]);	
		}else if(name == "-samplerate"){		
			samplerate=atof(argv[++n]);	
	    }else if(name == "-outFile"){
	    	name=argv[++n];
	    	if(name == "-"){
	    		outFile=stdout;
	    		fprintf(stderr,"outFile=stdout\n");
	    	}else{
				 outFile=fopen(argv[n],"wb");
				 if(outFile == NULL){
					 fprintf(stderr,"Could Not Open %s to Write\n",argv[n]);
				 }
	         }
	    }else if(name == "-inFile"){
	         inFile=fopen(argv[++n],"rb");
	         if(inFile == NULL){
	             fprintf(stderr,"Could Not Open %s to Write\n",argv[n]);
	         }
		}
	
	}
	
	fc *= 1e6;
		
	//fprintf(stderr,"fc %g MHZ samplerate %g\n",fc/1e6,samplerate);
	
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
	    fprintf(stderr,"Could Not Open %s to Read\n",file);
	    return;
	}
*/


	sdrIn->startPlay();

	applFrame *grab=new applFrame(NULL,name,sdrIn);
	grab->Show();

	grabList.push_back(grab);

	gBasicPane=pBasicPane;

	grab->SetLabel(name);

	gBasicPane->gSpectrum->iWait=1;

	std::thread(&Spectrum::startRadio2,gBasicPane->gSpectrum).detach();   	

	Sleep2(50);

	gBasicPane->gSpectrum->iWait=0;
	
	//fprintf(stderr,"samplerate %g samplewidth %g\n",sdrIn->samplerate,sdrIn->samplewidth);
	
}

startWindow::startWindow(wxWindow *frame, const wxString &title)
    : wxWindow(frame,32000)
{
	 wxStaticBox *box = new wxStaticBox(this, wxID_ANY, "&Start Options",wxPoint(10,10), wxSize(135, 150),wxBORDER_SUNKEN );
	 box->SetToolTip(wxT("This is tool tip") );

  	new wxButton(box,ID_ABOUT,wxT("About"),wxPoint(20,10));
  	
  	new wxButton(box,ID_RADIO,wxT("Radio"),wxPoint(20,40));
  	
    new wxButton(box,ID_FILE,wxT("File"),wxPoint(20,70));
   	
    new wxButton (box,ID_QUIT,wxT("Quit"),wxPoint(20,100));
      
    wxStaticBox *box2 = new wxStaticBox(this, wxID_ANY, "&Device String",wxPoint(10,170), wxSize(130, 60),wxBORDER_SUNKEN );
	box2->SetToolTip(wxT("This is tool tip") );

    textDevice=new wxTextCtrl(box2,ID_TEXTCTRL,wxT(""),
          wxPoint(5,10), wxSize(120, 30));
          
    grab=NULL;

   //   wxString value=text->GetValue();

}

startWindow::~startWindow()
{
	//fprintf(stderr,"exit startWindow %p\n",this);
}
BEGIN_EVENT_TABLE(startWindow, wxWindow)
EVT_BUTTON(ID_ABOUT, startWindow::OnAbout)
EVT_BUTTON(ID_RADIO, startWindow::OnRadio)
EVT_BUTTON(ID_QUIT, startWindow::OnQuit)
EVT_BUTTON(ID_FILE, startWindow::OnFile)
//EVT_BUTTON(ID_TEST, startWindow::OnTest)
END_EVENT_TABLE()

void startWindow::openWindows(char *name)
{
	wxString title=name;
		
	grab=new applFrame(NULL,name,NULL);
	grab->Show();
	
	grabList.push_back(grab);
	
}
#define POS(r, c)        wxGBPosition(r,c)
#define SPAN(r, c)       wxGBSpan(r,c)


wxBEGIN_EVENT_TABLE(applFrame, wxFrame)
EVT_MENU_RANGE(INPUT_MENU,INPUT_MENU+99,applFrame::OnInputSelect)
EVT_MENU_RANGE(OUTPUT_MENU,OUTPUT_MENU+99,applFrame::OnOuputSelect)
EVT_MENU_RANGE(ID_PALETTE,ID_PALETTE+99,applFrame::OnPaletteSelected)
EVT_MENU_RANGE(ID_OPTIONS,ID_OPTIONS+99,applFrame::OnOptionsSelected)
EVT_MENU_RANGE(ID_DIRECT,ID_DIRECT+99,applFrame::OnDirectSelected)
EVT_MENU_RANGE(ID_BAND,ID_BAND+99,applFrame::OnBandSelected)
EVT_MENU_RANGE(ID_SAMPLERATE,ID_SAMPLERATE+99,applFrame::OnSampleRateSelected)
EVT_SIZE(applFrame::resized)
wxEND_EVENT_TABLE()
void applFrame::resized(wxSizeEvent& evt)
{
 evt.Skip();

//const wxSize size = evt.GetSize() * GetContentScaleFactor();

//fprintf(stderr,"applFrame::resized %d %d %f\n", size.x,size.y,GetContentScaleFactor());
	Refresh();

}


applFrame::applFrame(wxFrame* parent,wxString title,class sdrClass *sdrIn)
    : wxFrame(parent, wxID_ANY, "wxGridBagSizer Test Frame")
{
	sdr=sdrIn;
    wxMenu *menuFile = new wxMenu(_T(""), wxMENU_TEAROFF);
    //menuFile->Append(wxID_OPEN, _T("&Information Audio...\tCtrl-O"), _T("Infomation"));
    
    menuFile->Append(wxID_EXIT, _T("E&xit\tAlt-X"), _T("Quit this program"));
    
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, _T("&About...\tCtrl-A"), _T("Show about dialog"));
	
	wxMenu *inputMenu = new wxMenu;
	//fprintf(stderr,"s->inputNames.size() %lld\n",(long long)s->inputNames.size());
    for(unsigned long int n=0;n<s->inputNames.size();++n){
        wxMenu *subMenu = new wxMenu;
        inputMenu->AppendSubMenu(subMenu, s->inputNames[n].name, wxT("Description?"));
       // fprintf(stderr,"s->inputNames[n].name %s\n",s->inputNames[n].name.c_str());
       //int deviceID=inputNames[n].deviceID;
        for(unsigned long int k=0;k<s->inputNames[n].sampleRate.size();++k){
            std::stringstream srateName;
            srateName << ((float) (s->inputNames[n].sampleRate[k]) / 1000.0f) << "kHz";
            itm = subMenu->AppendRadioItem(INPUT_MENU + 10*n + k, srateName.str(), wxT("Description?"));
        }
    }
   
    wxMenu *outputMenu = new wxMenu;
 	//fprintf(stderr,"s->outputNames.size() %lld\n",(long long)s->outputNames.size());
	for(unsigned long int n=0;n<s->outputNames.size();++n){
        wxMenu *subMenu = new wxMenu;
    	outputMenu->AppendSubMenu(subMenu, s->outputNames[n].name, wxT("Description?"));
  //  	fprintf(stderr,"s->outputNames[n].name %s\n",s->outputNames[n].name.c_str());
       // int deviceID=s->outputNames[n].deviceID;
        for(unsigned long int k=0;k<s->outputNames[n].sampleRate.size();++k){
            std::stringstream srateName;
            srateName << ((float) (s->outputNames[n].sampleRate[k]) / 1000.0f) << "kHz";
            itm = subMenu->AppendRadioItem(OUTPUT_MENU + 10*n + k, srateName.str(), wxT("Description?"));
        }
  	}
  	
	
	std::vector<std::string> names;
	
	getPaletteNames(names);
	
	//fprintf(stderr,"names.size() %ld\n",(long)names.size());
	
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
//		fprintf(stderr,"flags.size() %ld\n",(long)flags.size());
		if(flags.size()){
			int count=0;
			for(size_t k=0;k<flags.size();++k){
			// fprintf(stderr,"k %d %s %s type %d\n",(int)k,flags[k].key.c_str(),flags[k].value.c_str(),(int)flags[k].type);
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

void applFrame::OnSampleRateSelected(wxCommandEvent& event)
{
	event.Skip();

	//int item=event.GetId()-ID_SAMPLERATE;
	
	wxString whatEvent=sampleRateMenu->GetLabelText(event.GetId());
	
	const char *what=whatEvent.c_str();

   	//printf("OnBandSelected %d ID_BAND %d what %s\n",item,ID_BAND,what);
   	
   	for(int n=0;n<(int)sdr->rate.size();++n){
		sampleRateMenu->Check(ID_SAMPLERATE+n,0);
	}
	
	sampleRateMenu->Check(event.GetId(),1);

   
   	sdr->waitPointer("iqToAudio(8)",&sdr->iqToAudio,0);

	sdr->waitPointer("frame(8)",&sdr->frame,0);

	sdr->waitPointer("doWhat(8)",&sdr->doWhat,0);
	
	sdr->samplerate=atof(what);
	
	sdr->samplewidth=sdr->samplerate;
	
	sdr->startPlay();
	
	std::thread(&sdrClass::run,sdr).detach();



}

void applFrame::OnBandSelected(wxCommandEvent& event)
{
	event.Skip();

	//int item=event.GetId()-ID_BAND;
	
	wxString whatEvent=bandMenu->GetLabelText(event.GetId());
	
	const char *what=whatEvent.c_str();

   	//printf("OnBandSelected %d ID_BAND %d what %s\n",item,ID_BAND,what);
   	
   	for(int n=0;n<(int)sdr->band.size();++n){
		bandMenu->Check(ID_BAND+n,0);
	}
	
	bandMenu->Check(event.GetId(),1);

   
   	sdr->waitPointer("iqToAudio(8)",&sdr->iqToAudio,0);

	sdr->waitPointer("frame(8)",&sdr->frame,0);

	sdr->waitPointer("doWhat(8)",&sdr->doWhat,0);
	
	sdr->bandwidth=atof(what);
	
	sdr->startPlay();
	
	std::thread(&sdrClass::run,sdr).detach();



}
void applFrame::OnOptionsSelected(wxCommandEvent& event){
	event.Skip();

	//int item=event.GetId()-ID_OPTIONS;
		
	wxString whatEvent=actionMenu->GetLabelText(event.GetId());
	
	const char *what=whatEvent.c_str();

   // printf("OnOptionsSelected %d INPUT_MENU %d checked %d what %s\n",item,ID_OPTIONS,actionMenu->IsChecked(event.GetId()),what);
    
    
    sdr->device->writeSetting(what,actionMenu->IsChecked(event.GetId()));
    
}

void applFrame::OnDirectSelected(wxCommandEvent& event){
	event.Skip();

	//int item=event.GetId()-ID_DIRECT;
	
	wxString whatEvent=directMenu->GetLabelText(event.GetId());
	
	const char *what=whatEvent.c_str();

	
  //  printf("OnDirectSelected %d ID_DIRECT %d what %s\n",item,ID_DIRECT,what);
    
    for(int n=0;n<3;++n){
		directMenu->Check(ID_DIRECT+n,0);
	}
	
	directMenu->Check(event.GetId(),1);
	
	sdr->device->writeSetting("direct_samp",what);

}

void applFrame::OnPaletteSelected(wxCommandEvent& event){
	event.Skip();

	int item=event.GetId()-ID_PALETTE;
//	printf("OnPaletteSelected %d INPUT_MENU %d\n",item,ID_PALETTE);
	
	char name[512];

	double pal[3*256];

	getPalette(item,name,pal);

	//printf("name %s\n",name);
	
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

void applFrame::OnInputSelect(wxCommandEvent& event){
	int item=event.GetId()-INPUT_MENU;
	//printf("OnInputSelect %d INPUT_MENU %d\n",item,INPUT_MENU);
	int n=item/10;
	s->inputID=s->inputNames[n].deviceID;
	int sound=item-10*n;
	//fprintf(stderr,"%s sampleRate %d\n",s->inputNames[n].name.c_str(),s->inputNames[n].sampleRate[sound]);
	SampleFrequency=s->inputNames[n].sampleRate[sound];
	//startAudio();
}

void applFrame::OnOuputSelect(wxCommandEvent& event){
	int item=event.GetId()-OUTPUT_MENU;
	//printf("OnOuputSelect %d OUTPUT_MENU %d\n",item,OUTPUT_MENU);
	int n=item/10;
	s->outputID=s->outputNames[n].deviceID;
	int sound=item-10*n;
	//fprintf(stderr,"%s sampleRate %d\n",s->outputNames[n].name.c_str(),s->outputNames[n].sampleRate[sound]);
	s->faudio=s->outputNames[n].sampleRate[sound];
	s->soundRun = -1;
	while(s->soundRun == -1){
		//fprintf(stderr,"Wait soundRun %d\n",s->soundRun);
		Sleep2(10);
	}
	std::thread(&soundClass::startSound,s).detach();
	
   	sdr->waitPointer("iqToAudio(8)",&sdr->iqToAudio,0);

	sdr->waitPointer("frame(8)",&sdr->frame,0);

	sdr->waitPointer("doWhat(8)",&sdr->doWhat,0);
		
	sdr->faudio=s->faudio;
	
	sdr->startPlay();
			
	std::thread(&sdrClass::run,sdr).detach();

}

applFrame::~applFrame()
{
//	fprintf(stderr,"applFrame::~applFrame\n");
	
	if(grabList.size() > 0){
		for(std::vector<applFrame *>::size_type k=0;k<grabList.size();++k){
			applFrame *grab=grabList[k];
			if(grab == this){
			    grabList[k]=NULL;
			    //fprintf(stderr,"applFrame remove from list\n");
			}
		}
	}

	checkall();
	
	//fprintf(stderr,"exit applFrame %p\n",this);

}

void startWindow::OnAbout(wxCommandEvent& event)
{
	event.Skip();

	fprintf(stderr,"OnAbout\n");
}
void startWindow::OnRadio(wxCommandEvent& event)
{	
	event.Skip();

	wxFrame *frame= new wxFrame(NULL, wxID_ANY, wxT("Device Select"), wxDefaultPosition, wxSize(400,400));
		
	new selectionWindow(frame, "Device",textDevice);
	
	frame->Show();
	
	wxWindow::SetSize(wxDefaultCoord,wxDefaultCoord,400,400);
	
}
void startWindow::OnFile(wxCommandEvent& event)
{
	event.Skip();

	OpenFile();
}
void startWindow::OnQuit(wxCommandEvent& event)
{

	if(grabList.size() > 0){
		for(std::vector<applFrame *>::size_type k=0;k<grabList.size();++k){
			grab=grabList[k];
			if(grab)delete grab;
		}
	}

	wxWindow *parent=GetParent();
	
	parent->Destroy();
}
void startWindow::openWavFile(const char *file)
{
	
 	wxFrame *frame2 = new wxFrame(NULL,wxID_ANY,file);
	new startWaveFile(frame2,file);
	frame2->SetSize(wxDefaultCoord,wxDefaultCoord,340,300);
	frame2->Show();
	
	
}
void startWindow::openIQFile(const char *file)
{
	char name[512];
	
	mstrncpy(name,(char *)file,sizeof(name));
	
	//fprintf(stderr,"openIQFile %s name %s\n",file,name);
	
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
	    fprintf(stderr,"Could Not Open %s to Read\n",file);
	    return;
	}
	sdrIn->startPlay();
	
	applFrame *grab=new applFrame(NULL,file,sdrIn);
	grab->Show();
	
	grabList.push_back(grab);
	
	gBasicPane=pBasicPane;
	
	grab->SetLabel(file);

     gBasicPane->gSpectrum->iWait=1;
	
     std::thread(&Spectrum::startRadio2,gBasicPane->gSpectrum).detach();   	
     
     Sleep2(50);
     
     gBasicPane->gSpectrum->iWait=0;

}

void startWindow::OpenFile()
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
        
        fprintf(stderr,"OpenFile file %s Index %d\n",file,Index);
        
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


selectionWindow::selectionWindow(wxWindow *frame, const wxString &title,wxTextCtrl *textDevice)
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
		fprintf(stderr,"Error: enumerate Found No Devices - Try Again !\n");
		return;
	}
	
	wxStaticBox *box2 = new wxStaticBox(this, wxID_ANY, "&Parameters",wxPoint(10,10), wxSize(220, 80),wxBORDER_SUNKEN );
	box2->SetToolTip(wxT("This is tool tip") );
	new wxStaticText(box2,wxID_STATIC,wxT("Frequency(MHZ)"),wxPoint(0,5), wxDefaultSize,wxALIGN_LEFT);
	text=new wxTextCtrl(box2,ID_TEXTCTRL,wxT("101.5"),wxPoint(110,5), wxSize(80, 25));
	new wxStaticText(box2,wxID_STATIC,wxT("Mode :"),wxPoint(0,30), wxDefaultSize,wxALIGN_LEFT);
	
	wxArrayString strings;
	
	strings.Add("AM");
	strings.Add("NAM");
	strings.Add("FM");
	strings.Add("NBFM");
	strings.Add("USB");
	strings.Add("LSB");
	strings.Add("CW");
	
	boxMode=new wxComboBox(box2,ID_COMBOMODE,wxT("Mode"),wxPoint(50,30),wxDefaultSize,
	                   strings,wxCB_DROPDOWN);
	boxMode->SetSelection(2);
	wxStaticBox *box = new wxStaticBox(this, wxID_ANY, "&Device",wxPoint(10,10+90), wxSize(300, 20+2*length*30),wxBORDER_SUNKEN );
	box->SetToolTip(wxT("This is tool tip") );

    int outset1=0;

    SoapySDR::Kwargs deviceArgs;
    
    deviceNames.clear();
        
     for(size_t k=0;k<length;++k){
        std::string name;
        
        name="";
        
 
        try {

            deviceArgs = results[k];
            for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
               // fprintf(stderr,"%s=%s\n",it->first.c_str(),it->second.c_str());
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
				outset1 += 30;
				strings.Clear();

                std::vector<double> rate=devicer->listSampleRates(SOAPY_SDR_RX,0);
                for (size_t j = 0; j < rate.size(); j++)
                {
                    char data[256];
                    unsigned long long irate=rate[j];
                    sprintf(data,"%llu",irate);
                    strings.Add(data);

                }
                
				new wxStaticText(box,wxID_STATIC,wxT("Sample Rate :"),wxPoint(20,outset1+8), wxDefaultSize,wxALIGN_LEFT);

				boxList[k]=new wxComboBox(box,ID_FREQUENCY+k,wxT("Mode"),wxPoint(110,outset1+5),wxDefaultSize,
				   strings,wxCB_DROPDOWN);
				boxList[k]->SetSelection(0);

				outset1 += 30;

				SoapySDR::Device::unmake(devicer); 
            }
            
        } catch(const std::exception &e) {
            std::string streamExceptionStr = e.what();
            fprintf(stderr,"doRadioOpen Error: %s\n",streamExceptionStr.c_str());
        }
   }
 
}
selectionWindow::~selectionWindow()
{
	//delete m_context;
	//fprintf(stderr,"selectionWindow::~selectionWindow\n");
	//fprintf(stderr,"exit selectionWindow %p\n",this);

}

BEGIN_EVENT_TABLE(selectionWindow, wxWindow)
EVT_COMMAND_RANGE(ID_DEVICE,ID_DEVICE+199,wxEVT_BUTTON,selectionWindow::OnDevice)
END_EVENT_TABLE()

void selectionWindow::killMe() 
{
		
	wxWindow *parent=GetParent();
	if(parent)parent->Destroy();
	
}
void selectionWindow::OnDevice(wxCommandEvent& event) 
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
	
	wxString name=deviceNames[id-ID_DEVICE];
	
	applFrame *grab=new applFrame(NULL,name,sdrIn);
	grab->Show();
	
	grabList.push_back(grab);
	
	gBasicPane=pBasicPane;
	
	grab->SetLabel(deviceNames[id-ID_DEVICE]);
	    	
    std::thread(&Spectrum::startRadio2,gBasicPane->gSpectrum).detach();
    
	Sleep2(50);

	gBasicPane->gSpectrum->iWait=0;
	gBasicPane->iRefresh=1;

	killMe();

//	fprintf(stderr,"OnDevice  11\n");
   
    
}

BEGIN_EVENT_TABLE(TopPane, wxWindow)
EVT_MOTION(TopPane::mouseMoved)
EVT_PAINT(TopPane::render)
EVT_LEFT_DOWN(TopPane::mouseDown)
EVT_CHECKBOX(ID_RECORD, TopPane::Record)
EVT_CHECKBOX(ID_FC, TopPane::Record)

END_EVENT_TABLE()

#include <time.h>       /* time_t, struct tm, difftime, time, mktime */

void TopPane::Record(wxCommandEvent& event) 
{    
	event.Skip();
	
	if(event.GetId() == ID_FC){
		idoFC=rbox2->GetValue();
		//fprintf(stderr,"ID_FC idoFC %d\n",idoFC);
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
	
	font= new wxFont(35,wxFONTFAMILY_MODERN,wxNORMAL,wxNORMAL);
	
	fontSize=font->GetPixelSize();

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
	//fprintf(stderr,"TopPane::~TopPane\n");
	//fprintf(stderr,"exit TopPane %p\n",this);

}


void TopPane::mouseDown(wxMouseEvent& event)
{
	event.Skip();

	wxPoint p = event.GetLogicalPosition(wxClientDC(this));
	int yh=fontSize.GetHeight()/2-10;
	int diff=p.x-pt.x;
	nchar=(diff)/fontSize.GetWidth();
	if(diff >= 0 && nchar >=0 && nchar <= 14){
		if(nchar == 3 || nchar == 7 || nchar == 11)return;
		int up=1;
		if(p.y > yh)up=0;
		//fprintf(stderr,"p.y %d up %d yh %d\n",p.y,up,yh);
		
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
		
//		fprintf(stderr,"TopPane::mouseDown f %lld k %d nchar %d up %d %f\n",fl,k,nchar,up,value);
		Refresh();

	}
}


void TopPane::mouseMoved(wxMouseEvent& event)
{
	event.Skip();

	wxPoint p = event.GetLogicalPosition(wxClientDC(this));
	
	nchar=(p.x-pt.x)/fontSize.GetWidth();

	//fprintf(stderr,"TopPane::mouseMoved diff %d character %d\n",p.x-pt.x,nchar);
	
	Refresh();
	
}

void TopPane::render( wxPaintEvent& evt )
{
	evt.Skip();
	
	wxPaintDC dc(this);
	
	dc.SetFont(*font);
	dc.SetBackgroundMode(wxTRANSPARENT);
	dc.SetTextForeground(*wxBLACK);
	dc.SetTextBackground(*wxWHITE);
	
	//fprintf(stderr,"TopPane f %g \n",sdr->f);

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
		wxPoint Rect[4];	
		wxColour *tGreen=new wxColour(0,255,0,64);
		dc.SetPen(wxPen(*wxWHITE));
		//dc.SetBrush(*wxGREEN);
		dc.SetBrush(*tGreen);
		Rect[0].x=0;
		Rect[0].y=0;
		Rect[1].x=0;
		Rect[1].y=fontSize.y;
		Rect[2].x=fontSize.x;
		Rect[2].y=fontSize.y;
		Rect[3].x=fontSize.x;
		Rect[3].y=0;
		dc.DrawPolygon(WXSIZEOF(Rect),Rect,	pt.x+nchar*fontSize.x,0);
	//	fprintf(stderr,"draw nchar %d\n",nchar);

	}

	
//	fprintf(stderr,"TopPane:render\n");
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
EVT_SLIDER(ID_MININUM,BasicPane::OnMinimun)
EVT_SLIDER(ID_MAXINUM,BasicPane::OnMaximun)
EVT_COMMAND_RANGE(ID_RXGAIN,ID_RXGAIN+99,wxEVT_SLIDER,BasicPane::setRxGain)
EVT_CHECKBOX(ID_CHECKAUTO,BasicPane::OnCheckAuto)
EVT_CHECKBOX(ID_SWAPIQ,BasicPane::OnCheckAuto)
EVT_CHECKBOX(ID_SOFTAUTOGAIN,BasicPane::OnCheckAuto)
EVT_CHECKBOX(ID_SETGAIN,BasicPane::OnCheckAuto)
EVT_BUTTON(ID_STARTSEND, BasicPane::startSend)
EVT_BUTTON(ID_STOPSEND, BasicPane::stopSend)
EVT_BUTTON(ID_ALPHA, BasicPane::stopSend)
EVT_LIST_ITEM_SELECTED(ID_RXFREQUENCY,BasicPane::OnListSelected)
END_EVENT_TABLE()
void dummpy11(){;}
void BasicPane::OnListSelected(wxListEvent& event) 
{    
	event.Skip();
	
	//int id=event.GetId();
	
    int index=event.GetIndex();
    
    wxString text=listFrequency->GetItemText(index,1);
    
	const char *freq=text;

	//fprintf(stderr,"OnListSelected id %d index %d freq %g\n",id,index,atof(freq));
	
	sdr->setFrequencyFC(atof(freq)*1e6);
	
	gTopPane->Refresh();
	
}
void BasicPane::startSend(wxCommandEvent& event) 
{    
	event.Skip();
	
	
	wxString value=sendAddress->GetValue();
	
	const char *Address=value;
	
	if(sendFlag){
		fprintf(stderr,"Send Already Running\n");
		return;
	}else{	
	    sendFlag=1;
		sdr->fillBuffer=1;
//		fprintf(stderr,"startSend Type %d Mode %d Address %s\n",sendTypeBox->GetSelection(),sendModeBox->GetSelection(),Address);	
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
		
		//fprintf(stderr,"UsePlotScales %d sPmin %g sPmax %g\n",gWaterFall->pd.UsePlotScales,gWaterFall->pd.sPmin,gWaterFall->pd.sPmax);
			
		return;
	}
	
	sendFlag=0;
	
	sdr->fillBuffer=0;
	
	//fprintf(stderr,"stopSend \n");

   // int id=event.GetId();
}

BasicPane::BasicPane(wxWindow *frame, const wxString &title,class sdrClass *sdrIn)
    : wxWindow(frame,32000)
    , m_timer(this,TIMER_ID)
{
	m_timer.Start(50);
	
	zerol((char *)rx,sizeof(rxs));
	
	sendFlag=0;
	
	lineAlpha=0.1;
	
	softAutoGain=1;
	
	sdr=sdrIn;
	
	pBasicPane=this;
	
	gTopPane=pTopPane;
	
	gTopPane->sdr=sdr;
	
	gSpectrum=pSpectrum;
	
	gSpectrum->sdr=sdr;
	
	gSpectrum->gTopPane=pTopPane;
	
	gWaterFall=pWaterFall;

	gWaterFall->sdr=sdr;
	
	gWaterFall->gTopPane=pTopPane;
	
	gWaterFall->gSpectrum=gSpectrum;
	
	ScrolledWindow = new wxScrolledWindow(this,wxID_ANY,wxPoint(0,0),
	 	                                   wxSize(340,200),wxVSCROLL ); // wxHSCROLL
 	int pixX=10;
 	int pixY=10;
 	int ux=200;
 	int uy=200;
 	ScrolledWindow->SetScrollbars(pixX,pixY,ux,uy);
 	
	int yloc=0;

	wxStaticBox *box = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Set Volume ",wxPoint(20,yloc), wxSize(230, 80),wxBORDER_SUNKEN );
	box->SetToolTip(wxT("This is tool tip") );

	new wxSlider(box,SCROLL_GAIN,100,0,100,wxPoint(10,0),wxSize(210,30),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);

	yloc += 85;   
	
	wxCheckBox *cbox;
	if(sdr->hasGainMode){
		cbox=new wxCheckBox(ScrolledWindow,ID_CHECKAUTO, "&Hardware AGC",wxPoint(20,yloc), wxSize(230, 25));
		cbox->SetValue(0);	
		yloc += 25;   
	}
	
	cbox=new wxCheckBox(ScrolledWindow,ID_SWAPIQ, "&I/Q Swap",wxPoint(20,yloc), wxSize(230, 25));
	cbox->SetValue(0);	
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

			new wxSlider(box,ID_RXGAIN+n,rmid,rxGainRange.minimum(),rxGainRange.maximum(),wxPoint(10,0),wxSize(210,-1),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);

			yloc += 85;   
		}
	
	}
	

	box = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Zoom",wxPoint(20,yloc), wxSize(230, 80),wxBORDER_SUNKEN );
	box->SetToolTip(wxT("This is tool tip") );

	new wxSlider(box,ID_SAMPLEWIDTH,100,0,100,wxPoint(10,0),wxSize(210,-1),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);

	yloc += 85;   

	box = new wxStaticBox(ScrolledWindow, wxID_ANY, "Minimun Value ",wxPoint(20,yloc), wxSize(230, 80),wxBORDER_SUNKEN );
	box->SetToolTip(wxT("This is tool tip") );

	new wxSlider(box,ID_MININUM,-130,-200,100,wxPoint(10,0),wxSize(210,-1),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);

	yloc += 85;

	box = new wxStaticBox(ScrolledWindow, wxID_ANY, "Maximun Value",wxPoint(20,yloc), wxSize(230, 80),wxBORDER_SUNKEN );

	new wxSlider(box,ID_MAXINUM,-30,-200,100,wxPoint(10,0),wxSize(210,-1),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);
  	
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
	new wxStaticText(panel2,wxID_STATIC,wxT("FFT size:"),wxPoint(0,12), wxDefaultSize,wxALIGN_LEFT);

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
	
	new wxStaticText(panel2,wxID_STATIC,wxT("Window:"),wxPoint(0,42), wxDefaultSize,wxALIGN_LEFT);

	filterCombo=new wxComboBox(panel2,ID_COMBOFILTER,wxT("FILTER"),wxPoint(55,40),wxDefaultSize,
	                   strings,wxCB_DROPDOWN);
	filterCombo->SetSelection(5);
	
//	fprintf(stderr,"Ant %p count %d\n",sdr->antenna,sdr->antennaCount);
	
	if(sdr->antennaNames.size() > 0){
 		strings.Clear();
       for (size_t i=0;i<sdr->antennaNames.size();++i){
       		strings.Add(sdr->antennaNames[i]);
        }
		new wxStaticText(panel2,wxID_STATIC,wxT("Antenna:"),wxPoint(0,72), wxDefaultSize,wxALIGN_LEFT);

		antennaCombo=new wxComboBox(panel2,ID_COMBOANTENNA,wxT("FILTER"),wxPoint(60,70),wxDefaultSize,
	                   strings,wxCB_DROPDOWN);
		antennaCombo->SetSelection(0);
        

	}

    yloc += 110;
    

       wxPanel *panel3 = new wxPanel(ScrolledWindow,wxID_ANY, wxPoint(20,yloc), wxSize(230, 230),wxBORDER_SUNKEN | wxFULL_REPAINT_ON_RESIZE,wxT("Control2"));

      wxString computers2[] =
      { "Float", "Short int","Signed char","Unsigned char" };
      
	  sendTypeBox=new wxRadioBox(panel3, ID_DATATYPE,
		"Data Type", wxPoint(2,10), wxSize(120, 110),
		 4, computers2, 0, wxRA_SPECIFY_ROWS);
		 
	sendTypeBox->SetSelection(1);

      wxString computers3[] =
      { "Listen","TCP/IP","UDP","Speakers","Pipe" };
      
	  sendModeBox=new wxRadioBox(panel3, ID_DATAMODE,
		"Send Mode", wxPoint(125,10), wxSize(120, 110),
		 5, computers3, 0, wxRA_SPECIFY_ROWS);
		 
	sendModeBox->SetSelection(1);
	
	wxStaticBox *box22 = new wxStaticBox(panel3, wxID_ANY, "&Net-Address",wxPoint(1,125), wxSize(225, 60),wxBORDER_SUNKEN );

    sendAddress=new wxTextCtrl(box22,ID_TEXTCTRL,wxT("192.168.1.3:3500"),
          wxPoint(5,10), wxSize(220, 30));

  	new wxButton(panel3,ID_STARTSEND,wxT("Start"),wxPoint(20,200));
  	
  	new wxButton(panel3,ID_STOPSEND,wxT("Stop"),wxPoint(140,200));

 	yloc += 235;
	
	wxStaticBox *box33 = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Alpha",wxPoint(20,yloc), wxSize(225, 190),wxBORDER_SUNKEN );

    textAlpha=new wxTextCtrl(box33,ID_TEXTCTRL,wxT("0.1"),
          wxPoint(5,10), wxSize(220, 30));
          
    cbox=new wxCheckBox(box33,ID_SETGAIN, "&Power Range",wxPoint(20,45), wxSize(230, 25));
	cbox->SetValue(0);	


	new wxStaticText(box33,wxID_STATIC,wxT("pmin:"),wxPoint(20,75), wxSize(50, 30),wxALIGN_LEFT);

          
    rangeMin=new wxTextCtrl(box33,ID_TEXTCTRL,wxT("-120"),
          wxPoint(75,75), wxSize(100, 30));
          
  	new wxStaticText(box33,wxID_STATIC,wxT("pmax:"),wxPoint(20,105), wxSize(50, 30),wxALIGN_LEFT);
        
          
    rangeMax=new wxTextCtrl(box33,ID_TEXTCTRL,wxT("-30"),
          wxPoint(75,105), wxSize(100, 30));
          
    
  	new wxButton(box33,ID_ALPHA,wxT("Apply"),wxPoint(10,140));
	
	yloc += 200;
	
	wxStaticBox *box44 = new wxStaticBox(ScrolledWindow, wxID_ANY, "&TV Frequencies",wxPoint(20,yloc), wxSize(225, 190),wxBORDER_SUNKEN );
	
	//wxStaticBox *box44 = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Frequencies",wxPoint(20,yloc), wxDefaultSize,wxBORDER_SUNKEN );

	listFrequency=new wxListCtrl(box44,ID_RXFREQUENCY,wxPoint(20,10), wxSize(225, 190),wxLC_REPORT|wxLC_SINGLE_SEL);
	
	//listFrequency=new wxListCtrl(box44,wxID_ANY,wxPoint(0,0), wxDefaultSize,wxLC_REPORT|wxLC_VIRTUAL);
	
	
	//listFrequency=new wxListCtrl(this,wxID_ANY,wxPoint(20,yloc),wxDefaultSize,wxLC_REPORT|wxLC_SINGLE_SEL);
	
	
	wxListItem itemCol;
	
	itemCol.SetText("Channel");
	listFrequency->InsertColumn(0,itemCol);
	listFrequency->SetColumnWidth(0,60);
	
	itemCol.SetText("Freq");
	listFrequency->InsertColumn(1,itemCol);
	listFrequency->SetColumnWidth(1,155);
	
	      wxString computers31[] =
      { "2","3","4","5","6",
      	"7","8","9","10","11","12","13",
        "14","15","16","17","18", "19","20","21","22","23",
      	"24","25","26","27","28", "29","30","31","32","33",
        "34","35","36",""};

	      wxString computers32[] =
       { "57","63","69","79","85",
       	 "177","183","189","195","201","207","213",
         "473","479","485","491","497", "503","509","515","521","527",
      	 "533","539","545","551","557", "563","569","575","581","587",
         "593","599","605",""};

	
	
	for(int n=0;n<36;++n){
		wxString buf;

		buf.Printf("%d",n);
		
		listFrequency->InsertItem(n,computers31[n]);

		listFrequency->SetItemData(n,n);
	
		buf.Printf("Col 1, item %d",n);
		listFrequency->SetItem(n,1,computers32[n]);
	
	
	}
	

/*
	wxStaticBox *box44 = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Frequencies",wxPoint(20,yloc), wxSize(225, 190),wxBORDER_SUNKEN );
	
	listFrequency=new wxListCtrl(box44,wxID_ANY,wxPoint(0,0), wxDefaultSize,wxLC_REPORT|wxLC_VIRTUAL);
	
	wxString listFrequency::OnGetItemText(long item,long column) const
	{
		return wxString::Format(" Column %ld item %ld",column,item);
	}
	
	
	listFrequency->InsertColumn(0,"first");
	listFrequency->SetColumnWidth(0,155);

	listFrequency->InsertColumn(1,"second");
	listFrequency->SetColumnWidth(1,155);

	listFrequency->SetItemCount(1000000);
*/

    wxWindow::SetSize(wxDefaultCoord,wxDefaultCoord,280,100);
    
    iRefresh=1;
    
//    fprintf(stderr,"BasicPane Done\n");
    
}

void BasicPane::dataType(wxCommandEvent &event)
{
	event.Skip();
	
	//wxString name=event.GetString();
	
	//const char *mode=name.ToUTF8().data();

	//fprintf(stderr,"dataType %s GetSelection %d\n",mode,event.GetSelection());
	
}
void BasicPane::dataMode(wxCommandEvent &event)
{
	event.Skip();
	
	//wxString name=event.GetString();
	
	//const char *mode=name.ToUTF8().data();

	//fprintf(stderr,"dataMode %s GetSelection %d\n",mode,event.GetSelection());
	
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
	}
	
	if(sdr->inData != IN_RADIO)return;
		
	int gainMode=sdr->device->getGainMode(SOAPY_SDR_RX, sdr->channel);
	
	gainMode = !gainMode;
	
	sdr->device->setGainMode(SOAPY_SDR_RX, sdr->channel, gainMode);

	//fprintf(stderr,"OnCheckAuto id %d gainMode %d\n",id,gainMode);

}


void BasicPane::setRxGain(wxCommandEvent &event)
{
	event.Skip();
	
	int id=event.GetId()-ID_RXGAIN;
	
	//fprintf(stderr,"setRxGain id %d\n",id);
	
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
void BasicPane::OnTextBandWidth(wxCommandEvent &event)
{
	event.Skip();
	
	//fprintf(stderr,"OnTextBandWidth\n");
	
	wxString bandwidth=event.GetString();
	
	if(!bandwidth)return;

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
	
	//fprintf(stderr,"setBandwidth\n");
	
	//fprintf(stderr,"Filter %d\n",bandwidthCombo->GetSelection());
	
	
	wxString bandWidths=event.GetString();

	//const char *bandWidth=bandWidths.c_str();

   //fprintf(stderr,"bandWidth %s len %d\n",bandWidth,(int)strlen(bandWidth));

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
	//fprintf(stderr,"min gain %g\n",gain);
	if(gain+20 > gSpectrum->verticalMaximum)gSpectrum->verticalMaximum=gain+20;

}
void BasicPane::OnMaximun(wxCommandEvent &event)
{
	event.Skip();
	float gain=event.GetSelection();
	gSpectrum->verticalMaximum=gain;
	//fprintf(stderr,"max gain %g\n",gain);
	if(gain-20 < gSpectrum->verticalMinimum)gSpectrum->verticalMinimum=gain-20;

}

void BasicPane::setSampleRate(wxCommandEvent &event)
{
	event.Skip();
	fprintf(stderr,"setSampleRate\n");
	
	fprintf(stderr,"Filter %d\n",sampleRateCombo->GetSelection());
	

	wxString name=sampleRateCombo->GetValue();

	fprintf(stderr,"S1\n");

	
	if(!name)return;
	
	fprintf(stderr,"S2\n");
	
	const char *mode=name.ToUTF8().data();
	
	std::string number=mode;
	
	fprintf(stderr,"Filter %s\n",number.c_str());


}

void BasicPane::OnText(wxCommandEvent &event)
{
	event.Skip();
		
	wxString rates=event.GetString();
	
	if(!rates)return;

	sdr->waitPointer("iqToAudio(7)",&sdr->iqToAudio,0);

	sdr->waitPointer("frame(7)",&sdr->frame,0);

	sdr->waitPointer("doWhat(7)",&sdr->doWhat,0);
	
	sdr->samplerate=atof(rates);
	sdr->samplewidth=sdr->samplerate;
	
	sdr->startPlay();
	
	std::thread(&sdrClass::run,sdr).detach();
	
}

void BasicPane::OnScroll(wxCommandEvent &event)
{
	event.Skip();
	//int which=event.GetId();
	float gain=event.GetSelection();
	if(event.GetId() == SCROLL_GAIN){
	//	printf("OnScroll gain %g %d \n",gain,event.GetId());
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
	
	fprintf(stderr,"Filter %s %d\n",number.c_str(),filterCombo->GetSelection());
	
	gSpectrum->filterType=filterCombo->GetSelection();


}
 
 void BasicPane::render( wxPaintEvent& evt )
{
	evt.Skip();
	//fprintf(stderr,"BasicPane:render sdr->decodemode %d\n",sdr->decodemode);
	modeBox->SetSelection(sdr->decodemode);

}

 void BasicPane::resized(wxSizeEvent& evt)
{
//	wxGLCanvas::OnSize(evt);
	//fprintf(stderr,"BasicPane::resized %d %d\n",getWidth(),getHeight());
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

void BasicPane::mouseDown(wxMouseEvent& event) {	event.Skip(); printf("BasicPane::mouseDown\n");}

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
	//fprintf(stderr,"BasicPane::OnTimer\n");
	
	if(gSpectrum->iWait){
		iRefresh=1;
		Sleep2(10);
		return;
	}
	
	//fprintf(stderr,"BasicPane::OnTimer gSpectrum->iWait %d gSpectrum->sdr %p\n",gSpectrum->iWait,gSpectrum->sdr);
	
	//auto t1 = std::chrono::high_resolution_clock::now();

	
	if(gSpectrum->sdr->saveFlag){
		
		for(int n=0;n<gSpectrum->buffSendLength;++n){
			gSpectrum->buffSend2[2*n]=gSpectrum->sdr->saveBuff[2*n];
			gSpectrum->buffSend2[2*n+1]=gSpectrum->sdr->saveBuff[2*n+1];
			//if(fabs(buff[2*n]) > amax)amax=fabs(buff[2*n]);
		}
		
		//fprintf(stderr,"sdr->saveFlag %d buffSendLength %d buffFlag %d\n",sdr->saveFlag,buffSendLength,buffFlag);
		
		gSpectrum->buffFlag=1;
		
	    gSpectrum->sdr->saveFlag=0;
	}else{
		//fprintf(stderr,"Save Not Set\n");
	}
	
	if(iRefresh){
		iRefresh=0;
		gTopPane->Refresh();
		Refresh();
		//fprintf(stderr,"gTopPane->Refresh called f %g sdr->decodemode %d\n",sdr->f,sdr->decodemode);
	}
	
	if(gSpectrum->buffFlag){
		gSpectrum->Refresh();
		gWaterFall->Refresh();
	}else{
	 // fprintf(stderr,"OnTimer Skip call\n");
	}
	
	//auto t2 = std::chrono::high_resolution_clock::now();
	//std::chrono::duration<double> difference = t2 - t1;
	//fprintf(stderr,"Time in OnTimer %g\n",difference.count());

	
	
}



void BasicPane::OnComboSampleRate(wxCommandEvent& event)
{
	event.Skip();

	wxString rates=event.GetString();

//	const char *rate=rates.c_str();

//	fprintf(stderr,"rate %s len %d\n",rate,(int)strlen(rate));

	//fprintf(stderr,"Start Stop\n");

	sdr->waitPointer("iqToAudio(7)",&sdr->iqToAudio,0);

	sdr->waitPointer("frame(7)",&sdr->frame,0);

	sdr->waitPointer("doWhat(7)",&sdr->doWhat,0);

	
	sdr->samplerate=atof(rates);
	sdr->samplewidth=0;
	
	Sleep2(50);
	
	
	//fprintf(stderr,"Stop Stop sdr->samplerate %g\n",sdr->samplerate);

	
	sdr->startPlay();
	
	std::thread(&sdrClass::run,sdr).detach();
}

void BasicPane::OnComboAntenna(wxCommandEvent& event)
{
		event.Skip();
		
		wxString antennas=event.GetString();
		
		const char *antenna=antennas.c_str();
		
		//fprintf(stderr,"antenna %s len %d\n",antenna,(int)strlen(antenna));
		
		sdr->device->setAntenna(SOAPY_SDR_RX, sdr->channel, antenna);

}

void BasicPane::OnCombo(wxCommandEvent& event){
	//int ngroup=event.GetSelection();
//	printf("OnCombo %d IsChecked %d\n",item,event.IsChecked());
//	noteCheckBox=event.IsChecked();

		event.Skip();
		
		
	//fprintf(stderr,"BasicPane::OnCombo\n");

	wxString number=event.GetString();
	
	double value;
	
	number.ToCDouble(&value);
	

    if(!gSpectrum->buffSend10){
    	fprintf(stderr,"Radio Not Running\n");
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


//	printf("np %d iWait %d saveCall %d iWait %p\n",np,gSpectrum->iWait,gSpectrum->sdr->saveCall,&gSpectrum->iWait);
	
}


void BasicPane::OnButton2(wxCommandEvent& event) 
{    
	event.Skip();

   // int id=event.GetId();
    
   // int flag=event.GetValue();
     
   //	fprintf(stderr,"OnButton2 id %d flag %d\n",id,flag);
    

	//sdr->printDevices();
	
/*
	wxFrame *frame= new wxFrame(NULL, wxID_ANY, wxT("Device Select"), wxDefaultPosition, wxSize(400,400));
	
	selectionWindow *s=new selectionWindow(frame, "Device");
	
	s->gBasicPane=this;

	frame->Show();
*/
	
//	wxWindow::SetSize(wxDefaultCoord,wxDefaultCoord,200,200);
	
//	fprintf(stderr,"Select Device OUT\n");

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
	//fprintf(stderr,"exit1 BasicPane %p thread %llx\n",this,(long long)std::this_thread::get_id);
	if(sdr)delete sdr;
	sdr=NULL;
	//fprintf(stderr,"exit2 BasicPane %p thread %llx\n",this,(long long)std::this_thread::get_id);
}



BEGIN_EVENT_TABLE(WaterFall, wxGLCanvas)
EVT_MOTION(WaterFall::mouseMoved)
EVT_PAINT(WaterFall::render)
EVT_SIZE(WaterFall::resized)
EVT_LEFT_DOWN(WaterFall::mouseDown)
EVT_MOUSEWHEEL(WaterFall::mouseWheelMoved)
EVT_CHAR(WaterFall::OnChar)    
/*
EVT_LEFT_UP(WaterFall::mouseReleased)
EVT_RIGHT_DOWN(WaterFall::rightClick)
EVT_LEAVE_WINDOW(WaterFall::mouseLeftWindow)
EVT_KEY_DOWN(WaterFall::keyPressed)
EVT_KEY_UP(WaterFall::keyReleased)
EVT_TIMER(TIMER_ID,WaterFall::OnTimer)
*/
END_EVENT_TABLE()

void WaterFall::OnChar(wxKeyEvent& event) 
{
	extern soundClass *s;

	event.Skip();
	
    int keycode=event.GetKeyCode();

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
//		fprintf(stderr,"bw %g %ld\n",sdr->bw,(long)event.ControlDown());
	}
}
void WaterFall::mouseDown(wxMouseEvent& event) 
{
	extern soundClass *s;
	event.Skip();
	wxPoint pp3 = event.GetLogicalPosition(wxClientDC(this))*scaleFactor;
	if(sdr){
		double fx=sdr->fw-0.5*sdr->samplewidth + sdr->samplewidth*(pp3.x)/((double)getWidth());
		//printf("mouseDown x %d y %d sdr->fc %g sdr->f %g sdr->samplerate %g fx %g width %d\n",p1.x,p1.y,sdr->fc,sdr->f,sdr->samplerate,fx,getWidth());
		//printf(" fx %g\n",fx);
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
	//fprintf(stderr,"mouseMoved \n");
		event.Skip();

}
WaterFall::~WaterFall()
{
	if(m_context)delete m_context;
	
	if(water.data)cFree((char *)water.data);
	water.data=NULL;

	//fprintf(stderr,"exit WaterFall %p\n",this);
}

void WaterFall::prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluOrtho2D(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
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

void WaterFall::render2()
{


}
void WaterFall::render( wxPaintEvent& evt )
{
	evt.Skip();
//   fprintf(stderr,"WaterFall::render\n");

    if(!IsShown()) return;
    
    
    float *magnitude=gSpectrum->buffSend10;
    
    int length=gSpectrum->buffSendLength;
    
    //auto t1 = chrono::high_resolution_clock::now();
    

   // fprintf(stderr,"WaterFall render 2\n");
        
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
    
//   fprintf(stderr,"rmin %g rmax %g amin %g amax %g\n",rmin,rmax,amin,amax);
    
    
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
    
    
    //fprintf(stderr,"nmin %d nmax %d dxn %g\n",nmin,nmax,dxn);
   
    
    double dxw=(double)(water.xsize-1)/(double)(length-1);
   
    
    int ics=wateric[(int)(2*dxn+nmin)];
    
    for(int nnn=0;nnn<length;++nnn){
        int ic;
        
        int n=nnn*dxn+nmin+0.5;
        
        int next=(nnn+1)*dxn+nmin+0.5;
        
        ic=wateric[n];
 
        int nn=nnn*dxw+0.5;
        
        int nn2=next*dxw+0.5;
        
        

//            fprintf(stderr,"nn %d nn2 %d nnn %d n %d next %d ic %d ics %d\n",nn,nn2,nnn,n,next,ic,ics);

        if(ic > ics)ics=ic;
        
        if(nn == nn2){
        	//fprintf(stderr,"2 nn %d nn2 %d nnn %d\n",nn,nn2,nnn);
           continue;
        }
        ic=ics;
        
        ics=wateric[next];
        
        if(nn < 0 || nn >= water.SRect.xsize){
            fprintf(stderr,"nn %d nn2 %d nnn %d n %d next %d ic %d ics %d\n",nn,nn2,nnn,n,next,ic,ics);
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
    
   // fprintf(stderr,"x %d y %d xsize %d ysize %d \n",water.SRect.x,water.SRect.y,water.SRect.xsize,water.SRect.ysize);

    int xsize=water.SRect.xsize;
    
    int ysize=water.SRect.ysize;
    
    int nline=water.nline;
         
    glDrawPixels(xsize,ysize,GL_RGB, GL_UNSIGNED_BYTE, &water.data[nline*xsize*3]);

    glColor4f(0, 0, 0, 1);
	int xs=ftox(sdr->f-sdr->bw/2.0);
	DrawLine(xs, 0, xs, getHeight());
	xs=ftox(sdr->f+sdr->bw/2.0);
	DrawLine(xs, 0, xs, getHeight());
	
//	fprintf(stderr,"xs %d iWait %d\n",xs,iWait);

/*
 		
 		//fprintf(stderr," left %d right %d center %d xsize %d bw %g\n",ftox(sdr->f-sdr->bw/2.0),ftox(sdr->f+sdr->bw/2.0),ftox(sdr->f),box.xsize,sdr->bw);
 		
 		DrawBox(&box,0);
*/

//	fprintf(stderr,"Waterfall f %p\n",&sdr->f);

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
    
  //  fprintf(stderr,"WaterFall::SetWindow\n");
    
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
END_EVENT_TABLE()

// some useful events to use
void Spectrum::mouseMoved(wxMouseEvent& event) {
	//fprintf(stderr,"mouseMoved \n");
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
//		fprintf(stderr,"bw %g %ld\n",sdr->bw,(long)event.ControlDown());
	}

}

void Spectrum::OnChar(wxKeyEvent& event) 
{
	event.Skip();
	
	int keycode=event.GetKeyCode();
	
	fprintf(stderr,"Spectrum::OnChar %d\n",keycode);
	
    if(keycode == 'w'){
        iWait = !iWait;
        fprintf(stderr,"iWait %d\n",iWait);
    }

    if(keycode == 'n'){
        sdr->iWait = !sdr->iWait;
        fprintf(stderr,"sdr->iWait %d\n",sdr->iWait);
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
    
    filterType=FILTER_BLACKMANHARRIS7;
    
    startTime = new std::chrono::high_resolution_clock;
    
    lineDumpInterval=0.1;
    lineTime=ftime()+lineDumpInterval;

    
  //  printf("Groups of triangles %ld vender %s\n",triangle,glGetString(GL_VENDOR));
  
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
//	fprintf(stderr,"startRadio2 Start sdr %p thread %llx\n",sdr,(long long)std::this_thread::get_id);
	
    
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
	
	sdr->run();
	
//   fprintf(stderr,"startRadio2 out sdr %p thread %llx \n",sdr,(long long)std::this_thread::get_id);

}


void Spectrum::keyPressed(wxKeyEvent& event) 
{
    wxString key;
    long keycode = event.GetKeyCode();
    
    printf("keycode keyPressed %ld\n",keycode);
    
    if(keycode == 84){
    	doTestSpeed();
    }
        
    if(keycode == 87){
        iWait = !iWait;
        fprintf(stderr,"iWait %d\n",iWait);
    }

    if(keycode == 78){
        sdr->iWait = !sdr->iWait;
        fprintf(stderr,"sdr->iWait %d\n",sdr->iWait);
    }
    
	event.Skip();
}
int Spectrum::doTestSpeed()
{
    
	double start,end;
	long n;
	double den;
	
	fprintf(stderr,"doTestSpeed\n");
	//count=0;
	
	start=rtime();
	
	for(n=0;n<25;++n){
	    //count++;
		//doCommands(NewScene);
		angle += 2;
		//render2();
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
	//fprintf(stderr,"exit Spectrum %p\n",this);

}

void Spectrum::resized(wxSizeEvent& evt)
{
//	wxGLCanvas::OnSize(evt);
	//fprintf(stderr,"Spectrum::resized %d %d\n",getWidth(),getHeight());
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
			//printf("mouseDown x %d y %d sdr->fc %g sdr->f %g sdr->samplerate %g fx %g width %d\n",p1.x,p1.y,sdr->fc,sdr->f,sdr->samplerate,fx,getWidth());
			//printf(" fx %g\n",fx);
			if(sdr->samplescale < 0.98){
				sdr->setCenterFrequency(fx);
			}else{
				sdr->setFrequency(fx);
			}
			s->bS=sdr->bS;
			gTopPane->Refresh();
		}
	}else{
		printf("Spectrum::mouseDown x %d y %d\n",pp3.x,pp3.y);
	}

}

int Spectrum::ftox(double frequency){

	int x;
	
	x=(int)(getWidth()*(frequency-sdr->fw+0.5*sdr->samplewidth)/sdr->samplewidth);

	return x;
}
void Spectrum::render2( )
{
	;
}
/** Inits the OpenGL viewport for drawing in 3D. */
void Spectrum::prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
	
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // Black Background
    glClearDepth(1.0f);	// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
    glEnable(GL_COLOR_MATERIAL);
	
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
    float ratio_w_h = (float)(bottomrigth_x-topleft_x)/(float)(bottomrigth_y-topleft_y);
    gluPerspective(45 /*view angle*/, ratio_w_h, 0.1 /*clip close*/, 200 /*clip far*/);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
}
 
/** Inits the OpenGL viewport for drawing in 2D. */
void Spectrum::prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluOrtho2D(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
 
 
void Spectrum::render(wxPaintEvent& evt )
{
	evt.Skip();
	
	//static long long nc=0;
/*
	static int tick=0;
	tick++;
	tick=10;
*/
	//fprintf(stderr,"Spectrum render nc %lld\n",nc++);

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
 		
 		//fprintf(stderr," left %d right %d center %d xsize %d bw %g\n",ftox(sdr->f-sdr->bw/2.0),ftox(sdr->f+sdr->bw/2.0),ftox(sdr->f),box.xsize,sdr->bw);
 		
 		DrawBox(&box,0);
 		
		glColor4f(0, 0, 1, 1);
		
		//fprintf(stderr,"filterType %d\n",filterType);
		
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
			//fprintf(stderr,"xmnc %g xmxc %g value %g samplewidth %g\n",xmnc,xmxc,value,sdr->samplewidth);
			GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
			//fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
			
			for(double xp=xmnc;xp <= xmxc;xp += Large){
				char cbuff[256];
				sprintf(cbuff,"%g",xp/1e6);
			    double xx=((xp-(fc-bw))/(2.0*bw));
			    if(xx < 0.0 || xx > 1.0)continue;
			    int ixx=(int)(idx*xx);
			    if(ixx < ixmin || ixx > ixmax)continue;
 				DrawLine3(ixx, 0, ixx, getHeight()-15);
 				//fprintf(stderr," %g ",xx);
 				DrawString(ixx-10,getHeight()-13, cbuff);
			}
			//fprintf(stderr," idx %g\n",idx);
			
			
			xmnc=ymin;
			xmxc=ymax;
			//fprintf(stderr,"xmnc %g xmxc %g\n",xmnc,xmxc);
			GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
			//fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
			
			for(double xp=xmnc;xp <= xmxc;xp += Large){
				char cbuff[256];
				sprintf(cbuff,"%g",xp);
			    double xx=((xp-ymin)/(dy));
			    if(xx < 0.0 || xx > 1.0)continue;
			    int ixx=(int)(iymax+idy*xx);
			    //int ixx=(int)(iymax-xx*idy);
				//fprintf(stderr," %d %g ",ixx,xx);
 				DrawLine3(30, ixx, getWidth(), ixx);
 				//fprintf(stderr," %g ",xx);
 				DrawString(5,ixx-8,cbuff);
			}
			//fprintf(stderr," idy %g\n",idy);
		
 		}
		
		
/*		
		if(tick % 40 == 0){
			double dt=1.0;
			printf("plot %d signal\n",buffSendLength);
			for(int n=0;n<buffSendLength;++n){
			//	printf("%f %f\n",n*dt,(buffSend[2*n]*buffSend[2*n]+buffSend[2*n+1]*buffSend[2*n+1]));
				printf("%f %f\n",n*dt,buffSend10[n]);
			}
		}
*/

		
		//fprintf(stderr,"ixxmin %d ixxmax %d getWidth() %d iyymin %d iyymax %d getHeight() %d\n",ixxmin,ixxmax,getWidth(),iyymin,iyymax,getHeight());

		//auto t2 = chrono::high_resolution_clock::now();
		//std::chrono::duration<double> difference = t2 - t1;
		//std::cout << "Time "<< difference.count() << endl;
		//fprintf(stderr,"count %g\n",difference.count());

	}

	//fprintf(stderr,"Spectrum done\n");
		
    glFlush();
    SwapBuffers();
}

int DrawBox(uRect *box,int offset)
{
    if(box->xsize <= 0)return 0;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_BLEND); //Enable blending.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Set blending function.
    
    
    glColor4f(0.0, 1.0, 0.0, 0.5);
    glBegin(GL_QUADS);
    glVertex2f(box->x, box->y);
    glVertex2f(box->x, box->y+box->ysize);
    glVertex2f(box->x+box->xsize,box->y+box->ysize);
    glVertex2f(box->x+box->xsize, box->y);
    glEnd();
    
    glDisable(GL_BLEND);
    
    
    return 0;
}

int GetTime(long *Seconds,long *milliseconds)
{
	struct timeb t;
	
	if(!Seconds || !milliseconds)return 1;
	

	ftime(&t);

	*Seconds=(long)t.time;
	*milliseconds=t.millitm;
	
	return 0;
}
double rtime(void)
{
	long milliseconds;
	long Seconds;
	double ret;
	
	
	GetTime(&Seconds,&milliseconds);
	
	ret=(double)Seconds+(double)milliseconds/1000.;
	
	return ret;

}
int doWindow(float *x,long length,int type)
{
    static float *w=NULL;
    static long lengthSave = -1;
    if(lengthSave < length){
        if(w)delete w;
        lengthSave=length;
        w=new float[lengthSave];
//      fprintf(stderr,"lengthSave %ld\n",lengthSave);
   	}
   
    int i;
    
    if(!x)return 1;
    
    switch(type){
            
        case FILTER_RECTANGULAR:

            for(i=0; i<length; i++)
                w[i] = 1.0;
            
            break;
            

            
        case FILTER_HANN:
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_hann(i, (int)length);
#else
                w[i]=hann(i, (int)length);
#endif
            }
            break;

            
            
        case FILTER_HAMMING:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_hamming(i, (int)length);
#else
                w[i]=hamming(i, (int)length);
#endif
            }
            break;
            
        case FILTER_FLATTOP:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_flattop(i, (int)length);
#else
                w[i]=flattop(i, (int)length);
#endif
            }
            break;
            
            
        case FILTER_BLACKMANHARRIS:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_blackmanharris(i, (int)length);
#else
                w[i]=blackmanharris(i, (int)length);
#endif
            }
            break;
            
        case FILTER_BLACKMANHARRIS7:
            
            for(i=0; i<length; i++)  {
#ifdef WINDOWS_LONG_NAMES
                w[i]=liquid_blackmanharris7(i, (int)length);
#else
                w[i]=blackmanharris7(i, (int)length);
#endif
            }
            break;
    }
    
    for(i=0; i<length; i++){
        double amp;
        amp=w[i];
        x[2*i]=amp*x[2*i];
        x[2*i+1]=amp*x[2*i+1];
    }
    
    return 0;
    
}

int colorit(long count,double *level,double value,int *ic);


double red[256],green[256],blue[256],table[256];


int colorit(long count,double *level,double value,int *ic)
{
	long ib,it,ns;
	
	if(value <= *level){
		*ic=0;
		return 0;
	}
	it=count-1;
	if(value >= level[it]){
		*ic=it;
		return 0;
	}
	ib=0;
	while(it > ib+1){
		ns=(it+ib)/2;
		if(value > level[ns]){
			ib=ns;
		}else{
			it=ns;
		}
	}
	*ic=ib;
	return 0;
}

static int DrawSym(double xp,double yp,double hp,char *ib,double angle,int nchar);

static int smove(double x,double y);

static double xlast,ylast;

int DrawLine3(int x1, int y1, int x2, int y2)
{
    
    glPushAttrib(GL_ENABLE_BIT);
    
    glLineStipple(1, 0xAAAA);
    glEnable(GL_LINE_STIPPLE);
    glLineWidth((GLfloat)1.0);
    glBegin(GL_LINES);
    
    glVertex2f((x1),(y1));
    glVertex2f((x2),(y2));
    
    glEnd();
    
    glPopAttrib();
    
    return 0;
}

int DrawLine2(float x1, float y1, float x2, float y2)
{
    glBegin(GL_LINES);
    
    glVertex2f((x1),(y1));
    glVertex2f((x2),(y2));
    
    glEnd();
    
    return 0;
}
int DrawString(int x, int y, char *out)
{
   // int h;
    
    if(!out)return 1;
    
    glPushMatrix();
    glRasterPos2i( x, y);
        
    DrawSym((double)x,(double)y,8.0,out,0.0,(int)strlen(out));
    
    glPopMatrix();
    
    return 0;
}
static int swhere(double *x,double *y)
{
    *x=xlast;
    *y=ylast;
    return 0;
}
static int smove(double x,double y)
{
    xlast=x;
    ylast=y;
    
    return 0;
}
static int sdraw(double x,double y)
{
    
    DrawLine2(xlast, ylast, x, y);

    xlast=x;
    ylast=y;
    
    return 0;
}

static int DrawSym(double xp,double yp,double hp,char *ib,double angle,int nchar)
{
    static int icp[]={
        1,7,17,21,26,31,36,41,45,49,55,64,74,79,81,
        0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0, 0,0,
        0,0,0,0,0,0,0,0,
        83,87,91,101,107,110,112,117,119,128,134,143,159,163,
        172,183,188,204,215,0,0,215,220,0,0,220,229,243,251,258,
        265,271,283,291,299,305,313,316,321,325,334,341,353,363,
        373,378,384,387,392,397,403,412
    };
    
    static int ishape[]={
        47,17,11,71,77,47,47,37,15,13,31,51,73,75,57,47,47,11,71,47,
        47,41,00,14,74,17,71,00,11,77,47,14,41,74,47,47,14,74,47,41,
        11,77,17,71,17,77,11,71,17,44,77,00,44,41,17,71,00,11,77,17,
        11,71,77,47,41,00,14,74,77,17,11,71,74,77,11,71,17,77,47,41,
        00,00,27,16,12,21,47,56,52,41,25,43,00,33,35,00,45,23,00,24,
        44,24,44,00,35,33,32,41,30,24,44,31,41,42,32,31,11,57,21,12,
        16,27,47,56,52,41,21,26,37,31,00,21,41,15,16,27,47,56,55,12,
        11,51,15,16,27,47,56,55,44,53,52,41,21,12,13,00,34,44,41,47,
        14,54,57,17,14,44,53,52,41,21,12,56,47,27,16,12,21,41,52,53,
        44,14,16,17,57,56,31,24,15,16,27,47,56,55,44,53,52,41,21,12,
        
        13,24,44,54,24,15,16,27,47,56,52,41,21,12,35,55,00,33,53,11,
        16,27,47,56,51,00,14,54,17,11,41,52,53,44,44,55,56,47,17,00,
        14,44,56,47,27,16,12,21,41,52,52,56,47,17,11,41,52,57,17,11,
        51,00,44,14,57,17,11,00,44,14,54,52,41,21,12,16,27,47,56,00,
        44,64,11,17,00,14,54,00,57,51,17,57,00,37,31,00,11,51,57,52,
        41,21,12,13,17,11,00,14,51,00,14,57,17,11,51,11,17,35,57,51,
        11,17,51,57,21,12,16,27,47,56,52,41,21,11,17,47,56,55,44,14,
        21,12,16,27,47,56,52,41,21,00,42,51,11,17,47,56,55,44,14,00,
        44,51,56,47,27,16,15,53,52,41,21,12,17,57,00,37,31,17,12,21,
        41,52,57,17,31,57,17,11,34,51,57,17,51,00,11,57,17,34,57,00,
        34,31,17,57,56,12,11,51,00,24,44
    };
    double xzero,yzero,h;
    double sina,cosa,a;
    double dxzero,dyzero;
    double xx,yy,xxx,yyy;
    int nloop,ioff,iloop;
    int c,lc,ic,iloc,iend;
    int penup,i,iss,ix,iy;
    
    if(nchar == 0)return 0;
    swhere(&xzero,&yzero);
    if(xp != 999.)xzero=xp;
    if(yp != 999.)yzero=yp;
    h=hp/7.;
    ioff=1;
    nloop=nchar;
    if(nchar <= 0){
        nloop=1;
        if(nchar == -1)smove(xzero,yzero);
        if(nchar != -1)sdraw(xzero,yzero);
        ioff=4;
    }
    a=.017453293*angle;
    sina=sin(a);
    cosa=cos(a);
    dxzero=hp*cosa;
    dyzero=hp*sina;
    for(iloop=0;iloop<nloop;++iloop){
        c=ib[iloop];
        lc=c;
        if(c >= 'a'&& c <= 'z'){
            lc=lc-32;
        }
        ic=lc;
        if(ic < 0 || ic > 90)goto L110;
        iloc=icp[ic];
        if(iloc <= 0)goto L110;
        iend=icp[ic+1]-1;
        penup=TRUE;
        
        for(i=iloc;i<=iend;++i){
            iss=ishape[i-1];
            if(iss == 0){
                penup=TRUE;
                continue;
            }
            ix=(iss/10);
            iy=iss-10*ix;
            iy=10-iy;
            
            xx=(ix-ioff)*h;
            yy=(iy-ioff)*h;
            
            xxx=xzero+xx*cosa-yy*sina;
            yyy=yzero+xx*sina+yy*cosa;
            
            if(penup){
                smove(xxx,yyy);
            }else{
                sdraw(xxx,yyy);
            }
            penup=FALSE;
        }
        
    L110:
        if(nchar < 0)continue;
        xzero=xzero+dxzero;
        yzero=yzero+dyzero;
        
    }
    
    return 0;
}

static void GridPlotNeat(double *xmnc,double *xmxc,double *Large,double *Small)
{
    
    double xmn = *xmnc,xmx = *xmxc;
    double delx,step;
    int nn;
    long long stepi,min,max;
    double xmin,xmax;
    
    if(!xmnc || !xmxc || !Large || !Small)return;
    
    delx=xmx-xmn;
    
    step=delx/8;
    
    stepi=(long long)step;
    
    nn=0;
    while(stepi >= 10){
        stepi /= 10;
        ++nn;
    }
    
    while(nn-- > 0){
        stepi *= 10;
    }
    
    step=stepi;
    
    //printf("step %g stepi %lld\n",step,stepi);
    
    min=(xmn-step)/step;
    xmin=min*step;
    
    max=(xmx+step)/step;
    xmax=max*step;
    
    //printf("xmin %g xmax %g\n\n",xmin,xmax);
    
    *Large=step;
    
    *Small=step/5;
    
    *xmnc=xmin;
    
    *xmxc=xmax;

}

int testEM()
{
    double xmnc,xmxc,Large,Small;
    double fc,bw;
    
    fc=10.1e6;
    bw=10e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    fprintf(stderr,"xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    bw=1e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    fprintf(stderr,"xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    bw=0.1e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    fprintf(stderr,"xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    bw=0.01e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    fprintf(stderr,"xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    bw=0.001e6;
    xmnc=fc-bw;
    xmxc=fc+bw;
    fprintf(stderr,"xmnc %g xmxc %g\n",xmnc,xmxc);
    GridPlotNeat(&xmnc,&xmxc,&Large,&Small);
    fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
    
    return 0;
}

