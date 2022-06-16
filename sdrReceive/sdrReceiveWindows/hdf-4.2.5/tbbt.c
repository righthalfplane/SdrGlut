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

/* $Id: tbbt.c 4932 2007-09-07 17:17:23Z bmribler $ */

/* "tbbt.c" -- Routines for using threaded, balanced, binary trees. */
/* Extended from (added threads to) Knuth 6.2.3, Algorithm A (AVL trees) */
/* Basic tree structure by Adel'son-Vel'skii and Landis */

#include <stdio.h>  /* NULL */
#include "hdf.h"
#define TBBT_INTERNALS
#include "tbbt.h"
#define   Alloc(cnt,typ)   (typ *) HDmalloc( (cnt) * sizeof(typ) )
#define   Free(x)           (HDfree((VOIDP)x))

# define   KEYcmp(k1,k2,a) ((NULL!=compar) ? (*compar)( k1, k2, a) \
                 : HDmemcmp( k1, k2, 0<(a) ? (a) : (intn)HDstrlen(k1) )  )

VOID        tbbt1dump
            (TBBT_NODE * node, intn method);

/* Function Prototypes */
extern VOID tbbt_printNode(TBBT_NODE * node, VOID(*key_dump)(VOID *,VOID *));
extern VOID tbbt_dumpNode(TBBT_NODE *node, VOID (*key_dump)(VOID *,VOID *),
                          intn method);
extern VOID tbbt_dump(TBBT_TREE *ptree, VOID (*key_dump)(VOID *,VOID *), 
                      intn method);

static TBBT_NODE *tbbt_get_node(void);
static void tbbt_release_node(TBBT_NODE *nod);

/* #define TESTING */

/* Returns pointer to end-most (to LEFT or RIGHT) node of tree: */
static TBBT_NODE *
tbbt_end(TBBT_NODE * root, intn side)
{
    if (root == NULL)
        return (NULL);
    while (HasChild(root, side))
      {
          root = root->link[side];
      }
    return (root);
}

TBBT_NODE  *
tbbtfirst(TBBT_NODE * root)
{
    return (tbbt_end(root, LEFT));
}

TBBT_NODE  *
tbbtlast(TBBT_NODE * root)
{
    return (tbbt_end(root, RIGHT));
}

/* Return address of parent's pointer to neighboring node (to LEFT or RIGHT) **
   static TBBT_NODE **tbbt_anbr(TBBT_NODE *ptr, intn side )
   {
   TBBT_NODE **aptr;

   if(  ! HasChild( ptr, side )  )
   return(  & ptr->link[side]  );
   aptr= & ptr->link[side];
   while(  HasChild( *aptr, Other(side) )  )
   aptr= & (*aptr)->link[Other(side)];
   return( aptr );
   } */

/* Returns pointer to neighboring node (to LEFT or RIGHT): */
static TBBT_NODE *
tbbt_nbr(TBBT_NODE * ptr, intn side)
{
    /* return( *tbbt_anbr(ptr,side) ); */

    if (!HasChild(ptr, side))
        return (ptr->link[side]);
/*        return(NULL); */
    ptr = ptr->link[side];
    if(ptr==NULL)
        return(NULL);
    while (HasChild(ptr, Other(side)))
        ptr = ptr->link[Other(side)];
    return (ptr);
}

/* Returns pointer to node with previous key value: */
TBBT_NODE  *
tbbtnext(TBBT_NODE * node)
{
    return (tbbt_nbr(node, RIGHT));
}

/* Returns pointer to node with next key value: */
TBBT_NODE  *
tbbtprev(TBBT_NODE * node)
{
    return (tbbt_nbr(node, LEFT));
}

/* tbbtfind -- Look up a node in a tree based on a key value */
/* tbbtffind is based on this routine - fix bugs in both places! */
/* Returns a pointer to the found node (or NULL) */
TBBT_NODE  *
tbbtfind(TBBT_NODE * root, VOIDP key,
     intn (*compar) (VOIDP, VOIDP, intn), intn arg, TBBT_NODE ** pp)
{
    TBBT_NODE  *ptr = root;
    TBBT_NODE  *parent = NULL;
    intn        cmp = 1;

    if (ptr)
      {
          intn        side;

          while (0 != (cmp = KEYcmp(key, ptr->key, arg)))
            {
                parent = ptr;
                side = (cmp < 0) ? LEFT : RIGHT;
                if (!HasChild(ptr, side))
                    break;
                ptr = ptr->link[side];
            }
      }
    if (NULL != pp)
        *pp = parent;
    return ((0 == cmp) ? ptr : NULL);
}

