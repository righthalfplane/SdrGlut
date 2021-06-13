//
//  dialogTime.cpp
//  FireBall
//
//  Created by Dale Ranta on 1/16/16.
//
//

#include <stdio.h>

#include "Radio.h"

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


static GLUI_EditText *edit[26];

static char tex[26][255];


static struct tm today;
static struct tm record1;
static struct tm record2;
static struct tm record3;

static time_t record1time;
static time_t record2time;
static time_t record3time;

static RadioPtr sdr;

static int nlast;

static GLUI_EditText *filePath[3];
static GLUI_EditText *mode[3];
static GLUI_EditText *frequency[3];

static char freq[3][255];

static int on[3];

int Radio::dialogTime()
{
    time_t now;
    
    
    if(glui){
        glui->close();
    }
    
    sdr=this;
    
    record1time=rs.stop[0]/60;
    record2time=rs.stop[1]/60;
    record3time=rs.stop[2]/60;

    on[0]=rs.on[0];
    on[1]=rs.on[1];
    on[2]=rs.on[2];

    time(&now);  /* get current time; same as: now = time(NULL)  */
    
    today = *localtime(&now);
    
    record1 = *localtime(&rs.start[0]);
    record2 = *localtime(&rs.start[1]);
    record3 = *localtime(&rs.start[2]);

/*
    printf("rs.start[0] %ld\n",rs.start[0]);
    printf("tm_sec %d ",record1.tm_sec);
    printf("tm_min %d ",record1.tm_min);
    printf("tm_hour %d ",record1.tm_hour);
    printf("tm_mday %d ",record1.tm_mday);
    printf("tm_mon %d ",record1.tm_mon);
    printf("tm_year %d ",record1.tm_year);
    printf("tm_zone %s\n",record1.tm_zone);
    printf("mktime[0] %ld\n",mktime(&record1));

*/
    
    
    glui = GLUI_Master.create_glui( "Time Dialog" );
    
    GLUI_Panel *obj_panel =  glui->add_panel( "month" );
    
    msprintf(tex[0],sizeof(tex[0]),"%d",today.tm_mon+1);
    edit[0] =
    glui->add_edittext_to_panel(obj_panel, "Today:", GLUI_EDITTEXT_TEXT, tex[0] );
    edit[0]->w=80;
    
    msprintf(tex[1],sizeof(tex[1]),"%d",record1.tm_mon+1);
    edit[1] =
    glui->add_edittext_to_panel(obj_panel, "Record1:", GLUI_EDITTEXT_TEXT, tex[1] );
    edit[1]->w=80;
    
    msprintf(tex[2],sizeof(tex[2]),"%d",record2.tm_mon+1);
    edit[2] =
    glui->add_edittext_to_panel(obj_panel, "Record2:", GLUI_EDITTEXT_TEXT, tex[2] );
    edit[2]->w=80;
    
    msprintf(tex[19],sizeof(tex[19]),"%d",record3.tm_mon+1);
    edit[19] =
    glui->add_edittext_to_panel(obj_panel, "Record3:", GLUI_EDITTEXT_TEXT, tex[19] );
    edit[19]->w=80;
    

    glui->add_column(true);
    
    obj_panel =  glui->add_panel( "day" );
    
    
    msprintf(tex[3],sizeof(tex[3]),"%d",today.tm_mday);
    edit[3] =
    glui->add_edittext_to_panel(obj_panel, "Today:", GLUI_EDITTEXT_TEXT, tex[3] );
    edit[3]->w=80;
    
    msprintf(tex[4],sizeof(tex[4]),"%d",record1.tm_mday);
    edit[4] =
    glui->add_edittext_to_panel(obj_panel, "Record1:", GLUI_EDITTEXT_TEXT, tex[4] );
    edit[4]->w=80;
    
    msprintf(tex[5],sizeof(tex[5]),"%d",record2.tm_mday);
    edit[5] =
    glui->add_edittext_to_panel(obj_panel, "Record2:", GLUI_EDITTEXT_TEXT, tex[5] );
    edit[5]->w=80;
    
    msprintf(tex[20],sizeof(tex[20]),"%d",record2.tm_mday);
    edit[20] =
    glui->add_edittext_to_panel(obj_panel, "Record3:", GLUI_EDITTEXT_TEXT, tex[20] );
    edit[20]->w=80;
    

    glui->add_column(true);
    
    obj_panel =  glui->add_panel( "year" );
    
    
    msprintf(tex[6],sizeof(tex[6]),"%d",today.tm_year+1900);
    edit[6] =
    glui->add_edittext_to_panel(obj_panel, "Today:", GLUI_EDITTEXT_TEXT, tex[6] );
    edit[6]->w=80;
    
    msprintf(tex[7],sizeof(tex[7]),"%d",record1.tm_year+1900);
    edit[7] =
    glui->add_edittext_to_panel(obj_panel, "Record1:", GLUI_EDITTEXT_TEXT, tex[7] );
    edit[7]->w=80;
    
    msprintf(tex[8],sizeof(tex[8]),"%d",record2.tm_year+1900);
    edit[8] =
    glui->add_edittext_to_panel(obj_panel, "Record2:", GLUI_EDITTEXT_TEXT, tex[8] );
    edit[8]->w=80;
    
    
    msprintf(tex[21],sizeof(tex[21]),"%d",record3.tm_year+1900);
    edit[21] =
    glui->add_edittext_to_panel(obj_panel, "Record3:", GLUI_EDITTEXT_TEXT, tex[21] );
    edit[21]->w=80;
    
    
    glui->add_column(true);
    
    obj_panel =  glui->add_panel( "hour" );
    
    
    msprintf(tex[9],sizeof(tex[9]),"%d",today.tm_hour);
    edit[9] =
    glui->add_edittext_to_panel(obj_panel, "Today:", GLUI_EDITTEXT_TEXT, tex[9]);
    edit[9]->w=80;
    
    msprintf(tex[10],sizeof(tex[10]),"%d",record1.tm_hour);
    edit[10] =
    glui->add_edittext_to_panel(obj_panel, "Record1:", GLUI_EDITTEXT_TEXT, tex[10]);
    edit[10]->w=80;
    
    
    msprintf(tex[11],sizeof(tex[11]),"%d",record2.tm_hour);
    edit[11] =
    glui->add_edittext_to_panel(obj_panel, "Record2:", GLUI_EDITTEXT_TEXT, tex[11]);
    edit[11]->w=80;
    
    msprintf(tex[22],sizeof(tex[22]),"%d",record3.tm_hour);
    edit[22] =
    glui->add_edittext_to_panel(obj_panel, "Record3:", GLUI_EDITTEXT_TEXT, tex[22]);
    edit[22]->w=80;
    
    glui->add_column(true);
    
    obj_panel =  glui->add_panel( "minute" );
    
    msprintf(tex[12],sizeof(tex[12]),"%d",today.tm_min);
    edit[12] =
    glui->add_edittext_to_panel(obj_panel, "Today:", GLUI_EDITTEXT_TEXT, tex[12]);
    edit[12]->w=80;
    
    msprintf(tex[13],sizeof(tex[13]),"%d",record1.tm_min);
    edit[13] =
    glui->add_edittext_to_panel(obj_panel, "Record1:", GLUI_EDITTEXT_TEXT, tex[13]);
    edit[13]->w=80;
    
    msprintf(tex[14],sizeof(tex[14]),"%d",record2.tm_min);
    edit[14] =
    glui->add_edittext_to_panel(obj_panel, "Record2:", GLUI_EDITTEXT_TEXT, tex[14]);
    edit[14]->w=80;
    
    msprintf(tex[23],sizeof(tex[23]),"%d",record3.tm_min);
    edit[23] =
    glui->add_edittext_to_panel(obj_panel, "Record3:", GLUI_EDITTEXT_TEXT, tex[23]);
    edit[23]->w=80;
    
    glui->add_column(true);
    
    obj_panel =  glui->add_panel( "minutes to record" );
    
    msprintf(tex[15],sizeof(tex[15]),"----",-1);
    edit[15] =
    glui->add_edittext_to_panel(obj_panel, "Today:", GLUI_EDITTEXT_TEXT, tex[15]);
    edit[15]->w=80;

    msprintf(tex[16],sizeof(tex[16]),"%ld",record1time);
    edit[16] =
    glui->add_edittext_to_panel(obj_panel, "Record1:", GLUI_EDITTEXT_TEXT, tex[16]);
    edit[16]->w=80;
    
    msprintf(tex[17],sizeof(tex[17]),"%ld",record2time);
    edit[17] =
    glui->add_edittext_to_panel(obj_panel, "Record2:", GLUI_EDITTEXT_TEXT, tex[17]);
    edit[17]->w=80;
    
    msprintf(tex[18],sizeof(tex[18]),"%ld",record3time);
    edit[18] =
    glui->add_edittext_to_panel(obj_panel, "Record3:", GLUI_EDITTEXT_TEXT, tex[18]);
    edit[18]->w=80;
    
    glui->add_column(true);
    
    obj_panel =  glui->add_panel( "Record Information" );

    GLUI_Panel *panel3 = new GLUI_Panel(obj_panel, "Record1");
    
    new GLUI_Checkbox(panel3, "Record ", &on[0], 0, control_cb );

    filePath[0] =
    glui->add_edittext_to_panel(panel3, "Path:", GLUI_EDITTEXT_TEXT, rs.FilePath[0]);
    filePath[0]->w=200;
    
    
    new GLUI_Button(panel3, "Set Record File", 100, control_cb);
    
    if(rs.frequency[0] == 0){
        char *Mode_Names[] = {(char *)"FM",(char *)"NBFM",(char *)"AM",(char *)"NAM",(char *)"USB",(char *)"LSB",(char *)"CW"};
        rs.frequency[0]=rx->f;
        mstrncpy(rs.mode[0],Mode_Names[rx->decodemode],sizeof(rs.mode[0]));
    }
    
    msprintf(freq[0],sizeof(freq[0]),"%ld",(long)rs.frequency[0]);
    
    frequency[0] =
    glui->add_edittext_to_panel(panel3, "Frequency:", GLUI_EDITTEXT_TEXT, freq[0]);
    frequency[0]->w=200;
    
    mode[0] =
    glui->add_edittext_to_panel(panel3, "Mode:", GLUI_EDITTEXT_TEXT, rs.mode[0]);
    mode[0]->w=200;
    

    panel3 = new GLUI_Panel(obj_panel, "Record2");
    
    new GLUI_Checkbox(panel3, "Record ", &on[1], 0, control_cb );
    
    filePath[1] =
    glui->add_edittext_to_panel(panel3, "Path:", GLUI_EDITTEXT_TEXT, rs.FilePath[1]);
    filePath[1]->w=200;
    
    
    new GLUI_Button(panel3, "Set Record File", 101, control_cb);
    
    msprintf(freq[1],sizeof(freq[0]),"%ld",(long)rs.frequency[1]);
    
    frequency[1] =
    glui->add_edittext_to_panel(panel3, "Frequency:", GLUI_EDITTEXT_TEXT, freq[1]);
    frequency[1]->w=200;
    
    mode[1] =
    glui->add_edittext_to_panel(panel3, "Mode:", GLUI_EDITTEXT_TEXT, rs.mode[1]);
    mode[1]->w=200;
    
    
    panel3 = new GLUI_Panel(obj_panel, "Record3");
    
    new GLUI_Checkbox(panel3, "Record ", &on[2], 0, control_cb );
    
    filePath[2] =
    glui->add_edittext_to_panel(panel3, "Path:", GLUI_EDITTEXT_TEXT, rs.FilePath[2]);
    filePath[2]->w=200;
    
    
    new GLUI_Button(panel3, "Set Record File", 102, control_cb);
    
    msprintf(freq[2],sizeof(freq[0]),"%ld",(long)rs.frequency[2]);
    
    frequency[2] =
    glui->add_edittext_to_panel(panel3, "Frequency:", GLUI_EDITTEXT_TEXT, freq[2]);
    frequency[2]->w=200;
    
    mode[2] =
    glui->add_edittext_to_panel(panel3, "Mode:", GLUI_EDITTEXT_TEXT, rs.mode[2]);
    mode[2]->w=200;
    
    
    glui->add_column(true);


    new GLUI_Button(glui, "Refresh Now", 1, control_cb);
    
    new GLUI_Button(glui, "Start Recording", 4, control_cb);
    
    new GLUI_Button(glui, "Close", 2, control_cb);
    
    // glui->set_main_gfx_window( glutGetWindow() );
    
    return 0;
}
static int getValues()
{
    
  //  printf("%s %s %s %s %s\n",edit[0]->get_text(),edit[3]->get_text(),edit[6]->get_text(),edit[9]->get_text(),edit[12]->get_text());
    
  //  printf("%s %s %s %s %s\n",edit[1]->get_text(),edit[4]->get_text(),edit[7]->get_text(),edit[10]->get_text(),edit[13]->get_text());
    
  //  printf("%s %s %s %s %s\n",edit[2]->get_text(),edit[5]->get_text(),edit[8]->get_text(),edit[11]->get_text(),edit[14]->get_text());

    
    sscanf(edit[0]->get_text(),"%d", &today.tm_mon);
    sscanf(edit[3]->get_text(),"%d", &today.tm_mday);
    today.tm_mon -= 1;
    sscanf(edit[6]->get_text(),"%d", &today.tm_year);
    today.tm_year -= 1900;
    sscanf(edit[9]->get_text(),"%d", &today.tm_hour);
    sscanf(edit[12]->get_text(),"%d", &today.tm_min);
    
    record1.tm_sec=0;
    record2.tm_sec=0;
    record3.tm_sec=0;
    
    sscanf(edit[1]->get_text(),"%d", &record1.tm_mon);
    sscanf(edit[4]->get_text(),"%d", &record1.tm_mday);
    record1.tm_mon = record1.tm_mon-1;
    sscanf(edit[7]->get_text(),"%d", &record1.tm_year);
    record1.tm_year = record1.tm_year-1900;
    sscanf(edit[10]->get_text(),"%d", &record1.tm_hour);
    sscanf(edit[13]->get_text(),"%d", &record1.tm_min);
    
    sscanf(edit[2]->get_text(),"%d", &record2.tm_mon);
    sscanf(edit[5]->get_text(),"%d", &record2.tm_mday);
    record2.tm_mon = record2.tm_mon-1;
    sscanf(edit[8]->get_text(),"%d", &record2.tm_year);
    record2.tm_year = record2.tm_year-1900;
    sscanf(edit[11]->get_text(),"%d", &record2.tm_hour);
    sscanf(edit[14]->get_text(),"%d", &record2.tm_min);
    
    sscanf(edit[19]->get_text(),"%d", &record3.tm_mon);
    sscanf(edit[20]->get_text(),"%d", &record3.tm_mday);
    record3.tm_mon -= 1;
    sscanf(edit[21]->get_text(),"%d", &record3.tm_year);
    record3.tm_year -= 1900;
    sscanf(edit[22]->get_text(),"%d", &record3.tm_hour);
    sscanf(edit[23]->get_text(),"%d", &record3.tm_min);

    sdr->rs.start[0]=mktime(&record1);
    
    sdr->rs.start[1]=mktime(&record2);
    
    sdr->rs.start[2]=mktime(&record3);

    printf("start[0] %ld start[1] %ld start[2] %ld\n",sdr->rs.start[0],sdr->rs.start[1],sdr->rs.start[2]);

    sscanf(edit[16]->get_text(),"%ld", &sdr->rs.stop[0]);
    sdr->rs.stop[0] *= 60;
    sscanf(edit[17]->get_text(),"%ld", &sdr->rs.stop[1]);
    sdr->rs.stop[1] *= 60;
    sscanf(edit[18]->get_text(),"%ld", &sdr->rs.stop[2]);
    sdr->rs.stop[2] *= 60;

    
    sscanf(frequency[0]->get_text(),"%lf", &sdr->rs.frequency[0]);
    sscanf(frequency[1]->get_text(),"%lf", &sdr->rs.frequency[1]);
    sscanf(frequency[2]->get_text(),"%lf", &sdr->rs.frequency[2]);

    return 0;
}
static void IQSave(struct Scene *scene,char *name)
{
    
    if(GetWorking(scene->FilePathIQ,sizeof(scene->FilePathIQ))){
        strncatToPath(scene->FilePathIQ,name,sizeof(scene->FilePathIQ));
    }
    //fprintf(stderr,"FilePathIQ %s nlast %d\n",scene->FilePathIQ,nlast);
    
    filePath[nlast]->set_text(scene->FilePathIQ);
    
}
static void control_cb(int control)
{
    std::string file_name;
    time_t now;
    
    if(control > 99){
        void AudioSave(struct Scene *scene,char *name);
        control -= 100;
        nlast=control;
        dialogSaveC(sdr->scene,IQSave,0,NULL);
    }else if(control == 1)
    {
        time(&now);  /* get current time; same as: now = time(NULL)  */
        
        today = *localtime(&now);
        
        msprintf(tex[0],sizeof(tex[0]),"%d",today.tm_mon+1);
        edit[0]->set_text(tex[0]);
        
        msprintf(tex[3],sizeof(tex[3]),"%d",today.tm_mday);
        edit[3]->set_text(tex[3]);
        
        msprintf(tex[6],sizeof(tex[6]),"%d",today.tm_year+1900);
        edit[6]->set_text(tex[6]);
        
        msprintf(tex[9],sizeof(tex[9]),"%d",today.tm_hour);
        edit[9]->set_text(tex[9]);
       
        msprintf(tex[12],sizeof(tex[12]),"%d",today.tm_min);
        edit[12]->set_text(tex[12]);
    }
    else if(control == 4)
    {
        getValues();
        
        sdr->rs.on[0]=on[0];
        sdr->rs.on[1]=on[1];
        sdr->rs.on[2]=on[2];

        /*
        printf("tm_sec %d ",dvr.tm_sec);
        printf("tm_min %d ",dvr.tm_min);
        printf("tm_hour %d ",dvr.tm_hour);
        printf("tm_mday %d ",dvr.tm_mday);
        printf("tm_mon %d ",dvr.tm_mon);
        printf("tm_year %d ",dvr.tm_year+1900);
        printf("tm_zone %s\n",dvr.tm_zone);
        
        */
        
    }
    else if(control == 2)
    {
        getValues();
        
        sdr->rs.on[0]=on[0];
        sdr->rs.on[1]=on[1];
        sdr->rs.on[2]=on[2];
        
        glui->close();
        glui=NULL;
    }
    
    glutPostRedisplay();
}
