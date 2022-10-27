static int TransmitThread2(void *rxv)
{
    
    RadioPtr s=(RadioPtr)rxv;
    struct playData *rx=s->rx;
    
    //fprintf(stderr,"Transmit Thread Start\n");
    
    std::vector<SoapySDR::Kwargs> results;
    
    results = SoapySDR::Device::enumerate();
    
    SoapySDR::Kwargs deviceArgs;
    
    SoapySDR::Device *device=NULL;
    
    for(unsigned int k=0;k<results.size();++k){
        
        if(k == rx->deviceNumber){
            
            deviceArgs = results[k];
            device = SoapySDR::Device::make(deviceArgs);
            break;
            
        }
    }
    
    // fprintf(stderr,"getFullDuplex %d\n",device->getFullDuplex(SOAPY_SDR_TX, rx->channel));
    
    //const double frequency = 27.315e6;  //center frequency to 500 MHz
    double frequency = 87.6e6;  //center frequency to 500 MHz
    const double sample_rate = 2e6;    //sample rate to 5 MHz
    float As = 60.0f;
    
    std::vector<size_t> channels;
    
    channels = {0};
    
    frequency = s->tt.fc+s->tt.foffset;
    
    fprintf(stderr,"frequency %g Offset %g \n",s->tt.fc,s->tt.foffset);
    
    s->tt.info.device=device;
    
    device->setSampleRate(SOAPY_SDR_TX, rx->channel, sample_rate);
    
    fprintf(stderr,"Sample rate: %g MHz\n",sample_rate/1e6);
    
    //Set center frequency
    
    
    device->setFrequency(SOAPY_SDR_TX, rx->channel, frequency);
    
    device->setGain(SOAPY_SDR_TX, rx->channel, s->tt.gain);
    
    //  device->setAntenna(SOAPY_SDR_TX, rx->channel, BAND1);
    
    //Streaming Setup
    
    SoapySDR::Stream *txStream = device->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, channels);
    //fprintf(stderr,"txStream %p\n",txStream);
    
    //fprintf(stderr,"txStream %p\n",txStream);
    
    s->tt.info.txStream=txStream;
    
    //size_t MTU=device->getStreamMTU(txStream);
    
    // fprintf(stderr,"MTU %ld\n",MTU);
    
    
    //device->setHardwareTime(0); //clear HW time for easy debugging
    
    int ret4=device->activateStream(txStream);
    if(ret4)fprintf(stderr,"ret4 %d\n",ret4);
    
    unsigned int samples=48000;

    float Ratio1 = (float)(10000/ (float)samples);
    
    float Ratio2 = (float)(sample_rate/(float)10000);
    
    liquid_ampmodem_type type=LIQUID_AMPMODEM_DSB;
    int flag=0;
    if(s->tt.modetype == Mode_NBFM){
        Ratio1 = (float)(12500/ (float)samples);
        Ratio2 = (float)(sample_rate/(float)12500);
    }else if(s->tt.modetype == Mode_USB){
        type=LIQUID_AMPMODEM_USB;
        Ratio1 = (float)(6000/ (float)samples);
        Ratio2 = (float)(sample_rate/(float)6000);
        flag=1;
    }else if(s->tt.modetype == Mode_LSB){
        type=LIQUID_AMPMODEM_LSB;
        Ratio1 = (float)(6000/ (float)samples);
        Ratio2 = (float)(sample_rate/(float)6000);
        flag=1;
    }
    
    ampmodem demodAM;
    
#ifdef LIQUID_VERSION_4
    demodAM = ampmodem_create(0.5, 0.0, type, flag);
#else
    demodAM = ampmodem_create(0.5, type, flag);
#endif
    
    s->tt.info.modetype=s->tt.modetype;
    
    fprintf(stderr,"Ratio1 %g Ratio2 %g modetype %d\n",Ratio1,Ratio2,s->tt.modetype);
    
    msresamp_rrrf iqSampler  =  msresamp_rrrf_create(Ratio1, As);
    
    s->tt.info.iqSampler=iqSampler;
    
    msresamp_crcf iqSampler2  = msresamp_crcf_create(Ratio2, As);
    
    s->tt.info.iqSampler2=iqSampler2;
    
    s->tt.info.demodAM=demodAM;
    
    AMmod modulation(1.0);
    
    s->tt.info.am=&modulation;
    
    freqmod mod = freqmod_create(0.5);
    
    s->tt.info.mod=mod;
    
    
    std::vector<void *> buffs(2);
    
    unsigned int bufferFrames = 512;
    
    try {
        s->tt.audio->openStream( NULL, &s->tt.Params, RTAUDIO_SINT16, samples, &bufferFrames, &input, (void *)&s->tt.info );
    }
    catch ( RtAudioError& e ) {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
        goto cleanup;
    }
    
    fprintf(stderr,"getStreamSampleRate %u\n",s->tt.audio->getStreamSampleRate());
    
    fprintf(stderr,"Ready For Voice\n");
    
    try {
        s->tt.audio->startStream();
    }
    catch ( RtAudioError& e ) {
        std::cout << '\n' << e.getMessage() << '\n' << std::endl;
        goto cleanup;
    }
    
    while(s->tt.doTransmit == 1){
        if ( s->tt.audio->isStreamRunning() ) {
            Sleep2(10);
        }
    }
cleanup:
    
    if (  s->tt.audio->isStreamOpen() )  s->tt.audio->closeStream();
    
    freqmod_destroy(mod);
    
    if(demodAM)ampmodem_destroy(demodAM);
    
    device->deactivateStream(txStream);
    
    device->closeStream(txStream);
    
    SoapySDR::Device::unmake(device);
    
    s->tt.doTransmit = -1;
    
    device=NULL;
    
    txStream=NULL;
    
    //fprintf(stderr,"Transmit Thread End\n");
    
    return 0;
}