/* tbbtffind -- Look up a node in a tree based on a key value */
/* This routine is based on tbbtfind (fix bugs in both places!) */
/* Returns a pointer to the found node (or NULL) */
static TBBT_NODE  *
tbbtffind(TBBT_NODE * root, VOIDP key, uintn fast_compare, TBBT_NODE ** pp)
{
    TBBT_NODE  *ptr = root;
    TBBT_NODE  *parent = NULL;
    intn        cmp = 1;

    switch(fast_compare)
      {
        case TBBT_FAST_UINT16_COMPARE:
            if (ptr)
              {
                  intn        side;

                  while (0 != (cmp = (*(uint16 *)key - *(uint16 *)ptr->key)))
                    {
                        parent = ptr;
                        side = (cmp < 0) ? LEFT : RIGHT;
                        if (!HasChild(ptr, side))
                            break;
                        ptr = ptr->link[side];
                    }
              }
            if (NULL != pp)
                *pp = parent;
            return ((0 == cmp) ? ptr : NULL);

        case TBBT_FAST_INT32_COMPARE:
            if (ptr)
              {
                  intn        side;

                  while (0 != (cmp = (*(int32 *)key - *(int32 *)ptr->key)))
                    {
                        parent = ptr;
                        side = (cmp < 0) ? LEFT : RIGHT;
                        if (!HasChild(ptr, side))
                            break;
                        ptr = ptr->link[side];
                    }
              }
            if (NULL != pp)
                *pp = parent;
            return ((0 == cmp) ? ptr : NULL);

        default:
            return(NULL);
    } /* end switch */
} /* tbbtffind() */

/* tbbtdfind -- Look up a node in a "described" tree based on a key value */
/* Returns a pointer to the found node (or NULL) */
TBBT_NODE  *
tbbtdfind(TBBT_TREE * tree, VOIDP key, TBBT_NODE ** pp)
{
    if (tree == NULL)
        return (NULL);
    if(tree->fast_compare!=0)
        return (tbbtffind(tree->root, key, tree->fast_compare, pp));
    else
        return (tbbtfind(tree->root, key, tree->compar, tree->cmparg, pp));
}

/* tbbtless -- Find a node in a tree which is less than a key, */
/*  based on a key value */
/* Returns a pointer to the found node (or NULL) */
TBBT_NODE  *
tbbtless(TBBT_NODE * root, VOIDP key,
     intn (*compar) (VOIDP, VOIDP, intn), intn arg, TBBT_NODE ** pp)
{
    TBBT_NODE  *ptr = root;
    TBBT_NODE  *parent = NULL;
    intn        cmp = 1;

    if (ptr)
      {
          intn        side;

          while (0 != (cmp = KEYcmp(key, ptr->key, arg)))
            {
                parent = ptr;
                side = (cmp < 0) ? LEFT : RIGHT;
                if (!HasChild(ptr, side))
                    break;
                ptr = ptr->link[side];
            }
      }
    if(cmp!=0)
	/* didn't find an exact match, search back up the tree until a node */
	/* is found with a key less than the key searched for */
      {
	while((ptr=ptr->Parent)!=NULL) 
	  {
              cmp = KEYcmp(key, ptr->key, arg);
	      if(cmp<0) /* found a node which is less than the search for one */
		  break;
	  } /* end while */
	if(ptr==NULL) /* didn't find a node in the tree which was less */
	  cmp=1;
	else /* reset this for cmp test below */
	  cmp=0;
      } /* end if */
    if (NULL != pp)
        *pp = parent;
    return ((0 == cmp) ? ptr : NULL);
}

/* tbbtdless -- Find a node less than a key in a "described" tree */
/* Returns a pointer to the found node (or NULL) */
TBBT_NODE  *
tbbtdless(TBBT_TREE * tree, VOIDP key, TBBT_NODE ** pp)
{
    if (tree == NULL)
        return (NULL);
    return (tbbtless(tree->root, key, tree->compar, tree->cmparg, pp));
}

/* tbbtindx -- Look up the Nth node (in key order) */
/* Returns a pointer to the `indx'th node (or NULL) */
/* Bugs(fixed):
   Added NULL check for 'ptr' in while loop to 
     prevent endless loop condition. 
   Fixed bug where we subtracted children count from the wrong side of the
    tree. */
TBBT_NODE  *
tbbtindx(TBBT_NODE * root, int32 indx)
{
  TBBT_NODE  *ptr = root;

  /* I believe indx is 1 based */
  if (NULL == ptr || indx < 1)
    return (NULL);
  /* Termination condition is if the index equals the number of children on
     out left plus the current node */
  while (ptr != NULL && indx != ((int32) LeftCnt(ptr)) + 1 )
    {
      if (indx <= (int32) LeftCnt(ptr))
        {
#if 0
          indx -= LeftCnt(ptr);  /* ??....bug */
#endif
          ptr = ptr->Lchild;
        }
      else if (HasChild(ptr, RIGHT))
        { /* subtract children count from leftchild plus current node when
             we descend into a right branch */
          indx -= (int32)(LeftCnt(ptr) + 1);  
          ptr = ptr->Rchild;
        }
      else
        return (NULL);    /* Only `indx' or fewer nodes in tree */
    }
  return (ptr);
}

