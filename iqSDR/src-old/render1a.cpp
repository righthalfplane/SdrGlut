void Spectrum::render1a(wxPaintEvent& evt )
{
	evt.Skip();
	
	//winout("Spectrum render nc %lld\n",nc++);

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
    
    //winout("render1\n");
    
    int buffSendLength=0;
    
    float *buffSend2;
    
	if(!iWait){
	/*
  		uRect box;
	
 		box.x=ftox(sdr->f-sdr->bw/(2.0));
 		box.y=0;
 		box.xsize=ftox(sdr->f+sdr->bw/(2.0))-ftox(sdr->f-sdr->bw/(2.0));
 		box.ysize=getHeight();
 		 		
 		DrawBox(&box,0);
 	*/
 	
		glColor4f(0, 0, 1, 1);
		
		//winout("filterType %d\n",filterType);
		
		int ip = -1;
 	  	if(sdr->bS2)ip=sdr->bS2->popBuff();
      	if(ip < 0)return;
      	
      	int witch=ip % NUM_DATA_BUFF;
		buffSend2=sdr->bS2->buff[witch];
      	
      	buffSendLength=sdr->size;
		
		double amax=0;
		//double avg=0;
		for(int n=0;n<buffSendLength;++n){
			double v=(buffSend2[2*n]*buffSend2[2*n]+buffSend2[2*n+1]*buffSend2[2*n+1]);
        	v=fabs(v);
        	double mag=v;
        	if(mag > amax)amax=mag;
			//avg += mag;
		}
		
		//avg /= buffSendLength;
	
		//double shift=-100-avg;
		
		
		buffFlag=0;
		
		//double dnom=1;

		double ymin = -amax;
		double ymax =  amax;	
		if(ymin >= ymax)ymin=ymax-40;
		
		double dy=ymax-ymin;
		
		double iymin=0;
		double iymax=getHeight()-20;
		double idy=iymin-iymax;		
		
		double xmin=0;
		double xmax=1.0/sdr->ncut;
		double dx=xmax-xmin;

		double ixmin=50;
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
	
		double xmin2=0;
		double xmax2=1.0/sdr->ncut;
		double dx2=xmax2-xmin2;
		
		
		//fprintf(stderr,"buffSendLength %ld lineAlpha %g\n",(long)buffSendLength,lineAlpha);
			
		for(int n=0;n<buffSendLength;++n){
			double v=0;
			v=buffSend2[2*n];
			double x=dx2*n/((double)buffSendLength)+xmin2;
			double y=v;
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
			//if(n > 99900)fprintf(stderr,"n %d ix %d iy %d\n",n,ix,iy);
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
			xmnc=0.0;
			xmxc=1.0/sdr->ncut;
			//fprintf(stderr,"xmnc %g xmxc %g samplewidth %g\n",xmnc,xmxc,sdr->samplewidth);
			GridPlotNeat2(&xmnc,&xmxc,&Large,&Small);
			//fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
			
			for(double xp=xmnc;xp <= xmxc;xp += Large){
				char cbuff[256];
				sprintf(cbuff,"%g",xp);
			    double xx=((xp-xmin)/dx);
			    if(xx < 0.0 || xx > 1.0)continue;
			    int ixx=(int)(idx*xx);
			    if(ixx < ixmin || ixx > ixmax)continue;
 				DrawLine3(ixx, 0, ixx, getHeight()-15);
 				//winout(" %g ",xx);
 				DrawString(ixx-10,getHeight()-13, cbuff);
			}
			//winout(" idx %g\n",idx);
			

			xmnc=ymin;
			xmxc=ymax;
			//fprintf(stderr,"xmnc %g xmxc %g\n",xmnc,xmxc);
			GridPlotNeat2(&xmnc,&xmxc,&Large,&Small);
			//fprintf(stderr,"xmnc %g xmxc %g Large %g Small %g %g %g\n",xmnc,xmxc,Large,Small,xmnc/Large,xmxc/Large);
			
			for(double xp=xmnc;xp <= xmxc;xp += Large){
				char cbuff[256];
				sprintf(cbuff,"%g",xp);
			    double xx=((xp-ymin)/(dy));
			    if(xx < 0.0 || xx > 1.0)continue;
			    int ixx=(int)(iymax+idy*xx);
			    //int ixx=(int)(iymax-xx*idy);
				//winout(" %d %g ",ixx,xx);
 				DrawLine3(30, ixx, getWidth(), ixx);
 				//winout(" %g ",xx);
 				DrawString(5,ixx-8,cbuff);
			}
			//winout(" idy %g\n",idy);
		

		
 		}
		
		

		//auto t2 = chrono::high_resolution_clock::now();
		//std::chrono::duration<double> difference = t2 - t1;
		//std::cout << "Time "<< difference.count() << endl;
		//winout("count %g\n",difference.count());

	}

	//winout("Spectrum done\n");

	//fprintf(stderr,"Next 77\n");
		
    glFlush();
    SwapBuffers();    
    
 
}
 