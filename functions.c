
//das dann auch noch in eigenen Header?

#include "dataTypes.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>  //to work with time()
#include <math.h> //to work with time() ->dont foget to link with -lm

#define MAX_STRING_LEN 20

double value_of(u_int array_element); //function takes value of array and retunrns it ->this way original array value is left unmanipulated when kernel is run over it



/*
*
* INPUT
*
*/
//returns pointer to first position of array whose elements represent all the pixels of the ppm
//array-space is allocated inside function ->dont forget to free inside calling routine
s_pixel *read_from_ppm(FILE *input_ppm, u_int *color_depth, u_int *width, u_int *height)
{
#if FS_DEBUG
    fprintf(stderr,"\t-read_from_ppm- entered\n");
#endif
//variables for the various loops and ifs()
    int c = 0;
    unsigned int array_element = 0;
    int string_hit = 0, string_cnt = 0;
    char pixel_cnt = 0; //counts from 0 to 2 ->0= red information, 1 = green information, 2 = blue inforamtion of one pixel
    char *str_end = NULL; //to work with strtoul
    s_pixel *ppm_as_array = NULL;
    char arg[MAX_STRING_LEN]; //holds each word of the ppm-file during reading
//read throught the ppm-file skip comments, process header and pixels
    while ((c = fgetc(input_ppm)) != EOF)
    {
        if(isspace(c) == 0)
        {
            int i = 0;
            ungetc(c, input_ppm); //push character we pulled from file to id EOF condition back to stream
            while (i < (MAX_STRING_LEN-1))
            {
                arg[i] = fgetc(input_ppm);
                if(isspace((int)arg[i]) != 0)
                {
                    arg[i] = '\0';
                    break;
                }
                if(arg[i] == '#')
                {
                    arg[i] = '\0';
                    for( ; fgetc(input_ppm) != '\n'; );
                    break;
                }
                i++;
            }
            if(arg[i] != '\0') //don´t set an other null-termination character if one has already been set
            {
                arg[i+1] = '\0';
            }
            i = 0; //reset i

            if(strlen(arg) >= 1) //filter out empty strings (empty string <= 1, everything elese is hight)
            {
            #if INPUT_FS_DEBUG
                printf("current string: |%s|\n",arg);
            #endif
                //strcpy(arg, "P3");
        //identify magic number P3..if P6 was entered ->ERROR message this format is not supported by program, otherwise(if P3 is magic-no, no action needs to be taken)
                if(string_cnt == 0 && strncmp(&arg[0], "P6", 3) == 0) //magic number is P6 ->not supported
                {
                    fprintf(stderr,"ERROR: P6 ppm-image format is not supported by program\n");
                    exit(EXIT_FAILURE); //termiate program clean
                }
                if(string_cnt == 0 && strncmp(&arg[0], "P3", 3) == 0) //magic number is P3
                {
                #if INPUT_FS_DEBUG
                    printf("magic no found\n");
                #endif
                    string_hit = 1;
                }
//the first two number-string inside the ppm file are the width and height of the picture
                if(string_cnt == 1 && arg[0] != '\0') //read in height
                {
                //*width ->because we manipulate value that is pointed to by a pointer
                    *width = (unsigned int)strtoul(&arg[0],&str_end,10);
                /*check if strol() run proper*/
                //str_end == '\0', if not \0.. conversion failed ->problem character is pointet to by str_end
                    if(*str_end != '\0')
                    {
                        fprintf(stderr,"ERROR: problem with strtoul() of width and character: %c can`t be converted\n",*width);
                        exit(EXIT_FAILURE);
                    }
                //empty string was passed on to stroul
                    if(arg[0] == *str_end)
                    {
                            fprintf(stderr,"ERROR: empty string was passed on to strtoul() when reading in width\n");
                            exit(EXIT_FAILURE);
                    }
                //converted value falls out of range
                    if (errno == ERANGE)
                    {
                        fprintf(stderr,"range error when reading in width");
                        exit(EXIT_FAILURE);
                        errno = 0; //reset error identifer ->not needed in this case as be terminate the program one line above
                    }


                    string_hit = 1;
                }

                if(string_cnt == 2 && arg[0] != '\0') //read in height
                {
                    *height = (unsigned int)strtoul(&arg[0],&str_end,10);
                /*check if strol() run proper*/
                //str_end == '\0', if not \0.. conversion failed ->problem character is pointet to by str_end
                    if(*str_end != '\0')
                    {
                        fprintf(stderr,"ERROR: problem with strtoul() of height and character: %c can`t be converted\n",*width);
                        exit(EXIT_FAILURE);
                    }
                //empty string was passed on to stroul
                    if(arg[0] == *str_end)
                    {
                            fprintf(stderr,"ERROR: empty string was passed on to strtoul() when reading in height\n");
                            exit(EXIT_FAILURE);
                    }
                //converted value falls out of range
                    if (errno == ERANGE)
                    {
                        fprintf(stderr,"range error when reading in height");
                        exit(EXIT_FAILURE);
                        errno = 0; //reset error identifer ->not needed in this case as be terminate the program one line above
                    }

                    string_hit = 1;
                #if INPUT_FS_DEBUG
                    printf("format-height: %d\n",*height);
                    printf("format-width: %d\n",*width);
                #endif
                        //allocate array with fitting space to later save the pixels
                #if INPUT_FS_DEBUG
                    printf("\tarray space is allocated\n");
                #endif
                    ppm_as_array = (s_pixel*)calloc( (*height) * (*width), sizeof(s_pixel));
                    if(ppm_as_array == NULL)
                    {
                        fprintf(stderr,"ERROR: problem with array allocation iside input.c\n");
                        exit(EXIT_FAILURE);
                    }
                }


                if(string_cnt == 3)
                {
            //read in color depth
                    *color_depth = (unsigned int)strtoul(&arg[0],&str_end,10);
                /*check if strol() run proper*/
                //str_end == '\0', if not \0.. conversion failed ->problem character is pointet to by str_end
                    if(*str_end != '\0')
                    {
                        fprintf(stderr,"ERROR: problem with strtoul() of color_depth and characrter: %c can`t be converted\n",*width);
                        exit(EXIT_FAILURE);
                    }
                //empty string was passed on to stroul
                    if(arg[0] == *str_end)
                    {
                            fprintf(stderr,"ERROR: empty string was passed on to strtoul() when reading in color_depth\n");
                            exit(EXIT_FAILURE);
                    }
                //converted value falls out of range
                    if (errno == ERANGE)
                    {
                        fprintf(stderr,"range error when reading in color_depth\n");
                        exit(EXIT_FAILURE);
                        errno = 0; //reset error identifer ->not needed in this case as be terminate the program one line above
                    }

                //check if color depth is in valid range
                    //see ppm-specifications: http://netpbm.sourceforge.net/doc/ppm.html
                //The maximum color value (Maxval), again in ASCII decimal. Must be less than 65536 and more than 0 (this is guaranteed as color_depth is unsigned so no negativ value possible anyway)
                    if(*color_depth > 65536)
                    {
                        fprintf(stderr,"ERROR: color_depth of ppm is not in valid range 0..65536\n");
                        exit(EXIT_FAILURE);
                    }
                    string_hit = 1;
                #if INPUT_FS_DEBUG
                    printf("color_depth: %i\n",*color_depth);
                #endif
                }

//now read pixels to allocated array
                if(string_cnt >= 4)
                {
                    //static variable: holds value from function call/sub-routine to function call/sub-routine and is initalized with 0 upon first call
                    static char pixel_cnt_reset; //numeric value, but char saves memory space compared to int
    //read in red pixel value
                    if(pixel_cnt == 0)
                    {

/* stroul()  ->string to unsigned long (long...long integer)
general syntax:
    unsigned long strtoul( const char *str, char **str_end, int base );
parameters:
    # str...pointer to the null-terminated byte string to be interpreted
    # str_end...pointer to a pointer to character.
    # base...base of the interpreted integer value
return:
    # on success::  Integer value corresponding to the contents of str
    # on failure::  see error flags
error-flags:
    # If the converted value falls out of range of corresponding return type, range error occurs (errno is set to ERANGE)
      and ULONG_MAX or ULLONG_MAX is returned.
    # If no conversion can be performed, ​0​ is returned.
    # if a character in the string is not a vlid number of the given base (dec, hex, bin, oc,..) stroul stops the conversion
      and returns the value of the number represented by the string that it has read up to the false characrter + str_end points
      points to the false character (under a perfect read str_end should point to '\0')
*/
                        ppm_as_array[array_element].red = (unsigned int)strtoul(&arg[0],&str_end,10);
                    /*check if strol() run proper*/
                    //str_end == '\0', if not \0.. conversion failed ->problem character is pointet to by str_end
                        if(*str_end != '\0')
                        {
                            fprintf(stderr,"ERROR: problem with strtoul() of red pixel value, characrter: %c can`t be converted\n",\
                                ppm_as_array[array_element].red);
                            exit(EXIT_FAILURE);
                        }
                    //empty string was passed on to stroul
                        if(arg[0] == *str_end)
                        {
                                fprintf(stderr,"ERROR: empty string was passed on to strtoul() when reading in red pixel value\n");
                                exit(EXIT_FAILURE);
                        }
                    //converted value falls out of range
                        if (errno == ERANGE)
                        {
                            fprintf(stderr,"range error when reading in red pixel value\n");
                            exit(EXIT_FAILURE);
                            errno = 0; //reset error identifer ->not needed in this case as be terminate the program one line above
                        }

                    //check if pixel value is in valid range
                        if(ppm_as_array[array_element].red > *color_depth)
                        {
                            fprintf(stderr,"ERROR: red color value in pixel No: %i is out of range(0..%i)\n", (array_element+1), *color_depth);
                            exit(EXIT_FAILURE);
                        }
                    }
    //read in green pixel value
                    if(pixel_cnt == 1)
                    {
                        ppm_as_array[array_element].green = (unsigned int)strtoul(&arg[0],&str_end,10);
                    /*check if strol() run proper*/
                    //str_end == '\0', if not \0.. conversion failed ->problem character is pointet to by str_end
                        if(*str_end != '\0')
                        {
                            fprintf(stderr,"ERROR: problem with strtoul() of green pixel value, characrter: %c can`t be converted\n",\
                                ppm_as_array[array_element].green);
                            exit(EXIT_FAILURE);
                        }
                    //empty string was passed on to stroul
                        if(arg[0] == *str_end)
                        {
                                fprintf(stderr,"ERROR: empty string was passed on to strtoul() when reading in green pixel value\n");
                                exit(EXIT_FAILURE);
                        }
                    //converted value falls out of range
                        if (errno == ERANGE)
                        {
                            fprintf(stderr,"range error when reading in green pixel value\n");
                            exit(EXIT_FAILURE);
                            errno = 0; //reset error identifer ->not needed in this case as be terminate the program one line above
                        }

                    //check if pixel value is in valid range
                        if(ppm_as_array[array_element].green > *color_depth)
                        {
                            fprintf(stderr,"ERROR: green color value in pixel No: %i is out of range(0..%i)\n", (array_element+1), *color_depth);
                            exit(EXIT_FAILURE);
                        }
                    }
    //read in blue pixel value
                    if(pixel_cnt == 2)
                    {
                        ppm_as_array[array_element].blue = (unsigned int)strtoul(&arg[0],&str_end,10);
                    /*check if strol() run proper*/
                    //str_end == '\0', if not \0.. conversion failed ->problem character is pointet to by str_end
                        if(*str_end != '\0' && *str_end != EOF) //*str_end != EOF ...to avoid error when last element of ppm-file is read in
                        {
                            fprintf(stderr,"ERROR: problem with strtoul() of blue pixel value, characrter: %c can`t be converted\n",\
                                ppm_as_array[array_element].blue);
                            exit(EXIT_FAILURE);
                        }
                    //empty string was passed on to stroul
                        if(arg[0] == *str_end)
                        {
                                fprintf(stderr,"ERROR: empty string was passed on to strtoul() when reading in blue pixel value\n");
                                exit(EXIT_FAILURE);
                        }
                    //converted value falls out of range
                        if (errno == ERANGE)
                        {
                            fprintf(stderr,"range error when reading in blue pixel value\n");
                            exit(EXIT_FAILURE);
                            errno = 0; //reset error identifer ->not needed in this case as be terminate the program one line above
                        }

                    //check if pixel value is in valid range
                        if(ppm_as_array[array_element].blue > *color_depth)
                        {
                            fprintf(stderr,"ERROR: blue color value in pixel No: %i is out of range(0..%i)\n", (array_element+1), *color_depth);
                            exit(EXIT_FAILURE);
                        }

                        #if INPUT_FS_DEBUG
                        //print current pixel ->every 3rd read a new pixel is printed and also print current array-element position
                            printf("Array element: %i\t",array_element);
                            printf("PIXEL: %u, %u, %u\n\n",ppm_as_array[array_element].red, ppm_as_array[array_element].green, ppm_as_array[array_element].blue);
                        #endif
                    //reset pixel count and ikrement array position by one (next element -consisting of 3 variables(struct) can be read in)
                        pixel_cnt = 0;
                        pixel_cnt_reset = 1;
                        array_element++;
                    }
                //only increment pixel_cnt if it was`t just resetet
                    if(pixel_cnt_reset == 1)
                    {
                        pixel_cnt_reset = 0;
                    }
                    else
                    {
                        pixel_cnt++;
                    }

                }
                if(string_hit == 1)
                {
                    string_cnt++;
                }
            }

        }
    }
#if 0
    if (feof(input_ppm))         // hit end of file
    {
        fclose(input_ppm); //not realy necessary to close strem of file that was opened in read-mode
#if FS_DEBUG
    fprintf(stdout,"EOF succesfull encountered\n");
#endif
    }
    else         // some other error interrupted the read
    {
        fprintf(stderr,"ERROR: read from file failed, read was interupted by something other than EOF encounter\n");
        exit(EXIT_FAILURE);
    }
#endif
//check if size matches with elements inside the ppm ->to avoid writing over array end
    //to many pixels
#if INPUT_FS_DEBUG
    printf("array element: %i, format: %i (%i x %i)\n", array_element, (*width) * (*height), *width, *height);
#endif
    if(array_element > (*width) * (*height))
    {
        fprintf(stderr,"ERROR: There are more pixel informations than specified by the format in the given file\n");
        exit(EXIT_FAILURE);
    }
    //to few pixels
    if(array_element < (*width) * (*height))
    {
        fprintf(stderr,"ERROR: There are less pixel informations than specified by the format in the given file\n");
        exit(EXIT_FAILURE);
    }
#if FS_DEBUG
    fprintf(stderr,"\t-read_from_ppm- exited , color_depth:%u, width: %u, height:%u\n", *color_depth, *width, *height);
#endif
    return ppm_as_array; //return pointer to first pos. of array that holds all pixel infos of the input ppm-file
}

