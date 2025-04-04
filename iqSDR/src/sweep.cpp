#include "iqSDR.h"
#include "WarningBatch.h"
#include <string.h>
#include <vector>
#include <string>

void winout(const char *fmt, ...);

int copyl(char *p1,char *p2,long n);

void *cMalloc(unsigned long r, int tag);

void checkall(void);

extern StartWindow *startIt;

extern soundClass *s;

extern std::string ProgramVersion;

static std::vector<Sweep *> grabList;

BasicPane2 *pBasicPane2;
Spectrum2 *pSpectrum2;
WaterFall2 *pWaterFall2;
TopPane2 *pTopPane2;

static ostringstream outs;

std::vector<Sweep *> sgrabList;

#define POS(r, c)        wxGBPosition(r,c)
#define SPAN(r, c)       wxGBSpan(r,c)

wxBEGIN_EVENT_TABLE(Sweep, wxFrame)
EVT_MENU_RANGE(INPUT_MENU,INPUT_MENU+99,Sweep::OnInputSelect)
EVT_MENU_RANGE(OUTPUT_MENU,OUTPUT_MENU+99,Sweep::OnOuputSelect)
EVT_MENU_RANGE(ID_PALETTE,ID_PALETTE+99,Sweep::OnPaletteSelected)
EVT_MENU_RANGE(ID_OPTIONS,ID_OPTIONS+99,Sweep::OnOptionsSelected)
EVT_MENU_RANGE(ID_DIRECT,ID_DIRECT+99,Sweep::OnDirectSelected)
EVT_MENU_RANGE(ID_BAND,ID_BAND+99,Sweep::OnBandSelected)
EVT_MENU_RANGE(ID_SAMPLERATE,ID_SAMPLERATE+99,Sweep::OnSampleRateSelected)
EVT_SIZE(Sweep::resized)
EVT_MENU(wxID_ABOUT, Sweep::About)
EVT_MENU(ID_EXIT, Sweep::About)
wxEND_EVENT_TABLE()
void Sweep::About(wxCommandEvent &event)
{

	int item=event.GetId();
    if(item == wxID_ABOUT)wxMessageBox(ProgramVersion+"(c) 2025 Dale Ranta");
    if(item == ID_EXIT)startIt->OnQuit(event);
}

void Sweep::resized(wxSizeEvent& evt)
{
 	evt.Skip();

	//const wxSize size = evt.GetSize() * GetContentScaleFactor();

	//winout("Sweep::resized %d %d %f\n", size.x,size.y,GetContentScaleFactor());
	Refresh();

}


Sweep::Sweep(wxFrame* parent,wxString title,class sdrClass *sdrIn)
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
    

    
    m_gbs->Add(new TopPane2(this, "TopPane2"),   POS(0,0) , SPAN(1,3), wxEXPAND  );
    
    m_gbs->Add(new Spectrum2(this, args),   POS(1,1), SPAN(2,2), wxEXPAND  );
    
    m_gbs->Add(new WaterFall2(this, args),   POS(3,1) , SPAN(2,2), wxEXPAND  );

    m_gbs->Add(new BasicPane2(this, "Controls", sdrIn),   POS(1,0) , SPAN(4,1), wxEXPAND  );
    
    gWaterFall2=pWaterFall2;
    
    gBasicPane2=pBasicPane2;
    
    pBasicPane2->gSweep=this;

    m_gbs->AddGrowableCol(1);
    m_gbs->AddGrowableCol(2);
    m_gbs->AddGrowableRow(1);
    m_gbs->AddGrowableRow(2);
    m_gbs->AddGrowableRow(3);
    m_gbs->AddGrowableRow(4);

    this->SetSizerAndFit(m_gbs);
    SetClientSize(this->GetSize());

    pSpectrum2->iWait=1;

 
    //frame7->Show();
}

void Sweep::OnSampleRateSelected(wxCommandEvent& event)
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

void Sweep::OnBandSelected(wxCommandEvent& event)
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
void Sweep::OnOptionsSelected(wxCommandEvent& event){
	event.Skip();

	//int item=event.GetId()-ID_OPTIONS;
		
	wxString whatEvent=actionMenu->GetLabelText(event.GetId());
	
	const char *what=whatEvent.c_str();

   // winout("OnOptionsSelected %d INPUT_MENU %d checked %d what %s\n",item,ID_OPTIONS,actionMenu->IsChecked(event.GetId()),what);
    
    
    sdr->device->writeSetting(what,actionMenu->IsChecked(event.GetId()));
    
}

