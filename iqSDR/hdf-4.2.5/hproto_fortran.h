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

/* $Id: hproto.h 4784 2006-11-21 04:49:51Z bmribler $ */

#ifndef _H_PROTO_FORTRAN
#define _H_PROTO_FORTRAN
#include "h4config.h"


#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */


/*
   ** from dfanF.c
 */
#   define ndaiganl         H4_F77_FUNC(daiganl, DAIGANL)
#   define ndaigann         H4_F77_FUNC(daigann, DAIGANN)
#   define ndaipann         H4_F77_FUNC(daipann, DAIPANN)
#   define ndailist         H4_F77_FUNC(dailist, DAILIST)
#   define ndalref          H4_F77_FUNC(dalref, DALREF)
#   define ndaclear         H4_F77_FUNC(daclear, DACLEAR)
#   define ndfanlastref     H4_F77_FUNC(dfanlastref, DFANLASTREF)
#   define ndfanaddfds      H4_F77_FUNC(dfanaddfds, DFANADDFDS)
#   define ndfangetfidlen   H4_F77_FUNC(dfangetfidlen, DFANGETFIDLEN)
#   define ndfangetfdslen   H4_F77_FUNC(dfangetfdslen, DFANGETFDSLEN)
#   define ndfangetfid      H4_F77_FUNC(dfangetfid, DFANGETFID)
#   define ndfangetfds      H4_F77_FUNC(dfangetfds, DFANGETFDS)
#   define ndaafds          H4_F77_FUNC(daafds, DAAFDS)
#   define ndagfidl         H4_F77_FUNC(dagfidl, DAGFIDL)
#   define ndagfdsl         H4_F77_FUNC(dagfdsl, DAGFDSL)
#   define ndagfid          H4_F77_FUNC(dagfid, DAGFID)
#   define ndagfds          H4_F77_FUNC(dagfds, DAGFDS)
#   define ndaiafid         H4_F77_FUNC(daiafid, DAIAFID)

    HDFLIBAPI      FRETVAL(intf) ndaiganl
                (_fcd filename, intf * tag, intf * ref, intf * type,
                 intf * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndaigann
                (_fcd filename, intf * tag, intf * ref, _fcd annotation,
                 intf * maxlen, intf * type, intf * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndaipann
                (_fcd filename, intf * tag, intf * ref, _fcd annotation,
                 intf * annlen, intf * type, intf * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndailist
                (_fcd filename, intf * tag, intf reflist[], _fcd labellist,
          intf * listsize, intf * maxlen, intf * startpos,
                 intf * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndalref
                (void);

    HDFLIBAPI      FRETVAL(intf) ndaclear
                (void);

    HDFLIBAPI      FRETVAL(intf) ndfanlastref
                (void);

    HDFLIBAPI      FRETVAL(intf) ndfanaddfds
                (intf * dfile, _fcd desc, intf * desclen);

    HDFLIBAPI      FRETVAL(intf) ndfangetfidlen
                (intf * dfile, intf * isfirst);

    HDFLIBAPI      FRETVAL(intf) ndfangetfdslen
                (intf * dfile, intf * isfirst);

    HDFLIBAPI      FRETVAL(intf) ndfangetfid
                (intf * dfile, _fcd id, intf * maxlen, intf * isfirst);

    HDFLIBAPI      FRETVAL(intf) ndfangetfds
                (intf * dfile, _fcd id, intf * maxlen, intf * isfirst);

    HDFLIBAPI      FRETVAL(intf) ndaafds
                (intf * dfile, _fcd desc, intf * desclen);

    HDFLIBAPI      FRETVAL(intf) ndagfidl
                (intf * dfile, intf * isfirst);

    HDFLIBAPI      FRETVAL(intf) ndagfdsl
                (intf * dfile, intf * isfirst);

    HDFLIBAPI      FRETVAL(intf) ndagfid
                (intf * dfile, _fcd id, intf * maxlen, intf * isfirst);

    HDFLIBAPI      FRETVAL(intf) ndagfds
                (intf * dfile, _fcd id, intf * maxlen, intf * isfirst);

    HDFLIBAPI      FRETVAL(intf) ndaiafid
                (intf * dfile, _fcd id, intf * idlen);

/*
   ** from dfr8F.c
 */
#   define nd8spal           H4_F77_FUNC(d8spal, D8SPAL)
#   define nd8first          H4_F77_FUNC(d8first, D8FIRST)
#   define nd8igdim          H4_F77_FUNC(d8igdim, D8IGDIM)
#   define nd8igimg          H4_F77_FUNC(d8igimg, D8IGIMG)
#   define nd8ipimg          H4_F77_FUNC(d8ipimg, D8IPIMG)
#   define nd8iaimg          H4_F77_FUNC(d8iaimg, D8IAIMG)
#   define nd8irref          H4_F77_FUNC(d8irref, D8IRREF)
#   define nd8iwref          H4_F77_FUNC(d8iwref, D8IWREF)
#   define nd8inims          H4_F77_FUNC(d8inims, D8INIMS)
#   define nd8lref           H4_F77_FUNC(d8lref, D8LREF)
#   define ndfr8lastref      H4_F77_FUNC(dfr8lastref, DFR8LASTREF)
#   define ndfr8setpalette   H4_F77_FUNC(dfr8setpalette, DFR8SETPALETTE)
#   define ndfr8restart      H4_F77_FUNC(dfr8restart, DFR8RESTART)
#   define nd8scomp          H4_F77_FUNC(d8scomp, D8SCOMP)
#   define ndfr8scompress    H4_F77_FUNC(dfr8scompress, DFR8SCOMPRESS)
#   define nd8sjpeg          H4_F77_FUNC(d8sjpeg, D8SJPEG)
#   define ndfr8sjpeg        H4_F77_FUNC(dfr8sjpeg, DFR8SJPEG)

    HDFLIBAPI      FRETVAL(intf) nd8spal
                (_fcd pal);

    HDFLIBAPI      FRETVAL(intf) nd8first
                (void);

    HDFLIBAPI      FRETVAL(intf) nd8igdim
                (_fcd filename, intf * xdim, intf * ydim, intf * ispal,
                 intf * lenfn);

    HDFLIBAPI      FRETVAL(intf) nd8igimg
                (_fcd filename, _fcd image, intf * xdim, intf * ydim,
                 _fcd pal, intf * lenfn);

    HDFLIBAPI      FRETVAL(intf) nd8ipimg
                (_fcd filename, _fcd image, intf * xdim, intf * ydim,
                 intf * compress, intf * lenfn);

    HDFLIBAPI      FRETVAL(intf) nd8iaimg
                (_fcd filename, _fcd image, intf * xdim, intf * ydim,
                 intf * compress, intf * lenfn);

    HDFLIBAPI      FRETVAL(intf) nd8irref
                (_fcd filename, intf * ref, intf * fnlen);

    HDFLIBAPI      FRETVAL(intf) nd8iwref
                (_fcd filename, intf * ref, intf * fnlen);

    HDFLIBAPI      FRETVAL(intf) nd8inims
                (_fcd filename, intf * fnlen);

    HDFLIBAPI      FRETVAL(intf) nd8lref
                (void);

    HDFLIBAPI      FRETVAL(intf) ndfr8lastref
                (void);

    HDFLIBAPI      FRETVAL(intf) ndfr8setpalette
                (_fcd pal);

    HDFLIBAPI      FRETVAL(intf) ndfr8restart
                (void);

    HDFLIBAPI      FRETVAL(intf) nd8scomp
                (intf * scheme);

    HDFLIBAPI      FRETVAL(intf) ndfr8scompress
                (intf * scheme);

    HDFLIBAPI      FRETVAL(intf) nd8sjpeg
                (intf * quality, intf * force_baseline);

    HDFLIBAPI      FRETVAL(intf) ndfr8sjpeg
                (intf * quality, intf * force_baseline);

/*
   ** from dfsdF.c
 */
#   define ndsgdast          H4_F77_FUNC(dsgdast, DSGDAST)
#   define ndsgdisc          H4_F77_FUNC(dsgdisc, DSGDISC)
#   define ndsgrang          H4_F77_FUNC(dsgrang, DSGRANG)
#   define ndssdims          H4_F77_FUNC(dssdims, DSSDIMS)
#   define ndssdisc          H4_F77_FUNC(dssdisc, DSSDISC)
#   define ndssrang          H4_F77_FUNC(dssrang, DSSRANG)
#   define ndsclear          H4_F77_FUNC(dsclear, DSCLEAR)
#   define ndsslens          H4_F77_FUNC(dsslens, DSSLENS)
#   define ndsgdiln          H4_F77_FUNC(dsgdiln, DSGDILN)
#   define ndsgdaln          H4_F77_FUNC(dsgdaln, DSGDALN)
#   define ndsfirst          H4_F77_FUNC(dsfirst, DSFIRST)
#   define ndspslc           H4_F77_FUNC(dspslc, DSPSLC)
#   define ndseslc           H4_F77_FUNC(dseslc, DSESLC)
#   define ndsgnt            H4_F77_FUNC(dsgnt, DSGNT)
#   define ndssnt            H4_F77_FUNC(dssnt, DSSNT)
#   define ndsigdim          H4_F77_FUNC(dsigdim, DSIGDIM)
#   define ndsigdat          H4_F77_FUNC(dsigdat, DSIGDAT)
#   define ndsipdat          H4_F77_FUNC(dsipdat, DSIPDAT)
#   define ndsiadat          H4_F77_FUNC(dsiadat, DSIADAT)
#   define ndsigdas          H4_F77_FUNC(dsigdas, DSIGDAS)
#   define ndsigslc          H4_F77_FUNC(dsigslc, DSIGSLC)
#   define ndsigdis          H4_F77_FUNC(dsigdis, DSIGDIS)
#   define ndsisslc          H4_F77_FUNC(dsisslc, DSISSLC)
#   define ndsisdas          H4_F77_FUNC(dsisdas, DSISDAS)
#   define ndsisdis          H4_F77_FUNC(dsisdis, DSISDIS)
#   define ndsirref          H4_F77_FUNC(dsirref, DSIRREF)
#   define ndslref           H4_F77_FUNC(dslref, DSLREF)
#   define ndsinum           H4_F77_FUNC(dsinum, DSINUM)
#   define ndsip32s          H4_F77_FUNC(dsip32s, DSIP32S)
#   define ndsscal           H4_F77_FUNC(dsscal, DSSCAL)
#   define ndsgcal           H4_F77_FUNC(dsgcal, DSGCAL)
#   define ndfsdgetdatastrs  H4_F77_FUNC(dfsdgetdatastrs, DFSDGETDATASTRS)
#   define ndfsdgetdimscale  H4_F77_FUNC(dfsdgetdimscale, DFSDGETDIMSCALE)
#   define ndfsdgetrange     H4_F77_FUNC(dfsdgetrange, DFSDGETRANGE)
#   define ndfsdsetdims      H4_F77_FUNC(dfsdsetdims, DFSDSETDIMS)
#   define ndfsdsetdimscale  H4_F77_FUNC(dfsdsetdimscale, DFSDSETDIMSCALE)
#   define ndfsdsetrange     H4_F77_FUNC(dfsdsetrange, DFSDSETRANGE)
#   define ndfsdclear        H4_F77_FUNC(dfsdclear, DFSDCLEAR)
#   define ndfsdsetlengths   H4_F77_FUNC(dfsdsetlengths, DFSDSETLENGTHS)
#   define ndfsdgetdimlen    H4_F77_FUNC(dfsdgetdimlen, DFSDGETDIMLEN)
#   define ndfsdgetdatalen   H4_F77_FUNC(dfsdgetdatalen, DFSDGETDATALEN)
#   define ndfsdrestart      H4_F77_FUNC(dfsdrestart, DFSDRESTART)
#   define ndfsdputslice     H4_F77_FUNC(dfsdputslice, DFSDPUTSLICE)
#   define ndfsdendslice     H4_F77_FUNC(dfsdendslice, DFSDENDSLICE)
#   define ndfsdsetnt        H4_F77_FUNC(dfsdsetnt, DFSDSETNT)
#   define ndfsdgetnt        H4_F77_FUNC(dfsdgetnt, DFSDGETNT)
#   define ndfsdlastref      H4_F77_FUNC(dfsdlastref, DFSDLASTREF)
#   define ndsiwref          H4_F77_FUNC(dsiwref, DSIWREF)
#   define ndssfill          H4_F77_FUNC(dssfill, DSSFILL)
#   define ndsgfill          H4_F77_FUNC(dsgfill, DSGFILL)
#   define ndsisslab         H4_F77_FUNC(dsisslab, DSISSLAB)
#   define ndswslab          H4_F77_FUNC(dswslab, DSWSLAB)
#   define ndseslab          H4_F77_FUNC(dseslab, DSESLAB)
#   define ndsirslab         H4_F77_FUNC(dsirslab, DSIRSLAB)

    HDFLIBAPI      FRETVAL(intf) ndsgdisc
                (intf * dim, intf * maxsize, void * scale);

    HDFLIBAPI      FRETVAL(intf) ndsgrang
                (void * pmax, void * pmin);

    HDFLIBAPI      FRETVAL(intf) ndssdims
                (intf * rank, intf dimsizes[]);

    HDFLIBAPI      FRETVAL(intf) ndssdisc
                (intf * dim, intf * dimsize, void * scale);

    HDFLIBAPI      FRETVAL(intf) ndssrang
                (void * max, void * min);

    HDFLIBAPI      FRETVAL(intf) ndsclear
                (void);

    HDFLIBAPI      FRETVAL(intf) ndsslens
                (intf * maxlen_label, intf * maxlen_unit,
                 intf * maxlen_format, intf * maxlen_coordsys);

    HDFLIBAPI      FRETVAL(intf) ndsgdiln
                (intf * dim, intf * llabel, intf * lunit,
                 intf * lformat);

    HDFLIBAPI      FRETVAL(intf) ndsgdaln
                (intf * llabel, intf * lunit, intf * lformat,
                 intf * lcoordsys);

    HDFLIBAPI      FRETVAL(intf) ndsfirst
                (void);

    HDFLIBAPI      FRETVAL(intf) ndspslc
                (intf windims[], void * data, intf dims[]);

    HDFLIBAPI      FRETVAL(intf) ndseslc
                (void);

    HDFLIBAPI      FRETVAL(intf) ndssnt
                (intf * numbertype);

    HDFLIBAPI      FRETVAL(intf) ndsgnt
                (intf * pnumbertype);

    HDFLIBAPI      FRETVAL(intf) ndsigdim
                (_fcd filename, intf * prank, intf sizes[],
                 intf * maxrank, intf * lenfn);

    HDFLIBAPI      FRETVAL(intf) ndsigdat
                (_fcd filename, intf * rank, intf maxsizes[],
                 void * data, intf * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndsipdat
                (_fcd filename, intf * rank, intf dimsizes[],
                 void * data, intf * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndsiadat
                (_fcd filename, intf * rank, intf dimsizes[],
                 void * data, intf * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndsigslc
                (_fcd filename, intf winst[], intf windims[],
                 void * data, intf dims[], intf * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndsisslc
                (_fcd filename, intf * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndsirref
                (_fcd filename, intf * ref, intf * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndslref
                (void);

    HDFLIBAPI      FRETVAL(intf) ndsinum
                (_fcd filename, intf * len);

    HDFLIBAPI      FRETVAL(intf) ndsip32s
                (_fcd filename, intf * ref, intf * ispre32, intf * len);

    HDFLIBAPI      FRETVAL(intf) ndfsdgetdatastrs
                (_fcd label, _fcd unit, _fcd format, _fcd coordsys);

    HDFLIBAPI      FRETVAL(intf) ndfsdgetdimstrs
                (intf * dim, _fcd label, _fcd unit, _fcd format);

    HDFLIBAPI      FRETVAL(intf) ndfsdgetdimscale
                (intf * dim, intf * maxsize, void * scale);

    HDFLIBAPI      FRETVAL(intf) ndfsdgetrange
                (void * pmax, void * pmin);

    HDFLIBAPI      FRETVAL(intf) ndfsdsetdims
                (intf * rank, intf dimsizes[]);

    HDFLIBAPI      FRETVAL(intf) ndfsdsetdimscale
                (intf * dim, intf * dimsize, void * scale);

    HDFLIBAPI      FRETVAL(intf) ndfsdsetrange
                (void * max, void * min);

    HDFLIBAPI      FRETVAL(intf) ndfsdclear
                (void);

    HDFLIBAPI      FRETVAL(intf) ndfsdsetlengths
                (intf * maxlen_label, intf * maxlen_unit,
                 intf * maxlen_format, intf * maxlen_coordsys);

    HDFLIBAPI      FRETVAL(intf) ndfsdgetdimlen
                (intf * dim, intf * llabel, intf * lunit,
                 intf * lformat);

    HDFLIBAPI      FRETVAL(intf) ndfsdgetdatalen
                (intf * llabel, intf * lunit, intf * lformat,
                 intf * lcoordsys);

    HDFLIBAPI      FRETVAL(intf) ndfsdrestart
                (void);

    HDFLIBAPI      FRETVAL(intf) ndfsdputslice
                (intf windims[], void * data, intf dims[]);

    HDFLIBAPI      FRETVAL(intf) ndfsdendslice
                (void);

    HDFLIBAPI      FRETVAL(intf) ndfsdsetnt
                (intf * numbertype);

    HDFLIBAPI      FRETVAL(intf) ndfsdgetnt
                (intf * pnumbertype);

    HDFLIBAPI      FRETVAL(intf) ndfsdlastref
                (void);

    HDFLIBAPI      FRETVAL(intf) ndsisdis
                (intf * dim, _fcd flabel, _fcd funit, _fcd fformat,
             intf * llabel, intf * lunit, intf * lformat);

    HDFLIBAPI      FRETVAL(intf) ndsigdis
                (intf * dim, _fcd label, _fcd unit, _fcd format,
             intf * llabel, intf * lunit, intf * lformat);

    HDFLIBAPI      FRETVAL(intf) ndsisdas
                (_fcd flabel, _fcd funit, _fcd fformat, _fcd fcoordsys,
            intf * isfortran, intf * llabel, intf * lunit,
                 intf * lformat, intf * lcoordsys);

    HDFLIBAPI      FRETVAL(intf) ndsigdas
                (_fcd label, _fcd unit, _fcd format, _fcd coordsys, intf * llabel,
             intf * lunit, intf * lformat, intf * lcoord);

    HDFLIBAPI      FRETVAL(intf) ndsscal
                (float64 * cal, float64 * cal_err, float64 * ioff,
                 float64 * ioff_err, intf * cal_type);

    HDFLIBAPI      FRETVAL(intf) ndsgcal
                (float64 * cal, float64 * cal_err, float64 * ioff,
                 float64 * ioff_err, intf * cal_type);

    HDFLIBAPI      FRETVAL(intf) ndswref
                (_fcd filename, intf * fnlen, intf * ref);

    HDFLIBAPI      FRETVAL(intf) ndssfill
                (void * fill_value);

    HDFLIBAPI      FRETVAL(intf) ndsgfill
                (void * fill_value);

    HDFLIBAPI      FRETVAL(intf) ndssslab
                (_fcd filename, intf * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndswslab
                (intf start[], intf  stride[],
                 intf  cont[], void * data);

    HDFLIBAPI      FRETVAL(intf) ndseslab
                (void);

    HDFLIBAPI	FRETVAL(intf) ndsiwref
		(_fcd filename, intf * fnlen, intf * ref);

    HDFLIBAPI	FRETVAL(intf) ndsisslab
		(_fcd filename, intf * fnlen);

    HDFLIBAPI	FRETVAL(intf) ndsirslab
		(_fcd filename, intf * fnlen, intf start[], intf slab_size[],
	         intf stride[], void * buffer, intf buffer_size[]);

/*
   ** from dfpF.c
 */

#   define ndpigpal      H4_F77_FUNC(dpigpal, DPIGPAL)
#   define ndpippal      H4_F77_FUNC(dpippal, DPIPPAL)
#   define ndpinpal      H4_F77_FUNC(dpinpal, DPINPAL)
#   define ndpiwref      H4_F77_FUNC(dpiwref, DPIWREF)
#   define ndpirref      H4_F77_FUNC(dpirref, DPIRREF)
#   define ndprest       H4_F77_FUNC(dprest, DPREST)
#   define ndplref       H4_F77_FUNC(dplref, DPLREF)
#   define ndfprestart   H4_F77_FUNC(dfprestart, DFPRESTART)
#   define ndfplastref   H4_F77_FUNC(dfplastref, DFPLASTREF)

    HDFLIBAPI      FRETVAL(intf) ndpigpal
                (_fcd filename, _fcd pal, intf  * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndpippal
                (_fcd filename, _fcd pal, intf  * overwrite, _fcd filemode,
                 intf  * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndpinpal
                (_fcd filename, intf  * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndpirref
                (_fcd filename, intf  * ref, intf  * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndpiwref
                (_fcd filename, intf  * ref, intf  * fnlen);

    HDFLIBAPI      FRETVAL(intf) ndprest
                (void);

    HDFLIBAPI      FRETVAL(intf) ndplref
                (void);

    HDFLIBAPI      FRETVAL(intf) ndfprestart
                (void);

    HDFLIBAPI      FRETVAL(intf) ndfplastref
                (void);

/*
   ** from df24F.c
 */
#   define nd2reqil        H4_F77_FUNC(d2reqil, D2REQIL)
#   define ndf24reqil      H4_F77_FUNC(df24reqil, DF24REQIL)
#   define nd2sdims        H4_F77_FUNC(d2sdims, D2SDIMS)
#   define ndf24setdims    H4_F77_FUNC(df24setdims, DF24SETDIMS)
#   define nd2setil        H4_F77_FUNC(d2setil, D2SETIL)
#   define ndf24setil      H4_F77_FUNC(df24setil, DF24SETIL)
#   define nd2first        H4_F77_FUNC(d2first, D2FIRST)
#   define ndf24restart    H4_F77_FUNC(df24restart, DF24RESTART)
#   define nd2igdim        H4_F77_FUNC(d2igdim, D2IGDIM)
#   define nd2igimg        H4_F77_FUNC(d2igimg, D2IGIMG)
#   define nd2iaimg        H4_F77_FUNC(d2iaimg, D2IAIMG)
#   define nd2irref        H4_F77_FUNC(d2irref, D2IRREF)
#   define nd2inimg        H4_F77_FUNC(d2inimg, D2INIMG)
#   define nd2lref         H4_F77_FUNC(d2lref, D2LREF)
#   define nd2scomp        H4_F77_FUNC(d2scomp, D2SCOMP)
#   define ndf24scompress  H4_F77_FUNC(df24scompress, DF24SCOMPRESS)
#   define nd2sjpeg        H4_F77_FUNC(d2sjpeg, D2SJPEG)
#   define ndf24sjpeg      H4_F77_FUNC(df24sjpeg, DF24SJPEG)

    HDFLIBAPI      FRETVAL(intf) nd2reqil
                (intf  * il);

    HDFLIBAPI      FRETVAL(intf) nd2sdims
                (intf  * xdim, intf  * ydim);

    HDFLIBAPI      FRETVAL(intf) nd2igdim
                (_fcd filename, intf  * pxdim, intf  * pydim, intf  * pil,
                 intf  * fnlen);

    HDFLIBAPI      FRETVAL(intf) nd2igimg
                (_fcd filename, _fcd image, intf  * xdim, intf  * ydim,
                 intf  * fnlen);

    HDFLIBAPI      FRETVAL(intf) nd2iaimg
                (_fcd filename, _fcd image, intf  * xdim, intf  * ydim,
                 intf  * fnlen, intf  * newfile);

    HDFLIBAPI      FRETVAL(intf) nd2setil
                (intf  * il);

    HDFLIBAPI      FRETVAL(intf) nd2first
                (void);

    HDFLIBAPI      FRETVAL(intf) ndf24reqil
                (intf  * il);

    HDFLIBAPI      FRETVAL(intf) ndf24setdims
                (intf  * xdim, intf  * ydim);

    HDFLIBAPI      FRETVAL(intf) ndf24setil
                (intf  * il);

    HDFLIBAPI      FRETVAL(intf) ndf24restart
                (void);

    HDFLIBAPI      FRETVAL(intf) nd2irref
                (_fcd filename, intf  * ref, intf  * fnlen);

    HDFLIBAPI      FRETVAL(intf) nd2inimg
                (_fcd filename, intf  * fnlen);

    HDFLIBAPI      FRETVAL(intf) nd2lref
                (void);

    HDFLIBAPI      FRETVAL(intf) nd2scomp
                (intf * scheme);

    HDFLIBAPI      FRETVAL(intf) ndf24scompress
                (intf * scheme);

    HDFLIBAPI      FRETVAL(intf) nd2sjpeg
                (intf * quality, intf * force_baseline);

    HDFLIBAPI      FRETVAL(intf) ndf24sjpeg
                (intf * quality, intf * force_baseline);

/*
   ** from dfF.c
 */
#   define ndfiaccess H4_F77_FUNC(dfiaccess, DFIACCESS)
#   define ndfiopen   H4_F77_FUNC(dfiopen, DFIOPEN)
#   define ndfclose   H4_F77_FUNC(dfclose, DFCLOSE)
#   define ndfdesc    H4_F77_FUNC(dfdesc, DFDESC)
#   define ndfdup     H4_F77_FUNC(dfdup, DFDUP)
#   define ndfdel     H4_F77_FUNC(dfdel, DFDEL)
#   define ndfstart   H4_F77_FUNC(dfstart, DFSTART)
#   define ndfread    H4_F77_FUNC(dfread, DFREAD)
#   define ndfseek    H4_F77_FUNC(dfseek, DFSEEK)
#   define ndfwrite   H4_F77_FUNC(dfwrite, DFWRITE)
#   define ndfupdate  H4_F77_FUNC(dfupdate, DFUPDATE)
#   define ndfget     H4_F77_FUNC(dfget, DFGET)
#   define ndfput     H4_F77_FUNC(dfput, DFPUT)
#   define ndfsfind   H4_F77_FUNC(dfsfind, DFSFIND)
#   define ndffind    H4_F77_FUNC(dffind, DFFIND)
#   define ndferrno   H4_F77_FUNC(dferrno, DFERRNO)
#   define ndfnewref  H4_F77_FUNC(dfnewref, DFNEWREF)
#   define ndfnumber  H4_F77_FUNC(dfnumber, DFNUMBER)
#   define ndfstat    H4_F77_FUNC(dfstat, DFSTAT)
#   define ndfiishdf  H4_F77_FUNC(dfiishdf, DFIISHDF)

    HDFLIBAPI      FRETVAL(intf) ndfiopen
                (_fcd name, intf  * acc_mode, intf  * defdds, intf  * namelen);

    HDFLIBAPI      FRETVAL(intf) ndfclose
                (intf  * dfile);

    HDFLIBAPI      FRETVAL(intf) ndfdesc
                (intf  * dfile, intf  ptr[][4], intf  * begin,
                 intf  * num);

    HDFLIBAPI      FRETVAL(intf) ndfdup
                (intf  * dfile, intf  * tag, intf  * ref, intf  * otag,
                 intf  * oref);

    HDFLIBAPI      FRETVAL(intf) ndfdel
                (intf  * dfile, intf  * tag, intf  * ref);

    HDFLIBAPI      FRETVAL(intf) ndfiaccess
                (intf  * dfile, intf  * tag, intf  * ref, _fcd acc_mode, intf  * acclen);

    HDFLIBAPI      FRETVAL(intf) ndfstart
                (intf  * dfile, intf  * tag, intf  * ref, char  * acc_mode);

    HDFLIBAPI      FRETVAL(intf) ndfread
                (intf  * dfile, _fcd ptr, intf  * len);

    HDFLIBAPI      FRETVAL(intf) ndfseek
                (intf  * dfile, intf  * offset);

    HDFLIBAPI      FRETVAL(intf) ndfwrite
                (intf  * dfile, _fcd ptr, intf  * len);

    HDFLIBAPI      FRETVAL(intf) ndfupdate
                (intf  * dfile);

    HDFLIBAPI      FRETVAL(intf) ndfget
                (intf  * dfile, intf  * tag, intf  * ref, _fcd ptr);

    HDFLIBAPI      FRETVAL(intf) ndfput
                (intf  * dfile, intf  * tag, intf  * ref, _fcd ptr, intf  * len);

    HDFLIBAPI      FRETVAL(intf) ndfsfind
                (intf  * dfile, intf  * tag, intf  * ref);

    HDFLIBAPI      FRETVAL(intf) ndffind
                (intf  * dfile, intf  * itag, intf  * iref, intf  * len);

    HDFLIBAPI      FRETVAL(intf) ndferrno
                (void);

    HDFLIBAPI      FRETVAL(intf) ndfnewref
                (intf  * dfile);

    HDFLIBAPI      FRETVAL(intf) ndfnumber
                (intf  * dfile, intf  * tag);

    HDFLIBAPI      FRETVAL(intf) ndfiishdf
                (_fcd name, intf  * namelen);

/*
   ** from dfutil.c
 */
    HDFLIBAPI uint16 DFfindnextref
                (int32 file_id, uint16 tag, uint16 lref);

/*
   ** from dfutilF.c
 */
#   define ndfindnr          H4_F77_FUNC(dfindnr, DFINDNR)
#   define ndffindnextref    H4_F77_FUNC(dffindnextref, DFFINDNEXTREF)

    HDFLIBAPI      FRETVAL(intf) ndfindnr
                (intf  * dfile, intf  * tag, intf  * lref);

    HDFLIBAPI      FRETVAL(intf) ndffindnextref
                (intf  * dfile, intf  * tag, intf  * lref);

/*
   ** from herrF.c
 */
#   define nheprnt    H4_F77_FUNC(heprnt, HEPRNT)
#   define nheprntc   H4_F77_FUNC(heprntc, HEPRNTC)
#   define nhestringc H4_F77_FUNC(hestringc, HESTRINGC)

    HDFFCLIBAPI      FRETVAL(void) nheprnt
                (intf  * print_levels);

    HDFFCLIBAPI      FRETVAL(intf) nheprntc
                (_fcd filename, intf  * print_levels, intf *namelen);

    HDFFCLIBAPI      FRETVAL(intf) nhestringc
				(intf *error_code,_fcd error_message, intf *len);
/*
   ** from hfilef.c
*/
/* 
 *  Not all HDF4 Fortran functions are real Fortran functions, they are C wrappers called from a
 *  Fortran program. Since on Windows we have to have a real Fortran function, additional C wrappers
 *  were added at the time when Windows port was done for multi-file interfaces: for example, 
 *  hclose is a C wrapper on UNIX, while on Windows we have two functions: Fortran function hclose
 *  and C wrapper function hiclose called by hclose   
 *                                                                                    EIP 2007-09-14
 */
# ifndef _WIN32
#   define nhiopen       H4_F77_FUNC(hiopen, HIOPEN)
#   define nhclose       H4_F77_FUNC(hclose, HCLOSE)
#   define nhnumber      H4_F77_FUNC(hnumber, HNUMBER)
#   define nhxisdir      H4_F77_FUNC(hxisdir, HXISDIR)
#   define nhxiscdir     H4_F77_FUNC(hxiscdir, HXISCDIR)
#   define nhddontatexit H4_F77_FUNC(hddontatexit, HDDONTATEXIT)
#   define nhglibverc    H4_F77_FUNC(hglibverc, HGLIBVERC)
#   define nhgfilverc    H4_F77_FUNC(hgfilverc, HGFILVERC)
#   define nhiishdf      H4_F77_FUNC(hiishdf, HIISHDF)
#   define nhconfinfc    H4_F77_FUNC(hconfinfc, HCONFINFC)
#  else                                   /* !sl */
#   define nhiopen       H4_F77_FUNC(hiopen, HIOPEN)
#   define nhiclose      H4_F77_FUNC(hiclose, HICLOSE)
#   define nhinumbr      H4_F77_FUNC(hinumbr, HINUMBR)
#   define nhxisdir      H4_F77_FUNC(hxisdir, HXISDIR)
#   define nhxiscdir     H4_F77_FUNC(hxiscdir, HXISCDIR)
#   define nhddontatexit H4_F77_FUNC(hddontatexit, HDDONTATEXIT)
#   define nhglibverc    H4_F77_FUNC(hglibverc, HGLIBVERC)
#   define nhgfilverc    H4_F77_FUNC(hgfilverc, HGFILVERC)
#   define nhiishdf      H4_F77_FUNC(hiishdf, HIISHDF)
#   define nhconfinfc    H4_F77_FUNC(hconfinfc, HCONFINFC)
#  endif                                   /* !sl */


    HDFFCLIBAPI      FRETVAL(intf) nhiopen
                (_fcd name, intf  * acc_mode, intf  * defdds, intf  * namelen);

    HDFFCLIBAPI      FRETVAL(intf) nhclose
                (intf  * file_id);

    HDFLIBAPI	FRETVAL(intf) nhnumber
		(intf  * file_id, intf  * tag);

    HDFLIBAPI	FRETVAL(intf) nhxisdir
		(_fcd dir, intf * dirlen);

    HDFFCLIBAPI	FRETVAL(intf) nhxiscdir
		(_fcd dir, intf * dirlen);
 
    HDFFCLIBAPI      FRETVAL(intf)
                nhddontatexit(void);

    HDFFCLIBAPI      FRETVAL(intf)   
                nhglibverc(intf *major_v, intf *minor_v, intf *release,
                _fcd string, intf *len);

    HDFFCLIBAPI      FRETVAL(intf)
                nhgfilverc(intf *file_id, intf *major_v, intf *minor_v,
                intf *release, _fcd string, intf *len);

    HDFFCLIBAPI      FRETVAL(intf) nhiishdf
               (_fcd name, intf  * namelen);

    HDFFCLIBAPI FRETVAL(intf) nhiclose
               (intf  * file_id);
               
    HDFFCLIBAPI	FRETVAL(intf) nhinumbr
               (int32 file_id, uint16 tag);
    
    HDFFCLIBAPI	FRETVAL(intf) nhconfinfc
               (intf *coder_type, intf *info);
/*
   ** from dfufp2im.c
 */
#   define nduif2i       H4_F77_FUNC(duif2i, DUIF2I)

    HDFLIBAPI      FRETVAL(int) nduif2i
                (int32  * hdim, int32  * vdim, float32  * max,
        float32  * min, float32  hscale[], float32  vscale[],
                 float32  data[], _fcd palette, _fcd outfile,
              int  * ct_method, int32  * hres, int32  * vres,
                 int  * compress, int  * lenfn);

    HDFLIBAPI int  DFUfptoimage
                (int32 hdim, int32 vdim, float32 max, float32 min,
       float32  * hscale, float32  * vscale, float32  * data,
                 uint8  * palette, char  * outfile, int ct_method,
                 int32 hres, int32 vres, int compress);

/* for Multi-file fortran Annotation inteface */

/* 
 *  Not all HDF4 Fortran functions are real Fortran functions, they are C wrappers called from a
 *  Fortran program. Since on Windows we have to have a real Fortran function, additional C wrappers
 *  were added at the time when Windows port was done for multi-file interfaces: for example, 
 *  hclose is a C wrapper on UNIX, while on Windows we have two functions: Fortran function hclose
 *  and C wrapper function hiclose called by hclose   
 *                                                                                    EIP 2007-09-14
 */
# ifndef _WIN32
#  define nafstart      H4_F77_FUNC(afstart, AFSTART)
#  define naffileinfo   H4_F77_FUNC(affileinfo, AFFILEINFO)
#  define nafend        H4_F77_FUNC(afend, AFEND)
#  define nafcreate     H4_F77_FUNC(afcreate, AFCREATE)
#  define naffcreate    H4_F77_FUNC(affcreate, AFFCREATE)
#  define nafselect     H4_F77_FUNC(afselect, AFSELECT)
#  define nafnumann     H4_F77_FUNC(afnumann, AFNUMANN)
#  define nafannlist    H4_F77_FUNC(afannlist, AFANNLIST)
#  define nafannlen     H4_F77_FUNC(afannlen, AFANNLEN)
#  define nafwriteann   H4_F77_FUNC(afwriteann, AFWRITEANN)
#  define nafreadann    H4_F77_FUNC(afreadann, AFREADANN)
#  define nafendaccess  H4_F77_FUNC(afendaccess, AFENDACCESS)
#  define nafgettagref  H4_F77_FUNC(afgettagref, AFGETTAGREF)
#  define nafidtagref   H4_F77_FUNC(afidtagref, AFIDTAGREF)
#  define naftagrefid   H4_F77_FUNC(aftagrefid, AFTAGREFID)
#  define nafatypetag   H4_F77_FUNC(afatypetag, AFATYPETAG)
#  define naftagatype   H4_F77_FUNC(aftagatype, AFTAGATYPE)
# else                                     /* !sl */
#  define nafistart     H4_F77_FUNC(afistart, AFISTART)
#  define nafifinf      H4_F77_FUNC(afifinf, AFIFINF)
#  define nafiend       H4_F77_FUNC(afiend, AFIEND)
#  define naficreat     H4_F77_FUNC(aficreat, AFICREAT)
#  define nafifcreat    H4_F77_FUNC(afifcreat, AFIFCREAT)
#  define nafiselct     H4_F77_FUNC(afiselct, AFISELCT)
#  define nafinann      H4_F77_FUNC(afinann, AFINANN)
#  define nafialst      H4_F77_FUNC(afialst, AFIALST)
#  define nafialen      H4_F77_FUNC(afialen, AFIALEN)
#  define nafiwann      H4_F77_FUNC(afiwann, AFIWANN)
#  define nafirann      H4_F77_FUNC(afirann, AFIRANN)
#  define nafiendac     H4_F77_FUNC(afiendac, AFIENDAC)
#  define nafigtr       H4_F77_FUNC(afigtr, AFIGTR)
#  define nafiid2tr     H4_F77_FUNC(afiid2tr, AFIID2TR)
#  define nafitr2id     H4_F77_FUNC(afitr2id, AFITR2ID)
#  define nafitp2tg     H4_F77_FUNC(afitp2tg, AFITP2TG)
#  define nafitg2tp     H4_F77_FUNC(afitg2tp, AFITG2TP)
# endif                                     /* !sl */

/* Multi-file Annotation C-stubs for fortan interface found in mfanf.c */

HDFFCLIBAPI FRETVAL(intf)
nafstart(intf *file_id);

HDFFCLIBAPI FRETVAL(intf)
naffileinfo(intf *an_id, intf *num_flabel, intf *num_fdesc, intf *num_olabel,
            intf *num_odesc);

HDFFCLIBAPI FRETVAL(intf)
nafend(intf *an_id);

HDFFCLIBAPI FRETVAL(intf)
nafcreate(intf *an_id, intf *etag, intf *eref, intf *atype);

HDFFCLIBAPI FRETVAL(intf)
naffcreate(intf *an_id, intf *atype);

HDFFCLIBAPI FRETVAL(intf)
nafselect(intf *an_id, intf *idx, intf *atype);

HDFFCLIBAPI FRETVAL(intf)
nafnumann(intf *an_id, intf *atype, intf *etag, intf *eref);

HDFFCLIBAPI FRETVAL(intf)
nafannlist(intf *an_id, intf *atype, intf *etag, intf *eref, intf alist[]);

HDFFCLIBAPI FRETVAL(intf)
nafannlen(intf *ann_id);

HDFFCLIBAPI FRETVAL(intf)
nafwriteann(intf *ann_id,_fcd ann, intf *annlen);

HDFFCLIBAPI FRETVAL(intf)
nafreadann(intf *ann_id,_fcd ann, intf *maxlen);

HDFFCLIBAPI FRETVAL(intf)
nafendaccess(intf *ann_id);

HDFFCLIBAPI FRETVAL(intf)
nafgettagref(intf *an_id, intf *idx, intf *type, intf *tag, intf *ref);

HDFFCLIBAPI FRETVAL(intf)
nafidtagref(intf *ann_id, intf *tag, intf *ref);

HDFFCLIBAPI FRETVAL(intf)
naftagrefid(intf *an_id, intf *tag, intf *ref);

HDFFCLIBAPI FRETVAL(intf)
nafatypetag(intf *atype);

HDFFCLIBAPI FRETVAL(intf)
naftagatype(intf *tag);

/* if defined Windows */
/* Multi-file Annotation C-stubs for fortan interface found in mfanf.c */

HDFFCLIBAPI FRETVAL(intf)
nafistart(intf *file_id);

HDFFCLIBAPI FRETVAL(intf)
nafifinf(intf *an_id, intf *num_flabel, intf *num_fdesc, intf *num_olabel,
            intf *num_odesc);

HDFFCLIBAPI FRETVAL(intf)
nafiend(intf *an_id);

HDFFCLIBAPI FRETVAL(intf)
naficreat(intf *an_id, intf *etag, intf *eref, intf *atype);

HDFFCLIBAPI FRETVAL(intf)
nafifcreat(intf *an_id, intf *atype);

HDFFCLIBAPI FRETVAL(intf)
nafiselct(intf *an_id, intf *index, intf *atype);

HDFFCLIBAPI FRETVAL(intf)
nafinann(intf *an_id, intf *atype, intf *etag, intf *eref);

HDFFCLIBAPI FRETVAL(intf)
nafialst(intf *an_id, intf *atype, intf *etag, intf *eref, intf alist[]);

HDFFCLIBAPI FRETVAL(intf)
nafialen(intf *ann_id);

HDFFCLIBAPI FRETVAL(intf)
nafiwann(intf *ann_id,_fcd ann, intf *annlen);

HDFFCLIBAPI FRETVAL(intf)
nafirann(intf *ann_id,_fcd ann, intf *maxlen);

HDFFCLIBAPI FRETVAL(intf)
nafiendac(intf *ann_id);

HDFFCLIBAPI FRETVAL(intf)
nafigtr(intf *an_id, intf *index, intf *type, intf *tag, intf *ref);

HDFFCLIBAPI FRETVAL(intf)
nafiid2tr(intf *ann_id, intf *tag, intf *ref);

HDFFCLIBAPI FRETVAL(intf)
nafitr2id(intf *an_id, intf *tag, intf *ref);

HDFFCLIBAPI FRETVAL(intf)
nafitp2tg(intf *atype);

HDFFCLIBAPI FRETVAL(intf)
nafitg2tp(intf *tag);

/* endif defined Windows */


/* Multi-file Annotation C-routines found in mfan.c */
HDFLIBAPI int32 ANstart(int32 file_id);

HDFLIBAPI intn  ANfileinfo(int32 an_id, int32 *n_file_label, int32 *n_file_desc,
                        int32 *n_obj_label, int32 *n_obj_desc);

HDFLIBAPI int32 ANend(int32 an_id);

HDFLIBAPI int32 ANcreate(int32 an_id, uint16 elem_tag, uint16 elem_ref, 
                      ann_type type);

HDFLIBAPI int32 ANcreatef(int32 an_id, ann_type type);

HDFLIBAPI int32 ANselect(int32 an_id, int32 idx, ann_type type);

HDFLIBAPI intn  ANnumann(int32 an_id, ann_type type, uint16 elem_tag, 
                      uint16 elem_ref);

HDFLIBAPI intn  ANannlist(int32 an_id, ann_type type, uint16 elem_tag, 
                       uint16 elem_ref, int32 ann_list[]);

HDFLIBAPI int32 ANannlen(int32 ann_id);

HDFLIBAPI int32 ANwriteann(int32 ann_id, const char *ann, int32 annlen);

HDFLIBAPI int32 ANreadann(int32 ann_id, char *ann, int32 maxlen);

HDFLIBAPI intn  ANendaccess(int32 ann_id);

HDFLIBAPI int32 ANget_tagref(int32 an_id, int32 idx, ann_type type,
                          uint16 *ann_tag, uint16 *ann_ref);

HDFLIBAPI int32 ANid2tagref(int32 an_id, uint16 *ann_tag, uint16 *ann_ref);

HDFLIBAPI int32 ANtagref2id(int32 an_id, uint16 ann_tag, uint16 ann_ref);

HDFLIBAPI uint16 ANatype2tag(ann_type atype);

HDFLIBAPI ann_type ANtag2atype(uint16 atag);

HDFLIBAPI intn ANdestroy(void);

/* for Multi-file fortran GR interface */
/* 
 *  Not all HDF4 Fortran functions are real Fortran functions, they are C wrappers called from a
 *  Fortran program. Since on Windows we have to have a real Fortran function, additional C wrappers
 *  were added at the time when Windows port was done for multi-file interfaces: for example, 
 *  hclose is a C wrapper on UNIX, while on Windows we have two functions: Fortran function hclose
 *  and C wrapper function hiclose called by hclose   
 *                                                                                    EIP 2007-09-14
 */
# ifndef _WIN32
#  define nmgstart         H4_F77_FUNC(mgstart, MGSTART)
#  define nmgfinfo         H4_F77_FUNC(mgfinfo, MGFINFO)
#  define nmgend           H4_F77_FUNC(mgend, MGEND)
#  define nmgicreat        H4_F77_FUNC(mgicreat, MGICREAT)
#  define nmgselct         H4_F77_FUNC(mgselct, MGSELCT)
#  define nmgin2ndx        H4_F77_FUNC(mgin2ndx, MGIN2NDX)
#  define nmggiinf         H4_F77_FUNC(mggiinf, MGGIINF)
#  define nmgwcimg         H4_F77_FUNC(mgwcimg, MGWCIMG)
#  define nmgrcimg         H4_F77_FUNC(mgrcimg, MGRCIMG)
#  define nmgwrimg         H4_F77_FUNC(mgwrimg, MGWRIMG)
#  define nmgrdimg         H4_F77_FUNC(mgrdimg, MGRDIMG)
#  define nmgendac         H4_F77_FUNC(mgendac, MGENDAC)
#  define nmgid2rf         H4_F77_FUNC(mgid2rf, MGID2RF)
#  define nmgr2idx         H4_F77_FUNC(mgr2idx, MGR2IDX)
#  define nmgrltil         H4_F77_FUNC(mgrltil, MGRLTIL)
#  define nmgrimil         H4_F77_FUNC(mgrimil, MGRIMIL)
#  define nmggltid         H4_F77_FUNC(mggltid, MGGLTID)
#  define nmgglinf         H4_F77_FUNC(mgglinf, MGGLINF)
#  define nmgwrlut         H4_F77_FUNC(mgwrlut, MGWRLUT)
#  define nmgwclut         H4_F77_FUNC(mgwclut, MGWCLUT)
#  define nmgrdlut         H4_F77_FUNC(mgrdlut, MGRDLUT)
#  define nmgrclut         H4_F77_FUNC(mgrclut, MGRCLUT)
#  define nmgisxfil        H4_F77_FUNC(mgisxfil, MGISXFIL)
#  define nmgiscatt        H4_F77_FUNC(mgiscatt, MGISCATT)
#  define nmgisattr        H4_F77_FUNC(mgisattr, MGISATTR)
#  define nmgatinf         H4_F77_FUNC(mgatinf, MGATINF)
#  define nmggcatt         H4_F77_FUNC(mggcatt, MGGCATT)
#  define nmggnatt         H4_F77_FUNC(mggnatt, MGGNATT)
#  define nmggattr         H4_F77_FUNC(mggattr, MGGATTR)
#  define nmgifndat        H4_F77_FUNC(mgifndat, MGIFNDAT)
#  define nmgcgichnk        H4_F77_FUNC(mgcgichnk, MGCGICHNK)
#  define nmgcrcchnk        H4_F77_FUNC(mgcrcchnk, MGCRCCHNK)
#  define nmgcrchnk         H4_F77_FUNC(mgcrchnk, MGCRCHNK)
#  define nmgcscchnk        H4_F77_FUNC(mgcscchnk, MGCSCCHNK)
#  define nmgcschnk         H4_F77_FUNC(mgcschnk, MGCSCHNK)
#  define nmgcwcchnk        H4_F77_FUNC(mgcwcchnk, MGCWCCHNK)
#  define nmgcwchnk         H4_F77_FUNC(mgcwchnk, MGCWCHNK)
#  define nmgcscompress     H4_F77_FUNC(mgcscompress, MGCSCOMPRESS)
#  define nmgcgcompress     H4_F77_FUNC(mgcgcompress, MGCGCOMPRESS)
#  define nmglt2rf          H4_F77_FUNC(mglt2rf, MGLT2RF)
#  define nmgcgnluts        H4_F77_FUNC(mgcgnluts, MGCGNLUTS)
# else  /* !sl */
#  define nmgistrt          H4_F77_FUNC(mgistrt, MGISTRT)
#  define nmgifinf          H4_F77_FUNC(mgifinf, MGIFINF)
#  define nmgiend           H4_F77_FUNC(mgiend, MGIEND)
#  define nmgicreat         H4_F77_FUNC(mgicreat, MGICREAT)
#  define nmgislct          H4_F77_FUNC(mgislct, MGISLCT)
#  define nmgin2ndx         H4_F77_FUNC(mgin2ndx, MGIN2NDX)
#  define nmgigiinf         H4_F77_FUNC(mgigiinf, MGIGIINF)
#  define nmgiwcim          H4_F77_FUNC(mgiwcim, MGIWCIM)
#  define nmgircim          H4_F77_FUNC(mgircim, MGIRCIM)
#  define nmgiwimg          H4_F77_FUNC(mgiwim, MGIWIMG)
#  define nmgirimg          H4_F77_FUNC(mgirimg, MGIRIMG)
#  define nmgiendac         H4_F77_FUNC(mgiendac, MGIENDAC)
#  define nmgiid2r          H4_F77_FUNC(mgiid2r, MGIID2R)
#  define nmgir2dx          H4_F77_FUNC(mgir2dx, MGIR2DX)
#  define nmgiltil          H4_F77_FUNC(mgiltil, MGILTIL)
#  define nmgiimil          H4_F77_FUNC(mgiimil, MGIIMIL)
#  define nmgiglid          H4_F77_FUNC(mgiglid, MGIGLID)
#  define nmgiglinf         H4_F77_FUNC(mgiglinf, MGIGLINF)
#  define nmgiwrlt          H4_F77_FUNC(mgiwrlt, MGIWRLT)
#  define nmgiwclt          H4_F77_FUNC(mgiwclt, MGIWCLT)
#  define nmgirdlt          H4_F77_FUNC(mgirdlt, MGIRDLT)
#  define nmgirclt          H4_F77_FUNC(mgirclt, MGIRCLT)
#  define nmgisxfil         H4_F77_FUNC(mgisxfil, MGISXFIL)
#  define nmgiactp          H4_F77_FUNC(mgiactp, MGIACTP)
#  define nmgiscatt         H4_F77_FUNC(mgiscatt, MGISCATT)
#  define nmgisattr         H4_F77_FUNC(mgisattr, MGISATTR)
#  define nmgiainf          H4_F77_FUNC(mgiainf, MGIAINF)
#  define nmgigcat          H4_F77_FUNC(mgigcat, MGIGCAT)
#  define nmgignat          H4_F77_FUNC(mgignat, MGIGNAT)
#  define nmgigatt          H4_F77_FUNC(mgigatt, MGIGATT)
#  define nmgifndat         H4_F77_FUNC(mgifndat, MGIFNDAT)
#  define nmgcgichnk        H4_F77_FUNC(mgcgichnk, MGCGICHNK)
#  define nmgcrcchnk        H4_F77_FUNC(mgcrcchnk, MGCRCCHNK)
#  define nmgcrchnk         H4_F77_FUNC(mgcrchnk, MGCRCHNK)
#  define nmgcscchnk        H4_F77_FUNC(mgcscchnk, MGCSCCHNK)
#  define nmgcschnk         H4_F77_FUNC(mgcschnk, MGCSCHNK)
#  define nmgcwcchnk        H4_F77_FUNC(mgcwcchnk, MGCWCCHNK)
#  define nmgcwchnk         H4_F77_FUNC(mgcwchnk, MGCWCHNK)
#  define nmgcscompress     H4_F77_FUNC(mgcscompress, MGCSCOMPRESS)
#  define nmgcgcompress     H4_F77_FUNC(mgcgcompress, MGCGCOMPRESS)
#  define nmglt2rf          H4_F77_FUNC(mglt2rf, MGLT2RF)
#  define nmgcgnluts        H4_F77_FUNC(mgcgnluts, MGCGNLUTS)
# endif  /* sl */

/* Multi-file GR C-stubs for FORTRAN interface found in mfgrf.c */

HDFFCLIBAPI FRETVAL(intf)     /* !sl */
nmgiwimg(intf * riid, intf *start, intf *stride, intf *count, VOIDP data);

HDFFCLIBAPI FRETVAL(intf)     /* !sl */
nmgirimg(intf * riid, intf *start, intf *stride, intf *count, VOIDP data);

HDFFCLIBAPI FRETVAL(intf)     /* !sl */
nmgignat(intf * riid, intf *idx, VOIDP data);

HDFLIBAPI FRETVAL(intf)
nmgstart(intf * fid);

HDFLIBAPI FRETVAL(intf)
nmgfinfo(intf * grid,intf *n_datasets,intf *n_attrs);

HDFLIBAPI FRETVAL(intf)
nmgend(intf * grid);

HDFLIBAPI FRETVAL(intf)
nmgicreat(intf * grid, _fcd name, intf *ncomp, intf *nt, intf *il, intf dimsizes[2], intf *nlen);

HDFLIBAPI FRETVAL(intf)
nmgselct(intf * grid, intf *idx);

HDFLIBAPI FRETVAL(intf)
nmgin2ndx(intf * grid, _fcd name, intf *nlen);

HDFLIBAPI FRETVAL(intf)
nmggiinf(intf * riid, _fcd name, intf *ncomp, intf *nt, intf *il, intf *dimsizes, intf *nattr);


HDFLIBAPI FRETVAL(intf)
nmgwcimg(intf * riid, intf *start, intf *stride, intf *count, _fcd data);

HDFLIBAPI FRETVAL(intf)
nmgrcimg(intf * riid, intf *start, intf *stride, intf *count, _fcd data);

HDFLIBAPI FRETVAL(intf)
nmgwrimg(intf * riid, intf *start, intf *stride, intf *count, void * data);

HDFLIBAPI FRETVAL(intf)
nmgrdimg(intf * riid, intf *start, intf *stride, intf *count, void * data);

HDFLIBAPI FRETVAL(intf)
nmgendac(intf * riid);

HDFLIBAPI FRETVAL(intf)
nmgid2rf(intf * riid);

HDFLIBAPI FRETVAL(intf)
nmgr2idx(intf * grid, intf *ref);

HDFLIBAPI FRETVAL(intf)
nmgrltil(intf * riid, intf *il);

HDFLIBAPI FRETVAL(intf)
nmgrimil(intf * riid, intf *il);

HDFLIBAPI FRETVAL(intf)
nmggltid(intf * riid, intf *lut_index);

HDFLIBAPI FRETVAL(intf)
nmgglinf(intf * lutid, intf *ncomp, intf *nt, intf *il, intf *nentries);

HDFLIBAPI FRETVAL(intf)
nmgwrlut(intf * lutid, intf *ncomp, intf *nt, intf *il, intf *nentries, void * data);

HDFLIBAPI FRETVAL(intf)
nmgwclut(intf * lutid, intf *ncomp, intf *nt, intf *il, intf *nentries, _fcd data);

HDFLIBAPI FRETVAL(intf)
nmgrdlut(intf * lutid, void * data);

HDFLIBAPI FRETVAL(intf)
nmgrclut(intf * lutid, _fcd data);

HDFLIBAPI FRETVAL(intf)
nmgisxfil(intf * riid, _fcd filename, intf *offset, intf *nlen);

HDFLIBAPI FRETVAL(intf)
nmgsactp(intf * riid, intf *accesstype);

HDFLIBAPI FRETVAL(intf)
nmgiscatt(intf * riid, _fcd name, intf *nt, intf *count, _fcd data, intf *nlen);

HDFLIBAPI FRETVAL(intf)
nmgisattr(intf * riid, _fcd name, intf *nt, intf *count, void * data, intf *nlen);

HDFLIBAPI FRETVAL(intf)
nmgatinf(intf * riid, intf *idx, _fcd name, intf *nt, intf *count);

HDFLIBAPI FRETVAL(intf)
nmggcatt(intf * riid, intf *idx, _fcd data);

HDFLIBAPI FRETVAL(intf)
nmggnatt(intf * riid, intf *idx, void * data);

HDFLIBAPI FRETVAL(intf)
nmggattr(intf * riid, intf *idx, void * data);

HDFLIBAPI FRETVAL(intf)
nmgifndat(intf * riid, _fcd name, intf *nlen);

     HDFLIBAPI FRETVAL (intf)
     nmgcgichnk(intf *id, intf *dim_length, intf *flags);

    HDFLIBAPI FRETVAL (intf)
       nmgcrcchnk(intf *id, intf *start, _fcd char_data);

    HDFLIBAPI FRETVAL (intf)
       nmgcrchnk(intf *id, intf *start, VOIDP num_data);

    HDFLIBAPI FRETVAL (intf)
       nmgcscchnk(intf *id, intf *maxcache, intf *flags);

    HDFLIBAPI FRETVAL (intf)
      nmgcschnk(intf *id, intf *dim_length, intf *comp_type,
                intf *comp_prm);
    HDFLIBAPI FRETVAL (intf)
       nmgcwcchnk(intf *id, intf *start, _fcd char_data);

    HDFLIBAPI FRETVAL (intf)
       nmgcwchnk(intf *id, intf *start, VOIDP num_data);

    HDFLIBAPI FRETVAL (intf)
       nmgcscompress(intf *id, intf *comp_type, intf *comp_prm);

    HDFLIBAPI FRETVAL (intf)
       nmgcgcompress(intf *id, intf *comp_type, intf *comp_prm);

    HDFLIBAPI FRETVAL (intf)
       nmglt2rf(intf *id);

    HDFLIBAPI FRETVAL (intf)
       nmgcgnluts(intf *id);

#ifdef _WIN32
/* Multi-file GR C-stubs for FORTRAN interface found in mfgrf.c */
 HDFFCLIBAPI FRETVAL(intf)
nmgistrt(intf * fid);

 HDFFCLIBAPI FRETVAL(intf)
nmgifinf(intf * grid,intf *n_datasets,intf *n_attrs);

 HDFFCLIBAPI FRETVAL(intf)
nmgiend(intf * grid);

 HDFFCLIBAPI FRETVAL(intf)
nmgislct(intf * grid, intf *index);

 HDFFCLIBAPI FRETVAL(intf)
nmgigiinf(intf * riid, _fcd name, intf *ncomp, intf *nt, intf *il, intf *dimsizes, intf *nattr);

 HDFFCLIBAPI FRETVAL(intf)
nmgiwcim(intf * riid, intf *start, intf *stride, intf *count, _fcd data);

 HDFFCLIBAPI FRETVAL(intf)
nmgircim(intf * riid, intf *start, intf *stride, intf *count, _fcd data);

 HDFFCLIBAPI FRETVAL(intf)
nmgiendac(intf * riid);

 HDFFCLIBAPI FRETVAL(intf)
nmgiid2r(intf * riid);

 HDFFCLIBAPI FRETVAL(intf)
nmgir2dx(intf * grid, intf *ref);

 HDFFCLIBAPI FRETVAL(intf)
nmgiltil(intf * riid, intf *il);

 HDFFCLIBAPI FRETVAL(intf)
nmgiimil(intf * riid, intf *il);

 HDFFCLIBAPI FRETVAL(intf)
nmgiglid(intf * riid, intf *lut_index);

 HDFFCLIBAPI FRETVAL(intf)
nmgiglinf(intf * lutid, intf *ncomp, intf *nt, intf *il, intf *nentries);

 HDFFCLIBAPI FRETVAL(intf)
nmgiwrlt(intf * lutid, intf *ncomp, intf *nt, intf *il, intf *nentries, VOIDP data);

 HDFFCLIBAPI FRETVAL(intf)
nmgiwclt(intf * lutid, intf *ncomp, intf *nt, intf *il, intf *nentries, _fcd data);

 HDFFCLIBAPI FRETVAL(intf)
nmgirdlt(intf * lutid, VOIDP data);

 HDFFCLIBAPI FRETVAL(intf)
nmgirclt(intf * lutid, _fcd data);

 HDFFCLIBAPI FRETVAL(intf)
nmgiactp(intf * riid, intf *accesstype);

 HDFFCLIBAPI FRETVAL(intf)
nmgiainf(intf * riid, intf *index, _fcd name, intf *nt, intf *count);

 HDFFCLIBAPI FRETVAL(intf)
nmgigcat(intf * riid, intf *index, _fcd data);

 HDFFCLIBAPI FRETVAL(intf)
nmgigatt(intf * riid, intf *index, VOIDP data);

#endif /* _WIN32 */

/*
   ** from vgF.c
 */
/* 
 *  Not all HDF4 Fortran functions are real Fortran functions, they are C wrappers called from a
 *  Fortran program. Since on Windows we have to have a real Fortran function, additional C wrappers
 *  were added at the time when Windows port was done for multi-file interfaces: for example, 
 *  hclose is a C wrapper on UNIX, while on Windows we have two functions: Fortran function hclose
 *  and C wrapper function hiclose called by hclose   
 *                                                                                    EIP 2007-09-14
 */
# ifndef _WIN32
#   define  ndfivopn       H4_F77_FUNC(dfivopn, DFIVOPN)
#   define  ndfvclos       H4_F77_FUNC(dfvclos, DFVCLOS)
#   define  nvatchc        H4_F77_FUNC(vatchc, VATCHC)
#   define  nvdtchc        H4_F77_FUNC(vdtchc, VDTCHC)
#   define  nvgnamc        H4_F77_FUNC(vgnamc, VGNAMC)
#   define  nvgclsc        H4_F77_FUNC(vgclsc, VGCLSC)
#   define  nvinqc         H4_F77_FUNC(vinqc, VINQC)
#   define  nvdeletec      H4_F77_FUNC(vdeletec, VDELETEC)
#   define  nvgidc         H4_F77_FUNC(vgidc, VGIDC)
#   define  nvgnxtc        H4_F77_FUNC(vgnxtc, VGNXTC)
#   define  nvsnamc        H4_F77_FUNC(vsnamc, VSNAMC)
#   define  nvsclsc        H4_F77_FUNC(vsclsc, VSCLSC)
#   define  nvinsrtc       H4_F77_FUNC(vinsrtc, VINSRTC)
#   define  nvisvgc        H4_F77_FUNC(visvgc, VISVGC)
#   define  nvisvsc        H4_F77_FUNC(visvsc, VISVSC)
#   define  nvsatchc       H4_F77_FUNC(vsatchc, VSATCHC)
#   define  nvsdtchc       H4_F77_FUNC(vsdtchc, VSDTCHC)
#   define  nvsqref        H4_F77_FUNC(vsqref, VSQREF)
#   define  nvsqtag        H4_F77_FUNC(vsqtag, VSQTAG)
#   define  nvsgver        H4_F77_FUNC(vsgver, VSGVER)
#   define  nvsseekc       H4_F77_FUNC(vsseekc, VSSEEKC)
#   define  nvsgnamc       H4_F77_FUNC(vsgnamc, VSGNAMC)
#   define  nvsgclsc       H4_F77_FUNC(vsgclsc, VSGCLSC)
#   define  nvsinqc        H4_F77_FUNC(vsinqc, VSINQC)
#   define  nvsfexc        H4_F77_FUNC(vsfexc, VSFEXC)
#   define  nvsfndc        H4_F77_FUNC(vsfndc, VSFNDC)
#   define  nvsgidc        H4_F77_FUNC(vsgidc, VSGIDC)
#   define  nvsdltc        H4_F77_FUNC(vsdltc, VSDLTC)
#   define  nvsapp         H4_F77_FUNC(vsapp, VSAPP)
#   define  nvssnamc       H4_F77_FUNC(vssnamc, VSSNAMC)
#   define  nvssclsc       H4_F77_FUNC(vssclsc, VSSCLSC)
#   define  nvssfldc       H4_F77_FUNC(vssfldc, VSSFLDC)
#   define  nvssintc       H4_F77_FUNC(vssintc, VSSINTC)
#   define  nvsfdefc       H4_F77_FUNC(vsfdefc, VSFDEFC)
#   define  nvssextfc      H4_F77_FUNC(vssextfc, VSSEXTFC)
#   define  nvfnflds       H4_F77_FUNC(vfnflds, VFNFLDS)
#   define  nvffnamec      H4_F77_FUNC(vffnamec, VFFNAMEC)
#   define  nvfftype       H4_F77_FUNC(vfftype, VFFTYPE)
#   define  nvffisiz       H4_F77_FUNC(vffisiz, VFFISIZ)
#   define  nvffesiz       H4_F77_FUNC(vffesiz, VFFESIZ)
#   define  nvffordr       H4_F77_FUNC(vffordr, VFFORDR)
#   define  nvsfrdc        H4_F77_FUNC(vsfrdc, VSFRDC)
#   define  nvsfrd         H4_F77_FUNC(vsfrd, VSFRD)
#   define  nvsreadc       H4_F77_FUNC(vsreadc, VSREADC)
#   define  nvsfwrt        H4_F77_FUNC(vsfwrt, VSFWRT)
#   define  nvsfwrtc       H4_F77_FUNC(vsfwrtc, VSFWRTC)
#   define  nvswritc       H4_F77_FUNC(vswritc, VSWRITC)
#   define  nvsgintc       H4_F77_FUNC(vsgintc, VSGINTC)
#   define  nvseltsc       H4_F77_FUNC(vseltsc, VSELTSC)
#   define  nvsgfldc       H4_F77_FUNC(vsgfldc, VSGFLDC)
#   define  nvssizc        H4_F77_FUNC(vssizc, VSSIZC)
#   define  nventsc        H4_F77_FUNC(ventsc, VENTSC)
#   define  nvlonec        H4_F77_FUNC(vlonec, VLONEC)
#   define  nvslonec       H4_F77_FUNC(vslonec, VSLONEC)
#   define  nvfindc        H4_F77_FUNC(vfindc, VFINDC)
#   define  nvfndclsc      H4_F77_FUNC(vfndclsc, VFNDCLSC)
#   define  nvhscdc        H4_F77_FUNC(vhscdc, VHSCDC)
#   define  nvhsdc         H4_F77_FUNC(vhsdc, VHSDC)
#   define  nvhscdmc       H4_F77_FUNC(vhscdmc, VHSCDMC)
#   define  nvhsdmc        H4_F77_FUNC(vhsdmc, VHSDMC)
#   define  nvhmkgpc       H4_F77_FUNC(vhmkgpc, VHMKGPC)
#   define  nvflocc        H4_F77_FUNC(vflocc, VFLOCC)
#   define  nvinqtrc       H4_F77_FUNC(vinqtrc, VINQTRC)
#   define  nvntrc         H4_F77_FUNC(vntrc, VNTRC)
#   define  nvnrefs        H4_F77_FUNC(vnrefs, VNREFS)
#   define  nvgttrsc       H4_F77_FUNC(vgttrsc, VGTTRSC)
#   define  nvqref         H4_F77_FUNC(vqref, VQREF)
#   define  nvqtag         H4_F77_FUNC(vqtag, VQTAG)
#   define  nvgttrc        H4_F77_FUNC(vgttrc, VGTTRC)
#   define  nvadtrc        H4_F77_FUNC(vadtrc, VADTRC)
#   define  nvfstart       H4_F77_FUNC(vfstart, VFSTART)
#   define  nvfend         H4_F77_FUNC(vfend, VFEND)
#   define  nvsqfnelt      H4_F77_FUNC(vsqfnelt, VSQFNELT)
#   define  nvsqfintr      H4_F77_FUNC(vsqfintr, VSQFINTR)
#   define  nvsqfldsc      H4_F77_FUNC(vsqfldsc, VSQFLDSC)
#   define  nvsqfvsiz      H4_F77_FUNC(vsqfvsiz, VSQFVSIZ)
#   define  nvsqnamec      H4_F77_FUNC(vsqnamec, VSQNAMEC)
#   define  nvsfccpk       H4_F77_FUNC(vsfccpk, VSFCCPK)
#   define  nvsfncpk       H4_F77_FUNC(vsfncpk, VSFNCPK)
#   define  nvdtrc         H4_F77_FUNC(vdtrc, VDTRC)
#   define  nvscfcls       H4_F77_FUNC(vscfcls, VSCFCLS)
#   define  nvscsetblsz    H4_F77_FUNC(vscsetblsz, VSCSETBLSZ)
#   define  nvscsetnmbl    H4_F77_FUNC(vscsetnmbl, VSCSETNMBL)
#   define  nvscgblinfo    H4_F77_FUNC(vscgblinfo, VSCGBLINFO)
#  else                                /* !sl */
#   define  ndfivopn       H4_F77_FUNC(dfivopn, DFIVOPN)
#   define  ndfvclos       H4_F77_FUNC(dfvclos, DFVCLOS)
#   define  nvatchc        H4_F77_FUNC(vatchc, VATCHC)
#   define  nvdtchc        H4_F77_FUNC(vdtchc, VDTCHC)
#   define  nvgnamc        H4_F77_FUNC(vgnamc, VGNAMC)
#   define  nvgclsc        H4_F77_FUNC(vgclsc, VGCLSC)
#   define  nvinqc         H4_F77_FUNC(vinqc, VINQC)
#   define  nvdeletec      H4_F77_FUNC(vdeletec, VDELETEC)
#   define  nvgidc         H4_F77_FUNC(vgidc, VGIDC)
#   define  nvgnxtc        H4_F77_FUNC(vgnxtc, VGNXTC)
#   define  nvsnamc        H4_F77_FUNC(vsnamc, VSNAMC)
#   define  nvsclsc        H4_F77_FUNC(vsclsc, VSCLSC)
#   define  nvinsrtc       H4_F77_FUNC(vinsrtc, VINSRTC)
#   define  nvisvgc        H4_F77_FUNC(visvgc, VISVGC)
#   define  nvfistart      H4_F77_FUNC(vfistart, VFISTART)
#   define  nvfiend        H4_F77_FUNC(vfiend, VFIEND)
#   define  nvisvsc        H4_F77_FUNC(visvsc, VISVSC)
#   define  nvsatchc       H4_F77_FUNC(vsatchc, VSATCHC)
#   define  nvsdtchc       H4_F77_FUNC(vsdtchc, VSDTCHC)
#   define  nvsiqref       H4_F77_FUNC(vsiqref, VSIQREF)
#   define  nvsiqtag       H4_F77_FUNC(vsiqtag, VSIQTAG)
#   define  nvsigver       H4_F77_FUNC(vsigver, VSIGVER)
#   define  nvsseekc       H4_F77_FUNC(vsseekc, VSSEEKC)
#   define  nvsgnamc       H4_F77_FUNC(vsgnamc, VSGNAMC)
#   define  nvsgclsc       H4_F77_FUNC(vsglcsc, VSGCLSC)
#   define  nvsinqc        H4_F77_FUNC(vsinqc, VSINQC)
#   define  nvsfexc        H4_F77_FUNC(vsfexc, VSFEXC)
#   define  nvsfndc        H4_F77_FUNC(vsfnd, VSFNDC)
#   define  nvsgidc        H4_F77_FUNC(vsgidc, VSGIDC)
#   define  nvsdltc        H4_F77_FUNC(vsdltc, VSDLTC)
#   define  nvsapp         H4_F77_FUNC(vsapp, VSAPP)
#   define  nvssnamc       H4_F77_FUNC(vssnamc, VSSNAMC)
#   define  nvssclsc       H4_F77_FUNC(vssclsc, VSSCLSC)
#   define  nvssfldc       H4_F77_FUNC(vssfldc, VSSFLDC)
#   define  nvssintc       H4_F77_FUNC(vssintc, VSSINTC)
#   define  nvsfdefc       H4_F77_FUNC(vsfdefc, VSFDEFC)
#   define  nvssextfc      H4_F77_FUNC(vssextfc, VSSEXTFC)
#   define  nvfinflds      H4_F77_FUNC(vfinflds, VFINFLDS)
#   define  nvfifnm        H4_F77_FUNC(vfifnm, VFIFNM)
#   define  nvfiftp        H4_F77_FUNC(vfiftp, VFIFTP)
#   define  nvfifisz       H4_F77_FUNC(vfifisz, VFIFISZ)
#   define  nvfifesz       H4_F77_FUNC(vfifesz, VFIFESZ)
#   define  nvfifodr       H4_F77_FUNC(vfiford, VFIFODR)
#   define  nvsfirdc       H4_F77_FUNC(vsfirdc, VSFIRDC)
#   define  nvsfird        H4_F77_FUNC(vsfird, VSFIRD)
#   define  nvsreadc       H4_F77_FUNC(vsreadc, VSREADC)
#   define  nvsfiwrc       H4_F77_FUNC(vsfiwrc, VSFIWRC)
#   define  nvsfiwr        H4_F77_FUNC(vsfiwr, VSFIWR)
#   define  nvswritc       H4_F77_FUNC(vswritc, VSWRITC)
#   define  nvsgintc       H4_F77_FUNC(vsgintc, VSGINTC)
#   define  nvseltsc       H4_F77_FUNC(vseltsc, VSELTSC)
#   define  nvsgfldc       H4_F77_FUNC(vsgfldc, VSGFLDC)
#   define  nvssizc        H4_F77_FUNC(vssizc, VSSIZC)
#   define  nventsc        H4_F77_FUNC(ventsc, VENTSC)
#   define  nvlonec        H4_F77_FUNC(vlonec, VLONEC)
#   define  nvslonec       H4_F77_FUNC(vslonec, VSLONEC)
#   define  nvfindc        H4_F77_FUNC(vfindc, VFINDC)
#   define  nvfndclsc      H4_F77_FUNC(vfndclsc, VFNDCLSC)
#   define  nvhsdc         H4_F77_FUNC(vhsdc, VHSDC)
#   define  nvhscdc        H4_F77_FUNC(vhscdc, VHSCDC)
#   define  nvhscdmc       H4_F77_FUNC(vhscmc, VHSCDMC)
#   define  nvhsdmc        H4_F77_FUNC(vhsdmc, VHSDMC)
#   define  nvhmkgpc       H4_F77_FUNC(vhmkgpc, VHMKGPC)
#   define  nvflocc        H4_F77_FUNC(vflocc, VFLOCC)
#   define  nvfirefs       H4_F77_FUNC(vfirefs, VFIREFS)
#   define  nvfiqref       H4_F77_FUNC(vfiqref, VFIQREF)
#   define  nvfiqtag       H4_F77_FUNC(vfiqtag, VFIQTAG)
#   define  nvinqtrc       H4_F77_FUNC(vinqtrc, VINQTRC)
#   define  nvntrc         H4_F77_FUNC(vntrc, VNTRC)
#   define  nvgttrsc       H4_F77_FUNC(vgttrsc, VGTTRSC)
#   define  nvgttrc        H4_F77_FUNC(vgttrc, VGTTRC)
#   define  nvadtrc        H4_F77_FUNC(vadtrc, VADTRC)
#   define  nvsiqintr      H4_F77_FUNC(vsiqintr, VSIQINTR)
#   define  nvsiqnelt      H4_F77_FUNC(vsiqnelt, VSIQNELT)
#   define  nvsqfldsc      H4_F77_FUNC(vsqfldsc, VSQFLDSC)
#   define  nvsiqvsz       H4_F77_FUNC(vsiqvsz, VSIQVSZ)
#   define  nvsqnamec      H4_F77_FUNC(vsqnamec, VSQNAMEC)
#   define  nvsfccpk       H4_F77_FUNC(vsfccpk, VSFCCPK)
#   define  nvsfncpk       H4_F77_FUNC(vsfncpk, VSFNCPK)
#   define  nvdtrc         H4_F77_FUNC(vdtrc, VDTRC)
#   define  nvscfcls       H4_F77_FUNC(vscfcls, VSCFCLS)
#   define  nvscsetblsz    H4_F77_FUNC(vscsetblsz, VSCSETBLSZ)
#   define  nvscsetnmbl    H4_F77_FUNC(vscsetnmbl, VSCSETNMBL)
#   define  nvscgblinfo    H4_F77_FUNC(vscgblinfo, VSCGBLINFO)
#  endif                                /* !sl */

    HDFFCLIBAPI      FRETVAL(intf) ndfivopn
                (_fcd filename, intf  * acc_mode, intf  * defdds, intf  * namelen);

    HDFFCLIBAPI      FRETVAL(intf) ndfvclos
                (intf  * file_id);

    HDFFCLIBAPI      FRETVAL(intf) nvatchc
                (intf  * f, intf  * vgid, _fcd accesstype);

    HDFFCLIBAPI      FRETVAL(intf) nvdtchc
                (intf  * vkey);

    HDFFCLIBAPI      FRETVAL(intf) nvgnamc
                (intf  * vkey, _fcd vgname);

    HDFFCLIBAPI      FRETVAL(intf) nvgclsc
                (intf  * vkey, _fcd vgclass);

    HDFFCLIBAPI      FRETVAL(intf) nvinqc
                (intf  * vkey, intf  * nentries, _fcd vgname);

    HDFFCLIBAPI      FRETVAL(intf) nvdeletec
                (intf  *f, intf  * vkey);

    HDFFCLIBAPI      FRETVAL(intf) nvgidc
                (intf  * f, intf  * vgid);

    HDFFCLIBAPI      FRETVAL(intf) nvgnxtc
                (intf  * vkey, intf  * id);

    HDFFCLIBAPI      FRETVAL(intf) nvsnamc
                (intf  * vkey, _fcd vgname, intf  * vgnamelen);

    HDFFCLIBAPI      FRETVAL(intf) nvsclsc
                (intf  * vkey, _fcd vgclass, intf  * vgclasslen);

    HDFFCLIBAPI      FRETVAL(intf) nvinsrtc
                (intf  * vkey, intf  * vobjptr);

    HDFFCLIBAPI      FRETVAL(intf) nvisvgc
                (intf  * vkey, intf  * id);

    HDFFCLIBAPI      FRETVAL(intf) nvfstart
                (intf  * f);

    HDFFCLIBAPI      FRETVAL(intf) nvfend
                (intf  * f);

    HDFFCLIBAPI      FRETVAL(intf) nvisvsc
                (intf  * vkey, intf  * id);

    HDFFCLIBAPI      FRETVAL(intf) nvsatchc
                (intf  * f, intf  * vsref, _fcd accesstype);

    HDFFCLIBAPI      FRETVAL(intf) nvsdtchc
                (intf  * vkey);

    HDFFCLIBAPI      FRETVAL(intf) nvsqref
                (intf  * vkey);

    HDFFCLIBAPI      FRETVAL(intf) nvsqtag
                (intf  * vkey);
  

    HDFFCLIBAPI      FRETVAL(intf) nvsgver
                (intf  * vkey);

    HDFFCLIBAPI      FRETVAL(intf) nvsseekc
                (intf  * vkey, intf  * eltpos);

    HDFFCLIBAPI      FRETVAL(intf) nvsgnamc
                (intf  * vkey, _fcd vsname, intf *vsnamelen);

    HDFFCLIBAPI      FRETVAL(intf) nvsgclsc
                (intf  * vkey, _fcd vsclass, intf *vsclasslen);

    HDFFCLIBAPI      FRETVAL(intf) nvsinqc
                (intf  * vkey, intf  * nelt, intf  * interlace, _fcd fields,
		intf  * eltsize, _fcd vsname, intf *fieldslen, intf *vsnamelen);

    HDFFCLIBAPI      FRETVAL(intf) nvsfexc
                (intf  * vkey, _fcd fields, intf  * fieldslen);

    HDFFCLIBAPI      FRETVAL(intf) nvsfndc
                (intf  * f, _fcd name, intf  * namelen);

    HDFFCLIBAPI      FRETVAL(intf) nvsgidc
                (intf  * f, intf  * vsref);

    HDFFCLIBAPI      FRETVAL(intf) nvsdltc
                (intf  * f, intf  * vsref);

    HDFFCLIBAPI      FRETVAL(intf) nvsapp
                (intf  * vkey, intf  *blk);

    HDFFCLIBAPI      FRETVAL(intf) nvssnamc
                (intf  * vkey, _fcd vsname, intf  * vsnamelen);

    HDFFCLIBAPI      FRETVAL(intf) nvssclsc
                (intf  * vkey, _fcd vsclass, intf  * vsclasslen);

    HDFFCLIBAPI      FRETVAL(intf) nvssfldc
                (intf  * vkey, _fcd fields, intf  * fieldslen);

    HDFFCLIBAPI      FRETVAL(intf) nvssintc
                (intf  * vkey, intf  * interlace);

    HDFFCLIBAPI      FRETVAL(intf) nvsfdefc
                (intf  * vkey, _fcd field, intf  * localtype,
                 intf  * order, intf  * fieldlen);

    HDFFCLIBAPI      FRETVAL(intf) nvssextfc
                (intf  * vkey, _fcd fname, intf  * offset,
                 intf  * fnamelen);

    HDFFCLIBAPI      FRETVAL(intf) nvfnflds
                (intf  * vkey);

    HDFFCLIBAPI      FRETVAL(intf) nvffnamec
                (intf  * vkey, intf  *idx, _fcd fname, intf *len);

    HDFFCLIBAPI      FRETVAL(intf) nvfftype
                (intf  * vkey, intf  *idx);

    HDFFCLIBAPI      FRETVAL(intf) nvffisiz
                (intf  * vkey, intf  *idx);

    HDFFCLIBAPI      FRETVAL(intf) nvffesiz
                (intf  * vkey, intf  *idx);

    HDFFCLIBAPI      FRETVAL(intf) nvffordr
                (intf  * vkey, intf  *idx);

    HDFFCLIBAPI      FRETVAL(intf) nvsfrdc
                (intf  * vkey, _fcd  cbuf, intf  * nelt,
                 intf  * interlace);

    HDFFCLIBAPI      FRETVAL(intf) nvsfrd
                (intf  * vkey, intf  * buf, intf  * nelt,
                 intf  * interlace);

    HDFFCLIBAPI      FRETVAL(intf) nvsreadc
                (intf  * vkey, uint8  * buf, intf  * nelt,
                 intf  * interlace);

    HDFFCLIBAPI      FRETVAL(intf) nvsfwrtc
                (intf  * vkey, _fcd  cbuf, intf  * nelt,
                 intf  * interlace);

    HDFFCLIBAPI      FRETVAL(intf) nvsfwrt
                (intf  * vkey, intf  * buf, intf  * nelt,
                 intf  * interlace);

    HDFFCLIBAPI      FRETVAL(intf) nvswritc
                (intf  * vkey, uint8  * buf, intf  * nelt,
                 intf  * interlace);

    HDFFCLIBAPI      FRETVAL(intf) nvsgintc
                (intf  * vkey);

    HDFFCLIBAPI      FRETVAL(intf) nvseltsc
                (intf  * vkey);

    HDFFCLIBAPI      FRETVAL(intf) nvsgfldc
                (intf  * vkey, _fcd fields);

    HDFFCLIBAPI      FRETVAL(intf) nvssizc
                (intf  * vkey, _fcd fields, intf  * fieldslen);

    HDFFCLIBAPI      FRETVAL(intf) nventsc
                (intf  * f, intf  * vgid);

    HDFFCLIBAPI      FRETVAL(intf) nvlonec
                (intf  * f, intf  * idarray, intf  * asize);

    HDFFCLIBAPI      FRETVAL(intf) nvslonec
                (intf  * f, intf  * idarray, intf  * asize);

    HDFFCLIBAPI      FRETVAL(intf) nvfindc
                (intf  * f, _fcd name, intf  * namelen);

    HDFFCLIBAPI      FRETVAL(intf) nvfndclsc
                (intf  * f, _fcd vgclass, intf  * classlen);

    HDFFCLIBAPI      FRETVAL(intf) nvhscdc
                (intf  * f, _fcd field, _fcd  cbuf, intf  * n, 
                 intf  * datatype, _fcd vsname, _fcd vsclass, 
                 intf  * fieldlen, intf  * vsnamelen,
                 intf  * vsclasslen);

    HDFFCLIBAPI      FRETVAL(intf) nvhsdc
                (intf  * f, _fcd field, uint8  * buf, intf  * n, 
                 intf  * datatype, _fcd vsname, _fcd vsclass, 
                 intf  * fieldlen, intf  * vsnamelen,
                 intf  * vsclasslen);

    HDFFCLIBAPI      FRETVAL(intf) nvhscdmc
                (intf  * f, _fcd field, _fcd  cbuf, intf  * n,
                 intf  * datatype, _fcd vsname, _fcd vsclass, 
                 intf  * order, intf  * fieldlen, intf * vsnamelen,
                 intf  * vsclasslen);

    HDFFCLIBAPI      FRETVAL(intf) nvhsdmc
                (intf  * f, _fcd field, uint8  * buf, intf  * n,
                 intf  * datatype, _fcd vsname, _fcd vsclass, 
                 intf  * order, intf  * fieldlen, intf * vsnamelen,
                 intf  * vsclasslen);

    HDFFCLIBAPI      FRETVAL(intf) nvhmkgpc
                (intf  * f, intf  * tagarray, intf  * refarray, intf  * n,
                 _fcd vgname, _fcd vgclass, intf  * vgnamelen, intf  * vgclasslen);

    HDFFCLIBAPI      FRETVAL(intf) nvflocc
                (intf  * vkey, _fcd field, intf  * fieldlen);

    HDFFCLIBAPI      FRETVAL(intf) nvinqtrc
                (intf  * vkey, intf  * tag, intf  * ref);

    HDFFCLIBAPI      FRETVAL(intf) nvntrc
                (intf  * vkey);

    HDFFCLIBAPI      FRETVAL(intf) nvnrefs
                (intf  * vkey, intf  *tag);

    HDFFCLIBAPI      FRETVAL(intf) nvqref
                (intf  * vkey);

    HDFFCLIBAPI      FRETVAL(intf) nvqtag
                (intf  * vkey);

    HDFFCLIBAPI      FRETVAL(intf) nvgttrsc
                (intf  * vkey, intf  * tagarray, intf  * refarray, intf  * n);

    HDFFCLIBAPI      FRETVAL(intf) nvgttrc
                (intf  * vkey, intf  * which, intf  * tag, intf  * ref);

    HDFFCLIBAPI      FRETVAL(intf) nvadtrc
                (intf  * vkey, intf  * tag, intf  * ref);

    HDFFCLIBAPI      FRETVAL(intf) nvdtrc
                (intf  * vkey, intf  * tag, intf  * ref);

    HDFFCLIBAPI      FRETVAL(intf) nvsqfnelt
                (intf * vkey, intf * nelt);

    HDFFCLIBAPI      FRETVAL(intf) nvsqfintr
                (intf * vkey, intf * interlace);

    HDFFCLIBAPI      FRETVAL(intf) nvsqfldsc
                (intf * vkey, _fcd fields, intf *fieldslen);

    HDFFCLIBAPI      FRETVAL(intf) nvsqfvsiz
                (intf * vkey, intf * size);

    HDFFCLIBAPI      FRETVAL(intf) nvsqnamec  
                (intf * vkey, _fcd name, intf *namelen);

    HDFFCLIBAPI      FRETVAL(intf) nvsfccpk
                (intf *vs, intf *packtype, _fcd buflds, intf *buf, intf *bufsz,
        intf *nrecs, _fcd pckfld, _fcd fldbuf, intf *buflds_len, intf *fld_len);

    HDFFCLIBAPI      FRETVAL(intf) nvsfncpk
                (intf *vs, intf *packtype, _fcd buflds, intf *buf, intf *bufsz,
        intf *nrecs, _fcd pckfld, intf *fldbuf, intf *buflds_len, intf *fld_len);

    HDFFCLIBAPI      FRETVAL(intf) nvscsetblsz
                (intf *id, intf *block_size);

    HDFFCLIBAPI      FRETVAL(intf) nvscsetnmbl
                (intf *id, intf *num_blocks);

    HDFFCLIBAPI      FRETVAL(intf) nvscgblinfo
                (intf *id, intf *block_size, intf *num_blocks);

     HDFFCLIBAPI FRETVAL(intf) nvscfcls
                (intf  * id, _fcd name, intf  * namelen);

     HDFFCLIBAPI FRETVAL(intf) nvfistart
                (intf  * f);

     HDFFCLIBAPI FRETVAL(intf) nvfiend
                (intf  * f);

     HDFFCLIBAPI FRETVAL(intf) nvsiqref
                (intf  * vkey);

     HDFFCLIBAPI FRETVAL(intf) nvsiqtag
                (intf  * vkey);
  
     HDFFCLIBAPI FRETVAL(intf) nvsigver
                (intf  * vkey);

     HDFFCLIBAPI FRETVAL(intf) nvfinflds
                (intf  * vkey);

     HDFFCLIBAPI FRETVAL(intf) nvfifnm
                (intf  * vkey, intf  *index, _fcd fname);

     HDFFCLIBAPI FRETVAL(intf) nvfiftp
                (intf  * vkey, intf  *index);

     HDFFCLIBAPI FRETVAL(intf) nvfifisz
                (intf  * vkey, intf  *index);
     HDFFCLIBAPI FRETVAL(intf) nvfifesz
                (intf  * vkey, intf  *index);
     HDFFCLIBAPI FRETVAL(intf) nvfifodr
                (intf  * vkey, intf  *index);

     HDFFCLIBAPI FRETVAL(intf) nvsfirdc
                (intf  * vkey, _fcd  cbuf, intf  * nelt,
                 intf  * interlace);

     HDFFCLIBAPI FRETVAL(intf) nvsfird
                (intf  * vkey, intf  * buf, intf  * nelt,
                 intf  * interlace);

     HDFFCLIBAPI FRETVAL(intf) nvsfiwrc
                (intf  * vkey, _fcd  cbuf, intf  * nelt,
                 intf  * interlace);

     HDFFCLIBAPI FRETVAL(intf) nvsfiwr
                (intf  * vkey, intf  * buf, intf  * nelt,
                 intf  * interlace);

     HDFFCLIBAPI FRETVAL(intf) nvfirefs
                (intf  * vkey, intf  *tag);

     HDFFCLIBAPI FRETVAL(intf) nvfiqref
                (intf  * vkey);

     HDFFCLIBAPI FRETVAL(intf) nvfiqtag
                (intf  * vkey);

     HDFFCLIBAPI FRETVAL(intf) nvsiqnelt
                (intf * vkey, intf *nelt);

     HDFFCLIBAPI FRETVAL(intf) nvsiqintr
                (intf * vkey, intf *interlace);

     HDFFCLIBAPI FRETVAL(intf) nvsqfldsc
                (intf * vkey, _fcd fields, intf *fieldslen);

     HDFFCLIBAPI FRETVAL(intf) nvsiqvsz
                (intf * vkey, intf *ret_size);

 /* 
  ** from vattrf.c
  */
/* 
 *  Not all HDF4 Fortran functions are real Fortran functions, they are C wrappers called from a
 *  Fortran program. Since on Windows we have to have a real Fortran function, additional C wrappers
 *  were added at the time when Windows port was done for multi-file interfaces: for example, 
 *  hclose is a C wrapper on UNIX, while on Windows we have two functions: Fortran function hclose
 *  and C wrapper function hiclose called by hclose   
 *                                                                                    EIP 2007-09-14
 */
# ifndef _WIN32
#  define nvsfcfdx   H4_F77_FUNC(vsfcfdx, VSFCFDX)
#  define nvsfcsat   H4_F77_FUNC(vsfcsat, VSFCSAT)
#  define nvsfcsca   H4_F77_FUNC(vsfcsca, VSFCSCA)
#  define nvsfnats   H4_F77_FUNC(vsfnats, VSFNATS)
#  define nvsffnas   H4_F77_FUNC(vsffnas, VSFFNAS)
#  define nvsfcfda   H4_F77_FUNC(vsfcfda, VSFCFDA)
#  define nvsfcain   H4_F77_FUNC(vsfcain, VSFCAIN)
#  define nvsfgnat   H4_F77_FUNC(vsfgnat, VSFGNAT)
#  define nvsfgcat   H4_F77_FUNC(vsfgcat, VSFGCAT)
#  define nvsfisat   H4_F77_FUNC(vsfisat, VSFISAT)
#  define nvfcsatt   H4_F77_FUNC(vfcsatt, VFCSATT)
#  define nvfcscat   H4_F77_FUNC(vfcscat, VFCSCAT)
#  define nvfnatts   H4_F77_FUNC(vfnatts, VFNATTS)
#  define nvfcfdat   H4_F77_FUNC(vfcfdat, VFCFDAT)
#  define nvfainfo   H4_F77_FUNC(vfainfo, VFAINFO)
#  define nvfgnatt   H4_F77_FUNC(vfgnatt, VFGNATT)
#  define nvfgcatt   H4_F77_FUNC(vfgcatt, VFGCATT)
#  define nvfgver    H4_F77_FUNC(vfgver,  VFGVER)
# else                          /* !sl */
#  define nvsfcfdx   H4_F77_FUNC(vsfcfdx, VSFCFDX)
#  define nvsfcsat   H4_F77_FUNC(vsfcsat, VSFCSAT)
#  define nvsfcsca   H4_F77_FUNC(vsfcsca, VSFCSCA)
#  define nvsfcnats  H4_F77_FUNC(vsfcnats, VSFCNATS)
#  define nvsfcfnas  H4_F77_FUNC(vsfcfnas, VSFCFNAS)
#  define nvsfcfda   H4_F77_FUNC(vsfcfda, VSFCFDA)
#  define nvsfcainf  H4_F77_FUNC(vsfcainf, VSFCAINF)
#  define nvsfcgna   H4_F77_FUNC(vsfcgna, VSFCGNA)
#  define nvsfcgca   H4_F77_FUNC(vsfcgca, VSFCGCA)
#  define nvsfcisa   H4_F77_FUNC(vsfcisa, VSFCISA)
#  define nvfcsatt   H4_F77_FUNC(vfcsatt, VFCSATT)
#  define nvfcscat   H4_F77_FUNC(vfcscat, VFCSCAT)
#  define nvfcnats   H4_F77_FUNC(vfcnats, VFCNATS)
#  define nvfcfdat   H4_F77_FUNC(vfcfdat, VFCFDAT)
#  define nvfcainf   H4_F77_FUNC(vfcainf, VFCAINF)
#  define nvfcgnat   H4_F77_FUNC(vfcgnat, VFCGNAT)
#  define nvfcgcat   H4_F77_FUNC(vfcgcat, VFCGCAT)
#  define nvfcgver   H4_F77_FUNC(vfcgver, VFCGVER)
# endif                          /* !sl */

   HDFLIBAPI   FRETVAL(intf) nvsfcfdx
            (intf *vsid, _fcd fldnm, intf *findex, intf *fldnmlen);
   HDFLIBAPI   FRETVAL(intf) nvsfcsat
            (intf *vsid, intf *findex, _fcd attrnm, intf *dtype,
             intf *count, intf *values, intf *attrnmlen);
   HDFLIBAPI   FRETVAL(intf) nvsfcsca
            (intf *vsid, intf *findex, _fcd attrnm, intf *dtype,
             intf *count, _fcd values, intf *attrnmlen);
   HDFLIBAPI   FRETVAL(intf) nvsfnats
            (intf *vsid);
   HDFLIBAPI   FRETVAL(intf) nvsffnas
            (intf *vsid, intf *findex);
   HDFLIBAPI   FRETVAL(intf) nvsfcfda
            (intf *vsid, intf *findex, _fcd attrnm, intf *attrnmlen);
   HDFLIBAPI   FRETVAL(intf) nvsfcain
            (intf *vsid, intf *findex, intf *aindex, _fcd attrname,
             intf *dtype, intf *count, intf *size, intf *attrnamelen);
   HDFLIBAPI   FRETVAL(intf) nvsfgnat
            (intf *vsid, intf *findex, intf *aindex, intf *values);
   HDFLIBAPI   FRETVAL(intf) nvsfgcat
            (intf *vsid, intf *findex, intf *aindex, _fcd values);
   HDFLIBAPI   FRETVAL(intf) nvsfisat
            (intf *vsid);
   HDFLIBAPI   FRETVAL(intf) nvfcsatt
            (intf *vgid, _fcd attrnm, intf *dtype,
             intf *count, intf *values, intf *attrnmlen);
   HDFLIBAPI   FRETVAL(intf) nvfcscat
            (intf *vgid, _fcd attrnm, intf *dtype, intf *count,
             _fcd values, intf *attrnmlen);
   HDFLIBAPI   FRETVAL(intf) nvfnatts
            (intf *vgid);
   HDFLIBAPI   FRETVAL(intf) nvfcfdat
            (intf *vgid, _fcd attrnm, intf *attrnmlen);
   HDFLIBAPI   FRETVAL(intf) nvfainfo
            (intf *vgid, intf *aindex, _fcd attrname,
             intf *dtype, intf *count, intf *size);
   HDFLIBAPI   FRETVAL(intf) nvfgnatt
            (intf *vgid, intf *aindex, intf *values);
   HDFLIBAPI   FRETVAL(intf) nvfgcatt
            (intf *vgid, intf *aindex, _fcd values);
   HDFLIBAPI   FRETVAL(intf) nvfgver
            (intf *vgid);

/* Added for windows */
   HDFFCLIBAPI FRETVAL(intf) nvsfainf
            (intf *vsid, intf *findex, intf *aindex, _fcd attrname,
             intf *dtype, intf *count, intf *size);
  
   HDFFCLIBAPI FRETVAL(intf) nvsfcnats
            (intf *vsid);

   HDFFCLIBAPI FRETVAL(intf) nvsfcfnas
            (intf *vsid, intf *findex);
   
   HDFFCLIBAPI FRETVAL(intf) nvsfcainf
            (intf *vsid, intf *findex, intf *aindex, _fcd attrname,
             intf *dtype, intf *count, intf *size);

   HDFFCLIBAPI FRETVAL(intf) nvsfcgna
            (intf *vsid, intf *findex, intf *aindex, intf *values);

   HDFFCLIBAPI FRETVAL(intf) nvsfcgca
            (intf *vsid, intf *findex, intf *aindex, _fcd values);

   HDFFCLIBAPI FRETVAL(intf) nvsfcisa
            (intf *vsid);

   HDFFCLIBAPI FRETVAL(intf) nvfcnats
            (intf *vgid);

   HDFFCLIBAPI FRETVAL(intf) nvfcainf
            (intf *vgid, intf *aindex, _fcd attrname,
             intf *dtype, intf *count, intf *size);

   HDFFCLIBAPI FRETVAL(intf) nvfcgnat
            (intf *vgid, intf *aindex, intf *values);

   HDFFCLIBAPI FRETVAL(intf) nvfcgcat
            (intf *vgid, intf *aindex, _fcd values);

   HDFFCLIBAPI FRETVAL(intf) nvfcgver
            (intf *vgid);
/* End of windows */


/* 
  ** from dfufp2i.c
 */

#   define nduif2i       H4_F77_FUNC(duif2i, DUIF2I)

    HDFLIBAPI   FRETVAL(int) nduif2i
             (int32 *hdim, int32 *vdim, float32 *max, float32 *min, float32 hscale[],
              float32 vscale[], float32 data[], _fcd palette, _fcd outfile,
              int *ct_method, int32 *hres, int32 *vres, int *compress, int *lenfn);


#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif                          /* _H_PROTO_FORTRAN */