/*
*
* CALCULATIONS
*
*/
//this function creates a frame around the original ppm-picture
//frame is initalized to hold all -0- (done by allocations space with calloc() that automaically allocated 0-initilized memory)
s_pixel *create_frame(s_pixel *unframed_ppm, const u_int img_height, const u_int img_width)
{
    s_pixel *ppm_with_frame = NULL;
    s_pixel *h_ppm_with_frame = NULL;
    //allocate array space for new array that than hold original ppm (pp_as_array) + a frame that holds values zero
        //using calloc() ->all allocated arrayspace is automatically initilaized with zero
    if( (ppm_with_frame = calloc( ((img_height+2)*(img_width+2)), sizeof(s_pixel))) == NULL)
    {
        fprintf(stderr,"**ERROR**\t calloc() inside create_frame() failed\n");
        exit(EXIT_FAILURE);
    }
  /*eventally do a memset() here so that value for frame can be chosen??*/

    h_ppm_with_frame = ppm_with_frame; //help pointer points to first pos of ppm_with_frame ->needed because ppm_with_frame is moved in following for-loop

//copy inner pixel values leave frame all 0
    for(u_int row = 1; row <= (img_height+2); row++)
    {
        for(u_int col = 1; col <= (img_width+2); col++)
        {
          //copy pixel from original array to framed array(inside frame)
          if(row != 1 && row != (img_height+2) && col != 1 && col != (img_width+2))
          {
            ppm_with_frame->red = unframed_ppm->red;
            ppm_with_frame->green = unframed_ppm->green;
            ppm_with_frame->blue = unframed_ppm->blue;
            unframed_ppm++; //move unframed ppm one pos
          }
            ppm_with_frame++; //move framed array one pos
        }
    }
    return h_ppm_with_frame;
}


