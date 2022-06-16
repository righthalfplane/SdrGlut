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

#ifdef RCSID
static char RcsId[] = "@(#)$Revision: 4932 $";
#endif

/* $Id: hbitio.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*
FILE
   hbitio.c
   HDF bit level I/O routines
REMARKS
   These functions operate on top of the "H" layer routines
   (i.e. they call Hstartread, Hstartwrite, Hread, Hseek, Hwrite, etc.)
   and depend on them for all actual I/O to data elements in the
   file.  This may be somewhat slow, but it prevents having
   to duplicate code for that access.
EXPORTED ROUTINES
   Hstartbitread  - open a dataset for bitfile dataset reading
   Hstartbitwrite - open a dataset for bitfile dataset writing
   Happendable    - make a writable dataset appendable
   Hbitread       - read bits from a bitfile dataset
   Hbitwrite      - write bits to a bitfile dataset
   Hbitseek       - seek to a given bit offset in a bitfile dataset
   Hendbitaccess  - close off access to a bitfile dataset
LOCAL ROUTINES
   HIbitflush         - flush the bits out to a writable bitfile
   HIget_bitfile_rec  - get a free bitfile record 
   HIread2write       - switch from reading bits to writing them
   HIwrite2read       - switch from writing bits to reading them
AUTHOR
   Quincey Koziol
MODIFICATION HISTORY
   3/15/92     Starting writing
*/

#define BITMASTER
#include "hdf.h"
#include "hfile.h"

/* Local Variables */

/* Whether we've installed the library termination function yet for this interface */
PRIVATE intn library_terminate = FALSE;

/* Local Function Declarations */
PRIVATE bitrec_t * HIget_bitfile_rec(void);

PRIVATE intn HIbitflush(bitrec_t * bitfile_rec, intn flushbit, intn writeout);

PRIVATE intn HIwrite2read(bitrec_t * bitfile_rec);
PRIVATE intn HIread2write(bitrec_t * bitfile_rec);

PRIVATE intn HIbitstart(void);

/* #define TESTING */
/* Actual Function Definitions */

/*--------------------------------------------------------------------------

 NAME
       Hstartbitread -- locate and position a bit-read access elt on a tag/ref
 USAGE
       int32 Hstartbitread(fileid, tag, ref)
       int fileid;             IN: id of file to attach access element to
       int tag;                IN: tag to search for
       int ref;                IN: ref to search for
 RETURNS
       returns id of bit-access element if successful, otherwise FAIL (-1)
 DESCRIPTION
        Calls Hstartread and initializes bit-level structures.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
Hstartbitread(int32 file_id, uint16 tag, uint16 ref)
{
    CONSTR(FUNC, "Hstartbitread");  /* for HERROR */
    int32       aid;            /* Access ID for the bit-level routines to use */
    struct bitrec_t *bitfile_rec;   /* Pointer to the bitfile record */
    int32       ret_value;          /* return bit ID */

    /* clear error stack */
    HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(HIbitstart()==FAIL)
            HRETURN_ERROR(DFE_CANTINIT, FAIL);

    /* Try to get an AID */
    if ((aid = Hstartread(file_id, tag, ref)) == FAIL)
        HRETURN_ERROR(DFE_BADAID, FAIL);

    /* get a slot in the access record array */
    if ((bitfile_rec = HIget_bitfile_rec()) == NULL)
        HRETURN_ERROR(DFE_TOOMANY, FAIL);

    bitfile_rec->acc_id = aid;
    ret_value= HAregister_atom(BITIDGROUP,bitfile_rec);
    bitfile_rec->bit_id=ret_value;
    if (HQuerylength(aid, &bitfile_rec->max_offset) == FAIL)
        HRETURN_ERROR(DFE_INTERNAL, FAIL);
    bitfile_rec->byte_offset = 0;

    bitfile_rec->access = 'r';
    bitfile_rec->mode = 'r';
    bitfile_rec->bytez = bitfile_rec->bytea + BITBUF_SIZE;

    /* pre-read the first block into the buffer */
    if (bitfile_rec->max_offset > bitfile_rec->byte_offset)
      {
          int32       read_size;    /* number of bytes to read into buffer */
          int32       n;        /* number of bytes actually read */

          read_size = MIN((bitfile_rec->max_offset - bitfile_rec->byte_offset), BITBUF_SIZE);
          if ((n = Hread(bitfile_rec->acc_id, read_size, bitfile_rec->bytea)) == FAIL)
              return (FAIL);    /* EOF? somebody pulled the rug out from under us! */
          bitfile_rec->buf_read = (intn) n;    /* keep track of the number of bytes in buffer */
          bitfile_rec->bytep = bitfile_rec->bytea;  /* set to the beginning of the buffer */
      }     /* end if */
    else
      {
          bitfile_rec->bytep = bitfile_rec->bytez;  /* set to the end of the buffer to force read */
          bitfile_rec->buf_read = 0;    /* set the number of bytes in buffer to 0 */
      }     /* end else */
    bitfile_rec->block_offset = 0;
    bitfile_rec->count = 0;

    return(ret_value);
}   /* Hstartbitread() */

