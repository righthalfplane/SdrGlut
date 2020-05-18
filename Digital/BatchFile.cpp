#define EXTERN22 extern
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <random>
#include "Utilities.h"
#include "SceneList.h"
#include "Poly.h"
#include "CLines.h"
#include "Radio.h"

int doWindow(double *x,double *y,long length,int type);

extern "C" int doFFT2(double *x,double *y,long length,int direction);

char *DefaultPathString(void);

int doFIRRead(BatchPtr Batch,double value);

int doDft(BatchPtr Batch,double value);

int doForce(BatchPtr Batch,CommandPtr cp);

int doDiff(BatchPtr Batch);

int doBilinear(BatchPtr Batch,double value);

int doForces(BatchPtr Batch,double value);

int doWarp(BatchPtr Batch,double value);

int doInvert(BatchPtr Batch,double value);

int doBatch(BatchPtr Batch,CommandPtr cp);

int doButter(BatchPtr Batch,double value,double value1);

int doChev(BatchPtr Batch,double value,double value2);

int doLow(BatchPtr Batch,double value,double value2);

int doHigh(BatchPtr Batch,double value,double value2);

int doTrans(BatchPtr Batch,double value,double value2);

int doSweep(BatchPtr Batch,double value1,double value2,double value3,double value4,double ilog);

int doBand(BatchPtr Batch,double value1,double value2,double value3);

int doMarch(BatchPtr Batch,double value1,double value2,double value3);

int doResponse(BatchPtr Batch,double value);

static char working[256];

static char fname[256];

static struct Icon myIcon;

static int initScene(struct Scene *scene)
{
    
    if(!scene)return 1;
    
    SceneInit(scene);
    
    scene->windowType=FileTypeLines;
    
    scene->scale.showPalette=1;
    
    scene->scale.updateTemperatureScale=1;
    
    scene->scale.logscale=1;
    
    return 0;
}

int BatchPlot(char *name,int flag,double *x,double *y,long n)
{
    char namewindow[256];
    struct Scene *scenel2;
    struct SceneList *list;
    CLines *lines2;

    mstrncpy(namewindow,fname,sizeof(namewindow));
    mstrncat(namewindow,(char*)"-",sizeof(namewindow));
    mstrncat(namewindow,name,sizeof(namewindow));
    fprintf(stderr,"BatchPlot %ld fname %s namewindow %s\n",n,fname,namewindow);
    
    list=SceneNext();
    if(list == NULL)
    {
        WarningPrint("FMRadio : Error Allocation Scene Memory File\n");
        return 0;
    }
    
    scenel2=&list->scene;
    zerol((char *)scenel2,sizeof(struct Scene));
    
    initScene(scenel2);
    
    lines2 = CLines::CLinesOpen(scenel2,-1000);
    
    if(flag == 1){
        //std::default_random_engine generator;
        //std::uniform_real_distribution<double> distribution(-1.0, 1.0);

        double *real=new double[n];
        double *imag=new double[n];
        double *rl=new double[n];
        for(int k=0;k<n;++k){
            real[k]=y[k];
            //real[k]=distribution(generator);
            imag[k]=0.0;
        }
        
        doWindow(real,imag,n,FILTER_BLACKMANHARRIS);
        
        for(int k=0;k<n;++k){
            real[k] *= pow(-1.0,k);
            imag[k] *= pow(-1.0,k);
        }
        
        doFFT2(real,imag,(long)n,1);
        
        for(int k=0;k<n;++k){
            rl[k]=sqrt(real[k]*real[k]+imag[k]*imag[k]);
        }
        
        for(int k=0;k<n;++k){
            real[k]=k*myIcon.pl->sampleRate/(double)n;
        }
        
        lines2->plotPutData(scenel2,real,&rl[n/2],n/2,-1L);
        
        printf("BatchPlot FFT %ld",n);
        
        delete [] real;
        delete [] imag;
        delete [] rl;

    }else{
        lines2->plotPutData(scenel2,x,y,n,-1L);
    }
    
    lines2->sceneSource=NULL;
    
    lines2->wShift=0;
    
    lines2->lines->Plot->yLogScale=0;
    
    lines2->lines->Plot->gridHash=1;
    
    glutSetWindowTitle(namewindow);

    return 0;
}

