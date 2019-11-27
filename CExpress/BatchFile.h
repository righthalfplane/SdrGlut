#ifndef __BATCHFILE__
#define __BATCHFILE__

#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif 

// #include "Xappl.h"
                         /* c_plusplus || __cplusplus */
typedef struct CommandInfo{
	char *command[256];
	double value[256];
	int count[256];
	int type[256];
	int getKind;
	char *line;
	int nword; 
	int n;   
} *CommandPtr;

typedef struct BatchInfo{
	FILE *input;
//    IconPtr myIcon;
 //   IconPtr myIcon2;
 //   int (*getLine)(IconPtr myIcon,char *line,long len);
} *BatchPtr;


int NextLine(FILE *input,char *line,int linelength);
int getCommand(char *line,CommandPtr cp);
char *stringCommand(CommandPtr cp);
int doubleCommand(double *value,CommandPtr cp);
int processFile(char *pathname);

int ProcessLine(char *line,BatchPtr Batch);

int BatchNextLine(BatchPtr Batch,char *line,long len);

int doSage1DBatch(BatchPtr Batch,char *name);
int doSage3DBatch(BatchPtr Batch,char *name);
int doSage2DBatch(BatchPtr Batch,char *name);
int doSDS3DBatch(BatchPtr Batch,char *name);
int doSDS2DBatch(BatchPtr Batch,char *name);
int do24BitBatch(BatchPtr Batch,char *name);
int doBatchDraw(BatchPtr Batch);
/* int doSageRewrite(BatchPtr Batch,char *name); */

// int NewEditLine(IconPtr myIcon,char *line,long len);
int doSageVolBatch(BatchPtr Batch,char *name);
int  doBatchExpression(BatchPtr Batch);

#define BATCH_DOUBLE	0
#define BATCH_STRING	1
#define BATCH_BYTES		2

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */



#endif