/*--------------------------------------------------------------------------

 NAME
       Hstartbitwrite -- set up a bit access elt for a write
 USAGE
       int32 Hstartbitwrite(fileid, tag, ref, len)
       int fileid;             IN: id of file to write to
       int tag;                IN: tag to write to
       int ref;                IN: ref to write to
       long length;            IN: the length of the data element (in bytes)
 RETURNS
       returns id of bit access element if successful and FAIL otherwise
 DESCRIPTION
       Set up a bit-write access elt to write out a data element.  Calls
       Hstartwrite for most initialization and just initializes the bit-
       level stuff here.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
Hstartbitwrite(int32 file_id, uint16 tag, uint16 ref, int32 length)
{
    CONSTR(FUNC, "Hstartbitwrite");     /* for HERROR */
    bitrec_t   *bitfile_rec;    /* access record */
    int32       aid;            /* Access ID for the bit-level routines to use */
    intn        exists;         /* whether dataset exists already */
    int32       ret_value;          /* return bit ID */

    /* clear error stack and check validity of file id */
    HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
        if(HIbitstart()==FAIL)
            HRETURN_ERROR(DFE_CANTINIT, FAIL);

    exists = (Hexist(file_id, tag, ref) == SUCCEED) ? TRUE : FALSE;

    /* Try to get an AID */
    if ((aid = Hstartwrite(file_id, tag, ref, length)) == FAIL)
        HRETURN_ERROR(DFE_BADAID, FAIL);

    /* get empty slot in bit-access records */
    if ((bitfile_rec = HIget_bitfile_rec()) == NULL)
        HRETURN_ERROR(DFE_TOOMANY, FAIL);

    bitfile_rec->acc_id = aid;
    ret_value= HAregister_atom(BITIDGROUP,bitfile_rec);
    bitfile_rec->bit_id=ret_value;
    bitfile_rec->byte_offset = 0;
    bitfile_rec->block_offset = 0;
    if (exists == TRUE)
      {
          if (HQuerylength(aid, &bitfile_rec->max_offset) == FAIL)
              HRETURN_ERROR(DFE_INTERNAL, FAIL);

          /* pre-read the first block into the buffer */
          if (bitfile_rec->max_offset > bitfile_rec->byte_offset)
            {
                int32       read_size;  /* number of bytes to read into buffer */
                int32       n;  /* number of bytes actually read */

                read_size = MIN((bitfile_rec->max_offset - bitfile_rec->byte_offset), BITBUF_SIZE);
                if ((n = Hread(bitfile_rec->acc_id, read_size, bitfile_rec->bytea)) == FAIL)
                    HRETURN_ERROR(DFE_READERROR, FAIL);     /* EOF? somebody pulled the rug out from under us! */
                bitfile_rec->buf_read = (intn) n;  /* keep track of the number of bytes in buffer */
                if (Hseek(bitfile_rec->acc_id, bitfile_rec->block_offset, DF_START) == FAIL)
                    HRETURN_ERROR(DFE_SEEKERROR, FAIL);
            }   /* end if */
      }     /* end if */
    else
      {
          bitfile_rec->max_offset = 0;
          bitfile_rec->buf_read = 0;    /* set the number of bytes in buffer to 0 */
      }     /* end else */
    bitfile_rec->access = 'w';
    bitfile_rec->mode = 'w';
    bitfile_rec->bytez = bitfile_rec->bytea + BITBUF_SIZE;
    bitfile_rec->bytep = bitfile_rec->bytea;    /* set to the beginning of the buffer */
    bitfile_rec->count = BITNUM;
    bitfile_rec->bits = 0;

    return ret_value;
}   /* end Hstartbitwrite() */