int BatchNextLine(BatchPtr Batch,char *line,long len)
{
	if(!Batch || !line || (len <= 0))return 1;
	if(!Batch->input){
	    if(Batch->getLine){
	    	return (*Batch->getLine)(Batch->myIcon2,line,len);
	    }else{
	        return 1;
	    }
	}
	
	return NextLine(Batch->input,line,(int)len);
}
int processFile(char *pathname)
{
	struct BatchInfo Batch;
	char line[4096];
	double start,end;
	FILE *input;

	if(!pathname)return 1;
	
	
	zerol((char *)&myIcon,sizeof(struct Icon));
	
	zerol((char *)&Batch,sizeof(struct BatchInfo));
    
    myIcon.pl=new Poly;

	Batch.myIcon=&myIcon;
	
	input=NULL;
	
	start=rtime();
    
    char *p=strrchr(pathname,FILE_NAME_SEPERATOR_CHAR);
    if(p){
        mstrncpy(fname,p,strlen(p)+1);
    }else{
        mstrncpy(fname,pathname,strlen(pathname)+1);
    }

	input=fopen(pathname,"r");
	if(input == NULL){
	    WarningPrint("Could Not Open %s To Read Errno %d\n",pathname,errno);
        GetWorking((char *)working,(long)sizeof(working));
        WarningPrint("Working Directory = %s\n",working);
	    return 1;
	}
	
	Batch.input=input;

	while(1){
	    if(BatchNextLine(&Batch,line,sizeof(line)))break;
	    if(ProcessLine(line,&Batch))break;
	}
	
	end=rtime();
	
	printf("Total Time in processFile %.2f Seconds\n",end-start);
		
	if(input)fclose(input);
    
    delete myIcon.pl;
	
	return 0;
}
int ProcessLine(char *line,BatchPtr Batch)
{
	struct CommandInfo cp;
	int ret;

	if(!line || !Batch)return 1;
	
	ret=1;

	zerol((char *)&cp,sizeof(struct CommandInfo));

	if(getCommand(line,&cp))goto ErrorOut;
	
	for(cp.n=0;cp.n<cp.nword;++cp.n){
	    if(doBatch(Batch,&cp))goto ErrorOut;
	}
	
	ret=0;
	
ErrorOut:	

	getCommand(NULL,&cp);
	
	return ret;
}
int doBatch(BatchPtr Batch,CommandPtr cp)
{
	char *command;
    double value,value1,value2,value3,value4;
	int ret;
	
	if(!Batch || !cp)return 1;
	
	ret = 1;
	command=stringCommand(cp);
	if(!command)goto ErrorOut;
    
   // fprintf(stderr,"command %s\n",command);
	
	if(!mstrcmp((char *)"butter",command)){
        ++(cp->n);
        ret=doubleCommand(&value,cp);
        if(ret)goto ErrorOut;
        ++(cp->n);
        ret=doubleCommand(&value1,cp);
        if(ret)goto ErrorOut;
    	doButter(Batch,value,value1);
    }else if(!mstrcmp((char *)"chev",command)){
        ++(cp->n);
        ret=doubleCommand(&value,cp);
        if(ret)goto ErrorOut;
        ++(cp->n);
        ret=doubleCommand(&value2,cp);
        if(ret)goto ErrorOut;
        doChev(Batch,value,value2);
    }else if(!mstrcmp((char *)"low",command)){
        ++(cp->n);
        ret=doubleCommand(&value,cp);
        if(ret)goto ErrorOut;
        ++(cp->n);
        ret=doubleCommand(&value2,cp);
        if(ret)goto ErrorOut;
        doLow(Batch,value,value2);
    }else if(!mstrcmp((char *)"high",command)){
        ++(cp->n);
        ret=doubleCommand(&value,cp);
        if(ret)goto ErrorOut;
        ++(cp->n);
        ret=doubleCommand(&value2,cp);
        if(ret)goto ErrorOut;
        doHigh(Batch,value,value2);
    }else if(!mstrcmp((char *)"sweep",command)){
        ++(cp->n);
        ret=doubleCommand(&value1,cp);
        if(ret)goto ErrorOut;
        ++(cp->n);
        ret=doubleCommand(&value2,cp);
        if(ret)goto ErrorOut;
        ++(cp->n);
        ret=doubleCommand(&value3,cp);
        if(ret)goto ErrorOut;
        ++(cp->n);
        ret=doubleCommand(&value4,cp);
        if(ret)goto ErrorOut;
        doSweep(Batch,value1,value2,value3,value4,0);
    }else if(!mstrcmp((char *)"band",command)){
        ++(cp->n);
        ret=doubleCommand(&value1,cp);
        if(ret)goto ErrorOut;
        ++(cp->n);
        ret=doubleCommand(&value2,cp);
        if(ret)goto ErrorOut;
        ++(cp->n);
        ret=doubleCommand(&value3,cp);
        if(ret)goto ErrorOut;
         doBand(Batch,value1,value2,value3);
    }else if(!mstrcmp((char *)"march",command)){
        ++(cp->n);
        ret=doubleCommand(&value1,cp);
        if(ret)goto ErrorOut;
        ++(cp->n);
        ret=doubleCommand(&value2,cp);
        if(ret)goto ErrorOut;
        ++(cp->n);
        ret=doubleCommand(&value3,cp);
        if(ret)goto ErrorOut;
        doMarch(Batch,value1,value2,value3);
    }else if(!mstrcmp((char *)"warp",command)){
        ++(cp->n);
        ret=doubleCommand(&value,cp);
        if(ret)goto ErrorOut;
        doWarp(Batch,value);
    }else if(!mstrcmp((char *)"invert",command)){
        ++(cp->n);
        ret=doubleCommand(&value,cp);
        if(ret)goto ErrorOut;
        doInvert(Batch,value);
    }else if(!mstrcmp((char *)"bilinear",command)){
        ++(cp->n);
        ret=doubleCommand(&value,cp);
        if(ret)goto ErrorOut;
        doBilinear(Batch,value);
    }else if(!mstrcmp((char *)"forces",command)){
        ++(cp->n);
        ret=doubleCommand(&value,cp);
        if(ret)goto ErrorOut;
        doForces(Batch,value);
    }else if(!mstrcmp((char *)"response",command)){
        ++(cp->n);
        ret=doubleCommand(&value,cp);
        if(ret)goto ErrorOut;
        doResponse(Batch,value);
    }else if(!mstrcmp((char *)"diff",command)){
        doDiff(Batch);
    }else if(!mstrcmp((char *)"trans",command)){
        ++(cp->n);
        ret=doubleCommand(&value1,cp);
        if(ret)goto ErrorOut;
        ++(cp->n);
        ret=doubleCommand(&value2,cp);
        if(ret)goto ErrorOut;
        ++(cp->n);
        doTrans(Batch,value1,value2);
    }else if(!mstrcmp((char *)"force",command)){
        doForce(Batch,cp);
    }else if(!mstrcmp((char *)"samplerate",command)){
        struct Poly *pl=Batch->myIcon->pl;
        ++(cp->n);
        ret=doubleCommand(&value,cp);
        if(ret)goto ErrorOut;
        pl->sampleRate=value;
    }else if(!mstrcmp((char *)"exit",command)){
        goto ErrorOut;
    }else if(!mstrcmp((char *)"stop",command)){
        goto ErrorOut;
    }else if(!mstrcmp((char *)"dft",command)){
        ++(cp->n);
        ret=doubleCommand(&value,cp);
        if(ret)goto ErrorOut;
        doDft(Batch,value);
    }else if(!mstrcmp((char *)"fir",command)){
        ++(cp->n);
        ret=doubleCommand(&value,cp);
        if(ret)goto ErrorOut;
        doFIRRead(Batch,value);
	}else{
	    WarningPrint("doBatch Unknown Command %s\n",command);
	    goto ErrorOut;
	}
	
	ret = 0;
	
ErrorOut:
	return ret;
}
int doFIRRead(BatchPtr Batch,double value)
{
    struct Poly *pl=Batch->myIcon->pl;
    double value1,value2;
    int nf=(int)value;
    char line[4096];
    struct CommandInfo cp;
    char *command;
    int ret;
    
    if(pl->FIRCoefficients) delete [] pl->FIRCoefficients;
    
    pl->FIRCount=0;
    
    pl->FIRCoefficients = new double[nf];
    
    double *xnp=new double[(long)nf];
    double *ynp=new double[(long)nf];

    zerol((char *)&cp,sizeof(struct CommandInfo));
    
    for(int n=0;n<nf;++n){
        if(BatchNextLine(Batch,line,sizeof(line)))break;
        if(getCommand(line,&cp))break;
        ret=doubleCommand(&value1,&cp);
        ++(cp.n);
        ret=doubleCommand(&value2,&cp);
        ++(cp.n);
        //printf("value1 %f value2 %f\n",value1,value2);
        
        xnp[pl->FIRCount]=(double)n/pl->sampleRate;
        ynp[pl->FIRCount]=value2;

        pl->FIRCoefficients[pl->FIRCount++]=value2;
        
   }

    BatchPlot((char *)"FIRCoefficients",0,xnp,ynp,(long)pl->FIRCount);
    
    delete [] xnp;
    delete [] ynp;

    if(BatchNextLine(Batch,line,sizeof(line)))return 1;
    if(getCommand(line,&cp))return 1;
    command=stringCommand(&cp);
    if(!command)return 1;
    
    //printf("command = %s\n",command);
   
    return 0;
}
int doDft(BatchPtr Batch,double value)
{
    struct Poly *pl=Batch->myIcon->pl;
    
    pl->dft((int)value);
    
    return 0;
}
int doForce(BatchPtr Batch,struct CommandInfo *cp)
{
    char *command;
    double value;
    int ret;

    struct Poly *pl=Batch->myIcon->pl;
    
    
    ++(cp->n);
    command=stringCommand(cp);
    if(!command)goto ErrorOut;
    ++(cp->n);
    ret=doubleCommand(&value,cp);
    if(ret)goto ErrorOut;
    ++(cp->n);

    
      if(!mstrcmp((char *)"step",command)){
          
          int np=(int)value;
          
          fprintf(stderr,"step %d\n",np);
          
          double *force = new double[np];
          
          for(int n=0;n<np;++n)force[n]=1;
          
          pl->force(force,np);
          
          delete [] force;
          
          
      }else if(!mstrcmp((char *)"impulse",command)){
          
          int np=(int)value;
          
          fprintf(stderr,"impulse %d\n",(int)value);
          
          double *force = new double[np];
          
          force[0]=1;
          
          for(int n=1;n<np;++n)force[n]=0;
          
          pl->force(force,np);
          
          delete [] force;
      }else if(!mstrcmp((char *)"random",command)){
          std::default_random_engine generator;
          std::uniform_real_distribution<double> distribution(-1.0, 1.0);

          int np=(int)value;
          
          fprintf(stderr,"random %d\n",(int)value);
          
          double *force = new double[np];
          
          for(int n=0;n<np;++n)force[n]=distribution(generator);
          
          pl->force(force,np);
          
          delete [] force;
      }else if(!mstrcmp((char *)"sin",command)){
          double a=value;
          double npp;
          double f;
          
          ret=doubleCommand(&f,cp);
          if(ret)goto ErrorOut;
          ++(cp->n);
          
          ret=doubleCommand(&npp,cp);
          if(ret)goto ErrorOut;
          ++(cp->n);
          
          int np=(int)npp;
          
          double *force = new double[np];
          
          double pi=4.0*atan(1.0);
          
          fprintf(stderr,"step %d a %g f %g\n",np,a,f);

          for(int n=0;n<np;++n){
              force[n]=a*sin(2*pi*f*n/(double)pl->sampleRate);
          }
          
          pl->force(force,np);
          
          delete [] force;

     }
ErrorOut:
    return 0;
}
int doDiff(BatchPtr Batch)
{
    struct Poly *pl=Batch->myIcon->pl;
    
    pl->diff();
    
    return 0;
}
int doResponse(BatchPtr Batch,double value)
{
    struct Poly *pl=Batch->myIcon->pl;
    
    pl->response(value);
    
    return 0;
}

