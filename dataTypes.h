#ifndef _dataTypes_
#define _dataTypes_

/*
* DEFINES used in mulitple files
*/
#define FS_DEBUG 1
#define KERNEL_FS_DEBUG 1

/*
* USER-DEFINED DATATYPES
*/
typedef unsigned int u_int;

typedef struct{
  u_int red;
  u_int green;
  u_int blue;
} s_pixel;

typedef struct{
  int depth;
  int width;
  int height;
} s_img;



#endif //_dataTypes_
