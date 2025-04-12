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
static char RcsId[] = "@(#)$Revision: 5080 $";
#endif

/* $Id: hchunks.c 5080 2008-10-01 17:59:05Z bmribler $ */

/*LINTLIBRARY */
/* ------------------------------ HMCxxx -------------------------------
   Routines to implement chunked elements via a Vdatas for
   the chunk table and using a new data tag DFTAG_CHUNK to represent
   each chunk object. As a result the *total* number of chunks
   for all the chunked elements in an HDF file can only be as 
   large as sizeof(uint16) = 65,536(i.e. number of refs).

   This layer only has to deal with Chunks from a STDIO programming
   model as this how special elements are viewed by other API's in the library.
   The layers above deal with the more complex issues 
   of deciding what data to read/write next given the users request.
   This layer basically chunks the element from a stream of bytes.
   Note that this is different than if the layer was integrated
   with say the SDS layer.
   
   NOTE: GeorgeV's standard Disclaimer <here>. 
         I was coerced to do it this way....:-)
         If you break it .....you fix it...

   Description of file format headers for chunked element
   ------------------------------------------------------
   A chunked element is a special element.

   Special elements are
   flagged with a set high-bit in their tag.  Thus, a tag t has
   BASETAG == t & 0x7f and is a special tag if t & 0x80 != 0x00

   The first 16 bits of the meta-element that this tag/ref points to
   tells us what type of special element this is.  If these 16 bits is
   SPECIAL_CHUNKED, then it contains information about the chunked element.
   After this 16 bits, 32 bit which is the length of each chunk, after
   which is the information header:

   File Description of Chunked Element
   ****************************************

   NOTE: I know some of the fields could be 1 byte instead of 4 bytes
         but I decided to make them 4 to allow the fields to change
         their behaviour in the future.....i.e some could hold tag/refs..

   DD for Chunked Element pointing to Chunked Description Record(12 byes )
   =======================================================================
   <-  2 bytes -> <- 2 bytes -> <- 4 bytes -> <- 4bytes ->
   --------------------------------------------------------
   |extended tag | reference # |  Offset     |  Length    |
   --------------------------------------------------------
                                    \______________/
   __________________________________________|
   V
   CHUNKED DESCRIPTION RECORD(6 + 9 + 12 + 8 + 12 x ndims + 4 + fill_val_len + 
                              'specialness' bytes) > 52 bytes
   ============================================================================
   <-  2 bytes -> <-  4 bytes  -> (6 bytes)
   --------------------------------
   |sp_tag_desc  |sp_tag_head_len | ... cont'd
   --------------------------------

   <-1 byte-> <- 4 bytes -> <-  4bytes  -> ( 9 bytes)
   ------------------------------------------
   | version |    flag     | elm_tot_length |... cont'd
   ------------------------------------------
        
   <- 4 bytes  -> <- 4 bytes  -> <- 2 bytes -> <- 2 bytes -> (12 bytes)
   ---------------------------------------------------------
...| chunk_size  |  nt_size     | chk_tbl_tag | chk_tbl_ref | ...cont'd
   ---------------------------------------------------------

   <- 2 bytes -> <- 2 bytes -> <- 4 bytes -> (8 bytes)
   ----------------------------------------
...| sp_tag     | sp_ref      |  ndims   |  ...cont'd
   ----------------------------------------

   <- 4 bytes   -> <- 4 bytes -> <- 4 bytes -> (12 x ndims bytes)
   --------------------------------------------
...|    flag      | dim_length  | chunk_length |  ... x Number of dimensions
   --------------------------------------------

   <-      4 bytes    -> <- variable bytes -> 
   ------------------------------------------
...| fill_val_num_bytes | fill value......  |   ...(optional cont'd)
   ------------------------------------------

          Optinal number of these depending on multiple 'specialness'
          set in 'flag' field.

   <- 2 byte   -> <-      4 bytes  -> <- variable bytes -> (6 + variable bytes)
   --------------------------------------------------------
...| sp_tag_desc | sp_tag_header len | sp_tag_header......|...
   --------------------------------------------------------

   Fields
   ------
   sp_tag_desc     - SPECIAL_CHUNKED(16 bit constant), identifies this as
                     a chunked element description record
   sp_tag_head_len - length of this special element header only.(4 bytes)
                     Does not include length of header with additional 
                     'specialness' headers.
                     NOTE: This is done to make this header layout similiar to the 
                           multiple 'specialiness' layout.
   version        - version info (8 bit field)
   flag           - bit field to set additional specialness  (32 bit field)
                    only bottom 8bits used for now.
   elem_tot_len   - Valid logical Length of the entire element(4 bytes)
                    The logical physical length is this value multiplied
                    by 'nt_size'.
                    The actual physical length used for storage can be
                    greater than the dataset size due to ghost areas in
                    chunks. Partial chunks are not distinguished from
                    regular chunks.
   chunk_size     - logical size of data chunks(4 bytes)
   nt_size        - Number type size i.e size of data type (4 bytes)
   chk_tbl_tag    - Tag of chunk table i.e. Vdata (2 bytes)
   chk_tbl_ref    - Reference number of the chunk table
                    i.e. Vdata (2 bytes)
   sp_tag         - For future use i.e. special table for 'ghost' chunks(2 bytes)
   sp_ref         - For future use(2 bytes)
   ndims          - number of dimensions for the chunked element.(4 bytes)
   file_val_num_bytes - number of bytes in fill value (4 bytes)
   fill value         - fill value (variable bytes)

   Fields for each dimension: (12 x ndims bytes)
   --------------------------------------
   flag           - (32 bit field) 
                    |High 8bits|Medium High 8bits|Medium Low 8bit|Low 8bits|
                    o distrib_type (Low 8 bits, bits 0-7) - 
                      type of data distribution along this dimension 
                      0x00->None, 
                      0x01->Block
                      Currently only block distribution is supported but
                      this is not checked or verified for now.
                    o other (Medium Low 8 bits, bits 7-15)
                      0x00->Regular dimension, 
                      0x01->UNLIMITED dimension
   dim_length     - current length of this dimension. (4 bytes)
   chunk_length   - length of the chunk along this dimension (4 bytes)

   Fields for each additional 'specialness' (Optional)
   -------------------------------------------
   sp_tag_desc    - SPECIAL_xxx(16 bit constant), identifies this as
                    a 'xxx' element description record .(16 bit field)
   sp_tag_header_len - length of special element header(4 bytes)
   sp_tag_header     - special header.(variable bytes)


   Chunk Table(variable bytes per record in Vdata due to size of origin)
   ====================================================================
   <-  variable bytes -> <- 2 bytes -> <- 2 bytes -> 
   -------------------------------------------------
   |      origin        |  chunk_tag  | chunk_ref_1 |   
   -------------------------------------------------
           -                 -             -             N is number of
           -                 -             -             chunk records
           -                 -             -             in Vdata
   -------------------------------------------------
   |      origin        |  chunk_tag  | chunk_ref_N |
   -------------------------------------------------
                                 \______________/
   __________________________________________|
   V
    <-   2 bytes  -> <- 2 bytes -> <- 4 bytes -> <- 4bytes ->
    --------------------------------------------------------
    | DFTAG_CHUNK   | chunk_ref_N |  Offset     |  Length    |
    --------------------------------------------------------
                                         \______________/
        __________________________________________|
        V
        -----------------------
        | Data_chunk          |
        -----------------------
                  Note: The "Length" here is specified by "chk_size" x "nt_size". 

   Fields
   ------
   origin    - specifies the coordinates of the chunk in the overall
               chunk array. (variable field, depends on number of 
               dimensions of chunked element).
   chunk_tag - DFTAG_CHUNK for now(could be another chunked element to
               allow recursive chunked elements(DFTAG_CHUNKED)) (16 bit field)
   chunk_ref - Reference number of chunk itself   (16 bit field)

EXPORTED ROUTINES
=================
   User Public  
   -----------
   HMCcreate       -- create a chunked element
   HMCwriteChunk   -- write out the specified chunk to a chunked element
   HMCreadChunk    -- read the specified chunk from a chunked element
   HMCsetMaxcache  -- maximum number of chunks to cache 
   HMCPcloseAID    -- close file but keep AID active (For Hnextread())

   Library Private
   ---------------
   HMCPstread      -- open an access record for reading
   HMCPstwrite     -- open an access record for writing
   HMCPseek        -- set the seek posn
   HMCPchunkread   -- read a single chunk out of a chunked element
   HMCPread        -- read some data out of a chunked element
   HMCPchunkwrite  -- write out a single chunk to a chunked element
   HMCPwrite       -- write out some data to a chunked element
   HMCPinquire     -- Hinquire for chunked element
   HMCPendacess    -- close a chunked element AID
   HMCPinfo        -- return info about a chunked element
   HMCPgetnumrecs  -- get the number of records in a chunked element

   TBBT helper rotuines
   -------------------
   chkcompare      -- comprares 2 chunk records
   chkfreekey      -- frees chunk key
   chkdestroynode  -- destroys chunk record

LOCAL ROUTINES
==============
   Chunking helper routines
   ------------------------
   create_dim_recs           -- create the appropriate arrays in memory
   update_chunk_indices_seek -- translate seek pos to chunk and pos in chunk
   compute_chunk_to_seek     -- translate chunk coordiantes to seek postion
   update_chunk_indices      -- not used
   compute_array_to_chunk    -- not used
   calculate_num_to_chunk    -- not used
   compute_chunk_to_array    -- translate chunk arrays to user array
   compute_array_to_seek     -- translate user array to user seek position
   calculate_seek_in_chunk   -- translate pos in chunk to seek pos in chunk
   update_seek_pos_chunk     -- update chunk seek array with seek pos in chunk 
   calculate_chunk_num       -- translate chunk coordinates to a number
   calculate_chunk_for_chunk -- calculate number of bytes to operate on chunk

   Common Routine
   -------------
   HMCIstaccess -- set up AID to access a chunked element

   AUTHOR 
   ------- 
   -GeorgeV - 9/3/96
*/

/* For debugging */
/*
#define CHK_DEBUG_1
#define CHK_DEBUG_2
#define CHK_DEBUG_3
#define CHK_DEBUG_4
#define CHK_DEBUG_5
#define CHK_DEBUG_10
*/


/* For Statistics from the chunk cache.
   Note thate 'mache.c' must be compilied with -DSTATISTICS */
/*
#define STATISTICS 
*/

#define  _HCHUNKS_MAIN_  /* Master chunk handling file */
#include "hdf.h"
#include "hfile.h"
#include "mcache.h" /* cache */
#include "hchunks.h"

/* private functions */
PRIVATE int32 
HMCIstaccess(accrec_t * access_rec,  /* IN: access record to fill in */
             int16 acc_mode          /* IN: access mode */ );

/* -------------------------------------------------------------------------
NAME
    create_dim_recs -- create the appropriate arrays in memory
DESCRIPTION
    Given number of dimensions create the following 3 arrays.
    1. Dimension record array contains a record for each dimension.
    2. Seek chunk indice array contains the seek postion relative to
       the logical representation of the chunked array.
    3. The seek position chunk array contains the seek postion 
       relative to the chunk itself.
    4. The user array contains the users seek postion in the element
RETURNS
    SUCCEED/FAIL

AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
PRIVATE int32
create_dim_recs(DIM_REC **dptr, /* OUT: dimension record pointers */
                int32  **sbi,   /* OUT: seek chunk indices array */
                int32  **spb,   /* OUT: seek pos w/ chunk array */
                int32  **sui,   /* OUT: seek user indicies array */
                int32 ndims     /* IN: number of dimension of element */)
{
    CONSTR(FUNC, "create_dim_recs");   /* for HERROR */
    int32 i;
    int32 ret_value = SUCCEED;

    /* allocate space for demension records pointers */
    if ((*dptr = (DIM_REC *)HDmalloc(sizeof(DIM_REC) * (size_t)ndims)) == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* allocate space for seek chunk indices and chunk seek positions */
    if ((*sbi = (int32 *)HDmalloc(sizeof(int32)*(size_t)ndims)) == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    if ((*spb = (int32 *)HDmalloc(sizeof(int32)*(size_t)ndims)) == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* allocate space for user seek indicies */
    if ((*sui = (int32 *)HDmalloc(sizeof(int32)*(size_t)ndims)) == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* for each dimension */
    for (i = 0; i < ndims; i++)
      {
          /* Initialize values for dimension record */
          (*dptr)[i].flag = 0;
          (*dptr)[i].dim_length = 0;
          (*dptr)[i].chunk_length = 0;
          (*dptr)[i].distrib_type = 0;
          (*dptr)[i].unlimited = 0;
          (*dptr)[i].last_chunk_length = 0;
          (*dptr)[i].num_chunks = 0;

          (*sbi)[i] = 0;
          (*spb)[i] = 0;
          (*sui)[i] = 0;
      } /* end for i */

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
          if (*dptr != NULL)
              HDfree(*dptr);
          if (*sbi != NULL)
              HDfree(*sbi);
          if (*spb != NULL)
              HDfree(*spb);
          if (*sui != NULL)
              HDfree(*sui);
      } /* end if */

    /* Normal function cleanup */
    return ret_value;
} /* end create_dim_recs() */

/* -------------------------------------------------------------------------
NAME
    update_chunk_indicies_seek -- translate seek pos to chunk and pos in chunk
DESCRIPTION
    Give seek location within an element, calculate which chunk in
    chunk array and position within chunk.
RETURNS
    Nothing
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
PRIVATE void
update_chunk_indicies_seek(int32 sloc,    /* IN: physical Seek loc in element */
                           int32 ndims,   /* IN: number of dimensions of elem */
                           int32 nt_size, /* IN: number type size */
                           int32 *sbi,    /* IN: seek chunk indicies array */
                           int32 *spb,    /* IN: seek pos w/ chunk array */
                           DIM_REC *ddims /* IN: dim record ptrs */)
{
    int32 i;
    int32 stmp;

    /* adjust physical seek->logical seek by using number type size */
    stmp = sloc / nt_size;
#ifdef CHK_DEBUG_1
          printf("ucis: sloc=%d, stmp=%d \n", sloc,stmp);
#endif
    for(i = ndims - 1; i >= 0 ; i--) 
      { /* Calculate which chunk index in chunk representation */
          sbi[i] = (int32)((stmp % ddims[i].dim_length) 
                           / ddims[i].chunk_length);
          /* calculate starting postion in the chunk itself */
          spb[i] = (int32)((stmp % ddims[i].dim_length) 
                           % ddims[i].chunk_length);

          stmp = stmp / ddims[i].dim_length;
      } /* end for i */
#ifdef CHK_DEBUG_1
    printf("ucis: chunk_array =(");
    for(i = 0; i < ndims; i++) 
        printf("%d%s", sbi[i], i!= ndims-1 ? ",":NULL);
    printf(")\n");

    printf("ucis: chunk_pos_array =(");
    for(i = 0; i < ndims; i++) 
        printf("%d%s", spb[i], i!= ndims-1 ? ",":NULL);
    printf(")\n");
#endif 
} /* update_chunk_indicies_seek()*/

#ifdef UNUSED
/* -------------------------------------------------------------------------
NAME
    compute_chunk_to_seek -- translate chunk coordinates to chunk seek postion
DESCRIPTION
    Calculate new chunk seek postion given seek chunk array and seek postion
    within that chunk array. 
RETURNS
    Nothing
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
PRIVATE void
compute_chunk_to_seek(int32 *chunk_seek, /* OUT: new physical chunk seek pos in element*/
                     int32 ndims,       /* IN: number of dims */
                     int32 nt_size,     /* IN: number type size */
                     int32 *sbi,        /* IN: seek chunk array */
                     int32 *spb,        /* IN; seek pos w/ chunk array */
                     DIM_REC *ddims,    /* IN: dim record ptrs */
                     int32 chunk_size   /* IN: physical size of chunk */)
{
    int32 j;
    int32 new_seek;
    int32 l_chunk_size = 0;

    /* Adjust physical chunk_size -> logical chunk size by size of number type */
    l_chunk_size = chunk_size / nt_size;

    /* Calculate Seek Location in element 
     * First calculste seek-chunk position in element
     * i.e seek position according to chunk first */
    *chunk_seek = sbi[ndims -1];
    for(j = ndims - 1; j; j--) 
      {
          *chunk_seek = (*chunk_seek * ddims[j-1].num_chunks)
              + sbi[j-1]; 
      }

    /* must get chunk_size from somewhere else
     * to give us position in file relative to chunk.
     * Next comes adjustment of seek for postion inside chunk*/
    *chunk_seek *= l_chunk_size; 
#ifdef CHK_DEBUG_1
    printf("ccs:  chunk_seek = %d(chunk# %d)\n", *chunk_seek,
           *chunk_seek/l_chunk_size);       
#endif
    /* Calculate seek position in chunk */
    new_seek = spb[ndims - 1];
    for(j = ndims - 1; j; j--) 
      {
          new_seek = (new_seek * ddims[j - 1].chunk_length) 
              + spb[j - 1];
      }

    /* add seek position in chunk to seek-chunk offset */
    new_seek += *chunk_seek;
#ifdef CHK_DEBUG_1
    printf("ccs:   calculated seek position in file is %d\n", new_seek);
#endif

    /* multiply by number type size to get new physical seek positon */
    *chunk_seek = new_seek * nt_size;

} /* compute_chunk_to_seek() */
#endif /* UNUSED */

#if 0 /* NOT USED */

/* -------------------------------------------------------------------------
NAME
    update_chunk_indices
DESCRIPTION
    Given chunk size and current chunk in chunk array and postion within
    that chunk, update to new chunk in chunk array and postion within 
    the new chunk.
RETURNS
    Nothing
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
PRIVATE void
update_chunk_indices(int32 chunk_size, /* IN: physical size of chunk read/written*/
                     int32 ndims,      /* IN: number of dimensions */
                     int32 nt_size,    /* IN: number type size */
                     int32 *sbi,       /* IN: seek chunk indicies array */
                     int32 *spb,       /* IN: seek pos w/ chunk array */
                     DIM_REC *ddims    /* IN: dim record ptrs */)
{
    int32 change = 1;
    int32 index = 0;
    int32 l_chunk_size = 0;
#ifdef CHK_DEBUG_1
    int32 i;
#endif

    /* Adjust physical chunk_size -> logical chunk size by size of number type */
    l_chunk_size = chunk_size / nt_size;
#ifdef CHK_DEBUG_1
    printf("uci: l_chunk_size=%d \n", l_chunk_size);
#endif
    while(change && index < ndims) 
      { /* add chunk written to current chunk dimension */
          spb[index] += l_chunk_size;
          change = 0;
          if(spb[index] == ddims[index].chunk_length) 
            { /* we've move to next chunk since we filled the previous one */
#ifdef CHK_DEBUG_1
                fprintf(stderr,"uci: going to next chunk along spb[%d] to access\n",
                        index);
#endif
                spb[index] = 0; /* position at beginning of chunk */
                if(++(sbi[index]) == ddims[index].num_chunks) 
                  { /* we've written to all the chunks in this demension, 
                       so reset for this dimension */
#ifdef CHK_DEBUG_1
                    fprintf(stderr,"uci: accessed all chunks along sbi[%d] so reset \n", 
                            index);
#endif
                      sbi[index] = 0;
                      change = 1;
                  } 
                index++; /* go to next dimension */
                l_chunk_size = 1; 
            }
      } /* end while "change" */

#ifdef CHK_DEBUG_1
    printf("uci : sbi =(");
    for(i = 0; i < ndims; i++) 
        printf("%d%s", sbi[i], i!= ndims-1 ? ",":NULL);
    printf(")\n");

    printf("uci : spb =(");
    for(i = 0; i < ndims; i++) 
        printf("%d%s", spb[i], i!= ndims-1 ? ",":NULL);
    printf(")\n");
#endif 
} /* update_chunk_indices() */

