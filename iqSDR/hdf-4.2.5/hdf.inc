C****************************************************************************
C* NCSA HDF                                                                 *
C* Software Development Group                                               *
C* National Center for Supercomputing Applications                          *
C* University of Illinois at Urbana-Champaign                               *
C* 605 E. Springfield, Champaign IL 61820                                   *
C*                                                                          *
C* For conditions of distribution and use, see the accompanying             *
C* hdf/COPYING file.                                                        *
C*                                                                          *
C****************************************************************************
C
C $Id: hdf.inc 4703 2005-12-18 00:54:15Z epourmal $
C
C *-----------------------------------------------------------------------------
C * File: 	hdf.inc
C * Purpose:	Fortran header file for HDF routines
C * Contents: 
C *     Tag definitions
C *     Error return codes
C *    	Logical constants
C * Remarks: This file can be included with Fortran user programs.  As a
C *          general rule, don't use DFNT constants that don't include a
C *          number in their name.  E.g., don't use DFNT_FLOAT, use
C *          DFNT_FLOAT32 or DFNT_FLOAT64.  The DFNT constants that don't
C *          include numbers are for backward compatibility only.  Also,
C *          there are no current plans to support 128-bit number types.
C *          For more information about constants in this file, see the
C *          equivalent constant declarations in the C include file 'hdf.h'
C *------------------------------------------------------------------------


C	Error Return Codes 

      integer   DFE_NOERROR,   DFE_NONE,        DFE_FNF   
      integer   DFE_DENIED,    DFE_ALROPEN,     DFE_TOOMANY
      integer   DFE_BADNAME,   DFE_BADACC,      DFE_BADOPEN
      integer   DFE_NOTOPEN,   DFE_CANTCLOSE,   DFE_DFNULL
      integer   DFE_ILLTYPE,   DFE_UNSUPPORTED, DFE_BADDDLIST
      integer   DFE_NOTDFFILE, DFE_SEEDTWICE,   DFE_NOSPACE
      integer   DFE_NOSUCHTAG, DFE_READERROR

      parameter(DFE_NOERROR      =   0)
      parameter(DFE_NONE         =   0)
      parameter(DFE_FNF        	 =  -1)
      parameter(DFE_DENIED       =  -2)
      parameter(DFE_ALROPEN      =  -3)
      parameter(DFE_TOOMANY      =  -4)
      parameter(DFE_BADNAME   	 =  -5)
      parameter(DFE_BADACC       =  -6)
      parameter(DFE_BADOPEN      =  -7)
      parameter(DFE_NOTOPEN      =  -8)
      parameter(DFE_CANTCLOSE    =  -9)
      parameter(DFE_DFNULL       = -10)
      parameter(DFE_ILLTYPE      = -11)
      parameter(DFE_UNSUPPORTED  = -12)
      parameter(DFE_BADDDLIST    = -13)
      parameter(DFE_NOTDFFILE    = -14)
      parameter(DFE_SEEDTWICE    = -15)
      parameter(DFE_NOSPACE      = -16)
      parameter(DFE_NOSUCHTAG    = -17)
      parameter(DFE_READERROR    = -18)

      integer	DFE_WRITEERROR,  DFE_SEEKERROR,   DFE_NOFREEDD
      integer   DFE_BADTAG,      DFE_BADREF,      DFE_RDONLY
      integer   DFE_BADCALL,     DFE_BADPTR,      DFE_BADLEN
      integer   DFE_BADSEEK,     DFE_NOMATCH,     DFE_NOTINSET
      integer   DFE_BADDIM,      DFE_BADOFFSET,   DFE_BADSCHEME
      integer   DFE_NODIM,       DFE_NOTENOUGH,   DFE_NOVALS
      integer   DFE_CORRUPT,     DFE_BADFP
 
      parameter(DFE_WRITEERROR           = -19)
      parameter(DFE_SEEKERROR            = -20)
      parameter(DFE_NOFREEDD             = -21)
      parameter(DFE_BADTAG               = -22)
      parameter(DFE_BADREF               = -23)
      parameter(DFE_RDONLY               = -24)
      parameter(DFE_BADCALL              = -25)
      parameter(DFE_BADPTR               = -26)
      parameter(DFE_BADLEN               = -27)
      parameter(DFE_BADSEEK              = -28)
      parameter(DFE_NOMATCH              = -29)
      parameter(DFE_NOTINSET             = -30)
      parameter(DFE_BADDIM               = -31)
      parameter(DFE_BADOFFSET            = -32)
      parameter(DFE_BADSCHEME            = -33)
      parameter(DFE_NODIM                = -34)
      parameter(DFE_NOTENOUGH            = -35)
      parameter(DFE_NOVALS               = -36)
      parameter(DFE_CORRUPT              = -37)
      parameter(DFE_BADFP                = -38)

      integer	DFE_NOREF,       DFE_BADDATATYPE, DFE_BADMCTYPE
      integer   DFE_BADNUMTYPE,  DFE_BADORDER,    DFE_ARGS
      integer   DFE_INTERNAL,    DFE_DUPDD,       DFE_CANTMOD
      integer   DFE_RANGE,       DFE_BADTABLE,    DFE_BADSDG
      integer   DFE_BADNDG,      DFE_BADFIELDS,   DFE_NORESET
      integer   DFE_NOVS,        DFE_VGSIZE,      DFE_DIFFFILES
      integer   DFE_VTAB,        DFE_BADAID

      parameter(DFE_NOREF                = -39)
      parameter(DFE_BADDATATYPE          = -40)
      parameter(DFE_BADMCTYPE            = -41)
      parameter(DFE_BADNUMTYPE           = -42)
      parameter(DFE_BADORDER             = -43)
      parameter(DFE_ARGS                 = -44)
      parameter(DFE_INTERNAL             = -45)
      parameter(DFE_DUPDD                = -46)
      parameter(DFE_CANTMOD              = -47)
      parameter(DFE_RANGE                = -48)
      parameter(DFE_BADTABLE             = -49)
      parameter(DFE_BADSDG               = -50)
      parameter(DFE_BADNDG               = -51)
      parameter(DFE_BADFIELDS            = -52)
      parameter(DFE_NORESET              = -53)
      parameter(DFE_NOVS                 = -54)
      parameter(DFE_VGSIZE               = -55)
      parameter(DFE_DIFFFILES            = -56)
      parameter(DFE_VTAB                 = -57)
      parameter(DFE_BADAID               = -58)

      integer   DFE_OPENAID, DFE_BADCONV, DFE_GENAPP, DFE_CANTFLUSH
      integer   DFE_BADTYPE, DFE_SYMSIZE, DFE_BADATTACH
      integer   DFE_CANTDETACH

      parameter(DFE_OPENAID              = -59)
      parameter(DFE_BADCONV              = -60)
      parameter(DFE_GENAPP               = -61)
      parameter(DFE_CANTFLUSH            = -62)
      parameter(DFE_BADTYPE              = -63)
      parameter(DFE_SYMSIZE              = -64)
      parameter(DFE_BADATTACH            = -65)
      parameter(DFE_CANTDETACH           = -66)

