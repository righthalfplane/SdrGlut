#ifndef __SDRWXH__

#define __SDRWXH__


//#define GLEW_IN
#ifdef GLEW_IN
#include <GL/glew.h>
#endif

#include <wx/wx.h>
#include <wx/sizer.h>
#include <wx/radiobox.h>
#include <wx/textctrl.h>
#include <wx/glcanvas.h>
#include <wx/splitter.h>
#include <wx/arrstr.h>
#include <wx/ctrlsub.h>
#include <wx/window.h>
#include <wx/gbsizer.h>
#include <wx/listctrl.h>
#include <wx/dataview.h>
#include <wx/cmdline.h>
#include <wx/event.h>


#include <string.h>

#if !wxUSE_GLCANVAS
    #error "OpenGL required: set wxUSE_GLCANVAS to 1 and rebuild the library"
#endif


// include OpenGL
#ifdef __WXMAC__
#include "OpenGL/glu.h"
#include "OpenGL/gl.h"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif
 
#include <sys/timeb.h>

#include <fftw3.h>

#include <chrono>

#include "sdr.h"

#include "graphics.h"


double rtime(void);

#define FLOAT_NOT_SET ((double)(-1.23e-21))

#define maxmy(x1,x2)    (((x1) > (x2)) ? (x1) : (x2))

 enum {
    ID_INSERT = wxID_HIGHEST,
    ID_DELETE,
    ID_RENDER,
    TEXT_CLIPBOARD_COPY,
    TEXT_CLIPBOARD_PASTE,
    TIMER_ID,
    ID_COMBOBOX,
    ID_COMBOMODE,
    wxID_MAIN_SPLITTER,
    wxID_BM_SPLITTER,
    ID_ABOUT,
    ID_RADIO,
    ID_QUIT,
    ID_FILE,
    ID_DEVICE,
    ID_FREQUENCY=ID_DEVICE+200,
    ID_DUMMY=ID_FREQUENCY+200,
    ID_TEST,
    ID_SETDEVICE,
    ID_TEXTCTRL,
    ID_STARTRADIO,
    ID_RADIOBOX,
    INPUT_MENU,
	OUTPUT_MENU=INPUT_MENU+100,
    ID_COMBOFILTER=OUTPUT_MENU+100,
    ID_COMBOANTENNA=ID_COMBOFILTER+100,
    ID_COMBOSAMPLERATE=ID_COMBOANTENNA+100,
    ID_COMBOBANDWIDTH=ID_COMBOSAMPLERATE+100,
    ID_PALETTE=ID_COMBOBANDWIDTH+100,
    ID_OPTIONS=ID_PALETTE+100,
    ID_DIRECT=ID_OPTIONS+100,
    ID_BAND=ID_DIRECT+100,
    ID_SAMPLERATE=ID_BAND+100,
    SCROLL_GAIN=ID_SAMPLERATE+100,
    ID_RXGAIN,
    ID_SAMPLEWIDTH=ID_RXGAIN+100,
    ID_COMBOBUTTON,
    ID_MININUM,
    ID_MAXINUM,
    ID_CHECKAUTO,
    ID_RECORD,
    SCROLL_GAIN2,
    SCROLL_TIME2,
    ID_PAUSE,
    TIMER_ID2,
    ID_DATATYPE,
    ID_DATAMODE,
    ID_STARTSEND,
    ID_STOPSEND,
    ID_SWAPIQ,
    ID_OSCILLOSCOPE,
    ID_SCROLLED,
    ID_ROTATEDATA,
    ID_ALPHA,
    ID_SOFTAUTOGAIN,
    ID_SETGAIN,
    ID_RXFREQUENCY,
    ID_SWEEP,
    ID_FC=ID_RXFREQUENCY+100,
    ID_VIEWSELECTED,
    ID_EXIT,
};


 struct WaterFall4{
    unsigned char *data;
    unsigned char ic[2*131072];
    double amin,amax;
    uRect SRect;
    uRect DRect;
    int nline;
    int xsize;
    int ysize;
};

