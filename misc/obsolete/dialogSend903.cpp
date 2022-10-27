int Radio::dialogSend(struct Scene *scene)
{
    if(!scene)return 1;
    
    if(bb.glui){
        bb.glui->close();
    }
    
    bb.sceneLocal=scene;
    
    bb.glui = GLUI_Master.create_glui(rx->driveName);
    
    GLUI_Panel *obj_panel =  bb.glui->add_panel( "Data Type" );

    bb.group2 =
    bb.glui->add_radiogroup_to_panel(obj_panel,&bb.datatype,Mode_Buttons,control_cb2);
    
    bb.glui->add_radiobutton_to_group( bb.group2, "Float" );
    bb.glui->add_radiobutton_to_group( bb.group2, "Short int" );
    bb.glui->add_radiobutton_to_group( bb.group2, "Signed char" );
    bb.glui->add_radiobutton_to_group( bb.group2, "UnSigned char" );

    obj_panel =  bb.glui->add_panel( "Send Mode" );
    
    bb.group2 =
    bb.glui->add_radiogroup_to_panel(obj_panel,&bb.sendmode,Mode_Buttons,control_cb2);
    
    bb.glui->add_radiobutton_to_group( bb.group2, "Listen Mode" );
    bb.glui->add_radiobutton_to_group( bb.group2, "TCP/IP" );
    bb.glui->add_radiobutton_to_group( bb.group2, "UDP" );

    obj_panel =  bb.glui->add_panel( "Send Information" );


    new GLUI_Checkbox( obj_panel, "Frequency", &bb.frequencyFlag, 10, control_cb2 );
    
    new GLUI_Checkbox( obj_panel, "Demodulate Mode", &bb.demodulationFlag, 11, control_cb2 );

    obj_panel =  bb.glui->add_panel( "Tcp-Address" );
    
    
    bb.edittext1 =
    bb.glui->add_edittext_to_panel( obj_panel, "", GLUI_EDITTEXT_TEXT, bb.text1);
    bb.edittext1->w=200;

/*
    obj_panel =  bb.glui->add_panel( "Voice Control" );
    
    strncpy(bb.text2,"./speechcontrol.py",sizeof(bb.text2));
    bb.edittext2 =
    bb.glui->add_edittext_to_panel( obj_panel, "Location:", GLUI_EDITTEXT_TEXT, bb.text2);
    bb.edittext2->w=200;
    
    new GLUI_Button(obj_panel,"Start Voice Control", 9,control_cb2);
    
    new GLUI_Button(obj_panel, "Stop Voice Control", 10,control_cb2);
*/

    obj_panel =  bb.glui->add_panel( "Commands" );
    
    new GLUI_Button(obj_panel, "Send", 2,control_cb2);
    
    new GLUI_Button(obj_panel, "Stop", 4, control_cb2);
    
    new GLUI_Button(obj_panel, "Close", 6, control_cb2);

    bb.sub_window=glutGetWindow();
    
    //bb.sub_window=bb.glui->get_glut_window_id();
    
    bb.glui->set_main_gfx_window(bb.sub_window);

    return 0;
}

static void control_cb2(int control)
{
    RadioPtr s=(RadioPtr)FindSceneRadio(glutGetWindow());
    if(!s)return;
    
    sscanf(s->bb.edittext1->get_text(),"%s",s->bb.text1);
    
    sscanf(s->bb.edittext2->get_text(),"%s",s->bb.text2);

    s->rx->demodulationFlag=s->bb.demodulationFlag;
    
    s->rx->frequencyFlag=s->bb.frequencyFlag;

    if(control == Mode_Buttons)
    {
        fprintf(stderr,"Mode_Buttons %d\n",s->bb.datatype);
    }else if(control == 2){
        (*s->rx->pStartSend)(s->rx,s->bb.text1,s->bb.datatype,s->bb.sendmode);
   } else if(control == 4){
       s->rx->controlSend = -1;
       fprintf(stderr,"Stop Command Send\n");
    } else if(control == 6){
        s->bb.glui->close();
        s->bb.glui=NULL;
    } else if(control == 9){
        if(s->voicecontrol == 0){
            std::thread(&Radio::doVoice,s).detach();
        }else{
            fprintf(stderr,"Voice Control Running flag: %d\n",s->voicecontrol);
        }
    } else if(control == 10){
        if(s->voicecontrol == 1)s->voicecontrol=2;
   }
    glutPostRedisplay();
}