C internal file access codes

      integer 	DFACC_READ, DFACC_WRITE, DFACC_CREATE, DFACC_ALL  
      integer	DFACC_RDONLY, DFACC_RDWR, DFACC_CLOBBER

      parameter(DFACC_READ               = 1)
      parameter(DFACC_WRITE              = 2)
      parameter(DFACC_CREATE             = 4)
      parameter(DFACC_ALL                = 7)
      parameter(DFACC_RDONLY             = 1)
      parameter(DFACC_RDWR               = 3)
      parameter(DFACC_CLOBBER            = 4)

C	Access types for SDsetaccesstype

      integer	DFACC_DEFAULT, DFACC_SERIAL, DFACC_PARALLEL
      parameter(DFACC_DEFAULT           = 0)
      parameter(DFACC_SERIAL            = 1)
      parameter(DFACC_PARALLEL	        = 9)

C	Constants for DFSDsetorder

      integer 	DFO_FORTRAN, DFO_C
 
      parameter(DFO_FORTRAN            = 1)
      parameter(DFO_C                  = 2)

C	Definitions of storage convention

      integer	DFNTF_IEEE, DFNTF_VAX, DFNTF_CRAY, DFNTF_PC
      integer   DFNTF_CONVEX, DFNTF_VP
 
      parameter(DFNTF_IEEE             = 1)
      parameter(DFNTF_VAX              = 2)
      parameter(DFNTF_CRAY             = 3)
      parameter(DFNTF_PC               = 4)
      parameter(DFNTF_CONVEX           = 5)
      parameter(DFNTF_VP               = 6)

C       Masks for types

      integer   DFNT_HDF, DFNT_NATIVE, DFNT_CUSTOM, DFNT_LITEND

      parameter(DFNT_HDF                 = 0)
      parameter(DFNT_NATIVE              = 4096)
      parameter(DFNT_CUSTOM              = 8192)
      parameter(DFNT_LITEND              = 16384)

C	Number type info codes 

      integer	DFNT_NONE, DFNT_QUERY, DFNT_VERSION
 
      parameter(DFNT_NONE       = 0)
      parameter(DFNT_QUERY      = 0)
      parameter(DFNT_VERSION    = 1)
      
      integer   DFNT_FLOAT32, DFNT_FLOAT, DFNT_FLOAT64
      integer   DFNT_DOUBLE,  DFNT_FLOAT128

      parameter(DFNT_FLOAT32    = 5)
      parameter(DFNT_FLOAT      = 5)
      parameter(DFNT_FLOAT64    = 6)
      parameter(DFNT_DOUBLE     = 6)
      parameter(DFNT_FLOAT128   = 7)

      integer   DFNT_INT8,  DFNT_UINT8
      integer   DFNT_INT16, DFNT_UINT16
      integer   DFNT_INT32, DFNT_UINT32
      integer   DFNT_INT64, DFNT_UINT64
      integer   DFNT_INT128,DFNT_UINT128
 
      parameter(DFNT_INT8       = 20)
      parameter(DFNT_UINT8      = 21)
      parameter(DFNT_INT16      = 22)
      parameter(DFNT_UINT16     = 23)
      parameter(DFNT_INT32      = 24)
      parameter(DFNT_UINT32     = 25)
      parameter(DFNT_INT64      = 26)
      parameter(DFNT_UINT64     = 27)
      parameter(DFNT_INT128     = 28)
      parameter(DFNT_UINT128    = 29)

      integer 	DFNT_UCHAR8, DFNT_UCHAR, DFNT_CHAR8
      integer   DFNT_CHAR,   DFNT_CHAR16, DFNT_UCHAR16
 
      parameter(DFNT_UCHAR8     = 3)
      parameter(DFNT_UCHAR      = 3)
      parameter(DFNT_CHAR8      = 4)
      parameter(DFNT_CHAR       = 4)
      parameter(DFNT_CHAR16     = 42)
      parameter(DFNT_UCHAR16    = 43)

      integer	DFNT_NFLOAT32, DFNT_NFLOAT, DFNT_NFLOAT64
      integer	DFNT_NDOUBLE,  DFNT_NFLOAT128

      parameter(DFNT_NFLOAT32   = 4101)
      parameter(DFNT_NFLOAT     = 4101)
      parameter(DFNT_NFLOAT64   = 4102)
      parameter(DFNT_NDOUBLE    = 4102)
      parameter(DFNT_NFLOAT128  = 4103)

      integer    DFNT_NINT8,  DFNT_NUINT8
      integer    DFNT_NINT16, DFNT_NUINT16
      integer    DFNT_NINT32, DFNT_NUINT32
      integer    DFNT_NINT64, DFNT_NUINT64
      integer    DFNT_NINT128,DFNT_NUINT128
 
      parameter(DFNT_NINT8       = 4116)
      parameter(DFNT_NUINT8      = 4117)
      parameter(DFNT_NINT16      = 4118)
      parameter(DFNT_NUINT16     = 4119)
      parameter(DFNT_NINT32	 = 4120)
      parameter(DFNT_NUINT32     = 4121)
      parameter(DFNT_NINT64      = 4122)
      parameter(DFNT_NUINT64     = 4123)
      parameter(DFNT_NINT128     = 4124)
      parameter(DFNT_NUINT128    = 4125)

      integer 	DFNT_NUCHAR8, DFNT_NUCHAR, DFNT_NCHAR8
      integer	DFNT_NCHAR,   DFNT_NCHAR16, DFNT_NUCHAR16
 
      parameter(DFNT_NUCHAR8  = 4099)
      parameter(DFNT_NUCHAR   = 4099)
      parameter(DFNT_NCHAR8   = 4100)
      parameter(DFNT_NCHAR    = 4100)
      parameter(DFNT_NCHAR16  = 4138)
      parameter(DFNT_NUCHAR16 = 4139)

      integer	DFNT_LFLOAT32, DFNT_LFLOAT, DFNT_LFLOAT64
      integer	DFNT_LDOUBLE,  DFNT_LFLOAT128

      parameter(DFNT_LFLOAT32  = 16389)
      parameter(DFNT_LFLOAT    = 16389)
      parameter(DFNT_LFLOAT64  = 16390)
      parameter(DFNT_LDOUBLE   = 16390)
      parameter(DFNT_LFLOAT128 = 16391)

      integer   DFNT_LINT8,DFNT_LUINT8,DFNT_LINT16,DFNT_LUINT16
      integer   DFNT_LINT32,DFNT_LUINT32,DFNT_LINT64,DFNT_LUINT64
      integer   DFNT_LINT128,DFNT_LUINT128
 
      parameter(DFNT_LINT8      = 16404)
      parameter(DFNT_LUINT8     = 16405)
      parameter(DFNT_LINT16     = 16406)
      parameter(DFNT_LUINT16    = 16407)
      parameter(DFNT_LINT32     = 16408)
      parameter(DFNT_LUINT32    = 16409)
      parameter(DFNT_LINT64     = 16410)
      parameter(DFNT_LUINT64    = 16411)
      parameter(DFNT_LINT128    = 16412)
      parameter(DFNT_LUINT128   = 16413)

      integer 	DFNT_LUCHAR8, DFNT_LUCHAR, DFNT_LCHAR8
      integer	DFNT_LCHAR,   DFNT_LCHAR16, DFNT_LUCHAR16
 
      parameter(DFNT_LUCHAR8    = 16387)
      parameter(DFNT_LUCHAR     = 16387)
      parameter(DFNT_LCHAR8     = 16388)
      parameter(DFNT_LCHAR      = 16388)
      parameter(DFNT_LCHAR16    = 16426)
      parameter(DFNT_LUCHAR16   = 16427)