/* swapkid -- Often refered to as "rotating" nodes.  ptr and ptr's `side'
 * child, kid, are swapped so ptr becomes kid's `Other(side)' child.
 * Here is how a single swap (rotate) works:
 *
 *           |           --side-->         |
 *         (ptr)                         (kid)
 *        /     \                       /     \
 *    +-A-+    (kid)                 (ptr)    +-C-+
 *    |   |   /     \               /     \   |   |
 *    |...| +-B-+  +-C-+         +-A-+  +-B-+ |...|
 *          |   |  |   |         |   |  |   |
 *          |...|  |...|         |...|  |...|
 * `deep' contains the relative depths of the subtrees so, since we set
 * `deep[1]' (the relative depth of subtree [B]) to 0, `deep[2]' is the depth
 * of [C] minus the depth of [B] (-1, 0, or 1 since `kid' is never too out of
 * balance) and `deep[0]' is the depth of [A] minus the depth of [B].  These
 * values are used to compute the balance levels after the rotation.  Note that
 * [A], [B], or [C] can have depth 0 so `link[]' contains threads rather than
 * pointers to children.
 */
static TBBT_NODE *
swapkid(TBBT_NODE ** root, TBBT_NODE * ptr, intn side)
{
    TBBT_NODE  *kid = ptr->link[side];  /* Sibling to be swapped with parent */
    intn        deep[3];        /* Relative depths of three sub-trees involved. */
    /* 0:ptr->link[Other(side)], 1:kid->link[Other(side)], 2:kid->link[side] */
    TBBT_FLAG   ptrflg;         /* New value for ptr->flags (ptr->flags used after set) */
    TBBT_LEAF   plcnt, prcnt,   /* current values of the ptr's and kid's leaf count */
                klcnt, krcnt;

    deep[2] = (deep[1] = 0) + Delta(kid, side);
    deep[0] = Max(0, deep[2]) + 1 - Delta(ptr, side);
    kid->Parent = ptr->Parent;
    ptrflg = (TBBT_FLAG)SetFlags(ptr, side, deep[0],
                  HasChild(ptr, Other(side)) && HasChild(kid, Other(side)));
    plcnt = LeftCnt(ptr);
    prcnt = RightCnt(ptr);
    klcnt = LeftCnt(kid);
    krcnt = RightCnt(kid);
    if (HasChild(kid, Other(side)))
      {
          ptr->link[side] = kid->link[Other(side)];     /* Real child */
          ptr->link[side]->Parent = ptr;
      }
    else
      {
          ptr->link[side] = kid;    /* Thread */
      }
    /* Update grand parent's pointer: */
    if (NULL == ptr->Parent)
      {
          *root = kid;
      }
    else if (ptr /*->Lchild*/  == ptr->Parent->Lchild)
      {
          ptr->Parent->Lchild = kid;
      }
    else
      {
          ptr->Parent->Rchild = kid;
      }
    ptr->Parent = kid;
    kid->link[Other(side)] = ptr;
#if defined (MAC) || defined (macintosh) || defined (SYMANTEC_C) /* Macro substitution limit on Mac */
    kid->flags = SetFlags(kid, (1 + 2 - (side)),
                        deep[2] - 1 - Max(deep[0], 0), HasChild(kid, side));
#else  /* !macintosh */
    kid->flags = (TBBT_FLAG)SetFlags(kid, Other(side),
                        deep[2] - 1 - Max(deep[0], 0), HasChild(kid, side));
#endif /* !macintosh */

    /* update leaf counts */
    if (side == LEFT)
      {     /* kid's left count doesn't change, nor ptr's r-count */
          kid->rcnt = prcnt + krcnt + 1;    /* kid's leafs+former parent's leafs+parent */
          ptr->lcnt = krcnt;
      }     /* end if */
    else
      {     /* kid's right count doesn't change, nor ptr's l-count */
          kid->lcnt = plcnt + klcnt + 1;    /* kid's leafs+former parent's leafs+parent */
          ptr->rcnt = klcnt;
      }     /* end if */
    ptr->flags = ptrflg;
    return (kid);
}

/* balance -- Move up tree, incrimenting number of left children when needed
 * and looking for unbalanced ancestors.  Adjust all balance factors and re-
 * balance through "rotation"s when needed.
 */
