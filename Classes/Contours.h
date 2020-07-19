/*
 *  Contours.h
 *  Rx3
 *
 *  Created by Dale Ranta on 2/5/14.
 *  Copyright 2014. All rights reserved.
 *
 */
#ifndef _Contours_h_
#define _Contours_h_

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
	
#include "SetFrameData.h"
#include "DrawPalette.h"
#include "Linedata.h"
	
	struct cdata{
		double *xdata;
		double *ydata;
		double *vdata;
		long nndata;
		long nlevel;
	};
	
	struct cdatal{
		struct cdata *cList;
		long count;
		long cmax;
	};
	
	
	int LineCountours(double *x,double *y,double *v,long *material,
					  double *levels,long nx,long ny,long nl,struct cdatal *d);
	int AreaContour(double *levels,long nl,struct SetFrameData *sd,void *lines);
	
#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif

