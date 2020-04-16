

/*
* INCLUDES
*/
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h> //perror(),..
#include <ctype.h> //isdigit(),..
#include <unistd.h>  //getopt()
#include <string.h> //strncmp(),..
#include <sys/sysinfo.h> // get_nprocs()
/*
* DEFINES
*/
#define FS_DEBUG 1
#define SANE_NO_OF_PROCS 6
/*
* PROTOTYPES
*/
void print_help (void);


int main (int argc, char *argv[])
{
	int getopt_return = 0;
	FILE *input_ppm = NULL; //FILE pointer to input-ppm file (image we want to filter)
	FILE *output_ppm = NULL; //FILE pointer to ppm-file that we will create and write our filtered image too
 //set to 1 if user has entered -p
	int p_flag = 0;
//used to calculate wether or not correct arguments were entered
	int checksum = 0;
	/*		checksum explained
	* -p hat WErigkeit 1, -k,-i,-o haben WErtigkeit 2
	* wenn nun ein -p, -i,.. vorkommt wird checksum um die jewilige Wertigkeit erhöht (+1 bzw. +2)
	* am ende des getopt()+switch() kontrolliere ich dann ob checksum > 6 ist, wenn nicht, war der Input falsch
	* mögliche Werte für checksum: 1+2+2+2=7(p,k,i,o); 2+2+2=6(k,i,o);||->1+2+2=5(p,k,i) würde als Fehleingabe erkannt
	*/
//amoutn of processes to create for parallelisation
	int No_procs = 0;
//variables used to process kernel
	int kernel[9] = {0, 0, 0 ,0 , 0 , 0, 0, 0, 0};
	int i = 0, digit_cnt = 0, comma_cnt = 0;
	int error_flag = 0;
//to work with strtod()
	 char *strtod_error = NULL;
	 char *strtod_input = NULL;
//to work with strtol()
	 char *strtol_end = NULL;

	//-------getopt() to process input---------------------
	//run getopt till getopt() return = -1, otherwise returns value of identifier [ASCII Table value]
	//each loop cycle getop() is called and returns one identifier: -p,-k,-i,....
	//if there is ":" behind the identifier, getopt reads the string following the identifier and writes it to optarg (global char array ->null terminated sting)
	while ( (getopt_return = getopt (argc, argv, "p:k:i:o:h")) != -1)
		switch (getopt_return) {
		//----------------------------------------------------------------------
		//identifier -p... string that follows is number that speciefs amount of child proceses to create
		case 'p':
			checksum = checksum + 1;
			p_flag = 1; //set flag ->used later on in main
			errno = 0; //reset errno previous to function call
			No_procs = (int)strtol(optarg,&strtol_end,10);
			if(*strtol_end != '\0')
			{
				fprintf(stderr,"--ERROR--\t %c is not a number",*strtol_end);
			}
			//do some furhter errorchecking
			//value was to big, to small, or NULL-pointer was passed to strtol
	        if(errno != 0)
					{
	            perror("strtol(-p), perror:");
					}

						//empty string was passed to strtol()
		        if(*optarg == '\0')
						{
	            fprintf(stderr,"--ERROR--\t empty pointer was passed to strtol()\n");
	        		print_help();
						}
					if(No_procs > SANE_NO_OF_PROCS || No_procs < 0)
					{
						fprintf(stderr,"--ERROR--\t %d is an insane number of proceses\n",No_procs);
						fprintf(stderr,"--ERROR--\t\tNOTE: max ammoutn of processes: %d\n",(int)SANE_NO_OF_PROCS);
						fprintf(stderr,"--ERROR--\t\tNOTE: min ammoutn of processes: 0 or more\n");
						print_help();
					}

			break;
		//----------------------------------------------------------------------
		//identifier -k.. string that follows is filter kernel
		case 'k':
			checksum = checksum + 2;
		//check if 9 numbers and 8 ',' were entered

			 while(optarg[i] != '\0')  //parse throug optarg ->NULL-terminated string
			 {
			 // is not a number && is not '-' && is not ',' ->ERROR
					 if(isdigit((int)optarg[i]) == 0 && optarg[i] != '-' && optarg[i] != ',')
					 {
							 fprintf(stderr,"--ERROR--\t character %c of kernel can`t be processed\n",optarg[i]);
							 exit(EXIT_FAILURE);
					 }
					 if(isdigit((int)optarg[i]))
					 {
							 digit_cnt++;
					 }
					 if(optarg[i] == ',')
					 {
							 comma_cnt++;
					 }
					 i++; //move to next array position
			 }
			 if(comma_cnt != 8)
			 {
					 fprintf(stderr,"--ERROR--\t kernel contained the wrong amomount of commas to seperate the numbers\n");
					 error_flag = 1;
			 }
			 if(digit_cnt != 9)
			 {
					 fprintf(stderr,"--ERROR--\t kernel contained the wrong ammount of numbers\n");
					 error_flag = 1;
			 }
			 if(error_flag == 1) //this way both errors can be displayed, otherwise only the first one would get noticed
			 {
				print_help();
			 }
	 //now that we know that the kernel-string is of corect syntax, we process it
	 //no more errorchecking for strtod() required
			 i = 0; //reset array position
			 strtod_input = &optarg[0];
			 while(i < 9)
			 {
					 kernel[i] = (int)strtod(strtod_input, &strtod_error);
					 #if FS_DEBUG
							 printf("**DEBUG**\tinside while():%i || kernel: %i || strtod_error: %c\n",i, kernel[i], *strtod_error);
					 #endif
//if a comma is encountered ->move pointer and increment i; also incrment by one if string-termination('\0') is encountered
					 if(*strtod_error == ',' || *strtod_error == '.' || *strtod_error == '\0')
					 {
							 strtod_error++; //move pointer by one position
							 strtod_input = strtod_error; //let input pointer point to new position
							 i++;
					 }
			 }

			break;
		//----------------------------------------------------------------------
		//identifier -i... string that follows is name of ppm-file to filter
		case 'i':
			checksum = checksum + 2;
			input_ppm = fopen (optarg, "r"); //optarg -> holds string that followed the -i
			if (input_ppm == NULL) {
				fprintf (stderr, "--ERROR--\t fopen() for input_ppm failed\n");
				exit (EXIT_FAILURE);
			}
#if FS_DEBUG
				fprintf (stdout, "**DEBUG**\tfopen(%s,'r') of input_ppm file worked\n",optarg);
#endif
			break;
		//----------------------------------------------------------------------
		//identifier -o... string that follows is name that filtered ppm should habe
		case 'o':
			checksum = checksum + 2;
			//check if user has entered correct syntax ->.ppm(\0) Endung sind die letzten 5 charactere
			//übergib an strncmp() also den string erst ab der viert-letzten Stelle &filename[Stelle]
			if (strncmp (&optarg[strlen (optarg) - 4], ".ppm", 5) != 0) {
				fprintf (stderr, "--ERROR--\t fopen() for output_ppm failed\n");
				fprintf (stderr, "\t\tthe name of the output file has no .ppm ending\n");
				exit (EXIT_FAILURE);
			}
			//chek if filename is already taken, to avoid overwriting existing files
			//if such a file exist fopne() in read mode returns pointer other than null
			if ( (output_ppm = fopen (optarg, "r")) != NULL) {
				fprintf (stderr, "--ERROR--\t filenmae for output.ppm already exists\n");
				exit (EXIT_FAILURE);
			}
			//no file with same name exist so create one
			if ( (output_ppm = fopen (optarg, "w")) == NULL) {
				fprintf (stderr, "--ERROR--\t fopen(%s,'w') for output_ppm failed\n", optarg);
				exit (EXIT_FAILURE);
			}
			#if FS_DEBUG
							fprintf (stdout, "**DEBUG**\tfopen(%s,'w') created output-file\n",optarg);
			#endif
			break;
		//----------------------------------------------------------------------
		//identifier -h... print helpmessage
		case 'h':
			print_help();
			break;
		//----------------------------------------------------------------------
		//wrong identifier entered
		default :
			fprintf (stderr, "--ERROR--\t wrong identifiers entered\n");
			print_help();
		}
	//check for correct input
		if(checksum < 6)
		{
			fprintf(stderr,"--ERROR--\t insuficient arguments to progam\n");
			print_help();
		}
	/*set No_procs to system-core number if user has not specified
	 how many child processes he wants to creat, ->has not provided -p*/
	 if(p_flag = 0)
	 {
		 No_procs = get_nprocs(); /*return number of processors(cores) configured on system*/
	 }
	 #ifdef FS_DEBUG
		fprintf(stdout,"**DEBUG**\tNo_procs: %d, p_flag: %d\n",No_procs, p_flag);
	#endif
	//get input_ppm content
	get_ppm();
	//creat frame around input_ppm
	frame_ppm();
	//try send pixel-packages to Master_Slave-msq
	
	//try to retrieve-package from Slave_Master-msq
		//count each retieved pacakge ++ ->if count is pixelcount ->kill children, close msq, print ppm, close all alloc mem

	//save content of filtered image to output_ppm
	print_ppm();


	return 0;

}


void print_help (void)
{
	fprintf (stdout, "\n****************************HELP_MESSAGE**********************************\n\n");
	fprintf (stdout, "./img_filter [-p No_processes] -k kernel -i input_img -o output_img [-h]\n");
	fprintf (stdout, "Arguments inside brackets are optional\n\n");
	fprintf (stdout, "**************************************************************************\n");
	exit (EXIT_FAILURE);
}
