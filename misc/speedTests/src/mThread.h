#ifndef __MTHREAD__

#define __MTHREAD__


#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif                          /* c_plusplus || __cplusplus */
						
	int launchThread(void *data,int (*sageThread)(void *data));
	
	int zerol(char *p,unsigned long n);
	
	int Sleep2(int ms);

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */

#endif
