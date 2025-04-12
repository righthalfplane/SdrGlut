$! ****************************************************************************
$! * NCSA HDF                                                                 *
$! * Software Development Group                                               *
$! * National Center for Supercomputing Applications                          *
$! * University of Illinois at Urbana-Champaign                               *
$! * 605 E. Springfield, Champaign IL 61820                                   *
$! *                                                                          *
$! * For conditions of distribution and use, see the accompanying             *
$! * hdf/COPYING file.                                                        *
$! *                                                                          *
$! ***************************************************************************
$!
$! $Id: MAKENOF.COM 3722 1999-04-27 19:52:13Z epourmal $
$! Makefile for VAX/VMX systems.
$!
$! Make a no-fortran-stub HDF library
$!
$ if f$getsyi("arch_name") .eqs. "VAX"
$ then 
$ ccopt = "/DECC/STANDARD=VAXC"
$ else
$ ccopt = ""
$ endif
$ ccc := cc 'ccopt /opt/nodebug/include=([-.jpeg],[-.zlib])/define=VMS
$ type sys$input
	Creating base HDF library
$!
$ cobj="df24,dfan,dfcomp, dfgr, dfgroup, dfimcomp, dfp, dfr8, dfsd, "+-
"dfstubs, dfutil,hblocks, herr, hextelt, hfile, hkit, vg, vgp, vhi, "+-
"vio, vparse, vrw, vsfld, dfconv, vconv, dfrle, dfjpeg, dfunjpeg, "+-
"dfufp2i,dfknat,dfkswap,dfkcray, dfkvms, dfkconv, dfkfuji,hbitio, "+-
"tbbt,hdfalloc,mfgr,cdeflate,cnbit,cnone,crle,cskphuff,hcomp,mfan," +-
"mstdio,atom, bitvect, dynarray, hfiledd, glist,hchunks,mcache, " +- 
"vattr, hbuffer, hcompri"
$!
$ ccc 'cobj 
$ library/create []df  'cobj
$ type sys$input
	Created base HDF library
$!
