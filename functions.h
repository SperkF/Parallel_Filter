#ifndef _Functions_
#define _Functions_

//inlcudes
#include "dataTypes.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>  //to work with time()
#include <math.h> //to work with time() ->dont foget to link with -lm
#include <sys/types.h> // for message-queue key_type,..
#include <sys/msg.h>   // for message-queue

//defines
#define MAX_STRING_LEN 20

//protoytypes
s_pixel *read_from_ppm (FILE *input_ppm, u_int *color_depth, u_int *width, u_int *height);
s_pixel *create_frame (s_pixel *ppm_as_array, const u_int img_height, const u_int img_width);
s_pixel *apply_kernel (const s_pixel *original_array, const int *kernel, const u_int height, const u_int width, const u_int color_depth);
void print_ppm (FILE *output_ppm, s_pixel *array, const u_int color_depth, const u_int width, const u_int height, const int *kernel);
s_pixel filter(int *kernel, s_pixel *pixel,int color_depth);
void term_with_MQ_del(key_t Master_Slave_Queue_ID, key_t Slave_Master_Queue_ID);

#endif
