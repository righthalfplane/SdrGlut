/*
 *  WriteToGLUIWindow.cpp
 *  FireBall
 *
 *  Created by Dale Ranta on 8/28/12.
 *  Copyright 2012. All rights reserved.
 *
 */

#include "firstFile.h"
#include "DialogSave.h"
#include "mThread.h"
#include <cstdio>
#include <cstdlib>
#include <stdarg.h>
#include "Radio.h"
/* #define EXTERN  */

#include "BackgroundQ.h"
#include "LoadFiles.h"
#include "CWindow.h"

#include <GL/glui.h>

extern "C" int WriteToGLUIWindow(char *message);


static void textbox_cb(GLUI_Control *control) {
    //printf("Got textbox callback\n");
}

static void menu_select(int item);


static int rxScan(GLUI_TextBox3 *moo);

static std::vector<std::string> modes;
static std::vector<std::string> freq;

extern int getFrequencyData(char **list);

int GLUI_TextBox3::key_handler(unsigned char key, int modifiers)
{
  //  fprintf(stderr,"key %d\n",key);
    
    int win1=glutGetWindow();
    RadioPtr s=(RadioPtr)FindSceneRadio(win1);
    if(!s){
        fprintf(stderr,"menu_select RadioPtr NOT Found win1 %d\n",win1);
        return 1;
    }
    
    GLUI_TextBox3 *moo=s->moo;

    const char *test=moo->get_text();

    if(key == 6){
        
        menu_select(31);
        
        int ns=moo->sel_end+1;
        
        if(ns >= (int)(strlen(test)-1)){
            moo->update_and_draw_text();
            return false;
        }

        int end=0;
        for(int n=moo->sel_end+1;n<(int)strlen(test);++n){
            if(test[n] == '\n'){
                end=n;
                break;
            }
        }
        
        fprintf(stderr,"ns %d end %d\n",ns,end);
        
        if(end-ns > 4){
            moo->sel_end=end;
            moo->sel_start=ns;
        }

        moo->insertion_pt=moo->sel_end+1;
        moo->update_and_draw_text();

        return false;
    }else if(key == 2){

        int ns=moo->sel_start-2;
        
        if(ns < 0){
            ns=moo->sel_start=0;
            moo->sel_end=(int)strlen(test);
            for(int n=1;n<(int)strlen(test);++n){
                if(test[n] == '\n'){
                    moo->sel_end=n;
                    break;
                }
            }

        }else{
            int end=moo->sel_start-1;
            
            int start=0;
            
            for(int n=ns;n>0;--n){
                if(test[n] == '\n'){
                    start=n+1;
                    break;
                }
            }
            
           // fprintf(stderr,"start %d end %d\n",start,end);
            
            if(end-start > 4){
                moo->sel_start=start;
                moo->sel_end=end;
            }
            
        }
    
        menu_select(31);
        
        moo->update_and_draw_text();

        return false;
    }
    GLUI_TextBox::key_handler( key, modifiers);

    return false;

}
int GLUI_TextBox3::special_handler( int key, int modifiers)
{
//     fprintf(stderr,"key %d\n",key);
    
    if(key == 114)return false;
    
//    int ptsave=moo->insertion_pt;
    
    GLUI_TextBox::special_handler( key, modifiers);
    
    return false;
}
int GLUI_TextBox3::mouse_down_handler( int local_x, int local_y)
{
    GLUI_TextBox::mouse_down_handler( local_x, local_y);
    
   // fprintf(stderr,"a mouse_down_handler insertion_pt %d\n",insertion_pt);
    
    int win1=glutGetWindow();
    RadioPtr s=(RadioPtr)FindSceneRadio(win1);
    if(!s){
        fprintf(stderr,"menu_select RadioPtr NOT Found win1 %d\n",win1);
        return 1;
    }
    

    s->insert=insertion_pt;

    return false;
    
}

GLUI_TextBox3::GLUI_TextBox3( GLUI_Node *parent,
                             bool scroll, int id,
                             GLUI_CB cb) : GLUI_TextBox(parent,scroll,id,cb)
{
    ;
}


