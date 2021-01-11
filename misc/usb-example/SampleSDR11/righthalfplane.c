// righthalfplane: demonstrate SSB demod
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sndfile.h>
#include <complex.h>
#include <liquid/liquid.h>

// cc -o righthalfplane righthalfplane.c -lliquid -lsndfile

int main(int argc, char*argv[])
{
    // options
    char *       filename_raw       = "s_rfspace_IQ_27335000_50000_fc.raw";
    char *       filename_wav       = "righthalfplane.wav";
    float        fc                 = -0.2f; // AM carrier
    liquid_ampmodem_type type       = LIQUID_AMPMODEM_USB;
    int          suppressed_carrier = 1;
    float        mod_index          = 0.1;
    unsigned int sample_rate_raw    = 50000;
    unsigned int sample_rate_wav    = 16000;

    // open input file for processing
    FILE * fid_raw = fopen(filename_raw,"r");

    // open output sound file for writing
    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof (sfinfo));
    sfinfo.samplerate = sample_rate_wav;
    sfinfo.channels   = 1; // mono
    sfinfo.format     = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    SNDFILE * fid_wav = sf_open(filename_wav, SFM_WRITE, &sfinfo);

    // mixer for down-conversion
    nco_crcf mixer = nco_crcf_create(LIQUID_VCO);
    nco_crcf_set_frequency(mixer, 2*M_PI*fc);

    // rational rate resampler with low-pass filter
    unsigned int gcd = liquid_gcd(sample_rate_raw, sample_rate_wav);
    unsigned int Q   = sample_rate_raw / gcd;           // input rate
    unsigned int P   = sample_rate_wav / gcd;           // output rate
    unsigned int m   = 12;                              // filter semi-length
    float        bw  = 3.0e3f / (float)sample_rate_raw; // filter bandwidth
    float        As  = 40.0f;                           // stop-band suppression [dB]
    rresamp_crcf resamp = rresamp_crcf_create_kaiser(P,Q,m,bw,As);
    rresamp_crcf_print(resamp);
    
    fprintf(stderr,"Q %u P %u gcd %u\n",Q,P,gcd);

    // analog demodulator
    ampmodem demod = ampmodem_create(mod_index, 0, type, suppressed_carrier);

    // audio AGC
    agc_rrrf agc = agc_rrrf_create();
    agc_rrrf_set_bandwidth(agc, 400.0f / (float)sample_rate_wav); // agc response
    agc_rrrf_set_scale(agc, 0.05f); // output audio scale (avoid clipping)

    // allocate buffers for sample processing (two each complex and real)
    unsigned int  buf_len = P > Q ? P : Q;
    float complex buf_0[buf_len];
    float complex buf_1[buf_len];
    float         buf_2[buf_len];
    float         buf_3[buf_len];

    // process entire file
    unsigned int num_blocks = 0;
    while (!feof(fid_raw)) {
        // read 'Q' raw samples into buffer
        if (fread(buf_0, sizeof(float complex), Q, fid_raw) != Q)
            break;

        // skip first few blocks (corrupt?)
        num_blocks++;
        if (num_blocks < 3)
            continue;

        // mix down 'Q' samples
        nco_crcf_mix_block_down(mixer, buf_0, buf_1, Q);

        // resample 'Q' samples in buf_1 into 'P' samples in buf_0
        rresamp_crcf_execute(resamp, buf_1, buf_0);

        // perform amplitude demodulation
        ampmodem_demodulate_block(demod, buf_0, P, buf_2);

        // apply automatic gain control
        agc_rrrf_execute_block(agc, buf_2, P, buf_3);

        // save results to audio file
        sf_write_float(fid_wav, buf_3, P);
    }
    sf_close(fid_wav) ;
    printf("output audio written to %s\n", filename_wav);

    // destroy objects and return
    nco_crcf_destroy(mixer);
    rresamp_crcf_destroy(resamp);
    ampmodem_destroy(demod);
    agc_rrrf_destroy(agc);
    return 0;
}
