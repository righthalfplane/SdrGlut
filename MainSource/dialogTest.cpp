#include <stdio.h>
#include "sdrReceive.h"
int dialogTest(void);

 int dialogTest()
 {
     SoapySDR::Kwargs deviceArgs;
     struct SceneList *list;
     struct Scene *scene;
     int FileType;
     

     FileType=FileTypeSdrReceive;
     list=SceneNext();
     if(list == NULL)
     {
         WarningPrint("doRadioOpen : Error Allocation Scene Memory File\n",FileType);
         return 1;
     }
     scene=&list->scene;
     zerol((char *)scene,sizeof(struct Scene));
     SceneInit(scene);
     scene->windowType=FileTypeSdrReceive;
     
     SdrReceivePtr w = new SdrReceive(scene,deviceArgs);
     
     if(w == NULL){
         WarningBatch((char *)"Radio of Memory");
         return 1;
     }
     

     if(w->OpenError != FALSE){
         delete w;
         return 0;
     }
     
     w->OpenWindows(scene);
     
     myAppl=(CWinPtr)w;
     
     AddWindowList(myAppl);
     
    //glutSetWindow(w->lines2->window);
    //glutSetWindowTitle(w->rx->driveName);
     
    w->backGroundEvents=1;

    printf("dialogTest 4\n");

     return 0;
 }