void Sweep::OnDirectSelected(wxCommandEvent& event){
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

void Sweep::OnPaletteSelected(wxCommandEvent& event){
	event.Skip();

	int item=event.GetId()-ID_PALETTE;
//	winout("OnPaletteSelected %d INPUT_MENU %d\n",item,ID_PALETTE);
	
	char name[512];

	double pal[3*256];

	getPalette(item,name,pal);

	//winout("name %s\n",name);
	
	for(int n=0;n<256;++n){
		gWaterFall2->pd.palette[3*n]=pal[3*n]*255;
		gWaterFall2->pd.palette[3*n+1]=pal[3*n+1]*255;
		gWaterFall2->pd.palette[3*n+2]=pal[3*n+2]*255;
	}
	
	for(int n=0;n<27;++n){
		paletteMenu->Check(ID_PALETTE+n,0);
	}
	
	paletteMenu->Check(event.GetId(),1);

	Refresh();
}

void Sweep::OnInputSelect(wxCommandEvent& event){
	int item=event.GetId()-INPUT_MENU;
	//winout("OnInputSelect %d INPUT_MENU %d\n",item,INPUT_MENU);
	int n=item/10;
	s->inputID=s->inputNames[n].deviceID;
	int sound=item-10*n;
	//winout("%s sampleRate %d\n",s->inputNames[n].name.c_str(),s->inputNames[n].sampleRate[sound]);
	SampleFrequency=s->inputNames[n].sampleRate[sound];
	//startAudio();
}

void Sweep::OnOuputSelect(wxCommandEvent& event){
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

Sweep::~Sweep()
{
//	winout("Sweep::~Sweep\n");
	
	if(sgrabList.size() > 0){
		for(std::vector<Sweep *>::size_type k=0;k<sgrabList.size();++k){
			Sweep *grab=sgrabList[k];
			if(grab == this){
			    sgrabList[k]=NULL;
//			    winout("Sweep remove from list\n");
			}
		}
	}

	
	//winout("exit Sweep %p\n",this);

}
BEGIN_EVENT_TABLE(TopPane2, wxWindow)
EVT_MOTION(TopPane2::mouseMoved)
EVT_PAINT(TopPane2::render)
EVT_LEFT_DOWN(TopPane2::mouseDown)
EVT_CHECKBOX(ID_RECORD, TopPane2::Record)
EVT_CHECKBOX(ID_FC, TopPane2::Record)
EVT_CHAR(TopPane2::OnChar)    
END_EVENT_TABLE()

void TopPane2::OnChar(wxKeyEvent& event) 
{
	event.Skip();
	
	int keycode=event.GetKeyCode();
	
	//winout("TopPane2::OnChar %d\n",keycode);
	
    if(keycode == 'f'){
        gSpectrum2->iFreeze = !gSpectrum2->iFreeze;
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

void TopPane2::Record(wxCommandEvent& event) 
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

TopPane2::TopPane2(wxWindow *frame, const wxString &title)
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
	
	pTopPane2=this;
		
	rbox=new wxCheckBox(this,ID_RECORD, "&record", wxPoint(20,0), wxSize(80, 40));
	rbox->SetValue(0);	

	rbox2=new wxCheckBox(this, ID_FC, "&fc", wxPoint(90,0), wxSize(60, 40));
	rbox2->SetValue(0);	

	wxWindow::SetSize(wxDefaultCoord,wxDefaultCoord,400,40);
}
TopPane2::~TopPane2()
{
	//winout("TopPane2::~TopPane2\n");
	//winout("exit TopPane2 %p\n",this);

}


void TopPane2::mouseDown(wxMouseEvent& event)
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
		
//		winout("TopPane2::mouseDown f %lld k %d nchar %d up %d %f\n",fl,k,nchar,up,value);
		Refresh();

	}
}


void TopPane2::mouseMoved(wxMouseEvent& event)
{
	event.Skip();

	wxPoint p = event.GetLogicalPosition(wxClientDC(this));
	
	nchar=(p.x-pt.x)/fontSize.x;

	//winout("TopPane2::mouseMoved diff %d character %d\n",p.x-pt.x,nchar);
	
	Refresh();
	
}

#include <wx/graphics.h>

void TopPane2::render( wxPaintEvent& evt )
{
	evt.Skip();
	
	wxPaintDC dc(this);
	
	dc.SetFont(*font);
	dc.SetBackgroundMode(wxTRANSPARENT);
	dc.SetTextForeground(*wxBLACK);
	dc.SetTextBackground(*wxWHITE);
	
	//winout("TopPane2 f %g \n",sdr->f);

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

	
//	winout("TopPane2:render\n");
}
BEGIN_EVENT_TABLE(BasicPane2, wxWindow)
EVT_RADIOBOX(ID_DATATYPE,BasicPane2::dataType)
EVT_RADIOBOX(ID_DATAMODE,BasicPane2::dataMode)
//EVT_MOTION(Spectrum22::mouseMoved)
EVT_PAINT(BasicPane2::render)
EVT_TIMER(TIMER_ID,BasicPane2::OnTimer)
EVT_LEFT_DOWN(BasicPane2::mouseDown)
EVT_BUTTON(ID_STARTRADIO, BasicPane2::startRadio)
EVT_RADIOBOX(ID_RADIOBOX, BasicPane2::radioBox)
EVT_COMBOBOX(ID_COMBOBOX,BasicPane2::OnCombo)
EVT_COMBOBOX(ID_COMBOMODE,BasicPane2::OnCombo)
EVT_COMBOBOX(ID_COMBOFILTER,BasicPane2::OnComboFilter)
EVT_COMBOBOX(ID_COMBOANTENNA,BasicPane2::OnComboAntenna)
EVT_COMBOBOX(ID_COMBOSAMPLERATE,BasicPane2::OnComboSampleRate)
EVT_COMBOBOX(ID_COMBOBANDWIDTH,BasicPane2::setBandwidth)
//EVT_TEXT(ID_COMBOSAMPLERATE, BasicPane2::OnText)
EVT_TEXT_ENTER(ID_COMBOSAMPLERATE, BasicPane2::OnText)
EVT_TEXT_ENTER(ID_COMBOBANDWIDTH, BasicPane2::OnTextBandWidth)
EVT_BUTTON(ID_COMBOBUTTON, BasicPane2::setSampleRate)
EVT_SIZE(BasicPane2::resized)
EVT_SLIDER(SCROLL_GAIN,BasicPane2::OnScroll)
EVT_SLIDER(ID_SAMPLEWIDTH,BasicPane2::setSampleWidth)
EVT_SLIDER(ID_ROTATEDATA,BasicPane2::setDataRotate)
EVT_SLIDER(ID_MININUM,BasicPane2::OnMinimun)
EVT_SLIDER(ID_MAXINUM,BasicPane2::OnMaximun)
EVT_COMMAND_RANGE(ID_RXGAIN,ID_RXGAIN+99,wxEVT_SLIDER,BasicPane2::setRxGain)
EVT_CHECKBOX(ID_CHECKAUTO,BasicPane2::OnCheckAuto)
EVT_CHECKBOX(ID_SWAPIQ,BasicPane2::OnCheckAuto)
EVT_CHECKBOX(ID_OSCILLOSCOPE,BasicPane2::OnCheckAuto)
EVT_CHECKBOX(ID_SOFTAUTOGAIN,BasicPane2::OnCheckAuto)
EVT_CHECKBOX(ID_SETGAIN,BasicPane2::OnCheckAuto)
EVT_BUTTON(ID_STARTSEND, BasicPane2::startSend)
EVT_BUTTON(ID_STOPSEND, BasicPane2::stopSend)
EVT_BUTTON(ID_ALPHA, BasicPane2::stopSend)
EVT_DATAVIEW_ITEM_ACTIVATED(ID_VIEWSELECTED, BasicPane2::OnViewSelected)
EVT_DATAVIEW_SELECTION_CHANGED(ID_VIEWSELECTED, BasicPane2::OnViewSelected)
END_EVENT_TABLE()
void dummpy12(){;}
void BasicPane2::OnViewSelected(wxDataViewEvent& event) 
{    
	event.Skip();
	
	long int nRow=(long)listctrlFreq->GetSelectedRow();
	
	//winout("nRow %ld\n",nRow);
	
	wxString nn=listctrlFreq->GetTextValue(nRow,1);

	//winout("%s\n",static_cast<const char*>(nn));
		
	const char *freq=nn;
		
	sdr->setFrequencyFC(atof(freq)*1e6);
	
	gTopPane2->Refresh();

}
void BasicPane2::startSend(wxCommandEvent& event) 
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
void BasicPane2::stopSend(wxCommandEvent& event) 
{    
	event.Skip();
	
	if(event.GetId() == ID_ALPHA){
	
		wxString value=textAlpha->GetValue();
	
		const char *alpha=value;

		lineAlpha=atof(alpha);
		
		if(lineAlpha < 0.0)lineAlpha=0.0;
		
		if(lineAlpha > 1.0)lineAlpha=1.0;
		
		gSpectrum2->lineAlpha=lineAlpha;
		
		wxString value1=rangeMin->GetValue();
	
		const char *alpha1=value1;

		gWaterFall2->pd.sPmin=atof(alpha1);
	
		wxString value2=rangeMax->GetValue();
	
		const char *alpha2=value2;

		gWaterFall2->pd.sPmax=atof(alpha2);
		
		//winout("UsePlotScales %d sPmin %g sPmax %g\n",gWaterFall2->pd.UsePlotScales,gWaterFall2->pd.sPmin,gWaterFall2->pd.sPmax);
			
		return;
	}
	
	sendFlag=0;
	
	sdr->fillBuffer=0;
	
	//winout("stopSend \n");

   // int id=event.GetId();
}

int BasicPane2::SetScrolledWindow()
{

	if(ScrolledWindow)ScrolledWindow->Destroy();
	
	ScrolledWindow = new wxScrolledWindow(gBasicPane2,ID_SCROLLED,wxPoint(0,0),
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

BasicPane2::BasicPane2(wxWindow *frame, const wxString &title,class sdrClass *sdrIn)
    : wxWindow(frame,32000)
    , m_timer(this,TIMER_ID)
{
	m_timer.Start(50);
	
	zerol((char *)sx,sizeof(sxs));
	
	sendFlag=0;
	
	lineAlpha=0.1;
	
	softAutoGain=1;
	
	sdr=sdrIn;
	
	pBasicPane2=this;
	
	gBasicPane2=pBasicPane2;
	
	gTopPane2=pTopPane2;
	
	gTopPane2->sdr=sdr;
	
	gTopPane2->gSpectrum2=pSpectrum2;
	
	gSpectrum2=pSpectrum2;
	
	gSpectrum2->sdr=sdr;
	
	gSpectrum2->gTopPane2=pTopPane2;
	
	gSpectrum2->gBasicPane2=pBasicPane2;
	
	gWaterFall2=pWaterFall2;

	gWaterFall2->sdr=sdr;
	
	gWaterFall2->gTopPane2=pTopPane2;
	
	gWaterFall2->gSpectrum2=gSpectrum2;
	
	gWaterFall2->gBasicPane2=pBasicPane2;
	
	scrolledWindowFlag=0;
	
	ScrolledWindow=NULL;
	
    SetScrolledWindow();
}

void BasicPane2::dataType(wxCommandEvent &event)
{
	event.Skip();
	
	//wxString name=event.GetString();
	
	//const char *mode=name.ToUTF8().data();

	//winout("dataType %s GetSelection %d\n",mode,event.GetSelection());
	
}
void BasicPane2::dataMode(wxCommandEvent &event)
{
	event.Skip();
	
	//wxString name=event.GetString();
	
	//const char *mode=name.ToUTF8().data();

	//winout("dataMode %s GetSelection %d\n",mode,event.GetSelection());
	
}
void BasicPane2::OnCheckAuto(wxCommandEvent &event)
{
	event.Skip();
	
	int id=event.GetId();
		
	if(id == ID_SWAPIQ){
		sdr->IQSwap=event.GetSelection();
		return;
	}else if(id == ID_SOFTAUTOGAIN){
		softAutoGain=event.GetSelection();
		gSpectrum2->softAutoGain=softAutoGain;
		return;
	}else if(id == ID_SETGAIN){
		int flag=event.GetSelection();
		if(flag){
		    char value[256];
			gWaterFall2->pd.UsePlotScales=1;
		    sprintf(value,"%g",gWaterFall2->pmin);
		    rangeMin->SetValue(value);
		    sprintf(value,"%g",gWaterFall2->pmax);
		    rangeMax->SetValue(value);
		}else{
			gWaterFall2->pd.UsePlotScales=0;	
		}
		return;
	}else if(id == ID_OSCILLOSCOPE){
		int flag=event.GetSelection();
		//winout("ID_OSCILLOSCOPE flag %d\n",flag);
		gSpectrum2->oscilloscope=flag;
		scrolledWindowFlag=flag;
		SetScrolledWindow();
		wxSize size = gSweep->GetClientSize();
		size.x += 4;
		size.y += 4;
		gSweep->SetClientSize(size);
		size.x -= 4;
		size.y -= 4;
		gSweep->SetClientSize(size);
		sdr->initPlay();
		if(flag == 1){		
		    gSpectrum2->sdr->bS2->mutex1.lock();
			gSpectrum2->sdr->bS2->bufftop=0;
			gSpectrum2->sdr->witch=0;
		    gSpectrum2->sdr->bS2->mutex1.unlock();
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


void BasicPane2::setRxGain(wxCommandEvent &event)
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
void BasicPane2::setSampleWidth(wxCommandEvent &event)
{
	event.Skip();
	
	double samplewidth=event.GetSelection();
	
	sdr->setSampleWidth(samplewidth);	
}
void BasicPane2::setDataRotate(wxCommandEvent &event)
{
	event.Skip();
	
	sampleDataRotate=event.GetSelection();
	
	sampleDataRotate=(200-sampleDataRotate)/200;
	
	Sleep2(100);
	
	//fprintf(stderr,"sampleDataRotate %g\n",sampleDataRotate);
	
}
void BasicPane2::OnTextBandWidth(wxCommandEvent &event)
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
void BasicPane2::setBandwidth(wxCommandEvent &event)
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
void BasicPane2::OnMinimun(wxCommandEvent &event)
{
	event.Skip();

	float gain=event.GetSelection();
	gSpectrum2->verticalMinimum=gain;
	//winout("min gain %g\n",gain);
	if(gain+20 > gSpectrum2->verticalMaximum)gSpectrum2->verticalMaximum=gain+20;

}
void BasicPane2::OnMaximun(wxCommandEvent &event)
{
	event.Skip();
	float gain=event.GetSelection();
	gSpectrum2->verticalMaximum=gain;
	//winout("max gain %g\n",gain);
	if(gain-20 < gSpectrum2->verticalMinimum)gSpectrum2->verticalMinimum=gain-20;

}

void BasicPane2::setSampleRate(wxCommandEvent &event)
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

void BasicPane2::OnText(wxCommandEvent &event)
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

void BasicPane2::OnScroll(wxCommandEvent &event)
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

 void BasicPane2::OnComboFilter(wxCommandEvent& event){

	event.Skip();
	
	wxString name=event.GetString();

	const char *mode=name.ToUTF8().data();

	std::string number=mode;
	
	winout("Filter %s %d\n",number.c_str(),filterCombo->GetSelection());
	
	gSpectrum2->filterType=filterCombo->GetSelection();


}
 
 void BasicPane2::render( wxPaintEvent& evt )
{
	evt.Skip();
	//winout("BasicPane2:render sdr->decodemode %d\n",sdr->decodemode);
	modeBox->SetSelection(sdr->decodemode);

}

 void BasicPane2::resized(wxSizeEvent& evt)
{
//	wxGLCanvas::OnSize(evt);
	//winout("BasicPane2::resized %d %d\n",getWidth(),getHeight());
	ScrolledWindow->SetSize(0,0,getWidth(),getHeight());
	evt.Skip();
   	Refresh();
}

int BasicPane2::getWidth()
{
    return GetSize().x;
}
 
int BasicPane2::getHeight()
{
    return GetSize().y;
}

void BasicPane2::mouseMoved(wxMouseEvent& event) {	event.Skip();}

void BasicPane2::mouseDown(wxMouseEvent& event) {	event.Skip(); winout("BasicPane2::mouseDown\n");}

/*
void BasicPane2::OnButton(wxCommandEvent& event) 
{
//    event.Skip();

	wxWindow *parent=GetParent();
	wxWindow *parent2=parent->GetParent();
	if(parent2)parent2->Destroy();
//	parent->Destroy();

}
*/

void BasicPane2::startRadio(wxCommandEvent& event) 
{    
	event.Skip();
	
}

void BasicPane2::OnTimer(wxTimerEvent &event){
		event.Skip();

	if(!gSpectrum2){
		Sleep2(10);
		return;
	}
	//static long count;
	
	//winout("BasicPane2::OnTimer count %ld gSpectrum2->iWait %d\n",count++,gSpectrum2->iWait);
	
	if(gSpectrum2->iWait){
		iRefresh=1;
		Sleep2(10);
		return;
	}
	
	//winout("BasicPane2::OnTimer gSpectrum2->iWait %d gSpectrum2->sdr %p\n",gSpectrum2->iWait,gSpectrum2->sdr);
	
	//auto t1 = std::chrono::high_resolution_clock::now();

	
	if(gSpectrum2->sdr->saveFlag){
		
		for(int n=0;n<gSpectrum2->buffSendLength;++n){
			gSpectrum2->buffSend2[2*n]=gSpectrum2->sdr->saveBuff[2*n];
			gSpectrum2->buffSend2[2*n+1]=gSpectrum2->sdr->saveBuff[2*n+1];
			//if(fabs(buff[2*n]) > amax)amax=fabs(buff[2*n]);
		}
		
		//winout("sdr->saveFlag %d buffSendLength %d buffFlag %d\n",sdr->saveFlag,buffSendLength,buffFlag);
		
		gSpectrum2->buffFlag=1;
		
	    gSpectrum2->sdr->saveFlag=0;
	}else{
		//winout("Save Not Set\n");
	}
	
	if(iRefresh){
		iRefresh=0;
		gTopPane2->Refresh();
		Refresh();
		if(ScrolledWindow){
			ScrolledWindow->Refresh();
		}
		//winout("gTopPane2->Refresh called f %g sdr->decodemode %d\n",sdr->f,sdr->decodemode);
	}
	
	if(gSpectrum2->buffFlag){
		gSpectrum2->Refresh();
		gWaterFall2->Refresh();
	}else{
	 // winout("OnTimer Skip call\n");
	}
	
	//auto t2 = std::chrono::high_resolution_clock::now();
	//std::chrono::duration<double> difference = t2 - t1;
	//winout("Time in OnTimer %g\n",difference.count());

	
	
}



void BasicPane2::OnComboSampleRate(wxCommandEvent& event)
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

void BasicPane2::OnComboAntenna(wxCommandEvent& event)
{
		event.Skip();
		
		wxString antennas=event.GetString();
		
		const char *antenna=antennas.c_str();
		
		//winout("antenna %s len %d\n",antenna,(int)strlen(antenna));
		
		sdr->device->setAntenna(SOAPY_SDR_RX, sdr->channel, antenna);

}

void BasicPane2::OnCombo(wxCommandEvent& event){
	//int ngroup=event.GetSelection();
//	winout("OnCombo %d IsChecked %d\n",item,event.IsChecked());
//	noteCheckBox=event.IsChecked();

		event.Skip();
		
		
	//winout("BasicPane2::OnCombo\n");

	wxString number=event.GetString();
	
	double value;
	
	number.ToCDouble(&value);
	

    if(!gSpectrum2->buffSend10){
    	winout("Radio Not Running\n");
    	return;
	}
	
    gSpectrum2->iWait=1;
	Sleep2(20);
	gSpectrum2->sdr->saveCall=0;
	Sleep2(20);
	gSpectrum2->buffSendLength=0;
	gSpectrum2->buffFlag=1;
	
	int np=(int)value;
	

	if(gSpectrum2->buffSend)cFree((char *)gSpectrum2->buffSend);
	gSpectrum2->buffSend=(float *)cMalloc(sizeof(float)*8*np,9588);
	
	if(gSpectrum2->buffSend2)cFree((char *)gSpectrum2->buffSend2);
	gSpectrum2->buffSend2=(float *)cMalloc(sizeof(float)*8*np,95889);
	
	if(gSpectrum2->buffSend10)cFree((char *)gSpectrum2->buffSend10);
	gSpectrum2->buffSend10=(float *)cMalloc(sizeof(float)*8*np,9889);

	
	memset(gSpectrum2->buffSend10, 0, sizeof(float)*8*np);
		
	
	gSpectrum2->p1 = fftwf_plan_dft_1d(np,(fftwf_complex *)gSpectrum2->buffSend2, (fftwf_complex *)gSpectrum2->buffSend, FFTW_FORWARD, FFTW_ESTIMATE);


	gSpectrum2->sdr->setDataSave(np);
	

	gSpectrum2->buffFlag=0;
	
	gSpectrum2->iWait=0;
	
	gSpectrum2->buffSendLength=np;


//	winout("np %d iWait %d saveCall %d iWait %p\n",np,gSpectrum2->iWait,gSpectrum2->sdr->saveCall,&gSpectrum2->iWait);
	
}


void BasicPane2::OnButton2(wxCommandEvent& event) 
{    
	event.Skip();

}
void BasicPane2::radioBox(wxCommandEvent& event) 
{
	event.Skip();
	
	wxString name=event.GetString();
	
	const char *mode=name.ToUTF8().data();
	
	sdr->setMode(mode);
	
}
BasicPane2::~BasicPane2()
{
	//winout("exit1 BasicPane2 %p thread %llx\n",this,(long long)std::this_thread::get_id);
	if(sdr)delete sdr;
	sdr=NULL;
	//winout("exit2 BasicPane2 %p thread %llx\n",this,(long long)std::this_thread::get_id);
}
BEGIN_EVENT_TABLE(WaterFall2, wxGLCanvas)
EVT_MOTION(WaterFall2::mouseMoved)
EVT_PAINT(WaterFall2::render)
EVT_SIZE(WaterFall2::resized)
EVT_LEFT_DOWN(WaterFall2::mouseDown)
EVT_MOUSEWHEEL(WaterFall2::mouseWheelMoved)
EVT_CHAR(WaterFall2::OnChar)    

END_EVENT_TABLE()

void WaterFall2::OnChar(wxKeyEvent& event) 
{
	extern soundClass *s;

	event.Skip();
	
    int keycode=event.GetKeyCode();
    
   // winout("WaterFall2::OnChar %d\n",keycode);
    
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

void WaterFall2::mouseWheelMoved(wxMouseEvent& event)
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
		gTopPane2->Refresh();
//		winout("bw %g %ld\n",sdr->bw,(long)event.ControlDown());
	}
}
void WaterFall2::mouseDown(wxMouseEvent& event) 
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
		gTopPane2->Refresh();

	}
}
// some useful events to use
void WaterFall2::mouseMoved(wxMouseEvent& event) {
	//winout("mouseMoved \n");
		event.Skip();

}
WaterFall2::~WaterFall2()
{
	if(m_context)delete m_context;
	
	if(water.data)cFree((char *)water.data);
	water.data=NULL;

	//winout("exit WaterFall2 %p\n",this);
}

int WaterFall2::getWidth()
{
    return (int)(GetSize().x*scaleFactor);
}
 
int WaterFall2::getHeight()
{
    return (int)(GetSize().y*scaleFactor);
}
void WaterFall2::resized(wxSizeEvent& evt)
{
	evt.Skip();
	
	scaleFactor=GetContentScaleFactor();

	SetWindow();
	
   	Refresh();
}
int WaterFall2::ftox(double frequency){

	int x;
	
	x=(int)(getWidth()*(frequency-sdr->fw+0.5*sdr->samplewidth)/sdr->samplewidth);

	return x;
}

void WaterFall2::render( wxPaintEvent& evt )
{
	evt.Skip();
//   winout("WaterFall2::render\n");

    if(!IsShown()) return;
    
    if(iWait)return;
    
    float *magnitude=gSpectrum2->buffSend10;
    
    int length=gSpectrum2->buffSendLength;
    
    //auto t1 = chrono::high_resolution_clock::now();
    

   // winout("WaterFall2 render 2\n");
        
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

//	winout("WaterFall2 f %p\n",&sdr->f);

    	if(range)delete [] range;
    	range=NULL;
    	if(wateric)delete [] wateric;
    	wateric=NULL;

    	glFlush();
    	SwapBuffers();
    }
int WaterFall2::SetWindow()
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
    
  //  winout("WaterFall2::SetWindow\n");
    
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

 int WaterFall2::FloatToImage(float *d,long length,struct paletteDraw *pd,unsigned char *bp)
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

WaterFall2::WaterFall2(wxFrame* parent, int* args) :
    //wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxSize(800,600), wxFULL_REPAINT_ON_RESIZE)
   // wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
   // wxGLCanvas(parent, wxID_ANY, args, wxPoint(200,0), wxSize(800,200), wxFULL_REPAINT_ON_RESIZE)
      wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxSize(400,100), wxFULL_REPAINT_ON_RESIZE)
{

	verticalMinimum=-130;
		
	verticalMaximum=-30;
	
	water.data=NULL;

	pWaterFall2=this;
	
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


BEGIN_EVENT_TABLE(Spectrum2, wxGLCanvas)
EVT_MOTION(Spectrum2::mouseMoved)
EVT_LEFT_DOWN(Spectrum2::mouseDown)
EVT_LEFT_UP(Spectrum2::mouseReleased)
EVT_RIGHT_DOWN(Spectrum2::rightClick)
EVT_LEAVE_WINDOW(Spectrum2::mouseLeftWindow)
EVT_SIZE(Spectrum2::resized)
//EVT_KEY_DOWN(Spectrum2::keyPressed)
//EVT_KEY_UP(Spectrum2::keyReleased)
EVT_MOUSEWHEEL(Spectrum2::mouseWheelMoved)
EVT_PAINT(Spectrum2::render)
EVT_CHAR(Spectrum2::OnChar)    
//EVT_BUTTON(ID_DELETE,Spectrum2::DeleteRow )
EVT_IDLE(Spectrum2::OnIdle)
END_EVENT_TABLE()

void Spectrum2::OnIdle(wxIdleEvent& event)
{	
	event.Skip();
	if(oscilloscope == 1){
	    //fprintf(stderr,"Spectrum2::OnIdle\n");
		Refresh();
	}
}

// some useful events to use
void Spectrum2::mouseMoved(wxMouseEvent& event) {
	//winout("mouseMoved \n");
	event.Skip();
}
void Spectrum2::mouseReleased(wxMouseEvent& event) {event.Skip();}
void Spectrum2::rightClick(wxMouseEvent& event) {event.Skip();}
void Spectrum2::mouseLeftWindow(wxMouseEvent& event) {event.Skip();}
void Spectrum2::keyReleased(wxKeyEvent& event) {event.Skip();}

void Spectrum2::mouseWheelMoved(wxMouseEvent& event)
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
	
		gTopPane2->Refresh();
//		winout("bw %g %ld\n",sdr->bw,(long)event.ControlDown());
	}

}

void Spectrum2::OnChar(wxKeyEvent& event) 
{
	event.Skip();
	
	int keycode=event.GetKeyCode();
	
	//winout("Spectrum2::OnChar %d\n",keycode);
	
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
  
extern int InitGL;



Spectrum2::Spectrum2(wxFrame* parent, int* args) :
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
	
	pSpectrum2=this;
	

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
  
	//Bind(wxEVT_CHAR, &Spectrum2::OnChar, this);    
}



void Spectrum2::InitOpenGl()
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
double Spectrum2::ftime()
{
	//auto t1=startTime->now();
	//std::chrono::time_point<std::chrono::system_clock> tt = std::chrono::system_clock::now();
	auto t2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> t5 = t2.time_since_epoch();
	//std::chrono::duration<double> difference = t2-t2;
	return t5.count();
}

void Spectrum2::startRadio2() 
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


void Spectrum2::keyPressed(wxKeyEvent& event) 
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
int Spectrum2::doTestSpeed()
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

Spectrum2::~Spectrum2()
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
	//winout("exit Spectrum2 %p\n",this);

}

void Spectrum2::resized(wxSizeEvent& evt)
{
//	wxGLCanvas::OnSize(evt);
	//winout("Spectrum2::resized %d %d\n",getWidth(),getHeight());
	evt.Skip();
	scaleFactor=GetContentScaleFactor();
	Refresh();
}
 
 
int Spectrum2::getWidth()
{
    return (int)(GetSize().x*scaleFactor);
}
 
int Spectrum2::getHeight()
{
    return (int)(GetSize().y*scaleFactor);
}
void Spectrum2::mouseDown(wxMouseEvent& event) 
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
			//winout("Spectrum2::mouseDown %p %p\n",s->bS,sdr->bS);
			gTopPane2->Refresh();
		}
	}else{
		winout("Spectrum2::mouseDown x %d y %d\n",pp3.x,pp3.y);
	}

}

int Spectrum2::ftox(double frequency){

	int x;
	
	x=(int)(getWidth()*(frequency-sdr->fw+0.5*sdr->samplewidth)/sdr->samplewidth);

	return x;
}
void Spectrum2::render(wxPaintEvent& evt )
{
	if(oscilloscope == 1){
		render1(evt);	
	}else{
		render2(evt);
	}
}

void Spectrum2::render1(wxPaintEvent& evt )
{
	evt.Skip();
	
	//winout("Spectrum2 render nc %lld\n",nc++);

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
				if(gBasicPane2->sampleDataRotate > 0.0){
					//fprintf(stderr,"gBasicPane->sampleDataRotate \n");
					//gBasicPane->sampleDataRotate=0.5;
					int nn=gBasicPane2->sampleDataRotate*(sdr->size-1)+k;
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
				if(gBasicPane2->sampleDataRotate > 0.0){
					//fprintf(stderr,"gBasicPane->sampleDataRotate \n");
					//gBasicPane->sampleDataRotate=0.5;
					int nn=gBasicPane2->sampleDataRotate*(sdr->size-1)+k;
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

		//winout("Spectrum2 render 1 count %ld amax %g ip %d num %d amax2 %g cosdt %g sindt %g  coso %g sino %g\n",count1++,
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

	//winout("Spectrum2 render 1 count %ld amax %g iWait %d\n",count1++,amax,iWait);

	//fprintf(stderr,"Next 77\n");
		
    
 
}
 
void Spectrum2::render2(wxPaintEvent& evt )
{
	evt.Skip();
	
	//static long long nc=0;
/*
	static int tick=0;
	tick++;
	tick=10;
*/
	//winout("Spectrum2 render nc %lld\n",nc++);

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

	//winout("Spectrum2 done\n");
		
}

void Spectrum2::render1a(wxPaintEvent& evt )
{
	evt.Skip();
	
	//winout("Spectrum2 render nc %lld\n",nc++);

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

	//winout("Spectrum2 done\n");

	//fprintf(stderr,"Next 77\n");
		
    glFlush();
    SwapBuffers();    
    
 
}



