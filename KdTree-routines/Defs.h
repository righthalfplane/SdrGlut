/*
 * D E F S . H
 *
 */
 
#ifndef __DEFS_H__
#define __DEFS_H__

#include "firstFile.h"

#include <stdio.h>
 
#include "Tools.h"

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
	
	

#define M_AMBIENT		1
#define M_DIFFUSE		2
#define M_MIRROR		4
#define M_TRANSMIT		8
#define M_INDEX			16
#define M_SPECULAR		32
#define M_COLORLEVEL	64
#define M_PATTERN		128
#define M_BUMP			256
#define M_TRANSPARENT	512
#define M_EMISSIVE	   1024


	
	
#define		T_TRINANGLE		(12)
#define		T_TRIN			(14)
#define		T_SPHERE		(15)
#define		T_TRIC1N1		(16)
#define		T_TRI		    (17)

 
	
#define		NSLABS		(3)
#define		BUNCHINGFACTOR	(10)
#define		PQSIZE		(10000)
 
#define Point Point2
 
#ifndef HUGE
#define HUGE 1e60
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif
 
typedef double Flt ;
typedef Flt Vec[3] ;
	
typedef float Fltf ;
typedef Fltf Vecf[3] ;

typedef Vecf Colorf ;
typedef Vecf Pointf ;
	
typedef Vec Colorv ;
typedef Vec Point ;
typedef Vec ColorVEC ;
 
#define MakeVector(x, y, z, v)		(v)[0]=(x),(v)[1]=(y),(v)[2]=(z)
#define VecNegate(a)	(a)[0]=0-(a)[0];\
			(a)[1]=0-(a)[1];\
			(a)[2]=0-(a)[2];
#define VecDot(a,b)	((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecLen(a)	(sqrt(VecDot(a,a)))
#define VecCopy(a,b)	 (b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];
#define VecCopy2(a,b)	 (b)[0]=(float)((a)[0]);(b)[1]=(float)((a)[1]);(b)[2]=(float)((a)[2]);
#define VecAdd(a,b,c)	 (c)[0]=(a)[0]+(b)[0];\
			 (c)[1]=(a)[1]+(b)[1];\
			 (c)[2]=(a)[2]+(b)[2]
#define VecSub(a,b,c)	 (c)[0]=(a)[0]-(b)[0];\
			 (c)[1]=(a)[1]-(b)[1];\
			 (c)[2]=(a)[2]-(b)[2]
#define VecComb(A,a,B,b,c)	(c)[0]=(A)*(a)[0]+(B)*(b)[0];\
				(c)[1]=(A)*(a)[1]+(B)*(b)[1];\
			 	(c)[2]=(A)*(a)[2]+(B)*(b)[2]
#define VecAddS(A,a,b,c)	 (c)[0]=(A)*(a)[0]+(b)[0];\
				 (c)[1]=(A)*(a)[1]+(b)[1];\
				 (c)[2]=(A)*(a)[2]+(b)[2]
#define VecCross(a,b,c)	 (c)[0]=(a)[1]*(b)[2]-(a)[2]*(b)[1];\
			 (c)[1]=(a)[2]*(b)[0]-(a)[0]*(b)[2];\
			 (c)[2]=(a)[0]*(b)[1]-(a)[1]*(b)[0]
 
#define ColorToVec(a,b)	 (b)[0]=(a.red);(b)[1]=(a.green);(b)[2]=(a.blue);

#define PToVec(a,b)	 (b)[0]=(a.x);(b)[1]=(a.y);(b)[2]=(a.z);


 
/* #define max(a,b) 	((a)>(b)?(a):(b)) */
/* #define min(a,b) 	((a)<(b)?(a):(b)) */
 
/*----------------------------------------------------------------------*/
 
typedef struct Material Surface ;
 
typedef struct t_light {
	Vec	light_pos ;
	Flt	light_brightness ;
	Flt	light_bright;
	Flt	red ;
	Flt	green ;
	Flt	blue ;
	Flt vmin;
	Flt vmax;
	Flt vdiff;
	long type;

} Light ;
 
extern struct t_object *dummy_t_object;

