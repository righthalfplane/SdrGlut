#include "mThread.h"
#include "iqSDR.h"
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cerrno>
#include <vector>
#include <chrono>
#include <thread>
#include <complex>
#include <iostream>
#include <cstring>
#include <wx/file.h>

using namespace std;

static int copyl(char *p1,char *p2,long n);

SOCKET connectToServer(char *serverName,unsigned short *Port);

int sendto2(SOCKET socket,char *data,long size,struct sockaddr *server_addr,size_t sizeaddr);

int netWrite(SOCKET toServerSocket,char *buffout,long HdfSize);

int writeLab(SOCKET toServerSocket,char *Type,long size);

long Bytes;

void winout(const char *fmt, ...);

int BasicPane::txPipe()
{

	winout("txPipe \n");
	
	int outFilenum=fileno(stdout);
	
	sdr->fillBuffer=1;
    while(sendFlag > 0){
      if(sdr->fillBuffer == 0){
		int ret = write(outFilenum,sdr->sendBuff, sizeof(float)*2*sdr->size);
		if(ret <= 0){
			winout("txPipe Write Error\n");
		}
      	sdr->fillBuffer=1;
	  }
	}
	sdr->fillBuffer=0;
	return 0;
}
int BasicPane::SendStart(char *name,int type,int mode)
{
    if(!sx)return 0;

	//++mode;
	
	if(mode == 4){
		std::thread(&BasicPane::txPipe,this).detach();
		return 0;
	}
	
	copyl(name,addressName,strlen(name)+1);
	
	
	
   // winout("addressName %s mode %d\n",addressName,mode);
    
    sx->aminGlobal2=0;
    sx->amaxGlobal2=0;

    sx->name=name;
    sx->dataType=type;
    sx->sendMode=mode;
    
    if(mode < 2){
        sx->send=(SOCKET)connectToServer((char *)name,&sx->Port);
        if(sx->send == -1){
            winout("TCP connect failed\n");
            return 1;
        }
    }else{
        if ((sx->send = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        {
            winout("UDP connect failed\n");
            return 1;
        }
        
        int broadcast=1;
 
        setsockopt(sx->send, SOL_SOCKET, SO_BROADCAST,
                   (const char *)&broadcast, sizeof broadcast);

     // winout("SendStart sx->name %s\n",rx->name);
      
        if(mode == 3)return 0;
        
    }
 
    if(sx->sendBuff1)cFree((char *)sx->sendBuff1);
    sx->sendBuff1=(float *)cMalloc(2*sdr->size*sizeof(float),18887);
    if(sx->sendBuff2)cFree((char *)sx->sendBuff2);
    sx->sendBuff2=(float *)cMalloc(2*sdr->size*sizeof(float),18888);
      	
    std::thread(&BasicPane::rxSend,this).detach();

    return 0;
}

int BasicPane::rxSend()
{
    
    struct sockaddr_in server_addr;
    struct hostent *host;
    char name[256];
    int ret=0;
    long size=2;
    
    if(!sx)return 0;
    
    copyl(addressName,name,strlen(addressName)+1);
    
    char *find=strrchr(name,':');
	if(find)*find=0;
    
    host= (struct hostent *) gethostbyname((char *)name);
    if(!host){
        winout("Host %s Not Found - sendAudio uses port 5000 no port should be specified\n",addressName);
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    //bzero(&(server_addr.sin_zero),8);
    zerol((char *)&(server_addr.sin_zero),sizeof(server_addr.sin_zero));

//	winout("rxSend 1\n");
    sdr->fillBuffer=1;
    int mode=sx->sendMode;;
    int type=sx->dataType;
    while(sendFlag > 0){
 	  //winout("rxSend type %d fillBuffer %d mode %d\n",type,sdr->fillBuffer,mode);
 	  
 	  int ip=sdr->bS2->popBuff();
      if(ip >= 0){
			int witch=ip % NUM_DATA_BUFF;
			float *buf=sdr->bS2->buff[witch];
			for(int n=0;n<2*sdr->size;++n){
				sdr->sendBuff[n]=buf[n];
			}
           if(type == 0){
               double amin =  1e33;
               double amax = -1e33;
               double average=0;
               for(int n=0;n<2*sdr->size;++n){
                   double v=sdr->sendBuff[n];
                   average += v;
                   if(v > amax)amax=v;
                   if(v < amin)amin=v;
               }
                              
               average /= 2*sdr->size;
               
               amin -= average;
               
               amax -= average;
               
               if(sx->aminGlobal2 == 0.0)sx->aminGlobal2=amin;
               sx->aminGlobal2 = 0.8*sx->aminGlobal2+0.2*amin;
               amin=sx->aminGlobal2;
               
               if(sx->amaxGlobal2 == 0.0)sx->amaxGlobal2=amax;
               sx->amaxGlobal2 = 0.8*sx->amaxGlobal2+0.2*amax;
               amax=sx->amaxGlobal2;
               
               //winout("a amin %g amax %g ",amin,amax);

               double dnom=0.0;
               if((amax-amin) > 0){
                   dnom=2.0/(amax-amin);
               }else{
                   dnom=1.0;
               }
               
               double gain=0.9;
               
               float *data=(float *)sx->sendBuff2;
               
               amin =  1e33;
               amax = -1e33;
               
               long int count=0;

               for(int n=0;n<2*sdr->size;++n){
                   double v;
                   v=sdr->sendBuff[n];
                   v=gain*((v-average)*dnom);
                   if(v < amin)amin=v;
                   if(v > amax)amax=v;
                   if(v < -1.0){
                       v = -1.0;
                       ++count;
                   }else if(v > 1.0){
                       v=1.0;
                       ++count;
                   }
                   data[n]=(float)v;
               }
               
              // winout("f amin %g amax %g count %ld\n",amin,amax,count);

               size=(long)(sdr->size*sizeof(float));
               if(mode == 0){
                   if(writeLab(sx->send,(char *)"FLOA",size))return 1;
                   if(netWrite(sx->send,(char *)data,size))return 1;
                   if(writeLab(sx->send,(char *)"FLOA",size))return 1;
                   if(netWrite(sx->send,(char *)&data[sdr->size],size))return 1;    
               }else if(mode == 1){
                  if(netWrite(sx->send,(char *)sx->sendBuff2,size))return 1;
                  if(netWrite(sx->send,(char *)&sx->sendBuff2[sdr->size],size))return 1;
            	}else{
                	ret=sendto2(sx->send,(char *)sx->sendBuff2, size,
                            (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
                	ret=sendto2(sx->send,(char *)&sx->sendBuff2[sdr->size], size,
                            (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
               }
            }else if(type == 1){
                double amin =  1e33;
                double amax = -1e33;
                double average=0;
                for(int n=0;n<2*sdr->size;++n){
                   double v=sdr->sendBuff[n];
                    average += v;
                    if(v > amax)amax=v;
                    if(v < amin)amin=v;
                }
                
               // winout("type 1 r amin %g amax %g sdr->size %d\n",amin,amax,sdr->size);
                
                average /= 2*sdr->size;
                
                amin -= average;
                
                amax -= average;
                
                if(sx->aminGlobal2 == 0.0)sx->aminGlobal2=amin;
                sx->aminGlobal2 = 0.8*sx->aminGlobal2+0.2*amin;
                amin=sx->aminGlobal2;
                
                if(sx->amaxGlobal2 == 0.0)sx->amaxGlobal2=amax;
                sx->amaxGlobal2 = 0.8*sx->amaxGlobal2+0.2*amax;
                amax=sx->amaxGlobal2;
                
                //winout("a amin %g amax %g ",amin,amax);
                
                double dnom=0.0;
                if((amax-amin) > 0){
                    dnom=65534.0/(amax-amin);
                }else{
                    dnom=65534.0;
                }
                
                double gain=0.9;
                
                short int *data=(short int *)sx->sendBuff2;
                
                amin =  1e33;
                amax = -1e33;
                
              //  long int count=0;

                for(int n=0;n<2*sdr->size;++n){          
                    double v=sdr->sendBuff[n];
                    v=gain*((v-average)*dnom);
                    if(v < amin)amin=v;
                    if(v > amax)amax=v;
                    if(v < -32768){
                        v = -32768;
             //           ++count;
                    }else if(v > 32767){
                        v=32767;
             //           ++count;
                   }
                    data[n]=(short int)v;
                }
               // winout(" f amin %g amax %g count %ld\n",amin,amax,count);
                size=sdr->size*sizeof(short int);
                if(mode == 0){
                    if(writeLab(sx->send,(char *)"SHOR",size))return 1;
                    if(netWrite(sx->send,(char *)data,size))return 1;
                    if(writeLab(sx->send,(char *)"SHOR",size))return 1;
                    if(netWrite(sx->send,(char *)&data[sdr->size],size))return 1;
                }else if(mode == 1){
                    if(netWrite(sx->send,(char *)sx->sendBuff2,size))return 1;
                    if(netWrite(sx->send,(char *)&data[sdr->size],size))return 1;
                }else{

                    ret=sendto2(sx->send,(char *)sx->sendBuff2, size,
                                (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
                    ret=sendto2(sx->send,(char *)&data[sdr->size], size,
                           (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
                }
           }else if(type == 2){
               double amin =  1e33;
               double amax = -1e33;
               double average=0;
               for(int n=0;n<2*sdr->size;++n){
                   double v=sdr->sendBuff[n];
                   average += v;
                   if(v > amax)amax=v;
                   if(v < amin)amin=v;
               }
               //winout("r amin %g amax %g ",amin,amax);
               
               average /= 2*sdr->size;
               
               amin -= average;
               
               amax -= average;
               
               if(sx->aminGlobal2 == 0.0)sx->aminGlobal2=amin;
               sx->aminGlobal2 = 0.8*sx->aminGlobal2+0.2*amin;
               amin=sx->aminGlobal2;
               
               if(sx->amaxGlobal2 == 0.0)sx->amaxGlobal2=amax;
               sx->amaxGlobal2 = 0.8*sx->amaxGlobal2+0.2*amax;
               amax=sx->amaxGlobal2;

               //winout("a amin %g amax %g ",amin,amax);
               double dnom=0.0;
               if((amax-amin) > 0){
                   dnom=255.0/(amax-amin);
               }else{
                   dnom=255.0;
               }
               
               double gain=0.9;
               
               signed char *data=(signed char *)sx->sendBuff2;
               
               amin =  1e33;
               amax = -1e33;
               
             //  long int count=0;

               for(int n=0;n<2*sdr->size;++n){
                   double v=sdr->sendBuff[n];
                   v=gain*((v-average)*dnom);
                   if(v < amin)amin=v;
                   if(v > amax)amax=v;
                   if(v < -128){
               //        ++count;
                       v = -128;
                   }else if(v > 127){
                       v=127;
              //         ++count;
                 }
                   data[n]=(signed char)v;
               }
               //winout(" f amin %g amax %g count %ld\n",amin,amax,count);
               size=sdr->size*sizeof(signed char);
               if(mode == 0){
                   if(writeLab(sx->send,(char *)"SIGN",size))return 1;
                   if(netWrite(sx->send,(char *)data,size))return 1;
                   if(writeLab(sx->send,(char *)"SIGN",size))return 1;
                   if(netWrite(sx->send,(char *)&data[sdr->size],size))return 1;
               }else if(mode == 1){
                   if(netWrite(sx->send,(char *)sx->sendBuff2,size))return 1;
                   if(netWrite(sx->send,(char *)&data[sdr->size],size))return 1;
               }else{
                   ret=sendto2(sx->send,(char *)sx->sendBuff2, size,
                               (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
                   ret=sendto2(sx->send,(char *)&data[sdr->size], size,
                          (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
               }
           }else if(type == 3){
               double amin =  1e33;
               double amax = -1e33;
               double average=0;
               for(int n=0;n<2*sdr->size;++n){
                   double v=sdr->sendBuff[n];
                   average += v;
                   if(v > amax)amax=v;
                   if(v < amin)amin=v;
               }
               //winout("r amin %g amax %g ",amin,amax);
               
               average /= 2*sdr->size;
               
               amin -= average;
               
               amax -= average;
               
               if(sx->aminGlobal2 == 0.0)sx->aminGlobal2=amin;
               sx->aminGlobal2 = 0.8*sx->aminGlobal2+0.2*amin;
               amin=sx->aminGlobal2;
               
               if(sx->amaxGlobal2 == 0.0)sx->amaxGlobal2=amax;
               sx->amaxGlobal2 = 0.8*sx->amaxGlobal2+0.2*amax;
               amax=sx->amaxGlobal2;
               //winout("a a amin %g amax %g ",amin,amax);

               double dnom=0.0;
               if((amax-amin) > 0){
                   dnom=255.0/(amax-amin);
               }else{
                   dnom=255.0;
               }
                              
               double gain=0.9;
               
               unsigned char *data=(unsigned char *)sx->sendBuff2;
               
               amin =  1e33;
               amax = -1e33;
               
               
              // long int count=0;
               
               for(int n=0;n<2*sdr->size;++n){
                   double v=sdr->sendBuff[n];
                   v=gain*((v-average)*dnom+141.0);
                   if(v < amin)amin=v;
                   if(v > amax)amax=v;
                   if(v < 0){
                       v = 0;
                //       ++count;
                   }else if(v > 255){
                       v=255;
                //       ++count;
                  }
                   data[n]=(unsigned char)v;
               }
              // winout("f  amin %g amax %g count %ld\n",amin,amax,count);
               size=sdr->size*sizeof(unsigned char);
               if(mode == 0){
                   if(writeLab(sx->send,(char *)"USIG",size))return 1;
                   if(netWrite(sx->send,(char *)data,size))return 1;
                   if(writeLab(sx->send,(char *)"USIG",size))return 1;
                   if(netWrite(sx->send,(char *)&data[sdr->size],size))return 1;
               }else if(mode == 1){
                   if(netWrite(sx->send,(char *)sx->sendBuff2,size))return 1;
                   if(netWrite(sx->send,(char *)&data[sdr->size],size))return 1;
              }else{
                   ret=sendto2(sx->send,(char *)sx->sendBuff2, size,
                               (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
                   ret=sendto2(sx->send,(char *)&data[sdr->size], size,
                               (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
               }
           }
           sdr->fillBuffer = 1;
        }else{
            Sleep2(10);
        }
    }
  
    sdr->fillBuffer = 0;

    if(sx->send >= 0){
        if(mode < 2){
            shutdown(sx->send,2);
            winout("shutdown mode %d\n",mode);
        }
        closesocket(sx->send);
    }
    
    return ret;
}

int BasicPane::sendAudio(int short *data,int length)
{
    char broadcastname[256];
    struct sockaddr_in server_addr;
    struct hostent *host;
    int ret;

    //winout("sendAudio length %d sendMode %d controlSend %d\n",length,sx->sendMode,sx->controlSend);
    
    if(!sx)return 1;
    
    if(sx->sendMode < 3)return 0;
    
    if(sendFlag <= 0){
        if(sx->send)closesocket(sx->send);
        sx->send=0;
        return 0;
    }
    
    
    if(addressName[0]){
        mstrncpy(broadcastname,(char *)addressName,sizeof(broadcastname));
    }else{
        mstrncpy(broadcastname,(char *)"0.0.0.0",sizeof(broadcastname));
    }

    host= (struct hostent *) gethostbyname((char *)broadcastname);
    
    if(!host){
        winout("Host %s Not Found - sendAudio uses port 5000 no port should be specified\n",broadcastname);
        return 1;
    }
    
    //host= (struct hostent *) gethostbyname((char *)"127.0.0.1");
    //host= (struct hostent *) gethostbyname((char *)"0.0.0.0");
    // host= (struct hostent *) gethostbyname((char *)"192.255.255.255");
    //host= (struct hostent *) gethostbyname((char *)"192.168.0.255");
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    //bzero(&(server_addr.sin_zero),8);
    zerol((char *)&(server_addr.sin_zero),sizeof(server_addr.sin_zero));
    
    ret=sendto2(sx->send,(char *)data, length*2,
                 (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    //static long nn=0;
    //winout("sendAudio length %d nn %ld\n",length,nn++);
    if(ret)winout("sendto2 Error\n");
    
   // winout("sendAudio sx->name %s ret %d length %d\n",addressName,ret,length);

    
    //static FILE *out=NULL;

    //if(!out)out=fopen("junk2.raw","wb");
    
    //if(out)fwrite(data,2,length,out);

    return 0;
}

int BasicPane2::txPipe()
{

	winout("txPipe \n");
	
	int outFilenum=fileno(stdout);
	
	sdr->fillBuffer=1;
    while(sendFlag > 0){
      if(sdr->fillBuffer == 0){
		int ret = write(outFilenum,sdr->sendBuff, sizeof(float)*2*sdr->size);
		if(ret <= 0){
			winout("txPipe Write Error\n");
		}
      	sdr->fillBuffer=1;
	  }
	}
	sdr->fillBuffer=0;
	return 0;
}
int BasicPane2::SendStart(char *name,int type,int mode)
{
    if(!sx)return 0;

	//++mode;
	
	if(mode == 4){
		std::thread(&BasicPane2::txPipe,this).detach();
		return 0;
	}
	
	copyl(name,addressName,strlen(name)+1);
	
	
	
   // winout("addressName %s mode %d\n",addressName,mode);
    
    sx->aminGlobal2=0;
    sx->amaxGlobal2=0;

    sx->name=name;
    sx->dataType=type;
    sx->sendMode=mode;
    
    if(mode < 2){
        sx->send=(SOCKET)connectToServer((char *)name,&sx->Port);
        if(sx->send == -1){
            winout("TCP connect failed\n");
            return 1;
        }
    }else{
        if ((sx->send = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        {
            winout("UDP connect failed\n");
            return 1;
        }
        
        int broadcast=1;
 
        setsockopt(sx->send, SOL_SOCKET, SO_BROADCAST,
                   (const char *)&broadcast, sizeof broadcast);

     // winout("SendStart sx->name %s\n",rx->name);
      
        if(mode == 3)return 0;
        
    }
 
    if(sx->sendBuff1)cFree((char *)sx->sendBuff1);
    sx->sendBuff1=(float *)cMalloc(2*sdr->size*sizeof(float),18887);
    if(sx->sendBuff2)cFree((char *)sx->sendBuff2);
    sx->sendBuff2=(float *)cMalloc(2*sdr->size*sizeof(float),18888);
      	
    std::thread(&BasicPane2::rxSend,this).detach();

    return 0;
}

int BasicPane2::rxSend()
{
    
    struct sockaddr_in server_addr;
    struct hostent *host;
    char name[256];
    int ret=0;
    long size=2;
    
    if(!sx)return 0;
    
    copyl(addressName,name,strlen(addressName)+1);
    
    char *find=strrchr(name,':');
	if(find)*find=0;
    
    host= (struct hostent *) gethostbyname((char *)name);
    if(!host){
        winout("Host %s Not Found - sendAudio uses port 5000 no port should be specified\n",addressName);
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    //bzero(&(server_addr.sin_zero),8);
    zerol((char *)&(server_addr.sin_zero),sizeof(server_addr.sin_zero));

//	winout("rxSend 1\n");
    sdr->fillBuffer=1;
    int mode=sx->sendMode;;
    int type=sx->dataType;
    while(sendFlag > 0){
 	  //winout("rxSend type %d fillBuffer %d mode %d\n",type,sdr->fillBuffer,mode);
 	  
 	  int ip=sdr->bS2->popBuff();
      if(ip >= 0){
			int witch=ip % NUM_DATA_BUFF;
			float *buf=sdr->bS2->buff[witch];
			for(int n=0;n<2*sdr->size;++n){
				sdr->sendBuff[n]=buf[n];
			}
           if(type == 0){
               double amin =  1e33;
               double amax = -1e33;
               double average=0;
               for(int n=0;n<2*sdr->size;++n){
                   double v=sdr->sendBuff[n];
                   average += v;
                   if(v > amax)amax=v;
                   if(v < amin)amin=v;
               }
                              
               average /= 2*sdr->size;
               
               amin -= average;
               
               amax -= average;
               
               if(sx->aminGlobal2 == 0.0)sx->aminGlobal2=amin;
               sx->aminGlobal2 = 0.8*sx->aminGlobal2+0.2*amin;
               amin=sx->aminGlobal2;
               
               if(sx->amaxGlobal2 == 0.0)sx->amaxGlobal2=amax;
               sx->amaxGlobal2 = 0.8*sx->amaxGlobal2+0.2*amax;
               amax=sx->amaxGlobal2;
               
               //winout("a amin %g amax %g ",amin,amax);

               double dnom=0.0;
               if((amax-amin) > 0){
                   dnom=2.0/(amax-amin);
               }else{
                   dnom=1.0;
               }
               
               double gain=0.9;
               
               float *data=(float *)sx->sendBuff2;
               
               amin =  1e33;
               amax = -1e33;
               
               long int count=0;

               for(int n=0;n<2*sdr->size;++n){
                   double v;
                   v=sdr->sendBuff[n];
                   v=gain*((v-average)*dnom);
                   if(v < amin)amin=v;
                   if(v > amax)amax=v;
                   if(v < -1.0){
                       v = -1.0;
                       ++count;
                   }else if(v > 1.0){
                       v=1.0;
                       ++count;
                   }
                   data[n]=(float)v;
               }
               
              // winout("f amin %g amax %g count %ld\n",amin,amax,count);

               size=(long)(sdr->size*sizeof(float));
               if(mode == 0){
                   if(writeLab(sx->send,(char *)"FLOA",size))return 1;
                   if(netWrite(sx->send,(char *)data,size))return 1;
                   if(writeLab(sx->send,(char *)"FLOA",size))return 1;
                   if(netWrite(sx->send,(char *)&data[sdr->size],size))return 1;    
               }else if(mode == 1){
                  if(netWrite(sx->send,(char *)sx->sendBuff2,size))return 1;
                  if(netWrite(sx->send,(char *)&sx->sendBuff2[sdr->size],size))return 1;
            	}else{
                	ret=sendto2(sx->send,(char *)sx->sendBuff2, size,
                            (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
                	ret=sendto2(sx->send,(char *)&sx->sendBuff2[sdr->size], size,
                            (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
               }
            }else if(type == 1){
                double amin =  1e33;
                double amax = -1e33;
                double average=0;
                for(int n=0;n<2*sdr->size;++n){
                   double v=sdr->sendBuff[n];
                    average += v;
                    if(v > amax)amax=v;
                    if(v < amin)amin=v;
                }
                
               // winout("type 1 r amin %g amax %g sdr->size %d\n",amin,amax,sdr->size);
                
                average /= 2*sdr->size;
                
                amin -= average;
                
                amax -= average;
                
                if(sx->aminGlobal2 == 0.0)sx->aminGlobal2=amin;
                sx->aminGlobal2 = 0.8*sx->aminGlobal2+0.2*amin;
                amin=sx->aminGlobal2;
                
                if(sx->amaxGlobal2 == 0.0)sx->amaxGlobal2=amax;
                sx->amaxGlobal2 = 0.8*sx->amaxGlobal2+0.2*amax;
                amax=sx->amaxGlobal2;
                
                //winout("a amin %g amax %g ",amin,amax);
                
                double dnom=0.0;
                if((amax-amin) > 0){
                    dnom=65534.0/(amax-amin);
                }else{
                    dnom=65534.0;
                }
                
                double gain=0.9;
                
                short int *data=(short int *)sx->sendBuff2;
                
                amin =  1e33;
                amax = -1e33;
                
              //  long int count=0;

                for(int n=0;n<2*sdr->size;++n){          
                    double v=sdr->sendBuff[n];
                    v=gain*((v-average)*dnom);
                    if(v < amin)amin=v;
                    if(v > amax)amax=v;
                    if(v < -32768){
                        v = -32768;
             //           ++count;
                    }else if(v > 32767){
                        v=32767;
             //           ++count;
                   }
                    data[n]=(short int)v;
                }
               // winout(" f amin %g amax %g count %ld\n",amin,amax,count);
                size=sdr->size*sizeof(short int);
                if(mode == 0){
                    if(writeLab(sx->send,(char *)"SHOR",size))return 1;
                    if(netWrite(sx->send,(char *)data,size))return 1;
                    if(writeLab(sx->send,(char *)"SHOR",size))return 1;
                    if(netWrite(sx->send,(char *)&data[sdr->size],size))return 1;
                }else if(mode == 1){
                    if(netWrite(sx->send,(char *)sx->sendBuff2,size))return 1;
                    if(netWrite(sx->send,(char *)&data[sdr->size],size))return 1;
                }else{

                    ret=sendto2(sx->send,(char *)sx->sendBuff2, size,
                                (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
                    ret=sendto2(sx->send,(char *)&data[sdr->size], size,
                           (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
                }
           }else if(type == 2){
               double amin =  1e33;
               double amax = -1e33;
               double average=0;
               for(int n=0;n<2*sdr->size;++n){
                   double v=sdr->sendBuff[n];
                   average += v;
                   if(v > amax)amax=v;
                   if(v < amin)amin=v;
               }
               //winout("r amin %g amax %g ",amin,amax);
               
               average /= 2*sdr->size;
               
               amin -= average;
               
               amax -= average;
               
               if(sx->aminGlobal2 == 0.0)sx->aminGlobal2=amin;
               sx->aminGlobal2 = 0.8*sx->aminGlobal2+0.2*amin;
               amin=sx->aminGlobal2;
               
               if(sx->amaxGlobal2 == 0.0)sx->amaxGlobal2=amax;
               sx->amaxGlobal2 = 0.8*sx->amaxGlobal2+0.2*amax;
               amax=sx->amaxGlobal2;

               //winout("a amin %g amax %g ",amin,amax);
               double dnom=0.0;
               if((amax-amin) > 0){
                   dnom=255.0/(amax-amin);
               }else{
                   dnom=255.0;
               }
               
               double gain=0.9;
               
               signed char *data=(signed char *)sx->sendBuff2;
               
               amin =  1e33;
               amax = -1e33;
               
             //  long int count=0;

               for(int n=0;n<2*sdr->size;++n){
                   double v=sdr->sendBuff[n];
                   v=gain*((v-average)*dnom);
                   if(v < amin)amin=v;
                   if(v > amax)amax=v;
                   if(v < -128){
               //        ++count;
                       v = -128;
                   }else if(v > 127){
                       v=127;
              //         ++count;
                 }
                   data[n]=(signed char)v;
               }
               //winout(" f amin %g amax %g count %ld\n",amin,amax,count);
               size=sdr->size*sizeof(signed char);
               if(mode == 0){
                   if(writeLab(sx->send,(char *)"SIGN",size))return 1;
                   if(netWrite(sx->send,(char *)data,size))return 1;
                   if(writeLab(sx->send,(char *)"SIGN",size))return 1;
                   if(netWrite(sx->send,(char *)&data[sdr->size],size))return 1;
               }else if(mode == 1){
                   if(netWrite(sx->send,(char *)sx->sendBuff2,size))return 1;
                   if(netWrite(sx->send,(char *)&data[sdr->size],size))return 1;
               }else{
                   ret=sendto2(sx->send,(char *)sx->sendBuff2, size,
                               (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
                   ret=sendto2(sx->send,(char *)&data[sdr->size], size,
                          (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
               }
           }else if(type == 3){
               double amin =  1e33;
               double amax = -1e33;
               double average=0;
               for(int n=0;n<2*sdr->size;++n){
                   double v=sdr->sendBuff[n];
                   average += v;
                   if(v > amax)amax=v;
                   if(v < amin)amin=v;
               }
               //winout("r amin %g amax %g ",amin,amax);
               
               average /= 2*sdr->size;
               
               amin -= average;
               
               amax -= average;
               
               if(sx->aminGlobal2 == 0.0)sx->aminGlobal2=amin;
               sx->aminGlobal2 = 0.8*sx->aminGlobal2+0.2*amin;
               amin=sx->aminGlobal2;
               
               if(sx->amaxGlobal2 == 0.0)sx->amaxGlobal2=amax;
               sx->amaxGlobal2 = 0.8*sx->amaxGlobal2+0.2*amax;
               amax=sx->amaxGlobal2;
               //winout("a a amin %g amax %g ",amin,amax);

               double dnom=0.0;
               if((amax-amin) > 0){
                   dnom=255.0/(amax-amin);
               }else{
                   dnom=255.0;
               }
                              
               double gain=0.9;
               
               unsigned char *data=(unsigned char *)sx->sendBuff2;
               
               amin =  1e33;
               amax = -1e33;
               
               
              // long int count=0;
               
               for(int n=0;n<2*sdr->size;++n){
                   double v=sdr->sendBuff[n];
                   v=gain*((v-average)*dnom+141.0);
                   if(v < amin)amin=v;
                   if(v > amax)amax=v;
                   if(v < 0){
                       v = 0;
                //       ++count;
                   }else if(v > 255){
                       v=255;
                //       ++count;
                  }
                   data[n]=(unsigned char)v;
               }
              // winout("f  amin %g amax %g count %ld\n",amin,amax,count);
               size=sdr->size*sizeof(unsigned char);
               if(mode == 0){
                   if(writeLab(sx->send,(char *)"USIG",size))return 1;
                   if(netWrite(sx->send,(char *)data,size))return 1;
                   if(writeLab(sx->send,(char *)"USIG",size))return 1;
                   if(netWrite(sx->send,(char *)&data[sdr->size],size))return 1;
               }else if(mode == 1){
                   if(netWrite(sx->send,(char *)sx->sendBuff2,size))return 1;
                   if(netWrite(sx->send,(char *)&data[sdr->size],size))return 1;
              }else{
                   ret=sendto2(sx->send,(char *)sx->sendBuff2, size,
                               (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
                   ret=sendto2(sx->send,(char *)&data[sdr->size], size,
                               (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
               }
           }
           sdr->fillBuffer = 1;
        }else{
            Sleep2(10);
        }
    }
  
    sdr->fillBuffer = 0;

    if(sx->send >= 0){
        if(mode < 2){
            shutdown(sx->send,2);
            winout("shutdown mode %d\n",mode);
        }
        closesocket(sx->send);
    }
    
    return ret;
}

int BasicPane2::sendAudio(int short *data,int length)
{
    char broadcastname[256];
    struct sockaddr_in server_addr;
    struct hostent *host;
    int ret;

    //winout("sendAudio length %d sendMode %d controlSend %d\n",length,sx->sendMode,sx->controlSend);
    
    if(sx->sendMode < 3)return 0;
    
    if(sendFlag <= 0){
        if(sx->send)closesocket(sx->send);
        sx->send=0;
        return 0;
    }
    
    
    if(addressName[0]){
        mstrncpy(broadcastname,(char *)addressName,sizeof(broadcastname));
    }else{
        mstrncpy(broadcastname,(char *)"0.0.0.0",sizeof(broadcastname));
    }

    host= (struct hostent *) gethostbyname((char *)broadcastname);
    
    if(!host){
        winout("Host %s Not Found - sendAudio uses port 5000 no port should be specified\n",broadcastname);
        return 1;
    }
    
    //host= (struct hostent *) gethostbyname((char *)"127.0.0.1");
    //host= (struct hostent *) gethostbyname((char *)"0.0.0.0");
    // host= (struct hostent *) gethostbyname((char *)"192.255.255.255");
    //host= (struct hostent *) gethostbyname((char *)"192.168.0.255");
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5000);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    //bzero(&(server_addr.sin_zero),8);
    zerol((char *)&(server_addr.sin_zero),sizeof(server_addr.sin_zero));
    
    ret=sendto2(sx->send,(char *)data, length*2,
                 (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    //static long nn=0;
    //winout("sendAudio length %d nn %ld\n",length,nn++);
    if(ret)winout("sendto2 Error\n");
    
   // winout("sendAudio sx->name %s ret %d length %d\n",addressName,ret,length);

    
    //static FILE *out=NULL;

    //if(!out)out=fopen("junk2.raw","wb");
    
    //if(out)fwrite(data,2,length,out);

    return 0;
}


int sendto2(SOCKET socket,char *data,long size,struct sockaddr *server_addr,size_t sizeaddr)
{
    int ret;
    long n;

    while(size > 0){
        n=size;
        //if(n > 2040*4)n=2040*4;
        if(n > 1472)n=1472;
        ret=(int)sendto(socket,data, n, 0, server_addr, (socklen_t)sizeaddr);
        if(ret == -1){
            winout("sendto error %d EMSGSIZE %d\n",errno,EMSGSIZE);
            return 1;
        }
        //winout("socket %d ret %d\n",socket,ret);
        data += ret;
        size -= ret;
    }
    return 0;

}

int netWrite(SOCKET toServerSocket,char *buffout,long HdfSize)
{
	long n;
	
	//winout("HdfSize %ld\n",HdfSize);
	Bytes += HdfSize;
	while(HdfSize > 0){
		n=HdfSize;
		if(n > 120000)n=120000;
        n=::send(toServerSocket,buffout,n,0);
		if(n < 0){
			winout("Write Socket Error (%ld)\n",(long)errno);
			return 1;
		}
		buffout += n;
		HdfSize -= n;
	}
        
	return 0;
}

int writeString(SOCKET toServerSocket,char *buffout,long HdfSize)
{
	return netWrite(toServerSocket,buffout,HdfSize);
}

int writeLong(SOCKET toServerSocket,long i)
{
	unsigned char c[4];
	
	c[0] = (unsigned char)(i & 255);
	c[1] = (unsigned char)((i >> 8) & 255);
	c[2] = (unsigned char)((i >> 16) & 255);
	c[3] = (unsigned char)((i >> 24) & 255);

	if(writeString(toServerSocket,(char *)c,4L))return 1;

	return 0;
}


SOCKET connectToServer(char *serverName,unsigned short *Port)
{
	struct sockaddr_in serverSocketAddr;
	struct hostent *serverHostEnt;
	SOCKET toServerSocket;
	int ret;
	unsigned int hostAddr;
	unsigned int oneNeg;
	short result,Try;
	char *np;
	int buf_size;

    ret=0;
    
	/* oneNeg=0xffffffff; */
	oneNeg = -1L;

	long netsize=200000;

	buf_size=(int)(netsize+30);

	result= -1;

    zerol((char *)&serverSocketAddr, sizeof(serverSocketAddr));

	if(!(np=strrchr(serverName,':'))){
	    winout("Bad Address (%s)",serverName);
	    return result;
	}else{
	    *np=0;
	    np += 1;
	    *Port=(unsigned short)atol(np);
	}

//	hostAddr=(unsigned int)inet_addr(serverName);
	inet_pton(AF_INET, serverName, &hostAddr);
	if((long)hostAddr != (long)oneNeg){
	    serverSocketAddr.sin_addr.s_addr=hostAddr;
	    winout("Found Address %lx hostAddr %x oneNeg %x diff %x\n",(long)hostAddr,hostAddr,oneNeg,hostAddr-oneNeg);
	}else{
	    serverHostEnt=gethostbyname(serverName);
	    if(serverHostEnt == NULL){
	        winout("Could Not Find Host (%s)\n",serverName);
	        return result;
	    }
	    copyl((char *)serverHostEnt->h_addr,(char *)&serverSocketAddr.sin_addr,serverHostEnt->h_length);
	}
	serverSocketAddr.sin_family=AF_INET;
	serverSocketAddr.sin_port=htons(*Port);
	Try=0;
	while(Try++ < 4){
	    if((toServerSocket=socket(AF_INET,SOCK_STREAM,0)) < 0){
            winout("socket Error  (%ld)\n",(long)SOCKET_ERRNO);
	        return toServerSocket;
	    }

	    ret=setsockopt( toServerSocket, SOL_SOCKET, SO_SNDBUF,
                  (char *)&buf_size, sizeof(int) );
        if(ret < 0)winout("setsockopt SO_SNDBUF failed\n");

	
	    ret=connect(toServerSocket,(struct sockaddr *)&serverSocketAddr,sizeof(serverSocketAddr));
	    if(ret == -1){
                if (SOCKET_ERRNO == SOCKET_ECONNREFUSED)  {
                    winout("Connection Refused  Try(%d)\n",Try);
                    closesocket(toServerSocket);
                    std::this_thread::sleep_for(std::chrono::seconds(4));
                    continue;
                }else{
                    winout("Connection Error  (%ld)\n",(long)SOCKET_ERRNO);
                    return ret;
                }
	    }
	    return toServerSocket;
	}

       return ret;
}
static int copyl(char *p1,char *p2,long n)
{
	if(!p1 || !p2)return 1;

	while(n-- > 0)*p2++ = *p1++;

	return 0;
}
int writeLab(SOCKET toServerSocket,char *Type,long size)
{
	
	if(writeString(toServerSocket,Type,4L))return 1;
	if(writeLong(toServerSocket,(long)size))return 1;
	return 0;
}