class Sweep;

class Spectrum;

class TopPane : public wxWindow
{
    wxGLContext*	m_context;

public:
	TopPane(wxWindow *frame, const wxString& title);
	virtual ~TopPane();
    

	void resized(wxSizeEvent& evt);
    
	int getWidth();
	int getHeight();
	float angle;
	
	wxRadioBox *radioBoxp;
	wxTextCtrl *text;
	
	wxTimer m_timer;
	
	class sdrClass *sdr;
	
	const wxPoint pt =  wxPoint(160,-5);
	
	wxSize fontSize;
	
	wxFont *font;
	
	int nchar;
	
	int idoFC;
	
	Spectrum *gSpectrum;

	wxFrame *frame;
    
	void render(wxPaintEvent& evt);
		
	wxCheckBox *rbox;
	
	wxCheckBox *rbox2;

	// events
	void OnCombo(wxCommandEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
    void DeleteRow( wxCommandEvent& event );
    void OnButton( wxCommandEvent& event );
    void Record( wxCommandEvent& event );
    void startRadio( wxCommandEvent& event );
    void radioBox( wxCommandEvent& event );
    
	DECLARE_EVENT_TABLE()
}; 
 
class WaterFall;

class BasicPane;

class ApplFrame : public wxFrame
{
public:
    ApplFrame(wxFrame* parent,wxString title,class sdrClass *sdrIn);
    ~ApplFrame();

	void About(wxCommandEvent &event );
    void OnHideBtn(wxCommandEvent&);
    void OnShowBtn(wxCommandEvent&);
    void OnMoveBtn(wxCommandEvent&);
    void OnInputSelect(wxCommandEvent& event);
    void OnOuputSelect(wxCommandEvent& event);
    void resized(wxSizeEvent& evt);
    void OnPaletteSelected(wxCommandEvent& event);
    void OnOptionsSelected(wxCommandEvent& event);
    void OnDirectSelected(wxCommandEvent& event);
    void OnBandSelected(wxCommandEvent& event);
    void OnSampleRateSelected(wxCommandEvent& event);
    void OnChar(wxKeyEvent& event);


	WaterFall *gWaterFall;
	
	BasicPane *gBasicPane;

	wxMenu *actionMenu;
	
	wxMenu *directMenu;
	
	wxMenu *bandMenu;
	
	wxMenu *sampleRateMenu;
	
	class sdrClass *sdr;

	wxMenuItem *itm;
	
	wxMenu *paletteMenu;

	int SampleFrequency;
	
    wxGridBagSizer*     m_gbs;

private:
    wxPanel*            m_panel;
    wxButton*           m_hideBtn;
    wxButton*           m_showBtn;
    wxTextCtrl*         m_hideTxt;

    wxButton*           m_moveBtn1;
    wxButton*           m_moveBtn2;
    wxGBPosition        m_lastPos;

    wxDECLARE_EVENT_TABLE();
};

class BasicPane;


class SelectionWindow : public wxWindow
{
    wxGLContext*	m_context;

public:
	SelectionWindow(wxWindow *frame, const wxString& title,wxTextCtrl *text);
	virtual ~SelectionWindow();
    
	void resized(wxSizeEvent& evt);
    
	int getWidth();
	int getHeight();
	float angle;
	

	wxCheckBox *pSweep;

	wxRadioBox *radioBoxp;
	wxTextCtrl *text;
	
	
	wxComboBox *boxList[200];
	wxComboBox *boxMode;
	
	wxTimer m_timer;
	
	class sdrClass *sdr;

	void killMe();

    BasicPane *gBasicPane;
    
	void render(wxPaintEvent& evt);
	
	void openIQFile(const char *file);
	
	void openWindows(char *name);
	
	std::vector<std::string> deviceNames;


	void OpenFile();
	// events
	void OnDevice(wxCommandEvent& event );
	void OnAbout(wxCommandEvent& event );
	void OnRadio(wxCommandEvent& event );
	void OnQuit(wxCommandEvent& event );
	void doQuit();
	void OnFile(wxCommandEvent& event );

	void OnCombo(wxCommandEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
    void DeleteRow( wxCommandEvent& event );
    void OnButton( wxCommandEvent& event );
    void OnButton2( wxCommandEvent& event );
    void startRadio( wxCommandEvent& event );
    void radioBox( wxCommandEvent& event );
    
	DECLARE_EVENT_TABLE()
}; 
 
class StartWaveFile : public wxWindow
{
 //   wxGLContext*	m_context;

public:
	StartWaveFile(wxWindow *frame, const wxString& title);
	virtual ~StartWaveFile();
	void mouseDown(wxMouseEvent& event);
	
	void OnScroll(wxCommandEvent& event);

	void Pause( wxCommandEvent& event );

	void OnIdle(wxTimerEvent &event);
	
	wxSlider *sliderTime;

	wxTimer m_timer;

	int wavBuffer();
	
	long long CurrentFrame;

	volatile int wavFlag;
	
	int nReadBlock;
	
	wxCheckBox *pbox;
	
	double gain;
	
	volatile int iPause;
	
	SNDFILE *infile;
    SF_INFO sfinfo;
    
    cStack *bS;
	DECLARE_EVENT_TABLE()

};

class StartWindow : public wxWindow
{
    wxGLContext*	m_context;

public:
	StartWindow(wxWindow *frame, const wxString& title);
	virtual ~StartWindow();
    
	void resized(wxSizeEvent& evt);
	
    
	int getWidth();
	int getHeight();
	float angle;
	
	wxFrame *frame;
	
	wxFrame *frame7;
	
	ApplFrame *grab;

	wxRadioBox *radioBoxp;
	
	wxTextCtrl *text;
	
	wxTextCtrl *textDevice;
	
	wxTimer m_timer;
	
	class sdrClass *sdr;

    BasicPane *gBasicPane;
    
	void render(wxPaintEvent& evt);
	
	void openIQFile(const char *file);
	
	void openWavFile(const char *file);
	
	void openArgcArgv(int argc,char **argv);
	
	void openWindows(char *name);

	void OpenFile();
	// events
	void OnAbout(wxCommandEvent& event );
	void OnRadio(wxCommandEvent& event );
	void OnQuit(wxCommandEvent& event );
	void doQuit();
	void OnFile(wxCommandEvent& event );
	void OnTest(wxCommandEvent& event );
	void OnIdle(wxIdleEvent& event); 


	void OnCombo(wxCommandEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
    void DeleteRow( wxCommandEvent& event );
    void OnButton( wxCommandEvent& event );
    void OnButton2( wxCommandEvent& event );
    void startRadio( wxCommandEvent& event );
    void radioBox( wxCommandEvent& event );
    
	DECLARE_EVENT_TABLE()
}; 
 
class Spectrum : public wxGLCanvas
{
    wxGLContext*	m_context;

public:
	Spectrum(wxFrame* parent, int* args);
	virtual ~Spectrum();
    
	void resized(wxSizeEvent& evt);
	
	float scaleFactor;
    
	int getWidth();
	int getHeight();
	float angle;
	
	float *buffSend10;
	float *buffSend2;
	float *buffSend;
	int buffSendLength;
	fftwf_plan p1;
	double lineDumpInterval;
	double lineTime;
	
	volatile int buffFlag;
	
	char **argv;
	int argc;

	int filterType;
	
	int iHaveData;
	
	int iFreeze;
	
	float *buff1;
	
	float *buff2;
	
	float *buff3;
	
	int buffSize;
		
	int decodemode1;
	
	msresamp_crcf iqSampler1;
	
	volatile int iWait; 
	
	volatile int softAutoGain;
	
	volatile int oscilloscope;
	
	TopPane *gTopPane;
	
	BasicPane *gBasicPane;
	
	double lineAlpha;
	
	class sdrClass *sdr;
	
	void startRadio2();
	
	int ftox(double frequency);
	
	double ftime(); 
	std::chrono::high_resolution_clock *startTime;
		
