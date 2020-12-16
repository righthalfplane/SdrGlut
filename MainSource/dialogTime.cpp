//
//  dialogTime.cpp
//  FireBall
//
//  Created by Dale Ranta on 1/16/16.
//
//

#include <stdio.h>

#include <time.h>       /* time_t, struct tm, difftime, time, mktime */


#include "firstFile.h"
#include <cstdio>
#include <cstdlib>


//#include "CWindowRangeDialog.h"

#include "LoadFiles.h"
#include "Utilities.h"
#include "SceneList.h"

#include <GL/glui.h>

static GLUI *glui;

static void control_cb(int control);


static char text1[255]="1";
static char text2[255]="2";
static char text3[255]="2";
static char text4[255]="2";
static char text5[255]="2";

static GLUI_EditText *edittext1;
static GLUI_EditText *edittext2;
static GLUI_EditText *edittext3;
static GLUI_EditText *edittext4;
static GLUI_EditText *edittext5;


static char text1a[255]="1";
static char text2a[255]="2";
static char text3a[255]="2";
static char text4a[255]="2";
static char text5a[255]="2";

static GLUI_EditText *edittext1a;
static GLUI_EditText *edittext2a;
static GLUI_EditText *edittext3a;
static GLUI_EditText *edittext4a;
static GLUI_EditText *edittext5a;


static char tex1[255]="1";
static char tex2[255]="2";
static char tex3[255]="2";
static char tex4[255]="2";
static char tex5[255]="2";

static GLUI_EditText *edittex1;
static GLUI_EditText *edittex2;
static GLUI_EditText *edittex3;
static GLUI_EditText *edittex4;
static GLUI_EditText *edittex5;


static char tex1a[255]="1";
static char tex2a[255]="2";
static char tex3a[255]="2";
static char tex4a[255]="2";
static char tex5a[255]="2";

static GLUI_EditText *edittex1a;
static GLUI_EditText *edittex2a;
static GLUI_EditText *edittex3a;
static GLUI_EditText *edittex4a;
static GLUI_EditText *edittex5a;

struct tm dvr  =  {0,58,7,14,9,2015-1900,0,0,1};
struct tm real =  {0,13,7,20,0,2016-1900,0,0,0};

static char sec[255]="0";
static GLUI_EditText *editsec;


struct tm today;
struct tm offset =  {0,58,7,14,9,2015-1900};


double secondsOffSet=0;


