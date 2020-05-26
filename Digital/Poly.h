//
//  Poly.h
//  digital
//
//  Created by Dale on 12/17/16.
//  Copyright © 2016 Dale. All rights reserved.
//

#ifndef Poly_hpp
#define Poly_hpp

#include <stdio.h>

#include <stdio.h>

#include <complex>

#include <iostream>

#include "Utilities.h"

#include "BatchFile.h"


#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif


#define max(x1,x2)    (((x1) > (x2)) ? (x1) : (x2))
#define min(x1,x2)    (((x1) < (x2)) ? (x1) : (x2))


using namespace std;


enum fType { SIN = 1, COS, EXP, IMPULSE, STEP, IC, DELAY};

struct poleszeros{
    int np;
    int nz;
    complex<double> *poles;
    complex<double> *zeros;
    double *ts;
    double *rs;
    double con;
};

struct BiQuad{
    double kk;
    double a1;
    double a2;
    double b0;
    double b1;
    double b2;
    double dx1;
    double dx2;
    double dy1;
    double dy2;
};

class Poly {
    
public:
    int SetPolesAndZeros(int np,int nz);
    int doButterWorth(int np);
    int doChev(int np,double r);
    int norm();
    int diff();
    int cascadeEM();
    int dft(int npoints);
    int low(double w0,int inorm);
    int high(double w0,int inorm);
    int sweep(double f1,double f2,int ns,int npass,int ilog);
    int band(double w0,int flag,double iter);
    int march(int nstep,double step,int flag);
    int trans(BatchPtr Batch,int nzero,int npole);
    int forces(BatchPtr Batch,int nforces);
    int writePoly(char *type);
    int warp(double wT);
    int invert(int flag);
    int bilinear(double wT);
    int response(double wT);
    int cresponse(double wT);
    int mult(complex<double> *sum,complex<double> *t1,complex<double> *t2,int n1,int n2,int *n3);
    int canelPolesZeros(complex<double> *poles,complex<double> *zeros,int *np,int *nz);
    int force(double *input,int npoint);
    int forceFIR(double *input,int npoint);
    int forceCascade(double *input,int npoint);
    int forcepoleszeros(int);
    Poly();
    ~Poly();
    double sampleRate;
    double con;
    int iprint;
    int iangle;
    int nz;
    int np;
    int nforces;
    double *delay;
    double *coef1;
    double *coef2;
    struct poleszeros *pz;
    int *type;
    int nfore;
    int nback;
    complex<double> *poles;
    complex<double> *zeros;
    double *fore;
    double *back;
    double *FIRCoefficients;
    int FIRCount;
    double thetaNorm;
    struct BiQuad *biquad;
    int cascade;
};








#endif /* Poly_h */