int doBilinear(BatchPtr Batch,double value)
{
    struct Poly *pl=Batch->myIcon->pl;
    
    pl->bilinear(value);
    
    return 0;
}

int doForces(BatchPtr Batch,double value)
{
    struct Poly *pl=Batch->myIcon->pl;
    
    pl->forces(Batch,value);
    
    return 0;
}


int doWarp(BatchPtr Batch,double value)
{
    struct Poly *pl=Batch->myIcon->pl;
    
    pl->warp(value);
    
    return 0;
}
int doInvert(BatchPtr Batch,double value)
{
    struct Poly *pl=Batch->myIcon->pl;
    
    pl->invert(value);
    
    return 0;
}


int doBand(BatchPtr Batch,double value1,double value2,double value3)
{
    struct Poly *pl=Batch->myIcon->pl;
    
    pl->band(value1,(int)value2,value3);
    
    return 1;
}

int doMarch(BatchPtr Batch,double value1,double value2,double value3)
{
    struct Poly *pl=Batch->myIcon->pl;
    
    pl->march(value1,value2,value3);
    
    return 1;
}

int doHigh(BatchPtr Batch,double value,double value2)
{
    struct Poly *pl=Batch->myIcon->pl;
    
    pl->high(value,(int)value2);
    
    return 0;
}
int doTrans(BatchPtr Batch,double value1,double value2)
{
    struct Poly *pl=Batch->myIcon->pl;
    
    pl->trans(Batch,(int)value1,(int)value2);
    
    return 1;
}