int dialogTime()
{
    time_t now;
    
    
    if(glui){
        glui->close();
    }
    
    
    time(&now);  /* get current time; same as: now = time(NULL)  */
    
    today = *localtime(&now);

    glui = GLUI_Master.create_glui( "Time Dialog" );
    
    GLUI_Panel *obj_panel =  glui->add_panel( "month" );
    
    msprintf(text2,sizeof(text2),"%d",dvr.tm_mon+1);
    edittext2 =
    glui->add_edittext_to_panel(obj_panel, "Old:", GLUI_EDITTEXT_TEXT, text2 );
    edittext2->w=80;
    
    msprintf(text2a,sizeof(text2a),"%d",real.tm_mon+1);
    edittext2a =
    glui->add_edittext_to_panel(obj_panel, "New:", GLUI_EDITTEXT_TEXT, text2a );
    edittext2a->w=80;
    
    msprintf(tex2,sizeof(tex2),"%d",today.tm_mon+1);
    edittex2 =
    glui->add_edittext_to_panel(obj_panel, "Today:", GLUI_EDITTEXT_TEXT, tex2 );
    edittex2->w=80;
    
    msprintf(tex2a,sizeof(tex2a),"%d",offset.tm_mon+1);
    edittex2a =
    glui->add_edittext_to_panel(obj_panel, "Calc:", GLUI_EDITTEXT_TEXT, tex2a );
    edittex2a->w=80;
    
    glui->add_column(true);
    
    obj_panel =  glui->add_panel( "day" );

    msprintf(text1,sizeof(text1),"%d",dvr.tm_mday);
    edittext1 =
    glui->add_edittext_to_panel(obj_panel, "Old:", GLUI_EDITTEXT_TEXT, text1 );
    edittext1->w=80;
    
    
    msprintf(text1a,sizeof(text1a),"%d",real.tm_mday);
    edittext1a =
    glui->add_edittext_to_panel(obj_panel, "New:", GLUI_EDITTEXT_TEXT, text1a );
    edittext1a->w=80;
    
    
    
    msprintf(tex1,sizeof(tex1),"%d",today.tm_mday);
    edittex1 =
    glui->add_edittext_to_panel(obj_panel, "Today:", GLUI_EDITTEXT_TEXT, tex1 );
    edittex1->w=80;
    
    
    msprintf(tex1a,sizeof(tex1a),"%d",offset.tm_mday);
    edittex1a =
    glui->add_edittext_to_panel(obj_panel, "Calc:", GLUI_EDITTEXT_TEXT, tex1a );
    edittex1a->w=80;
    
    
    
    glui->add_column(true);
    
    
    
    
    obj_panel =  glui->add_panel( "year" );
    
    
    msprintf(text3,sizeof(text3),"%d",dvr.tm_year+1900);
    edittext3 =
    glui->add_edittext_to_panel(obj_panel, "Old:", GLUI_EDITTEXT_TEXT, text3 );
    edittext3->w=80;
    
    msprintf(text3a,sizeof(text3a),"%d",real.tm_year+1900);
    edittext3a =
    glui->add_edittext_to_panel(obj_panel, "New:", GLUI_EDITTEXT_TEXT, text3a );
    edittext3a->w=80;
    
    
    msprintf(tex3,sizeof(tex3),"%d",today.tm_year+1900);
    edittex3 =
    glui->add_edittext_to_panel(obj_panel, "Today:", GLUI_EDITTEXT_TEXT, tex3 );
    edittex3->w=80;
    
    msprintf(tex3a,sizeof(tex3a),"%d",offset.tm_year+1900);
    edittex3a =
    glui->add_edittext_to_panel(obj_panel, "Calc:", GLUI_EDITTEXT_TEXT, tex3a );
    edittex3a->w=80;
    
    
    glui->add_column(true);
    
    obj_panel =  glui->add_panel( "hour" );
    
    msprintf(text4,sizeof(text4),"%d",dvr.tm_hour);
    edittext4 =
    glui->add_edittext_to_panel(obj_panel, "Old:", GLUI_EDITTEXT_TEXT, text4);
    edittext4->w=80;
    
    msprintf(text4a,sizeof(text4a),"%d",real.tm_hour);
    edittext4a =
    glui->add_edittext_to_panel(obj_panel, "New:", GLUI_EDITTEXT_TEXT, text4a );
    edittext4a->w=80;
    
    
    msprintf(tex4,sizeof(text4),"%d",today.tm_hour);
    edittex4 =
    glui->add_edittext_to_panel(obj_panel, "Today:", GLUI_EDITTEXT_TEXT, tex4);
    edittex4->w=80;
    
    msprintf(tex4a,sizeof(tex4a),"%d",offset.tm_hour);
    edittex4a =
    glui->add_edittext_to_panel(obj_panel, "Calc:", GLUI_EDITTEXT_TEXT, tex4a );
    edittex4a->w=80;
    
    
    

    
    glui->add_column(true);
    
    obj_panel =  glui->add_panel( "minute" );
    
    msprintf(text5,sizeof(text5),"%d",dvr.tm_min);
    edittext5 =
    glui->add_edittext_to_panel(obj_panel, "Old:", GLUI_EDITTEXT_TEXT, text5);
    edittext5->w=80;
    
    msprintf(text5a,sizeof(text5a),"%d",real.tm_min);
    edittext5a =
    glui->add_edittext_to_panel(obj_panel, "New:", GLUI_EDITTEXT_TEXT, text5a );
    edittext5a->w=80;
    
    msprintf(tex5,sizeof(tex5),"%d",today.tm_min);
    edittex5 =
    glui->add_edittext_to_panel(obj_panel, "Today:", GLUI_EDITTEXT_TEXT, tex5);
    edittex5->w=80;
    
    msprintf(tex5a,sizeof(tex5a),"%d",offset.tm_min);
    edittex5a =
    glui->add_edittext_to_panel(obj_panel, "Calc:", GLUI_EDITTEXT_TEXT, tex5a );
    edittex5a->w=80;
    
    glui->add_column(true);
    
    msprintf(sec,sizeof(sec),"%ld",(long)secondsOffSet);
    editsec =
    glui->add_edittext( "Offset:", GLUI_EDITTEXT_TEXT, sec );
    editsec->w=160;
    
    
    new GLUI_Button(glui, "Refresh", 1, control_cb);
    
    new GLUI_Button(glui, "Calculate", 5, control_cb);
    
    new GLUI_Button(glui, "Offset", 4, control_cb);
    
    new GLUI_Button(glui, "Close", 2, control_cb);
    
    // glui->set_main_gfx_window( glutGetWindow() );
    
    return 0;
}
static int getValues()
{
    
    sscanf(edittext1->get_text(),"%d", &dvr.tm_mday);
    sscanf(edittext2->get_text(),"%d", &dvr.tm_mon);
    dvr.tm_mon -= 1;
    sscanf(edittext3->get_text(),"%d", &dvr.tm_year);
    dvr.tm_year -= 1900;
    sscanf(edittext4->get_text(),"%d", &dvr.tm_hour);
    sscanf(edittext5->get_text(),"%d", &dvr.tm_min);
    
    sscanf(edittex1->get_text(),"%d", &today.tm_mday);
    sscanf(edittex2->get_text(),"%d", &today.tm_mon);
    today.tm_mon -= 1;
    sscanf(edittex3->get_text(),"%d", &today.tm_year);
    today.tm_year -= 1900;
    sscanf(edittex4->get_text(),"%d", &today.tm_hour);
    sscanf(edittex5->get_text(),"%d", &today.tm_min);
    
    sscanf(edittext1a->get_text(),"%d", &real.tm_mday);
    sscanf(edittext2a->get_text(),"%d", &real.tm_mon);
    real.tm_mon -= 1;
    sscanf(edittext3a->get_text(),"%d", &real.tm_year);
    real.tm_year -= 1900;
    sscanf(edittext4a->get_text(),"%d", &real.tm_hour);
    sscanf(edittext5a->get_text(),"%d", &real.tm_min);
    
    sscanf(editsec->get_text(),"%lg", &secondsOffSet);
    
    return 0;
}

