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

typedef struct {
  long mtype;
  int pixel_index;
  s_pixel pixel_pkg[9]; //pixel package to filter
} master_slave_mq_pkg;

typedef struct {
  long mtype;
  int pixel_index;
  s_pixel filtered_pixel; //pixel package to filter
} slave_master_mq_pkg;

#endif //_dataTypes_