int doSweep(BatchPtr Batch,double value1,double value2,double value3,double value4,double ilog)
{
    struct Poly *pl=Batch->myIcon->pl;
    
    pl->sweep(value1,value2,(int)value3,(int)value4,(int)ilog);

    return 1;
}
int doLow(BatchPtr Batch,double value,double value2)
{
    struct Poly *pl=Batch->myIcon->pl;
    
    pl->low(value,(int)value2);
    
    return 0;
}


int doChev(BatchPtr Batch,double value,double value2)
{
    struct Poly *pl=Batch->myIcon->pl;
    
    int np=(int)value;
    
    pl->doChev(np,value2);
    
    return 0;
}
int doButter(BatchPtr Batch,double value,double value1)
{
    struct Poly *pl=Batch->myIcon->pl;
    
	int np=(int)value;
    
    pl->doButterWorth(np);
    
	return 0;
}
int getCommand(char *line,CommandPtr cp)
{
	static char number[]={
			'0','1','2','3','4','5','6','7','8','9',
			'+','-','.'};
	char buff[256];
	int c,n,nw,iret,k,inum;
	
	if(!cp)return 1;

	for(n=0;n<256;++n){
	    cp->type[n]=BATCH_DOUBLE;
	    cp->value[n]=0;
	    if(cp->command[n])cFree((char *)cp->command[n]);
	    cp->command[n]=NULL;
	}

	cp->line=line;
	cp->nword=0;
	cp->n=0;
	
    if(!line)return 1;
    
	nw=0;
	do{

		while((c = *line) != 0 && (c == ' ' || c == '\n' || c == '\r' || c == '\t')){
			line++;
		}
		
		n=0;
		if(c == 0){
			break;
		}else if(c == '"'){	
		    line++;	
			while((c = *line++) != 0 && c != '"' && n < 255 ){
				buff[n++]=c;
			}
			buff[n]=0;
			iret = 0;
	    	cp->type[nw]=BATCH_STRING;
		    cp->command[nw]=strsave(buff,9153);
		    if(!cp->command[nw]){
				return 1;
		    }
			continue;
		}else{
			while((c = *line++) != 0 && c != ' ' && c != '\n' && c != '\r' && c != '\t'
							 && n < 255 ){
				buff[n++]=c;
			}
		}
		
		if(c == ' ' || c == '"' || c == '\t'){
			iret = 0;
		}else{
			iret = 1;
		}
	
		buff[n]=0;
	
		if(!mstrcmp(buff,(char *)".") || !mstrcmp(buff,(char *)"..")){
	    	cp->type[nw]=BATCH_STRING;
		    cp->command[nw]=strsave(buff,9154);
		    if(!cp->command[nw])return 1;
			continue;
		}
	
		inum = 0;
		for(k=0;k<(int)sizeof(number);++k){
			if(*buff == number[k]){
				inum = 1;
				break;
			}
		}
	
		if(inum && (cp->getKind != BATCH_STRING)){
			 cp->value[nw] = atof(buff);
		}else{
		    cp->command[nw]=strsave(buff,9155);
		    if(!cp->command[nw])return 1;
	    	cp->type[nw]=BATCH_STRING;
		}
	
    }while((++nw < 256) &&  !iret);

    cp->nword = nw;

	cp->getKind=0;
	    	
    return 0;
}
int NextLine(FILE *input,char *line,int linelength)
{
	int nc;
	int c;

	if(!input || !line || linelength <= 0)return 1;

	--linelength;
	
doWhite:

	while(1){
	    c=fgetc(input);
	    if(c != ' ' || c != '\t' || c != ','){
	        break;
	    }
	}

	if(c == EOF){
	    return 1;
	}else if(c == '!'){
	    while(1){
	        c=fgetc(input);
	        if(c == '\n' || c == '\r' || c == EOF){
	            break;
	        }
	    }
	    if(c == EOF){
	        return 1;
	    }else{
	       goto doWhite;
	    }
	}

	ungetc(c,input);

	nc=0;
	while(1){
	    c=fgetc(input);
	    if(c == '\n' || c == '\r' || c == EOF){
	        break;
	    }
	    if(nc >= linelength)break;
	    line[nc++] = c;
	}

	if(c == EOF){
	    return 1;
	}
	
	if((nc == 0) && (linelength > 0))goto doWhite;
	
	line[nc]=0;
	
	return 0;
}
int doubleCommand(double *value,CommandPtr cp)
{
	if(!cp || !value)return 1;
	if(cp->n >= cp->nword)return 1;
	if(cp->type[cp->n] != BATCH_DOUBLE)return 1;
	*value=cp->value[cp->n];
	return 0;
}

char *stringCommand(CommandPtr cp)
{
	if(!cp)return NULL;
	if(cp->n >= cp->nword)return NULL;
	if(cp->type[cp->n] != BATCH_STRING)return NULL;
	return cp->command[cp->n];
}