/*--------------------------------------------------------------------------

 NAME
       Hbitappendable -- make a bitio AID appendable
 USAGE
       intn Hbitappendable(bitid)
       int32 bitid;         IN: id of bit-element to make appendable
 RETURNS
        SUCCEED for success
        FAIL to indicate failure
 DESCRIPTION
       If a dataset is at the end of a file, allow Hbitwrite()s to write
       past the end of a file.  Allows expanding datasets without the use
       of linked blocks.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
Hbitappendable(int32 bitid)
{
    CONSTR(FUNC, "Hbitappendable");     /* for HERROR */
    bitrec_t   *bitfile_rec;    /* access record */

    /* clear error stack and check validity of file id */
    HEclear();

    if ((bitfile_rec = HAatom_object(bitid)) == NULL)
        HRETURN_ERROR(DFE_ARGS, FAIL);

    /* Check for write access */
    if (bitfile_rec->access != 'w')
        HRETURN_ERROR(DFE_BADACC, FAIL);

    if (Happendable(bitfile_rec->acc_id) == FAIL)
        HRETURN_ERROR(DFE_NOTENOUGH, FAIL);
    return (SUCCEED);
}   /* end Hbitappendable() */

/*--------------------------------------------------------------------------

 NAME
       Hbitwrite -- write a number of bits out to a bit-element
 USAGE
       intn Hbitwrite(bitid, count, data)
       int32 bitid;         IN: id of bit-element to write to
       intn count;          IN: number of bits to write
       uint32 data;         IN: actual data bits to output
                            (bits to output must be in the low bits)
 RETURNS
       the number of bits written for successful write,
       FAIL to indicate failure
 DESCRIPTION
       Write a number of bits out to a bit-element.  This function
       buffers the bits and then writes them out when appropriate
       with Hwrite().
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
Hbitwrite(int32 bitid, intn count, uint32 data)
{
    CONSTR(FUNC, "Hbitwrite");  /* for HERROR */
    static int32 last_bit_id=(-1); /* the bit ID of the last bitfile_record accessed */
    static bitrec_t   *bitfile_rec=NULL; /* access record */
    intn        orig_count = count;     /* keep track of orig, number of bits to output */

    /* clear error stack and check validity of file id */
    HEclear();

    if (count <= 0)
        HRETURN_ERROR(DFE_ARGS, FAIL);

    /* cache the bitfile_record since this routine gets called so many times */
    if(bitid!=last_bit_id)
      {
/* This needs a mutex semaphore when we go to a multi-threaded version of the library -QAK */
        bitfile_rec = HAatom_object(bitid);
        last_bit_id=bitid;
      } /* end if */

    if (bitfile_rec == NULL)
        HRETURN_ERROR(DFE_ARGS, FAIL);

    /* Check for write access */
    if (bitfile_rec->access != 'w')
        HRETURN_ERROR(DFE_BADACC, FAIL);

    if (count > (intn)DATANUM)
        count = (intn)DATANUM;

    /* change bitfile modes if necessary */
    if (bitfile_rec->mode == 'r')
        HIread2write(bitfile_rec);

    data &= maskl[count];

    /* if the new bits will not fill up a byte, then just */
    /* merge the new bits into the current bits buffer */
    if (count < bitfile_rec->count)
      {
          bitfile_rec->bits |= (uint8)(data << (bitfile_rec->count -= count));
          return (orig_count);
      }     /* end if */

    /* fill up the current bits buffer and output the byte */
    *(bitfile_rec->bytep) = (uint8) (bitfile_rec->bits | (uint8)(data >> (count -= bitfile_rec->count)));
    bitfile_rec->byte_offset++;
    if (++bitfile_rec->bytep == bitfile_rec->bytez)
      {
          int32       write_size;

          write_size = (int32)(bitfile_rec->bytez - bitfile_rec->bytea);
          bitfile_rec->bytep = bitfile_rec->bytea;
          if (Hwrite(bitfile_rec->acc_id, write_size, bitfile_rec->bytea) == FAIL)
              HRETURN_ERROR(DFE_WRITEERROR, FAIL);
          bitfile_rec->block_offset += write_size;

          /* check if we should pre-read the next block into the buffer */
          if (bitfile_rec->max_offset > bitfile_rec->byte_offset)
            {
                int32       read_size;  /* number of bytes to read into buffer */
                int32       n;  /* number of bytes actually read */

                read_size = MIN((bitfile_rec->max_offset - bitfile_rec->byte_offset), BITBUF_SIZE);
                if ((n = Hread(bitfile_rec->acc_id, read_size, bitfile_rec->bytea)) == FAIL)
                    HRETURN_ERROR(DFE_READERROR, FAIL);     /* EOF? somebody pulled the rug out from under us! */
                bitfile_rec->buf_read = n;  /* keep track of the number of bytes in buffer */
                if (Hseek(bitfile_rec->acc_id, bitfile_rec->block_offset, DF_START) == FAIL)
                    HRETURN_ERROR(DFE_SEEKERROR, FAIL);
            }   /* end if */
      }     /* end if */

    /* output any and all remaining whole bytes */
    while (count >= (intn)BITNUM)
      {
          *(bitfile_rec->bytep) = (uint8) (data >> (count -= (intn)BITNUM));
          bitfile_rec->byte_offset++;
          if (++bitfile_rec->bytep == bitfile_rec->bytez)
            {
                int32       write_size;

                write_size = (int32)(bitfile_rec->bytez - bitfile_rec->bytea);
                bitfile_rec->bytep = bitfile_rec->bytea;
                if (Hwrite(bitfile_rec->acc_id, write_size, bitfile_rec->bytea) == FAIL)
                    HRETURN_ERROR(DFE_WRITEERROR, FAIL);
                bitfile_rec->block_offset += write_size;

                /* check if we should pre-read the next block into the buffer */
                if (bitfile_rec->max_offset > bitfile_rec->byte_offset)
                  {
                      int32       read_size;    /* number of bytes to read into buffer */
                      int32       n;    /* number of bytes actually read */

                      read_size = MIN((bitfile_rec->max_offset - bitfile_rec->byte_offset), BITBUF_SIZE);
                      if ((n = Hread(bitfile_rec->acc_id, read_size, bitfile_rec->bytea)) == FAIL)
                          HRETURN_ERROR(DFE_READERROR, FAIL);   /* EOF? somebody pulled the rug out from under us! */
                      bitfile_rec->buf_read = n;    /* keep track of the number of bytes in buffer */
                      if (Hseek(bitfile_rec->acc_id, bitfile_rec->block_offset, DF_START) == FAIL)
                          HRETURN_ERROR(DFE_SEEKERROR, FAIL);
                  }     /* end if */
            }   /* end if */
      }     /* end while */

    /* put any remaining bits into the bits buffer */
    if ((bitfile_rec->count = (intn)BITNUM - count) > 0)
        bitfile_rec->bits = (uint8) (data << bitfile_rec->count);

    /* Update the offset in the buffer */
    if (bitfile_rec->byte_offset > bitfile_rec->max_offset)
        bitfile_rec->max_offset = bitfile_rec->byte_offset;

    return (orig_count);
}   /* end Hbitwrite() */

