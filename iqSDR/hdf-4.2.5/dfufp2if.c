/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF.  The full HDF copyright notice, including       *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at      *
 * http://hdfgroup.org/products/hdf4/doc/Copyright.html.  If you do not have *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "hdf.h"
#include "dfufp2i.h"
#include "dfsd.h"
#include "hproto_fortran.h"

/*-----------------------------------------------------------------------------
 * Name:    duif2i
 * Purpose: Intermediate Fortran callable version of DFUfptoimage
 *          (See DFUfptoimage for details)
 *
 * Invokes: DFUfptoimage
 *---------------------------------------------------------------------------*/

FRETVAL(int)
nduif2i(int32 *hdim, int32 *vdim, float32 *max, float32 *min, float32 hscale[],
        float32 vscale[], float32 data[], _fcd palette, _fcd outfile,
        int *ct_method, int32 *hres, int32 *vres, int *compress, int *lenfn)
{
    char       *fn;
    int         ret;

    fn = HDf2cstring(outfile, *lenfn);
    if (!fn)
	return(FAIL);
    ret = DFUfptoimage(*hdim, *vdim, (float32)*max, (float32)*min, hscale, vscale, data,
                (uint8 *)_fcdtocp(palette), fn, *ct_method, *hres, *vres, *compress);
    HDfree(fn);
    return (ret);
}