 	volatile double verticalMinimum;
	
	volatile double verticalMaximum;
	
	volatile double amaxGlobal;
   
	void render(wxPaintEvent& evt);
	void render1(wxPaintEvent& evt);
	void render1a(wxPaintEvent& evt);
	void render2(wxPaintEvent& evt);
	void prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
	//void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
	void InitOpenGl();
	int doTestSpeed();
    
	// events
	void OnIdle(wxIdleEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
    void DeleteRow( wxCommandEvent& event );
    
	DECLARE_EVENT_TABLE()
};

class WaterFall : public wxGLCanvas
{
    wxGLContext*	m_context;

public:
	WaterFall(wxFrame* parent, int* args);
	virtual ~WaterFall();
    
	void resized(wxSizeEvent& evt);
	
	float scaleFactor;
    
	int getWidth();
	int getHeight();
	float angle;
	
	volatile double verticalMinimum;
	
	volatile double verticalMaximum;
		
	volatile int buffFlag;
	 
	volatile int iWait; 
	
	struct WaterFall4 water;
	
	struct paletteDraw pd;
	
	double pmin;
	
    double pmax;

	
	TopPane *gTopPane;
	
	Spectrum *gSpectrum;
	
	BasicPane *gBasicPane;
	
	class sdrClass *sdr;
	
	void startRadio2();
	
	int ftox(double frequency);
	
	double ftime(); 
	std::chrono::high_resolution_clock *startTime;
	int SetWindow();
    int FloatToImage(float *d,long length,struct paletteDraw *pd,unsigned char *bp);
	void render(wxPaintEvent& evt);
	void prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
	//void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
	void InitOpenGl();
	int doTestSpeed();
    
	// events
	void OnIdle(wxIdleEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
    void DeleteRow( wxCommandEvent& event );
    
	DECLARE_EVENT_TABLE()
};

struct sendData{
	char *name;
	int dataType;
	int sendMode;
	SOCKET send;
	volatile int controlSend;
    volatile int fillBuffer;
	unsigned short Port;
	double aminGlobal2;
    double amaxGlobal2;
    float *sendBuff1;
    float *sendBuff2;
};


class BasicPane;

class BasicPane : public wxWindow
{
    wxGLContext*	m_context;

public:
	BasicPane(wxWindow *frame, const wxString& title,class sdrClass *sdrIn);
	virtual ~BasicPane();
    

	wxScrolledWindow *ScrolledWindow;

	void resized(wxSizeEvent& evt);
	
	int SetScrolledWindow();
	
	int scrolledWindowFlag;
	
	int SendStart(char *name,int type,int mode);
	
	int sendAudio(int short *data,int length);
	
	int rxSend();
	
	int txPipe();
    
	int getWidth();
	int getHeight();
	float angle;
	
	int softAutoGain;

	Spectrum *gSpectrum;
	
	WaterFall *gWaterFall;
	
	TopPane *gTopPane;
	
	ApplFrame *gApplFrame;
	
	BasicPane *gBasicPane;

	wxRadioBox *radioBoxp;
	
	wxTextCtrl *text;
	
	wxRadioBox *modeBox;
	
	wxTimer m_timer;
	
	class sdrClass *sdr;

	wxFrame *frame;
    
	void render(wxPaintEvent& evt);
	
	int iRefresh;
	
	volatile int sendFlag;
	
	double lineAlpha;
	
	double sampleDataRotate;
	
	wxComboBox *fftCombo;
	
	wxComboBox *filterCombo;
	
	wxComboBox *antennaCombo;
	
	wxComboBox *sampleRateCombo;
	
	wxComboBox *bandwidthCombo;
	
	wxRadioBox *sendTypeBox;
	
	wxRadioBox *sendModeBox;
	
	wxTextCtrl *sendAddress;
	
	wxTextCtrl *textAlpha;
	
	struct sendData sxs;
	
	struct sendData *sx=&sxs;
	
	char addressName[256];
	
	wxTextCtrl *rangeMin;
	