/*--------------------------------------------------------------------------

 NAME
       Hbitread -- read a number of bits from a bit-element
 USAGE
       intn Hbitread(bitid, count, data)
       int32 bitid;         IN: id of bit-element to write to
       intn count;          IN: number of bits to write
       uint32 *data;        IN: pointer to the bits to read
                            OUT: points to the bits read in
                            (bits input will be in the low bits)
 RETURNS
       the number of bits read for successful write,
       FAIL to indicate failure
 DESCRIPTION
       Read a number of bits from a bit-element.  This function
       buffers the bits and then reads them when appropriate
       with Hread().
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
Hbitread(int32 bitid, intn count, uint32 *data)
{
    CONSTR(FUNC, "Hbitread");   /* for HERROR */
    static int32 last_bit_id=(-1); /* the bit ID of the last bitfile_record accessed */
    static bitrec_t   *bitfile_rec=NULL;    /* access record */
    uint32 l;
    uint32      b = 0;          /* bits to return */
    intn        orig_count;     /* the original number of bits to read in */
    int32       n;

    /* clear error stack and check validity of file id */
    HEclear();

    if (count <= 0)
        HRETURN_ERROR(DFE_ARGS, FAIL);

    /* cache the bitfile_record since this routine gets called so many times */
    if(bitid!=last_bit_id)
      {
/* This needs a mutex semaphore when we go to a multi-threaded version of the library -QAK */
        bitfile_rec = HAatom_object(bitid);
        last_bit_id=bitid;
      } /* end if */

    if (bitfile_rec == NULL)
        HRETURN_ERROR(DFE_ARGS, FAIL);

    /* Check for write access */
    /* change bitfile modes if necessary */
    if (bitfile_rec->mode == 'w')
        HIwrite2read(bitfile_rec);

    if (count > (intn)DATANUM)    /* truncate the count if it's too large */
        count = DATANUM;

    /* if the request can be satisfied with just the */
    /* buffered bits then do the shift and return */
    if (count <= bitfile_rec->count)
      {
          *data = (uint32)((uintn)bitfile_rec->bits >> (bitfile_rec->count -= count)) & (uint32) maskc[count];
          return (count);
      }     /* end if */

    /* keep track of the original number of bits to read in */
    orig_count = count;

    /* get all the buffered bits into the correct position first */
    if (bitfile_rec->count > 0)
      {
          b = (uint32)(bitfile_rec->bits & maskc[bitfile_rec->count]);
          b <<= (count -= bitfile_rec->count);
      }     /* end if */

    /* bring in as many whole bytes as the request allows */
    while (count >= (intn)BITNUM)
      {
          if (bitfile_rec->bytep == bitfile_rec->bytez)
            {
                n = Hread(bitfile_rec->acc_id, BITBUF_SIZE, bitfile_rec->bytea);
                if (n == FAIL)
                  {     /* EOF */
                      bitfile_rec->count = 0;   /* make certain that we don't try to access the file->bits information */
                      *data = b;    /* assign the bits read in */
                      return (orig_count - count);  /* break out now */
                  }     /* end if */
                bitfile_rec->block_offset += bitfile_rec->buf_read;     /* keep track of the number of bytes in buffer */
                bitfile_rec->bytez = n + (bitfile_rec->bytep = bitfile_rec->bytea);
                bitfile_rec->buf_read = n;  /* keep track of the number of bytes in buffer */
            }   /* end if */
          l = (uint32) (*bitfile_rec->bytep++);
          b |= (uint32)(l << (count -= (intn)BITNUM));
          bitfile_rec->byte_offset++;
          if (bitfile_rec->byte_offset > bitfile_rec->max_offset)
              bitfile_rec->max_offset = bitfile_rec->byte_offset;
      }     /* end while */

    /* split any partial request with the bits buffer */
    if (count > 0)
      {
          if (bitfile_rec->bytep == bitfile_rec->bytez)
            {
                n = Hread(bitfile_rec->acc_id, BITBUF_SIZE, bitfile_rec->bytea);
                if (n == FAIL)
                  {     /* EOF */
                      bitfile_rec->count = 0;   /* make certain that we don't try to access the file->bits information */
                      *data = b;    /* assign the bits read in */
                      return (orig_count - count);  /* return now */
                  }     /* end if */
                bitfile_rec->block_offset += bitfile_rec->buf_read;     /* keep track of the number of bytes in buffer */
                bitfile_rec->bytez = n + (bitfile_rec->bytep = bitfile_rec->bytea);
                bitfile_rec->buf_read = n;  /* keep track of the number of bytes in buffer */
            }   /* end if */
          bitfile_rec->count = ((intn)BITNUM - count);
          l = (uint32) (bitfile_rec->bits = *bitfile_rec->bytep++);
          b |= l >> bitfile_rec->count;
          bitfile_rec->byte_offset++;
          if (bitfile_rec->byte_offset > bitfile_rec->max_offset)
              bitfile_rec->max_offset = bitfile_rec->byte_offset;
      }     /* end if */
    else
        bitfile_rec->count = 0;

    *data = b;
    return (orig_count);
}   /* end Hbitread() */

/*--------------------------------------------------------------------------

 NAME
       Hbitseek -- seek to a given bit position in a bit-element
 USAGE
       intn Hbitseek(bitid, offset)
       int32 bitid;         IN: id of bit-element to write to
       intn byte_offset;    IN: byte offset in the bit-element
       intn bit_offset;     IN: bit offset from the byte offset

 RETURNS
       returns FAIL (-1) if fail, SUCCEED (0) otherwise.
 DESCRIPTION
       Seek to a bit offset in a bit-element.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
        If seeking to the 15th bit in a bit-element, the call would look like:
            Hbitseek(bitid,1,7);

        Converting from a direct bit offset variable to this call looks like:
            Hbitseek(bitid,bit_offset/8,bit_offset%8);
REVISION LOG
--------------------------------------------------------------------------*/
intn
Hbitseek(int32 bitid, int32 byte_offset, intn bit_offset)
{
    CONSTR(FUNC, "Hbitseek");   /* for HERROR */
    bitrec_t   *bitfile_rec;    /* access record */
    int32       seek_pos;       /* position of block to seek to */
    int32       read_size;      /* number of bytes to read into buffer */
    int32       n;              /* number of bytes actually read */
    intn        new_block;      /* whether to move to another block in the dataset */

    /* clear error stack and check validity of file id */
    HEclear();

    if (byte_offset < 0 || bit_offset < 0 || bit_offset > ((intn)BITNUM - 1)
        || (bitfile_rec = HAatom_object(bitid)) == NULL
        || byte_offset > bitfile_rec->max_offset)
        HRETURN_ERROR(DFE_ARGS, FAIL);

    /* determine whether we need to seek to another block in the file */
    new_block = (byte_offset < bitfile_rec->block_offset
         || byte_offset >= bitfile_rec->block_offset + BITBUF_SIZE)
        ? TRUE : FALSE;
    if (bitfile_rec->mode == 'w')
        if (HIbitflush(bitfile_rec, -1, new_block) == FAIL)     /* flush, but merge */
            HRETURN_ERROR(DFE_WRITEERROR, FAIL);

    if (new_block == TRUE)
      {
          seek_pos = (byte_offset / BITBUF_SIZE) * BITBUF_SIZE;
          if (Hseek(bitfile_rec->acc_id, seek_pos, DF_START) == FAIL)
              HRETURN_ERROR(DFE_SEEKERROR, FAIL);

          read_size = MIN((bitfile_rec->max_offset - seek_pos), BITBUF_SIZE);
          if ((n = Hread(bitfile_rec->acc_id, read_size, bitfile_rec->bytea)) == FAIL)
              HRETURN_ERROR(DFE_READERROR, FAIL);   /* EOF? somebody pulled the rug out from under us! */
          bitfile_rec->bytez = n + (bitfile_rec->bytep = bitfile_rec->bytea);
          bitfile_rec->buf_read = n;    /* keep track of the number of bytes in buffer */
          bitfile_rec->block_offset = seek_pos;
          if (bitfile_rec->mode == 'w')     /* if writing, return the file offset to it's original position */
              if (Hseek(bitfile_rec->acc_id, seek_pos, DF_START) == FAIL)
                  HRETURN_ERROR(DFE_SEEKERROR, FAIL);
      }     /* end if */

    bitfile_rec->byte_offset = byte_offset;

    /* set to the correct position in the buffer */
    bitfile_rec->bytep = bitfile_rec->bytea + (byte_offset - bitfile_rec->block_offset);
    if (bit_offset > 0)
      {
          bitfile_rec->count = ((intn)BITNUM - bit_offset);
          if (bitfile_rec->mode == 'w')
            {   /* if writing, mask off bits not yet written */
                bitfile_rec->bits = *(bitfile_rec->bytep);
                bitfile_rec->bits &= maskc[bit_offset] << bitfile_rec->count;
            }   /* end if */
          else
            {
                bitfile_rec->bits = *bitfile_rec->bytep++;
            }   /* end else */
      }     /* end if */
    else
      {
          if (bitfile_rec->mode == 'w')
            {   /* if writing, mask off bits not yet written */
                bitfile_rec->count = BITNUM;
                bitfile_rec->bits = 0;
            }   /* end if */
          else
            {
                bitfile_rec->count = 0;
            }   /* end else */
      }     /* end else */

    return (SUCCEED);
}   /* end Hbitseek() */