static      VOID
balance(TBBT_NODE ** root, TBBT_NODE * ptr, intn side, intn added)
{
    intn        deeper = added; /* 1 if sub-tree got longer; -1 if got shorter */
    intn        odelta;
    intn        obal;

    while (NULL != ptr)
      {
          odelta = Delta(ptr, side);    /* delta before the node was added */
          obal = UnBal(ptr);
          if (LEFT == side)     /* One more/fewer left child: */
              if (0 < added)
                  ptr->lcnt++;  /* LeftCnt(ptr)++ */
              else
                  ptr->lcnt--;  /* LeftCnt(ptr)-- */
          else if (0 < added)
              ptr->rcnt++;  /* RightCnt(ptr)++ */
          else
              ptr->rcnt--;  /* RightCnt(ptr)-- */
          if (0 != deeper)
            {   /* One leg got longer or shorter: */
                if ((deeper < 0 && odelta < 0) || (deeper > 0 && odelta > 0))
                  {     /* Became too unbalanced: */
                      TBBT_NODE  *kid;

                      ptr->flags |= TBBT_DOUBLE;    /* Mark node too unbalanced */
                      if (deeper < 0)   /* Just removed a node: */
                          side = Other(side);   /* Swap with child from other side. */
                      else
                          /* Just inserted a node: */ if (ptr->Parent && UnBal(ptr->Parent))
                        {
                            deeper = 0;     /* Fix will re-shorten sub-tree. */
                        }
                      kid = ptr->link[side];
                      if (Heavy(kid, Other(side)))
                        {   /* Double rotate needed: */
                            kid = swapkid(root, kid, Other(side));
                            ptr = swapkid(root, ptr, side);
                        }
                      else
                        {   /* Just rotate parent and kid: */
                            if (HasChild(kid, side))    /* In this case, sub-tree gets */
                                if (ptr->Parent && UnBal(ptr->Parent))
                                  {
                                      deeper = 0;   /* re-lengthened after a node removed. */
                                  }
                            ptr = swapkid(root, ptr, side);
                        }
                  }
                else if (obal)
                  {     /* Just became balanced: */
                      ptr->flags &= ~TBBT_UNBAL;
                      if (0 < deeper)
                        {   /* Shorter of legs lengthened */
                            ptr->flags |= TBBT_INTERN;  /* Mark as internal node now */
                            deeper = 0;     /* so max length unchanged */
                        }   /* end if */
                  }
                else if (deeper < 0)
                  {     /* Just became unbalanced: */
                      if (ptr->link[Other(side)] != NULL && ptr->link[Other(side)]->Parent == ptr)
                        {
                            ptr->flags |= (TBBT_FLAG)TBBT_HEAVY(Other(side));  /* Other side longer */
                            if (ptr->Parent)
                                if (ptr->Parent->Rchild == ptr) {     /* we're the right child */
                                    if (Heavy(ptr->Parent, RIGHT) && LeftCnt(ptr->Parent) == 1)
                                        deeper = 0;
                                    else
                                        /* we're the left child */ if (Heavy(ptr->Parent, LEFT))
                                        if (ptr->Parent->Rchild && !UnBal(ptr->Parent->Rchild))
                                            deeper = 0;
                                }
                        }
                  }
                else
                  {     /* Just became unbalanced: */
                      ptr->flags |= (TBBT_FLAG)TBBT_HEAVY(side);   /* 0<deeper: Our side longer */
                  }     /* end else */
            }
          if (ptr->Parent)
            {
                if (ptr == (ptr->Parent->Rchild))
                    side = RIGHT;
                else
                    side = LEFT;
            }   /* end if */
          ptr = ptr->Parent;    /* Move up the tree */
      }
    /* total tree depth += deeper; */
}
/* Here is how rotatation rebalances a tree:
 * Either the deletion of a node shortened the sub-tree [A] (to length `h')
 * while [B] or [C] or both are length `h+1'  or  the addition of a node
 * lengthened [B] or [C] to length `h+1' while the other and [A] are both
 * length `h'.  Each case changes `ptr' from being "right heavy" to being
 * overly unbalanced.
 * This           |                      Becomes:      |
 * sub-tree:    (ptr)                                (kid)
 *             /     \          --side-->           /     \
 *         +-A-+    (kid)                        (ptr)   +-C-+
 *         |   |   /     \                      /     \  |   |
 *         | h | +-B-+  +-C-+               +-A-+  +-B-+ | h |
 *         |   | |   |  |   |               |   |  |   | |   |
 *         +---+ | h |  | h |               | h |  | h | +---+
 *         : - : |   |  |   |               |   |  |   | : 1 :
 *         `- -' +---+  +---+               +---+  +---+ + - +
 *               : 1 :  : 1 :                      : 1 :
 *               + - +  + - +                      + - +
 *
 * However, if [B] is long (h+1) while [C] is short (h), a double rotate is
 * required to rebalance.  In this case, [A] was shortened or [X] or [Y] was
 * lengthened so [A] is length `h' and one of [X] and [Y] is length `h' while
 * the other is length `h-1'.  Swap `kid' with `babe' then `ptr' with `babe'.
 * This          |                         Becomes:     |
 * sub-tree:   (ptr)                                  (babe)
 *            /     \             --side-->          /      \
 *       +-A-+       (kid)                      (ptr)       (kid)
 *       |   |      /     \                    /     \     /     \
 *       | h |    (babe)   +-C-+             +-A-+ +-X-+ +-Y-+ +-C-+
 *       |   |   /      \  |   |             |   | |h-1| |h-1| |   |
 *       +---+ +-X-+ +-Y-+ | h |             | h | +---+ +---+ | h |
 *       : - : |h-1| |h-1| |   |             |   | : 1 : : 1 : |   |
 *       `- -' +---+ +---+ +---+             +---+ + - + + - + +---+
 *             : 1 : : 1 :
 *             + - + + - +
 *
 * Note that in the node insertion cases total sub-tree length always increases
 * by one then decreases again so after the rotation(s) no more rebalancing is
 * required.  In the node removal cases, the single rotation reduces total sub-
 * tree length unless [B] is length `h+1' (`ptr' ends of "right heavy") while
 * the double rotation ALWAYS reduces total sub-tree length.  Thus removing a
 * single node can require log(N) rotations for rebalancing.  On average, only
 * are usually required.
 */