/* -------------------------------------------------------------------------
NAME
    compute_array_to_chunk
DESCRIPTION
    Calculate chunk array indicies and position within chunk 
    given user array indicies i.e. translates array postion in user 
    array to position in overall chunk array and within the chunk.
RETURNS
    Nothing
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
PRIVATE void
compute_array_to_chunk(int32 *chunk_indicies, /* OUT: chunk indicies */
                       int32 *chunk_array_ind,/* OUT: chunk array indices */
                       int32 *array_indicies, /* IN: array indicies */
                       int32 ndims,           /* IN: number of dims */
                       DIM_REC *ddims         /* IN: dim record ptrs */ )
{
    int32 j;

    for(j = 0; j < ndims; j++) 
      {   /* set postion in overall chunk array */
          chunk_indicies[j] = array_indicies[j] / ddims[j].chunk_length;
          /* set postion within the chunk itself */
          chunk_array_ind[j] = array_indicies[j] % ddims[j].chunk_length;
      }
} /* compute_array_to_chunk() */


/* -------------------------------------------------------------------------
NAME
    calculate_num_to_chunk
DESCRIPTION
    Calculate seek chunk array given chunk number

    Not implemented
RETURNS
    Nothing
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
PRIVATE void
calculate_num_to_chunk(int32 chunk_num, /* IN: chunk number within element */
                       int32 ndims,     /* IN: number of dims */
                       int32 *sbi,      /* IN/OUT: seek chunk array */
                       DIM_REC *ddims   /* IN: dim record ptrs */ )
{
    int32 j;
    int nchunk_num = chunk_num;
   

} /* calculate_num_chunk() */

#endif /* NOT USED */

/* -------------------------------------------------------------------------
NAME
    compute_chunk_to_array -- translate chunk arrays to user array
DESCRIPTION
    Calculate user array indicies given overall array chunk indicies 
    and position within chunk. 
RETURNS
    Nothing
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
PRIVATE void
compute_chunk_to_array(int32 *chunk_indicies, /* IN: chunk indicies */
                       int32 *chunk_array_ind,/* IN: chunk array indices */
                       int32 *array_indicies, /* OUT: array indicies */
                       int32 ndims,           /* IN: number of dims */
                       DIM_REC *ddims         /* IN: dim record ptrs */ )
{
    int32 j;

    for(j = 0; j < ndims; j++) 
      {   /* set postion in using overall chunk array */
          array_indicies[j] = chunk_indicies[j] * ddims[j].chunk_length;

          /* set postion  using the chunk itself 
             need to adjust for last chunk along each dimension */
          if (chunk_indicies[j] == (ddims[j].num_chunks -1))
            { /* last chunk along this dimension */
              array_indicies[j] += (chunk_array_ind[j] > ddims[j].last_chunk_length)?
                  ddims[j].last_chunk_length : chunk_array_ind[j];
            }
          else /* not last chunk along a dimension */
              array_indicies[j] +=  chunk_array_ind[j];
      }

#ifdef CHK_DEBUG_1
          printf("ccta: array_indicies:(");
          for (j = 0; j < ndims; j++)
              printf("%d%s", array_indicies[j], j!= ndims-1 ? ",":NULL);
          printf(")\n");
#endif
} /* compute_chunk_to_array() */

/* -------------------------------------------------------------------------
NAME
    compute_array_to_seek -- translate user array to user seek position
DESCRIPTION
    Computer user seek postion within element given user array.
RETURNS
    Nothing
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
PRIVATE void
compute_array_to_seek(int32 *user_seek,      /* OUT: user seek */
                      int32 *array_indicies, /* IN: user array indicies */
                      int32 nt_size,         /* IN: number type size */
                      int32 ndims,           /* IN: number of dims */
                      DIM_REC *ddims         /* IN: dim record ptrs */ )
{
    int32 j;
    int32 cnum;

    /* Calculate seek position within user array */
    *user_seek = array_indicies[ndims - 1];
    if (ndims > 1)
      {   
          cnum = 1;
          for(j = ndims - 2; j >= 0 ; j--) 
            {
                cnum *= ddims[j + 1].dim_length;      
                *user_seek += (array_indicies[j] * cnum );
            }
      }

    /* multiply by number type size to get new physical user seek positon */
    *user_seek = *user_seek * nt_size;
   
} /* compute_array_to_seek() */

/* -------------------------------------------------------------------------
NAME
    calculate_seek_in_chunk -- translate pos in chunk to seek pos in chunk
DESCRIPTION
    Calculate seek postion within chunk
RETURNS
    Nothing
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
PRIVATE void
calculate_seek_in_chunk(int32 *chunk_seek,/* OUT: new physical seek pos in element*/
                        int32 ndims,      /* IN: number of dims */
                        int32 nt_size,    /* IN: number type size */
                        int32 *spb,       /* IN; seek pos w/ chunk array */
                        DIM_REC *ddims    /* IN: dim record ptrs */ )
{
    int32 j;
    int32 cnum;

    /* Calculate seek position within chunk */
    *chunk_seek = spb[ndims - 1];
    if (ndims > 1)
      {   
          cnum = 1;
          for(j = ndims - 2; j >= 0 ; j--) 
            {
                cnum *= ddims[j + 1].chunk_length;      
                *chunk_seek += (spb[j] * cnum );
            }
      }

    /* multiply by number type size to get new physical seek positon */
    *chunk_seek = *chunk_seek * nt_size;
   
} /* calculate_seek_in_chunk() */

/* -------------------------------------------------------------------------
NAME
    update_seek_pos_chunk -- update chunk seek array with seek pos in chunk 
DESCRIPTION
    Update chunk seek array with seek pos in chunk. 
RETURNS
    Nothing
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
PRIVATE void
update_seek_pos_chunk(int32 chunk_seek, /* IN: physical seek pos in chunk */
                      int32 ndims,      /* IN: number of dims */
                      int32 nt_size,    /* IN: number type size */
                      int32 *spb,       /* OUT: seek pos w/ chunk array */
                      DIM_REC *ddims    /* IN: dim record ptrs */ )
{
    int32 i;
    int32 stmp;

    /* adjust physical seek->logical seek by using number type size */
    stmp = chunk_seek / nt_size;

    for(i = ndims - 1; i >= 0 ; i--) 
      { 
          /* calculate starting postion in the chunk itself */
          spb[i] = (int32)(stmp % ddims[i].chunk_length);
          stmp = stmp / ddims[i].chunk_length;
      } /* end for i */

#ifdef CHK_DEBUG_1
    printf("uspc: spb[] =(");
    for(i = 0; i < ndims; i++) 
        printf("%d%s", spb[i], i!= ndims-1 ? ",":NULL);
    printf(")\n");
#endif    
} /* update_seek_pos_chunk() */


/* -------------------------------------------------------------------------
NAME
    calculate_chunk_num - translate chunk coordinates to a number
DESCRIPTION
    Calculate new chunk number given seek chunk array and seek postion
    within that chunk array. 
RETURNS
    Nothing
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
PRIVATE void
calculate_chunk_num(int32 *chunk_num, /* OUT: new chunk number within element */
                    int32 ndims,      /* IN: number of dims */
                    int32 *sbi,       /* IN: seek chunk array */
                    DIM_REC *ddims    /* IN: dim record ptrs */ )
{
    int32 j;
    int32 cnum;

    /* Calculate chunk number from overall chunk array indicies */
    *chunk_num = sbi[ndims - 1];
    if (ndims > 1)
      {   
          cnum = 1;
          for(j = ndims - 2; j >= 0 ; j--) 
            {
                cnum *= ddims[j + 1].num_chunks;      
                *chunk_num += (sbi[j] * cnum );
            }
      }

} /* calculate_chunk_num() */

/* -------------------------------------------------------------------------
NAME
    calculate_chunk_for_chunk - calculate number of bytes to operate on chunk
DESCRIPTION
  Given the length of bytes to operate on and the size of bytes
  already operated on, calculate how big of chunk can be written
  to the current chunk.
RETURNS
    Nothing
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
PRIVATE void
calculate_chunk_for_chunk(int32 *chunk_size,   /* OUT: chunk size for this chunk */
                          int32 ndims,         /* IN: number of dims */
                          int32 nt_size,       /* IN: number type size */
                          int32 len,           /* IN: total length to operate on */
                          int32 bytes_finished,/* IN: bytes already operted on*/
                          int32 *sbi,          /* IN: seek chunk array */
                          int32 *spb,          /* IN: seek pos w/ chunk array */
                          DIM_REC *ddims       /* IN: dim record ptrs */)
{
    /* Is this the last chunk along fastest changing dimension(i.e. subscript).
       In future maybe need to handle variable case of any dimension being
       the fastest. */
    if (sbi[ndims - 1] == (ddims[ndims - 1].num_chunks - 1))
      { /* last chunk */
          /* Calculate size of chunk to write for the last chunk */
          if ((ddims[ndims -1].last_chunk_length - spb[ndims - 1]) * nt_size 
              > (len - bytes_finished))
              *chunk_size = len - bytes_finished; /* less than a chunk to write */
          else /* last full chunk */
              *chunk_size = (ddims[ndims - 1].last_chunk_length - spb[ndims -1]) * nt_size; 

      }
    else /* not the last chunk */
      {
          /* Calculate size of chunk to write in this chunk */
          if ((ddims[ndims -1].chunk_length - spb[ndims - 1]) * nt_size 
              > (len - bytes_finished))
              *chunk_size = len - bytes_finished; /* less than a chunk to write */
          else /* full chunk */
              *chunk_size = (ddims[ndims - 1].chunk_length - spb[ndims -1]) * nt_size; 
      }
} /* calculate_chunk_for_chunk() */

/* -------------------------------------------------------------------------
NAME
    chkcompare
DESCRIPTION
   Compares two chunk B-tree keys for equality.  Similar to memcmp.

   *** Only called by B-tree routines, should _not_ be called externally ***
RETURNS

AUTHOR
   -GeorgeV - 9/3/96   
---------------------------------------------------------------------------*/
intn
chkcompare(void * k1,   /* IN: first key */
           void * k2,   /* IN: second key */
           intn cmparg /* IN: not sure? */)
{
    intn  ret_value;
    /* shut compiler up */
    cmparg = cmparg;

    /* valid for integer keys */
    ret_value = ((intn) ((*(int32 *) k1) - (*(int32 *) k2)));    

    return ret_value;
}   /* chkcompare */

/********* Helper fcns for dealing with chunk table TBBT tree ***************/

/* -------------------------------------------------------------------------
NAME
    chkfreekey
DESCRIPTION
    Free key - used by tbbt routines 
    *** Only called by B-tree routines, should _not_ be called externally ***

RETURNS
   Nothing
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
void
chkfreekey(void * key /*IN: chunk key */ )
{
    if (key != NULL)
        HDfree(key);
} /* chkfreekey() */

/* -------------------------------------------------------------------------
NAME
    chkdestroynode
DESCRIPTION
   Frees chunk B-Tree nodes
   *** Only called by B-tree routines, should _not_ be called externally ***

RETURNS
   Nothing
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
void
chkdestroynode(void * n /* IN: chunk record */ )
{
    CHUNK_REC *t=(CHUNK_REC *)n;

    if (t != NULL)
      {   
          /* free orgin first */
          if (t->origin != NULL)
              HDfree(t->origin);

          /* free chunk record structure */
          HDfree((void *) t);
      }
}   /* chkdestroynode */