int Radio::doFrequencyFile(char *path)
{
    char word[20000];
    int k,itWas,c;
    char *buff;
    long length;
    
    if(getFrequencyData(&buff))return 1;

    length=(long)strlen(buff);
    
    fprintf(stderr,"File length %ld\n",length);
    
    itWas = -7777;
    k=0;
        for(long n=0;n<length;++n){
            c=buff[n];
            if(c == '\n' || c == '\r' || (k >= 19998)){
                if((c == '\n') && (itWas == '\r')){
                    continue;
                }
                word[k++]='\n';
                word[k++]='\0';
                WriteToWindow(word);
                k=0;
            }else{
                word[k++]=buff[n];
            }
            itWas = c;
        }
    
    if(k != 0){
        word[k++]='\n';
        word[k++]='\0';
        WriteToWindow(word);
    }
    return 0;
}
static void SaveIt(struct Scene *scene,char *name)
{
	FILE *out;
    
    int win1=glutGetWindow();
    RadioPtr s=(RadioPtr)FindSceneRadio(win1);
    if(!s){
        fprintf(stderr,"SaveIt RadioPtr NOT Found win1 %d\n",win1);
        return;
    }

		if(s->moo){
			const char *text=s->moo->get_text();
			if(text){
				out=fopen(name,"w");
				if(out){
					fprintf(out,"%s",text);
					fclose(out);
				}
			}
		}

}

static int rxScan(GLUI_TextBox3 *moo)
{
     int n;
    
    int win1=glutGetWindow();
    RadioPtr s=(RadioPtr)FindSceneRadio(win1);
    if(!s){
        fprintf(stderr,"rxScan RadioPtr NOT Found win1 %d\n",win1);
        return 1;
    }

    const char *test=s->moo->get_text();
    
    char *buff = new char[strlen(test)+10];
    
    int start=s->moo->sel_start;
    int end=s->moo->sel_end;
   // fprintf(stderr,"Start Scan start %d end %d\n",start,end);
    if(start > end){
        n=start;
        start=end;
        end=n;
    }
    n=0;
    for(int k=start;k<end;++k){
        if(test[k] == ',' || test[k] == 10){
            buff[n++]=0;
        }else{
            buff[n++]=test[k];
        }
    }
    buff[n++]=0;

    s->scanFrequencies.clear();
    s->scanWait=1;
    
    int n1=-1;
    int n2=-1;
    for(int k=0;k<n;++k){
      if(n1 == -1 && buff[k] == 0){
        n1=k+1;
      }else if(n1 > -1 && n2 == -1 && buff[k] == 0){
          n2=k+1;
         // fprintf(stderr,"buff1 %s buff2 %s\n",&buff[n1],&buff[n2]);
          // sendMessageGlobal(&buff[n1],&buff[n2],M_FREQUENCY_SCAN);
          s->scanFrequencies.push_back(1.0e6*atof(&buff[n1]));
          for(int i=n2;i<n;++i){
              if(buff[i] == 0){
                  k=i;
                  break;
              }
          }
          n1=-1;
          n2=-1;
      }
    }
    
    s->scanWait=0;
    s->pauseChannel=0;
    
    return 1;
}
static void menu_select(int item)
{
    static char buff[4096];
	GLUI *glui;
    int n;
    
    int win1=glutGetWindow();
    RadioPtr s=(RadioPtr)FindSceneRadio(win1);
    if(!s){
        fprintf(stderr,"menu_select RadioPtr NOT Found win1 %d\n",win1);
        GLUI *gluiw;
        gluiw = GLUI_Master.find_glui_by_window_id(win1);
        if(gluiw){
            gluiw->close();
        }
        
        s->moo=NULL;
        
        s->gluiID = -1;

        return;
    }

    
    //fprintf(stderr,"menu_select window %d\n",glutGetWindow());

    if(item == 400){
        rxScan(s->moo);
   // }else if(item == 401){
   //     scanFlag=0;
    }else if(item == 405){
       // rxScan2((void *)moo);
    }else if(item == 32){
		dialogSaveC(NULL,SaveIt,3,NULL);
		return;
    }else if(item == 35){
        const char *test=s->moo->get_text();
        int start=s->moo->sel_start;
        int end=s->moo->sel_end;
        if(start > end){
            n=start;
            start=end;
            end=n;
        }
        n=0;
        for(int k=start;k<end;++k){
            buff[n++]=test[k];
            if(n > 4094)break;
        }
        buff[n++]=0;
    }else if(item == 36){
        s->moo->text.insert(s->moo->insertion_pt,buff);
        s->moo->insertion_pt += (int)strlen(buff);
      //  fprintf(stderr,"point %d buff %s\n",moo->insertion_pt,buff);
    }else if(item == 33){
        glui = GLUI_Master.find_glui_by_window_id(s->gluiID);

		if(glui){
			glui->close();
		}
		
		s->moo=NULL;
		
		s->gluiID = -1;
    }else if(item == 31){
        int n;
        
        const char *test=s->moo->get_text();
        
        int start=s->moo->sel_start;
        int end=s->moo->sel_end;
        if(start > end){
            n=start;
            start=end;
            end=n;
        }
        n=0;
        for(int k=start;k<end;++k){
            buff[n++]=test[k];
            if(n > 254)break;
        }
        buff[n++]=0;
        int n1=-1;
        int n2=-1;
        for(int k=0;k<n;++k){
            if(n1 == -1 && buff[k] == ','){
                n1=k+1;
            }else if(n1 > -1 && n2 == -1 && buff[k] == ','){
                buff[k]=0;
                n2=k+1;
            } else if(buff[k] == ','){
                buff[k]=0;
            }
          
        }
        
        if(n1 == -1 || n2 == -1)return;

       // fprintf(stderr,"%d %d buff %s\n",moo->sel_start,moo->sel_end,buff);
        
        s->sendMessage(&buff[n1],&buff[n2],M_SEND);
        
    }
}