/*--------------------------------------------------------------------------

 NAME
       Hgetbit -- read 1 bit from a bit-element
 USAGE
       intn Hgetbit(bitid)
       int32 bitid;         IN: id of bit-element to read from
 RETURNS
       the bit read in (0/1) on success, FAIL(-1) to indicate failure
 DESCRIPTION
       Read one bit from a bit-element.  This function is mostly a wrapper
       around Hbitread.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
Hgetbit(int32 bitid)
{
    CONSTR(FUNC, "Hgetbit");    /* for HERROR */
    uint32      data;

    if (Hbitread(bitid, 1, &data) == FAIL)
        HRETURN_ERROR(DFE_BITREAD, FAIL)
            return ((intn) data);
}   /* end Hgetbit() */

#ifdef OLD_WAY
/*--------------------------------------------------------------------------

 NAME
       Hputbit -- write 1 bit to a bit-element
 USAGE
       intn Hputbit(bitid,bit)
       int32 bitid;         IN: id of bit-element to read from
       intn bit;            IN: bit to write
 RETURNS
       SUCCEED on success, FAIL(-1) to indicate failure
 DESCRIPTION
       Write one bit to a bit-element.  This function is mostly a wrapper
       around Hbitwrite.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn
Hputbit(int32 bitid, intn bit)
{
    CONSTR(FUNC, "Hputbit");    /* for HERROR */

    if (Hbitwrite(bitid, 1, (uint32) bit) == FAIL)
        HRETURN_ERROR(DFE_BITWRITE, FAIL)
            return (SUCCEED);
}   /* end Hputbit() */
#endif /* OLD_WAY */

