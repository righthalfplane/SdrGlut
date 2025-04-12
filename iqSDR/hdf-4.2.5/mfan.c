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

/* $Id: mfan.c 4932 2007-09-07 17:17:23Z bmribler $ */

/*-----------------------------------------------------------------------------
 * File:     mfan.c
 * Author:   GeorgeV
 * Purpose:  Multi-file read and write annotations: labels and descriptions 
 *           of data items and file
 * Invokes:  
 * Comments:
 *           Currently the id returned by ANstart() is the same as the
 *           one returned by Hopen().
 *
 *           Currently as least 4 TBBT trees are created to handle 
 *           annotations(1 for file_ann, 1 for file_desc, 1 for data_ann 
 *           and 1 for data_desc) for each file which is not pretty but it 
 *           does make search/find of annotations much easier.
 *
 *           NOTE2: Note that any mention of file ID's except in ANStart() 
 *                  should now refer to annotation interface ID.
 *
 * Contents: 
 *
 *  Private Routines:
 *  -----------------
 *  NOTES: TYPE here refers to file/data label/description types 
 *         They are AN_FILE_LABEL, AN_FILE_DESC, AN_DATA_LABEL, AN_DATA_DESC
 *         The tag/ref refers to data tag/ref
 *
 *  ANinit      - Intialize the annotation interface
 *  ANdestroy   - Un-intialize the annotation interface
 *
 *  ANIanncmp    - compare two annotation handles(ann_id's) 
 *                  (used in annotation TBBTtree)
 *  ANIaddentry:  - add entry to corresponding annotation TBBTtree
 *  ANIcreate_ann_tree - create annotation TBBTtree 
 *  ANIfind:      - return annotation handle(ann_id) if found of given TYPE/ref
 *  ANInumann:    - return number of annotations that match TYPE/tag/ref
 *  ANIannlist:   - return list of handles(ann_id's) that match TYPE/tag/ref
 *  ANIannlen:    - get length of annotation given handle(ann_id)
 *  ANIreadann:   - read annotation given handle(ann_id)
 *  ANIwriteann:  - write annotation given handle(ann_id)
 *  ANIcreate     - create a new file/label annotation 
 *                  and return a handle(ann_id)
 *
 *  Public API routines:
 *  --------------------
 *  ANstart     - open file for annotation handling, returns an_id
 *  ANfileinfo  - get number of file/data annotations in file. Indices returned
 *                are used in ANselect() calls.
 *  ANend       - end access to annotation handling on file
 *  ANcreate    - create a new data annotation and return a handle(ann_id)
 *  ANcreatef   - create a new file annotation and return a handle(ann_id)
 *  ANselect    - returns an annotation handle(ann_id) from index for 
 *                a particular annotation TYPE. This handle is then used for
 *                calls like ANwriteann(), ANreadann(), ANannlen(),..etc
 *  ANnumann:   - return number of annotations that match TYPE/tag/ref
 *  ANannlist:  - return list of handles(ann_id's) that match TYPE/tag/ref
 *  ANannlen:   - get length of annotation given handle(ann_id)
 *  ANreadann:  - read annotation given handle(ann_id)
 *  ANwriteann: - write annotation given handle(ann_id)
 *  ANendaccess - end access to annotation using handle(ann_id)
 *
 *  Public Miscellaneous API Routines
 *  ---------------------------------
 *  ANget_tagref - get tag/ref pair to annotation type and index
 *  ANid2tagref  - get tag/ref pair to annotation ID
 *  ANtagref2id  - get annotation id given tag/ref
 *  ANatype2tag - annotation type to corresponding annotation TAG
 *  ANtag2atype - annotation TAG to corresponding annotation type
 *
 *---------------------------------------------------------------------------*/

#ifndef MFAN_C  /* define main annoation source file */
#define MFAN_C

#include "mfan.h"
#include "atom.h"
#include "hfile.h" /* needed for filerec_t */

/* Whether we've installed the library termination function yet for this 
   interface */
PRIVATE intn library_terminate = FALSE;

/* Function Prototypes for fcns used by TBBT. Can not be PRIVATE. */
extern void ANfreedata(void * data);
extern void ANfreekey(void * key);
extern void dumpentryKey(void *key, void *data);
extern intn ANIanncmp(void * i, void * j, intn value);

/* private initialization routine */
PRIVATE intn ANIstart(void);

/*-----------------------------------------------------------------------------
 *                          Internal Routines
 *---------------------------------------------------------------------------*/

/* ------------------------ Routines for TBBT ------------------------------*/
/* free data - used by tbbt routines */
void
ANfreedata(void * data)
{
    HDfree(data);
} /* ANfreekey() */

/* free key - used by tbbt routines */
void
ANfreekey(void * key)
{
    HDfree(key);
} /* ANfreekey() */

#ifdef AN_DEBUG
/* The following routine is used for debugging purposes to dump 
 * key/data pairs from the TBBT trees 
 * eg. tbbt_dump(tree, dumpentryKey, 0)*/
void
dumpentryKey(void *key, void *data)
{
    ANentry *node = NULL;

    printf("key = %4.4d, data = ",*(int*)key);
    if (data != NULL) 
      {
          node = (ANentry *) data;
          printf("ann_id=%ld, annref=%d, elmtag=%d, elmref=%d\n", 
                 (long)node->ann_id, node->annref, node->elmtag, node->elmref);
      }
    else 
        printf("(NULL)\n");
    return;
} /* dumpentryKey() */
#endif /* AN_DEBUG */

/* ------------------------ Private Internal Routines ---------------------*/

/* ------------------------------- ANIanncmp -------------------------------- 
 NAME
	ANIanncmp -- compare two annotation keys or id's

 DESCRIPTION
    Compares two annotation keys. Used by tree for annotations
    Also used to compare annotation id's since also 32 bit value.

 RETURNS
    Returns 0 if i=j, -1 if i > j and 1 if i < j

 AUTHOR
    GeorgeV.
--------------------------------------------------------------------------- */
intn 
ANIanncmp(void * i,   /* IN: annotation key(tag,ref) */
          void * j,   /* IN: annotation key(tag,ref) */
          intn value /* not used */)
{
    /* shut compiler up */
    value = value;

    if (*(int32 *)i == *(int32 *)j) 
        return 0;
    if (*(int32 *)i > *(int32 *)j) 
        return -1; 
    else 
        return 1;
} /* ANIanncmp */

/*--------------------------------------------------------------------------
 NAME
    ANIstart - AN-level initialization routine.

 DESCRIPTION
    Register the shut-down routine (ANPdestroy) for call with atexit()

 RETURNS
    Returns SUCCEED/FAIL

 AUTHOR
    GeorgeV.

--------------------------------------------------------------------------*/
PRIVATE intn 
ANIstart(void)
{
    CONSTR(FUNC, "ANIstart");    /* for HERROR */
    intn        ret_value = SUCCEED;

    /* Don't call this routine again... */
    library_terminate = TRUE;

    /* Install atexit() library cleanup routine ANdestroy() */
    if (HPregister_term_func(&ANdestroy) != 0)
        HGOTO_ERROR(DFE_CANTINIT, FAIL);

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */

    return(ret_value);
} /* end ANIstart() */

/* ------------------------------- ANinit -------------------------------- 
 NAME
	ANinit -- Initialize Annotation Interface

 DESCRIPTION
    Initializes the annotation interface i.e. installs library
    cleanup routine and creates the Atom group ANIDGROUP for annotations 
    if necessary.

 RETURNS
    SUCCEED or FAIL

 AUTHOR
    GeorgeV.

--------------------------------------------------------------------------- */
PRIVATE int32
ANinit(void)
{
    CONSTR(FUNC, "ANinit");
    int32 ret_value = SUCCEED;
    
    /* Clear error stack */
    HEclear();

    /* Perform global, one-time initialization */
    if (library_terminate == FALSE)
      {
        if(ANIstart()==FAIL)
            HGOTO_ERROR(DFE_CANTINIT, FAIL);

        /* Initialize the atom groups for annotations */
        HAinit_group(ANIDGROUP,ANATOM_HASH_SIZE);
      }

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANinit() */

/* ------------------------------- ANdestroy -------------------------------- 
 NAME
	ANdestroy -- Un-Initialize Annotation Interface

 DESCRIPTION
    Destroys annotation Atom group ANIDGROUP. This routine is usually
    registered with the atexit() rouinte.

 RETURNS
    SUCCEED or FAIL

 AUTHOR
    GeorgeV.

--------------------------------------------------------------------------- */
intn
ANdestroy(void)
{
#ifdef LATER
    CONSTR(FUNC, "ANdestroy");
#endif /* LATER */
    int32    ret_value = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* Destroy the atom groups for annotations */
    HAdestroy_group(ANIDGROUP);

    return ret_value;
} /* ANdestroy () */

/*--------------------------------------------------------------------------
 NAME
   ANIaddentry -- add entry to corresponding annotation TBBT tree and 
                  atom group.

 DESCRIPTION
   Adds annotation to correct tree and register with atom group

 RETURNS
   annotation ID if successful and FAIL (-1) otherwise

 AUTHOR
    GeorgeV.

 -------------------------------------------------------------------------*/
PRIVATE int32
ANIaddentry(int32 an_id,    /* IN: annotation interface id */
            ann_type type,  /* IN: annotation type 
                                   AN_DATA_LABEL for data labels, 
                                   AN_DATA_DESC for data descriptions,
                                   AN_FILE_LABEL for file labels,
                                   AN_FILE_DESC for file descritpions.*/ 
            uint16 ann_ref, /* IN: ref of annotation */
            uint16 elmtag,  /* IN: tag of item of which this is annotation */
            uint16 elmref,  /* IN: ref of item of which this is annotation */
            intn new_ann    /* IN: new annotation flag */)
{
    CONSTR(FUNC, "ANIaddentry");
    filerec_t   *file_rec = NULL;		/* file record pointer */
    uint16       ann_tag  = DFTAG_NULL;
    int32       *ann_key  = NULL;
    ANentry     *ann_entry  = NULL;
    ANnode      *ann_node   = NULL;
    int32        ret_value  = SUCCEED;
 
    /* Clear error stack */
    HEclear();

    /* convert an_id i.e. file_id to file rec and check for validity */
    file_rec = HAatom_object(an_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Check for empty annotation tree of 'type'? */   
    if (file_rec->an_num[type] == -1)
      {
          if ((file_rec->an_tree[type] = 
               (TBBT_TREE *) tbbtdmake(ANIanncmp,sizeof(int32),0)) == NULL)
            {
              HE_REPORT_GOTO("failed to create annotation tree", FAIL);
            }

          file_rec->an_num[type] = 0;
      }

    /* Which type of annotation file/data label or desc? */
    switch((ann_type)type)
      {
      case AN_DATA_LABEL:
          ann_tag = DFTAG_DIL;
          break;
      case AN_DATA_DESC:
          ann_tag = DFTAG_DIA;
          break;
      case AN_FILE_LABEL:
          ann_tag = DFTAG_FID;
          break;
      case AN_FILE_DESC:
          ann_tag = DFTAG_FD;
          break;
      default:
          HE_REPORT_GOTO("Bad annotation type for this call",FAIL);
      }

    /* allocate space for key */
    if ((ann_key = (int32 *)HDmalloc(sizeof(int32))) == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* Create 32bit key from type/ref 
     *  ----------------------------
     *  | type(16bits)| ref(16bits) |
     *  -----------------------------*/
    *ann_key = AN_CREATE_KEY(type, ann_ref);

    /* Initialize annotation node for insertion in annotation atom group*/
    if ((ann_node = HDmalloc(sizeof(ANnode))) == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    ann_node->file_id = an_id;
    ann_node->ann_key = *ann_key;
    ann_node->new_ann = new_ann;

    /* Initialize annotation entry for insertion into corresponding TBBT */
    if ((ann_entry = HDmalloc(sizeof(ANentry))) == NULL)
        HGOTO_ERROR(DFE_NOSPACE, FAIL);

    /* register annotation with atom group ANIDGROUP */
    ann_entry->annref  = ann_ref;
    ann_entry->ann_id = HAregister_atom(ANIDGROUP,ann_node);
    if (FAIL == ann_entry->ann_id)
        HE_REPORT_GOTO("failed to insert annotation into ANIDGROUP atom group", 
                       FAIL);

    if (type == AN_DATA_LABEL || type == AN_DATA_DESC)
      {
          ann_entry->elmtag = (uint16) elmtag;
          ann_entry->elmref = (uint16) elmref;
      }
    else
      {
          ann_entry->elmtag = ann_tag;
          ann_entry->elmref = ann_ref;
      }

    /* Add annotation entry to 'type' tree */
    if (tbbtdins(file_rec->an_tree[type], ann_entry, ann_key) == NULL)
        HE_REPORT_GOTO("failed to insert annotation into 'type' tree", FAIL);

    /* increment number of annotatiosn of 'type' */
    file_rec->an_num[type] += 1;

    /* return annotation id */
    ret_value = ann_entry->ann_id;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
          if (ann_key != NULL)
              HDfree(ann_key);
          if (ann_entry != NULL)
              HDfree(ann_entry);
          if (ann_node != NULL)
              HDfree(ann_node);
      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANIaddentry */

/*--------------------------------------------------------------------------
 NAME
   ANIcreate_ann_tree --  create an annotation tree of 'type' for given file 

 DESCRIPTION
   Creates either a label or descritption annotation TBBT tree. 

 RETURNS
   Number of annotations of 'type' in file if successful and 
   FAIL (-1) otherwise

 AUTHOR
    GeorgeV.

 -------------------------------------------------------------------------*/
PRIVATE intn
ANIcreate_ann_tree(int32    an_id,/* IN: annotation interface id */
                   ann_type type  /* IN: AN_DATA_LABEL for data labels, 
                                         AN_DATA_DESC for data descriptions,
                                         AN_FILE_LABEL for file labels,
                                         AN_FILE_DESC for file descritpions.*/)
{
    CONSTR(FUNC, "ANIcreate_ann_tree");
    filerec_t  *file_rec = NULL;		/* file record pointer */
    uint8       datadi[4];
    int32       more_anns;
    int32       aid = FAIL;
    int32       nanns;
    int32       i;
    int32      *ann_key = NULL; 
    uint16      ann_tag;
    uint16      ann_ref;
    uint8      *dptr = NULL;
    ANentry    *ann_entry  = NULL;
    ANnode     *ann_node   = NULL;
    intn        ret_value  = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* convert an_id i.e. file_id to file rec and check for validity */
    file_rec = HAatom_object(an_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Check for empty annotation tree of 'type'? */   
    if (file_rec->an_num[type] == -1)
      {
          if ((file_rec->an_tree[type] = 
               (TBBT_TREE *) tbbtdmake(ANIanncmp,sizeof(int32),0)) == NULL)
            {
              HE_REPORT_GOTO("failed to create annotation tree", FAIL);
            }
          file_rec->an_num[type] = 0; /* initialize to zero entries */
      }
    else
      {
          ret_value = file_rec->an_num[type];  /* tree already created */
          goto done; /* we are done */
      }

    /* Which type of annotation data/file label or desc? */
    switch((ann_type)type)
      {
      case AN_DATA_LABEL:
          ann_tag = DFTAG_DIL;
          break;
      case AN_DATA_DESC:
          ann_tag = DFTAG_DIA;
          break;
      case AN_FILE_LABEL:
          ann_tag = DFTAG_FID;
          break;
      case AN_FILE_DESC:
          ann_tag = DFTAG_FD;
          break;
      default:
          HE_REPORT_GOTO("Bad annotation type for this call",FAIL);
      }

    /* Get number of annotations of 'type' in file */
    if ((nanns = Hnumber(an_id, ann_tag)) == 0)
      { /* ZERO annotations of 'type' in file */
          file_rec->an_num[type] = 0;
          ret_value =  file_rec->an_num[type];  
          goto done; /* we are done */
      }

    /* start read on 'type' annotations  in file 
     * note that so far an_id == file_id */
    if ((aid = Hstartread(an_id, ann_tag, DFREF_WILDCARD)) == FAIL)
      {
        HE_REPORT_GOTO("Hstartread failed to read annotation", FAIL);
      }
    else
        more_anns = SUCCEED; 

    /* Process annotations of 'type' in file */
    for (i = 0; (i < nanns) && (more_anns != FAIL); i++)
      { /* see if annotation is there */
          if (FAIL == Hinquire(aid, (int32 *) NULL, (uint16 *) NULL, &ann_ref,
                               (int32 *) NULL, (int32 *) NULL, (int32 *) NULL,
                               (int16 *) NULL, (int16 *) NULL))
            {   /* record what we found so far and return */
                file_rec->an_num[type] = nanns;
                ret_value =  FAIL;
                goto done; /* we are done */
            }

          /* if data annotation, read data annotation tag/ref */
          if(type != AN_FILE_LABEL && type != AN_FILE_DESC)
            {
                if ((int32) FAIL == Hread(aid, (int32) 4, datadi))
                  {   /* record what we found so far and return */
                      file_rec->an_num[type] = nanns;
                      ret_value =  FAIL;
                      goto done; /* we are done */
                  }
            }

          /* allocate space for key */
          if ((ann_key = (int32 *)HDmalloc(sizeof(int32))) == NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);

          /* Create key from tag/ref pair 
           *  ----------------------------
           *  | tag(16bits) | ref(16bits) |
           *  -----------------------------*/
          *ann_key = AN_CREATE_KEY(type, ann_ref);

          /* Initialize annotation node for insertion in annotation atom group*/
          if ((ann_node = HDmalloc(sizeof(ANnode))) == NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);

          ann_node->file_id = an_id;
          ann_node->ann_key = *ann_key;
          ann_node->new_ann = 0;       /* not a newly created annotation */

          /* Initialize annotation entry for insertion into corresponding TBBT */
          /* and  decode data tag/ref */
          if ((ann_entry = HDmalloc(sizeof(ANentry))) == NULL)
              HGOTO_ERROR(DFE_NOSPACE, FAIL);

          ann_entry->annref  = ann_ref;
          /* ann_entry->ann_id  = *ann_id = num_anns++; */
          ann_entry->ann_id = HAregister_atom(ANIDGROUP,ann_node);
          if (FAIL == ann_entry->ann_id)
              HE_REPORT_GOTO("failed to insert annotation into ann_id Group", FAIL);

          /* Check if data annotation to decode data tag/ref */
          if(type != AN_FILE_LABEL && type != AN_FILE_DESC)
            {
                dptr = (uint8 *) &(datadi[0]);
                UINT16DECODE(dptr, ann_entry->elmtag);
                UINT16DECODE(dptr, ann_entry->elmref);
            }
          else
            {
                ann_entry->elmtag = ann_tag;
                ann_entry->elmref = ann_ref;
            }

          /* Add annotation entry to 'type' tree */
          if (tbbtdins(file_rec->an_tree[type], ann_entry, ann_key) == NULL)
              HE_REPORT_GOTO("failed to insert annotation into 'type' tree", FAIL);

          /* set read on next annotation */
          more_anns = Hnextread(aid, ann_tag, DFREF_WILDCARD, DF_CURRENT);
      } /* end for "more_anns" */
    
    /* Finish access*/
    if (FAIL != aid)
      {
        if (FAIL == Hendaccess(aid))
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
      }

    /* set return value */
    ret_value = file_rec->an_num[type] = nanns;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
          if (ann_key != NULL)
              HDfree(ann_key);
          if (ann_entry != NULL)
              HDfree(ann_entry);
          if (ann_node != NULL)
              HDfree(ann_node);
          if (FAIL != aid)
              Hendaccess(aid);
      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANIcreate_ann_tree */

#if NOT_USED_YET
/*--------------------------------------------------------------------------
 NAME
     ANIfind -- find annotation id for given annotation type and ref number

 DESCRIPTION
     Find annotation id for the given annotation type and ref number

 RETURNS
    Annotation id if successful and FAIL (-1) otherwise

 AUTHOR
    GeorgeV.

 -------------------------------------------------------------------------*/ 
PRIVATE int32
ANIfind(int32    an_id, /* IN: annotation interface id */
        ann_type type,  /* IN: AN_DATA_LABEL for data labels, 
                               AN_DATA_DESC for data descriptions,
                               AN_FILE_LABEL for file labels,
                               AN_FILE_DESC for file descritpions.*/ 
        uint16   ann_ref /* IN: ref of annotation */)
{
    CONSTR(FUNC, "ANIfind");
    filerec_t  *file_rec = NULL;		/* file record pointer */
    TBBT_NODE  *entry    = NULL;
    ANentry    *ann_entry  = NULL;
    int32       ann_key;
    int32       ret_value = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* convert an_id i.e. file_id to file rec and check for validity */
    file_rec = HAatom_object(an_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Check for empty annotation tree of 'type'? */   
    if (file_rec->an_num[type] == -1)
      {
          if ((file_rec->an_tree[type] = 
               (TBBT_TREE *) tbbtdmake(ANIanncmp,sizeof(int32),0)) == NULL)
            {
              HE_REPORT_GOTO("failed to create annotation tree", FAIL);
            }

          file_rec->an_num[type] = 0; /* intialize after allocation */
      }

    /* Create key from type/ref pair 
     *  ----------------------------
     *  | type(16bits) | ref(16bits) |
     *  -----------------------------*/
    ann_key = AN_CREATE_KEY(type, ann_ref);

    /* See if annotation of 'type' with ref exits */
    if ((entry = tbbtdfind(file_rec->an_tree[type], &ann_key, NULL)) == NULL)
        HE_REPORT_GOTO("failed to find annotation of 'type'", FAIL);

    /* get annotation entry from node */
    ann_entry = (ANentry *) entry->data; 

    /* return annotation id */
    ret_value = ann_entry->ann_id;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANIfind */
#endif /* NOT_USED_YET */

/*--------------------------------------------------------------------------
 NAME
   ANInumann -- find number of annotation of 'type' that 
                 match the given element tag/ref 

 DESCRIPTION
   Find number of annotation of 'type' for the given element 
   tag/ref pair. Not used for file lables/descs since there
   can only be one set of them for a file.

 RETURNS
   Number of annotation found if successful and FAIL (-1) otherwise

 AUTHOR
    GeorgeV.

 -------------------------------------------------------------------------*/ 
PRIVATE intn
ANInumann(int32    an_id,   /* IN: annotation interface id */
          ann_type type,    /* IN: AN_DATA_LABEL for data labels, 
                                   AN_DATA_DESC for data descriptions,
                                   AN_FILE_LABEL for file labels,
                                   AN_FILE_DESC for file descritpions.*/
          uint16   elem_tag, /* IN: tag of item of which this is annotation */
          uint16   elem_ref  /* IN: ref of item of which this is annotation */)
{
    CONSTR(FUNC, "ANInumann");
    filerec_t  *file_rec = NULL;		/* file record pointer */
    TBBT_NODE  *entry    = NULL;
    ANentry    *ann_entry = NULL;
    intn        nanns     = 0; 
    intn        ret_value = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* convert an_id i.e. file_id to file rec and check for validity */
    file_rec = HAatom_object(an_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Empty annotation tree */
    if (file_rec->an_num[type] == -1)
      {
          if (ANIcreate_ann_tree(an_id, type) == FAIL)
              HGOTO_ERROR(DFE_BADCALL, FAIL);
      }

    /* Traverse the list looking for a match */
    for(entry = tbbtfirst((TBBT_NODE *)*(file_rec->an_tree[type])); 
        entry != NULL; entry = tbbtnext(entry))
      {
          ann_entry = (ANentry *) entry->data; /* get annotation entry from node */
          if ((ann_entry->elmref == elem_ref) && (ann_entry->elmtag == elem_tag))
            {
                nanns++; /* increment ref counter if match */
            }
      }

    /* return number of annotation references found for tag/ref */
    ret_value = nanns;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANInumann */

/*--------------------------------------------------------------------------
 NAME
   ANIannlist -- generate list of annotation ids of 'type' that 
                 match the given element tag/ref 

 DESCRIPTION
       Find and generate list of annotation ids of 'type' for the given 
       element tag/ref pair

 RETURNS
       number of annotations ids found if successful and FAIL (-1) otherwise

 AUTHOR
    GeorgeV.

 ------------------------------------------------------------------------*/ 
PRIVATE intn
ANIannlist(int32    an_id,    /* IN: annotation interface id */
           ann_type type,     /* IN: AN_DATA_LABEL for data labels, 
                                     AN_DATA_DESC for data descriptions,
                                     AN_FILE_LABEL for file labels,
                                     AN_FILE_DESC for file descritpions.*/
           uint16   elem_tag, /* IN: tag of item of which this is annotation*/
           uint16   elem_ref, /* IN: ref of item of which this is annotation */
           int32    ann_list[]/* OUT: array of ann_id's that match criteria. */)
{
    CONSTR(FUNC, "ANIannlist");
    filerec_t  *file_rec = NULL;		/* file record pointer */
    TBBT_NODE  *entry    = NULL;
    ANentry    *ann_entry = NULL;
    intn        nanns     = 0;
    intn        ret_value = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* convert an_id i.e. file_id to file rec and check for validity */
    file_rec = HAatom_object(an_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Empty annotation tree */
    if (file_rec->an_num[type] == -1)
      {
          if (ANIcreate_ann_tree(an_id, type) == FAIL)
              HGOTO_ERROR(DFE_BADCALL, FAIL);
      }

    /* Traverse the list looking for a match */
    for(entry = tbbtfirst((TBBT_NODE *)*(file_rec->an_tree[type])); 
        entry != NULL; entry = tbbtnext(entry))
      {
          ann_entry = (ANentry *) entry->data; /* get annotation entry from node */
          if ((ann_entry->elmref == elem_ref) && (ann_entry->elmtag == elem_tag))
            { /* save ref of ann match in list */
                ann_list[nanns++] = ann_entry->ann_id; 
            }
      }

    /* return number of annotation id's found for tag/ref */
    ret_value = nanns;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANIannlist */

/*--------------------------------------------------------------------------
 NAME
   ANIannlen -- get length of annotation givne annotation id

 DESCRIPTION
   Uses the annotation id to find ann_key & file_id

 RETURNS
   length of annotation if successful and FAIL (-1) otherwise

 AUTHOR
    GeorgeV.

 ------------------------------------------------------------------------*/
PRIVATE int32
ANIannlen(int32 ann_id /*  IN: annotation id */)
{
    CONSTR(FUNC, "ANIannlen");
    ANnode    *ann_node   = NULL;
    int32      file_id = FAIL;
    int32      type;
    int32      ann_key;
    uint16     ann_tag;
    uint16     ann_ref;
    int32      ann_length =(-1);
    int32      ret_value  = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* get annotation record */
    ann_node = HAatom_object(ann_id);
    if (NULL == ann_node)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get file id and annotation key */
    file_id = ann_node->file_id;
    ann_key = ann_node->ann_key;
    type    = AN_KEY2TYPE(ann_key);
    ann_ref = AN_KEY2REF(ann_key);

    /* Valid file id */
    if (file_id == FAIL)
        HE_REPORT_GOTO("bad file_id", FAIL);

    /* set type annotation tag */
    switch((int32)type)
      {
      case AN_DATA_LABEL:
          ann_tag = DFTAG_DIL;
          break;
      case AN_DATA_DESC:
          ann_tag = DFTAG_DIA;
          break;
      case AN_FILE_LABEL:
          ann_tag = DFTAG_FID;
          break;
      case AN_FILE_DESC:
          ann_tag = DFTAG_FD;
          break;
      default:
          HE_REPORT_GOTO("Bad annotation type for this call",FAIL);
      }

    if (ann_tag == DFTAG_DIL || ann_tag == DFTAG_DIA)
      {  /* 4=length of data tag/ref for data label or description */
          if ((ann_length = Hlength(file_id, ann_tag, ann_ref)) == FAIL) 
              HE_REPORT_GOTO("Failed to find annotation length", FAIL);
          ann_length -= 4;
      }
    else if (ann_tag == DFTAG_FID || ann_tag == DFTAG_FD)
      {  /* not data tag/ref for file label or description */
          if ((ann_length = Hlength(file_id, ann_tag, ann_ref)) == FAIL) 
              HE_REPORT_GOTO("Failed to find annotation length", FAIL);
      }

    /* return the length */
    ret_value = (ann_length);

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANIannlen */

/*--------------------------------------------------------------------------
 NAME
   ANIreadann -- read annotation given ann_id

 DESCRIPTION
   Gets tag and ref of annotation.  Finds DD for that annotation.
   Reads the annotation, taking care of NULL terminator, if necessary.

 RETURNS
   SUCCEED (0) if successful and FAIL (-1) otherwise

 AUTHOR
    GeorgeV.

 ------------------------------------------------------------------------*/
PRIVATE intn
ANIreadann(int32 ann_id, /* IN: annotation id (handle) */ 
           char *ann,    /* OUT: space to return annotation in */
           int32 maxlen  /* IN: size of space to return annotation in */)
{
    CONSTR(FUNC, "ANIreadann");
    ANnode  *ann_node   = NULL;
    int32   file_id = FAIL;
    int32   type;
    int32   ann_key;
    int32   aid = FAIL;
    int32   ann_len;
    uint16  ann_tag;
    uint16  ann_ref;
    uint8   datadi[4] = {0,0,0,0};   /* to read in and discard data/ref! */
    intn    ret_value = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* get annotation record */
    ann_node = HAatom_object(ann_id);
    if (NULL == ann_node)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get file id and annotation key */
    file_id = ann_node->file_id;
    ann_key = ann_node->ann_key;
    type    = AN_KEY2TYPE(ann_key);
    ann_ref = AN_KEY2REF(ann_key);

    /* Valid file id */
    if (file_id == FAIL)
        HE_REPORT_GOTO("bad file_id", FAIL);

    /* set type tag */
    switch((int32)type)
      {
      case AN_DATA_LABEL:
          ann_tag = DFTAG_DIL;
          break;
      case AN_DATA_DESC:
          ann_tag = DFTAG_DIA;
          break;
      case AN_FILE_LABEL:
          ann_tag = DFTAG_FID;
          break;
      case AN_FILE_DESC:
          ann_tag = DFTAG_FD;
          break;
      default:
          HE_REPORT_GOTO("Bad annotation type for this call",FAIL);
      }

    /* find DD for that annotation using tag/ref */
    aid = Hstartread(file_id, ann_tag, ann_ref);
    if (aid == FAIL)
        HE_REPORT_GOTO("Failed to get access to annotation",FAIL);

    if (FAIL == Hinquire(aid, (int32 *) NULL, (uint16 *) NULL, (uint16 *) NULL, 
                         &ann_len, (int32 *) NULL, (int32 *) NULL, 
                         (int16 *) NULL, (int16 *) NULL))
      {
          HE_REPORT_GOTO("Failed to get annotation",FAIL);
      }

    /* first four bytes were tag/ref if data annotation, so they don't count */
    if (ann_tag == DFTAG_DIL || ann_tag == DFTAG_DIA)
        ann_len -= 4;    
#ifdef AN_DEBUG
    printf("ANIreadann: from Hinquire, ann_len=%d, maxlen=%d\n", 
           ann_len, maxlen);
#endif
    /* Check length of space provided
     * if not enough space, truncate annotation 
     * Labels need space for null terminator, Descriptions don't */
    if (ann_tag == DFTAG_FID || ann_tag == DFTAG_DIL)
      { /* Labels */
          if (ann_len > maxlen - 1)
              ann_len = maxlen - 1;
      }
    else
      { /* Descriptions */
          if (ann_len > maxlen)
              ann_len = maxlen;
      }

    /* If data label or description need to read past tag/ref */
    if (ann_tag == DFTAG_DIL || ann_tag == DFTAG_DIA)
      { /* Data label/description */
          if ((int32) FAIL == Hread(aid, (int32) 4, datadi))
              HE_REPORT_GOTO("Failed to go past tag/ref",FAIL);
      }

    /* read itslef annotation now..*/
    if ((int32) FAIL == Hread(aid, ann_len, ann))
        HE_REPORT_GOTO("Failed to read annotation",FAIL);

    /* If Label need to NULL terminate string */
    if (ann_tag == DFTAG_FID || ann_tag == DFTAG_DIL)
        ann[ann_len] = '\0';     
#ifdef AN_DEBUG
    printf("ANIreadann: ann_len=%d, ann=%s\n", ann_len,ann);
#endif
    /* Close access to annotation object */
    if (FAIL != aid)
      {
        if (FAIL == Hendaccess(aid))
            HGOTO_ERROR(DFE_INTERNAL, FAIL);
      }

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
          if (FAIL != aid)
              Hendaccess(aid);
      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANIreadann */

/*--------------------------------------------------------------------------
 NAME
   ANIwriteann -- write annotation given ann_id

 DESCRIPTION
   Checks for pre-existence of given annotation, replacing old one if it
   exists. Writes out annotation.

 RETURNS
   SUCCEED (0) if successful and FAIL (-1) otherwise

 AUTHOR
    GeorgeV.

 ------------------------------------------------------------------------*/
PRIVATE intn
ANIwriteann(int32 ann_id,    /* IN: annotation id */
            const char *ann, /* IN: annotation to write */
            int32 ann_len    /* IN: length of annotation */)
{
    CONSTR(FUNC, "ANIwriteann");
    filerec_t  *file_rec = NULL;		/* file record pointer */
    TBBT_NODE  *entry  = NULL;
    ANentry    *ann_entry  = NULL;
    ANnode     *ann_node   = NULL;
    int32       file_id = FAIL;
    int32       type;
    int32       ann_key;
    int32       aid = FAIL;
    int         newflag = 0;
    uint16      ann_tag;
    uint16      ann_ref;
    uint16      elem_tag;
    uint16      elem_ref;
    uint8       datadi[4] = {0,0,0,0};      /* to hold data tag/ref for writing */
    uint8      *ptr = NULL;
    intn        ret_value = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* get annotation record */
    ann_node = HAatom_object(ann_id);
    if (NULL == ann_node)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get file id and annotation key */
    file_id = ann_node->file_id;
    ann_key = ann_node->ann_key;
    type    = AN_KEY2TYPE(ann_key);
    ann_ref = AN_KEY2REF(ann_key);

    /* convert file_id to file rec and check for validity */
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_INTERNAL, FAIL);

    /* set type tag */
    switch((int32)type)
      {
      case AN_DATA_LABEL:
          ann_tag = DFTAG_DIL;
          break;
      case AN_DATA_DESC:
          ann_tag = DFTAG_DIA;
          break;
      case AN_FILE_LABEL:
          ann_tag = DFTAG_FID;
          break;
      case AN_FILE_DESC:
          ann_tag = DFTAG_FD;
          break;
      default:
          HE_REPORT_GOTO("Bad annotation type for this call",FAIL);
      }

    /* Get annotation entry so that we can extract tag/ref of element 
     * Note that for file labels and descriptions the tag/ref contain
     * DFTAG_XXX and annotation reference number */
    if ((entry = tbbtdfind(file_rec->an_tree[type], &ann_key, NULL)) == NULL)
        HE_REPORT_GOTO("failed to retrieve annotation of 'type' tree", FAIL);

    ann_entry = (ANentry *) entry->data;

    elem_tag = ann_entry->elmtag;
    elem_ref = ann_entry->elmref;
    newflag  = ann_node->new_ann;

    /* is this a new annotation */
    if (newflag == 1)
      { 
          ann_node->new_ann = 0; /* set new annotation entry to false */
      }

    /* If annotation exists, re-used the DD and rewrite new annotation 
       while preserving tag/ref. We assume annotations are not stored
       as linked-blocks for now. */
    if (newflag == 0)
      {  /* annotation exists in file, re-writing */
          /* Not new, re-used the tag/ref(i.e. DD) for new annotation.
             pointer to old annotation is lost. */
          if (HDreuse_tagref(file_id, ann_tag, ann_ref) == FAIL)
              HE_REPORT_GOTO("Unable to replace old annotation",FAIL);
      }

    if (ann_tag == DFTAG_DIL || ann_tag == DFTAG_DIA)
      { /* Data label/description 
         * Note: cannot use Hputelement because need to write data tag/ref */
          aid = Hstartwrite(file_id, ann_tag, ann_ref, ann_len + 4);
          if (aid == FAIL)
              HE_REPORT_GOTO("Failed to start write access on annotation",FAIL);

          /* Setup up to write annotation tag/ref */
          ptr = (uint8 *) &(datadi[0]);   /* first, write the object's tag/ref */
          UINT16ENCODE(ptr, elem_tag);
          UINT16ENCODE(ptr, elem_ref);

          /* write annotation tag/ref of element */
          if ((int32) FAIL == Hwrite(aid, (int32) 4, datadi))
              HE_REPORT_GOTO("Failed to write tag/ref of annotation",FAIL);

#ifdef AN_DEBUG
          printf("ANIwriteann: ann_len=%d, ann=%s\n", ann_len,ann);
#endif
          /* then write the annotation itself */
          if ((int32) FAIL == Hwrite(aid, ann_len, ann))
              HE_REPORT_GOTO("Failed to write annotation",FAIL);

          /* end access to annotation */
          if (FAIL == Hendaccess(aid)) 
              HE_REPORT_GOTO("Failed to end access to annotation",FAIL);
      }
    else /* file label/description */
      { 
         /* write out file label/description */
          if (FAIL == Hputelement(file_id, ann_tag, ann_ref, (const uint8 *)ann, ann_len))
              HE_REPORT_GOTO("Failed to write file annotation",FAIL);
#ifdef AN_DEBUG
          printf("ANIwriteann: fann_len=%d, fann=%s\n", ann_len,ann);
#endif
      }

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
          if (FAIL != aid)
              Hendaccess(aid); 
      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANIwriteann */

/* ------------------------------- ANIcreate ------------------------------- 
 NAME
	ANIcreate - create a new annotation and return a handle(id)

 DESCRIPTION
    Creates an annotation, returns an 'an_id' to work with the new 
    annotation which can either be a label or description.

 RETURNS
    An ID to an annotation which can either be a label or description

 AUTHOR
    GeorgeV.

--------------------------------------------------------------------------- */
PRIVATE intn 
ANIcreate(int32    file_id,  /* IN: file ID */
          uint16   elem_tag, /* IN: tag of item to be assigned annotation */
          uint16   elem_ref, /* IN: reference number of itme to be assigned ann */ 
          ann_type type      /* IN: AN_DATA_LABEL for data labels, 
                                    AN_DATA_DESC for data descriptions,
                                    AN_FILE_LABEL for file labels,
                                    AN_FILE_DESC for file descritpions.*/)
{
    CONSTR(FUNC, "ANIcreate");    /* for HERROR */
    int32   ann_id = FAIL;
    uint16  ann_tag;
    uint16  ann_ref;
    intn    ret_value = SUCCEED;

    /* Clear error stack */
    HEclear();
  
    /* Valid file id */
    if (HAatom_group(file_id) != FIDGROUP)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* deal with type */
    switch((ann_type)type)
      {
      case AN_DATA_LABEL:
          ann_tag = DFTAG_DIL;
          ann_ref = Htagnewref(file_id,ann_tag);
          break;
      case AN_DATA_DESC:
          ann_tag = DFTAG_DIA;
          ann_ref = Htagnewref(file_id,ann_tag);
          break;
      case AN_FILE_LABEL:
          /* for file label set elmement tag/ref to ann_tag & ref */
          ann_tag = DFTAG_FID;
          ann_ref = Htagnewref(file_id,ann_tag);
          elem_tag = ann_tag;
          elem_ref = ann_ref;
          break;
      case AN_FILE_DESC:
          /* for file desc set elmement tag/ref to ann_tag & ref */
          ann_tag = DFTAG_FD;
          ann_ref = Htagnewref(file_id,ann_tag);
          elem_tag = ann_tag;
          elem_ref = ann_ref;
          break;
      default:
          HE_REPORT_GOTO("Bad annotation type for this call",FAIL);
      }

    /* Check tag and ref */
    if (!elem_tag)
        HGOTO_ERROR(DFE_BADTAG, FAIL);
    if (!elem_ref)
        HGOTO_ERROR(DFE_BADREF, FAIL);
  
    /* put new annotation tag/ref into 'type' TBBTtree */
    if ((ann_id = ANIaddentry(file_id, type, ann_ref, elem_tag, elem_ref, 1)) == FAIL)
        HE_REPORT_GOTO("Failed to add annotation to TBBT tree",FAIL);

    ret_value = ann_id;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANIcreate() */

/* --------------------- Exported Multi-file Interface ----------------------*/

/* ------------------------------- ANstart -------------------------------- 
 NAME
	ANstart -- open file for annotation handling

 DESCRIPTION
    Start annotation handling on the file return a annotation ID to the file.

 RETURNS
    A file ID or FAIL. Note that we use 'an_id' which is the same
    as the file id.

 AUTHOR
    GeorgeV.

--------------------------------------------------------------------------- */
EXPORT int32
ANstart(int32 file_id /* IN: file to start annotation access on*/)
{
    CONSTR(FUNC, "ANstart");
    filerec_t  *file_rec = NULL;		/* file record pointer */
    int32       ret_value = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* convert file id to file rec and check for validity */
    file_rec = HAatom_object(file_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* call ANinit, should just register termination function once 
       no matter how many times it is called. */
    ANinit();

    ret_value = file_id;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */
      } /* end if */

    /* Normal function cleanup */
    return ret_value;
} /* ANstart() */

/*--------------------------------------------------------------------------
 NAME
    ANfileinfo - Report high-level information about the ANxxx interface 
                 for a given file.

 DESCRIPTION
    Reports general information about the number of file and object(i.e. data)
    annotations in the file. This routine is generally used to find
    the range of acceptable indices for ANselect calls.

 RETURNS
    SUCCEED/FAIL

 AUTHOR
    GeorgeV.

--------------------------------------------------------------------------*/
EXPORT intn 
ANfileinfo(int32  an_id,        /* IN:  annotation interface id */
           int32 *n_file_label, /* OUT: the # of file labels */
           int32 *n_file_desc,  /* OUT: the # of file descriptions */
           int32 *n_obj_label,  /* OUT: the # of object labels */
           int32 *n_obj_desc    /* OUT: the # of object descriptions */)
{
    CONSTR(FUNC, "ANfileinfo");    /* for HERROR */
    filerec_t  *file_rec  = NULL;  /* file record pointer */
    intn        ret_value = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* convert an_id i.e. file_id to file rec and check for validity */
    file_rec = HAatom_object(an_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Empty file label annotation tree? */
    if (file_rec->an_num[AN_FILE_LABEL] == -1)
      {
          if ((*n_file_label = ANIcreate_ann_tree(an_id, AN_FILE_LABEL)) == FAIL)
              HE_REPORT_GOTO("failed to create file label annotation TBBTtree", FAIL);
      }
    else
        *n_file_label = file_rec->an_num[AN_FILE_LABEL];

    /* Empty file descritpiton annotation tree? */
    if (file_rec->an_num[AN_FILE_DESC] == -1)
      {
          if ((*n_file_desc = ANIcreate_ann_tree(an_id, AN_FILE_DESC)) == FAIL)
              HE_REPORT_GOTO("failed to create file desc annotation TBBTtree", FAIL);
      }
    else
        *n_file_desc = file_rec->an_num[AN_FILE_DESC];

    /* Empty label annotation tree? */
    if (file_rec->an_num[AN_DATA_LABEL] == -1)
      {
          if ((*n_obj_label = ANIcreate_ann_tree(an_id, AN_DATA_LABEL)) == FAIL)
              HE_REPORT_GOTO("failed to create data label annotation TBBTtree", FAIL);
      }
    else
        *n_obj_label = file_rec->an_num[AN_DATA_LABEL];

    /* Empty descritpiton annotation tree? */
    if (file_rec->an_num[AN_DATA_DESC] == -1)
      {
          if ((*n_obj_desc = ANIcreate_ann_tree(an_id, AN_DATA_DESC)) == FAIL)
              HE_REPORT_GOTO("failed to create data desc annotation TBBTtree", FAIL);
      }
    else
        *n_obj_desc = file_rec->an_num[AN_DATA_DESC];

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value;
} /* ANfileinfo() */

/* -------------------------------- ANend ---------------------------------
 NAME
	ANend -- End annotation access to file file

 DESCRIPTION
    End annotation access to file.

 RETURNS
    SUCCEED / FAIL
--------------------------------------------------------------------------- */
EXPORT int32
ANend(int32 an_id /* IN: Annotation ID of file to close */)
{
    CONSTR(FUNC,"ANend");
    filerec_t *file_rec = NULL;  /* file record pointer */
    TBBT_NODE *aentry   = NULL;
    ANentry   *ann_entry = NULL;
    ANnode    *ann_node  = NULL;
    int32      ret_value = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* convert an_id i.e. file_id to file rec and check for validity */
    file_rec = HAatom_object(an_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* NEED to delete trees of annotations attached to node 
     * NOTE: This could be written shorter using a for loop....
     * or something....  */

    /* free file label annotation rb tree */
    if (file_rec->an_tree[AN_FILE_LABEL] != NULL) 
      { /* Traverse tree puling ann_id's to delete from annotation atom group */
          for(aentry = tbbtfirst((TBBT_NODE *)*(file_rec->an_tree[AN_FILE_LABEL])); 
              aentry != NULL;
              aentry = tbbtnext(aentry))
            { /* get annotation entry from node */
                ann_entry = (ANentry *) aentry->data; 

                /* delete annotation node from annotation group  */
                if(NULL == (ann_node = HAremove_atom(ann_entry->ann_id)))
                    HE_REPORT_GOTO("Failed to remove annotation with ann_id", FAIL);

                if(ann_node != NULL)
                    HDfree(ann_node); /* free node */
            } /* end for 'entry */
          /* finally free tree */
          tbbtdfree(file_rec->an_tree[AN_FILE_LABEL], ANfreedata, ANfreekey);  
      }

    /* free file desc annotation rb tree */
    if (file_rec->an_tree[AN_FILE_DESC] != NULL) 
      { /* Traverse tree puling ann_id's to delete from annotation atom group */
          for(aentry = tbbtfirst((TBBT_NODE *)*(file_rec->an_tree[AN_FILE_DESC])); 
              aentry != NULL;
              aentry = tbbtnext(aentry))
            { /* get annotation entry from node */
                ann_entry = (ANentry *) aentry->data; 

                /* delete annotation node from annotation group  */
                if(NULL == (ann_node = HAremove_atom(ann_entry->ann_id)))
                    HE_REPORT_GOTO("Failed to remove annotation with ann_id", FAIL);

                if(ann_node != NULL)
                    HDfree(ann_node); /* free node */

            } /* end for 'entry */
          /* finally free tree */
          tbbtdfree(file_rec->an_tree[AN_FILE_DESC], ANfreedata, ANfreekey);  
      }

    /* free label annotation rb tree */
    if (file_rec->an_tree[AN_DATA_LABEL] != NULL) 
      { /* Traverse tree puling ann_id's to delete from annotation atom group */
          for(aentry = tbbtfirst((TBBT_NODE *)*(file_rec->an_tree[AN_DATA_LABEL])); 
              aentry != NULL;
              aentry = tbbtnext(aentry))
            { /* get annotation entry from node */
                ann_entry = (ANentry *) aentry->data; 

                /* delete annotation node from annotation group  */
                if(NULL == (ann_node = HAremove_atom(ann_entry->ann_id)))
                    HE_REPORT_GOTO("Failed to remove annotation with ann_id", FAIL);

                if(ann_node != NULL)
                    HDfree(ann_node); /* free node */
            } /* end for 'entry */
          /* finally free tree */
          tbbtdfree(file_rec->an_tree[AN_DATA_LABEL], ANfreedata, ANfreekey);  
      }

    /* free desc annotation rb tree */
    if (file_rec->an_tree[AN_DATA_DESC] != NULL) 
      { /* Traverse tree puling ann_id's to delete from annotation atom group */
          for(aentry = tbbtfirst((TBBT_NODE *)*(file_rec->an_tree[AN_DATA_DESC])); 
              aentry != NULL;
              aentry = tbbtnext(aentry))
            { /* get annotation entry from node */
                ann_entry = (ANentry *) aentry->data; 

                /* delete annotation node from annotation group  */
                if(NULL == (ann_node = HAremove_atom(ann_entry->ann_id)))
                    HE_REPORT_GOTO("Failed to remove annotation with ann_id", FAIL);

                if(ann_node != NULL)
                    HDfree(ann_node); /* free node */

            } /* end for 'entry */
          /* finally free tree */
          tbbtdfree(file_rec->an_tree[AN_DATA_DESC], ANfreedata, ANfreekey);  
      }

    /* re-initalize everything in file record for annotations so
       the a ANstart() works. */
    file_rec->an_tree[AN_DATA_LABEL] = NULL;
    file_rec->an_tree[AN_DATA_DESC]  = NULL;
    file_rec->an_tree[AN_FILE_LABEL] = NULL;
    file_rec->an_tree[AN_FILE_DESC]  = NULL;
    file_rec->an_num[AN_DATA_LABEL] = -1;   
    file_rec->an_num[AN_DATA_DESC]  = -1;   
    file_rec->an_num[AN_FILE_LABEL] = -1;   
    file_rec->an_num[AN_FILE_DESC]  = -1;   

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value;
} /* ANend() */

/* ------------------------------------------------------------------------ 
 NAME
	ANcreate - create a new element annotation and return a handle(id)

 DESCRIPTION
    Creates a data annotation, returns an 'an_id' to work with the new 
    annotation which can either be a label or description.

 RETURNS
    An ID to an annotation which can either be a label or description

 AUTHOR
    GeorgeV.

--------------------------------------------------------------------------- */
EXPORT int32
ANcreate(int32    an_id,    /* IN: annotation interface ID */
         uint16   elem_tag, /* IN: tag of item to be assigned annotation */ 
         uint16   elem_ref, /* IN: reference number of itme to be assigned ann */ 
         ann_type type      /* IN: AN_DATA_LABEL for data labels, 
                                  AN_DATA_DESC for data descriptions*/)
{
#ifdef LATER
    CONSTR(FUNC, "ANcreate");    /* for HERROR */
#endif /* LATER */
    int32    ret_value;

    ret_value = (ANIcreate(an_id, elem_tag, elem_ref, type));

    return ret_value;
} /* ANcreate() */

/* ------------------------------------------------------------------------ 
 NAME
	ANcreatef - create a new file annotation and return a handle(id)

 DESCRIPTION
    Creates a file annotation, returns an 'an_id' to work with the new 
    file annotation which can either be a label or description.

 RETURNS
    An ID to an annotation which can either be a file label or description        

 AUTHOR
    GeorgeV.

--------------------------------------------------------------------------- */
EXPORT int32
ANcreatef(int32    an_id,/* IN: annotation interface ID */
          ann_type type  /* IN:  AN_FILE_LABEL for file labels,
                                 AN_FILE_DESC for file descritpions.*/)
{
#ifdef LATER
    CONSTR(FUNC, "ANcreatef");    /* for HERROR */
#endif /* LATER */
    uint16 ann_tag;
    uint16 ann_ref;
    int32  ret_value = SUCCEED;

    /* deal with type */
    switch((ann_type)type)
      {
      case AN_FILE_LABEL:
          ann_tag = DFTAG_FID;
          ann_ref = 0; /* initalize with invalid ref, 
                          will be replaced in ANIcreate() */
          break;
      case AN_FILE_DESC:
          ann_tag = DFTAG_FD;
          ann_ref = 0; /* initialize with invalid ref, 
                          will be replaced in ANIcreate() */
          break;
      default:
          HE_REPORT_GOTO("Bad annotation type for this call",FAIL);
      }

    ret_value = ANIcreate(an_id, ann_tag, ann_ref, type);

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value;
} /* ANcreateann() */

/* ------------------------------- ANselect ------------------------------- 
 NAME
	ANselect -- get an annotation ID from index of 'type'

 DESCRIPTION
    The position index is ZERO based

 RETURNS
    An ID to an annotation type which can either be a label or description 

 AUTHOR
    GeorgeV.

--------------------------------------------------------------------------- */
EXPORT int32
ANselect(int32    an_id, /* IN: annotation interface ID */
         int32    index, /* IN: index of annottion to get ID for */
         ann_type type   /* IN: AN_DATA_LABEL for data labels, 
                                AN_DATA_DESC for data descriptions,
                                AN_FILE_LABEL for file labels,
                                AN_FILE_DESC for file descritpions.*/)
{
    CONSTR(FUNC, "ANselect");    /* for HERROR */
    filerec_t *file_rec = NULL;  /* file record pointer */
    TBBT_NODE *entry    = NULL;
    ANentry   *ann_entry = NULL;
    int32      ret_value = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* convert an_id i.e. file_id to file rec and check for validity */
    file_rec = HAatom_object(an_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Empty annotation tree */
    if (file_rec->an_num[type] == -1)
      {
          if (ANIcreate_ann_tree(an_id, type) == FAIL)
              HGOTO_ERROR(DFE_BADCALL, FAIL);
      }

    /* check index and adjust to 1 based for tbbtindx()*/
    if(index >= 0 && index < file_rec->an_num[type])
        index++;
    else
        HE_REPORT_GOTO("bad index", FAIL);

    /* find 'index' entry */
    if ((entry = tbbtindx((TBBT_NODE *)*(file_rec->an_tree[type]), index)) == NULL)
        HE_REPORT_GOTO("failed to find 'index' entry", FAIL);

    ann_entry = (ANentry *) entry->data; 

    /* return ann_id */
    ret_value = ann_entry->ann_id;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */
    return ret_value;
} /* ANselect() */

/*--------------------------------------------------------------------------
 NAME
   ANnumann -- find number of annotation of 'type' that 
               match the given element tag/ref 

 DESCRIPTION
   Find number of annotation of 'type' for the given element 
   tag/ref pair. Should not be used for File labels and
   descriptions.

 RETURNS
   Number of annotations found if successful and FAIL (-1) otherwise.

 AUTHOR
    GeorgeV.

 ------------------------------------------------------------------------*/ 
EXPORT intn
ANnumann(int32    an_id,    /* IN: annotation interface id */
         ann_type type,     /* IN: AN_DATA_LABEL for data labels, 
                                   AN_DATA_DESC for data descriptions,
                                   AN_FILE_LABEL for file labels,
                                   AN_FILE_DESC for file descritpions.*/
         uint16   elem_tag, /* IN: tag of item of which this is annotation */
         uint16   elem_ref  /* IN: ref of item of which this is annotation */ )
{
    CONSTR(FUNC, "ANnumann");
    intn   ret_value = SUCCEED;

    /* deal with invalid types */
    if(type == AN_FILE_LABEL || type == AN_FILE_DESC)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    ret_value = ANInumann(an_id, type, elem_tag, elem_ref);

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANnumann() */

/*--------------------------------------------------------------------------
 NAME
   ANannlist -- generate list of annotation ids of 'type' that 
                match the given element tag/ref 

 DESCRIPTION
   Find and generate list of annotation ids of 'type' for the given 
   element tag/ref pair. Should not be used for File labels and
   descriptions.

 RETURNS
   Number of annotations ids found if successful and FAIL (-1) otherwise

 AUTHOR
    GeorgeV.

 ------------------------------------------------------------------------*/ 
EXPORT intn
ANannlist(int32    an_id,      /* IN: annotation interface id */
          ann_type type,       /* IN: AN_DATA_LABEL for data labels, 
                                      AN_DATA_DESC for data descriptions,
                                      AN_FILE_LABEL for file labels,
                                      AN_FILE_DESC for file descritpions.*/
          uint16   elem_tag,   /* IN: tag of item of which this is annotation */
          uint16   elem_ref,   /* IN: ref of item of which this is annotation */
          int32    ann_list[]  /* OUT: array of ann_id's that match criteria. */)
{
    CONSTR(FUNC, "ANannlist");
    intn  ret_value = SUCCEED;

    /* deal with invalid types */
    if(type == AN_FILE_LABEL || type == AN_FILE_DESC)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    ret_value = ANIannlist(an_id, type, elem_tag, elem_ref, ann_list);

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANannlist() */

/*--------------------------------------------------------------------------
 NAME
   ANannlen -- get length of annotation givne annotation id

 DESCRIPTION
   Uses the annotation id to find ann_key & file_id

 RETURNS
   length of annotation if successful and FAIL (-1) otherwise

 AUTHOR
    GeorgeV.

 ------------------------------------------------------------------------*/
EXPORT int32
ANannlen(int32 ann_id /* IN: annotation id */)
{
#ifdef LATER
    CONSTR(FUNC, "ANannlen");
#endif /* LATER */
    int32  ret_value;

    ret_value =  ANIannlen(ann_id);
    return ret_value;
} /* ANannlen() */

/*--------------------------------------------------------------------------
 NAME
   ANwriteann -- write annotation given ann_id

 DESCRIPTION
   Checks for pre-existence of given annotation, replacing old one if it
   exists. Writes out annotation.

 RETURNS
   SUCCEED (0) if successful and FAIL (-1) otherwise

 AUTHOR
    GeorgeV.

 ------------------------------------------------------------------------*/
EXPORT int32
ANwriteann(int32 ann_id,     /* IN: annotation id */
           const char *ann,  /* IN: annotation to write */
           int32 annlen      /* IN: length of annotation */)
{
#ifdef LATER
    CONSTR(FUNC, "ANwriteann");    /* for HERROR */
#endif /* LATER */
    int32  ret_value;

    ret_value = ANIwriteann(ann_id, ann, annlen);
    return ret_value;
} /* ANwriteann() */

/*--------------------------------------------------------------------------
 NAME
   ANreadann -- read annotation given ann_id

 DESCRIPTION
   Gets tag and ref of annotation.  Finds DD for that annotation.
   Reads the annotation, taking care of NULL terminator, if necessary.

 RETURNS
   SUCCEED (0) if successful and FAIL (-1) otherwise

 AUTHOR
    GeorgeV.

 ------------------------------------------------------------------------*/
EXPORT int32
ANreadann(int32 ann_id,  /* IN: annotation id (handle) */
          char *ann,     /* OUT: space to return annotation in */
          int32 maxlen   /* IN: size of space to return annotation in */)
{
#ifdef LATER
    CONSTR(FUNC, "ANreadann");    /* for HERROR */
#endif /* LATER */
    int32   ret_value;

    ret_value = ANIreadann(ann_id, ann, maxlen);
    return ret_value;
} /* ANreadann() */

/* ----------------------------------------------------------------------- 
 NAME
	ANendaccess -- end access to an annotation given it's id

 DESCRIPTION
    Terminates access to an annotation. For now does nothing

 RETURNS
    SUCCEED(0) or FAIL(-1)

 AUTHOR
    GeorgeV.

--------------------------------------------------------------------------- */
EXPORT intn
ANendaccess(int32 ann_id /* IN: annotation id */)
{
#ifdef LATER
    CONSTR(FUNC, "ANendaccess");    /* for HERROR */
#endif /* LATER */
    intn  ret_value = SUCCEED;
    /* shut compiler up */
    ann_id=ann_id;

    return ret_value;
} /* ANendaccess() */

/* ----------------------------------------------------------------------- 
 NAME
	ANget_tagref - get tag/ref pair for annotation based on type and index

 DESCRIPTION
    Get the tag/ref of the annotation based on  the type and index of the 
    annotation. The position index is zero based

 RETURNS
    A tag/ref pair for an annotation type which can either be a 
    label or description.

 AUTHOR
    GeorgeV.

--------------------------------------------------------------------------- */
EXPORT int32
ANget_tagref(int32    an_id, /* IN: annotation interface ID */
             int32    index, /* IN: index of annotation to get tag/ref for */
             ann_type type,  /* IN: AN_DATA_LABEL for data labels, 
                                    AN_DATA_DESC for data descriptions,
                                    AN_FILE_LABEL for file labels,
                                    AN_FILE_DESC for file descritpions.*/
             uint16  *tag,   /* OUT: Tag for annotation */
             uint16  *ref    /* OUT: ref for annotation */)
{
    CONSTR(FUNC, "ANget_tagref");    /* for HERROR */
    filerec_t *file_rec = NULL;  /* file record pointer */
    TBBT_NODE *entry    = NULL;
    ANentry   *ann_entry = NULL;
    int32      ret_value = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* convert an_id i.e. file_id to file rec and check for validity */
    file_rec = HAatom_object(an_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* Empty annotation tree */
    if (file_rec->an_num[type] == -1)
      {
          if (ANIcreate_ann_tree(an_id, type) == FAIL)
              HGOTO_ERROR(DFE_BADCALL, FAIL);
      }

    /* check index and adjust to 1 based for tbbtindx()*/
    if(index >= 0 && index <= file_rec->an_num[type])
        index++;
    else
        HE_REPORT_GOTO("bad index", FAIL);

    /* find 'index' entry */
    if ((entry = tbbtindx((TBBT_NODE *)*(file_rec->an_tree[type]), index)) == NULL)
        HE_REPORT_GOTO("failed to find 'index' entry", FAIL);

    ann_entry = (ANentry *) entry->data; 

    /* set ref */
    *ref = ann_entry->annref;

    /* set tag based on type */
    switch((int32)type)
      {
      case AN_DATA_LABEL:
          *tag = DFTAG_DIL;
          break;
      case AN_DATA_DESC:
          *tag = DFTAG_DIA;
          break;
      case AN_FILE_LABEL:
          *tag = DFTAG_FID;
          break;
      case AN_FILE_DESC:
          *tag = DFTAG_FD;
          break;
      default:
          HE_REPORT_GOTO("Bad annotation type for this call",FAIL);
      }

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANget_tagref() */

/*--------------------------------------------------------------------------
 NAME
    ANid2tagref -- get tag/ref given annotation id

 DESCRIPTION
    Uses the annotation id to find ann_node entry which contains ann_ref

 RETURNS
    SUCCEED(0) if successful and FAIL (-1) otherwise.  

 AUTHOR
    GeorgeV.

 ------------------------------------------------------------------------*/
int32
ANid2tagref(int32   ann_id,/* IN: annotation id */
            uint16 *tag,   /* OUT: Tag for annotation */
            uint16 *ref    /* OUT: ref for annotation */)
{
    CONSTR(FUNC, "ANid2tagref");
    ANnode    *ann_node = NULL;
    int32      file_id  = FAIL;
    int32      type;
    int32      ann_key;
    uint16     ann_ref;
    int32      ret_value = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* Valid annotation id */
    /* get annotation record */
    ann_node = HAatom_object(ann_id);
    if (NULL == ann_node)
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* get file id and annotation key */
    file_id = ann_node->file_id;
    ann_key = ann_node->ann_key;
    type    = AN_KEY2TYPE(ann_key);
    ann_ref = AN_KEY2REF(ann_key);

    /* Valid file id */
    if (file_id == FAIL)
        HE_REPORT_GOTO("bad file_id", FAIL);

    *ref = ann_ref;
    /* set type annotation tag */
    switch((int32)type)
      {
      case AN_DATA_LABEL:
          *tag = DFTAG_DIL;
          break;
      case AN_DATA_DESC:
          *tag = DFTAG_DIA;
          break;
      case AN_FILE_LABEL:
          *tag = DFTAG_FID;
          break;
      case AN_FILE_DESC:
          *tag = DFTAG_FD;
          break;
      default:
          HE_REPORT_GOTO("Bad annotation type for this call",FAIL);
      }

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANid2tagref */

/*--------------------------------------------------------------------------
 NAME
       ANtagref2id -- get annotation id given tag/ref

 DESCRIPTION
       Gets the annotation id of the annotation given the tag/ref of
       the annotation itself and the annotation interface id.

 RETURNS
       Annotation id of annotation if successful and FAIL(-1) otherwise. 

 AUTHOR
    GeorgeV.

 ------------------------------------------------------------------------*/
int32
ANtagref2id(int32  an_id,   /* IN  Annotation interface id */
            uint16 ann_tag, /* IN: Tag for annotation */
            uint16 ann_ref  /* IN: ref for annotation */)
{
    CONSTR(FUNC, "ANtagref2id");
    filerec_t *file_rec = NULL;		/* file record pointer */
    TBBT_NODE *entry    = NULL;
    ANentry   *ann_entry = NULL;
    int32      ann_key;
    ann_type   type;
    int32      ret_value = SUCCEED;

    /* Clear error stack */
    HEclear();

    /* convert an_id i.e. file_id to file rec and check for validity */
    file_rec = HAatom_object(an_id);
    if (BADFREC(file_rec))
        HGOTO_ERROR(DFE_ARGS, FAIL);

    /* set type given annotation tag */
    switch((uint16)ann_tag)
      {
      case DFTAG_DIL:
          type = AN_DATA_LABEL;
          break;
      case DFTAG_DIA:
          type = AN_DATA_DESC;
          break;
      case  DFTAG_FID:
          type = AN_FILE_LABEL;
          break;
      case  DFTAG_FD:
          type = AN_FILE_DESC;
          break;
      default:
          HE_REPORT_GOTO("Bad annotation type for this call",FAIL);
      }

    /* Check for empty annotation tree of 'type'? */   
    if (file_rec->an_num[type] == -1)
      {
          if (ANIcreate_ann_tree(an_id, type) == FAIL)
              HGOTO_ERROR(DFE_BADCALL, FAIL);
      }

    /* Create key from type/ref pair 
     *  ----------------------------
     *  | type(16bits) | ref(16bits) |
     *  -----------------------------*/
    ann_key = AN_CREATE_KEY(type, ann_ref);

    /* See if annotation of 'type' with ref exits */
    if ((entry = tbbtdfind(file_rec->an_tree[type], &ann_key, NULL)) == NULL)
        HE_REPORT_GOTO("failed to find annotation of 'type'", FAIL);

    /* get annotation entry from node */
    ann_entry = (ANentry *) entry->data; 

    /* return annotation id */
    ret_value = ann_entry->ann_id;

  done:
    if(ret_value == FAIL)   
      { /* Error condition cleanup */

      } /* end if */

    /* Normal function cleanup */

    return ret_value;
} /* ANtagref2id */

/*-------------------------------------------------------------------- 
 NAME
     ANatype2tag - annotation type to corresponding annotation TAG

 DESCRIPTION
     Translate annotation type to corresponding TAG

 RETURNS
     Returns TAG corresponding to annotatin type

 AUTHOR
    GeorgeV.

--------------------------------------------------------------------*/
EXPORT uint16
ANatype2tag(ann_type atype /* IN: Annotation type */)
{   /* Switch on annotation type "atype" */
#ifdef LATER
    CONSTR(FUNC, "ANatype2tag");    /* for HERROR */
#endif /* LATER */
    uint16 ann_tag;

    switch((ann_type)atype) 
      {
      case AN_FILE_LABEL: ann_tag = DFTAG_FID; break;
      case AN_FILE_DESC:  ann_tag = DFTAG_FD;  break;
      case AN_DATA_LABEL: ann_tag = DFTAG_DIL; break;
      case AN_DATA_DESC:  ann_tag = DFTAG_DIA; break;
      default: ann_tag = DFTAG_NULL;  /*changed from 5 to DFTAG_NULL -BMR*/
      } /* switch */
    return ann_tag;
} /* ANatype2tag */

/*-------------------------------------------------------------------- 
 NAME
     ANtag2atype - annotation TAG to corresponding annotation type

 DESCRIPTION
     Translate annotation TAG to corresponding atype

 RETURNS
     Returns type corresponding to annotatin TAG

 AUTHOR
    GeorgeV.

--------------------------------------------------------------------*/
EXPORT ann_type
ANtag2atype(uint16 atag /* IN: annotation tag */)
{   /* Switch on annotation tag */
#ifdef LATER
    CONSTR(FUNC, "ANtag2atype");    /* for HERROR */
#endif /* LATER */
    ann_type atype;

    switch((uint16)atag) 
      {
      case DFTAG_FID: atype = AN_FILE_LABEL; break;
      case DFTAG_FD:  atype = AN_FILE_DESC;  break;
      case DFTAG_DIL: atype = AN_DATA_LABEL; break;
      case DFTAG_DIA: atype = AN_DATA_DESC;  break;
          /* This will cause warnings on some compiliers */
      default: atype = AN_UNDEF;
      } /* switch */
    return atype;
} /* ANtag2atype */

#endif /* MFAN_C */
