class GLUIAPI GLUI_Button2 : public GLUI_Button
{
public:
    RadioPtr s;
    virtual int mouse_down_handler( int local_x, int local_y );
    virtual int mouse_up_handler( int local_x, int local_y, bool inside );
    GLUI_Button2( GLUI_Node *parent, const char *name,RadioPtr s,
                int id=-1, GLUI_CB cb=GLUI_CB() );
    
};
int GLUI_Button2::mouse_down_handler( int local_x, int local_y )
 {
     double fc,foffset;
     
     sscanf(s->tt.edittext2->get_text(),"%lg", &fc);
     sscanf(s->tt.edittext1->get_text(),"%lg", &foffset);
     
    fprintf(stderr,"mouse_down_handler\n");
    if(s->tt.doTransmit == 0){
         s->tt.fc=fc;
         s->tt.foffset=foffset;
         //s->rx->pstopPlay(s->rx);
         s->tt.doTransmit=1;
         launchThread((void *)s,TransmitThread);
     }
     
     GLUI_Button::mouse_down_handler( local_x, local_y);
     
     return false;
     
 }
int GLUI_Button2::mouse_up_handler( int local_x, int local_y, bool inside )
{
    fprintf(stderr,"mouse_up_handler %d \n",s->tt.doTransmit);
    if(s->tt.doTransmit == 1){
        s->tt.doTransmit=0;
        int count=0;
        while(s->tt.doTransmit == 0){
           Sleep2(10);
            if(++count > 200)break;
        }
        //fprintf(stderr,"count2 %d %d\n",count2,s->tt.doTransmit);
        s->tt.doTransmit=0;
        //s->rx->pstartPlay(s->rx);
        //s->rx->pplayRadio(s->rx);
    }
    
    GLUI_Button::mouse_up_handler( local_x, local_y,inside );

    return false;
    
}

GLUI_Button2::GLUI_Button2( GLUI_Node *parent, const char *name,RadioPtr ss,
                           int id, GLUI_CB cb) : GLUI_Button(parent,name,id,cb)
{
    s=ss;
}