extern struct Scene *sceneDEFs;
	
extern struct t_isect *hit_ddd;
	
extern struct Ray *ray_ddd;

struct t_objectprocs {
	int 	(*print) (struct Scene *scene, struct t_object *,long ) ;
	int 	(*intersect) (struct t_object *obj, struct Ray *ray, struct t_isect *hit) ;
	int 	(*normal) (struct t_object *obj, struct t_isect *hit, Point P, Point N) ;
	int 	(*shader) (struct t_object *obj, int level, Flt weight, Vec P, Vec N, Vec I, struct t_isect *hit, Vec col, struct Ray *ray) ;
	int 	(*drawGlut) (struct Scene *scene, struct t_object *obj) ;
	int 	(*getValue) (struct t_object *obj, struct Ray *ray, double *value, Vec normal) ;
	int 	(*setColor) (struct Scene *scene,struct t_object *) ;
	int 	(*getData) (struct t_object *obj, Vec normal, Vec centroid, double *area, double *temperature) ;
	int 	(*setData) (struct t_object *, double) ;
	int 	(*getInformation) (struct t_object *, Vec points[3],Vec value) ;
};
	
typedef struct t_object {
	unsigned short 	o_type ;
	Flt	o_dmin[NSLABS] ;
	Flt	o_dmax[NSLABS] ;
	struct t_objectprocs *o_procs ;
	struct Material 	*o_surf ;
	void	* o_data ;
	int       iHit;
} Object ;
 
typedef struct t_compositedata {
	unsigned short 	c_size ;
	Object *	c_object[BUNCHINGFACTOR] ;
} CompositeData ;
 
typedef struct t_objectprocs ObjectProcs ;
 
typedef struct t_isect {
	Flt 		isect_t;
	Flt 		isect_value;
	int 		isect_enter;
	Object 		* isect_prim;
	Surface 	* isect_surf;
	Flt 		s1;
	Flt 		s2;
	long        bestObject;
	Vec			color;
} Isect ;
  
#define degtorad(x)	(((Flt)(x))*PI/180.0)

typedef struct t_qelem {
	Flt	q_key ;
	Object	* q_obj ;
} Qelem ;

extern struct Scene *SceneRd;

typedef struct Ray {
	Point P;
	Point D;
	Flt	SourceDistance;
	int Qsize;
	Qelem *Q;
	void *traverseStack;
	struct Scene *scene;
	Flt	rayeps;
	int	maxlevel;
	Vec BackgroundColor;
	Flt isect_t;
	long SkipObjects;
	double tmax;
	double value;
	long ncell;
} Ray;
 
#define RayPoint(ray,t,point)	VecAddS(t,(ray)->D,(ray)->P,point)

  

struct Raydata{
	void    *traverseStack;
	long bestObject;
	Isect nhits ;
	Ray *ray;
	double min_dist;
	void *userData;
};

extern struct Color *colorDEFSH;


Object *MakeTriangle(struct Scene *scene,int npoints,int direction,Surface *s, Vec *points,Vec *normals,Vec *col);

Object *MakeTriN(int npoints,Surface *s, Vec *points,Vec *normals);

Object *MakeTriN2(int npoints,Surface *s, Vec *points,Vec *normals,Vec *col);

Object *MakeTriNC(int npoints,Surface *s, Vec *points,Vec *normals,Flt *values);

Object *MakeTri(struct Scene *scene,Vec * points);

Object *MakeSphere(struct Color *color,Vec pos,Flt radius);

int Shade(Ray *ray,int level, Flt weight, Vec P,Vec N,Vec I, Isect *hit,Colorv col);

Object *MakeTriC1N1(int npoints,Surface *s, Vec *points,Vec *normals,Vec *col);

int SaveObject(Object *o,int flag);

int AllocatePQueue(Ray *ray);

int DallocatePQueue(Ray *ray);

int PriorityQueueInsert(Ray *ray,Flt key, Object * obj);

int PriorityQueueDelete(Ray *ray,Flt * key, Object ** obj);

int PriorityQueueNull(Ray *ray);

int PriorityQueueEmpty(Ray *ray);

int BuildBoundingSlabs(void);


#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif

