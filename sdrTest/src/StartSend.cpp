static int StartSend(struct playData *rx,char *name,int type)
{
    if(!rx)return 0;

    if(rx->controlSend >= 0){
        printf("Already Running - Cannot Start New Transfer\n");
        return 0;
    }else{
        printf("name %s\n",name);
    }
    
    rx->send=(SOCKET)connectToServer((char *)name,&rx->Port);
    if(rx->send == -1){
        fprintf(stderr,"connect failed\n");
        return 1;
    }
    
    rx->dataType=type;
    rx->controlSend = 0;
    launchThread((void *)rx,rxSend);
    
    return 0;
}
int rxSend(void *rxv)
{
    
    struct playData *rx=(struct playData *)rxv;
    long size=2;
    
    if(!rx)return 0;
    
    rx->save=new saveData;
    
    rx->samplerate_save=-1;
    rx->fc_save=-1;

    int type=rx->dataType;
    while(rx->controlSend >= 0){
       if(rx->controlSend == 1){
            writeStat(rx->send,rx);
           if(type == 0){
               double amin =  1e33;
               double amax = -1e33;
               double average=0;
               for(int n=0;n<2*rx->size;++n){
                   double v=rx->sendBuff1[n];
                   average += v;
                   if(v > amax)amax=v;
                   if(v < amin)amin=v;
               }
               //printf("r amin %g amax %g ",amin,amax);
               
               average /= 2*rx->size;
               
               amax -= average;
               
               amax -= average;
               
               if(rx->aminGlobal2 == 0.0)rx->aminGlobal2=amin;
               rx->aminGlobal2 = 0.8*rx->aminGlobal2+0.2*amin;
               amin=rx->aminGlobal2;
               
               if(rx->amaxGlobal2 == 0.0)rx->amaxGlobal2=amax;
               rx->amaxGlobal2 = 0.8*rx->amaxGlobal2+0.2*amax;
               amax=rx->amaxGlobal2;
               
               //printf("a amin %g amax %g ",amin,amax);

               double dnom=0.0;
               if((amax-amin) > 0){
                   dnom=65534.0/(amax-amin);
               }else{
                   dnom=65534.0;
               }
               
               double gain=0.9;
               
               float *data=(float *)rx->sendBuff2;
               
               amin =  1e33;
               amax = -1e33;
               
               long int count=0;

               for(int n=0;n<2*rx->size;++n){
                   double v;
                   v=rx->sendBuff1[n];
                   v=gain*((v-average)*dnom);
                   if(v < amin)amin=v;
                   if(v > amax)amax=v;
                   if(v < -32768){
                       v = -32768;
                       ++count;
                   }else if(v > 32767){
                       v=32767;
                       ++count;
                   }
                   data[n]=(float)v;
               }
               //printf("f amin %g amax %g count %ld\n",amin,amax,count);

               size=(long)(rx->size*sizeof(float));
               if(writeLab(rx->send,(char *)"FLOA",size))return 1;
               if(netWrite(rx->send,(char *)rx->sendBuff2,size))return 1;
               if(writeLab(rx->send,(char *)"FLOA",size))return 1;
               if(netWrite(rx->send,(char *)&rx->sendBuff2[rx->size],size))return 1;
            }else if(type == 1){
                double amin =  1e33;
                double amax = -1e33;
                double average=0;
                for(int n=0;n<2*rx->size;++n){
                    double v=rx->sendBuff1[n];
                    average += v;
                    if(v > amax)amax=v;
                    if(v < amin)amin=v;
                }
                //printf("r amin %g amax %g ",amin,amax);
                
                average /= 2*rx->size;
                
                amax -= average;
                
                amax -= average;
                
                if(rx->aminGlobal2 == 0.0)rx->aminGlobal2=amin;
                rx->aminGlobal2 = 0.8*rx->aminGlobal2+0.2*amin;
                amin=rx->aminGlobal2;
                
                if(rx->amaxGlobal2 == 0.0)rx->amaxGlobal2=amax;
                rx->amaxGlobal2 = 0.8*rx->amaxGlobal2+0.2*amax;
                amax=rx->amaxGlobal2;
                
                //printf("a amin %g amax %g ",amin,amax);
                
                double dnom=0.0;
                if((amax-amin) > 0){
                    dnom=65534.0/(amax-amin);
                }else{
                    dnom=65534.0;
                }
                
                double gain=0.9;
                
                short int *data=(short int *)rx->sendBuff2;
                
                amin =  1e33;
                amax = -1e33;
                
                long int count=0;

                for(int n=0;n<2*rx->size;++n){
                    double v;
                    v=rx->sendBuff1[n];
                    v=gain*((v-average)*dnom);
                    if(v < amin)amin=v;
                    if(v > amax)amax=v;
                    if(v < -32768){
                        v = -32768;
                        ++count;
                    }else if(v > 32767){
                        v=32767;
                        ++count;
                   }
                    data[n]=(short int)v;
                }
               // printf(" f amin %g amax %g count %ld\n",amin,amax,count);
                size=rx->size*sizeof(short int);
                if(writeLab(rx->send,(char *)"SHOR",size))return 1;
                if(netWrite(rx->send,(char *)rx->sendBuff2,size))return 1;
                if(writeLab(rx->send,(char *)"SHOR",size))return 1;
                if(netWrite(rx->send,(char *)&data[rx->size],size))return 1;
           }else if(type == 2){
               double amin =  1e33;
               double amax = -1e33;
               double average=0;
               for(int n=0;n<2*rx->size;++n){
                   double v=rx->sendBuff1[n];
                   average += v;
                   if(v > amax)amax=v;
                   if(v < amin)amin=v;
               }
               //printf("r amin %g amax %g ",amin,amax);
               
               average /= 2*rx->size;
               
               amax -= average;
               
               amax -= average;
               
               if(rx->aminGlobal2 == 0.0)rx->aminGlobal2=amin;
               rx->aminGlobal2 = 0.8*rx->aminGlobal2+0.2*amin;
               amin=rx->aminGlobal2;
               
               if(rx->amaxGlobal2 == 0.0)rx->amaxGlobal2=amax;
               rx->amaxGlobal2 = 0.8*rx->amaxGlobal2+0.2*amax;
               amax=rx->amaxGlobal2;

               //printf("a amin %g amax %g ",amin,amax);
               double dnom=0.0;
               if((amax-amin) > 0){
                   dnom=255.0/(amax-amin);
               }else{
                   dnom=255.0;
               }
               
               double gain=0.9;
               
               signed char *data=(signed char *)rx->sendBuff2;
               
               amin =  1e33;
               amax = -1e33;
               
               long int count=0;

               for(int n=0;n<2*rx->size;++n){
                   double v;
                   v=rx->sendBuff1[n];
                   v=gain*((v-average)*dnom);
                   if(v < amin)amin=v;
                   if(v > amax)amax=v;
                   if(v < -128){
                       ++count;
                       v = -128;
                   }else if(v > 127){
                       v=127;
                       ++count;
                 }
                   data[n]=(signed char)v;
               }
               //printf(" f amin %g amax %g count %ld\n",amin,amax,count);
               size=rx->size*sizeof(signed char);
               if(writeLab(rx->send,(char *)"SIGN",size))return 1;
               if(netWrite(rx->send,(char *)rx->sendBuff2,size))return 1;
               if(writeLab(rx->send,(char *)"SIGN",size))return 1;
               if(netWrite(rx->send,(char *)&data[rx->size],size))return 1;
               
           }else if(type == 3){
               double amin =  1e33;
               double amax = -1e33;
               double average=0;
               for(int n=0;n<2*rx->size;++n){
                   double v=rx->sendBuff1[n];
                   average += v;
                   if(v > amax)amax=v;
                   if(v < amin)amin=v;
               }
               //printf("r amin %g amax %g ",amin,amax);
               
               average /= 2*rx->size;
               
               amax -= average;
               
               amax -= average;
               
               if(rx->aminGlobal2 == 0.0)rx->aminGlobal2=amin;
               rx->aminGlobal2 = 0.8*rx->aminGlobal2+0.2*amin;
               amin=rx->aminGlobal2;
               
               if(rx->amaxGlobal2 == 0.0)rx->amaxGlobal2=amax;
               rx->amaxGlobal2 = 0.8*rx->amaxGlobal2+0.2*amax;
               amax=rx->amaxGlobal2;
               //printf("a a amin %g amax %g ",amin,amax);

               double dnom=0.0;
               if((amax-amin) > 0){
                   dnom=255.0/(amax-amin);
               }else{
                   dnom=255.0;
               }
                              
               double gain=0.9;
               
               unsigned char *data=(unsigned char *)rx->sendBuff2;
               
               amin =  1e33;
               amax = -1e33;
               
               
               long int count=0;
               
               for(int n=0;n<2*rx->size;++n){
                   double v;
                   v=rx->sendBuff1[n];
                   v=gain*((v-average)*dnom+141.0);
                   if(v < amin)amin=v;
                   if(v > amax)amax=v;
                   if(v < 0){
                       v = 0;
                       ++count;
                   }else if(v > 255){
                       v=255;
                       ++count;
                  }
                   data[n]=(unsigned char)v;
               }
              // printf("f  amin %g amax %g count %ld\n",amin,amax,count);
               size=rx->size*sizeof(unsigned char);
               if(writeLab(rx->send,(char *)"USIG",size))return 1;
               if(netWrite(rx->send,(char *)rx->sendBuff2,size))return 1;
               if(writeLab(rx->send,(char *)"USIG",size))return 1;
               if(netWrite(rx->send,(char *)&data[rx->size],size))return 1;
           }
           rx->controlSend = 0;
        }else{
            Sleep2(20);
        }
    }
  

    if(rx->send >= 0){
        doEnd(rx->send);
        shutdown(rx->send,2);
        closesocket(rx->send);
    }
    
    delete rx->save;

    return 0;
}
int writeStat(SOCKET toServerSocket,struct playData *rx)
{
    double buff[2];
    
    if(!rx)return 0;
    
    if((rx->save->samplerate != rx->samplerate) || (rx->save->fc != rx->fc)){
        rx->save->fc=rx->fc;
        rx->save->samplerate=rx->samplerate;
    
        buff[0]=rx->fc;
        buff[1]=rx->samplerate;
    
        long size=(long)2*sizeof(double);
    
        if(writeLab(toServerSocket,(char *)"STAT",size))return 1;
    
        if(netWrite(toServerSocket,(char *)buff,size))return 1;
    
    }
    
    if(rx->frequencyFlag && (rx->save->f != rx->f)){
        rx->save->f = rx->f;
        buff[0]=rx->f;
        long size=(long)sizeof(double);
        if(writeLab(toServerSocket,(char *)"F   ",size))return 1;
        if(netWrite(toServerSocket,(char *)buff,size))return 1;
    }
    
    if(rx->demodulationFlag && (rx->save->decodemode != rx->decodemode)){
        rx->save->decodemode = rx->decodemode;
        buff[0]=rx->decodemode;
        long size=(long)sizeof(double);
        if(writeLab(toServerSocket,(char *)"DECO",size))return 1;
        if(netWrite(toServerSocket,(char *)buff,size))return 1;

    }
    
    return 0;
}