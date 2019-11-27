#ifndef _MESSAGE_
#define _MESSAGE_


#if defined c_plusplus || defined __cplusplus
extern      "C"
{
#endif

#define MessageType_LIMITS      		1000
#define MessageType_SET_FRAME_NUMBER	1001

#define MessageType_SET_PALETTE			1002
#define MessageType_SET_AREA_RANGE		1003
#define MessageType_SET_REGION_DATA		1004
#define MessageType_SET_SYNC_REGION		1005

#define MessageType_GET_FRAME_COUNT		1006
#define MessageType_GET_FILE_OWNER		1007
#define MessageType_GET_FILE_LIST		1008

#define MessageType_GET_LINE_DATA		1009
#define MessageType_GET_AREA_DATA		1010
#define MessageType_GET_REGION_DATA		1011
#define MessageType_GET_LIMITS_DATA		1012
#define MessageType_GET_INFORMATION		1014
#define MessageType_GET_SCREEN_DATA		1015
#define MessageType_GET_CELL_DATA		1016
#define MessageType_PLOT_LINE_DATA		1017
#define MessageType_SEND_COMMAND_DATA	1018
#define MessageType_SET_CELL_DATA		1019


#define MessageType_DO_DIALOG				1030

#define MessageType_DO_PLANE_DIALOG			1031
#define MessageType_DO_RANGE_DIALOG			1032
#define MessageType_DO_SCALES_DIALOG		1033
#define MessageType_DO_SELECTION_DIALOG		1034
#define MessageType_DO_VECTOR_DIALOG		1035
#define MessageType_DO_AREA_DIALOG		    1036
#define MessageType_DO_GENERAL_DIALOG		1037
#define MessageType_DO_STREAMLINE_DIALOG	1038

#define MessageType_PRINT_FILE_INFORMATION	1039

struct Message1000{
    double xminData;
    double xmaxData;
    double yminData;
    double ymaxData;
    int MaintainAspectRatio;
    long ixmax;
    long iymax;

    long ixminData;
    long ixmaxData;
    long iyminData;
    long iymaxData;
    
    long CurrentFrame;
    
    long FrameCount;
    
    unsigned char *red;
    unsigned char *green;
    unsigned char *blue;
    
    struct FileInfo2 **FilesList;
    int *FileOwner;
    long FileCount;
    long FileCountMax;
    char **nameList;
	long nameCount;
};

/*
int SendMessageToAll(IconPtr myIcon,int SendToSelf,
                long MessageType,void *message);

*/

//int SendMessageByIcon(IconPtr myIcon,long MessageType,void *message);

int SendMessageByName(char *name,long MessageType,void *message);

struct SyncStruct{
    double xminData;
    double yminData;
    double xmaxData;
    double ymaxData;
    double xCurrent;
    double yCurrent;
    double xStart;
    double yStart;
};

#if defined c_plusplus || defined __cplusplus
}
#endif                          /* c_plusplus || __cplusplus */


#endif