/* ----------------------------- HMCIstaccess ------------------------------
NAME
   HMCIstaccess -- set up AID to access a chunked elem

DESCRIPTION
   Calls to HMCIstread and HMCIstwrite resolve to this function.
   Given an active AID fill in all of the special information.
   If this information has already been read in for a different
   element use that else we must go out to the HDF file and
   pull in the information ourselves

   This routine also creates the chunk cache for the chunked element. 
   The cache is initialzed with the physical size of each chunk, 
   the number of chunks in the object i.e. object size/ chunk size,
   and the maximum number of chunks to cache in memory. Chunks in
   the cache are dealt with by their number i.e. translation of
   'origin' of chunk to a unique number. The default maximum number
   of chunks is the cache is set the number of chunks along the
   last dimension.

   NOTE: The cache itself could be used to cache any object into a number 
   of fixed size chunks so long as the read/write(page-in/page-out) routines know
   how to deal with getting the correct chunk based on a number. These
   routines can be found in 'mcache.c'.

RETURNS
   The AID of the access record on success FAIL on error.
AUTHOR
   -GeorgeV - 9/3/96
----------------------------------------------------------------------------*/
PRIVATE int32
HMCIstaccess(accrec_t *access_rec, /* IN: access record to fill in */
             int16 acc_mode        /* IN: access mode */)
{
    CONSTR(FUNC, "HMCIstaccess");    /* for HERROR */
    filerec_t  *file_rec = NULL;     /* file record */
    chunkinfo_t *info    = NULL;     /* information about data elt */
    int32       dd_aid;              /* AID for writing the special info */
    uint16      data_tag, data_ref;  /* Tag/ref of the data in the file */
    uint8       local_ptbuf[6];      /* 6 bytes for special header length */
#if 0
    uint8       *c_sp_header = NULL;   /* special element header(dynamic) */
#endif
    uint8       c_sp_header[256]="" ;   /* special element header buffer.
                                           dynamic allocation causes 
                                           a problem on the HPUX -GV */
    int32       interlace;           /* type of interlace */
    int32       vdata_size;          /* size of Vdata */
    int32       num_recs;            /* number of Vdatas */
    uint8       *v_data = NULL;      /* Vdata record */
    CHUNK_REC   *chkptr = NULL;      /* Chunk record */
    int32       *chk_key   = NULL;   /* chunk key */
    int32       npages     = 1;      /* number of chunks */
    int32       chunks_needed;       /* default chunk cache size  */
    int32       access_aid = FAIL;   /* access id */
    int32       ret_value = SUCCEED;
    char        name[VSNAMELENMAX + 1];  /* Vdata name */
    char        class[VSNAMELENMAX + 1]; /* Vdata class */
    char        v_class[VSNAMELENMAX + 1] = ""; /* Vdata class for comparison */
    intn        i,j,k;                     /* loop indicies */

    /* Check args */
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);
        
    /* validate file record id */
    file_rec = HAatom_object(access_rec->file_id);
    if (BADFREC(file_rec) || !(file_rec->access & acc_mode))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* set up some data in access record */
    access_rec->special = SPECIAL_CHUNKED;
    access_rec->posn    = 0;
    access_rec->access  = (uint32)(acc_mode|DFACC_READ);

    /*
     * Lets free old special info first,if one exists,
     * before copying a new one
     *
     * Hmm.....this is what other special elements do currently
     * don't know if this is really necessary.....but leave in for now..
     */
    if (access_rec->special_info != NULL)
      {   /* special information record */
          chunkinfo_t *tmpinfo = (chunkinfo_t *) access_rec->special_info;

          if (--(tmpinfo->attached) == 0)
            {   /* the last one so now.. */
                /* free old info from Chunk tables ..etc*/

                /* Sync chunk cache */
                mcache_sync(info->chk_cache);

                /* close/free chunk cache */
                mcache_close(info->chk_cache);

                /* Use Vxxx interface to free Vdata info */
                VSdetach(info->aid);

                /* free chunk tree */
                tbbtdfree(info->chk_tree, chkdestroynode, chkfreekey);

                /* free up stuff in special info */
                if (tmpinfo->ddims != NULL)
                    HDfree(tmpinfo->ddims);
                if (tmpinfo->seek_chunk_indices != NULL)
                    HDfree(tmpinfo->seek_chunk_indices);
                if (tmpinfo->seek_pos_chunk != NULL)
                    HDfree(tmpinfo->seek_pos_chunk);
                if (tmpinfo->seek_user_indices != NULL)
                    HDfree(tmpinfo->seek_user_indices);

                if (tmpinfo->fill_val != NULL)
                    HDfree(tmpinfo->fill_val);

                if (tmpinfo->comp_sp_tag_header != NULL)
                    HDfree(tmpinfo->comp_sp_tag_header);
                if (tmpinfo->cinfo != NULL)
                    HDfree(tmpinfo->cinfo);
                if (tmpinfo->minfo != NULL)
                    HDfree(tmpinfo->minfo);
                /* free info struct last */
                HDfree(tmpinfo);

                access_rec->special_info = NULL;
            }
      } /* end if special->info already */

    /* get the info for the dataset i.e. tag/ref*/
    /* get info about chunk table i.e. Vdata? */
    if(HTPinquire(access_rec->ddid,&data_tag,&data_ref,NULL,NULL)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* if the special information are already in some other acc elt,
     * point to it  and return */
    access_rec->special_info = HIgetspinfo(access_rec);
    if (access_rec->special_info)
      { /* special info exists */
          ((chunkinfo_t *) access_rec->special_info)->attached++;
          file_rec->attach++;
          info = (chunkinfo_t *) access_rec->special_info;          
          /* set return value */
          access_aid = HAregister_atom(AIDGROUP,access_rec);
      }
    else /* need to allocate a new special info and get it */
      {
          /* allocate space for specail chunk info */
          if ((info = HDmalloc(sizeof(chunkinfo_t))) == NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);

          info->seek_chunk_indices = NULL;
          info->seek_pos_chunk     = NULL;
          info->seek_user_indices = NULL;
          info->ddims     = NULL;
          info->chk_tree  = NULL;
          info->chk_cache = NULL;
          info->fill_val  = NULL;
          info->minfo     = NULL;
          info->cinfo     = NULL;
          info->comp_sp_tag_header   = NULL;
          info->comp_sp_tag_head_len = 0;
          info->num_recs  = 0; /* zero records to start with */

          /* read the special info structure from the file */
          if((dd_aid=Hstartaccess(access_rec->file_id,data_tag,data_ref,DFACC_READ))==FAIL)
              HGOTO_ERROR(DFE_CANTACCESS, FAIL);

          if (Hseek(dd_aid, 2, DF_START) == FAIL)
              HGOTO_ERROR(DFE_SEEKERROR, FAIL);

          /* first read specail tag header length which is 4 bytes */
          if (Hread(dd_aid, 4, local_ptbuf) == FAIL)
              HGOTO_ERROR(DFE_READERROR, FAIL);

          /* Decode it */
          {
              uint8      *p = local_ptbuf;
              INT32DECODE(p, info->sp_tag_header_len);   /* 4 bytes */          
          }

          /* Sanity check, the 256 limit is arbitrary and can 
             be removed later....*/
          if (info->sp_tag_header_len < 0 || info->sp_tag_header_len > 256)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

#if 0 /* dynamic alocation causes a problem on HPUX, removed for now -GV */
          /* Allocate buffer space for rest of special header */
          if (( c_sp_header = (uint8 *) HDcalloc(info->sp_tag_header_len,1))==NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
#endif
          /* first read special header in */
          if (Hread(dd_aid, info->sp_tag_header_len, c_sp_header) == FAIL)
              HGOTO_ERROR(DFE_READERROR, FAIL);

          /* decode first special element header  */
          {
              uint8      *p = c_sp_header;

              /* version info */
              HDmemcpy(&info->version,p,1);      /* 1 byte  */
              p = p + 1;

              /* Should check version here to see if we can handle 
                 this version of special format header before we go on */
              if (info->version != _HDF_CHK_HDR_VER)
                  HGOTO_ERROR(DFE_INTERNAL, FAIL);

              INT32DECODE(p, info->flag);         /* 4 bytes */
              INT32DECODE(p, info->length);       /* 4 bytes */
              INT32DECODE(p, info->chunk_size);   /* 4 bytes */
              INT32DECODE(p, info->nt_size);      /* 4 bytes */
              UINT16DECODE(p, info->chktbl_tag);  /* 2 bytes */
              UINT16DECODE(p, info->chktbl_ref);  /* 2 bytes */
              UINT16DECODE(p, info->sp_tag);      /* 2 bytes */
              UINT16DECODE(p, info->sp_ref);      /* 2 bytes */
              INT32DECODE(p, info->ndims);        /* 4 bytes */
                                                  /* = 29 bytes */

              /* create dimension, seek_block and seek_pos arrays 
                 given number of dims */
              if (create_dim_recs(&(info->ddims),&(info->seek_chunk_indices),
                                  &(info->seek_pos_chunk),
                                  &(info->seek_user_indices),info->ndims) == FAIL)
                  HGOTO_ERROR(DFE_INTERNAL, FAIL);

              /* decode dimension stuff */
              for (j = 0; j < info->ndims; j++)
                {
                    int32 odd_size;

                    INT32DECODE(p,(info->ddims[j].flag));          /* 4 bytes */
                    INT32DECODE(p,(info->ddims[j].dim_length));    /* 4 bytes */
                    INT32DECODE(p,(info->ddims[j].chunk_length));  /* 4 bytes */
                                                                  /* = 12 bytes */  

                    /* check 'flag' and decode settings */
                    info->ddims[j].distrib_type = (int32)(0xff & info->ddims[j].flag);
                    info->ddims[j].unlimited = (int32)
                                    (0xff & ((uint32)(info->ddims[j].flag >> 8)));

                    info->ddims[j].num_chunks = info->ddims[j].dim_length /
                        info->ddims[j].chunk_length;
                    /* check to see if need to increase # of chunks along this dim*/
                    if ((odd_size = (info->ddims[j].dim_length % info->ddims[j].chunk_length)))

                      {
                          info->ddims[j].num_chunks++; /* increase by one */
                          /* set last chunk length */
                          info->ddims[j].last_chunk_length = odd_size; 
                      }
                    else
                        info->ddims[j].last_chunk_length = info->ddims[j].chunk_length; /*  */	    

                    npages = npages * info->ddims[j].num_chunks;
                }   /* = 12 x ndims bytes */

              /* decode fill value length */
              INT32DECODE(p,(info->fill_val_len));   /* 4 bytes */

              /* allocate space for fill value */
              if ((info->fill_val = HDmalloc((size_t)info->fill_val_len ))==NULL)
                  HGOTO_ERROR(DFE_NOSPACE, FAIL);

              /* finally decode fill value */
              HDmemcpy(info->fill_val,p, info->fill_val_len); /* 1 byte */

          } /* end decode special header */

          /* if multiply special deal with now */
          switch(info->flag & 0xff) /* only using 8bits for now */
            {
            case SPECIAL_COMP:
            {
                uint16     sp_tag;

                /* first read specail tag header length which is 2+4 bytes */
                if (Hread(dd_aid, 6, local_ptbuf) == FAIL)
                    HGOTO_ERROR(DFE_READERROR, FAIL);

                /* Decode compression header length */
                {
                    uint8      *p = NULL;

                    p = local_ptbuf;
                    UINT16DECODE(p, sp_tag);                     /* 2 bytes */
                    INT32DECODE(p, info->comp_sp_tag_head_len);   /* 4 bytes */      
                }

                /* Sanity check */
                if (info->sp_tag_header_len < 0 || sp_tag != SPECIAL_COMP)
                    HGOTO_ERROR(DFE_INTERNAL, FAIL);

                /* Allocate buffer space for compression special header */
                if (( info->comp_sp_tag_header = HDcalloc(info->comp_sp_tag_head_len,1))==NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);

                /* read special header in */
                if (Hread(dd_aid, info->comp_sp_tag_head_len, info->comp_sp_tag_header) == FAIL)
                    HGOTO_ERROR(DFE_READERROR, FAIL);

                /* allocate compression special info  */
                if (( info->cinfo = (comp_info *) HDmalloc(sizeof(comp_info)))==NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);
                if (( info->minfo = (model_info *) HDmalloc(sizeof(model_info)))==NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);

                /* Decode header */
                if (HCPdecode_header((uint8 *)info->comp_sp_tag_header,
                                     (comp_model_t *)&info->model_type, info->minfo, 
                                     (comp_coder_t *)&info->comp_type, info->cinfo) == FAIL)
                    HGOTO_ERROR(DFE_INTERNAL, FAIL);
            }
              break;
            default:
                /* Do nothing */
                break;
            } /* end switch on specialness */

          /* end access to special info stuff */
          if(Hendaccess(dd_aid)==FAIL)
              HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

          /* set up the chunk tables of the information */
          /* intialize TBBT tree of CHUNK records*/
          info->chk_tree = tbbtdmake(chkcompare, sizeof(int32), TBBT_FAST_INT32_COMPARE);

          /* Use Vdata interface to read in chunk table and
             store per chunk-info in memory using TBBT trees  */ 

          /* Start access on Vdata */
          if(Vstart(access_rec->file_id) == FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

          /* Attach to Vdata with write access if we are writing 
             else read access */
          if (access_rec->access & DFACC_WRITE)
            {
                if((info->aid = VSattach(access_rec->file_id, (int32)info->chktbl_ref, "w")) == FAIL)
                    HGOTO_ERROR(DFE_CANTATTACH, FAIL);
            }
          else /* attach with read access only */
            {
                if((info->aid = VSattach(access_rec->file_id, (int32)info->chktbl_ref, "r")) == FAIL)
                    HGOTO_ERROR(DFE_CANTATTACH, FAIL);
            }

          /* get relevant info on Vdata */
          if ((VSinquire(info->aid, &num_recs, &interlace, NULL, &vdata_size, name)) == FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

          /* Get class of Vdata */
          if ((VSgetclass(info->aid, class)) == FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

          /* verify class and version */
          sprintf(v_class,"%s%d",_HDF_CHK_TBL_CLASS,_HDF_CHK_TBL_CLASS_VER);
          if (HDstrncmp(class,v_class,HDstrlen(v_class)) != 0 )
            {
#ifdef CHK_DEBUG_2
                fprintf(stderr," error, wrong class=%s, %d \n",class,HDstrlen(class));
                fprintf(stderr,"            v_class=%s, %d \n",v_class,HDstrlen(v_class));
#endif
              HGOTO_ERROR(DFE_INTERNAL, FAIL);
            }

          /* Check to see if any chunks have been written out yet */
          if (num_recs > 0)
            { /* Yes */
                /* Set the fields to read */
                if(VSsetfields(info->aid,_HDF_CHK_FIELD_NAMES)==FAIL)
                    HGOTO_ERROR(DFE_BADFIELDS,FAIL);

                /* Allocate space for a single Vdata record */
                if ((v_data = HDmalloc((size_t)vdata_size)) == NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);

                /* for each record read it in and put into TBBT tree 
                   NOTE: Should change this to a single VSread() but then
                   would have to store all the v_data rec's somewhere
                   before inserting them into the TBBT tree...
                   ....for somone to do later if performance of VSread() is bad.
                   Technically a B+-Tree should have been used instead or
                   better yet the Vdata implementation should be re-written to use one.
                   Note that chunk tag DTAG_CHUNK is not verified here.
                   It is checked in HMCPchunkread() before the chunk is read. */
                for (j = 0; j < num_recs; j++)
                  {
                      uint8 *pntr = NULL;

                      /* read single record */
                      if(VSread(info->aid,v_data,1,FULL_INTERLACE)==FAIL)
                          HGOTO_ERROR(DFE_VSREAD,FAIL);
        
                      pntr = v_data; /* set pointer to vdata record */

                      /* Allocate space for a chunk record */
                      if ((chkptr = (CHUNK_REC *) HDmalloc(sizeof(CHUNK_REC))) == NULL)
                          HGOTO_ERROR(DFE_NOSPACE, FAIL);

                      /* Allocate space for a origin in chunk record */
                      if ((chkptr->origin = (int32 *) HDmalloc((size_t)info->ndims*sizeof(int32))) == NULL)
                          HGOTO_ERROR(DFE_NOSPACE, FAIL);

                      /* allocate space for key */
                      if ((chk_key = (int32 *)HDmalloc(sizeof(int32))) == NULL)
                          HGOTO_ERROR(DFE_NOSPACE, FAIL);

                      /* Copy origin first */
                      for (k = 0; k < info->ndims; k++)
                        {
                            HDmemcpy(&chkptr->origin[k],pntr,sizeof(int32));
                            pntr += sizeof(int32);

                        }

#ifdef CHK_DEBUG_2
                      printf(" chkptr->origin = (");
                      for (k = 0; k < info->ndims; k++)
                          printf("%d%s", chkptr->origin[k], k!= info->ndims-1 ? ",":NULL);
                      printf("), ");
#endif

                      /* Copy tag next. 
                         Note: Verification of tag as DTAG_CHUNK is done in
                         HMCPchunkread() before the chunk object is read.
                         In the future the tag/ref pair could point to
                         another chunk table...etc.
                         */
                      HDmemcpy(&chkptr->chk_tag,pntr,sizeof(uint16));
                      pntr += sizeof(uint16);
#ifdef CHK_DEBUG_2
                      printf(" chktpr->chk_tag=%d, ",chkptr->chk_tag);
#endif
                      /* Copy ref last */
                      HDmemcpy(&chkptr->chk_ref,pntr,sizeof(uint16));
#ifdef CHK_DEBUG_2
                      printf(" chktpr->chk_ref=%d, ",chkptr->chk_ref);
                      printf("\n");
#endif
                      /* now compute chunk number from origin */
                      calculate_chunk_num(chk_key, info->ndims, chkptr->origin, 
                                          info->ddims);

                      chkptr->chunk_number = *chk_key; 

                      /* set chunk number to record number */
                      chkptr->chk_vnum = info->num_recs++;

                      /* add to TBBT tree based on chunk number as the key */
                      tbbtdins(info->chk_tree, chkptr , chk_key);   
                  } /* end for num_recs */
            } /* end if num_recs */

          /* set return value */
          access_aid = HAregister_atom(AIDGROUP,access_rec);

          /* create chunk cache with 'maxcache' set to the number of chunks
             along the last dimension i.e subscript changes the fastest*/
	  chunks_needed = 1;
	  for (i = 1; i < info->ndims; i++) {
		chunks_needed *= info->ddims[i].num_chunks;
	  }
          if ((info->chk_cache = 
               mcache_open(&access_rec->file_id,                   /* cache key */
                           access_aid,                             /* object id */
                           (info->chunk_size*info->nt_size),       /* chunk size */
                           chunks_needed, /* maxcache */
                           npages,                                 /* num chunks */
                           0                                       /* flags */)) 
              == NULL)
              HE_REPORT_GOTO("failed to find initialize chunk cache", FAIL);

          /* set up chunk read/write routines 
             These routines do the actual reading/writing of data 
             from the file in whole chunks only. */
          mcache_filter(info->chk_cache, /* cache handle */
                        HMCPchunkread,   /* page-in routine */
                        HMCPchunkwrite,  /* page-out routine */
                        access_rec       /* object handle */);

          /* update chunk info data and file record info */
          info->attached = 1;
          file_rec->attach++;
          access_rec->special_info = (chunkinfo_t *) info;
      } /* end else need to get special info */

    /* access to data elments is done on a per chunk basis which
       can only be done in the read/write routines 
       i.e. the cache pagin/pageout routines....*/    

    ret_value = access_aid;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

          /* free info struct */
          if (info != NULL)
            {
                if (info->chk_cache != NULL)
                  {
                      /* Sync chunk cache */
                      mcache_sync(info->chk_cache);

                      /* close/free chunk cache */
                      mcache_close(info->chk_cache);
                  }

                if (info->aid != FAIL)
                    VSdetach(info->aid);

                /* free chunk tree */
                if (info->chk_tree != NULL)
                    tbbtdfree(info->chk_tree, chkdestroynode, chkfreekey);

                /* free up stuff in special info */
                if (info->ddims != NULL)
                    HDfree(info->ddims);
                if (info->seek_chunk_indices != NULL)
                    HDfree(info->seek_chunk_indices);
                if (info->seek_pos_chunk != NULL)
                    HDfree(info->seek_pos_chunk);
                if (info->seek_user_indices != NULL)
                    HDfree(info->seek_user_indices);
                if (info->fill_val != NULL)
                    HDfree(info->fill_val);
                if (info->comp_sp_tag_header != NULL)
                    HDfree(info->comp_sp_tag_header);
                if (info->cinfo != NULL)
                    HDfree(info->cinfo);
                if (info->minfo != NULL)
                    HDfree(info->minfo);

                HDfree(info);

                access_rec->special_info = NULL;
            }

      } /* end if */

    /* Normal function cleanup */
#if 0 /* dynamic alocation causes a problem on HPUX, removed for now -GV */
    /* free specail element header */
    if (c_sp_header != NULL)
        HDfree(c_sp_header);
#endif
    /* free allocated space for vdata record */
    if (v_data != NULL)
        HDfree(v_data);

    return ret_value;
}   /* HMCIstaccess */

/* ------------------------------------------------------------------------
NAME
   HMCcreate -- create a chunked element

DESCRIPTION
   This routine takes an HDF element and promotes it into a 
   chunked element.  Basically, the element becomes a chunked element
   allowing easy appending where the chunk records are stored in
   a Vdata.  If the element already exists, this is an error currently 
   otherwise a new element is created.

   All of the pieces of the chunked element are the same size from
   the stand point of the element. If compression is used then
   each chunk is compressed and the compression layer takes
   care of it as the chunk layer sees each chunks as a seperate
   HDF object(DFTAG_CHUNK). The proper compression special header
   needs to be passed to the compression layer.

   The Vdata(chunk table) is made appendable with linked-block
   table size of 128.

   This routine also creates the chunk cache for the chunked element. 
   The cache is initialzed with the physical size of each chunk, 
   the number of chunks in the object i.e. object size/ chunk size,
   and the maximum number of chunks to cache in memory. Chunks in
   the cache are dealt with by their number i.e. translation of
   'origin' of chunk to a unique number. The default maximum number
   of chunks is the cache is set the number of chunks along the
   last dimension.

   NOTE: The cache itself could be used to cache any object into a number 
   of fixed size chunks so long as the read/write(page-in/page-out) routines know
   how to deal with getting the correct chunk based on a number.These
   routines can be found in 'mcache.c'.

RETURNS
   The AID of newly created chunked element, FAIL on error.
AUTHOR
   -GeorgeV - 9/3/96
 --------------------------------------------------------------------------- */
int32
HMCcreate(int32 file_id,       /* IN: file to put chunked element in */
          uint16 tag,          /* IN: tag of element */
          uint16 ref,          /* IN: ref of element */
          uint8 nlevels,       /* IN: number of levels of chunks */
          int32 fill_val_len,  /* IN: fill value length in bytes */
          void *fill_val,      /* IN: fill value */
          HCHUNK_DEF *chk_array /* IN: structure describing chunk distribution
                                  can be an array? but we only handle 1 level */ )
{
    CONSTR(FUNC, "HMCcreate");     /* for HERROR */
    filerec_t  *file_rec   = NULL; /* file record */
    accrec_t   *access_rec = NULL; /* access record */
    int32       dd_aid     = FAIL; /* AID for writing the special info */
    chunkinfo_t *info      = NULL; /* information for the chunked elt */
    uint8       *c_sp_header = NULL; /* special element header */
    int32       npages     = 1;    /* i.e. number of chunks in element */
    int32       chunks_needed;     /* default size of chunk cache */
    int32       access_aid = FAIL; /* access id */
    uint16      chktbl_ref;        /* the ref of the link structure
                                      chunk table i.e. Vdata */
    uint16      special_tag;       /* special version of this tag */
    atom_t      data_id;           /* dd ID of existing regular element */
    int32       sp_tag_header_len = 0; /* length of special header */
    int32       data_len   = 1;        /* logical length of element */
    int32       ret_value  = SUCCEED;
    char        v_name[VSNAMELENMAX + 1] = "";/* name of vdata i.e. chunk table */
    char        v_class[VSNAMELENMAX + 1] = ""; /* Vdata class */
    intn        i;                 /* loop index */

    /* shut compiler up */
    nlevels=nlevels;

    /* clear error stack and validate file record id */
    HEclear();
    file_rec = HAatom_object(file_id);

    /* validate args */
    if (BADFREC(file_rec) || chk_array == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* check file access for write */
    if (!(file_rec->access & DFACC_WRITE))
        HGOTO_ERROR(DFE_DENIED, FAIL);

    /* check if we are accidently passwed a special tag already */
    if(SPECIALTAG(tag)
       || (special_tag = MKSPECIALTAG(tag)) == DFTAG_NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get empty slot in access records */
    access_rec = HIget_access_rec();
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_TOOMANY, FAIL);

    /* search for identical dd */
    if ((data_id = HTPselect(file_rec,tag,ref))!=FAIL)
      {
          /*  this is where if a tag was already special i.e. compressed
              we would have to note it and promote it maybe? */
          /* Check if the element is already special */
          if (HTPis_special(data_id)==TRUE)
            {
                HTPendaccess(data_id);
                HGOTO_ERROR(DFE_CANTMOD, FAIL);
            }   /* end if */

      } /* end if */

    /* In theory we can have more than one level of chunks so 
       we need to repeat the following steps. This would
       allow subchunking but currently haven't decided how
       the user would pass this info to routine to create the
       proper chunk tables...etc.

       Do we need to create special chunk table to handle the
       special chunks i.e. ghost chunks.-> Pass on this for now  */

#if 0
    /* Okay we need to get a new ref for CHUNK table tag for first level */
    chktbl_ref = Htagnewref(file_id,DFTAG_CHUNKED);
#endif

    /* allocate and fill in special chunk info struct for CHUNKs */
    if (( info = (chunkinfo_t *) HDmalloc(sizeof(chunkinfo_t)))==NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    info->attached     = 1;
    info->aid          = FAIL;
    info->version      = _HDF_CHK_HDR_VER ;     /* verson 1 for now */
    info->flag         = chk_array->chunk_flag; /* SPECIAL_COMP ? */
    info->cinfo        = NULL;
    info->minfo        = NULL; 
    info->comp_sp_tag_head_len = 0;
    info->comp_sp_tag_header   = NULL;
    info->chunk_size   = chk_array->chunk_size; /* logical chunk size */
    info->nt_size      = chk_array->nt_size;    /* number type size */
    info->ndims        = chk_array->num_dims;   /* number of dimensions */
    info->sp_tag       = DFTAG_NULL;            /* not used currently */
    info->sp_ref       = 0;                     /* not used currently */
    info->seek_chunk_indices = NULL;
    info->seek_pos_chunk = NULL;
    info->seek_user_indices = NULL;
    info->ddims          = NULL;
    info->chk_tree       = NULL;
    info->chk_cache      = NULL;
    info->num_recs       = 0;                   /* zero Vdata records to start */
    info->fill_val_len   = fill_val_len;        /* length of fill value */
    /* allocate space for fill value */
    if (( info->fill_val = HDmalloc((uint32)fill_val_len))==NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);
    /* copy fill value over */
    HDmemcpy(info->fill_val, fill_val, info->fill_val_len); /* fill_val_len bytes */

    /* if compression set then fill in info i.e ENCODE for storage */
    switch(info->flag & 0xff) /* only using 8bits for now */
      {
      case SPECIAL_COMP:
          /* set compression info */
          /* allocate compression special info  */
          if (( info->cinfo = (comp_info *) HDmalloc(sizeof(comp_info)))==NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
          if (( info->minfo = (model_info *) HDmalloc(sizeof(model_info)))==NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);
          
          /* find compression header length */
          info->comp_sp_tag_head_len = HCPquery_encode_header(
              (comp_model_t)chk_array->model_type, chk_array->minfo, 
              (comp_coder_t)chk_array->comp_type, chk_array->cinfo);

          /* allocate space for compression header */
          if (( info->comp_sp_tag_header = HDmalloc((size_t)info->comp_sp_tag_head_len))==NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);

          /* Encode header for storage */
          if (HCPencode_header((uint8 *)info->comp_sp_tag_header,
                               (comp_model_t)chk_array->model_type, chk_array->minfo, 
                               (comp_coder_t)chk_array->comp_type, chk_array->cinfo) == FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);

          /* Decode header back for memory */
          if (HCPdecode_header((uint8 *)info->comp_sp_tag_header,
                               (comp_model_t *)&info->model_type, info->minfo, 
                               (comp_coder_t *)&info->comp_type, info->cinfo) == FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);
          break;
      default:
          /* Do nothing */
          break;
      } /* end switch on specialness */

    /* Use Vxxx interface to create new Vdata to hold Chunk table */
    /* create/intialize chunk table (Vdata ) */

    /* Start access on Vdata */
    if(Vstart(file_id) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Create Vdata */
    if((info->aid = VSattach(file_id, -1, "w")) == FAIL)
        HGOTO_ERROR(DFE_CANTATTACH, FAIL);

    /* get ref of Vdata */
    chktbl_ref = (uint16)VSQueryref(info->aid);

    info->chktbl_ref      = chktbl_ref; /* ref of chunk table */

    /* get tag of Vdata */
    info->chktbl_tag = (uint16)VSQuerytag(info->aid);

#ifdef CHK_DEBUG_2
    fprintf(stderr,"HMCcreate: info->chktbl_tag =%d, info->chktbl_ref=%d \n", 
            info->chktbl_tag, info->chktbl_ref);
#endif
    /* Define fields of chunk table i.e. Vdata */

    /* Define origin - order based on number of dims */
    if(VSfdefine(info->aid,_HDF_CHK_FIELD_1, DFNT_INT32,info->ndims) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Define tag of chunk 
       Note that the tag could be another Chunk table to
       represent another level. useful for quadtrees...etc. */
    if(VSfdefine(info->aid,_HDF_CHK_FIELD_2, DFNT_UINT16,1) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Define ref of chunk 
       Note that the ref could be that of another Chunk table to
       represent another level. useful for quadtrees...etc. */
    if(VSfdefine(info->aid,_HDF_CHK_FIELD_3, DFNT_UINT16,1) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Set Vdata name based on tag and ref of element and of tag/ref of Vdata.
       ...sort of a back pointer...so sue me...*/
    sprintf(v_name,"%s%d_%d_%d_%d",_HDF_CHK_TBL_NAME,tag, ref,
            info->chktbl_tag, info->chktbl_ref);
    if(VSsetname(info->aid,v_name) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Set Vdata class and version */
    sprintf(v_class,"%s%d",_HDF_CHK_TBL_CLASS,_HDF_CHK_TBL_CLASS_VER);
    if(VSsetclass(info->aid,v_class) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Set the fields to write */
    if(VSsetfields(info->aid,_HDF_CHK_FIELD_NAMES)==FAIL)
        HGOTO_ERROR(DFE_BADFIELDS,FAIL);

    /* create dimension, seek_block and seek_pos arrays given number of dims */
    if (create_dim_recs(&(info->ddims),&(info->seek_chunk_indices),
                        &(info->seek_pos_chunk),
                        &(info->seek_user_indices),info->ndims) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Copy info from input to dimension arrays */
    data_len = 1;
    for (i = 0; i < info->ndims; i++)
      {
          int32 odd_size;

          info->ddims[i].distrib_type = chk_array->pdims[i].distrib_type;
          if (chk_array->pdims[i].dim_length == 0) /* check unlimited dimension */
            { /* yes, UNLIMITED */
                info->ddims[i].unlimited = 1; /* set flag */
                /* set dimension length to be at least the chunk length
                   along this dimension */
                info->ddims[i].dim_length = chk_array->pdims[i].chunk_length;
            }
          else /* not an unlimited dimension */
              info->ddims[i].dim_length = chk_array->pdims[i].dim_length;

          /* set dimension 'flag' */
          info->ddims[i].flag = 
          (int32)(0xffff & ((info->ddims[i].unlimited << 8)
                             | (info->ddims[i].distrib_type)));

          info->ddims[i].chunk_length = chk_array->pdims[i].chunk_length;
          info->ddims[i].num_chunks = info->ddims[i].dim_length /
                                      info->ddims[i].chunk_length;
          /* check to see if need to increase # of chunks along this dim */
          if ((odd_size = (info->ddims[i].dim_length % info->ddims[i].chunk_length)))
            {
                info->ddims[i].num_chunks++; /* increase by one */
                /* set last chunk length */
                info->ddims[i].last_chunk_length = odd_size; 
            }
          else
              info->ddims[i].last_chunk_length = info->ddims[i].chunk_length; /*  */	    


          /* calculate number of chunks/pages in element */
          npages = npages * info->ddims[i].num_chunks;

          /* compute logical element length */
          data_len *= info->ddims[i].dim_length;

#ifdef CHK_DEBUG_2
          printf("HMCcreate: dim[%d].dim_length=%d,",i,info->ddims[i].dim_length);
          printf("dim[%d].chunk_length=%d,",i,info->ddims[i].chunk_length);
          printf("dim[%d].num_chunks=%d \n",i,info->ddims[i].num_chunks);
#endif
      }  /* end for ndims */                                        
#ifdef CHK_DEBUG_2
    printf("\n");
#endif

    /* Make Vdata appendable with linked block table size of 'npages'
       if less than 128 and greater than 16.
       Not the best heuristic but for now it should be okay...*/
    if (npages > 16 && npages < 128)
      {
          if (VSappendable(info->aid, npages) == FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);
      }
    else if (npages < 16 )
      { /* 16 is default */
          if (VSappendable(info->aid, 16) == FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);
      }
    else /* use 128 for large chunk tables for now */
      {
          if (VSappendable(info->aid, 128) == FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);
      }

    /* Set logical length of element */
    info->length = data_len;              /* logical size of element */

    /* Calculate total length of this special element header
       including the fields for 'sp_tag_desc' and 'sp_tag_head_len'.
       See description of format header at top of file for more
       info on fields.
       Include also length for multiply specialness headers */
    switch(info->flag & 0xff) /* only using 8bits for now */
      {
      case SPECIAL_COMP:
          sp_tag_header_len = 6 + 9 + 12 + 8 +(12*info->ndims) + 4 + info->fill_val_len
                            + 6 + info->comp_sp_tag_head_len;
          break;
      default:
          sp_tag_header_len = 6 + 9 + 12 + 8 +(12*info->ndims) + 4 + info->fill_val_len;
          break;
      }

    /* Allocate buffer space for header */
    if (( c_sp_header = (uint8 *) HDcalloc(sp_tag_header_len,1))==NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* Calculate length of this special element header itself.
       Note the value of 'sp_tag_head_len' in the file is the 
       total length of this special object header - 6 bytes.
       beacuse the length of the fields 'sp_tag_desc'(2 bytes) and 
       'sp_tag_head_len' (4 bytes) which are not included 
       If also multiply special need to subtract another 6 byts plus
       length for multiply specialness headers */
    switch(info->flag & 0xff) /* only using 8bits for now */
      {
      case SPECIAL_COMP:
          info->sp_tag_header_len = sp_tag_header_len - 6 - 6 - info->comp_sp_tag_head_len;
          break;
      default:
          info->sp_tag_header_len = sp_tag_header_len - 6;
          break;
      }

    /* encode info into chunked descripton record */
    {
        uint8      *p = c_sp_header;
        intn        j;

        UINT16ENCODE(p, SPECIAL_CHUNKED);        /* 2 bytes */
        INT32ENCODE(p, info->sp_tag_header_len); /* 4 bytes */
        HDmemcpy(p, &info->version,1);           /* 1 byte  */
        p = p + 1;
        INT32ENCODE(p, info->flag);         /* 4 bytes */
        INT32ENCODE(p, info->length);       /* 4 bytes */
        INT32ENCODE(p, info->chunk_size);   /* 4 bytes */
        INT32ENCODE(p, info->nt_size);      /* 4 bytes */
        UINT16ENCODE(p, info->chktbl_tag);  /* 2 bytes */
        UINT16ENCODE(p, info->chktbl_ref);  /* 2 bytes */
        UINT16ENCODE(p, info->sp_tag);      /* 2 bytes */
        UINT16ENCODE(p, info->sp_ref);      /* 2 bytes */
        INT32ENCODE(p, info->ndims);        /* 4 bytes */
                                            /* = 35 bytes*/
        for (j = 0; j < info->ndims; j++)
          {
              INT32ENCODE(p,(info->ddims[j].flag));         /* 4 bytes */
              INT32ENCODE(p,(info->ddims[j].dim_length));   /* 4 bytes */
              INT32ENCODE(p,(info->ddims[j].chunk_length)); /* 4 bytes */
          }                                               /* = 12 x ndims bytes */

        /* now for fill value */
        INT32ENCODE(p,(info->fill_val_len));            /* 4 bytes */
        HDmemcpy(p,info->fill_val,info->fill_val_len); /* fill_val_len bytes */
        p = p + fill_val_len;

        /* Future to encode multiply specialness stuff
           header lengths, header,..etc*/
        switch(info->flag & 0xff) /* only using 8bits for now */
          {
          case SPECIAL_COMP:
              UINT16ENCODE(p, SPECIAL_COMP);              /* 2 bytes */
              INT32ENCODE(p, info->comp_sp_tag_head_len); /* 4 bytes */
              /* copy special element header */
              HDmemcpy(p,info->comp_sp_tag_header,info->comp_sp_tag_head_len); 
              p = p + info->comp_sp_tag_head_len;
              break;
          default:
              /* Do nothing */
              break;
          }
    }

    /* write the special info structure to fill */
    if((dd_aid = Hstartaccess(file_id,special_tag,ref,DFACC_ALL))==FAIL)
        HGOTO_ERROR(DFE_CANTACCESS, FAIL);

    /* write only the base 32 bytes ( 6 + 9 + 12 + 5)
       plus what is needed for each dimension which is (12 x ndims) bytes.
       plus for fill value 4 bytes + fill_val_len 
       plus in future multiply special headers  = sp_tag_header_len */
    if (Hwrite(dd_aid, sp_tag_header_len, c_sp_header) == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, FAIL);

    /* end access to special info stuff in file */
    if(Hendaccess(dd_aid)==FAIL)
        HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

    /* intialize TBBT tree of CHUNK records*/
    info->chk_tree = tbbtdmake(chkcompare, sizeof(int32), TBBT_FAST_INT32_COMPARE);

    /* Detach from the data DD ID */
    if(data_id != FAIL)
      {
          if(HTPendaccess(data_id)==FAIL)
              HGOTO_ERROR(DFE_INTERNAL, FAIL);
      }
#ifdef CHK_DEBUG_2
    fprintf(stderr,"HMCcreate: special_tag =%d, ref=%d \n", 
            special_tag, ref);
    fprintf(stderr,"HMCcreate: dd_aid =%d, data_id=%d \n", 
            dd_aid, data_id);
#endif
    /* update access record and file record */
    if((access_rec->ddid = HTPselect(file_rec,special_tag,ref))==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    access_rec->special      = SPECIAL_CHUNKED;
    access_rec->special_func = &chunked_funcs;
    access_rec->special_info = info;
    access_rec->posn         = 0;
    access_rec->access       = DFACC_RDWR;
    access_rec->file_id      = file_id;
    access_rec->appendable   = FALSE;     /* start data as non-appendable */

    file_rec->attach++;

#ifdef CHK_DEBUG_2
    fprintf(stderr,"HMCcreate: access_rec->ddid =%d \n", 
            access_rec->ddid);
#endif
    /* register this valid access record for the chunked element */
    access_aid = HAregister_atom(AIDGROUP,access_rec);

    chunks_needed = 1;
    for (i = 1; i < info->ndims; i++) {
	chunks_needed *= info->ddims[i].num_chunks;
    }
    /* create chunk cache */
    if ((info->chk_cache = 
         mcache_open(&access_rec->file_id,                   /* cache key */
                     access_aid,                             /* object id */
                     (info->chunk_size*info->nt_size),       /* chunk size */
                     chunks_needed, /* maxcache */
                     npages,                                 /* num chunks */
                     0                                       /* flags */)) 
        == NULL)
        HE_REPORT_GOTO("failed to initialize chunk cache", FAIL);

    /* 
      set up chunk read/write routines
      These routine are the actual routines that read/write
      whole chunks at a time.i.e. page-in/page-out routines
     */
    mcache_filter(info->chk_cache, /* cache handle */
                  HMCPchunkread,   /* page-in routine */
                  HMCPchunkwrite,  /* page-out routine */
                  access_rec       /* object handle */);

    ret_value = access_aid;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
          /* free info struct */
          if (info != NULL)
            {
                if (info->chk_cache != NULL)
                  {   /* Sync chunk cache */
                      mcache_sync(info->chk_cache);

                      /* close chunk cache */
                      mcache_close(info->chk_cache);
                  }

                if (info->aid != FAIL)
                    VSdetach(info->aid); /* detach from chunk table */

                /* free chunk tree */
                if (info->chk_tree != NULL)
                    tbbtdfree(info->chk_tree, chkdestroynode, chkfreekey);

                /* free up stuff in special info */
                if (info->ddims != NULL)
                    HDfree(info->ddims);
                if (info->seek_chunk_indices != NULL)
                    HDfree(info->seek_chunk_indices);
                if (info->seek_pos_chunk != NULL)
                    HDfree(info->seek_pos_chunk);
                if (info->fill_val != NULL)
                    HDfree(info->fill_val);
                if (info->comp_sp_tag_header != NULL)
                    HDfree(info->comp_sp_tag_header);
                if (info->cinfo != NULL)
                    HDfree(info->cinfo);
                if (info->minfo != NULL)
                    HDfree(info->minfo);
                HDfree(info); /* free spcial info last */
            }

          /* free access record */
          if(access_rec != NULL)
              HIrelease_accrec_node(access_rec);
      } /* end if */

    /* Normal function cleanup */
    /* free special element header */
    if (c_sp_header != NULL)
        HDfree(c_sp_header);

    return ret_value;
} /* HMCcreate() */

/*--------------------------------------------------------------------------
NAME
     HMCgetcompress - get compression information for chunked element

DESCRIPTION
     Checks if the given element is compressed then get the compression
     information using HCPdecode_header.
     This routine is used by HCgetcompress for the chunked element part.

RETURNS
     Returns SUCCEED/FAIL

REVISION LOG
     September 2001: Added to fix bug #307 - BMR

-------------------------------------------------------------------------- */
intn
HMCgetcompress( accrec_t*    access_rec, /* IN: access record */
		comp_coder_t* comp_type, /* OUT: compression type */
		comp_info* c_info)       /* OUT: retrieved compression info */
{
    CONSTR(FUNC, "HMCgetcompress");   /* for HERROR */
    chunkinfo_t *info = NULL;   /* chunked element information record */
    model_info  m_info;         /* modeling information - dummy */
    comp_model_t model_type;    /* modeling type - dummy */
    intn        ret_value = SUCCEED;

    /* Get the special info from the given record */
    info = (chunkinfo_t *) access_rec->special_info;
    if (info == NULL) HGOTO_ERROR(DFE_COMPINFO, FAIL);

    /* If this chunked element is compressed, retrieve its comp info */
    if (info->flag == SPECIAL_COMP)
    {
        /* Decode header from storage */
        ret_value = HCPdecode_header((uint8 *)info->comp_sp_tag_header,
                 &model_type, &m_info, /* dummy */ 
		 comp_type, c_info);
    }
    /* It's not compressed */
    else
	*comp_type = COMP_CODE_NONE;

  done:
    if(ret_value == FAIL)
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value;
} /* HMCgetcompress() */


/*--------------------------------------------------------------------------
NAME
     HMCgetcomptype - get compression information for chunked element

DESCRIPTION
     Checks if the given element is compressed then get the compression
     information using HCPdecode_header.
     This routine is used by HCgetcompress for the chunked element part.

RETURNS
     Returns SUCCEED/FAIL

REVISION LOG
     September 2001: Added to fix bug #307 - BMR

-------------------------------------------------------------------------- */
intn
HMCgetcomptype(int32 dd_aid, /* IN: access id of header info */
		comp_coder_t* comp_type) /* OUT: compression type */
{
    CONSTR(FUNC, "HMCgetcomptype");   /* for HERROR */
    uint8 *bufp;		/* pointer to buffer */
    uint8  version;      /* Version of this Chunked element */
    int32  flag;         /* flag for multiply specialness ...*/
    uint16 c_type;    /* compression type */
    uint8 *c_sp_header = NULL; /* special element header */
    int32  sp_tag_header_len = 0; /* length of special header */
    int32  comp_sp_tag_head_len; /* Compression header length */
    VOID  *comp_sp_tag_header = NULL;  /* compression header */
    uint8  local_ptbuf[6];      /* 6 bytes for special header length */
    intn   ret_value = SUCCEED;

    /* first read special tag header length which is 4 bytes */
    if (Hread(dd_aid, 4, local_ptbuf) == FAIL)
	HGOTO_ERROR(DFE_READERROR, FAIL);

    /* Decode it */
    bufp = local_ptbuf;
    INT32DECODE(bufp, sp_tag_header_len);   /* 4 bytes */

    /* Sanity check */
    if (sp_tag_header_len < 0)
	HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Allocate buffer space for rest of special header */
    if ((c_sp_header = (uint8 *) HDcalloc(sp_tag_header_len,1))==NULL)
	HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* read special info header in */
    if (Hread(dd_aid, sp_tag_header_len, c_sp_header) == FAIL)
	HGOTO_ERROR(DFE_READERROR, FAIL);

    /* decode special info header */
    bufp = c_sp_header;

    /* version info */
    HDmemcpy(&version, bufp, 1);      /* 1 byte  */
    bufp = bufp + 1;

    /* Should check version here to see if we can handle
    this version of special format header before we go on */
    if (version != _HDF_CHK_HDR_VER)
	HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* flag indicating multiple specialness */
    INT32DECODE(bufp, flag);         /* 4 bytes */

    /* check for further specialness */
    switch(flag & 0xff)
      {
	/* if the element is also compressed, read the compress special info
	   header and decode to get the compression coder */
	case SPECIAL_COMP:
	{
	    uint16     sp_tag;

	    /* Read compression special tag and header length, 2+4 bytes */
	    if (Hread(dd_aid, 6, local_ptbuf) == FAIL)
		HGOTO_ERROR(DFE_READERROR, FAIL);

	    /* Decode compression header length */
		bufp = local_ptbuf;
		UINT16DECODE(bufp, sp_tag);		/* 2 bytes */
		INT32DECODE(bufp, comp_sp_tag_head_len);   /* 4 bytes */ 

	    /* Sanity check */
	    if (comp_sp_tag_head_len < 0 || sp_tag != SPECIAL_COMP)
		HGOTO_ERROR(DFE_INTERNAL, FAIL);

	    /* Allocate buffer space for compression special header */
	    if ((comp_sp_tag_header = HDcalloc(comp_sp_tag_head_len,1))==NULL)
		HGOTO_ERROR(DFE_NOSPACE, FAIL);

	    /* Read compression special header in */
	    if (Hread(dd_aid, comp_sp_tag_head_len, comp_sp_tag_header) == FAIL)
		HGOTO_ERROR(DFE_READERROR, FAIL);

	    /* Decode header to get compression type */
	    bufp = comp_sp_tag_header;
	    bufp = bufp + 2;	/* skip model type */
	    UINT16DECODE(bufp, c_type);     /* get encoding type */
	    *comp_type=(comp_coder_t)c_type;
	    break;
	}
	/* It's not compressed */
	default:
	    *comp_type = COMP_CODE_NONE;
      } /* switch flag */

  done:
    if(ret_value == FAIL)
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    /* Free special element headers */
    if (c_sp_header != NULL)
        HDfree(c_sp_header);
    if (comp_sp_tag_header != NULL)
        HDfree(comp_sp_tag_header);

    return ret_value;
} /* HMCgetcomptype() */

/*--------------------------------------------------------------------------
NAME
     HMCgetdatasize - get data sizes of the chunked element

DESCRIPTION
     This routine was intented to be used by HCPgetdatasize for the chunked 
     element part.  

     HMCgetdatasize proceeds as followed:
     - decode the chunking info special header to get the chunk table info
     - get access to the chunk table via Vdata interface
     - get the size of the chunk table to determine if the data has been written
     - if the element is also compressed, read each vdata record to obtain the
	tag/ref pair of the compression special header and read the header
     - decode the compression special header to get the compressed data ref# and
	retrieve the compressed data length via Hlength
     - if uncompressed size is requested by the caller, calculate the actual 
	size of the uncompressed data by (chunk size * number of records)
     - if compressed size is requested by the caller, calculate the total 
	compressed size by accumulating the compressed size of all chunks.

RETURNS
     Returns SUCCEED/FAIL

REVISION LOG
     September 2008: Added to fix bugzilla #587 - BMR

-------------------------------------------------------------------------- */
intn
HMCgetdatasize(int32 file_id,
		uint8 *p, /* IN: access id of header info */
		int32 *comp_size, /* OUT: size of compressed data */
		int32 *orig_size) /* OUT: size of uncompression type */
{
    CONSTR(FUNC, "HMCgetdatasize");	/* for HERROR */
    uint16	 comp_ref = 0;		/* ref# of compressed data */
    char         vsname[VSNAMELENMAX + 1];  /* Vdata name */
    char         v_class[VSNAMELENMAX + 1] = ""; /* Vdata class for comparison */
    char         vsclass[VSNAMELENMAX + 1]; /* Vdata class */
    int32        vdata_size;		/* size of Vdata */
    chunkinfo_t* chkinfo=NULL;		/* chunked element information */
    uint8       *v_data = NULL;		/* Vdata record */
    int32        num_recs=0,		/* number of records in chunk table */
		 chk_data_size=0,	/* non-compressed data size */
		 chk_comp_data_size=0,	/* compressed data size */
		 chktab_id=-1,		/* chunk table (vdata) id */
		 chk_aid=-1,		/* a single chunk aid */
		 len = 0;		/* length of a compressed chunk */
    uint8	 chk_spbuf[10];		/* 10 bytes for special tag, version, 
					   uncomp len, comp ref# */
    int		 j, k;
    intn	 ret_value = SUCCEED;

    /* Skip 4byte header len */
    p = p + 4;

    /* Allocate and fill in special chunk info struct for CHUNKs */
    if (( chkinfo = (chunkinfo_t *) HDmalloc(sizeof(chunkinfo_t)))==NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* Version info */
    HDmemcpy(&chkinfo->version, p, 1);      /* 1 byte  */
    p = p + 1;

    /* Should check version here to see if we can handle this version of
       special format header before we go on */
    if (chkinfo->version != _HDF_CHK_HDR_VER)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Flag indicating multiple specialness, used to find out if this element
       is also compressed or something else */
    INT32DECODE(p, chkinfo->flag);         /* 4 bytes */

    /* Length of uncompressed data as a whole, size of each chunk, and size of
       number type */
    INT32DECODE(p, chkinfo->length);       /* 4 bytes */
    INT32DECODE(p, chkinfo->chunk_size);   /* 4 bytes */
    INT32DECODE(p, chkinfo->nt_size);      /* 4 bytes */

    /* Get chunk data size */
    chk_data_size = chkinfo->chunk_size * chkinfo->nt_size;

    /* Get tag/ref of chunk table, 2 bytes each */
    INT16DECODE(p, chkinfo->chktbl_tag);
    INT16DECODE(p, chkinfo->chktbl_ref);

    /* Skip sp_tag and sp_ref then get ndims for use in skipping origins */
    p  = p + 2 + 2;
    INT32DECODE(p, chkinfo->ndims);        /* 4 bytes */

    /* Make sure it is really the vdata */
    if (chkinfo->chktbl_tag == DFTAG_VH)
    {
	/* Use Vdata interface to access chunk table */

	/* Start access on Vdata */
	if(Vstart(file_id) == FAIL)
	    HGOTO_ERROR(DFE_INTERNAL, FAIL);

	/* Attach to the chunk table vdata and get its num of records */
	if ((chktab_id = VSattach(file_id,(int32)chkinfo->chktbl_ref,"r")) == FAIL)
	    HGOTO_ERROR(DFE_CANTATTACH, FAIL);

	if ((VSinquire(chktab_id, &num_recs, NULL, NULL, &vdata_size, vsname)) == FAIL)
	    HGOTO_ERROR(DFE_INTERNAL, FAIL);

	/* Only continue reading the chunk table to get compressed data size 
	   if it is requested and if data had been written, i.e. chunk table is
	   not empty */
	if (comp_size != NULL && num_recs > 0)
	{
	    /* Check for further specialness.  If chunks are also compressed, 
	       then the chunk table will be read for each chunk's tag/ref, 
	       which points to the compression info of the chunk, to get the 
	       compressed data size */
	    switch(chkinfo->flag & 0xff)
	    {
		/* Element is also compressed, read and decode the compression 
		   special info header of each chunk and get the compressed 
		   data size */
		case SPECIAL_COMP:
		{
		    uint16     sp_tag;

		    /* Get class of Vdata */
		    if ((VSgetclass(chktab_id, vsclass)) == FAIL)
			HGOTO_ERROR(DFE_INTERNAL, FAIL);

		    /* Verify class and version */
		    sprintf(v_class,"%s%d",_HDF_CHK_TBL_CLASS,_HDF_CHK_TBL_CLASS_VER);
		    if (HDstrncmp(vsclass,v_class,HDstrlen(v_class)) != 0 )
		    {
			HGOTO_ERROR(DFE_INTERNAL, FAIL);
		    }

		    /* Set the fields to read */
		    if(VSsetfields(chktab_id,_HDF_CHK_FIELD_NAMES)==FAIL)
			HGOTO_ERROR(DFE_BADFIELDS,FAIL);

		    /* Allocate space for a single Vdata record */
		    if ((v_data = HDmalloc((size_t)vdata_size)) == NULL)
			HGOTO_ERROR(DFE_NOSPACE, FAIL);

		    /* Read in the tag/ref of each chunk then get the 
			compression info header the tag/ref points to and 
			decode the compressed data size */
		    for (j = 0; j < num_recs; j++)
		    {
			uint8 *pntr = NULL;       /* temp pointer to vdata record */
			uint16 chk_tag, chk_ref;  /* each chunk's tag/ref */

			/* Read single record */
			if(VSread(chktab_id,v_data,1,FULL_INTERLACE)==FAIL)
			    HGOTO_ERROR(DFE_VSREAD,FAIL);
        
			pntr = v_data; /* set pointer to vdata record */

			/* Skip origin first */
			for (k = 0; k < chkinfo->ndims; k++)
			{
			    pntr += sizeof(int32);
			}

			/* Get the chunk's tag and ref */
			HDmemcpy(&chk_tag, pntr, sizeof(uint16));
			pntr += sizeof(uint16);
			HDmemcpy(&chk_ref, pntr, sizeof(uint16));

			/* Prepare to read the info which the tag/ref points to */
			chk_aid = Hstartaccess(file_id, MKSPECIALTAG(chk_tag), chk_ref, DFACC_READ);
			if (chk_aid == FAIL)
			    HGOTO_ERROR(DFE_BADAID, FAIL);

			/* Read 10 bytes: special tag (2), comp. version (2), 
			   uncomp length (4), and comp. ref# (2) */
			if (Hread(chk_aid, 10, chk_spbuf) == FAIL)
			    HGOTO_ERROR(DFE_READERROR, FAIL);

			/* Decode and check the special tag to be sure */
			p = chk_spbuf;
			UINT16DECODE(p, sp_tag);             /* 2 bytes */
			if (sp_tag == SPECIAL_COMP)
			{
			    /* Skip compression version (2 bytes) and 
				uncompressed data length (4 bytes) */
			    p = p + 2 + 4;

			    /* Get ref # of compressed data (2 bytes) */
			    UINT16DECODE(p, comp_ref);

			    /* Get length of compressed data.  Note that this 
				length is specified as compressed chunk size 
				times nt_size. */
			    if ((len = Hlength(file_id, DFTAG_COMPRESSED, comp_ref)) == FAIL)
				HGOTO_ERROR(DFE_BADLEN, FAIL);

			    /* Accumulate compressed size of all chunks. */
			    chk_comp_data_size = chk_comp_data_size + len;
			}

			/* sp_tag is not SPECIAL_COMP, while 'chkinfo->flag' 
			   above is SPECIAL_COMP, something must be wrong */
			else
			    HGOTO_ERROR(DFE_COMPINFO, FAIL);

			/* End access to special info of an individual chunk */
			if(Hendaccess(chk_aid)==FAIL)
			    HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);
		    } /* for each record */
		    break;
		}
		default:
		/* Element is not compressed, use non-compressed data size.
		   Note: must multiply by num_recs here because when element is
		   compressed, chk_comp_data_size was calculated by accumulating
		   "len" of each compressed chunk (see case above) */
		   chk_comp_data_size = chk_data_size * num_recs;
	    } /* switch flag */
	} /* if comp_size != NULL && num_recs >= 0 */

	if (VSdetach(chktab_id) == FAIL)
	    HGOTO_ERROR(DFE_CANTENDACCESS, FAIL);

    } /* it is a vdata */
    else
	HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* Return requested sizes */
    if (comp_size != NULL)
	*comp_size = chk_comp_data_size;
    if (orig_size != NULL)
	*orig_size = chk_data_size * num_recs;

  done:
    if(ret_value == FAIL)
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    /* Free allocated space for vdata record */
    if (v_data != NULL)
        HDfree(v_data);

    /* Free special chunk info struct */
    if (chkinfo != NULL)
	HDfree(chkinfo);

    return ret_value;
} /* HMCgetdatasize */

/*--------------------------------------------------------------------------
NAME
     HMCsetMaxcache - maximum number of chunks to cache 

DESCRIPTION
     Set the maximum number of chunks to cache.

     The values set here affects the current object's caching behaviour.

     If the chunk cache is full and 'maxcache' is greater then the
     current 'maxcache' value, then the chunk cache is reset to the new
     'maxcache' value, else the chunk cache remains at the current
     'maxcache' value.

     If the chunk cache is not full, then the chunk cache is set to the
     new 'maxcache' value only if the new 'maxcache' value is greater than the
     current number of chunks in the cache.

     Use flags arguement of 'HMC_PAGEALL' if the whole object is to be cached 
     in memory otherwise passs in zero.

RETURNS
     Returns number of 'maxcache' if successful and FAIL otherwise

AUTHOR
   -GeorgeV - 9/3/96

NOTE
     This calls the real routine mcache_set_maxcache().
     Currently 'maxcache' has to be greater than 1. 

-------------------------------------------------------------------------- */
int32
HMCsetMaxcache(int32 access_id, /* IN: access aid to mess with */
               int32 maxcache,  /* IN: max number of pages to cache */
               int32 flags      /* IN: flags = 0, HMC_PAGEALL */)
{
    CONSTR(FUNC, "HMCsetMaxcache");   /* for HERROR */
    accrec_t    *access_rec = NULL;   /* access record */
    chunkinfo_t *info       = NULL;   /* chunked element information record */
    int32       ret_value = SUCCEED;

    /* shut compiler up */
    flags=flags;

#ifdef CHK_DEBUG_2
    fprintf(stderr,"HMCsetMaxcache: access_id =%d \n", access_id);
#endif
    /* Check args */
    access_rec = HAatom_object(access_id);
    if (access_rec == NULL || maxcache < 1)
        HGOTO_ERROR(DFE_ARGS, FAIL);

#ifdef CHK_DEBUG_2
    fprintf(stderr,"HMCsetMaxcache: access_rec->special =%d \n", access_rec->special);
    fprintf(stderr,"HMCsetMaxcache: access_rec->ddid =%d \n", access_rec->ddid);
#endif

    /* since this routine can be called by the user,
       need to check if this access id is special CHUNKED */
    if (access_rec->special == SPECIAL_CHUNKED)
      {
          info     = (chunkinfo_t *) (access_rec->special_info);

          if (info != NULL)
              ret_value =  mcache_set_maxcache(info->chk_cache,maxcache);
          else 
              ret_value = FAIL;
      }
    else /* not special */
        ret_value = FAIL;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value;
} /* HMCsetMaxcache() */

/* ------------------------------ HMCPstread -------------------------------
NAME
   HMCPstread -- open an access record of chunked element for reading

DESCRIPTION
   Calls to HMCIstaccess to fill in the access rec for
   reading

RETURNS
   The AID of the access record on success FAIL on error.
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
int32
HMCPstread(accrec_t * access_rec /* IN: access record to fill in */)
{
    int32 ret_value;

    ret_value = HMCIstaccess(access_rec, DFACC_READ);

    return ret_value;
}   /* HMCPstread */

/* ------------------------------ HMCPstwrite ------------------------------
NAME
   HMCPstwrite -- open an access record of a chunked elmenent for writing

DESCRIPTION
   Calls to HMCIstaccess to fill in the access rec for
   writing

RETURNS
   The AID of the access record on success FAIL on error.
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
int32
HMCPstwrite(accrec_t * access_rec /* IN: access record to fill in */)
{
    int32  ret_value;

    ret_value = HMCIstaccess(access_rec, DFACC_WRITE);

    return ret_value;
}   /* HMCPstwrite */


/* ------------------------------- HMCPseek --------------------------------
NAME
   HMCPseek -- set the seek posn in the chunked elemnent

DESCRIPTION
   Set the seek posn in the given chunked element

RETURNS
   SUCCEED / FAIL
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
int32
HMCPseek(accrec_t * access_rec,  /* IN: access record to mess with */
         int32 offset,           /* IN: seek offset */
         int origin              /* IN: where we should calc the offset from */)
{
    CONSTR(FUNC, "HMCPseek");    /* for HERROR */
    chunkinfo_t *info = NULL;    /* information for the chunked elt */
    int32   ret_value = SUCCEED;

#ifdef CHK_DEBUG_3
    printf("HMCPseek called with offset %d \n",offset);
#endif
    /* Check args */
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* validate access record */
    if (access_rec->special != SPECIAL_CHUNKED)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* get special info */
    info = (chunkinfo_t *) (access_rec->special_info);

    /* adjust the offset according to origin and validate */
    /* there is no upper bound to posn */
    if (origin == DF_CURRENT)
        offset += access_rec->posn;
    if (origin == DF_END)
        offset += (info->length * info->nt_size); /* adjust by number type size */
    if (offset < 0)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    /* Seek to given location(bytes) for reading/writing */
    /* i.e calculate chunk indices given seek location 
       this will update the proper arrays in the special info struct */
    update_chunk_indicies_seek(offset,info->ndims, info->nt_size,
                               info->seek_chunk_indices,
                               info->seek_pos_chunk,info->ddims);

    /* set position in access record */
    access_rec->posn = offset;

#ifdef CHK_DEBUG_3
    printf("HMCPseek new user seek postion in element is  %d \n",offset);
#endif

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value;
}   /* HMCPseek */

/* ------------------------------- HMCPchunkread --------------------------------
NAME
   HMCPchunkread - read a chunk

DESCRIPTION
   Read in a whole chunk from a chunked element given the chunk number.  

   This is used as the 'page-in-chunk' routine for the cache.
   Only the cache should call this routine.

RETURNS
   The number of bytes read or FAIL on error
AUTHOR
   -GeorgeV - 9/3/96
--------------------------------------------------------------------------- */
int32
HMCPchunkread(void  *cookie,    /* IN: access record to mess with */
              int32 chunk_num,  /* IN: chunk to read */
              void  *datap      /* OUT: buffer for data */)
{
    CONSTR(FUNC, "HMCPchunkread");    /* for HERROR */
    accrec_t * access_rec = (accrec_t *)cookie; /* access record */
    chunkinfo_t *info    = NULL; /* information record for this special data elt */
    CHUNK_REC   *chk_rec = NULL; /* chunk record */
    TBBT_NODE   *entry   = NULL; /* chunk node from TBBT */
    uint8       *bptr    = NULL; /* pointer to data buffer */
    int32       chk_id   = FAIL; /* chunk id */
    int32       bytes_read = 0;    /* total # bytes read for this call of HMCIread */
    int32       read_len = 0;      /* length of bytes to read */
    int32       nitems = 1;        /* used in HDmemfill(), */
    int32       ret_value = SUCCEED;

    /* Check args */
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* set inputs */
    bptr = (uint8 *) datap;
    info = (chunkinfo_t *) (access_rec->special_info);
    bytes_read    = 0;
    read_len      = (info->chunk_size * info->nt_size); 

#ifdef CHK_DEBUG_3
    printf("HMCPchunkread called with chunk %d \n",chunk_num);
#endif
    /* find chunk record in TBBT */
    if ((entry = (tbbtdfind(info->chk_tree, &chunk_num, NULL))) == NULL)
      { /* does not exist */
          /* calculate number of fill value items to fill buffer with */
          nitems = (info->chunk_size * info->nt_size) / info->fill_val_len;

          /* copy fill values into buffer and return */
          if (HDmemfill(datap,info->fill_val, (uint32)info->fill_val_len,(uint32)nitems) == NULL)
              HE_REPORT_GOTO("HDmemfill failed to fill read chunk", FAIL);
      }
    else /* exists in TBBT */
      {
          /* get chunk record from node */
          chk_rec = (CHUNK_REC *) entry->data; 

          /* check to see if has been written to */
          if (chk_rec->chk_tag != DFTAG_NULL && BASETAG(chk_rec->chk_tag) == DFTAG_CHUNK)
            { /* valid chunk in file */
                /* Start read on chunk */
                if ((chk_id = Hstartread(access_rec->file_id, chk_rec->chk_tag,
                                         chk_rec->chk_ref)) == FAIL)
                  {
                      Hendaccess(chk_id);
                      HE_REPORT_GOTO("Hstartread failed to read chunk", FAIL);
                  }

                /* read data from chunk */
                if (Hread(chk_id, read_len, bptr) == FAIL)
                    HGOTO_ERROR(DFE_READERROR, FAIL);

                bytes_read = read_len;

                /* end access to chunk */
                if (Hendaccess(chk_id) == FAIL)
                    HE_REPORT_GOTO("Hendaccess failed to end access to chunk", FAIL);
              
            }
          else if (chk_rec->chk_tag == DFTAG_NULL) 
            {/* chunk has not been written, so return fill value buffer */
                /* calculate number of fill value items to fill buffer with */
                nitems = (info->chunk_size * info->nt_size) / info->fill_val_len;

                /* copy fill values into buffer and return */
                if (HDmemfill(datap,info->fill_val, (uint32)info->fill_val_len,(uint32)nitems) == NULL)
                    HE_REPORT_GOTO("HDmemfill failed to fill read chunk", FAIL);
            }
          else /* not a valid chunk ref for now */
            {  
                /* For now DFTAG_CHUNK is the only allowed value.
                   In the future this could be another Chunk table. */
                HE_REPORT_GOTO("Not a valid Chunk object, wrong tag for chunk", FAIL);
            }

      } /* end else exists in TBBT tree */

    ret_value = bytes_read; /* number of bytes read */

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
          if (chk_id != FAIL)
              Hendaccess(chk_id);
      } /* end if */

    /* Normal function cleanup */
#ifdef CHK_DEBUG_3
    printf("HMCPchunkread exit with ret_value= %d \n",ret_value);
#endif
    return ret_value;
} /* HMCPchunkread() */

/* ------------------------------- HMCreadChunk ---------------------------
NAME
   HMCreadChunk -- read a whole chunk

DESCRIPTION
   Read a whole chunk from a chunked element.  

   This can be used by users to read whole chunks from the file
   based on chunk origin for now i.e postion of chunk in overall
.  chunk array.

RETURNS
   The number of bytes read or FAIL on error
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
int32
HMCreadChunk(int32 access_id,  /* IN: access aid to mess with */
             int32 *origin,    /* IN: origin of chunk to read */
             void *datap /* IN: buffer for data */)
{
    CONSTR(FUNC, "HMCreadChunk");  /* for HERROR */
    accrec_t    *access_rec = NULL; /* access record */
#ifdef UNUSED
    uint8       *data       = NULL; /* data buffer */
#endif /* UNUSED */
    filerec_t   *file_rec   = NULL; /* file record */
    chunkinfo_t *info       = NULL; /* chunked element information record */
    uint8       *bptr       = NULL; /* data buffer pointer */
    void        *chk_data   = NULL; /* chunk data */
    uint8       *chk_dptr   = NULL; /* chunk data pointer */
    int32       relative_posn;      /* relative position in chunked element */
    int32       bytes_read = 0;     /* total #bytes read  */
    int32       read_len = 0;       /* bytes to read next */
    int32       chunk_num = -1;     /* chunk number */
    int32       ret_value = SUCCEED;
    intn        i;

#ifdef CHK_DEBUG_5
    printf("HMCreadChunk: entered \n");
#endif
    /* Check args */
    access_rec = HAatom_object(access_id);
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if (origin == NULL || datap == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* validate file records */
    file_rec =  HAatom_object(access_rec->file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* can read from this file? */
    if (!(file_rec->access & DFACC_READ))
        HGOTO_ERROR(DFE_DENIED, FAIL);

    /* since this routine can be called by the user,
       need to check if this access id is special CHUNKED */
    if (access_rec->special == SPECIAL_CHUNKED)
      {
          /* Set inputs */
#ifdef UNUSED
          data     = (uint8 *) datap;
#endif /* UNUSED */
          info     = (chunkinfo_t *) (access_rec->special_info);
          relative_posn = access_rec->posn;
          read_len      = (info->chunk_size * info->nt_size);
          bytes_read    = 0;
          bptr          = datap;
          
          /* copy origin over to seek chunk indicies 
             and set position within chunk to beginning of that chunk */
          for (i = 0; i < info->ndims; i++)
            {
              info->seek_chunk_indices[i] = origin[i];
              info->seek_pos_chunk[i] = 0;
            }

#ifdef CHK_DEBUG_5
          printf(" Seek start(in chunk array):(");
          for (i = 0; i < info->ndims; i++)
              printf("%d%s", info->seek_chunk_indices[i], i!= info->ndims-1 ? ",":NULL);
          printf(")\n");
#endif
          /* calculate chunk number from origin */
          calculate_chunk_num(&chunk_num, info->ndims, origin, info->ddims);

#ifdef CHK_DEBUG_5
    printf("HMCreadChunk called with chunk %d \n",chunk_num);
#endif
          /* currently get chunk data from cache based on chunk number 
             Note the cache deals with objects starting from 1 not 0 */
          if ((chk_data = mcache_get(info->chk_cache, /* cache handle */
                                              chunk_num+1,     /* chunk number */
                                              0                /* flag: unused */)) 
              == NULL)
              HE_REPORT_GOTO("failed to find chunk record", FAIL);

          chk_dptr = chk_data; /* set chunk data ptr */

          /* copy data from chunk to users buffer */
          HDmemcpy(bptr, chk_dptr, read_len);        

          /* put chunk back to cache and mark it as *not* DIRTY */
          if (mcache_put(info->chk_cache, /* cache handle */
                         chk_data,        /* whole data chunk */
                         0                /* flag: 0->not DIRTY */) 
              == FAIL)
              HE_REPORT_GOTO("failed to put chunk back in cache", FAIL);

          /* adjust number of bytes already read */
          bytes_read = read_len; 

#ifdef CHK_DEBUG_5
          printf("HMCreadChunk: read %d bytes already\n", bytes_read);
#endif

          /*update chunk seek indicies after reading chunk */
          update_seek_pos_chunk(bytes_read,info->ndims,info->nt_size,
                                info->seek_pos_chunk,
                                info->ddims);

          /* compute user array for chunk arrays */
          compute_chunk_to_array(info->seek_chunk_indices,info->seek_pos_chunk,
                                 info->seek_user_indices,
                                 info->ndims,info->ddims);

          /* calculate new read seek postion in element from user array */
          compute_array_to_seek(&relative_posn,
                                 info->seek_user_indices,
                                 info->nt_size,info->ndims,info->ddims);

#ifdef CHK_DEBUG_5
          printf("HMCreadChunk: new postion in element is %d\n", relative_posn);
#endif
          /* update access record with bytes read */
          access_rec->posn = relative_posn;

#ifdef CHK_DEBUG_5
          /* for info only */
          compute_chunk_to_seek(&relative_posn,info->ndims,info->nt_size,
                               info->seek_chunk_indices,
                               info->seek_pos_chunk,info->ddims,
                               info->chunk_size);
          printf("HMCreadChunk: new chunk seek postion in element is %d\n", relative_posn);
#endif

          ret_value = bytes_read;
      }
    else /* not special chunked element */
        ret_value = FAIL;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
      } /* end if */

    /* Normal function cleanup */

#ifdef CHK_DEBUG_5
    printf("HMCreadChunk: exited, ret=%d \n",ret_value);
#endif
    return ret_value;
} /* HMCreadChunk() */

/* ------------------------------- HMCPread --------------------------------
NAME
   HMCPread - read data from a chunked element

DESCRIPTION
   Read in some data from a chunked element. 

   Data is obtained from the cache which takes care of reading
   in the proper chunks to satisfy the request.

RETURNS
   The number of bytes read or FAIL on error
AUTHOR
   -GeorgeV - 9/3/96
--------------------------------------------------------------------------- */
int32
HMCPread(accrec_t * access_rec, /* IN: access record to mess with */
         int32 length,          /* IN: number of bytes to read */
         void * datap            /* OUT: buffer for data */)
{
    CONSTR(FUNC, "HMCPread");    /* for HERROR */
#ifdef UNUSED
    uint8       *data = NULL;    /* data buffer */
#endif /* UNUSED */
    chunkinfo_t *info = NULL;    /* information record for this special data elt */
    int32       relative_posn = 0; /* relative position in chunk of data elt */
    int32       bytes_read = 0;  /* total # bytes read for this call of HMCIread */
    uint8       *bptr = NULL;    /* data buffer pointer */
    int32       read_len = 0;    /* amount of data to copy */
    int32       read_seek = 0;   /* next read seek position */
    int32       chunk_size = 0;  /* size of data to read from chunk */
    int32       chunk_num = 0;   /* next chunk number */
    void        *chk_data = NULL; /* chunk data */
    uint8       *chk_dptr = NULL; /* pointer to chunk data */
#ifdef CHK_DEBUG_3
    int         i;
#endif
    int32       ret_value = SUCCEED;

#ifdef CHK_DEBUG_3
    printf("HMCPread called with length %d \n",length);
#endif
    /* Check args */
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

#ifdef CHK_DEBUG_2
    fprintf(stderr,"HMCread: access_rec->special =%d \n", access_rec->special);
    fprintf(stderr,"HMCread: access_rec->ddid =%d \n", access_rec->ddid);
#endif

    /* set inputs */
#ifdef UNUSED
    data = (uint8 *) datap;
#endif /* UNUSED */
    info = (chunkinfo_t *) (access_rec->special_info);
    relative_posn = access_rec->posn; /* current seek postion in element */

    /* validate length and set proper length */
    if (length == 0)
        length = (info->length * info->nt_size) - access_rec->posn;
    else if (length < 0)
        HGOTO_ERROR(DFE_RANGE, FAIL);

    if (access_rec->posn + length > (info->length * info->nt_size))
        length = (info->length * info->nt_size) - access_rec->posn;

    /* should chunk indicies be updated with relative_posn? 
       or did last operation update it already */
    update_chunk_indicies_seek(access_rec->posn,info->ndims, info->nt_size,
                               info->seek_chunk_indices,
                               info->seek_pos_chunk,info->ddims);

    /* enter translating length to proper filling of buffer from chunks */
    bptr = datap;
    bytes_read = 0;
    read_len = length; 
    while (bytes_read < read_len) 
      {
          /* for debuging */
#ifdef CHK_DEBUG_3
          printf(" Seek start(in chunk array):(");
          for (i = 0; i < info->ndims; i++)
              printf("%d%s", info->seek_chunk_indices[i], i!= info->ndims-1 ? ",":NULL);
          printf(")\n");
          printf(" Seek start(within the chunk):(");
          for (i = 0; i < info->ndims; i++)
              printf("%d%s", info->seek_pos_chunk[i], i!= info->ndims-1 ? ",":NULL);
          printf(")\n");
#endif

          /* calculate chunk to retrieve on this pass */
          calculate_chunk_num(&chunk_num,info->ndims,info->seek_chunk_indices,
                              info->ddims);

          /* calculate contiguous chunk size that we can read from this chunk 
             during this pass */
          calculate_chunk_for_chunk(&chunk_size,info->ndims,info->nt_size,
                                    read_len,bytes_read,
                                    info->seek_chunk_indices,
                                    info->seek_pos_chunk,info->ddims);
#ifdef CHK_DEBUG_3
          printf("  reading chunk(%d) with %d bytes\n", chunk_num, chunk_size);
#endif

          /* would be nice to get Chunk record from TBBT based on chunk number 
             and then get chunk data base on chunk vdata number but
             currently the chunk calculations return chunk 
             numbers and not Vdata record numbers. 
             This would reduce some overhead in the number of chunks
             dealt with in the cache */

          /* currently get chunk data from cache based on chunk number 
             Note the cache deals with objects starting from 1 not 0 */
          if ((chk_data = mcache_get(info->chk_cache, /* cache handle */
                                              chunk_num+1,     /* chunk number */
                                              0                /* flag: unused */)) 
              == NULL)
              HE_REPORT_GOTO("failed to find chunk record", FAIL);

          chk_dptr = chk_data; /* set chunk data ptr */

          /* calculate position in chunk */
          calculate_seek_in_chunk(&read_seek,info->ndims,info->nt_size,
                                  info->seek_pos_chunk,
                                  info->ddims);

          chk_dptr += read_seek; /* move to correct position in chunk */

#ifdef CHK_DEBUG_3
          printf("  read pos in chunk(%d) is %d bytes\n", chunk_num, read_seek);
#endif
          /* copy data from chunk to users buffer */
          HDmemcpy(bptr, chk_dptr, chunk_size);        

#ifdef CHK_DEBUG_10
          printf(" chk_dptr={");
          for (i = 0; i < chunk_size; i++)
              printf("%d,",(uint8)*((uint8 *)(chk_dptr)+i));
          printf("}\n");
#endif
          /* put chunk back to cache */
          if (mcache_put(info->chk_cache, /* cache handle */
                         chk_data,        /* whole data chunk */
                         0                /* flag: 0->not DIRTY */) 
              == FAIL)
              HE_REPORT_GOTO("failed to put chunk back in cache", FAIL);

          /* increment buffer pointer */
          bptr += chunk_size;

          /* adjust number of bytes already read */
          bytes_read += chunk_size; 

#ifdef CHK_DEBUG_3
          printf("  read %d bytes already\n", bytes_read);
#endif
          /* update relative position i.e. user element seek postion 
             with chunk size written */
          relative_posn += chunk_size;
#ifdef CHK_DEBUG_3
          printf("  relative_posn = %d bytes \n", relative_posn);
#endif
          /* i.e calculate chunk indices given seek location 
             this will update the proper arrays in the special info struct */
          update_chunk_indicies_seek(relative_posn,info->ndims, info->nt_size,
                                     info->seek_chunk_indices,
                                     info->seek_pos_chunk,info->ddims);
      } /* end while "bytes_read" */

    /* update access record postion with bytes read */
    access_rec->posn += bytes_read;

    ret_value = bytes_read;

#ifdef CHK_DEBUG_10
    printf(" datap={");
    for (i = 0; i < length; i++)
        printf("%d,",(uint8)*((uint8 *)(datap)+i));
    printf("}\n");
#endif
  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value;
}   /* HMCPread  */

/* ------------------------------- HMCPchunkwrite -------------------------------
NAME
   HMCPchunkwrite -- write out chunk

DESCRIPTION
   Write a whole chunk to a chunked element given the chunk number.  

   This is used as the 'page-out-chunk' routine for the cache.
   Only the cache should call this routine.

RETURNS
   The number of bytes written or FAIL on error
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
int32
HMCPchunkwrite(void  *cookie,    /* IN: access record to mess with */
               int32 chunk_num,  /* IN: chunk number */
               const void *datap /* IN: buffer for data */)
{
    CONSTR(FUNC, "HMCPchunkwrite");   /* for HERROR */
    accrec_t * access_rec = (accrec_t *)cookie; /* access record */
    chunkinfo_t *info    = NULL;  /* chunked element information record */
    CHUNK_REC   *chk_rec = NULL;  /* current chunk */
    TBBT_NODE   *entry   = NULL;  /* node off of  chunk tree */
    uint8       *v_data  = NULL;  /* chunk table record i.e Vdata record */
    CHUNK_REC   *chkptr  = NULL;  /* Chunk record to inserted in TBBT  */
    const void  *bptr    = NULL;  /* data buffer pointer */
    int32       chk_id   = FAIL ; /* chunkd accces id */
#ifdef UNUSED
    uint8      *data     = NULL;  /* data buffer */
    int32       relative_posn;     /* relative position in chunked element */
#endif /* UNUSED */
    int32       bytes_written = 0; /* total #bytes written by HMCIwrite */
    int32       write_len = 0;     /* nbytes to write next */
    int32       ret_value = SUCCEED;
    intn        k;                 /* loop index */

    /* Check args */
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Set inputs */
    info     = (chunkinfo_t *) (access_rec->special_info);
#ifdef UNUSED
    data     = (uint8 *) datap;
    relative_posn = access_rec->posn;
#endif /* UNUSED */
    write_len     = (info->chunk_size * info->nt_size);
    bytes_written = 0;
    bptr          = datap;

#ifdef CHK_DEBUG_4
    printf("HMCPchunkwrite called with chunk %d \n",chunk_num);
#endif
    /* find chunk record in TBBT */
    if ((entry = (tbbtdfind(info->chk_tree, &chunk_num, NULL))) == NULL)
        HE_REPORT_GOTO("failed to find chunk record", FAIL);

    chk_rec = (CHUNK_REC *) entry->data; /* get file entry from node */

    /* Check to see if already created in chunk table */
    if (chk_rec->chk_tag == DFTAG_NULL)
      { /* does not exists in Vdata table and in file but does in TBBT */
          uint8 *pntr = NULL;

          chkptr = chk_rec;
          /* so create a new Vdata record */
          /* Allocate space for a single Chunk record in Vdata */
          if (v_data == NULL)
            {
                if ((v_data = HDmalloc(((size_t)info->ndims*sizeof(int32))
                                               + (2*sizeof(uint16)))) == NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);
            }

          /* Initialize chunk record */
          chkptr->chk_tag = DFTAG_CHUNK;
          chkptr->chk_ref = Htagnewref(access_rec->file_id, DFTAG_CHUNK);
#ifdef CHK_DEBUG_4
          printf(" chktpr->chk_tag=%d, ",chkptr->chk_tag);
          printf(" chktpr->chk_ref=%d, ",chkptr->chk_ref);
          printf(" chkptr->origin = (");
          for (k = 0; k < info->ndims; k++)
              printf("%d%s", chkptr->origin[k], k!= info->ndims-1 ? ",":NULL);
          printf(")\n");
#endif

          if (chkptr->chk_ref == 0) {
                    /* out of ref numbers -- extremely fatal  */
                    HGOTO_ERROR(DFE_NOREF, FAIL);
          }
          /* Copy origin first to vdata record*/
          pntr = v_data;
          for (k = 0; k < info->ndims; k++)
            {
                HDmemcpy(pntr, &chkptr->origin[k],sizeof(int32));
                pntr += sizeof(int32);
            }

          /* Copy tag next */
          HDmemcpy(pntr, &chkptr->chk_tag,sizeof(uint16));
          pntr += sizeof(uint16);

          /* Copy ref last */
          HDmemcpy(pntr, &chkptr->chk_ref,sizeof(uint16));

          /* Add to Vdata i.e. chunk table */
          if(VSwrite(info->aid,v_data,1,FULL_INTERLACE)==FAIL)
              HGOTO_ERROR(DFE_VSWRITE,FAIL);

          /* Create compressed chunk if set 
             else start write access on element */
          switch(info->flag & 0xff) /* only using 8bits for now */
            {
            case SPECIAL_COMP: /* Create compressed chunk */
                if ((chk_id = HCcreate(access_rec->file_id, chk_rec->chk_tag,
                                       chk_rec->chk_ref,
                                       info->model_type, info->minfo,
                                       info->comp_type, info->cinfo)) == FAIL)
                    HE_REPORT_GOTO("HCcreate failed to read chunk", FAIL);
                break;
            default:
                /* Start write on chunk */
                if ((chk_id = Hstartwrite(access_rec->file_id, chk_rec->chk_tag,
                                          chk_rec->chk_ref,write_len)) == FAIL)
                    HE_REPORT_GOTO("Hstartwrite failed to read chunk", FAIL);
                break;
            }
      } /* not already in Vdata table */
    else
      { /* Already in table so start access */
        /* Start write on chunk */
        if ((chk_id = Hstartwrite(access_rec->file_id, chk_rec->chk_tag,
                                  chk_rec->chk_ref,write_len)) == FAIL)
            HE_REPORT_GOTO("Hstartwrite failed to read chunk", FAIL);
      }

    /* write data to chunk */
    if (Hwrite(chk_id, write_len, bptr) == FAIL)
        HGOTO_ERROR(DFE_WRITEERROR, FAIL);

    bytes_written = write_len;

    /* end access to chunk */
    if (Hendaccess(chk_id) == FAIL)
        HE_REPORT_GOTO("Hendaccess failed to end access to chunk", FAIL);

    ret_value = bytes_written;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
          if (chk_id != FAIL)
              Hendaccess(chk_id);
      } /* end if */

    /* Normal function cleanup */
    if (v_data != NULL)
        HDfree(v_data);

#ifdef CHK_DEBUG_4
    printf("HMCPchunkwrite exited with ret_value %d \n",ret_value);
#endif
    return ret_value;
} /* HMCPchunkwrite() */

/* ------------------------------- HMCwriteChunk ---------------------------
NAME
   HMCwriteChunk -- write out a whole chunk

DESCRIPTION
   Write out some data from a chunked element.  

   This can be used by users to write whole chunks to the file
   based on chunk origin for now i.e position of chunk in overall
   chunk array.

RETURNS
   The number of bytes written or FAIL on error
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
int32
HMCwriteChunk(int32 access_id,  /* IN: access aid to mess with */
              int32 *origin,    /* IN: origin of chunk to write */
              const void *datap /* IN: buffer for data */)
{
    CONSTR(FUNC, "HMCwriteChunk");  /* for HERROR */
    accrec_t    *access_rec = NULL; /* access record */
#ifdef UNUSED
    uint8       *data       = NULL; /* data buffer */
    CHUNK_REC   *chk_rec    = NULL; /* current chunk */
    TBBT_NODE   *entry      = NULL; /* node off of  chunk tree */
#endif /* UNUSED */
    filerec_t   *file_rec   = NULL; /* file record */
    chunkinfo_t *info       = NULL; /* chunked element information record */
    CHUNK_REC   *chkptr     = NULL; /* Chunk record to inserted in TBBT  */
    int32       *chk_key    = NULL; /* Chunk recored key for insertion in TBBT */
    const void  *bptr       = NULL; /* data buffer pointer */
    void        *chk_data   = NULL; /* chunk data */
    uint8       *chk_dptr   = NULL; /* chunk data pointer */
    int32       relative_posn;      /* relative position in chunked element */
    int32       bytes_written = 0;  /* total #bytes written by HMCIwrite */
    int32       write_len = 0;      /* bytes to write next */
    int32       chunk_num = -1;     /* chunk number */
    int32       ret_value = SUCCEED;
    intn        k;                  /* loop index */
    intn        i;


#ifdef CHK_DEBUG_4
    printf("HMCwriteChunk: entered \n");
#endif
    /* Check args */
    access_rec = HAatom_object(access_id);
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    if (origin == NULL || datap == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* validate file records */
    file_rec =  HAatom_object(access_rec->file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* can write in this file? */
    if (!(file_rec->access & DFACC_WRITE))
        HGOTO_ERROR(DFE_DENIED, FAIL);

    /* since this routine can be called by the user,
       need to check if this access id is special CHUNKED */
    if (access_rec->special == SPECIAL_CHUNKED)
      {
          /* Set inputs */
#ifdef UNUSED
          data     = (uint8 *) datap;
#endif /* UNUSED */
          info     = (chunkinfo_t *) (access_rec->special_info);
          relative_posn = access_rec->posn;
          write_len     = (info->chunk_size * info->nt_size);
          bytes_written = 0;
          bptr          = datap;

          /* copy origin over to seek chunk indicies 
             and set position within chunk to beginning of that chunk */
          for (i = 0; i < info->ndims; i++)
            {
              info->seek_chunk_indices[i] = origin[i];
              info->seek_pos_chunk[i] = 0;
            }

#ifdef CHK_DEBUG_4
          printf(" Seek start(in chunk array):(");
          for (i = 0; i < info->ndims; i++)
              printf("%d%s", info->seek_chunk_indices[i], i!= info->ndims-1 ? ",":NULL);
          printf(")\n");
#endif
          /* calculate chunk number from origin */
          calculate_chunk_num(&chunk_num, info->ndims, origin, 
                              info->ddims);

#ifdef CHK_DEBUG_4
    printf("HMCwriteChunk called with chunk %d \n",chunk_num);
#endif
          /* find chunk record in TBBT */
          if (tbbtdfind(info->chk_tree, &chunk_num, NULL) == NULL)
            { /* not in tree */
                
                /* so create a new chunk record */
                /* Allocate space for a chunk record */
                if ((chkptr = (CHUNK_REC *) HDmalloc(sizeof(CHUNK_REC))) == NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);

                /* Allocate space for a origin in chunk record */
                if ((chkptr->origin = (int32 *) HDmalloc((size_t)info->ndims*sizeof(int32))) == NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);

                /* allocate space for key */
                if ((chk_key = (int32 *)HDmalloc(sizeof(int32))) == NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);

                /* Initialize chunk record */
                chkptr->chk_tag = DFTAG_NULL;
                chkptr->chk_ref = 0;
#ifdef CHK_DEBUG_4
                printf("HMCwriteChunk: chktpr->chk_tag=%d, ",chkptr->chk_tag);
                printf(" chktpr->chk_ref=%d \n",chkptr->chk_ref);
#endif
                /* Intialize chunk origins */
                for (k = 0; k < info->ndims; k++)
                  {
                      chkptr->origin[k] = origin[k];
#ifdef CHK_DEBUG_4
                      printf("   chktpr->origin[%d]=%d, ",k,chkptr->origin[k]);
#endif
                  }
#ifdef CHK_DEBUG_4
                printf("\n");
#endif
                /* set chunk record number to next Vdata record number */
                chkptr->chk_vnum = info->num_recs++;

                /* set key to chunk number */
                chkptr->chunk_number = *chk_key = chunk_num;

                /* add to TBBT tree based on chunk number as the key */
                tbbtdins(info->chk_tree, chkptr , chk_key);   

#ifdef UNUSED
                /* assign over new chk */
                chk_rec = chkptr;
#endif /* UNUSED */

                /* re-intialize ptrs to allow for error-failure check */
                chkptr = NULL;
                chk_key = NULL;
            }
#ifdef UNUSED
          else /* already in TBBT tree */
              chk_rec = (CHUNK_REC *) entry->data; /* get file entry from node */
#endif /* UNUSED */

          /* would be nice to get Chunk record from TBBT based on chunk number 
             and then get chunk data base on chunk vdata number but
             currently the chunk calculations return chunk 
             numbers and not Vdata record numbers. 
             This would reduce some overhead in the number of chunks
             dealt with in the cache */

          /* get chunk data from cache based on chunk number 
             chunks in the cache start from 1 not 0 */
          if ((chk_data = mcache_get(info->chk_cache, /* cache handle */
                                              chunk_num+1,     /* chunk number */
                                              0                /* flag: unused */)) 
              == NULL)
              HE_REPORT_GOTO("failed to find chunk record", FAIL);

          chk_dptr = chk_data; /* set chunk data ptr */

          /* copy data from users buffer to chunk */
          HDmemcpy(chk_dptr, bptr, write_len);        

          /* put chunk back to cache and mark it as DIRTY */
          if (mcache_put(info->chk_cache, /* cache handle */
                         chk_data,        /* whole data chunk */
                         MCACHE_DIRTY     /* flag:  DIRTY */) 
              == FAIL)
              HE_REPORT_GOTO("failed to put chunk back in cache", FAIL);

          bytes_written = write_len;

          /*update chunk seek indicies after writing chunk */
          update_seek_pos_chunk(bytes_written,info->ndims,info->nt_size,
                                info->seek_pos_chunk,
                                info->ddims);

          /* calculate new read seek postion */
          compute_chunk_to_array(info->seek_chunk_indices,info->seek_pos_chunk,
                                 info->seek_user_indices,
                                 info->ndims,info->ddims);

          compute_array_to_seek(&relative_posn,
                                 info->seek_user_indices,
                                 info->nt_size,info->ndims,info->ddims);

#ifdef CHK_DEBUG_4
          printf(" new user seek postion in element is %d\n", relative_posn);
#endif
          /* update access record with bytes written */
          access_rec->posn = relative_posn;

#ifdef CHK_DEBUG_4
          /* for info only */
          compute_chunk_to_seek(&relative_posn,info->ndims,info->nt_size,
                               info->seek_chunk_indices,
                               info->seek_pos_chunk,info->ddims,
                               info->chunk_size);

          printf(" new chunk seek postion in element is %d\n", relative_posn);
#endif

          ret_value = bytes_written;
      }
    else /* not special chunked element */
        ret_value = FAIL;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
          /* check chunk ptrs */
          if (chkptr != NULL)
            {
                if (chkptr->origin != NULL)
                    HDfree(chkptr->origin);
                HDfree(chkptr);
            }
          if (chk_key != NULL)
              HDfree(chk_key);
      } /* end if */

    /* Normal function cleanup */

#ifdef CHK_DEBUG_4
    printf("HMCwriteChunk: exited, ret=%d \n",ret_value);
#endif
    return ret_value;
}   /* HMCwriteChunk */

/* ------------------------------- HMCPwrite -------------------------------
NAME
   HMCPwrite -- write out some data to a chunked element

DESCRIPTION
   Write out some data to a chunked element.  

   Data is obtained from the cache which takes care of obtaining
   the proper chunks to write to satisfy the request.

   The chunks are marked as dirty before being returned to the cache.

RETURNS
   The number of bytes written or FAIL on error
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
int32
HMCPwrite(accrec_t * access_rec, /* IN: access record to mess with */
          int32 length,          /* IN: number of bytes to write */
          const void * datap      /* IN: buffer for data */)
{
    CONSTR(FUNC, "HMCPwrite");    /* for HERROR */
    filerec_t   *file_rec = NULL; /* file record */
    chunkinfo_t *info     = NULL; /* chunked element information record */
#ifdef UNUSED
    CHUNK_REC   *chk_rec  = NULL; /* current chunk */
    uint8       *data     = NULL; /* data buffer */
    TBBT_NODE   *entry    = NULL; /* node off of  chunk tree */
#endif /* UNUSED */
    CHUNK_REC   *chkptr   = NULL; /* Chunk record to inserted in TBBT  */
    int32       *chk_key  = NULL; /* Chunk recored key for insertion in TBBT */
    const uint8 *bptr     = NULL; /* data buffer pointer */
    void        *chk_data = NULL; /* chunk data */
    uint8       *chk_dptr = NULL; /* chunk data pointer */
    int32       relative_posn;    /* relative position in chunked element */
    int32       bytes_written = 0;/* total #bytes written by HMCIwrite */
    int32       write_len = 0;    /* next write size */
    int32       write_seek = 0;   /* next write seek */
    int32       chunk_size = 0;   /* chunk size */
    int32       chunk_num = 0;    /* chunk number */
    int32       ret_value = SUCCEED;
    intn        k;                  /* loop index */
#ifdef CHK_DEBUG_4
    intn         i;
#endif

#ifdef CHK_DEBUG_4
    printf("HMCPwrite called with length %d \n",length);
#endif
    /* Check args */
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Set inputs */
#ifdef UNUSED
    data     = (uint8 *) datap;
#endif /* UNUSED */
    file_rec =  HAatom_object(access_rec->file_id);
    info     = (chunkinfo_t *) (access_rec->special_info);
    relative_posn = access_rec->posn;
    write_len     = length;

    /* validate length and file records */
    if (length <= 0)
        HGOTO_ERROR(DFE_RANGE, FAIL);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* should chunk indicies be updated with relative_posn? 
       or did last operation update it already */
    update_chunk_indicies_seek(access_rec->posn,info->ndims, info->nt_size,
                               info->seek_chunk_indices,
                               info->seek_pos_chunk,info->ddims);

    bytes_written = 0;
    bptr = datap;
    while (bytes_written < write_len) 
      {
          /* for debuging */
#ifdef CHK_DEBUG_4
          printf("Seek start(in chunk array):(");
          for (i = 0; i < info->ndims; i++)
              printf("%d%s", info->seek_chunk_indices[i], i!= info->ndims-1 ? ",":NULL);
          printf(")\n");
          printf(" Seek start(within the chunk):(");
          for (i = 0; i < info->ndims; i++)
              printf("%d%s", info->seek_pos_chunk[i], i!= info->ndims-1 ? ",":NULL);
          printf(")\n");
#endif

          /* calculate chunk to retrieve */
          calculate_chunk_num(&chunk_num,info->ndims,info->seek_chunk_indices,
                              info->ddims);

          /* calculate contiguous chunk size that we can write to this chunk */
          calculate_chunk_for_chunk(&chunk_size,info->ndims,info->nt_size,
                                    write_len,bytes_written,
                                    info->seek_chunk_indices,
                                    info->seek_pos_chunk,info->ddims);

#ifdef CHK_DEBUG_4
          printf("    writing chunk(%d) of %d bytes ->\n", chunk_num, chunk_size);
#endif

          /* find chunk record in TBBT */
          if (tbbtdfind(info->chk_tree, &chunk_num, NULL) == NULL)
            { /* not in tree */
                
                /* so create a new chunk record */
                /* Allocate space for a chunk record */
                if ((chkptr = (CHUNK_REC *) HDmalloc(sizeof(CHUNK_REC))) == NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);

                /* Allocate space for a origin in chunk record */
                if ((chkptr->origin = (int32 *) HDmalloc((size_t)info->ndims*sizeof(int32))) == NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);

                /* allocate space for key */
                if ((chk_key = (int32 *)HDmalloc(sizeof(int32))) == NULL)
                    HGOTO_ERROR(DFE_NOSPACE, FAIL);

                /* Initialize chunk record */
                chkptr->chk_tag = DFTAG_NULL;
                chkptr->chk_ref = 0;
#ifdef CHK_DEBUG_4
                printf(" chktpr->chk_tag=%d, ",chkptr->chk_tag);
                printf(" chktpr->chk_ref=%d, ",chkptr->chk_ref);
#endif
                /* Intialize chunk origins */
                for (k = 0; k < info->ndims; k++)
                      chkptr->origin[k] = info->seek_chunk_indices[k];
#ifdef CHK_DEBUG_4
                printf(" chkptr->origin = (");
                for (k = 0; k < info->ndims; k++)
                    printf("%d%s", chkptr->origin[k], k!= info->ndims-1 ? ",":NULL);
                printf(")\n");
#endif
                /* set chunk record number to next Vdata record number */
                chkptr->chk_vnum = info->num_recs++;

                /* set key to chunk number */
                chkptr->chunk_number = *chk_key = chunk_num;

                /* add to TBBT tree based on chunk number as the key */
                tbbtdins(info->chk_tree, chkptr , chk_key);   

#ifdef UNUSED
                /* assign over new chk */
                chk_rec = chkptr;
#endif /* UNUSED */

                /* re-intialize ptrs to allow for error-failure check */
                chkptr = NULL;
                chk_key = NULL;
            }
#ifdef UNUSED
          else /* already in TBBT tree */
              chk_rec = (CHUNK_REC *) entry->data; /* get file entry from node */
#endif /* UNUSED */

          /* would be nice to get Chunk record from TBBT based on chunk number 
             and then get chunk data base on chunk vdata number but
             currently the chunk calculations return chunk 
             numbers and not Vdata record numbers. 
             This would reduce some overhead in the number of chunks
             dealt with in the cache */
#ifdef CHK_DEBUG_4
          printf("  getting chunk %d from cache\n",chunk_num);
#endif
          /* get chunk data from cache based on chunk number 
             chunks in the cache start from 1 not 0 */
          if ((chk_data = mcache_get(info->chk_cache, /* cache handle */
                                              chunk_num+1,     /* chunk number */
                                              0                /* flag: unused */)) 
              == NULL)
              HE_REPORT_GOTO("failed to find chunk record", FAIL);

          chk_dptr = chk_data; /* set chunk data ptr */

          /* calculate position in chunk */
          calculate_seek_in_chunk(&write_seek,info->ndims,info->nt_size,
                                  info->seek_pos_chunk,
                                  info->ddims);

          chk_dptr += write_seek; /* move to correct position in chunk */

#ifdef CHK_DEBUG_4
          fprintf(stderr,"  write pos in chunk (%d) is %d bytes\n", chunk_num, write_seek);
#endif
          /* copy data from users buffer to chunk */
          HDmemcpy(chk_dptr, bptr, chunk_size);        

#ifdef CHK_DEBUG_10
          printf(" chk_dptr={");
          for (i = 0; i < chunk_size; i++)
              printf("%d,",(uint8)*((uint8 *)(chk_dptr)+i));
          printf("}\n");
#endif
          /* put chunk back to cache as DIRTY */
          if (mcache_put(info->chk_cache, /* cache handle */
                         chk_data,        /* whole data chunk */
                         MCACHE_DIRTY     /* flag:  DIRTY */) 
              == FAIL)
              HE_REPORT_GOTO("failed to put chunk back in cache", FAIL);

          /* increment buffer pointer */
          bptr += chunk_size;

          /* adjust number of bytes already written */
          bytes_written += chunk_size; 

#ifdef CHK_DEBUG_4
          printf("     written %d bytes already -> \n", bytes_written);
#endif
          /* update relative position i.e. user element seek postion 
             with chunk size written */
          relative_posn += chunk_size;
#ifdef CHK_DEBUG_4
          printf("  relative_posn = %d bytes \n", relative_posn);
#endif
          /* i.e calculate chunk indices given seek location 
             this will update the proper arrays in the special info struct */
          update_chunk_indicies_seek(relative_posn,info->ndims, info->nt_size,
                                     info->seek_chunk_indices,
                                     info->seek_pos_chunk,info->ddims);
      } /* end while "bytes_written" */

    /* update access record with bytes written */
    access_rec->posn += bytes_written; 

    ret_value = bytes_written;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
          /* check chunk ptrs */
          if (chkptr != NULL)
            {
                if (chkptr->origin != NULL)
                    HDfree(chkptr->origin);
                HDfree(chkptr);
            }
          if (chk_key != NULL)
              HDfree(chk_key);
      } /* end if */

    /* Normal function cleanup */

#ifdef CHK_DEBUG_4
    printf("HMCPwrite: exited, ret=%d \n",ret_value);
#endif
    return ret_value;
}   /* HMCPwrite */


/* ---------------------------------------------------------------------
NAME
   HMCPcloseAID -- close file but keep AID active

DESCRIPTION
   Close the file currently being pointed to by this AID but 
   do *NOT* free the AID.

   This will flush the chunk cache and free up the special info struct.

   This is called by Hnextread() which reuses an AID to point to
   the 'next' object as requested.  If the current object was an
   chunked object, the chunked information needs to be closed before all
   reference to it is lost.

   NOTE: Hnextread() is a bad fcn to use since it relies on previous state
         information. 

RETURNS
   SUCCEED / FAIL
AUTHOR
   -GeorgeV - 9/3/96
---------------------------------------------------------------------------*/
int32
HMCPcloseAID(accrec_t *access_rec /* IN:  access record of file to close */)
{
    CONSTR(FUNC, "HMCPcloseAID");    /* for HERROR */
    chunkinfo_t *info     = NULL;    /* special information record */
    int32       ret_value = SUCCEED;

    /* check args */
    info =  (chunkinfo_t *) access_rec->special_info;
    if (info == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* detach the special information record.
       If no more references to that, free the record */
    if (--(info->attached) == 0)
      {
#ifdef CHK_DEBUG_2
    fprintf(stderr,"HMCPcloseAID: info->attached =%d, last one \n", info->attached);
            
#endif
          if (info->chk_cache != NULL)
            {
                /* Sync chunk cache */
                mcache_sync(info->chk_cache);
#ifdef STATISTICS
                /* cache statistics if 'mcache.c' complied with -DSTATISTICS */
                mcache_stat(info->chk_cache);
#endif
                /* close chunk cache */
                mcache_close(info->chk_cache);
            } /* cache not empty */

          /* clean up chunk table lists and info record here */
          /* Use Vxxx interface to end access to Vdata info */
          if (info->aid != FAIL)
            {
                if (VSdetach(info->aid) == FAIL)
                    HGOTO_ERROR(DFE_CANTFLUSH, FAIL);
            }
          else
              HGOTO_ERROR(DFE_BADAID, FAIL);

          if (Vend(access_rec->file_id) == FAIL)
              HGOTO_ERROR(DFE_CANTFLUSH, FAIL);

          /* clean up chunk tree */
          tbbtdfree(info->chk_tree, chkdestroynode, chkfreekey);

          /* free up stuff in special info */
          if (info->ddims != NULL)
              HDfree(info->ddims);
          if (info->seek_chunk_indices != NULL)
              HDfree(info->seek_chunk_indices);
          if (info->seek_pos_chunk != NULL)
              HDfree(info->seek_pos_chunk);
          if (info->seek_user_indices != NULL)
              HDfree(info->seek_user_indices);
          if (info->fill_val != NULL)
              HDfree(info->fill_val);
          if (info->comp_sp_tag_header != NULL)
              HDfree(info->comp_sp_tag_header);
          if (info->cinfo != NULL)
              HDfree(info->cinfo);
          if (info->minfo != NULL)
              HDfree(info->minfo);
          /* finally free up info */
          HDfree(info);
          access_rec->special_info = NULL;
      } /* attached to info */
    else
      {
#ifdef CHK_DEBUG_2
    fprintf(stderr,"HMCPcloseAID: info->attached =%d \n", info->attached);
            
#endif
      }

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */

    return ret_value;
}   /* HMCPcloseAID */

/* ----------------------------- HPendaccess -----------------------------
NAME
   HMCPendacess -- close a chunk element AID

DESCRIPTION
   Free up all of the space used to store information about a
   chunked element. All relevant info will be flushed.
   Update proper records i.e. access_rec, file_rec..etc

RETURNS
   SUCCEED / FAIL
AUTHOR
   -GeorgeV - 9/3/96
--------------------------------------------------------------------------- */
intn
HMCPendaccess(accrec_t * access_rec /* IN:  access record to close */)
{
    CONSTR(FUNC, "HMCPendaccess");   /* for HERROR */
    filerec_t   *file_rec = NULL;    /* file record */
    intn        ret_value = SUCCEED;

    /* validate arguments first */
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);        

    /* get file rec and special info */
    file_rec = HAatom_object(access_rec->file_id);

    /* validate file record */
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* detach the special information record.
       If no more references to that, free the record */
    if (HMCPcloseAID(access_rec) == FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* update file and access records */
    if (HTPendaccess(access_rec->ddid) == FAIL)
        HGOTO_ERROR(DFE_CANTFLUSH, FAIL);

    /* detach from the file */
    file_rec->attach--;

#ifdef CHK_DEBUG_2
    fprintf(stderr,"HMCPendaccess: file_rec->attach =%d \n", file_rec->attach);
            
#endif
    /* free the access record */
    HIrelease_accrec_node(access_rec);

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value;
}   /* HMCPendaccess */

/* ------------------------------- HMCPinfo --------------------------------
NAME
   HMCPinfo -- return info about a chunked element

DESCRIPTION
   Return information about the given chunked element.  
   'info_chunk' is assumed to be non-NULL.
   
RETURNS
   SUCCEED / FAIL
AUTHOR
   -GeorgeV - 9/3/96
--------------------------------------------------------------------------- */
int32
HMCPinfo(accrec_t *access_rec,       /* IN: access record of access elemement */
         sp_info_block_t *info_chunk /* OUT: information about the special element */)
{
    CONSTR(FUNC, "HMCPinfo");       /* for HERROR */
    chunkinfo_t *info     = NULL;   /* special information record */
    int32       ret_value = SUCCEED;
    intn        i;                  /* loop variable */

    /* Check args */
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* validate access record */
    if (access_rec->special != SPECIAL_CHUNKED)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* fill in the info_chunk */
    info =  (chunkinfo_t *) access_rec->special_info;
    info_chunk->key        = SPECIAL_CHUNKED;
    info_chunk->chunk_size = (info->chunk_size * info->nt_size); /* phsyical size */
    info_chunk->ndims      = info->ndims; 
    if ((info->flag & 0xff) == SPECIAL_COMP) /* only using 8bits for now */
      {
          info_chunk->comp_type  = (int32)info->comp_type;
          info_chunk->model_type = (int32)info->model_type;
      }
    else
      {
          info_chunk->comp_type  = COMP_CODE_NONE;
          info_chunk->model_type = 0;
      }

    /* allocate space for chunk lengths */
    if (( info_chunk->cdims = (int32 *) HDmalloc((size_t)info->ndims*sizeof(int32)))==NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* copy info over */
    for (i = 0; i < info->ndims; i++)
      {
          info_chunk->cdims[i] = info->ddims[i].chunk_length; 
      }

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
          if (info_chunk->cdims != NULL)
              HDfree(info_chunk->cdims);
      } /* end if */

    /* Normal function cleanup */

    return ret_value;
}   /* HMCPinfo */

/* ------------------------------ HMCPinquire ----------------------------- 
NAME
   HMCPinquire -- Hinquire for chunked elements

DESCRIPTION
   Return interesting information about a chunked element.
   NULL can be passed for any of the OUT parameters if their
   value is not needed.

RETURNS
   SUCCEED/FAIL
AUTHOR
   -GeorgeV - 9/3/96
--------------------------------------------------------------------------- */
int32
HMCPinquire(accrec_t *access_rec,  /* IN:  access record to return info about */
            int32 *pfile_id,       /* OUT: file ID; */
            uint16 *ptag,          /* OUT: tag of info record; */
            uint16 *pref,          /* OUT: ref of info record; */
            int32 *plength,        /* OUT: length of element; */
            int32 *poffset,        /* OUT: offset of element -- meaningless */
            int32 *pposn,          /* OUT: current position in element; */
            int16 *paccess,        /* OUT: access mode; */
            int16 *pspecial        /* OUT: special code; */)
{
    CONSTR(FUNC, "HMCPinquire");    /* for HERROR */
    uint16      data_tag, data_ref; /* Tag/ref of the data in the file */
    chunkinfo_t *info = NULL;       /* special information record */
    int32       ret_value = SUCCEED;

    /* Check args */
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get special info */
    info =   (chunkinfo_t *) access_rec->special_info;

    /* get latest info for the dataset */
    if(HTPinquire(access_rec->ddid,&data_tag,&data_ref,NULL,NULL)==FAIL)
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* fill in the variables if they are present */
    if (pfile_id)
        *pfile_id = access_rec->file_id;
    if (ptag)
        *ptag = data_tag;
    if (pref)
        *pref = data_ref;
    if (plength)
        *plength = (info->length * info->nt_size);
    if (poffset)
        *poffset = 0;   /* meaningless */
    if (pposn)
        *pposn = access_rec->posn;
    if (paccess)
        *paccess = (int16)access_rec->access;
    if (pspecial)
        *pspecial = (int16)access_rec->special;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value;
}   /* HMCPinquire */

/* -------------------------------------------------------------------------
NAME
   HMCPgetnumrecs -- get the number of records in a chunked element
DESCRIPTION
   Retrieves the number of records in a chunked element.  
   This function was originally added for SDcheckempty/HDcheckempty to 
   determine whether a chunked SDS has been written with data.
RETURNS
   SUCCEED/FAIL - FAIL when num_recs is NULL
AUTHOR
   bmribler - 10/3/2004
---------------------------------------------------------------------------*/
int32
HMCPgetnumrecs(accrec_t* access_rec,	/* access record */
               int32 *num_recs		/* OUT: length of the chunked elt */)
{
    CONSTR(FUNC, "HMCPgetnumrecs");	/* for HGOTO_ERROR */
    chunkinfo_t *chunk_info = NULL;	/* chunked element information record */
    int32       ret_value = SUCCEED;

    /* Check args */
    if (access_rec == NULL)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get the special info from the given record */
    chunk_info = (chunkinfo_t *) access_rec->special_info;
    if (chunk_info == NULL) HGOTO_ERROR(DFE_ARGS, FAIL);

    if (num_recs)
        *num_recs = chunk_info->num_recs;
    else
	ret_value = FAIL;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value;
}   /* HMCPgetnumrecs */

