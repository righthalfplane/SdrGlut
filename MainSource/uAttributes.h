#ifndef _UATTRIBUTES_
#define _UATTRIBUTES_

struct uFontStuff{
	int txFont;
	int txFace;
	int txSize;
	int txMode;
	int txdum1;
	int txdum2;
};

struct uAttributes{
	struct uFontStuff font;	
	int nSolidFore;
	int nSolidBack;
	int nLineFore;
	int nLineBack;
	int SolidPattern;
	int LinePattern;
	int LineSymbol;
	int LineIncrement;
	double xLineWidth;
	double yLineWidth;
	char LineDash[16];
	int DashFlag;
	int LineDashNumber;
	char AnimationFormat[12];	
	double AnimationStart;
	double AnimationStep;
	int Animation;
	int AnimationJust;
	int hideLines;
	int doAreadFill;
};

/*
int uGetFontStuff(struct uFontStuff *f,IconPtr myIcon);
int uSetFontStuff(struct uFontStuff *f,IconPtr myIcon);
int uSetFontMenus(int FontMenuNumber,int StyleMenuNumber,IconPtr myIcon);
*/

#endif