/* Returns pointer to inserted node (or NULL) */
TBBT_NODE  *
tbbtins(TBBT_NODE ** root, VOIDP item, VOIDP key, intn (*compar)
        (VOIDP /* k1 */, VOIDP /* k2 */, intn /* arg */), intn arg)
{
    intn        cmp;
    TBBT_NODE  *ptr, *parent;

    if (NULL != tbbtfind(*root, (key ? key : item), compar, arg, &parent)
        || NULL == (ptr = tbbt_get_node()))
        return (NULL);
    ptr->data = item;
    ptr->key = key ? key : item;
    ptr->Parent = parent;
    ptr->flags = 0L;    /* No children on either side */
    ptr->lcnt = 0;
    ptr->rcnt = 0;
    if (NULL == parent)
      {     /* Adding first node to tree: */
          *root = ptr;
          ptr->Lchild = ptr->Rchild = NULL;
          return (ptr);
      }
    cmp = KEYcmp(ptr->key, parent->key, arg);
    if (cmp < 0)
      {
          ptr->Lchild = parent->Lchild;     /* Parent's thread now new node's */
          ptr->Rchild = parent;     /* New nodes right thread is parent */
          parent->Lchild = ptr;     /* Parent now has a left child */
      }
    else
      {
          ptr->Rchild = parent->Rchild;
          ptr->Lchild = parent;
          parent->Rchild = ptr;
      }
    balance(root, parent, (cmp < 0) ? LEFT : RIGHT, 1);
    return (ptr);
}

/* tbbtdins -- Insert a node into a "described" tree */
         /* Returns a pointer to the inserted node */
TBBT_NODE  *
tbbtdins(TBBT_TREE * tree, VOIDP item, VOIDP key)
{
    TBBT_NODE  *ret_node;       /* the node to return */

    if (tree == NULL)
        return (NULL);
    ret_node = tbbtins(&(tree->root), item, key, tree->compar, tree->cmparg);
    if (ret_node != NULL)
        tree->count++;
    return (ret_node);
}

/* tbbtrem -- Remove a node from a tree.  You pass in the address of the
 * pointer to the root node of the tree along, a pointer to the node you wish
 * to remove, and optionally the address of a pointer to hold the address of
 * the key value of the deleted node.  The second argument is usually the
 * result from a lookup function call (tbbtfind, tbbtdfind, or tbbtindx) so if
 * it is NULL, tbbtrem returns NULL.  Otherwise tbbtrem removes the node from
 * the tree and returns a pointer to the data item for that node and, if the
 * third argument is not NULL, the address of the key value for the deleted
 * node is placed in the buffer that it points to.
 */
          /* Data item pointer of deleted node is returned */
VOIDP
tbbtrem(TBBT_NODE ** root, TBBT_NODE * node, VOIDP *kp)
{
    TBBT_NODE  *leaf;           /* Node with one or zero children */
    TBBT_NODE  *par;            /* Parent of `leaf' */
    TBBT_NODE  *next;           /* Next/prev node near `leaf' (`leaf's `side' thread) */
    intn        side;           /* `leaf' is `side' child of `par' */
    VOIDP       data;           /* Saved pointer to data item of deleted node */

    if (NULL == root || NULL == node)
        return (NULL);  /* Argument couldn't find node to delete */
    data = node->data;  /* Save pointer to data item to be returned at end */
    if (NULL != kp)
        *kp = node->key;
    /* If the node to be removed is "internal" (children on both sides), we
     * replace it with it's previous (or next) node in the tree and delete that
     * previous (next) node (which has one or no children) instead. */
    if (Intern(node))
      {     /* Replace with a non-internal node: */
          if (Heavy(node, RIGHT))
            {   /* Pick "near-leaf" node from the */
                side = LEFT;    /* heavier of the sub-trees. */
            }
          else if (Heavy(node, LEFT))
            {
                side = RIGHT;
            }
          else
            {   /* If no sub-tree heavier, pick at "random" for "better */
                side = (0x10 & *(short *) &node) ? LEFT : RIGHT;    /* balance" */
            }
          leaf = tbbt_nbr(next = node, Other(side));
          par = leaf->Parent;
          if (par == next)
            {   /* Case 2x: `node' had exactly 2 descendants */
                side = Other(side);     /* Transform this to Case 2 */
                next = leaf->link[side];
            }
          node->data = leaf->data;
          node->key = leaf->key;
      }
    else
      {     /* Node has one or zero children: */
          leaf = node;  /* Simply remove THIS node */
          par = leaf->Parent;
          if (NULL == par)
            {   /* Case 3: Remove root (of 1- or 2-node tree) */
                side = (intn) UnBal(node);  /* Which side root has a child on */
                if (side)
                  {     /* Case 3a: Remove root of 2-node tree: */
                      *root = leaf = node->link[side];
                      leaf->Parent = leaf->link[Other(side)] = NULL;
                      leaf->flags = 0;  /* No left children, balanced, not internal */
                  }
                else
                  {     /* Case 3b: Remove last node of tree: */
                      *root = NULL;
                  }     /* end else */
                tbbt_release_node(node);
                return (data);
            }
          side = (par->Rchild == leaf) ? RIGHT : LEFT;
          next = leaf->link[side];
      }
    /* Now the deletion has been reduced to the following cases (and Case 3 has
     * been handled completely above and Case 2x has been transformed into
     * Case 2).  `leaf' is a node with one or zero children that we are going
     * to remove.  `next' points where the `side' thread of `leaf' points.
     * `par' is the parent of `leaf'.  The only posibilities (not counting
     * left/right reversals) are shown below:
     *       [Case 1]                  [Case 2]              [Case 2x]
     *            (next)                 (next)         ^         (next & par)
     *           /  ^   \               /  ^   \        |        /  ^         \
     *     . . .    |             . . .    |            |  (leaf)   /
     *   /          |           /          |            \_/      \_/
     * (par)        |         (par)        |             ^threads^
     *      \       |              \       |
     *     (leaf)   /             (leaf)   /            [Case 3a]    [Case 3b]
     *    /  ^   \_/<thread             \_/<thread       (root)
     * (n)   /                                                 \       (root)
     *    \_/<thread        --"side"-->                         (n)
     * Note that in Cases 1 and 2, `leaf's `side' thread can be NULL making
     * `next' NULL as well.  If you remove a node from a 2-node tree, removing
     * the root falls into Case 3a while removing the only leaf falls into
     * Case 2 (with `next' NULL and `par' the root node). */
    if (!UnBal(leaf))
      {     /* Case 2: `leaf' has no children: */
          par->link[side] = leaf->link[side];
          par->flags &= (TBBT_FLAG)(~(TBBT_INTERN | TBBT_HEAVY(side)));
      }
    else
      {     /* Case 1: `leaf' has one child: */
          TBBT_NODE  *n;

          if (HasChild(leaf, side))
            {   /* two-in-a-row cases */
                n = leaf->link[side];
                par->link[side] = n;
                n->Parent = par;
                if (HasChild(n, Other(side)))
                    while (HasChild(n, Other(side)))
                        n = n->link[Other(side)];
                n->link[Other(side)] = par;
            }   /* end if */
          else
            {   /* zig-zag cases */
                n = leaf->link[Other(side)];
                par->link[side] = n;
                n->Parent = par;
                if (HasChild(n, side))
                    while (HasChild(n, side))
                        n = n->link[side];
                n->link[side] = next;
            }   /* end else */
      }
    tbbt_release_node(leaf);
    balance(root, par, side, -1);
    ((TBBT_TREE *) root)->count--;
    return (data);
}

/* tbbtdmake - Allocate a new tree description record for an empty tree */
/* Returns a pointer to the description record */
TBBT_TREE  *
tbbtdmake(intn (*cmp) (VOIDP /* k1 */, VOIDP /* k2 */, intn /* arg */), intn arg, uintn fast_compare)
{
    TBBT_TREE  *tree = Alloc(1, TBBT_TREE);

    if (NULL == tree)
        return (NULL);
    tree->root = NULL;
    tree->count = 0;
    tree->fast_compare=fast_compare;
    tree->compar = cmp;
    tree->cmparg = arg;
    return (tree);
}

#ifdef WASTE_STACK
/* You can have a very simple recursive version that wastes lots of stack
 * space, this next less-simple recursive version that wastes less stack space,
 * or the last non-recursive version which isn't simple but saves stack space.
 */
