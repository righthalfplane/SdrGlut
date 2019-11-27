/*
 *  Matrix.h
 *  
 *
 *  Created by Dale Ranta on 7/15/13.
 *  Copyright 2013 Dale Ranta. All rights reserved.
 *
 */

#ifndef __Matrix_H__
#define __Matrix_H__

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
	
	
struct System{
	struct P x;
	struct P y;
	struct P z;
	struct P p;
	double Theta_x;
	double Theta_y;
	double Theta_z;
	double Scale_x;
	double Scale_y;
	double Scale_z;
};


struct Matrix{
	double x[4][4];
};


int rotate3d(struct System *Local);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */


#endif