/*

s_pixel filter(const int *kernel, master_slave_mq_pkg *MS_message, const int color_depth)
{
  int value;
  //calculate red value
  value = round(\
  MS_message.pixel_pkg[0]*kernel[0])
}
*/

s_pixel *apply_kernel(const s_pixel *framed_ppm, const int *kernel, const u_int height, const u_int width, const u_int color_depth)
{
    #if FS_DEBUG
        printf("apply_kernel() entered\n");
        int cnt = 0, cnt2 = 0;
    #endif
    double new_col_val = 0;
    s_pixel *filtered_ppm = NULL;
    s_pixel *h_filtered_ppm = NULL; //to later be able to return first pos of manipulated array
    filtered_ppm = calloc( ((width)*(height)) ,sizeof(s_pixel));
    if(filtered_ppm == NULL)
    {
        fprintf(stderr,"ERROR: inside apply_kernel() calloc() for manipulated_array failed\n");
        exit(EXIT_FAILURE);
    }

    h_filtered_ppm = filtered_ppm;
    if(h_filtered_ppm == NULL)
    {
        fprintf(stderr,"ERROR: inside apply_kernel() pointer refference of help_manipulated_array to manipulated_array failed\n");
        exit(EXIT_FAILURE);
    }

    for(u_int row = 1; row <= (height+2); row++)
    {
        for(u_int col = 1; col <= (width+2); col++)
        {
          if(row != 1 && row != (height+2) && col != 1 && col != (width+2))
          {
        //calcualte for red value of pixel
                new_col_val = round(
                            ((value_of((framed_ppm-width-1)->red)/255) * (*(kernel)) + (value_of((framed_ppm-width)->red)/255) * (*(kernel+3)) + (value_of((framed_ppm-width+1)->red)/255) * (*(kernel+6)) \
                            +(value_of((framed_ppm-1)->red)/255) * (*(kernel+1)) + (value_of((framed_ppm)->red)/255) * (*(kernel+4)) + (value_of((framed_ppm+1)->red)/255) * (*(kernel+7)) \
                            +(value_of((framed_ppm+width-1)->red)/255) * (*(kernel+2)) +(value_of((framed_ppm+width)->red)/255) * (*(kernel+5)) + (value_of((framed_ppm+width+1)->red)/255) * (*(kernel+8)) \
                            )*255 \
                                );
            //if color value is neagtiv
                if( new_col_val < 0)
                    filtered_ppm->red = 0;
            //else if color value is bigger than max color_depth
                if( new_col_val > color_depth)
                    filtered_ppm->red = color_depth;
            //neither smaller than 0, nor bigger than color_depth ->save value direct
                if( 0 < new_col_val && new_col_val < color_depth)
                    filtered_ppm->red = (u_int)new_col_val;

        //calcualte for green value of pixel
                new_col_val = round(
                            ((value_of((framed_ppm-width-1)->green)/255) * (*(kernel)) + (value_of((framed_ppm-width)->green)/255) * (*(kernel+3)) + (value_of((framed_ppm-width+1)->green)/255) * (*(kernel+6)) \
                            +(value_of((framed_ppm-1)->green)/255) * (*(kernel+1)) + (value_of((framed_ppm)->green)/255) * (*(kernel+4)) + (value_of((framed_ppm+1)->green)/255) * (*(kernel+7)) \
                            +(value_of((framed_ppm+width-1)->green)/255) * (*(kernel+2)) +(value_of((framed_ppm+width)->green)/255) * (*(kernel+5)) + (value_of((framed_ppm+width+1)->green)/255) * (*(kernel+8)) \
                            )*255 \
                                );
            //if color value is neagtiv
                if( new_col_val < 0)
                    filtered_ppm->green = 0;
            //else if color value is bigger than max color_depth
                if( new_col_val > color_depth)
                    filtered_ppm->green = color_depth;
            //neither smaller than 0, nor bigger than color_depth ->save value direct
                if( 0 < new_col_val && new_col_val < color_depth)
                    filtered_ppm->green = (u_int)new_col_val;

        //calcualte for blue value of pixel
                new_col_val = round(
                            ((value_of((framed_ppm-width-1)->blue)/255) * (*(kernel)) + (value_of((framed_ppm-width)->blue)/255) * (*(kernel+3)) + (value_of((framed_ppm-width+1)->blue)/255) * (*(kernel+6)) \
                            +(value_of((framed_ppm-1)->blue)/255) * (*(kernel+1)) + (value_of((framed_ppm)->blue)/255) * (*(kernel+4)) + (value_of((framed_ppm+1)->blue)/255) * (*(kernel+7)) \
                            +(value_of((framed_ppm+width-1)->blue)/255) * (*(kernel+2)) +(value_of((framed_ppm+width)->blue)/255) * (*(kernel+5)) + (value_of((framed_ppm+width+1)->blue)/255) * (*(kernel+8)) \
                            )*255 \
                                );
            //if color value is neagtiv
                if( new_col_val < 0)
                    filtered_ppm->blue = 0;
            //else if color value is bigger than max color_depth
                if( new_col_val > color_depth)
                    filtered_ppm->blue = color_depth;
            //neither smaller than 0, nor bigger than color_depth ->save value direct
                if( 0 < new_col_val && new_col_val < color_depth)
                    filtered_ppm->blue = (u_int)new_col_val;
                filtered_ppm++; //jump one position further in framed ppm
            }
            framed_ppm++; //jump one pos further in unframed ppm
        }
    }
    return h_filtered_ppm; //first pos of manipulated_array
}

