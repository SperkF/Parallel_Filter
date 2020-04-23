#ifndef _dataTypes_
#define _dataTypes_

/*
* DEFINES used in mulitple files
*/
#define FS_DEBUG 0
#define FS_DEBUG_MQ_CHATTER 0
#define FS_DEBUG_COM_SUM 0
#define INPUT_FS_DEBUG 0

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


struct mq_info{
  int mq_id;
  struct mq_info *pNext;
};

#endif //_dataTypes_