/*--------------------------------------------------------------------------

 NAME
       Hendbitaccess -- to dispose of a bitfile element
 USAGE
       int32 Hendbitaccess(bitfile_id,flushbit)
       int32 bitfile_id;        IN: id of bitfile element to dispose of
       intn flushbit;           IN: determines how to flush leftover bits
                                   (leftover bits are bits that have been
                                    buffered, but are less than the
                                    BITNUM (usually set to 8) number of
                                    bits)
                                    0 - flush with zeros
                                    1 - flush with ones
                                   -1 - throw away any leftover bits
 RETURNS
       returns SUCCEED (0) if successful, FAIL (-1) otherwise
 DESCRIPTION
       Used to dispose of a bitfile element.  Flushes any buffered bits
       to the dataset (if writing), and then calls Hendaccess.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
int32
Hendbitaccess(int32 bitfile_id, intn flushbit)
{
    CONSTR(FUNC, "Hendbitaccess");  /* for HERROR */
    bitrec_t   *bitfile_rec;    /* bitfile record */

    /* check validity of access id */
    bitfile_rec = HAatom_object(bitfile_id);
    if (bitfile_rec == NULL)
        HRETURN_ERROR(DFE_ARGS, FAIL);

    if (bitfile_rec->mode == 'w')
        if (HIbitflush(bitfile_rec, flushbit, TRUE) == FAIL)
            HRETURN_ERROR(DFE_WRITEERROR,FAIL);
    HDfree((VOIDP) bitfile_rec->bytea);    /* free the space for the buffer */

    if(HAremove_atom(bitfile_id)==NULL) 
        HRETURN_ERROR(DFE_WRITEERROR,FAIL);
    if(Hendaccess(bitfile_rec->acc_id)==FAIL)
        HRETURN_ERROR(DFE_CANTENDACCESS,FAIL);
    HDfree(bitfile_rec);

    return (SUCCEED);
}   /* end Hendbitaccess() */

/*--------------------------------------------------------------------------
 NAME
    HIbitstart
 PURPOSE
    Bit I/O initialization routine
 USAGE
    intn HIbitstart()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    One-time initialization of the interface
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE intn HIbitstart(void)
{
    CONSTR(FUNC, "HIbitstart");    /* for HERROR */
    intn        ret_value = SUCCEED;

    /* Don't call this routine again... */
    library_terminate = TRUE;

    /* Create the file ID and access ID groups */
    if(HAinit_group(BITIDGROUP,16)==FAIL)
      HGOTO_ERROR(DFE_INTERNAL, FAIL);

done:
  if(ret_value == FAIL)   
    { /* Error condition cleanup */

    } /* end if */

  /* Normal function cleanup */
    return(ret_value);
} /* end HIbitstart() */

/*--------------------------------------------------------------------------

 NAME
    HIbitflush -- flush the bits out to a writable bitfile
 USAGE
    intn HIbitflush(bitfile_rec,flushbit)
        bitrec_t *bitfile_rec;  IN: record of bitfile element to flush
        intn flushbit;          IN: determines how to flush leftover bits
                                   (leftover bits are bits that have been
                                    buffered, but are less than the
                                    BITNUM (usually set to 8) number of
                                    bits)
                                    0 - flush with zeros
                                    1 - flush with ones
                                   -1 - throw away any leftover bits
 RETURNS
    returns SUCCEED (0) if successful, FAIL (-1) otherwise
 DESCRIPTION
    Used to flush the buffer of a bitfile element, preserving the bits
    in the buffer which have not been modified.  The flushbits parameter
    is only used when the last bits written to the element are at the
    actual end of the dataset, not somewhere in the middle.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    This routine does _not_ leave the bitfile in a position to continue
    I/O from the current point, additional modifications would have to be
    made in order to accomodate this.
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE intn
HIbitflush(bitrec_t * bitfile_rec, intn flushbit, intn writeout)
{
    CONSTR(FUNC, "HIbitflush");
    intn        write_size;     /* number of bytes to write out */

    if (bitfile_rec->count < (intn)BITNUM)
      {     /* check if there are any */
          if (bitfile_rec->byte_offset > bitfile_rec->max_offset)
            {
                if (flushbit != (-1))   /* only flush bits if asked and there are bits to flush */
                    if (Hbitwrite(bitfile_rec->bit_id, bitfile_rec->count, (uint32) (flushbit ? 0xFF : 0)) == FAIL)
                        HRETURN_ERROR(DFE_WRITEERROR, FAIL);
            }   /* end if */
          else
            {   /* we are in the middle of a dataset and need to integrate */
                /* mask off a place for the new bits */
                *(bitfile_rec->bytep) &= (uint8)(~(maskc[(intn)BITNUM - bitfile_rec->count] << bitfile_rec->count));

                /* merge in new bits */
                *(bitfile_rec->bytep) |= bitfile_rec->bits;

                bitfile_rec->bytep++;
                bitfile_rec->byte_offset++;
                
                /* Update the offset in the buffer */
                if (bitfile_rec->byte_offset > bitfile_rec->max_offset)
                    bitfile_rec->max_offset = bitfile_rec->byte_offset;

                bitfile_rec->count = BITNUM;    /* reset count */
                bitfile_rec->bits=0;            /* reset bits */
            }   /* end else */
      }     /* end if */
    if (writeout == TRUE)
      {     /* only write data out if necessary */
          write_size = (intn) MIN((bitfile_rec->bytez - bitfile_rec->bytea),bitfile_rec->max_offset);
          if (write_size > 0)
              if (Hwrite(bitfile_rec->acc_id, write_size, bitfile_rec->bytea) == FAIL)
                  HRETURN_ERROR(DFE_WRITEERROR, FAIL);
      }     /* end if */

    return (SUCCEED);
}   /* HIbitflush */