C	tags and refs

      integer 	DFREF_WILDCARD, DFTAG_WILDCARD, DFTAG_NULL
      integer	DFTAG_LINKED, DFTAG_VERSION, DFTAG_COMPRESSED
 
      parameter(DFREF_WILDCARD  = 0, DFTAG_WILDCARD  = 0)
      parameter(DFTAG_NULL      = 1, DFTAG_LINKED    = 20)
      parameter(DFTAG_VERSION   = 30,DFTAG_COMPRESSED = 40)


C	utility set

      integer	DFTAG_FID, DFTAG_FD,  DFTAG_TID, DFTAG_TD
      integer	DFTAG_DIL, DFTAG_DIA, DFTAG_NT,  DFTAG_MT
 
      parameter(DFTAG_FID       = 100, DFTAG_FD        = 101)
      parameter(DFTAG_TID       = 102, DFTAG_TD        = 103)
      parameter(DFTAG_DIL       = 104, DFTAG_DIA       = 105)
      parameter(DFTAG_NT        = 106, DFTAG_MT        = 107)

C 	raster-8 set 

      integer	DFTAG_ID8, DFTAG_IP8, DFTAG_RI8
      integer	DFTAG_CI8, DFTAG_II8
 
      parameter(DFTAG_ID8       = 200, DFTAG_IP8       = 201)
      parameter(DFTAG_RI8       = 202, DFTAG_CI8       = 203)
      parameter(DFTAG_II8       = 204)

C	Raster Image set

      integer	DFTAG_ID, DFTAG_LUT, DFTAG_RI, DFTAG_CI
 
      parameter(DFTAG_ID        = 300, DFTAG_LUT       = 301)
      parameter(DFTAG_RI        = 302, DFTAG_CI        = 303)

      integer	DFTAG_RIG, DFTAG_LD,  DFTAG_MD, DFTAG_MA
      integer	DFTAG_CCN, DFTAG_CFM, DFTAG_AR
  
      parameter(DFTAG_RIG       = 306, DFTAG_LD        = 307)
      parameter(DFTAG_MD        = 308, DFTAG_MA        = 309)
      parameter(DFTAG_CCN       = 310, DFTAG_CFM       = 311)
      parameter(DFTAG_AR        = 312)

      integer	DFTAG_DRAW, DFTAG_RUN, DFTAG_XYP, DFTAG_MTO
 
      parameter(DFTAG_DRAW      = 400, DFTAG_RUN       = 401)
      parameter(DFTAG_XYP       = 500, DFTAG_MTO       = 501)

C	Tektronix 

      integer	DFTAG_T14, DFTAG_T105
 
      parameter(DFTAG_T14       = 602, DFTAG_T105      = 603)

C	Scientific Data set 

      integer   DFTAG_SDG, DFTAG_SDD, DFTAG_SD, DFTAG_SDS, DFTAG_SDL
      integer   DFTAG_SDU, DFTAG_SDF, DFTAG_SDM, DFTAG_SDC
      integer   DFTAG_SDT,DFTAG_SDLNK,DFTAG_NDG
      integer   DFTAG_BREQ,DFTAG_EREQ,DFTAG_CAL, DFTAG_FV
 
      parameter(DFTAG_SDG       = 700, DFTAG_SDD       = 701)
      parameter(DFTAG_SD        = 702, DFTAG_SDS       = 703)
      parameter(DFTAG_SDL       = 704, DFTAG_SDU       = 705)
      parameter(DFTAG_SDF       = 706, DFTAG_SDM       = 707)
      parameter(DFTAG_SDC       = 708, DFTAG_SDT       = 709)
      parameter(DFTAG_SDLNK     = 710, DFTAG_NDG       = 720)
      parameter(DFTAG_CAL       = 731, DFTAG_FV        = 732)
      parameter(DFTAG_BREQ      = 799, DFTAG_EREQ      = 780)

