        if(rx->cutOFFSearch > 0){
            //fprintf(stderr,"length %d cutOFF %g bw %g\n",length,rx->cutOFF,rx->bw);
            int itwas = -1;
            int ns=0;
            for(int k=0;k<length;++k){
                if(range[ns]+rx->bw > range[k]){
                    continue;
                }
               // fprintf(stderr,"low %g high %g\n",range[ns],range[k]);
                double dmin = -160;
                int nn = -1;
                double rr=0;
                for(int i=ns;i<k;++i){
                    if(i >= length)break;
                    rr=magnitude[i];
                    // fprintf(stderr,"i %d rr %g dmin %g\n",i,rr,dmin);
                    if(rr > dmin){
                        dmin=rr;
                        nn=i;
                    }
                }
                ns=k;
                if(nn > 0){
                    char *Mode_Names[] = {(char *)"FM",(char *)"NBFM",(char *)"AM",(char *)"NAM",(char *)"USB",(char *)"LSB",(char *)"CW"};
                    
                    static int count;
                    
                    if(dmin > rx->cutOFF){
                      // fprintf(stderr,"dmin %g range %g nn %d itwas %d\n",dmin,range[nn],nn,itwas);
                        if(nn-itwas > 5){
                           WarningPrint("F%d,%0.4f,%s\n",count++,range[nn]/1e6,Mode_Names[rx->decodemode]);
                        }
                        itwas=nn;
                    }
                }
            }
            
            
            rx->cutOFFSearch=0;
        }
    }
 