static void control_cb(int control)
{
    std::string file_name;
    time_t now;
    
    
    if(control == 1)
    {
        time(&now);  /* get current time; same as: now = time(NULL)  */
        
        today = *localtime(&now);
        
        msprintf(tex1,sizeof(tex1),"%d",today.tm_mday);
        edittex1->set_text(tex1);
        
        msprintf(tex2,sizeof(tex2),"%d",today.tm_mon+1);
        edittex2->set_text(tex2);
        
        msprintf(tex3,sizeof(tex3),"%d",today.tm_year+1900);
        edittex3->set_text(tex3);
        
        msprintf(tex4,sizeof(tex4),"%d",today.tm_hour);
        edittex4->set_text(tex4);
       
        msprintf(tex5,sizeof(tex5),"%d",today.tm_min);
        edittex5->set_text(tex5);
       

        
    }
    else if(control == 4)
    {
        getValues();
        
        /*
        printf("tm_sec %d ",dvr.tm_sec);
        printf("tm_min %d ",dvr.tm_min);
        printf("tm_hour %d ",dvr.tm_hour);
        printf("tm_mday %d ",dvr.tm_mday);
        printf("tm_mon %d ",dvr.tm_mon);
        printf("tm_year %d ",dvr.tm_year+1900);
        printf("tm_zone %s\n",dvr.tm_zone);
        
        
        printf("tm_sec %d ",real.tm_sec);
        printf("tm_min %d ",real.tm_min);
        printf("tm_hour %d ",real.tm_hour);
        printf("tm_mday %d ",real.tm_mday);
        printf("tm_mon %d ",real.tm_mon);
        printf("tm_year %d ",real.tm_year+1900);
        printf("tm_zone %s\n",real.tm_zone);
        */
        
        secondsOffSet = difftime(mktime(&real),mktime(&dvr));
        
       // printf("%ld\n",(long)secondsOffSet);
        
        msprintf(sec,sizeof(sec),"%ld",(long)secondsOffSet);
        editsec->set_text(sec);
        
    }
    else if(control == 2)
    {
        glui->close();
        glui=NULL;
    }
    else if(control == 5)
    {
        
        time_t tdvr;
        
        getValues();
                
        tdvr=mktime(&today);
        
        tdvr=tdvr-(long)secondsOffSet;
        
        offset = *localtime(&tdvr);
       
        msprintf(tex1a,sizeof(tex1a),"%d",offset.tm_mday);
        edittex1a->set_text(tex1a);
        
        msprintf(tex2a,sizeof(tex2a),"%d",offset.tm_mon+1);
        edittex2a->set_text(tex2a);
        
        msprintf(tex3a,sizeof(tex3a),"%d",offset.tm_year+1900);
        edittex3a->set_text(tex3a);
        
        msprintf(tex4a,sizeof(tex4a),"%d",offset.tm_hour);
        edittex4a->set_text(tex4a);
        
        msprintf(tex5a,sizeof(tex5a),"%d",offset.tm_min);
        edittex5a->set_text(tex5a);
        
        
    }
    glutPostRedisplay();
}
