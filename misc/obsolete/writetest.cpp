#include <stdio.h>

#include <complex>

#include <iostream>

using namespace std;

struct BiQuad{
    double kk;
    double b0;
    double b1;
    double b2;
    double a1;
    double a2;
    double dx1;
    double dx2;
    double dy1;
    double dy2;
};

struct BiQuad biquad[] = {
                            {   9.028964294e-02,    1.000000000e+00,    2.000000000e+00,    1.000000000e+00,   -1.529892261e+00,    8.910508325e-01},
                            {   8.157309006e-02,    1.000000000e+00,    2.000000000e+00,    1.000000000e+00,   -1.382196618e+00,    7.084889787e-01},
                            {   7.477331027e-02,    1.000000000e+00,    2.000000000e+00,    1.000000000e+00,   -1.266979301e+00,    5.660725420e-01},
                            {   6.955525230e-02,    1.000000000e+00,    2.000000000e+00,    1.000000000e+00,   -1.178563108e+00,    4.567841169e-01},
                            {   6.565859258e-02,    1.000000000e+00,    2.000000000e+00,    1.000000000e+00,   -1.112537046e+00,    3.751714162e-01},
                            {   6.289037207e-02,    1.000000000e+00,    2.000000000e+00,    1.000000000e+00,   -1.065631565e+00,    3.171930531e-01},
                            {   6.111552335e-02,    1.000000000e+00,    2.000000000e+00,    1.000000000e+00,   -1.035558046e+00,    2.800201390e-01},
                            {   6.024871905e-02,    1.000000000e+00,    2.000000000e+00,    1.000000000e+00,   -1.020870678e+00,    2.618655540e-01}
                         };

double sampleRate=10000;

double thetaNorm=0;

double Corner=1000;

int cascaderesponce(struct BiQuad *biquad,long cascade,int steps);

int cascadestart(struct BiQuad *biquad,long cascade);

int cascadesine(struct BiQuad *biquad,long cascade,double frequency,int steps);

int cascadeforce(struct BiQuad *biquad,long cascade,double *input,int npoint);

int main(int argv,char *argc[])
{
    long cascade=(long)(sizeof(biquad)/sizeof(struct BiQuad));

    printf("sampleRate %g cascade %ld\n",sampleRate,cascade);


    cascaderesponce(&biquad[0],cascade,101);

    cascadestart(&biquad[0],cascade);

    cascadesine(&biquad[0],cascade,Corner*0.5,101);

    cascadesine(&biquad[0],cascade,0.5*(Corner+0.5*sampleRate),101);

    return 0;

}
int cascadesine(struct BiQuad *biquad,long cascade,double frequency,int steps)
{
	printf("cascadesine frequency %g\n",frequency);
	
	double *force = new double[steps];
	
	double pi=4.0*atan(1.0);

    for(int n=0;n<steps;++n){
        force[n]=sin(2*pi*frequency*n/(double)sampleRate);
    }

	cascadeforce(biquad,cascade,force,steps);
	
	delete [] force;
	
	return 0;
}
int cascadeforce(struct BiQuad *biquad,long cascade,double *input,int steps)
{
    
    double scale=1.0;  // normalize at 0 Hz
    
    double t=1.0/sampleRate;
    
    fprintf(stderr,"        forceCascade steps %d period %18.9e\n\n",steps,t);

    fprintf(stderr,"           SECONDS,           AMPLITUDE,             INPUT\n");

    for(int n=0;n<steps;++n){
        double y;
        double x;
        
        x=input[n];
        
        y=0;
        
        for(int k=0;k<cascade;++k){
            x=x*biquad[k].kk;
            y=x*biquad[k].b0+biquad[k].dx1*biquad[k].b1+biquad[k].dx2*biquad[k].b2-
                             biquad[k].dy1*biquad[k].a1-biquad[k].dy2*biquad[k].a2;
            biquad[k].dx2=biquad[k].dx1;
            biquad[k].dx1=x;
            biquad[k].dy2=biquad[k].dy1;
            biquad[k].dy1=y;
            x=y;
        }
                
        fprintf(stderr,"    %18.9e, %18.9e , %18.9e\n",n*t,y/scale,x);
    }
    
    
    return 0;
}

int cascadestart(struct BiQuad *biquad,long cascade)
{
    for(int k=0;k<cascade;++k){
        biquad[k].dx1=0.0;
        biquad[k].dx2=0.0;
        biquad[k].dy1=0.0;
        biquad[k].dy2=0.0;
    
	}

	return 0;
}
int cascaderesponce(struct BiQuad *biquad,long cascade,int steps)
{
	
    complex<double> sum;
    
    double pi=4.0*atan(1.0);
    
    double dt=(pi)/(steps-1);

    fprintf(stderr,"\n      cresponse thetaNorm %g \n",thetaNorm);

    fprintf(stderr,"\n      FREQUENCY,         AMPLITUDE\n");

    for(int k=0;k<steps;++k){
        double theta=k*dt;
        complex<double> z = exp(complex<double>(0,theta));
        sum = 1.0;
        for(int k=0;k<cascade;++k){
            sum=sum*biquad[k].kk*(z*z*biquad[k].b0+z*biquad[k].b1+biquad[k].b2)/(z*z+z*biquad[k].a1+biquad[k].a2);
        }
        fprintf(stderr,"%18.9e, %18.9e %18.9e\n",theta*sampleRate/(2*pi),abs(sum),theta/pi);
    }
    
	return 0;
}