//function takes value of array and retunrns it ->this way original array value is left unmanipulated when kernel is run over it
double value_of(u_int array_element)
{
    return array_element;
}









/*
*
* OUTPUT
*
*/
//function returns: pointer to array-space that was freed at the end of function ->pointer should point to NULL now
//inside function do:
    //print ppm-head
    //print pixels with help_pointer
    //fclose(output_ppm)
    //free() allocated array space at the end and set pointer array to point to NULL (this pointer is also returned then)
/*print_ppm()
general syntax:
    - s_pixel *print_ppm(FILE *output_ppm, const s_pixel *array, const u_int color_depth, const u_int width, const u_int height)
paramters::
    - output_ppm...FILE pointer to fiel that the ppm-image should be writen to
    - array...holds the pixel values of the array that is going to be printed to output_ppm
    - color_depth...color_depth of ppm-image
    - width/height...widht and height of ppm-image
note::
    - function reads the elements(each element of type -pixel-) of an 1D array(pointed to by -array_pos-) and prints
      them into an ppm-file (to generate a picture)
    - not only the values of the array, are printed, but also a header(needed for every ppm-file)
return::
    - on success: returns NULL
    - on failure: undefined
*/
void print_ppm(FILE *output_ppm, s_pixel *array, const u_int color_depth, const u_int width, const u_int height, const int *kernel)
{

//================================print ppm head =========================================
/*      P3                                                                              *
 *      #ppm_created by Fabian Sperk //simple comment                                   *
 *      format (width height)                                                           *
 *      color_depth                                                                     */
//========================================================================================

    if( fprintf(output_ppm,"P3\n") < 0 ) //magic number
    {
        fprintf(stderr,"ERROR: print of magic-No to output-pmm failed\n");
        exit(EXIT_FAILURE);
    }
//----------
    time_t result = time(NULL);
    if(result == -1)
    {
        fprintf(stderr,"ERROR: time() inside UIF_output.c failed\n");
        exit(EXIT_FAILURE);
    }
    if( fprintf(output_ppm,"#ppm created by Fabian Sperk time upon creation: %s\n",asctime(gmtime(&result))) < 0 )
    {
        fprintf(stderr,"ERROR: print of magic-No to output-pmm failed\n");
        exit(EXIT_FAILURE);
    }
    if( fprintf(output_ppm,"#kernel applied was: ") < 0 )
    {
        fprintf(stderr,"ERROR: print of magic-No to output-pmm failed\n");
        exit(EXIT_FAILURE);
    }
    //print kernel values in loop ->kernel holds values as int
    for(int i = 0; i < 9; i++)
        fprintf(output_ppm,"%i ",kernel[i]);
    if( fprintf(output_ppm,"\n%i   %i #img_width * img_height\n",width, height) < 0 )
    {
        fprintf(stderr,"ERROR: print of magic-No to output-pmm failed\n");
        exit(EXIT_FAILURE);
    }
    if( fprintf(output_ppm,"%i #color_depth\n",color_depth) < 0 )
    {
        fprintf(stderr,"ERROR: print of magic-No to output-pmm failed\n");
        exit(EXIT_FAILURE);
    }

//========================================================================================
/*                                                                                      *
 * //──── //print each pixel (containing 3 values(r,g,b)) ──────────────────────────────*
 *        234 200 35                                                                    *
 *        23 76 89                                                                      */
//========================================================================================

//fill in ppm-body (=all pixels)
    for(u_int i = 0; i < (width*height); i++)
    {
    //print red-value of the pixel
        fprintf(output_ppm,"%i ",array->red);
    //print green-value of the pixel
        fprintf(output_ppm,"%i ",array->green);
    //print blue-value of the pixel
        fprintf(output_ppm,"%i",array->blue);
    //enter next row (we print red,gree,blue of one pixel in one line)
        fprintf(output_ppm,"\n");
    //move pointer by one position
        array++;
    }

//fclose(ouput_ppm) ->close output_ppm stream (which has been opend in write mode outside of the function)
    //chech return ->​0​ on success, EOF otherwise.
    if( fclose(output_ppm) != 0)
    {
        fprintf(stderr,"ERROR: fclose() inside print_ppm() failed\n");
        exit(EXIT_FAILURE);
    }

#if FS_DEBUG
    printf("\tPRINT_PPM exited\n");
#endif
}