int Radio::WriteToWindow(char *message)
{
	GLUI *gluiw;

    static int window=-1;
    
	if(!message)return 1;
	
	gluiw = GLUI_Master.find_glui_by_window_id(gluiID);
	
	if(!gluiw){
        gluiw = GLUI_Master.create_glui(rx->driveName, 0);
        if(!gluiw)return 1;
        gluiID=gluiw->get_glut_window_id();
        gluiw->set_main_gfx_window(glutGetWindow());
        GLUI_Panel *ep = new GLUI_Panel(gluiw,"",true);
        moo = new GLUI_TextBox3(ep,true,1,textbox_cb);
        moo->set_h(400);
        moo->set_w(610);
        
        //GLUI_Panel *panel3 = new GLUI_Panel(glui, "Scan Frequencies");
        //new GLUI_Button(panel3, "Stop", 401, menu_select);

        window=glutGetWindow();
        
        glutSetWindow(gluiID);

		glutCreateMenu(menu_select);
		
        glutAddMenuEntry("Set Frequency", 31);
        glutAddMenuEntry("Send Scan Frequencies", 400);
//        glutAddMenuEntry("Scan Frequencies", 405);
//        glutAddMenuEntry("Stop Scan", 401);
        glutAddMenuEntry("-------------", 34);
        glutAddMenuEntry("Copy", 35);
        glutAddMenuEntry("Paste", 36);
        glutAddMenuEntry("Save", 32);
        glutAddMenuEntry("-------------", 34);
        glutAddMenuEntry("Close", 33);

		glutAttachMenu(GLUT_RIGHT_BUTTON);
        
        mooWindow=glutGetWindow();
        
        fprintf(stderr,"gluiID %d dd.sub_window %d\n",gluiID,dd.sub_window);

        gluiw->set_main_gfx_window(dd.sub_window );

		glutSetWindow(window);
        
        moo->insertion_pt = -2;
        
	}
        if(moo){
            if(moo->insertion_pt == -2){
                moo->insertion_pt = -1;
                moo->text=message;
                insert=(int)strlen(message);
                moo->insertion_pt += (int)strlen(message)+1;
            }else{
                if(moo->insertion_pt == -1){
                    moo->insertion_pt=insert;
               }
                moo->text.insert(moo->insertion_pt,message);
                moo->insertion_pt += (int)strlen(message);
                insert=moo->insertion_pt;
                moo->redraw_window();
            }
        }
  
	return 0;
}

