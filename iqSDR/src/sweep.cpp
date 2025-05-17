#include "iqSDR.h"
#include "WarningBatch.h"
#include <string.h>
#include <vector>
#include <string>
#include <time.h>

int DFerror;


void winout(const char *fmt, ...);

int copyl(char *p1,char *p2,long n);

void *cMalloc(unsigned long r, int tag);

void checkall(void);

extern StartWindow *startIt;

extern char WarningBuff[256];

extern soundClass *s;

extern std::string ProgramVersion;

int fft(double *data,int nn,int isign);
int doFFT2(double *x,double *y,long length,int direction);
int doWindow(double *x,double *y,long length,int type);
int writesds(struct SDS2Dout *sdsout);
static int writesds2dFloat(struct SDS2Dout *sdsout);
static int writesds2dBytes(struct SDS2Dout *sdsout);
static int writesds2dDouble(struct SDS2Dout *sdsout);
static int writesds3dFloat(struct SDS2Dout *sdsout);
static int writesds3dDouble(struct SDS2Dout *sdsout);
int Warning(char *mess);

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
EVT_MENU(ID_OPEN, Sweep::OpenFile)
wxEND_EVENT_TABLE()

int checkOrder(unsigned char *cp,long length)
{
	if(!cp || length <= 0)return 1;
#ifdef PC
	{
		unsigned char c;
	    long np;
	    for(np=0;np<length;np += 4){
		    c=cp[np];
		    cp[np]=cp[np+3];
		    cp[np+3]=c;
		    c=cp[np+1];
		    cp[np+1]=cp[np+2];
		    cp[np+2]=c;
	    }
	}
#endif
	return 0;
}
int namesize(char *name)
{
	char *np;
	int n;

	if(!(np=strrchr(name,'/'))){
	    np=name;
	}else{
	    np += 1;
	}

	n=(int)strlen(np);

	return n;
}

void Sweep::openSweepFile(char *name)
{

	int ixmax,iymax,izmax;
	long size;
	char nameout[256];
	//extern int DFerror;
	int lastref,rank,maxrank=3;
	int32 dimsizes[3];


	//winout("file %s\n",name);
	
	
	DFSDclear();
	DFSDrestart();
   // DFSDsetNT(DFNT_FLOAT32);

	if(DFSDgetdims(name,&rank,dimsizes,maxrank) != -1) {
		ixmax = dimsizes[0];
		size = ixmax;
		if (rank>1) {
			iymax = dimsizes[1];
			size *= iymax;
		}
		if (rank>2) {
			izmax = dimsizes[2];
			size *= izmax;
		}
		size *= sizeof(double);
		
		float *buffin=(float *)cMalloc(size,2399);
		if(!buffin){
			fprintf(stderr,"openSweepFile out of Memory\n");
			return;
		}
		
		double *data=(double *)cMalloc(size,2399);
		if(!data){
			fprintf(stderr,"openSweepFile out of Memory\n");
			return;
		}
		
		
		if(DFSDgetdata(name,rank,dimsizes,buffin)==-1){
			fprintf(stderr,"Error (%d)  DFSDgetdata\n",DFerror);
			return;
		}
		
		double amin=1e33;
		double amax=-1e33;
		
		for(int n=0;n<ixmax*iymax;++n){
			float v=buffin[n];
			data[n]=buffin[n];
			if(v > amax)amax=v;
			if(v < amin)amin=v;
		}
		
		//fprintf(stderr,"amin %g amax %g\n",amin,amax);
		
		strcpy(nameout,name);
				
		lastref=DFSDlastref();
		if(lastref != -1) {
			DFANgetlabel(name,700,lastref,nameout,256);			
		}
		
		if(checkOrder((unsigned char *)buffin,size))return;
		
		int gotName=0;
		if(lastref != -1){
			zerol((char *)nameout,256L);
			if(DFANgetdesc(name,700,lastref,nameout,256) != -1){
				gotName=1;
			}
		}		
		double xmin,xmax;
		double ymin,ymax;
		double vmin,vmax;
		double zmin,zmax;
		
		if(gotName){
			sscanf(nameout,"xmin %lf ymin %lf zmin %lf xmax %lf ymax %lf zmax %lf vmin %lf vmax %lf",
			      &xmin,&ymin,&zmin,&xmax,&ymax,&zmax,&vmin,&vmax);
			fprintf(stderr,"xmin %g xmax %g\n",xmin,xmax);
			fprintf(stderr,"ymin %g ymax %g\n",ymin,ymax);
			fprintf(stderr,"vmin %g vmax %g\n",vmin,vmax);

		}else{
			return;	
		}
		
		rx->sweepLower=xmin;
		rx->sweepUpper=xmax;

		sdsout.path=(char *)name;
		//sdsout.path=rx->s2dName;
		sdsout.name=(char *)"Power Sweep";
		sdsout.ixmax=iymax;
		sdsout.iymax=ixmax;
		sdsout.xmin=xmin;
		sdsout.xmax=xmax;
		sdsout.ymin=ymin;
		sdsout.ymax=ymax;
		sdsout.zmin=zmin;
		sdsout.zmax=zmax;
		sdsout.time=0.0;
		sdsout.n=0;
		sdsout.pioName=(char *)"Power(db)";	
		sdsout.type=DATA_TYPE_FLOAT;	
		if(sdsout.data)cFree((char *)sdsout.data);
		sdsout.data=(double *)data;
		
		//fprintf(stderr,"sds->data %p sds->ixmax %ld\n",sdsout.data,sdsout.ixmax);
		
		gSpectrum2->nrow=0;
		
		if(buffin)cFree((char *)buffin);
		buffin=NULL;
		
		//winout("gotName %d name %s nameout %s\n",gotName,name,nameout);
	}
	
}