/*--------------------------------------------------------------------------
 HIget_bitfile_rec - get a new bitfile record 
--------------------------------------------------------------------------*/
PRIVATE bitrec_t *
HIget_bitfile_rec(void)
{
    CONSTR(FUNC, "HIget_bitfile_rec");
    bitrec_t *ret_value=NULL;

    ret_value = (bitrec_t *) HDcalloc(1, sizeof(bitrec_t));
    if ((ret_value->bytea = (uint8 *) HDmalloc(BITBUF_SIZE)) == NULL)
        HRETURN_ERROR(DFE_NOSPACE, NULL);

    return ret_value;
}   /* HIget_bitfile_rec */

/*--------------------------------------------------------------------------

 NAME
    HIread2write - switch from reading bits to writing them
 USAGE
    intn HIread2write(bitfile_rec)
        bitrec_t *bitfile_rec;  IN: record of bitfile element to switch
 RETURNS
    returns SUCCEED (0) if successful, FAIL (-1) otherwise
 DESCRIPTION
    Used to switch a bitfile (which has 'w' access) from read mode to write
    mode, at the same bit offset in the file.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE intn
HIread2write(bitrec_t * bitfile_rec)
{
    CONSTR(FUNC, "HIread2write");

    bitfile_rec->block_offset = (int32)LONG_MIN;    /* set to bogus value */
    bitfile_rec->mode = 'w';    /* change to write mode */
    if (Hbitseek(bitfile_rec->bit_id, bitfile_rec->byte_offset, ((intn)BITNUM - bitfile_rec->count)) == FAIL)
        HRETURN_ERROR(DFE_INTERNAL, FAIL);
    return (SUCCEED);
}   /* HIread2write */

/*--------------------------------------------------------------------------

 NAME
    HIwrite2read - switch from writing bits to reading them
 USAGE
    intn HIwrite2read(bitfile_rec)
        bitrec_t *bitfile_rec;  IN: record of bitfile element to switch
 RETURNS
    returns SUCCEED (0) if successful, FAIL (-1) otherwise
 DESCRIPTION
    Used to switch a bitfile (which has 'w' access) from write mode to read
    mode, at the same bit offset in the file.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
PRIVATE intn
HIwrite2read(bitrec_t * bitfile_rec)
{
    CONSTR(FUNC, "HIwrite2read");
    intn       prev_count = bitfile_rec->count;    /* preserve this for later */
    int32       prev_offset = bitfile_rec->byte_offset;

    if (HIbitflush(bitfile_rec, -1, TRUE) == FAIL)  /* flush any leftover bits */
        HRETURN_ERROR(DFE_WRITEERROR, FAIL);

    bitfile_rec->block_offset = (int32)LONG_MIN;    /* set to bogus value */
    bitfile_rec->mode = 'r';    /* change to read mode */
    if (Hbitseek(bitfile_rec->bit_id, prev_offset, ((intn)BITNUM - prev_count)) == FAIL)
        HRETURN_ERROR(DFE_INTERNAL, FAIL);
    return (SUCCEED);
}   /* HIwrite2read */

/*--------------------------------------------------------------------------
 NAME
    HPbitshutdown
 PURPOSE
    Terminate various static buffers.
 USAGE
    intn HPbitshutdown()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Free various buffers allocated in the Hbit routines.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Should only ever be called by the "atexit" function HDFend
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn HPbitshutdown(void)
{
    /* Shutdown the file ID atom group */
    HAdestroy_group(BITIDGROUP);

    return(SUCCEED);
} /* end HPbitshutdown() */