static      VOID(*FD) (VOIDP item), (*FK) (VOIDP key);
static      VOID
tbbt1free(TBBT_NODE * node)
{
    if (HasChild(node, LEFT))
        tbbt1free(node->Lchild);
    if (HasChild(node, RIGHT))
        tbbt1free(node->Rchild);
    if (NULL != FD)
        (*FD) (node->data);
    if (NULL != FK)
        (*FK) (node->key);
    tbbt_release_node(node);
}

VOID
tbbtfree(TBBT_NODE ** root, VOID(*fd) (VOIDP item), VOID(*fk) (VOIDP key))
{
    if (NULL == *root)
        return;
    FD = fd;
    FK = fk;
    tbbt1free(*root);
    *root = NULL;
}
#else  /* WASTE_STACK */

/* tbbtfree() - Free an entire tree not allocated with tbbtdmake(). */
VOID
tbbtfree(TBBT_NODE ** root, VOID(*fd) (VOIDP /* item */), VOID(*fk) (VOIDP /* key */))
{
    TBBT_NODE  *par, *node = *root;

    while (NULL != *root)
      {     /* While nodes left to be free()d */
          /* First time at this node (just moved down a new leg of tree) */
          if (!HasChild(node, LEFT))
              node->Lchild = NULL;
          if (!HasChild(node, RIGHT))
              node->Rchild = NULL;
          do
            {
                par = NULL;     /* Assume we aren't ready to move up tree yet */
                if (NULL != node->Lchild)
                    node = node->Lchild;    /* Move down this leg next */
                else if (NULL != node->Rchild)
                    node = node->Rchild;    /* Move down this leg next */
                else
                  {     /* No children; free node an move up: */
                      par = node->Parent;   /* Move up tree (stay in loop) */
                      if (NULL != fd)
                          (*fd) (node->data);
                      if (NULL != fk)
                          (*fk) (node->key);
                      if (NULL == par)  /* Just free()d last node */
                          *root = NULL;     /* NULL=par & NULL=*root gets fully out */
                      else if (node == par->Lchild)
                          par->Lchild = NULL;   /* Now no longer has this child */
                      else
                          par->Rchild = NULL;   /* Ditto */

                      tbbt_release_node(node);

                      node = par;   /* Move up tree; remember which node to do next */
                  }
            }
          while (NULL != par);  /* While moving back up tree */
      }
}
#endif /* WASTE_STACK */

VOID
tbbtprint(TBBT_NODE * node)
{
    if (node == NULL)
        return;
    printf("node=%p, key=%p, data=%p, flags=%x\n", node, node->key, node->data, (unsigned) node->flags);
    printf("Lcnt=%d, Rcnt=%d\n", (int) node->lcnt, (int) node->rcnt);
    printf("*key=%d\n", (int) *(int32 *) (node->key));
    printf("Lchild=%p, Rchild=%p, Parent=%p\n", node->Lchild, node->Rchild, node->Parent);
}   /* end tbbtprint() */

VOID
tbbt1dump(TBBT_NODE * node, intn method)
{
    if (node == NULL)
        return;
    switch (method)
      {
          case -1:      /* Pre-Order Traversal */
              tbbtprint(node);
              if (HasChild(node, LEFT))
                  tbbt1dump(node->Lchild, method);
              if (HasChild(node, RIGHT))
                  tbbt1dump(node->Rchild, method);
              break;

          case 1:   /* Post-Order Traversal */
              if (HasChild(node, LEFT))
                  tbbt1dump(node->Lchild, method);
              if (HasChild(node, RIGHT))
                  tbbt1dump(node->Rchild, method);
              tbbtprint(node);
              break;

          case 0:   /* In-Order Traversal */
          default:
              if (HasChild(node, LEFT))
                  tbbt1dump(node->Lchild, method);
              tbbtprint(node);
              if (HasChild(node, RIGHT))
                  tbbt1dump(node->Rchild, method);
              break;

      }     /* end switch() */
}   /* end tbbt1dump() */

VOID
tbbtdump(TBBT_TREE * tree, intn method)
{
    if (tree != NULL && *(TBBT_NODE **) tree != NULL)
      {
          printf("Number of nodes in the tree: %ld\n", tree->count);
          tbbt1dump(tree->root, method);
      }     /* end if */
    else
        printf("Tree is empty\n");
}   /* end tbbtdump() */

VOID
tbbt_printNode(TBBT_NODE * node, VOID(*key_dump)(VOID *,VOID *))
{

    if (node == NULL)
      {
        printf("ERROR:  null node pointer\n");
        return;
      }
    printf("node=%p, flags=%x, Lcnt=%ld, Rcnt=%ld\n", node, (unsigned)node->flags,
           (long)node->lcnt, (long)node->rcnt);
    printf("Lchild=%p, Rchild=%p, Parent=%p\n", node->Lchild, node->Rchild, node->Parent);
    if (key_dump != NULL)
      {
        (*key_dump)(node->key,node->data);
      }
    fflush(stdout);
#if 0
    printf("Lcnt=%d, Rcnt=%d\n", (int) node->lcnt, (int) node->rcnt);
    printf("*key=%d\n", (int) *(int32 *) (node->key));
    printf("Lchild=%p, Rchild=%p, Parent=%p\n", node->Lchild, node->Rchild, node->Parent);
#endif
}   /* end tbbt_printNode() */

