#ifndef __BATCHFILE__
#define __BATCHFILE__

#include <stdio.h>

typedef struct Icon {
    class Poly *pl;
    struct Icon *IconNext;
}Icon,*IconPtr;
      

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
    IconPtr myIcon;
    IconPtr myIcon2;
    int (*getLine)(IconPtr myIcon,char *line,long len);
} *BatchPtr;


int NextLine(FILE *input,char *line,int linelength);
int getCommand(char *line,CommandPtr cp);
char *stringCommand(CommandPtr cp);
int doubleCommand(double *value,CommandPtr cp);
int processFile(char *pathname);

int ProcessLine(char *line,BatchPtr Batch);

int BatchNextLine(BatchPtr Batch,char *line,long len);


#define BATCH_DOUBLE	0
#define BATCH_STRING	1
#define BATCH_BYTES		2

#endif