void Sweep::OpenFile(wxCommandEvent &event)
{
	static int index=0;
    
    wxFileDialog filedlg(this, _("Open File"), "", "",
                       "I/Q files (*.raw)|*.raw|"
                       "Wav files (*.wav)|*.wav|"
                       "Sweep files (*.s2d)|*.s2d|"
                       "TexT files (*.txt)|*.txt" 
                       , wxFD_OPEN|wxFD_FILE_MUST_EXIST);
                         
    filedlg.SetFilterIndex(index);

    if (filedlg.ShowModal() == wxID_OK)
    {
    
        
        wxString name = filedlg.GetPath();
        
        const char *file=name.ToUTF8().data();
        
        int Index=filedlg.GetFilterIndex();
        
       // winout("OpenFile file %s Index %d\n",file,Index);
        
        if(Index == 0)
        {     
    		//openIQFile(file);
    	} 
    	else if(Index == 1)
    	{
    		//openWavFile(file);
    	}
    	else if(Index == 2)
    	{
    		openSweepFile((char *)file);
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
    
    menuFile->Append(ID_OPEN, _T("&Open...\tCtrl-O"), _T("Open..."));
    
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

    sdr->device->writeSetting("direct_samp",value);
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
    
    gSpectrum2=pSpectrum2;
    
    pBasicPane2->gSweep=this;
    
    pSpectrum2->gSweep=this;
    
    pWaterFall2->gSweep=this;

    m_gbs->AddGrowableCol(1);
    m_gbs->AddGrowableCol(2);
    m_gbs->AddGrowableRow(1);
    m_gbs->AddGrowableRow(2);
    m_gbs->AddGrowableRow(3);
    m_gbs->AddGrowableRow(4);

    this->SetSizerAndFit(m_gbs);
    SetClientSize(this->GetSize());

    pSpectrum2->iWait=1;

	rx=&rxs;
	zerol((unsigned char *)&rxs,(&rxs.end-&rxs.start)+1);
	threadexit=0;
	FFTlength=32768;
	rx->FFTcount=4096;
	
	magnitude2=(double *)cMalloc(FFTlength*sizeof(double),9854);
	if(!magnitude2){
		winout("Error magnitude2\n");
		return;
	}
	
	zerol((char *)&sdsout,sizeof(struct SDS2Dout));

    lineDumpInterval=0.1;
    lineTime=rtime()+lineDumpInterval;

    //frame7->Show();
}

#ifdef _MSC_VER
struct tm *localtime_r (const time_t *timer, struct tm *result)
{
    struct tm *local_result = localtime (timer);
    if (local_result == NULL || result == NULL) return NULL;
    memcpy (result, local_result, sizeof (struct tm));
    return result;
}
#endif

int Sweep::sweepRadio()
{

	gSpectrum2->iWait=1;
	
	mprint("sweepLower %g MHZ sweepUpper %g MHZ sweepSize %g crop %g\n",rx->sweepLower,rx->sweepUpper,rx->sweepSize,rx->crop);
	
	rx->sweepLower *= 1e6;
	
	rx->sweepUpper *= 1e6;
	
	double rateSave=sdr->device->getSampleRate(SOAPY_SDR_RX, rx->channel);
	
	double fw=sdr->fw;
	double fc=sdr->fc;
	double samplewidth=sdr->samplewidth;
	
	sdr->setSampleWidth(rx->samplerate);
	
		
	double rate=sdr->device->getSampleRate(SOAPY_SDR_RX, rx->channel);
	
	rx->samplerate=rate;
	
	int saveLength=sdr->saveLength;
	sdr->setDataSave(rx->FFTcount);
	
	double lower=  1e33;
	double upper= -1e33;
	
	for(double n=rx->sweepLower;n<=rx->sweepUpper;n += rx->samplerate*(1.0-rx->crop)){
		double fc,fmins,fmaxs;
		fc=n+rx->samplerate*0.5*(1.0-rx->crop);
		fmins=fc-rx->samplerate*0.5*(1.0-rx->crop);
		fmaxs=fc+rx->samplerate*0.5*(1.0-rx->crop);
		if(fmins < lower)lower=fmins;
		if(fmaxs > upper)upper=fmaxs;
	}	
	
	rx->sweepLower2=lower;
	rx->sweepUpper2=upper;
	
	
	int counts=0;
	for(double n=rx->sweepLower2;n<=rx->sweepUpper2;n += rx->sweepSize){
		++counts;
	}
	
	mprint("steps %g counts %d\n",(upper-lower)/rx->sweepSize,counts);
	

	mprint("sweep Steps %d lower %g upper %g \n",counts,lower,upper);
	
	rx->sweepBuff=(float *)cMalloc(2*(counts+10)*sizeof(float),5790);
	if(!rx->sweepBuff){
		mprint("56 cMalloc Errror %ld\n",(long)(2*(counts+10)*sizeof(float)));
		sdr->setDataSave(saveLength);
		return 1;
	}

	zerol((char *)rx->sweepBuff,2*(counts+10)*sizeof(float));
  
	rx->ncut=sdr->ncut;
	
	int size=(int)rate/rx->ncut;

	rx->size=size;

	mprint("rate %f rx->size %d\n",rate,rx->size);
	
	//rx->cs->setBuff(rx);


	rx->frame=0;
	
//	rx->doWhat=Work;
	
	sdr->iWait=0;
	
	Sleep2(100);

	std::vector<float> db;
	std::vector<std::vector<float>> list;
	list.clear();
	
	
	//sdr->device->setFrequency(SOAPY_SDR_RX,rx->channel,"RF",lower);
	sdr->device->setFrequency(SOAPY_SDR_RX,rx->channel,lower);
	
	int itWas=-1;
	int pass2=0;
	while(1){
		if(itWas != sdr->witch){
			itWas=sdr->witch;
			if(pass2++ >= 11)break;
		}else{
			Sleep2(5);
		}
	}

	itWas=-1;
	int pass=0;
	threadexit=0;
  	while(!threadexit){	
		char t_str[50];
		time_t time_now;
		struct tm cal_time = {0};
		time_now = time(NULL);
		mprint("sweep %d\n",pass++);
		for(double n=rx->sweepLower;n<=rx->sweepUpper;n += rx->samplerate*(1.0-rx->crop)){
			localtime_r(&time_now, &cal_time);
		    strftime(t_str, 50, "%Y-%m-%d, %H:%M:%S,", &cal_time);
			double fc,fmins,fmaxs;
			fc=n+rx->samplerate*0.5*(1.0-rx->crop);
			fmins=fc-rx->samplerate*0.5*(1.0-rx->crop);
			fmaxs=fc+rx->samplerate*0.5*(1.0-rx->crop);
		    mprint("fc %g fmins %g fmaxs %g\n",fc/1e6,fmins/1e6,fmaxs/1e6);
		    if(fmins < lower)lower=fmins;
		    if(fmaxs > upper)upper=fmaxs;
			rx->fc=fc;
			//sdr->device->setFrequency(SOAPY_SDR_RX,rx->channel,"RF",rx->fc);
			sdr->device->setFrequency(SOAPY_SDR_RX,rx->channel,rx->fc);
			Sleep2(100);
			rx->aminGlobal3=0;
			int pass2=0;
			db.clear();
			zerol((char *)magnitude2,FFTlength*sizeof(double));
			while(1){
				if(itWas != sdr->witch){
					itWas=sdr->witch;
				    if(sdr->retFlag <= 0){
				        fprintf(stderr,"Skip Bad Read Data\n");
				        continue;
				    }
					updateSweep1(fmins,fmaxs);
					if(pass2++ >= 10)break;
				}else{
					Sleep2(5);
				}
			}
			//if(fc > 769e6 && fc < 771e6)zerol((char *)magnitude2,FFTlength*sizeof(double));
			updateSweep2(fmins,fmaxs,pass2);
			if(rx->out){
			    //fprintf(stderr,"rx->sweepFound %d\n",rx->sweepFound);
			    fprintf(rx->out,"%s ",t_str);
			    fprintf(rx->out," %.0f, %.0f, %.2f, %.0f,",fmins,fmaxs,rx->sweepSize,(double)rx->samplerate/(double)rx->ncut);
			    for(int n=0;n<rx->sweepFound;++n){
			        fprintf(rx->out," %.2f, ",rx->sweepBuff[n]);
			        db.push_back(rx->sweepBuff[n]);
			    }
			    fprintf(rx->out,"\n");
			    list.push_back(db);
			}else{
			    for(int n=0;n<rx->sweepFound;++n){
			        db.push_back(rx->sweepBuff[n]);
			    }
			    list.push_back(db);			
			}

		}
 	    mprint("\n");
		
   	}      
    //rx->doWhat=Wait;
    
    sdr->iWait=1;
  
    rx->frame=-1;

	if(rx->sweepBuff)cFree((char *)rx->sweepBuff);
	
	rx->buffout=NULL;

	if(list.size() > 0){
		long ysize=list.size();
		long xsize=0;
		for(int nl=0;nl<ysize;++nl){
		   // fprintf(stderr,"size %ld\n",(long)list[nl].size());
			if(list[nl].size() > xsize)xsize=(long)list[nl].size();
		}
		
		int shift=ysize/pass;
		
		fprintf(stderr,"xsize %ld ysize %ld pass %d shift %d\n",xsize,ysize,pass,shift);
				
		rx->buffout=(double *)cMalloc(xsize*ysize*sizeof(double),7777);
		if(!rx->buffout){
			fprintf(stderr,"Out of Memory\n");
			sdr->setDataSave(saveLength);
			return 1;
		}
		
		double xmin=1e33;
		double xmax=-1e33;
		
		int xsizel=xsize*shift;
		int ysizel=ysize/shift;
		int ny,nx;
		
		nx=0;
		ny=0;
		for(int nl=0;nl<ysize;++nl){
		  //  fprintf(stderr,"size %ld\n",(long)list[nl].size());
			for(int nc=0;nc<xsize;++nc){
				double f;
				if(nc == 0)f=0.0;
				if(nc < list[nl].size()){
					f=list[nl][nc];
				}
				rx->buffout[nx++ +ny*xsizel]=f;				
				if(f > xmax)xmax=f;
				if(f < xmin)xmin=f;
			}
			if(nx >= xsizel){
			    nx=0;
			    ny++;
			}
		}
		
   		char fname[256];
   		
		struct tm today;

   		time_t now;

    	time(&now);  /* get current time; same as: now = time(NULL)  */
    
    	today = *localtime(&now);
    	
    	sprintf(fname,"sweepFile_%4d%02d%02d_%02d%02d%02d.s2d%c\n",
    					today.tm_year+1900,today.tm_mon+1,today.tm_mday,
    					today.tm_hour,today.tm_min,today.tm_sec,'\0');
 				
		sdsout.path=(char *)fname;
		//sdsout.path=rx->s2dName;
		sdsout.name=(char *)"Power Sweep";
		sdsout.ixmax=xsizel;
		sdsout.iymax=ysizel;
		sdsout.xmin=lower;
		sdsout.xmax=upper;
		sdsout.ymin=0;
		sdsout.ymax=ysizel;
		sdsout.zmin=0;
		sdsout.zmax=0;
		sdsout.time=0.0;
		sdsout.n=0;
		sdsout.pioName=(char *)"Power(db)";	
		sdsout.type=DATA_TYPE_FLOAT;	
		if(sdsout.data)cFree((char *)sdsout.data);
		sdsout.data=rx->buffout;
			
		if(writesds(&sdsout))goto OutOfHere;
	
	}
 	
	
OutOfHere:

	sdr->setDataSave(saveLength);
	sdr->setSampleWidth(rateSave);
    sdr->fw=fw;
    sdr->samplewidth=samplewidth;
    sdr->setFrequencyFC(fc);
    gSpectrum2->iWait=0;

    //sdr->iWait=0;

	//if(buffout)cFree((char *)buffout);
	
	return 0;
}

int Sweep::updateSweep1(double fmins,double fmaxs)
{
    double *real,*imag;
    double amin,amax,v;
    
    if(!rx)return 0;
    
    if(rx->FFTcount > FFTlength){
        fprintf(stderr," FFTlength %ld\n",FFTlength);
        return 1;
    }
    
    int length=rx->FFTcount;
    
    for(int k=0;k<length;++k){
        rx->reals[k]=sdr->saveBuff[2*k];
        rx->imags[k]=sdr->saveBuff[2*k+1];
    }
    
    sdr->saveFlag=0;

    real=rx->reals;
    imag=rx->imags;
    
     
    doWindow(real,imag,length,rx->FFTfilter);
    
    double *data=rx->data;
    
    for(int n=0;n<length;++n){
        data[2*n]=real[n]*pow(-1.0,n);
        data[2*n+1]=imag[n]*pow(-1.0,n);
    }
        
    fft(data,(int)length,1);

       
    amin =  1e33;
    amax = -1e33;

    
     for(int n=0;n<length;++n){
        v=(data[2*n]*data[2*n]+data[2*n+1]*data[2*n+1]);
        //if(v > 0.0)v=10*log10(v);
        magnitude2[length-n-1] += v;
        if(v < amin)amin=v;
        if(v > amax)amax=v;
    }
    
   // fprintf(stderr,"fmins %g fmaxs %g amin %g amax %g\n",fmins/1e6,fmaxs/1e6,amin,amax);
     
    
    return 0;
}
int Sweep::updateSweep2(double fmins,double fmaxs,int pass)
{
    
    if(!rx)return 0;
    
    rx->sweepFound=0;
	int counts=0;
	for(double n=rx->sweepLower2;n<=rx->sweepUpper2;n += rx->sweepSize){
	    double ff=n+0.5*rx->sweepSize;
		++counts;
		if(ff < fmins || ff > fmaxs){
			//fprintf(stderr,"skip - ff %g fmins %g fmaxs %g \n",ff/1e6,fmins/1e6,fmaxs/1e6);
			continue;
		}
		
		int n1=fftIndex(ff-0.5*rx->sweepSize);
		int n2=fftIndex(ff+0.5*rx->sweepSize);
		//if(n1 == rx->FFTcount/2 || n2 == rx->FFTcount/2)continue;
		if(n1 < 0 || n2 < 0){
			fprintf(stderr,"skip - ff %g fc %g samplerate %d n1 %d n2 %d\n",ff/1e6,rx->fc/1e6,rx->samplerate,n1,n2);
		    continue;
		}
		double maxs=0.0;
		int nv=0;
		for(int m=n1;m<=n2;++m){
 	    	maxs += magnitude2[m];
 			++nv;
 		}
 		
 		maxs=maxs/((double)pass*nv);
 		if(maxs > 0.0)maxs=10*log10(maxs); 		
		rx->sweepBuff[rx->sweepFound++]=maxs;
		//fprintf(stderr,"%g %g n1 %d n2 %d maxs %g\n",n,ff/1e6,n1,n2,maxs);
		//fprintf(stderr,"ff %g maxs %g rx->sweepFound %d pass*nv %d n1 %d n2 %d\n",ff/1e6,maxs,rx->sweepFound,pass*nv,n1,n2);
	}
	
	return 0;
}
int Sweep::fftIndex(double frequency)
{
    if(!rx)return -1;
    double fac=((frequency - rx->fc)+0.5*rx->samplerate)/((double)rx->samplerate);
    int index=(int)((rx->FFTcount-1)*fac);
    if(index >= 0 && index < rx->FFTcount)return index;
    fprintf(stderr,"frequency %g fc %g index %d %g\n",frequency/1e6,rx->fc,index,fac);
    return -1;
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
	
	if(sdr->startPlay())return;
	
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
	
	if(sdr->startPlay())return;
	
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
	
	if(sdr->startPlay())return;
		
	std::thread(&sdrClass::run,sdr).detach();

}

Sweep::~Sweep()
{
//	winout("Sweep::~Sweep\n");
	
	if(magnitude2)cFree((char *)magnitude2);
	magnitude2=NULL;
	if(sdsout.data)cFree((char *)sdsout.data);
	sdsout.data=NULL;
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
EVT_MOUSEWHEEL(TopPane2::mouseWheelMoved) 
END_EVENT_TABLE()
void TopPane2::mouseWheelMoved(wxMouseEvent& event)
{
	event.Skip();
	
	int rot=event.GetWheelRotation();
	if(rot > 0){
		rot=0;
	}else if(rot < 0){
		rot = 1;
	}	
		
	wxPoint p = event.GetLogicalPosition(wxClientDC(this));

//	fprintf(stderr,"rot %d p.x %d p.y %d\n",rot,p.x,p.y);
//	int yh=fontSize.GetHeight()/2-10;
	int diff=p.x-pt.x;
	nchar=(diff)/fontSize.x;
	if(diff >= 0 && nchar >=0 && nchar <= 14){
		if(nchar == 3 || nchar == 7 || nchar == 11)return;
		int up=rot;
		//if(p.y > yh)up=0;
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
			if(fl <= 0.5*sdr->samplerate)fl=0.5*sdr->samplerate;
			sdr->setFrequencyFC((double)fl);
		}else{
			if(fl <= 0.0)fl=0.5*sdr->samplerate;
			sdr->setFrequency((double)fl);
		}
		
//		winout("TopPane::mouseDown f %lld k %d nchar %d up %d %f\n",fl,k,nchar,up,value);
		Refresh();

	}
	

}
void TopPane2::OnChar(wxKeyEvent& event) 
{
	event.Skip();
	
	int keycode=event.GetKeyCode();
	
	//winout("TopPane2::OnChar %d\n",keycode);
	
    if(keycode == 'f' || keycode == ' '){
        gSpectrum2->iFreeze = !gSpectrum2->iFreeze;
     //   winout("iWait %d\n",iWait);
    }
	
    if(keycode == 's'){
        gSpectrum2->iFreeze = 1;
        gSpectrum2->iHaveData=0;
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
		//fprintf(stderr,"fl %lld\n",fl);
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
EVT_TEXT_ENTER(ID_COMBOSAMPLERATE, BasicPane2::OnText)
EVT_TEXT_ENTER(ID_XMIN, BasicPane2::OnText)
EVT_TEXT_ENTER(ID_XMAX, BasicPane2::OnText)
EVT_TEXT_ENTER(ID_BANDWIDTHOVERIDE, BasicPane2::radioBox)
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
EVT_BUTTON(ID_SWEEP, BasicPane2::stopSend)
EVT_BUTTON(ID_SWEEPSTOP, BasicPane2::stopSend)
EVT_DATAVIEW_ITEM_ACTIVATED(ID_VIEWSELECTED, BasicPane2::OnViewSelected)
EVT_DATAVIEW_SELECTION_CHANGED(ID_VIEWSELECTED, BasicPane2::OnViewSelected)
EVT_SLIDER(ID_OSCILLOSCOPEZOOM,BasicPane2::setSampleWidth)
EVT_SLIDER(ID_OSCILLOSCOPESLIDE,BasicPane2::setDataRotate)

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
	}else if(event.GetId() == ID_SWEEP){
	
		wxString value=sweepLower->GetValue();
	
		const char *alpha=value;

		gSweep->rx->sweepLower=atof(alpha);

		value=sweepUpper->GetValue();
	
		alpha=value;

		gSweep->rx->sweepUpper=atof(alpha);

		value=sweepSize->GetValue();
	
		alpha=value;

		gSweep->rx->sweepSize=atof(alpha);

		value=sweepCrop->GetValue();
	
		alpha=value;

		gSweep->rx->crop=atof(alpha);
	
		winout("ID_SWEEP %g %g %g %g\n",gSweep->rx->sweepLower,gSweep->rx->sweepUpper,gSweep->rx->sweepSize,gSweep->rx->crop);
		
		//gSweep->sweepRadio();
		
		std::thread(&Sweep::sweepRadio,gSweep).detach();
		
		return;
	}else if(event.GetId() == ID_SWEEPSTOP){
		winout("Stop at End of Sweep Pass\n");
		gSweep->threadexit=1;
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

	wxStaticBox *box=NULL;
	
	wxCheckBox *cbox;
	if(sdr->hasGainMode){
		cbox=new wxCheckBox(ScrolledWindow,ID_CHECKAUTO, "&Hardware AGC",wxPoint(20,yloc), wxSize(230, 25));
		cbox->SetValue(1);	
		yloc += 25;   
	}
	
	cbox=new wxCheckBox(ScrolledWindow,ID_SWAPIQ, "&I/Q Swap",wxPoint(20,yloc), wxSize(90, 25));
	cbox->SetValue(0);	
	
	
	cbox=new wxCheckBox(ScrolledWindow,ID_OSCILLOSCOPE, "&Frequency Sweep",wxPoint(110,yloc), wxSize(130, 25));
	cbox->SetValue(scrolledWindowFlag);	
	yloc += 25;   

	cbox=new wxCheckBox(ScrolledWindow,ID_SOFTAUTOGAIN, "&Software AGC",wxPoint(20,yloc), wxSize(230, 25));
	cbox->SetValue(1);	
	yloc += 25;   
	
	
	if(!scrolledWindowFlag){
		box = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Set Volume ",wxPoint(20,yloc), wxSize(230, 80),wxBORDER_SUNKEN );
		box->SetToolTip(wxT("Set Volume") );

		new wxSlider(box,SCROLL_GAIN,100,0,100,wxPoint(10,15),wxSize(210,30),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);

		yloc += 85;   
	}else{
	    wxStaticBox *box2 = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Sweep Lower",wxPoint(10,yloc), wxSize(110, 75),wxBORDER_SUNKEN );
    	    sweepLower=new wxTextCtrl(box2,ID_TEXTCTRL,wxT("80"),
          	wxPoint(5,15), wxSize(100, 30));

	    box2 = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Sweep Upper",wxPoint(135,yloc), wxSize(110, 75),wxBORDER_SUNKEN );
    	    sweepUpper=new wxTextCtrl(box2,ID_TEXTCTRL,wxT("108"),
          	wxPoint(5,15), wxSize(100, 30));
		    yloc += 85;   

	    box2 = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Sweep Size",wxPoint(10,yloc), wxSize(110, 75),wxBORDER_SUNKEN );
    	    sweepSize=new wxTextCtrl(box2,ID_TEXTCTRL,wxT("5000"),
          	wxPoint(5,15), wxSize(100, 30));
		    
	    box2 = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Sweep Crop",wxPoint(135,yloc), wxSize(110, 75),wxBORDER_SUNKEN );
    	    sweepCrop=new wxTextCtrl(box2,ID_TEXTCTRL,wxT("0"),
          	wxPoint(5,15), wxSize(100, 30));
		    yloc += 85;   
		    		    
		    sweepButton=new wxButton(ScrolledWindow,ID_SWEEP,wxT("Start"),wxPoint(20,yloc));
		    
		    stopButton=new wxButton(ScrolledWindow,ID_SWEEPSTOP,wxT("Stop"),wxPoint(120,yloc));
		    		    
		    yloc += 30;   

	}
	
	if(!scrolledWindowFlag){
		box = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Zoom",wxPoint(20,yloc), wxSize(230, 80),wxBORDER_SUNKEN );
		box->SetToolTip(wxT("Zoom Data") );
		new wxSlider(box,ID_SAMPLEWIDTH,200,0,200,wxPoint(10,15),wxSize(210,-1),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);

		yloc += 85;   
	
	}else{
		box = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Oscilloscope Zoom ",wxPoint(20,yloc), wxSize(230, 80),wxBORDER_SUNKEN );
		box->SetToolTip(wxT("Zoom Oscilloscope Data") );
		new wxSlider(box,ID_OSCILLOSCOPEZOOM,200,0,200,wxPoint(10,15),wxSize(210,-1),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);

		yloc += 85;   
		
		box = new wxStaticBox(ScrolledWindow, wxID_ANY, "&Oscilloscope Slide ",wxPoint(20,yloc), wxSize(230, 80),wxBORDER_SUNKEN );
		box->SetToolTip(wxT("Slide Oscilloscope Data") );
		new wxSlider(box,ID_OSCILLOSCOPESLIDE,200,0,200,wxPoint(10,15),wxSize(210,-1),wxSL_HORIZONTAL | wxSL_AUTOTICKS | wxSL_LABELS);

		yloc += 85;   
	
	}
	
	
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
	
	if(!scrolledWindowFlag){
	
		wxPanel *panel1 = new wxPanel(ScrolledWindow,wxID_ANY, wxPoint(20,yloc), wxSize(230, 250),wxBORDER_SUNKEN | wxFULL_REPAINT_ON_RESIZE,wxT("Control2"));
	
			 wxString computers1[] =
		  { "AM", "NAM","FM","NBFM","USB","LSB","CW","IQ" };
	

		modeBox=new wxRadioBox(panel1, ID_RADIOBOX,
			"Choose Mode", wxPoint(20,10), wxDefaultSize,
			 8, computers1, 0, wxRA_SPECIFY_ROWS);	 
		modeBox->SetSelection(sdr->decodemode);
	
		wxStaticBox *box2 = new wxStaticBox(panel1, wxID_ANY, "&Bandwidth Override",wxPoint(20,195), wxSize(120, 45),wxBORDER_SUNKEN );
		bandwidthOverride=new wxTextCtrl(box2,ID_BANDWIDTHOVERIDE,wxT("0"),wxPoint(0,0), wxSize(110, 30));
		bandwidthOverride->SetWindowStyle(wxTE_PROCESS_ENTER);


		yloc += 255;
	}
  	
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
	new wxStaticText(panel2,wxID_STATIC,wxT("FFT size:"),wxPoint(1,12), wxDefaultSize,wxALIGN_LEFT);

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
	
	new wxStaticText(panel2,wxID_STATIC,wxT("Window:"),wxPoint(1,42), wxDefaultSize,wxALIGN_LEFT);

	filterCombo=new wxComboBox(panel2,ID_COMBOFILTER,wxT("FILTER"),wxPoint(65,40),wxDefaultSize,
	                   strings,wxCB_DROPDOWN);
	filterCombo->SetSelection(5);
	
//	winout("Ant %p count %d\n",sdr->antenna,sdr->antennaCount);
	
	if(sdr->antennaNames.size() > 0){
 		strings.Clear();
       for (size_t i=0;i<sdr->antennaNames.size();++i){
       		strings.Add(sdr->antennaNames[i]);
        }
		new wxStaticText(panel2,wxID_STATIC,wxT("Antenna:"),wxPoint(1,72), wxDefaultSize,wxALIGN_LEFT);

		antennaCombo=new wxComboBox(panel2,ID_COMBOANTENNA,wxT("FILTER"),wxPoint(65,70),wxDefaultSize,
	                   strings,wxCB_DROPDOWN);
		antennaCombo->SetSelection(0);
        

	}

    yloc += 110;
    
	if(!scrolledWindowFlag){
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
 	}
	
	if(!scrolledWindowFlag){
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
	}
	

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
	
	modeBox=NULL;
	
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
		}else{
			sdr->iWait=0;
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
	
	int id=event.GetId();
	
	double samplewidth=event.GetSelection();
	
	if(id == ID_SAMPLEWIDTH){
		sdr->setSampleWidth(samplewidth);	
	}else if(id == ID_OSCILLOSCOPEZOOM){
		//winout("ID_OSCILLOSCOPEZOOM %g\n",samplewidth);
		gSpectrum2->oscilloscopeZoom=samplewidth;
	}
}
void BasicPane2::setDataRotate(wxCommandEvent &event)
{
	event.Skip();
	
	int id=event.GetId();
	
	double data=event.GetSelection();
	
	if(id == ID_ROTATEDATA){
		//sampleDataRotate=event.GetSelection();
		sampleDataRotate=(200-sampleDataRotate)/200;
	}else if(id == ID_OSCILLOSCOPESLIDE){
		//winout("ID_OSCILLOSCOPESLIDE %g\n",data);
		gSpectrum2->oscilloscopeSlide=data;
	}
	
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
	
	if(sdr->startPlay())return;

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
	
	if(sdr->startPlay())return;
	
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
	
	int id=event.GetId();
	
	
	if(id == ID_XMIN){
		wxString value=sweepXmin->GetValue();

		const char *alpha=value;

		double xmin=atof(alpha)*1e6;

		value=sweepXmax->GetValue();

		alpha=value;

		double xmax=atof(alpha)*1e6;
			
		if(xmax <= xmin){
			xmax=xmin+2.0e6;
		}
		gSweep->rx->sweepLower=xmin;
		gSweep->rx->sweepUpper=xmax;
		return;
	}else if(id == ID_XMAX){
		wxString value=sweepXmin->GetValue();

		const char *alpha=value;

		double xmin=atof(alpha)*1e6;

		value=sweepXmax->GetValue();

		alpha=value;

		double xmax=atof(alpha)*1e6;	
		
		if(xmin >= xmax){
			xmin=xmax-2.0e6;
		}
		gSweep->rx->sweepLower=xmin;
		gSweep->rx->sweepUpper=xmax;
		return;
	}
	
	winout("BasicPane2::OnText id %d\n",id);
		
	wxString rates=event.GetString();
	
	if(!rates)return;
	
	s->bS=NULL;

	sdr->waitPointer("iqToAudio(7)",&sdr->iqToAudio,0);

	sdr->waitPointer("frame(7)",&sdr->frame,0);

	sdr->waitPointer("doWhat(7)",&sdr->doWhat,0);
	
	sdr->samplerate=atof(rates);
	sdr->samplewidth=sdr->samplerate;
	
	//fprintf(stderr,"samplerate %g\n",sdr->samplerate);
	

	if(sdr->startPlay())return;
	
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
	if(!scrolledWindowFlag)modeBox->SetSelection(sdr->decodemode);

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
		}
		
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

	
	if(sdr->startPlay())return;

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
	wxString value=bandwidthOverride->GetValue();
	const char *alpha=value;
	double xmin=atof(alpha);
	sdr->bandwidthOveride=xmin;
	
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
   
    if(keycode == 'f' || keycode == ' '){
        gSpectrum2->iFreeze = !gSpectrum2->iFreeze;
     //   winout("iWait %d\n",iWait);
    }

    
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
	
	if(gSpectrum2->oscilloscope == 1){
		double gxmin=gSpectrum2->xminView;
		double gxmax=gSpectrum2->xmaxView;
		double gdx=gxmax-gxmin;
		
		double ixmin=0;
		double ixmax=getWidth();
		double idx=ixmax-ixmin;
		
		double ff=gdx*(pp3.x-ixmin)/idx+gxmin;

		//fprintf(stderr,"Spectrum2::mouseDown ff %g MHZ\n",ff/1e6);	
		
		int flag=0;
		gSpectrum2->oscilloscope=flag;
		gBasicPane2->scrolledWindowFlag=flag;
		gBasicPane2->SetScrolledWindow();
		wxSize size = gSweep->GetClientSize();
		size.x += 4;
		size.y += 4;
		gSweep->SetClientSize(size);
		size.x -= 4;
		size.y -= 4;
		gSweep->SetClientSize(size);
		sdr->initPlay();
		sdr->setFrequencyFC(ff);
		sdr->iWait=0;
		s->bS=sdr->bS;
		//winout("Spectrum2::mouseDown %p %p\n",s->bS,sdr->bS);
		gTopPane2->Refresh();
		return;
	}
	
	if(sdr){
		double fx=sdr->fw-0.5*sdr->samplewidth + sdr->samplewidth*(pp3.x)/((double)getWidth());
		//winout("mouseDown x %d y %d sdr->fc %g sdr->f %g sdr->samplerate %g fx %g width %d\n",p1.x,p1.y,sdr->fc,sdr->f,sdr->samplerate,fx,getWidth());
		//winout(" fx %g\n",fx);
		if(sdr->samplescale < 0.98){
			sdr->setCenterFrequency(fx);
		}else{
			sdr->setFrequency(fx);
		}
		sdr->iWait=0;
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
	if(gSpectrum2->oscilloscope == 1){
		render1(evt);	
	}else{
		render2(evt);
	}
}
void WaterFall2::render1( wxPaintEvent& evt )
{
	evt.Skip();
//   winout("WaterFall2::render\n");

    if(!IsShown()) return;
    
    if(iWait)return;
    
    struct SDS2Dout *sds=&gSweep->sdsout;
    
    if(!sds->data){
    	return;
    }
    
    if(gSpectrum2->nrow >= sds->iymax)gSpectrum2->nrow=0;
    
    //fprintf(stderr,"gSpectrum2->nrow %ld\n",gSpectrum2->nrow);
    
    double *magnitude=&sds->data[gSpectrum2->nrow*sds->ixmax];    
    
    int length=sds->ixmax;
    
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
	float *magnitude2 = new float[length];

    pmin =  1e33;
    pmax = -1e33;

    double rmin=sds->xmin;
    double rmax=sds->xmax;
    
    double ddx=(rmax-rmin)/(double)(length);
    for(int n=0;n<length;++n){
        double r;
        r=sds->xmin+n*ddx;
        range[n]=r;
        double v=magnitude[n];
        if(v < pmin)pmin=v;
        if(v > pmax)pmax=v;
        magnitude2[n]=v;
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
    
    FloatToImage(magnitude2,length,&pd,wateric);
/*
    int ns1=3*water.xsize*(water.ysize-water.nline-1);
    int ns2=3*water.xsize*water.ysize+3*water.xsize*(water.ysize-1-water.nline++);
*/
    int ns1=3*water.xsize*(water.nline+water.ysize);
    int ns2=3*water.xsize*(water.nline++);

	double Slide=(gSpectrum2->oscilloscopeSlide)/200.0;
	if(Slide <= 0.0)Slide=1.0/200.0;
	
	double Zoom=(gSpectrum2->oscilloscopeZoom)/200.0;
	if(Zoom <= 0.0)Zoom=1.0/200.0;
	
	double gLength=Zoom*(rmax-rmin);
		
	double gxmin=(rmax-rmin-gLength)*Slide+rmin;
	double gxmax=gxmin+gLength;

    int nmin,nmax;
    nmin=length-1;
    nmax=0;
    for(int n=0;n<length;++n){
        if(range[n] <= gxmin)nmin=n;
        if(range[n] <= gxmax)nmax=n;
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

void WaterFall2::render2( wxPaintEvent& evt )
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
	
	nrow=0;

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
	if(oscilloscope == 1 && !gSweep->sdsout.data){
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
	
    if(keycode == 'f' || keycode == ' '){
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
	
	oscilloscopeSlide=200;
	
	oscilloscopeZoom=200;

	amaxGlobal=0;
	
	aminGlobal=0;

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

	nrow=0;
    
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
	
	if(oscilloscope == 1){
		double gxmin=xminView;
		double gxmax=xmaxView;
		double gdx=gxmax-gxmin;
		
		double ixmin=0;
		double ixmax=getWidth();
		double idx=ixmax-ixmin;
		
		double ff=gdx*(pp3.x-ixmin)/idx+gxmin;

		//fprintf(stderr,"Spectrum2::mouseDown ff %g MHZ\n",ff/1e6);	
		
		int flag=0;
		oscilloscope=flag;
		gBasicPane2->scrolledWindowFlag=flag;
		gBasicPane2->SetScrolledWindow();
		wxSize size = gSweep->GetClientSize();
		size.x += 4;
		size.y += 4;
		gSweep->SetClientSize(size);
		size.x -= 4;
		size.y -= 4;
		gSweep->SetClientSize(size);
		sdr->initPlay();
		sdr->setFrequencyFC(ff);
		sdr->iWait=0;
		s->bS=sdr->bS;
		//winout("Spectrum2::mouseDown %p %p\n",s->bS,sdr->bS);
		gTopPane2->Refresh();
		return;
	}

	if(sdr){
		double fx=sdr->fw-0.5*sdr->samplewidth + sdr->samplewidth*(pp3.x)/((double)getWidth());
		//winout("mouseDown x %d y %d sdr->fc %g sdr->f %g sdr->samplerate %g fx %g width %d\n",pp3.x,pp3.y,sdr->fc,sdr->f,sdr->samplerate,fx,getWidth());
		//winout(" fx %g sdr->fw %g\n",fx,sdr->fw);
		if(sdr->samplescale < 0.98){
			sdr->setCenterFrequency(fx);
		}else{
			sdr->setFrequency(fx);
		}
		sdr->iWait=0;
		s->bS=sdr->bS;
		//winout("Spectrum2::mouseDown %p %p\n",s->bS,sdr->bS);
		gTopPane2->Refresh();
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
	
	//static long long nc=0;
	//winout("Spectrum2 render nc %lld\n",nc++);

    if(!IsShown())return;
    
    if(rtime() < lineTime)return;

    lineTime=rtime()+lineDumpInterval;

    struct SDS2Dout *sds=&gSweep->sdsout;
    
    if(!sds->data){
    	return;
    }
    
        
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
    
    double *buffSend2;
        
	if(!iWait){
 	
		glColor4f(0, 0, 1, 1);
		
		if(buffSize != sds->ixmax){
			buffSize=sds->ixmax;
			if(buff1)cFree((char *)buff1);
			buff1=(float *)cMalloc(sizeof(float)*8*sds->ixmax,95288);
			if(buff2)cFree((char *)buff2);
			buff2=(float *)cMalloc(sizeof(float)*8*sds->ixmax,95288);
			if(buff3)cFree((char *)buff3);
			buff3=(float *)cMalloc(sizeof(float)*8*sds->ixmax,95288);
			iHaveData=0;
		}      	
      	
      	if(!iFreeze || !iHaveData){
      		nrow++;
			if(nrow >= sds->iymax)nrow=0;
		}
		
		buffSend2=&sds->data[nrow*sds->ixmax]; 
	
		unsigned int num;
		
		
		if(iFreeze && iHaveData){
			for (int k = 0 ; k < sds->ixmax; k++){
				buff2[k] = buff1[k];
			}
		
		}else{
			for (int k = 0 ; k < sds->ixmax; k++){
				float r = buffSend2[k];
				buff1[k] = r;
				buff2[k] = r;
			}
	
		
			iHaveData=1;
		}
		
/*
		int nmax2=-1;
		double amax2=0;
		
		for(int n=0;n<sds->ixmax;++n){
			double v=buff2[n];
        	if(v > 0.0)v=sqrt(v);
         	if(v > amax2){
         		amax2=v;
         		nmax2=n;
         	}
		}

		buff2[0]=buff2[0];
		buff2[1]=buff2[1];

*/
		num=0;
 /*
 		msresamp_crcf_reset(iqSampler1);
		msresamp_crcf_execute(iqSampler1, (liquid_float_complex *)&buff2[0], sds->ixmax, (liquid_float_complex *)&buff3[0], &num);  // decimate
*/
		for(int n=0;n<sds->ixmax;++n){
			buff3[n]=buff2[n];
		}
		
		num=sds->ixmax;

      	buffSendLength=num;
		
		//int nmax3=-1;
		double avg=0;
		for(int n=0;n<buffSendLength;++n){
			double v=buff3[n];
         	avg += v;
		}
		
		avg /= buffSendLength;
		
		
		//fprintf(stderr,"amax %g amin %g avg %g sds->ixmax %ld\n",amax,amin,avg,sds->ixmax);
	
		double shift=-100-avg;
			
		       			
		buffFlag=0;
		
		double ymin = verticalMinimum;
		double ymax = verticalMaximum;	
		//double ymin =  amin;
		//double ymax =  amax;	
		if(ymin >= ymax)ymin=ymax-40;
		
		double dy=ymax-ymin;
		
		double iymin=0;
		double iymax=getHeight()-20;
		double idy=iymin-iymax;		
		
		double xmin=sds->xmin;
		double xmax=sds->xmax;
		double dx=xmax-xmin;
		
		double Slide=(oscilloscopeSlide)/200.0;
		if(Slide <= 0.0)Slide=1.0/200.0;
		
		double Zoom=(oscilloscopeZoom)/200.0;
		if(Zoom <= 0.0)Zoom=1.0/200.0;
		
		double gLength=Zoom*dx;
	  		
		double gxmin=(dx-gLength)*Slide+xmin;
		double gxmax=gxmin+gLength;
		//double gdx=gxmax-gxmin;
		
		xminView=gxmin;
		xmaxView=gxmax;
		
		double ixmin=0;
		double ixmax=getWidth();
		double idx=ixmax-ixmin;
		

		int nmin=(sds->ixmax-1)*(gxmin-xmin)/dx;
		int nmax=(sds->ixmax-1)*(gxmax-xmin)/dx;
		int dn=nmax-nmin;

		//fprintf(stderr,"gxmain %g gxmax %g nmin %d nmax %d dn %d\n",gxmin,gxmax,nmin,nmax,dn);


		//int ixxmin,ixxmax,iyymin,iyymax;
	
		//ixxmin=100000000;
		//ixxmax= -100000000;
		//iyymin=100000000;
		//iyymax= -100000000;
	
		int ixold=0;
		int iyold=0;
		int iflag=0;
	
		
		double ddx=(xmax-xmin)/sds->ixmax;
			
		for(int n=0;n<sds->ixmax;++n){
			double sum=0;
			double v=0;
			if(softAutoGain == 1){
				v=buff3[n];
				sum=v+shift;
				//sum=(1.0-lineAlpha)*buff4[n]+lineAlpha*v+shift;
			}else{
				v=buff3[n];
				sum=v;
				//sum=(1.0-lineAlpha)*buff4[n]+lineAlpha*v;
			}
        	
			//buff4[n]=sum;
        	
			double y=sum;
			double x=n*ddx+xmin;
			if(x < gxmin || x > gxmax)continue;
			int ix;
			int iy;
			ix=(int)(n-nmin)*idx/dn+ixmin;
			//fprintf(stderr,"n %d x %g y %g ix %d xmin %g xmax %g\n",n,x,y,ix,xmin,xmax);
			if(ix <= ixmin || ix >= ixmax)continue;
			//if(ix < ixxmin)ixxmin=ix;
			//if(ix > ixxmax)ixxmax=ix;
			iy=(int)((y-ymin)*idy/dy+iymax);
			//fprintf(stderr,"iy %d iymax %g iymin %g y %g dy %g idy %g\n",iy,iymax,iymin,y,dy,idy);
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
			double xmns,xmxs;
			xmnc=gxmin/1e6;
			xmxc=gxmax/1e6;
			xmns=xmnc;
			xmxs=xmxc;
			//fprintf(stderr,"xmnc %g xmxc %g samplewidth %g samplescale %g\n",xmnc,xmxc,sdr->samplewidth,sdr->samplescale);
			GridPlotNeat2(&xmnc,&xmxc,&Large,&Small);
			//fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
			
			dx=xmxs-xmns;
			
			for(double xp=xmnc;xp <= xmxc;xp += Large){
				char cbuff[256];
			    double xx=(xp-xmns)/dx;
			    //fprintf(stderr,"xp %g xx %g ",xp,xx);
			    if(xx < 0.0 || xx > 1.0)continue;
			    int ixx=(int)(idx*xx+ixmin);
			   // fprintf(stderr,"ixx %d ixmin %g ixmax %g xp %g\n ",ixx,ixmin,ixmax,xp);
			    if(ixx < ixmin || ixx > ixmax)continue;
 				DrawLine3(ixx, 0, ixx, getHeight()-15);
 				sprintf(cbuff,"%g",xp);
				DrawString(ixx-10,getHeight()-13, cbuff);
			}
			//winout(" idx %g\n",idx);
			
		//exit(1);

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
	
  		int dxii=ftox(sdr->f+sdr->bw/(2.0))-ftox(sdr->f-sdr->bw/(2.0));
  		if(dxii < 4)dxii=4;
	
 		box.x=ftox(sdr->f)-dxii/2.0;
 		box.y=0;
 		box.xsize=dxii;
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

    if(!IsShown())return;
    
    if(rtime() < lineTime)return;

    lineTime=rtime()+lineDumpInterval;

    struct SDS2Dout *sds=&gSweep->sdsout;
    
    if(!sds->data){
    	return;
    }
    
        
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
    
    double *buffSend2;
    
    double amax=-1e33;
    double amin=1e33;
    
	if(!iWait){
 	
		glColor4f(0, 0, 1, 1);
		
		//winout("filterType %d\n",filterType);
		
      	if(decodemode1 != sdr->decodemode){
      		decodemode1=sdr->decodemode;
      		double Ratio = (float)(sds->ixmax/sdr->samplerate);
      		iqSampler1  = msresamp_crcf_create(Ratio, 60.0f);
      	}
    	
		if(buffSize != sds->ixmax){
			buffSize=sds->ixmax;
			if(buff1)cFree((char *)buff1);
			buff1=(float *)cMalloc(sizeof(float)*8*sds->ixmax,95288);
			if(buff2)cFree((char *)buff2);
			buff2=(float *)cMalloc(sizeof(float)*8*sds->ixmax,95288);
			if(buff3)cFree((char *)buff3);
			buff3=(float *)cMalloc(sizeof(float)*8*sds->ixmax,95288);
			iHaveData=0;
		}      	
      	
      	if(!iFreeze || !iHaveData){
      		nrow++;
			if(nrow >= sds->iymax)nrow=0;
		}
		
		buffSend2=&sds->data[nrow*sds->ixmax]; 
				
		//fprintf(stderr,"witch %d ip %d\n",witch,ip);
				
		unsigned int num;
		
		//fprintf(stderr,"iFreeze %d iHaveData %d\n ",iFreeze,iHaveData);
		
		if(iFreeze && iHaveData){
			for (int k = 0 ; k < sds->ixmax; k++){
				float r = buff1[k];
				float rr=r;
				buff2[k] = r;
				if(gBasicPane2->sampleDataRotate > 0.0){
					//gBasicPane->sampleDataRotate=0.5;
					int nn=gBasicPane2->sampleDataRotate*(sds->ixmax-1)+k;
					if(nn > sds->ixmax-1){
						nn -= sds->ixmax;
					}
					//fprintf(stderr,"nn %d k %d\n",nn,k);
					buff2[nn] = rr;
				}else{
					buff2[k] = rr;
				}
			}
		
			//fprintf(stderr,"1 num %d iFreeze %d\n",num,iFreeze);
		}else{
			for (int k = 0 ; k < sds->ixmax; k++){
				float r = buffSend2[k];
				float rr=r;
				buff1[k] = rr;
				buff2[k] = r;
				if(gBasicPane2->sampleDataRotate > 0.0){
					//fprintf(stderr,"gBasicPane->sampleDataRotate \n");
					//gBasicPane->sampleDataRotate=0.5;
					int nn=gBasicPane2->sampleDataRotate*(sds->ixmax-1)+k;
					if(nn > sds->ixmax-1){
						nn -= sds->ixmax;
					}
					//fprintf(stderr,"nn %d k %d\n",nn,k);
					buff2[nn] = rr;
				}else{
					buff2[k] = rr;
				}
			}
	
			//if(gBasicPane->sampleDataRotate > 0.0)exit(1);
		
			iHaveData=1;
			//fprintf(stderr,"2 num %d iFreeze %d\n",num,iFreeze);
		}
		
/*		
		int nmax2=-1;
		double amax2=0;
		
		for(int n=0;n<sds->ixmax;++n){
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
 /*
 		msresamp_crcf_reset(iqSampler1);
		msresamp_crcf_execute(iqSampler1, (liquid_float_complex *)&buff2[0], sds->ixmax, (liquid_float_complex *)&buff3[0], &num);  // decimate
*/
		for(int n=0;n<sds->ixmax;++n){
			buff3[n]=buff2[n];
		}
		
		num=sds->ixmax;

      	buffSendLength=num;
		
		//int nmax=-1;
		for(int n=0;n<buffSendLength;++n){
			double v=buff3[n];
         	if(v > amax){
         		amax=v;
         	  // nmax=n;
         	}
         	if(v < amin){
         		amin=v;
         	}
		}
		
		//static long int count=0;
		
		//winout("amax %g num %d %d amax2 %g nmax2 %d sds->ixmax %ld\n",amax,num,nmax,amax2,nmax2,sds->ixmax);
		//winout("count %ld xmin %g xmax %g ymin %g ymax %g ixmax %ld iymax %ld\n",count++,sds->xmin,sds->xmax,sds->ymin,sds->ymax,sds->ixmax,sds->iymax);
		
		if(amaxGlobal == 0.0)amaxGlobal=amax;
        amaxGlobal = 0.9*amaxGlobal+0.1*amax;
		amax=amaxGlobal;
		
		
		//static long int count1;

		//winout("Spectrum2 render 1 count %ld amax %g ip %d num %d amax2 %g cosdt %g sindt %g  coso %g sino %g\n",count1++,
		//       amax,ip,num,amax2,sdr->cosdt,sdr->sindt,sdr->coso,sdr->sino);
		       			
		buffFlag=0;
		
		double ymin =  amin;
		double ymax =  amax;	
		if(ymin >= ymax)ymin=ymax-40;
		
		double dy=ymax-ymin;
		
		double iymin=0;
		double iymax=getHeight()-20;
		double idy=iymin-iymax;		
		
		double xmin=sds->xmin;
		double xmax=sds->xmax;
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
	
		
		double ddx=(xmax-xmin)/sds->ixmax;
			
		for(int n=0;n<sds->ixmax;++n){
			double v;
			v=buff3[n];
			double y=v;
			double x=n*ddx+xmin;
			int ix;
			int iy;
			ix=(int)(x-xmin)*idx/dx+ixmin;
			//fprintf(stderr,"n %d x %g y %g ix %d xmin %g xmax %g\n",n,x,y,ix,xmin,xmax);
			if(ix <= ixmin || ix >= ixmax)continue;
			//if(ix < ixxmin)ixxmin=ix;
			//if(ix > ixxmax)ixxmax=ix;
			iy=(int)((y-ymin)*idy/dy+iymax);
			//fprintf(stderr,"iy %d iymax %g iymin %g y %g dy %g idy %g\n",iy,iymax,iymin,y,dy,idy);
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
			double xmns,xmxs;
			xmnc=sds->xmin/1e6;
			xmxc=sds->xmax/1e6;
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
			   // fprintf(stderr,"ixx %d ixmin %g ixmax %g xp %g\n ",ixx,ixmin,ixmax,xp);
			    if(ixx < ixmin || ixx > ixmax)continue;
 				DrawLine3(ixx, 0, ixx, getHeight()-15);
 				sprintf(cbuff,"%g",xp);
				DrawString(ixx-10,getHeight()-13, cbuff);
			}
			//winout(" idx %g\n",idx);
			
		//exit(1);

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
 
void Spectrum2::render1b(wxPaintEvent& evt )
{
	evt.Skip();
	
	//static long long nc=0;
	//winout("Spectrum2 render nc %lld\n",nc++);

    if(!IsShown())return;
    
    if(rtime() < lineTime)return;

    lineTime=rtime()+lineDumpInterval;

    struct SDS2Dout *sds=&gSweep->sdsout;
    
    if(!sds->data){
    	return;
    }
    
        
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
    
    double *buffSend2;
    
    double amax=-1e33;
    double amin=1e33;
    
	if(!iWait){
 	
		glColor4f(0, 0, 1, 1);
		
		if(buffSize != sds->ixmax){
			buffSize=sds->ixmax;
			if(buff1)cFree((char *)buff1);
			buff1=(float *)cMalloc(sizeof(float)*8*sds->ixmax,95288);
			if(buff2)cFree((char *)buff2);
			buff2=(float *)cMalloc(sizeof(float)*8*sds->ixmax,95288);
			if(buff3)cFree((char *)buff3);
			buff3=(float *)cMalloc(sizeof(float)*8*sds->ixmax,95288);
			iHaveData=0;
		}      	
      	
      	if(!iFreeze || !iHaveData){
      		nrow++;
			if(nrow >= sds->iymax)nrow=0;
		}
		
		buffSend2=&sds->data[nrow*sds->ixmax]; 
	
		unsigned int num;
		
		
		if(iFreeze && iHaveData){
			for (int k = 0 ; k < sds->ixmax; k++){
				buff2[k] = buff1[k];
			}
		
		}else{
			for (int k = 0 ; k < sds->ixmax; k++){
				float r = buffSend2[k];
				buff1[k] = r;
				buff2[k] = r;
			}
	
		
			iHaveData=1;
		}
		
/*
		int nmax2=-1;
		double amax2=0;
		
		for(int n=0;n<sds->ixmax;++n){
			double v=buff2[n];
        	if(v > 0.0)v=sqrt(v);
         	if(v > amax2){
         		amax2=v;
         		nmax2=n;
         	}
		}

		buff2[0]=buff2[0];
		buff2[1]=buff2[1];

*/
		num=0;
 /*
 		msresamp_crcf_reset(iqSampler1);
		msresamp_crcf_execute(iqSampler1, (liquid_float_complex *)&buff2[0], sds->ixmax, (liquid_float_complex *)&buff3[0], &num);  // decimate
*/
		for(int n=0;n<sds->ixmax;++n){
			buff3[n]=buff2[n];
		}
		
		num=sds->ixmax;

      	buffSendLength=num;
		
		//int nmax3=-1;
		for(int n=0;n<buffSendLength;++n){
			double v=buff3[n];
         	if(v > amax){
         		amax=v;
         	    //nmax3=n;
         	}
         	if(v < amin){
         		amin=v;
         	}
		}
/*
		static long int count=0;
		
		winout("amax %g amin %g num %d %d amax2 %g nmax3 %d sds->ixmax %ld\n",amax,amin,num,nmax3,amax2,nmax2,sds->ixmax);
		winout("count %ld xmin %g xmax %g ymin %g ymax %g ixmax %ld iymax %ld\n",count++,sds->xmin,sds->xmax,sds->ymin,sds->ymax,sds->ixmax,sds->iymax);
*/
		if(amaxGlobal == 0.0)amaxGlobal=amax;
        amaxGlobal = 0.9*amaxGlobal+0.1*amax;
		amax=amaxGlobal;
		
		if(aminGlobal == 0.0)aminGlobal=amin;
        aminGlobal = 0.9*aminGlobal+0.1*amin;
		amin=aminGlobal;
		
		
		//static long int count1;

		//winout("Spectrum2 render 1 count %ld amax %g ip %d num %d amax2 %g cosdt %g sindt %g  coso %g sino %g\n",count1++,
		//       amax,ip,num,amax2,sdr->cosdt,sdr->sindt,sdr->coso,sdr->sino);
		       			
		buffFlag=0;
		
		double ymin =  amin;
		double ymax =  amax;	
		if(ymin >= ymax)ymin=ymax-40;
		
		double dy=ymax-ymin;
		
		double iymin=0;
		double iymax=getHeight()-20;
		double idy=iymin-iymax;		
		
		double xmin=sds->xmin;
		double xmax=sds->xmax;
		double dx=xmax-xmin;

		double gxmin=gSweep->rx->sweepLower;
		double gxmax=gSweep->rx->sweepUpper;
		//double gdx=gxmax-gxmin;
		
		double ixmin=0;
		double ixmax=getWidth();
		double idx=ixmax-ixmin;
		

		int nmin=(sds->ixmax-1)*(gxmin-xmin)/dx;
		int nmax=(sds->ixmax-1)*(gxmax-xmin)/dx;
		int dn=nmax-nmin;

		//fprintf(stderr,"gxmain %g gxmax %g nmin %d nmax %d dn %d\n",gxmin,gxmax,nmin,nmax,dn);


		//int ixxmin,ixxmax,iyymin,iyymax;
	
		//ixxmin=100000000;
		//ixxmax= -100000000;
		//iyymin=100000000;
		//iyymax= -100000000;
	
		int ixold=0;
		int iyold=0;
		int iflag=0;
	
		
		double ddx=(xmax-xmin)/sds->ixmax;
			
		for(int n=0;n<sds->ixmax;++n){
			double v;
			v=buff3[n];
			double y=v;
			double x=n*ddx+xmin;
			if(x < gxmin || x > gxmax)continue;
			int ix;
			int iy;
			ix=(int)(n-nmin)*idx/dn+ixmin;
			//fprintf(stderr,"n %d x %g y %g ix %d xmin %g xmax %g\n",n,x,y,ix,xmin,xmax);
			if(ix <= ixmin || ix >= ixmax)continue;
			//if(ix < ixxmin)ixxmin=ix;
			//if(ix > ixxmax)ixxmax=ix;
			iy=(int)((y-ymin)*idy/dy+iymax);
			//fprintf(stderr,"iy %d iymax %g iymin %g y %g dy %g idy %g\n",iy,iymax,iymin,y,dy,idy);
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
			double xmns,xmxs;
			xmnc=gxmin/1e6;
			xmxc=gxmax/1e6;
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
			   // fprintf(stderr,"ixx %d ixmin %g ixmax %g xp %g\n ",ixx,ixmin,ixmax,xp);
			    if(ixx < ixmin || ixx > ixmax)continue;
 				DrawLine3(ixx, 0, ixx, getHeight()-15);
 				sprintf(cbuff,"%g",xp);
				DrawString(ixx-10,getHeight()-13, cbuff);
			}
			//winout(" idx %g\n",idx);
			
		//exit(1);

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

int doFFT2(double *x,double *y,long length,int direction)
{
	double *datar;
	long n,n2;
	int ifound;
	
	if(!x || !y)return 1;
	
	n2=2;
	ifound=FALSE;
	for(n=0;n<29;++n){
		if(length == n2){
			ifound=TRUE;
			break;
		}
		n2 *= 2;
	}
	
	if(!ifound){
	    fprintf(stderr,"doFFT Did not find power of 2 length %ld\n",length);
	    return 1;
	}
	
	datar=(double *)cMalloc(2*length*sizeof(double),9092);
	if(!datar){
	    fprintf(stderr,"doFFT out of Memory\n");
	    return 1;
	}
	
	for(n=0;n<length;++n){
		datar[2*n]=x[n];
		datar[2*n+1]=y[n];
	}
	
	fft(datar,(int)length,direction);
	
	for(n=0;n<length;++n){
		x[n]=datar[2*n];
		y[n]=datar[2*n+1];
	}
	
	if(datar)cFree((char *)datar);
	
	return 0;
	
}
int fft(double *data,int nn,int isign)
{
	double twopi,tempr,tempi,wstpr,wstpi;
	double wr,wi,theta,sinth,fni;
	int i,j,n,m,mmax,istep;
	
      data -= 1;
      j=1;
      n=2*nn;
      twopi=8.*atan(1.);
       for(i=1;i<=n;i += 2){
       if(i-j >= 0)goto L200;
       tempr=data[j];
       tempi=data[j+1];
       data[j]=data[i];
       data[j+1]=data[i+1];
       data[i]=tempr;
       data[i+1]=tempi;
L200:    m=n/2;
L300:    if(j-m > 0)goto L400;
		goto L500;
L400:    j=j-m;
       m=m/2;
       if(m-2 >= 0)goto L300;
L500:  j=j+m;
       }
      mmax=2;
L600:   if(mmax-n >= 0)goto L1000;
	  istep=2*mmax;
      theta=twopi/(double)(isign*mmax);
      sinth=sin(theta/2.);
      wstpr=-2.*sinth*sinth;
      wstpi=sin(theta);
      wr=1.;
      wi=0.;
		for(m=1;m<=mmax;m+=2){
			for( i=m;i<=n;i+=istep){
				j=i+mmax;
				tempr=wr*data[j]-wi*data[j+1];
				tempi=wr*data[j+1]+wi*data[j];
				data[j]=data[i]-tempr;
				data[j+1]=data[i+1]-tempi;
				data[i]=data[i]+tempr;
				data[i+1]=data[i+1]+tempi;
			}
			tempr=wr;
			wr=wr*wstpr-wi*wstpi+wr;
			wi=wi*wstpr+tempr*wstpi+wi;
		}
      mmax=istep;
      goto L600;
L1000: 

	if(isign > 0){
		fni=2.0/(double)nn;
	}else{
		fni=0.5;
	}
	for( i=1;i<=2*nn;++i){
	    data[i]=data[i]*fni;
	}
	return 0;
}
double atofs(char *s)
/* standard suffixes */
{
	char last;
	int len;
	double suff = 1.0;
	len = strlen(s);
	last = s[len-1];
	s[len-1] = '\0';
	switch (last) {
		case 'g':
		case 'G':
			suff *= 1e3;
		case 'm':
		case 'M':
			suff *= 1e3;
		case 'k':
		case 'K':
			suff *= 1e3;
			suff *= atof(s);
			s[len-1] = last;
			return suff;
	}
	s[len-1] = last;
	return atof(s);
}
int doWindow(double *x,double *y,long length,int type)
{
    static float *w=NULL;
    static long lengthSave;
    if(!w){
    	w=new float[length];
    	lengthSave=length;
    }else if(length > lengthSave){
    	delete[] w;
    	w=new float[length];
    	lengthSave=length;    
    }
    int i;
    
    if(!x || !y)return 1;
    
    switch(type){
            
        case FILTER_RECTANGULAR:

            for(i=0; i<length; i++)
                w[i] = 1.0;
            
            break;
            

            
        case FILTER_HANN:
//#define WINDOWS_LONG_NAMES
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
//#define WINDOWS_LONG_NAMES
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
        x[i]=amp*x[i];
        y[i]=amp*y[i];
    }
    
    return 0;
    
}

int writesds(struct SDS2Dout *sdsout)
{

	if(!sdsout){
	    Warning((char *)"writesds2d Found NULL struct pointer\n");
	    return 1;
	}
	
	if(sdsout->xmin >= sdsout->xmax){
	    sprintf(WarningBuff,(char *)"Warning            : Time %g\n",sdsout->time);
	    Warning(WarningBuff);
	    sprintf(WarningBuff,(char *)"Warning Bad Bounds : xmin %g xmax %g\n",sdsout->xmin,sdsout->xmax);
	    Warning(WarningBuff);
	    sdsout->xmax = sdsout->xmin+fabs(sdsout->xmin)+1.0;
	    sprintf(WarningBuff,(char *)"Reset To           : xmin %g xmax %g\n\n",sdsout->xmin,sdsout->xmax);
	    Warning(WarningBuff);
	}
	
	if(sdsout->ymin >= sdsout->ymax){
	    sprintf(WarningBuff,(char *)"Warning            : Time %g\n",sdsout->time);
	    Warning(WarningBuff);
	    sprintf(WarningBuff,(char *)"Warning Bad Bounds : ymin %g ymax %g\n",sdsout->ymin,sdsout->ymax);
	    Warning(WarningBuff);
	    sdsout->ymax = sdsout->ymin+fabs(sdsout->ymin)+1.0;
	    sprintf(WarningBuff,(char *)"Reset To           : ymin %g ymax %g\n\n",sdsout->ymin,sdsout->ymax);
	    Warning(WarningBuff);
	}
	
	if(sdsout->type == DATA_TYPE_DOUBLE){
		return writesds2dDouble(sdsout);
	}else if(sdsout->type == DATA_TYPE_FLOAT){
		return writesds2dFloat(sdsout);
	}else if(sdsout->type == DATA_TYPE_BYTE){
		return writesds2dBytes(sdsout);
	}else if(sdsout->type == DATA_TYPE_FLOAT_3D){
		return writesds3dFloat(sdsout);
	}else if(sdsout->type == DATA_TYPE_DOUBLE_3D){
		return writesds3dDouble(sdsout);
	}
	
	sprintf(WarningBuff,(char *)"writesds2d Found Unknown data type %d\n",sdsout->type);
	Warning(WarningBuff);

	return 1;
}
static int writesds3dDouble(struct SDS2Dout *sdsout)
{
//	extern int DFerror;
	int32 rank,size[3];
	double vmin,vmax;
	char buff[256];
	long n,length;
	int lastref;
	double v;
	int ret;
	
	ret=1;
	
	if(!sdsout){
	    Warning((char *)"writesds3dDouble Found NULL struct pointer\n");
		goto OutOfHere;
	}
	
	if(sdsout->type != DATA_TYPE_DOUBLE_3D){
	    sprintf(WarningBuff,(char *)"writesds3dDouble Found Wrong data type %d\n",sdsout->type);
	    Warning(WarningBuff);
		goto OutOfHere;
	}
	
	if(!sdsout->data){
	    Warning((char *)"writesds3dDouble Found NULL data Pointer\n");
		goto OutOfHere;
	}
	
	length=sdsout->ixmax*sdsout->iymax*sdsout->izmax;
	
	if(length <= 0){
	    Warning((char *)"writesds3dDouble Found data length less than one\n");
		goto OutOfHere;
	}
	

	vmin =  1e33;
	vmax = -1e33;
	for(n=0;n<length;++n){
	    v=sdsout->data[n];
	    if(v < vmin)vmin = v;
	    if(v > vmax)vmax = v;
	}
	/*
	fprintf(stderr,"writesds3dDouble dmin %g dmax %g\n",vmin,vmax);
	*/
	rank=3;
	size[0]=(int)sdsout->izmax;
	size[1]=(int)sdsout->iymax;
	size[2]=(int)sdsout->ixmax;

	if(sdsout->n == 0){
	    unlink((char *)sdsout->path);
	    DFSDclear();
	    DFSDrestart();
        DFSDsetNT(DFNT_FLOAT64);
	    if(DFSDputdata((char *)sdsout->path,rank,size,(float *)sdsout->data)){
	        sprintf(WarningBuff,(char *)"writesds3dDouble DFSDputdata error %d\n",DFerror);
	        Warning(WarningBuff);
	        goto OutOfHere;
	    }
	}else{
	    if(DFSDadddata((char *)sdsout->path,rank,size,(float *)sdsout->data)){
	        sprintf(WarningBuff,(char *)"writesds3dDouble DFSDadddata error %d\n",DFerror);
	        Warning(WarningBuff);
	        goto OutOfHere;
	    }
	}
	

	lastref=DFSDlastref();
	if(lastref == -1){
	    sprintf(WarningBuff,(char *)"writesds3dDouble DFSDlastref error %d\n",DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}

	if(DFANputlabel(sdsout->path,DFTAG_SDG,lastref,sdsout->name) == -1){
	    sprintf(WarningBuff,(char *)"writesds3dDouble DFANputlabel %s Name %s lastref %d error %d\n",
		               sdsout->path,sdsout->name,lastref,DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}
	if(sdsout->pioName){
	    sprintf(buff,(char *)"xmin %g ymin %g zmin %g xmax %g ymax %g zmax %g vmin %g vmax %g time %g pioName \"%s\" ",
	            sdsout->xmin,sdsout->ymin,sdsout->zmin,
                sdsout->xmax,sdsout->ymax,sdsout->zmax,
                vmin,vmax,sdsout->time,sdsout->pioName);
	}else{
	    sprintf(buff,(char *)"xmin %g ymin %g zmin %g xmax %g ymax %g zmax %g vmin %g vmax %g time %g",
	            sdsout->xmin,sdsout->ymin,sdsout->zmin,
                sdsout->xmax,sdsout->ymax,sdsout->zmax,
                vmin,vmax,sdsout->time);
	}
	if(DFANputdesc(sdsout->path,DFTAG_SDG,lastref,(char *)buff,strlen((char *)buff)) == -1){
	    sprintf(WarningBuff,(char *)"writesds3dDouble DFANputdesc %s Name %s lastref %d DFerror %d\n",
		sdsout->path,sdsout->name,lastref,DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}
	ret = 0;
OutOfHere:
	return ret;
}
static int writesds3dFloat(struct SDS2Dout *sdsout)
{
//	extern int DFerror;
	int32 rank,size[3];
	double vmin,vmax;
	char buff[256];
	long n,length;
	int lastref;
	float *data;
	double v;
	int ret;
	
	ret=1;
	
	data=NULL;

	if(!sdsout){
	    Warning((char *)"writesds3dFloat Found NULL struct pointer\n");
		goto OutOfHere;
	}
	
	if(sdsout->type != DATA_TYPE_FLOAT_3D){
	    sprintf(WarningBuff,(char *)"writesds3dFloat Found Wrong data type %d\n",sdsout->type);
	    Warning(WarningBuff);
		goto OutOfHere;
	}
	
	if(!sdsout->data){
	    Warning((char *)"writesds3dFloat Found NULL data Pointer\n");
		goto OutOfHere;
	}
	
	length=sdsout->ixmax*sdsout->iymax*sdsout->izmax;
	
	if(length <= 0){
	    Warning((char *)"writesds3dFloat Found data length less than one\n");
		goto OutOfHere;
	}
	
	data=(float *)cMalloc(length*sizeof(float),1002);
	if(!data){
	    sprintf(WarningBuff,(char *)"writesds3dFloat error Trying To allocate %ld Bytes\n",length*sizeof(float));
	    Warning(WarningBuff);
		goto OutOfHere;
	}


	vmin =  1e33;
	vmax = -1e33;
	for(n=0;n<length;++n){
	    v=sdsout->data[n];
	    data[n]=(float)v;
	    if(v < vmin)vmin = v;
	    if(v > vmax)vmax = v;
	}
/*
	fprintf(stderr,"writesds3dFloat dmin %g dmax %g\n",vmin,vmax);
*/

	rank=3;
	size[0]=(int)sdsout->izmax;
	size[1]=(int)sdsout->iymax;
	size[2]=(int)sdsout->ixmax;

	if(sdsout->n == 0){
	    unlink((char *)sdsout->path);
	    DFSDclear();
	    DFSDrestart();
        DFSDsetNT(DFNT_FLOAT32);
	    if(DFSDputdata((char *)sdsout->path,rank,size,(float *)data)){
	        sprintf(WarningBuff,(char *)"writesds3dFloat DFSDputdata error %d",DFerror);
	        Warning(WarningBuff);
	        goto OutOfHere;
	    }
	}else{
	    if(DFSDadddata((char *)sdsout->path,rank,size,(float *)data)){
	        sprintf(WarningBuff,(char *)"writesds3dFloat DFSDadddata error %d",DFerror);
	        Warning(WarningBuff);
	        goto OutOfHere;
	    }
	}
	

	lastref=DFSDlastref();
	if(lastref == -1){
	    sprintf(WarningBuff,(char *)"writesds3dFloat DFSDlastref error %d",DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}

	if(DFANputlabel(sdsout->path,DFTAG_SDG,lastref,sdsout->name) == -1){
	    sprintf(WarningBuff,(char *)"writesds3dFloat DFANputlabel %s Name %s lastref %d error %d",
		               sdsout->path,sdsout->name,lastref,DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}
	if(sdsout->pioName){
	    sprintf(buff,(char *)"xmin %g ymin %g zmin %g xmax %g ymax %g zmax %g vmin %g vmax %g time %g pioName \"%s\" ",
	            sdsout->xmin,sdsout->ymin,sdsout->zmin,
                sdsout->xmax,sdsout->ymax,sdsout->zmax,
                vmin,vmax,sdsout->time,sdsout->pioName);
	}else{
	    sprintf(buff,(char *)"xmin %g ymin %g zmin %g xmax %g ymax %g zmax %g vmin %g vmax %g time %g",
	            sdsout->xmin,sdsout->ymin,sdsout->zmin,
                sdsout->xmax,sdsout->ymax,sdsout->zmax,
                vmin,vmax,sdsout->time);
	}
	if(DFANputdesc(sdsout->path,DFTAG_SDG,lastref,(char *)buff,strlen((char *)buff)) == -1){
	    sprintf(WarningBuff,(char *)"writesds3dFloat DFANputdesc %s Name %s lastref %d DFerror %d",
		sdsout->path,sdsout->name,lastref,DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}
	ret = 0;
OutOfHere:
	if(data)cFree((char *)data);
	data=NULL;
	return ret;
}
static int writesds2dDouble(struct SDS2Dout *sdsout)
{
//	extern int DFerror;
	int32 rank,size[2];
	double vmin,vmax;
	char buff[256];
	long n,length;
	int lastref;
	double v;
	int ret;
	
	ret=1;
	
	if(!sdsout){
	    Warning((char *)"writesds2dDouble Found NULL struct pointer\n");
		goto OutOfHere;
	}
	
	if(sdsout->type != DATA_TYPE_DOUBLE){
	    sprintf(WarningBuff,(char *)"writesds2dDouble Found Wrong data type %d\n",sdsout->type);
	    Warning(WarningBuff);
		goto OutOfHere;
	}
	
	if(!sdsout->data){
	    Warning((char *)"writesds2dDouble Found NULL data Pointer\n");
		goto OutOfHere;
	}
	
	length=sdsout->ixmax*sdsout->iymax;
	
	if(length <= 0){
	    Warning((char *)"writesds2dDouble Found data length less than one\n");
		goto OutOfHere;
	}
	

	vmin =  1e33;
	vmax = -1e33;
	for(n=0;n<length;++n){
	    v=sdsout->data[n];
	    if(v < vmin)vmin = v;
	    if(v > vmax)vmax = v;
	}
	
	rank=2;
	size[0]=(int)sdsout->iymax;
	size[1]=(int)sdsout->ixmax;

	if(sdsout->n == 0){
	    unlink((char *)sdsout->path);
	    DFSDclear();
	    DFSDrestart();
        DFSDsetNT(DFNT_FLOAT64);
	    if(DFSDputdata((char *)sdsout->path,rank,size,(float *)sdsout->data)){
	        sprintf(WarningBuff,(char *)"writesds2dDouble DFSDputdata error %d",DFerror);
	        Warning(WarningBuff);
	        goto OutOfHere;
	    }
	}else{
	    if(DFSDadddata((char *)sdsout->path,rank,size,(float *)sdsout->data)){
	        sprintf(WarningBuff,(char *)"writesds2dDouble DFSDadddata error %d",DFerror);
	        Warning(WarningBuff);
	        goto OutOfHere;
	    }
	}
	

	lastref=DFSDlastref();
	if(lastref == -1){
	    sprintf(WarningBuff,(char *)"writesds2dDouble DFSDlastref error %d",DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}

	if(DFANputlabel(sdsout->path,DFTAG_SDG,lastref,sdsout->name) == -1){
	    sprintf(WarningBuff,(char *)"writesds2dDouble DFANputlabel %s Name %s lastref %d error %d",
		               sdsout->path,sdsout->name,lastref,DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}
	if(sdsout->pioName){
	    sprintf(buff,(char *)"xmin %g ymin %g zmin %g xmax %g ymax %g zmax %g vmin %g vmax %g time %g pioName \"%s\" ",
	            sdsout->xmin,sdsout->ymin,sdsout->zmin,
                sdsout->xmax,sdsout->ymax,sdsout->zmax,
                vmin,vmax,sdsout->time,sdsout->pioName);
	}else{
	    sprintf(buff,(char *)"xmin %g ymin %g zmin %g xmax %g ymax %g zmax %g vmin %g vmax %g time %g",
	            sdsout->xmin,sdsout->ymin,sdsout->zmin,
                sdsout->xmax,sdsout->ymax,sdsout->zmax,
                vmin,vmax,sdsout->time);
	}
	if(DFANputdesc(sdsout->path,DFTAG_SDG,lastref,(char *)buff,strlen((char *)buff)) == -1){
	    sprintf(WarningBuff,(char *)"writesds2dDouble DFANputdesc %s Name %s lastref %d DFerror %d",
		sdsout->path,sdsout->name,lastref,DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}
	ret = 0;
OutOfHere:
	return ret;
}

static int writesds2dBytes(struct SDS2Dout *sdsout)
{
//	extern int DFerror;
	double vmin,vmax;
	char buff[256];
	long n,length;
	int lastref;
	unsigned char *data;
	double v;
	int ret;
	
	ret=1;
	
	data=NULL;

	if(!sdsout){
	    Warning((char *)"writesds2dBytes Found NULL struct pointer\n");
		goto OutOfHere;
	}
	
	if(sdsout->type != DATA_TYPE_BYTE){
	    sprintf(WarningBuff,(char *)"writesds2dBytes Found Wrong data type %d\n",sdsout->type);
	    Warning(WarningBuff);
		goto OutOfHere;
	}
	
	if(!sdsout->data){
	    Warning((char *)"writesds2dBytes Found NULL data Pointer\n");
		goto OutOfHere;
	}
	
	length=sdsout->ixmax*sdsout->iymax;
	
	if(length <= 0){
	    Warning((char *)"writesds2dBytes Found data length less than one\n");
		goto OutOfHere;
	}
	
	data=(unsigned char *)cMalloc(length*sizeof(unsigned char),1001);
	if(!data){
	    sprintf(WarningBuff,(char *)"writesds2dBytes error Trying To allocate %ld Bytes\n",length*sizeof(unsigned char));
	    Warning(WarningBuff);
		goto OutOfHere;
	}


	vmin =  1e33;
	vmax = -1e33;
	for(n=0;n<length;++n){
	    v=sdsout->data[n];
	    if(v < vmin)vmin = v;
	    if(v > vmax)vmax = v;
	}
	
	if(vmax <= vmin){
		vmax=vmin+1+2*fabs(vmin);
		for(n=0;n<length;++n){
		    data[n]=(unsigned char)(2);
		}
	}else{
		for(n=0;n<length;++n){
			v=sdsout->data[n];
			data[n]=(unsigned char)(2+252.*(v-vmin)/(vmax-vmin));
		}
	}
		
	if(sdsout->n == 0){
	    unlink((char *)sdsout->path);
		DFR8restart();
	    if(DFR8putimage((char *)sdsout->path,data,(int)sdsout->ixmax,(int)sdsout->iymax,11)){
		    goto OutOfHere;
	    }

	}else{
	    if(DFR8addimage((char *)sdsout->path,data,(int)sdsout->ixmax,(int)sdsout->iymax,11)){
		    goto OutOfHere;
	    }
	}
	

	lastref=DFR8lastref();
	if(lastref == -1){
	    sprintf(WarningBuff,(char *)"writesds2dBytes DFR8lastref error %d",DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}

	if(DFANputlabel(sdsout->path,DFTAG_SDG,lastref,sdsout->name) == -1){
	    sprintf(WarningBuff,(char *)"writesds2dBytes DFANputlabel %s Name %s lastref %d error %d",
		               sdsout->path,sdsout->name,lastref,DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}
	if(sdsout->pioName){
	    sprintf(buff,(char *)"xmin %g ymin %g zmin %g xmax %g ymax %g zmax %g vmin %g vmax %g time %g pioName \"%s\" ",
	            sdsout->xmin,sdsout->ymin,sdsout->zmin,
                sdsout->xmax,sdsout->ymax,sdsout->zmax,
                vmin,vmax,sdsout->time,sdsout->pioName);
	}else{
	    sprintf(buff,(char *)"xmin %g ymin %g zmin %g xmax %g ymax %g zmax %g vmin %g vmax %g time %g",
	            sdsout->xmin,sdsout->ymin,sdsout->zmin,
                sdsout->xmax,sdsout->ymax,sdsout->zmax,
                vmin,vmax,sdsout->time);
	}
	if(DFANputdesc(sdsout->path,DFTAG_SDG,lastref,(char *)buff,strlen((char *)buff)) == -1){
	    sprintf(WarningBuff,(char *)"writesds2dBytes DFANputdesc %s Name %s lastref %d DFerror %d",
		sdsout->path,sdsout->name,lastref,DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}
	ret = 0;
OutOfHere:
	if(data)cFree((char *)data);
	data=NULL;
	return ret;
}


static int writesds2dFloat(struct SDS2Dout *sdsout)
{
//	extern int DFerror;
	int32 rank,size[2];
	double vmin,vmax;
	char buff[256];
	long n,length;
	int lastref;
	float *data;
	double v;
	int ret;
	
	ret=1;
	
	data=NULL;

	if(!sdsout){
	    Warning((char *)"writesds2dFloat Found NULL struct pointer\n");
		goto OutOfHere;
	}
	
	if(sdsout->type != DATA_TYPE_FLOAT){
	    sprintf(WarningBuff,(char *)"writesds2dFloat Found Wrong data type %d\n",sdsout->type);
	    Warning(WarningBuff);
		goto OutOfHere;
	}
	
	if(!sdsout->data){
	    Warning((char *)"writesds2dFloat Found NULL data Pointer\n");
		goto OutOfHere;
	}
	
	length=sdsout->ixmax*sdsout->iymax;
	
	if(length <= 0){
	    Warning((char *)"writesds2dFloat Found data length less than one\n");
		goto OutOfHere;
	}
	
	data=(float *)cMalloc(length*sizeof(float),1002);
	if(!data){
	    sprintf(WarningBuff,(char *)"writesds2dFloat error Trying To allocate %ld Bytes\n",length*sizeof(float));
	    Warning(WarningBuff);
		goto OutOfHere;
	}

	vmin =  1e33;
	vmax = -1e33;
	for(n=0;n<length;++n){
	    v=sdsout->data[n];
	    data[n]=(float)v;
	    if(v < vmin)vmin = v;
	    if(v > vmax)vmax = v;
	}
	
	rank=2;
	size[0]=(int)sdsout->iymax;
	size[1]=(int)sdsout->ixmax;

	if(sdsout->n == 0){
	    unlink((char *)sdsout->path);
	    DFSDclear();
	    DFSDrestart();
        DFSDsetNT(DFNT_FLOAT32);
	    if(DFSDputdata((char *)sdsout->path,rank,size,(float *)data)){
	        sprintf(WarningBuff,(char *)"writesds2d DFSDputdata error %d",DFerror);
	        Warning(WarningBuff);
	        goto OutOfHere;
	    }
	}else{
	    if(DFSDadddata((char *)sdsout->path,rank,size,(float *)data)){
	        sprintf(WarningBuff,(char *)"writesds2d DFSDadddata error %d",DFerror);
	        Warning(WarningBuff);
	        goto OutOfHere;
	    }
	}
	

	lastref=DFSDlastref();
	if(lastref == -1){
	    sprintf(WarningBuff,(char *)"writesds2d DFSDlastref error %d",DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}

	if(DFANputlabel(sdsout->path,DFTAG_SDG,lastref,sdsout->name) == -1){
	    sprintf(WarningBuff,(char *)"writesds2d DFANputlabel %s Name %s lastref %d error %d",
		               sdsout->path,sdsout->name,lastref,DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}
	if(sdsout->pioName){
	    sprintf(buff,(char *)"xmin %g ymin %g zmin %g xmax %g ymax %g zmax %g vmin %g vmax %g time %g pioName \"%s\" ",
	            sdsout->xmin,sdsout->ymin,sdsout->zmin,
                sdsout->xmax,sdsout->ymax,sdsout->zmax,
                vmin,vmax,sdsout->time,sdsout->pioName);
	}else{
	    sprintf(buff,(char *)"xmin %g ymin %g zmin %g xmax %g ymax %g zmax %g vmin %g vmax %g time %g",
	            sdsout->xmin,sdsout->ymin,sdsout->zmin,
                sdsout->xmax,sdsout->ymax,sdsout->zmax,
                vmin,vmax,sdsout->time);
	}
	if(DFANputdesc(sdsout->path,DFTAG_SDG,lastref,(char *)buff,strlen((char *)buff)) == -1){
	    sprintf(WarningBuff,(char *)"DFANputdesc %s Name %s lastref %d DFerror %d",
		sdsout->path,sdsout->name,lastref,DFerror);
	    Warning(WarningBuff);
	    goto OutOfHere;
	}
	ret = 0;
OutOfHere:
	if(data)cFree((char *)data);
	data=NULL;
	return ret;
}

int Warning(char *mess)
{
    if(!mess)return 1;
    fprintf(stderr,"%s",mess);
    return 0;
}