VOID
tbbt_dumpNode(TBBT_NODE *node, VOID (*key_dump)(VOID *,VOID *),
                        intn method)
{
    if (node == NULL)
        return;
    switch (method)
      {
          case -1:      /* Pre-Order Traversal */
              tbbt_printNode(node, key_dump);
              if (HasChild(node, LEFT))
                  tbbt_dumpNode(node->Lchild, key_dump, method);
              if (HasChild(node, RIGHT))
                  tbbt_dumpNode(node->Rchild, key_dump, method);
              break;

          case 1:   /* Post-Order Traversal */
              if (HasChild(node, LEFT))
                  tbbt_dumpNode(node->Lchild, key_dump, method);
              if (HasChild(node, RIGHT))
                  tbbt_dumpNode(node->Rchild, key_dump, method);
              tbbt_printNode(node, key_dump);
              break;

          case 0:   /* In-Order Traversal */
          default:
              if (HasChild(node, LEFT))
                  tbbt_dumpNode(node->Lchild, key_dump, method);
              tbbt_printNode(node, key_dump);
              if (HasChild(node, RIGHT))
                  tbbt_dumpNode(node->Rchild, key_dump, method);
              break;

      }     /* end switch() */
}   /* end tbbt_dumpNode() */

VOID
tbbt_dump(TBBT_TREE *ptree, VOID (*key_dump)(VOID *,VOID *), intn method)
{
	TBBT_TREE  *tree;

	tree = (TBBT_TREE *) ptree;
	printf("TBBT-tree dump  %p:\n\n",ptree);
	printf("capacity = %ld\n",(long)tree->count);
	printf("\n");
	tbbt_dumpNode(tree->root,key_dump, method);
	return;
}

/* Always returns NULL */
TBBT_TREE  *
tbbtdfree(TBBT_TREE * tree, VOID(*fd) (VOIDP /* item */), VOID(*fk) (VOIDP /* key */))
{
    if (tree == NULL)
        return (NULL);

    tbbtfree(&tree->root, fd, fk);
    Free(tree);
    return (NULL);
}

/* returns the number of nodes in the tree */
long
tbbtcount(TBBT_TREE * tree)
{
    if (tree == NULL)
        return (-1);
    else
        return ((long)tree->count);
}

/******************************************************************************
 NAME
     tbbt_get_node - Gets a tbbt node

 DESCRIPTION
    Either gets a tbbt node from the free list (if there is one available)
    or allocates a node.

 RETURNS
    Returns tbbt ptr if successful and NULL otherwise

*******************************************************************************/
static TBBT_NODE *tbbt_get_node(void)
{
    TBBT_NODE *ret_value=NULL;

    if(tbbt_free_list!=NULL)
      {
        ret_value=tbbt_free_list;
        tbbt_free_list=tbbt_free_list->Lchild;
      } /* end if */
    else
        ret_value=(TBBT_NODE *)Alloc(1,TBBT_NODE);

  return ret_value;
}   /* end tbbt_get_node() */

/******************************************************************************
 NAME
     tbbt_release_node - Releases a tbbt node

 DESCRIPTION
    Puts a tbbt node into the free list

 RETURNS
    No return value

*******************************************************************************/
static void tbbt_release_node(TBBT_NODE *nod)
{
    /* Insert the atom at the beginning of the free list */
    nod->Lchild=tbbt_free_list;
    tbbt_free_list=nod;
}   /* end tbbt_release_node() */

/*--------------------------------------------------------------------------
 NAME
    tbbt_shutdown
 PURPOSE
    Terminate various static buffers.
 USAGE
    intn tbbt_shutdown()
 RETURNS
    Returns SUCCEED/FAIL
 DESCRIPTION
    Free various buffers allocated in the tbbt routines.
 GLOBAL VARIABLES
 COMMENTS, BUGS, ASSUMPTIONS
    Should only ever be called by the "atexit" function HDFend
 EXAMPLES
 REVISION LOG
--------------------------------------------------------------------------*/
intn 
tbbt_shutdown(void)
{
    TBBT_NODE *curr;

    /* Release the free-list if it exists */
    if(tbbt_free_list!=NULL)
      {
        while(tbbt_free_list!=NULL)
          {
            curr=tbbt_free_list;
            tbbt_free_list=tbbt_free_list->Lchild;
            Free(curr);
          } /* end while */
      } /* end if */
  return (SUCCEED);
}	/* end tbbt_shutdown() */