C	VSets 

      integer	DFTAG_VG, DFTAG_VH, DFTAG_VS
 
      parameter(DFTAG_VG        = 1965, DFTAG_VH        = 1962)
      parameter(DFTAG_VS        = 1963)

C	compression schemes 

      integer	DFTAG_RLE, DFTAG_IMC, DFTAG_IMCOMP, DFTAG_JPEG
      integer   DFTAG_GREYJPEG
 
      parameter(DFTAG_RLE       =11, DFTAG_IMC       =12)
      parameter(DFTAG_IMCOMP   	=12, DFTAG_JPEG      =13)
      parameter(DFTAG_GREYJPEG 	=14)

C	SPECIAL CODES 

      integer	SPECIAL_LINKED, SPECIAL_EXT
 
      parameter(SPECIAL_LINKED 	= 1, SPECIAL_EXT 	= 2)

C	PARAMETERS 

      integer	DF_MAXFNLEN
      integer   SD_UNLIMITED
      integer   SD_DIMVAL_BW_COMP
      integer   SD_DIMVAL_BW_INCOMP 
      integer   SD_FILL
      integer   SD_NOFILL

      parameter(DF_MAXFNLEN     = 256, SD_UNLIMITED    = 0)
      parameter(SD_DIMVAL_BW_COMP = 1, SD_DIMVAL_BW_INCOMP = 0)
      parameter(SD_FILL           = 0, SD_NOFILL = 256)

      integer   HDF_VDATA
      
      parameter(HDF_VDATA = -1)

C       Standard return codes       
      integer	SUCCEED, FAIL  
 
      parameter(SUCCEED         = 0, FAIL     	= -1)


C	Compression Types 

      integer	COMP_NONE, COMP_RLE, COMP_IMCOMP, COMP_JPEG

      parameter(COMP_NONE       = 0, COMP_RLE        = 11)
      parameter(COMP_IMCOMP     = 12, COMP_JPEG       = 2)
C
C       Fortran chunking (SD and GR interfaces) and compression routines use
C       the following compression types:
C
      integer COMP_CODE_NONE, COMP_CODE_RLE, COMP_CODE_NBIT
      integer COMP_CODE_SKPHUFF, COMP_CODE_DEFLATE
      integer COMP_CODE_JPEG
      integer COMP_CODE_SZIP
      integer SZ_EC_OPTION_MASK, SZ_NN_OPTION_MASK
      integer COMP_DECODER_ENABLED, COMP_ENCODER_ENABLED
      parameter (COMP_CODE_NONE = 0)
      parameter (COMP_CODE_RLE  = 1)
      parameter (COMP_CODE_NBIT = 2)
      parameter (COMP_CODE_SKPHUFF = 3)
      parameter (COMP_CODE_DEFLATE = 4)
      parameter (COMP_CODE_SZIP = 5)
      parameter (COMP_CODE_JPEG = 6)
C
C     SZIP parameters
C
      parameter (SZ_EC_OPTION_MASK = 4)
      parameter (SZ_NN_OPTION_MASK = 32)
      parameter (COMP_DECODER_ENABLED = 1)
      parameter (COMP_ENCODER_ENABLED = 2)
C
C	Interlace Types 

      integer	MFGR_INTERLACE_PIXEL, MFGR_INTERLACE_LINE
      integer   MFGR_INTERLACE_COMPONENT

      parameter(MFGR_INTERLACE_PIXEL    = 0)
      parameter(MFGR_INTERLACE_LINE     = 1)
      parameter(MFGR_INTERLACE_COMPONENT= 2)

      integer	FULL_INTERLACE, NO_INTERLACE

      parameter(FULL_INTERLACE	= 0, NO_INTERLACE = 1)

C       Vdata fields packing types
      integer   HDF_VSPACK, HDF_VSUNPACK
      parameter (HDF_VSPACK   = 0, HDF_VSUNPACK = 1)

C    Multi-file Annotation types
      integer AN_DATA_LABEL, AN_DATA_DESC, AN_FILE_LABEL, AN_FILE_DESC

      parameter(AN_DATA_LABEL = 0, AN_DATA_DESC  = 1)
      parameter(AN_FILE_LABEL = 2, AN_FILE_DESC  = 3)

c******************End of hdf.inc***************************