	wxTextCtrl *rangeMax;
		
	wxDataViewListCtrl *listctrlFreq;
	
	  wxString computers31[35] =
      { "2","3","4","5","6",
      	"7","8","9","10","11","12","13",
        "14","15","16","17","18", "19","20","21","22","23",
      	"24","25","26","27","28", "29","30","31","32","33",
        "34","35","36"};

	  wxString computers32[35] =
       { "57","63","69","79","85",
       	 "177","183","189","195","201","207","213",
         "473","479","485","491","497", "503","509","515","521","527",
      	 "533","539","545","551","557", "563","569","575","581","587",
         "593","599","605"};

	

	// events
	void OnViewSelected(wxDataViewEvent &event);
	void OnTimer(wxTimerEvent &event);
	void OnCombo(wxCommandEvent& event);
	void OnComboFilter(wxCommandEvent& event);
	void OnComboAntenna(wxCommandEvent& event);
	void OnComboSampleRate(wxCommandEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
    void DeleteRow( wxCommandEvent& event );
    void OnButton( wxCommandEvent& event );
    void OnButton2( wxCommandEvent& event );
    void startRadio( wxCommandEvent& event );
    void startSend( wxCommandEvent& event );
    void stopSend( wxCommandEvent& event );
    void radioBox( wxCommandEvent& event );
    void dataType( wxCommandEvent& event );
    void dataMode( wxCommandEvent& event );
    void OnScroll(wxCommandEvent& event);
    void OnText(wxCommandEvent& event);
    void setSampleRate( wxCommandEvent& event );
    void OnMinimun(wxCommandEvent& event);
    void OnMaximun(wxCommandEvent& event);
    void OnCheckAuto(wxCommandEvent& event);
    void setBandwidth( wxCommandEvent& event );
    void OnTextBandWidth(wxCommandEvent& event);
    void setSampleWidth(wxCommandEvent& event);
    void setDataRotate(wxCommandEvent& event);
    void setRxGain(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
}; 
 
int cFree(char *p);

class TopPane2;
class BasicPane2;
class Spectrum2;
class WaterFall2;


class TopPane2 : public wxWindow
{
    wxGLContext*	m_context;

public:
	TopPane2(wxWindow *frame, const wxString& title);
	virtual ~TopPane2();
    

	void resized(wxSizeEvent& evt);
    
	int getWidth();
	int getHeight();
	float angle;
	
	wxRadioBox *radioBoxp;
	wxTextCtrl *text;
	
	wxTimer m_timer;
	
	class sdrClass *sdr;
	
	const wxPoint pt =  wxPoint(160,-5);
	
	wxSize fontSize;
	
	wxFont *font;
	
	int nchar;
	
	int idoFC;
	
	Spectrum2 *gSpectrum2;

	wxFrame *frame;
    
	void render(wxPaintEvent& evt);
		
	wxCheckBox *rbox;
	
	wxCheckBox *rbox2;

	// events
	void OnCombo(wxCommandEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
    void DeleteRow( wxCommandEvent& event );
    void OnButton( wxCommandEvent& event );
    void Record( wxCommandEvent& event );
    void startRadio( wxCommandEvent& event );
    void radioBox( wxCommandEvent& event );
    
	DECLARE_EVENT_TABLE()
}; 


class Spectrum2 : public wxGLCanvas
{
    wxGLContext*	m_context;

public:
	Spectrum2(wxFrame* parent, int* args);
	virtual ~Spectrum2();
    
	void resized(wxSizeEvent& evt);
	
	float scaleFactor;
    
	int getWidth();
	int getHeight();
	float angle;
	
	float *buffSend10;
	float *buffSend2;
	float *buffSend;
	int buffSendLength;
	fftwf_plan p1;
	double lineDumpInterval;
	double lineTime;
	
	volatile int buffFlag;
	
	char **argv;
	int argc;

	int filterType;
	
	int iHaveData;
	
	int iFreeze;
	
	float *buff1;
	
	float *buff2;
	
	float *buff3;
	
	int buffSize;
		
	int decodemode1;
	
	msresamp_crcf iqSampler1;
	
	volatile int iWait; 
	
	volatile int softAutoGain;
	
	volatile int oscilloscope;
	
	TopPane2 *gTopPane2;
	
	BasicPane2 *gBasicPane2;
	
	double lineAlpha;
	
	class sdrClass *sdr;
	
	void startRadio2();
	
	int ftox(double frequency);
	
	double ftime(); 
	std::chrono::high_resolution_clock *startTime;
		
 	volatile double verticalMinimum;
	
	volatile double verticalMaximum;
	
	volatile double amaxGlobal;
   
	void render(wxPaintEvent& evt);
	void render1(wxPaintEvent& evt);
	void render1a(wxPaintEvent& evt);
	void render2(wxPaintEvent& evt);
	void prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
	//void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
	void InitOpenGl();
	int doTestSpeed();
    
	// events
	void OnIdle(wxIdleEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
    void DeleteRow( wxCommandEvent& event );
    
	DECLARE_EVENT_TABLE()
};

class WaterFall2 : public wxGLCanvas
{
    wxGLContext*	m_context;

public:
	WaterFall2(wxFrame* parent, int* args);
	virtual ~WaterFall2();
    
	void resized(wxSizeEvent& evt);
	
	float scaleFactor;
    
	int getWidth();
	int getHeight();
	float angle;
	
	volatile double verticalMinimum;
	
	volatile double verticalMaximum;
		
	volatile int buffFlag;
	 
	volatile int iWait; 
	
	struct WaterFall4 water;
	
	struct paletteDraw pd;
	
	double pmin;
	
    double pmax;

	
	TopPane2 *gTopPane2;
	
	Spectrum2 *gSpectrum2;
	
	BasicPane2 *gBasicPane2;
	
	class sdrClass *sdr;
	
	void startRadio2();
	
	int ftox(double frequency);
	
	double ftime(); 
	std::chrono::high_resolution_clock *startTime;
	int SetWindow();
    int FloatToImage(float *d,long length,struct paletteDraw *pd,unsigned char *bp);
	void render(wxPaintEvent& evt);
	void prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
	//void prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
	void InitOpenGl();
	int doTestSpeed();
    
	// events
	void OnIdle(wxIdleEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
    void DeleteRow( wxCommandEvent& event );
    
	DECLARE_EVENT_TABLE()
};

class BasicPane2;


class BasicPane2 : public wxWindow
{
    wxGLContext*	m_context;

public:
	BasicPane2(wxWindow *frame, const wxString& title,class sdrClass *sdrIn);
	virtual ~BasicPane2();
    

	wxScrolledWindow *ScrolledWindow;

	void resized(wxSizeEvent& evt);
	
	int SetScrolledWindow();
	
	int scrolledWindowFlag;
	
	int SendStart(char *name,int type,int mode);
	
	int sendAudio(int short *data,int length);
	
	int rxSend();
	
	int txPipe();
    
	int getWidth();
	int getHeight();
	float angle;
	
	int softAutoGain;

	Spectrum2 *gSpectrum2;
	
	WaterFall2 *gWaterFall2;
	
	TopPane2 *gTopPane2;
	
	Sweep *gSweep;
	
	BasicPane2 *gBasicPane2;

	wxRadioBox *radioBoxp;
	
	wxTextCtrl *text;
	
	wxRadioBox *modeBox;
	
	wxTimer m_timer;
	
	class sdrClass *sdr;

	wxFrame *frame;
    
	void render(wxPaintEvent& evt);
	
	int iRefresh;
	
	volatile int sendFlag;
	
	double lineAlpha;
	
	double sampleDataRotate;
	
	wxComboBox *fftCombo;
	
	wxComboBox *filterCombo;
	
	wxComboBox *antennaCombo;
	
	wxComboBox *sampleRateCombo;
	
	wxComboBox *bandwidthCombo;
	
	wxRadioBox *sendTypeBox;
	
	wxRadioBox *sendModeBox;
	
	wxTextCtrl *sendAddress;
	
	wxTextCtrl *textAlpha;
	
	struct sendData sxs;
	
	struct sendData *sx=&sxs;
	
	char addressName[256];
	
	wxTextCtrl *rangeMin;
	
	wxTextCtrl *rangeMax;
		
	wxDataViewListCtrl *listctrlFreq;
	
	  wxString computers31[35] =
      { "2","3","4","5","6",
      	"7","8","9","10","11","12","13",
        "14","15","16","17","18", "19","20","21","22","23",
      	"24","25","26","27","28", "29","30","31","32","33",
        "34","35","36"};

	  wxString computers32[35] =
       { "57","63","69","79","85",
       	 "177","183","189","195","201","207","213",
         "473","479","485","491","497", "503","509","515","521","527",
      	 "533","539","545","551","557", "563","569","575","581","587",
         "593","599","605"};

	

	// events
	void OnViewSelected(wxDataViewEvent &event);
	void OnTimer(wxTimerEvent &event);
	void OnCombo(wxCommandEvent& event);
	void OnComboFilter(wxCommandEvent& event);
	void OnComboAntenna(wxCommandEvent& event);
	void OnComboSampleRate(wxCommandEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
	void OnChar(wxKeyEvent& event);
    void DeleteRow( wxCommandEvent& event );
    void OnButton( wxCommandEvent& event );
    void OnButton2( wxCommandEvent& event );
    void startRadio( wxCommandEvent& event );
    void startSend( wxCommandEvent& event );
    void stopSend( wxCommandEvent& event );
    void radioBox( wxCommandEvent& event );
    void dataType( wxCommandEvent& event );
    void dataMode( wxCommandEvent& event );
    void OnScroll(wxCommandEvent& event);
    void OnText(wxCommandEvent& event);
    void setSampleRate( wxCommandEvent& event );
    void OnMinimun(wxCommandEvent& event);
    void OnMaximun(wxCommandEvent& event);
    void OnCheckAuto(wxCommandEvent& event);
    void setBandwidth( wxCommandEvent& event );
    void OnTextBandWidth(wxCommandEvent& event);
    void setSampleWidth(wxCommandEvent& event);
    void setDataRotate(wxCommandEvent& event);
    void setRxGain(wxCommandEvent& event);
	DECLARE_EVENT_TABLE()
}; 
 



class Sweep : public wxFrame
{
public:
    Sweep(wxFrame* parent,wxString title,class sdrClass *sdrIn);
    ~Sweep();

	void About(wxCommandEvent &event );
    void OnHideBtn(wxCommandEvent&);
    void OnShowBtn(wxCommandEvent&);
    void OnMoveBtn(wxCommandEvent&);
    void OnInputSelect(wxCommandEvent& event);
    void OnOuputSelect(wxCommandEvent& event);
    void resized(wxSizeEvent& evt);
    void OnPaletteSelected(wxCommandEvent& event);
    void OnOptionsSelected(wxCommandEvent& event);
    void OnDirectSelected(wxCommandEvent& event);
    void OnBandSelected(wxCommandEvent& event);
    void OnSampleRateSelected(wxCommandEvent& event);
    void OnChar(wxKeyEvent& event);


	WaterFall2 *gWaterFall2;
	
	BasicPane2 *gBasicPane2;

	wxMenu *actionMenu;
	
	wxMenu *directMenu;
	
	wxMenu *bandMenu;
	
	wxMenu *sampleRateMenu;
	
	class sdrClass *sdr;

	wxMenuItem *itm;
	
	wxMenu *paletteMenu;

	int SampleFrequency;
	
    wxGridBagSizer*     m_gbs;

private:
    wxPanel*            m_panel;
    wxButton*           m_hideBtn;
    wxButton*           m_showBtn;
    wxTextCtrl*         m_hideTxt;

    wxButton*           m_moveBtn1;
    wxButton*           m_moveBtn2;
    wxGBPosition        m_lastPos;

    wxDECLARE_EVENT_TABLE();
};


#endif
