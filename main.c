/*
* Message Queues are persitent, which means that if thy are not closed explicitly, they stay around even after program termination
* In the terminal: use ipcs to get info on opend message-quese; and ipcrm -Q key or ipcrm -q ip to remove unwanted message-queues
* (can be looked up in OneNote)
*
*
*
*
*/

/*
* INCLUDES
*/
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h> //perror(),..
#include <ctype.h> //isdigit(),..
#include <unistd.h>  //getopt(), fork()
#include <string.h> //strncmp(),..
#include <sys/sysinfo.h> //get_nprocs() and get_nprocs_conf()
#include <sys/sysinfo.h> // get_nprocs()
#include <sys/types.h> // for message-queue key_type,..
#include <sys/msg.h>   // for message-queue
#include "dataTypes.h"
/*
* DEFINES
*/
#define SANE_NO_OF_PROCS 6
/*
* PROTOTYPES
*/
void print_help (void);
//s_pixel *get_ppm(FILE *input_ppm, u_int *img_depth, u_int *img_width, u_int *img_height);
s_pixel *read_from_ppm (FILE *input_ppm, u_int *color_depth, u_int *width, u_int *height);
s_pixel *create_frame (s_pixel *ppm_as_array, const u_int img_height, const u_int img_width);
s_pixel *apply_kernel (const s_pixel *original_array, const int *kernel, const u_int height, const u_int width, const u_int color_depth);
void print_ppm (FILE *output_ppm, s_pixel *array, const u_int color_depth, const u_int width, const u_int height, const int *kernel);
/*
* MAIN PROGRAM
*/
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
	int kernel[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
	int i = 0, digit_cnt = 0, comma_cnt = 0;
	int error_flag = 0;
//to work with strtod()
	char *strtod_error = NULL;
	char *strtod_input = NULL;
//to work with strtol()
	char *strtol_end = NULL;
//varaibles to process ppm picture
	s_pixel *ppm_array = NULL;
	s_pixel *framed_ppm_array = NULL;
	u_int img_depth = 0, img_width = 0, img_height = 0;

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
			No_procs = (int) strtol (optarg, &strtol_end, 10);
			if (*strtol_end != '\0') {
				fprintf (stderr, "--ERROR--\t %c is not a number", *strtol_end);
			}
			//do some furhter errorchecking
			//value was to big, to small, or NULL-pointer was passed to strtol
			if (errno != 0) {
				perror ("strtol(-p), perror:");
			}

			//empty string was passed to strtol()
			if (*optarg == '\0') {
				fprintf (stderr, "--ERROR--\t empty pointer was passed to strtol()\n");
				print_help();
			}
			if (No_procs > SANE_NO_OF_PROCS || No_procs < 0) {
				fprintf (stderr, "--ERROR--\t %d is an insane number of proceses\n", No_procs);
				fprintf (stderr, "--ERROR--\t\tNOTE: max ammoutn of processes: %d\n", (int) SANE_NO_OF_PROCS);
				fprintf (stderr, "--ERROR--\t\tNOTE: min ammoutn of processes: 0 or more\n");
				print_help();
			}
#ifdef FS_DEBUG
			fprintf (stdout, "**DEBUG**\tRun with user defined procs, No_procs: %d\n", No_procs);
#endif

			break;
		//----------------------------------------------------------------------
		//identifier -k.. string that follows is filter kernel
		case 'k':
			checksum = checksum + 2;
			//check if 9 numbers and 8 ',' were entered

			while (optarg[i] != '\0') { //parse throug optarg ->NULL-terminated string
				// is not a number && is not '-' && is not ',' ->ERROR
				if (isdigit ( (int) optarg[i]) == 0 && optarg[i] != '-' && optarg[i] != ',') {
					fprintf (stderr, "--ERROR--\t character %c of kernel can`t be processed\n", optarg[i]);
					exit (EXIT_FAILURE);
				}
				if (isdigit ( (int) optarg[i])) {
					digit_cnt++;
				}
				if (optarg[i] == ',') {
					comma_cnt++;
				}
				i++; //move to next array position
			}
			if (comma_cnt != 8) {
				fprintf (stderr, "--ERROR--\t kernel contained the wrong amomount of commas to seperate the numbers\n");
				error_flag = 1;
			}
			if (digit_cnt != 9) {
				fprintf (stderr, "--ERROR--\t kernel contained the wrong ammount of numbers\n");
				error_flag = 1;
			}
			if (error_flag == 1) { //this way both errors can be displayed, otherwise only the first one would get noticed
				print_help();
			}
			//now that we know that the kernel-string is of corect syntax, we process it
			//no more errorchecking for strtod() required
			i = 0; //reset array position
			strtod_input = &optarg[0];
			while (i < 9) {
				kernel[i] = (int) strtod (strtod_input, &strtod_error);
#if FS_DEBUG
				printf ("**DEBUG**\tinside while():%i || kernel: %i || strtod_error: %c\n", i, kernel[i], *strtod_error);
#endif
//if a comma is encountered ->move pointer and increment i; also incrment by one if string-termination('\0') is encountered
				if (*strtod_error == ',' || *strtod_error == '.' || *strtod_error == '\0') {
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
			fprintf (stdout, "**DEBUG**\tfopen(%s,'r') of input_ppm file worked\n", optarg);
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
			fprintf (stdout, "**DEBUG**\tfopen(%s,'w') created output-file\n", optarg);
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
	if (checksum < 6) {
		fprintf (stderr, "--ERROR--\t insuficient arguments to progam\n");
		print_help();
	}
/********************************************************************************************/
/*													GETOPT() END												*/
/********************************************************************************************/


	/*set No_procs to system-core number if user has not specified
	 how many child processes he wants to creat, ->has not provided -p*/
	if (p_flag == 0) {
		No_procs = get_nprocs(); /*return number of processors(cores) configured on system*/
#if FS_DEBUG
		fprintf (stdout, "**DEBGUG**\tRun with system defined procs, No_procs: %d\n", No_procs);
#endif
	}

	//get input_ppm content
	ppm_array = (s_pixel*) read_from_ppm (input_ppm, &img_depth, &img_width, &img_height);
	if (ppm_array == NULL) {
		fprintf (stderr, "**ERROR**:\t read_from_ppm() failed\n");
		exit (EXIT_FAILURE);
	}
#if FS_DEBUG
	FILE *debug_input = NULL;
	debug_input = fopen ("debug_input.ppm", "w");
	print_ppm (debug_input, ppm_array, img_depth, img_width, img_height, &kernel[0]);
	//fclose(debug_input);
#endif

	framed_ppm_array = create_frame (ppm_array, img_height, img_width);
	if (ppm_array == NULL) {
		fprintf (stderr, "**ERROR**:\t create_frame() failed\n");
		exit (EXIT_FAILURE);
	}
#if FS_DEBUG
	FILE *debug_framed = NULL;
	debug_framed = fopen ("debug_framed.ppm", "w");
	print_ppm (debug_framed, framed_ppm_array, img_depth, img_width + 2, img_height + 2, &kernel[0]);
	//fclose(debug_framed);
#endif

	/*setup everything for the message queues*/
	int Master_Slave_Queue_ID = 0;
	int Slave_Master_Queue_ID = 0;
	key_t mq_key = IPC_PRIVATE; //we are guaranteed to get a unique msq-key (no "collision" with other msqs on the system)
	int mq_perms = 0666; //all rw-, no sticky bit -> - rw- rw- rw- //<sticky> <usr> <grp> <oth>
	int mq_flags = IPC_CREAT | IPC_EXCL; //create msq exclusive
	master_slave_mq_pkg MS_message;
	slave_master_mq_pkg SM_message;

	/*create message queue* ->dont forget to destroy them before termination*/
	Master_Slave_Queue_ID = msgget (mq_key, mq_perms | mq_flags);
	if (Master_Slave_Queue_ID == -1) {
		fprintf (stderr, "**ERROR**:\tMaster->Slave MQ failed\n");
		if (msgctl (Master_Slave_Queue_ID, IPC_RMID, NULL) < 0) {
			fprintf (stderr, "**ERROR**\t close of MQ failed, lookup with ipcs and close manaly with ipcrm -x ip\n");
		}
		exit (EXIT_FAILURE);
	}
	Slave_Master_Queue_ID = msgget (mq_key, mq_perms | mq_flags);
	if (Slave_Master_Queue_ID == -1) {
		fprintf (stderr, "**ERROR**:\tSlave->Master MQ failed\n");
		if (msgctl (Slave_Master_Queue_ID, IPC_RMID, NULL) < 0) {
			fprintf (stderr, "**ERROR**\t close of MQ failed, lookup with ipcs and close manaly with ipcrm -x ip\n");
		}
		exit (EXIT_FAILURE);
	}


	/*fork() to create child processes*/
//child_PID takes pid of forked-off child (No_procs ammount of children will be created)
	pid_t *child_PID = NULL;
	child_PID = (pid_t*) calloc (No_procs, sizeof (pid_t));
	if (child_PID == NULL) {
		fprintf (stderr, "calloc for PID_child failed\n");
		exit (EXIT_FAILURE);
	}

//fork No_porcs times
	for (int i = 0; i < No_procs; i++) {
		*(child_PID + i) = fork();
		switch ( *(child_PID + i) ) {
		/**************************** ERROR **************************************************/
					case -1:		/*fork() failed*/
						fprintf (stderr, "master->chld fork %d failed\n", i);
						break;
		/**************************** CHILD **************************************************/
					case 0:			/*we are inside child*/
						if(FS_DEBUG)
							printf ("#child\t**DEBUG**\tchild %i with PID: %ld created\n", i, (long) getpid());

							//should I setup handler for SIGTERM??
							while(1) { //run loop and calcualte pixels till you get terminated via SIGTERM or SIGKILL
								errno = 0;
								if( (msgrcv(Master_Slave_Queue_ID, &MS_message, (sizeof(MS_message) - sizeof(long)), 1, 0)) < 0)
								{
									if(errno != EAGAIN) //SM M-queue is empty
									{
										fprintf(stderr,"--ERROR--\treading from Slave_Master M-queue failed\n");
										//close msqs
										exit(EXIT_FAILURE);
									}
								}
								//calculate pixel
								//SM_message.filtered_pixel = filter(&kernel[0], &MS_message);
								SM_message.pixel_index = MS_message.pixel_index;
								SM_message.filtered_pixel = MS_message.pixel_pkg[4]; //just echo image
								//return filtered pixel
								errno = 0;
								if( (msgsnd(Slave_Master_Queue_ID, &SM_message, (sizeof(SM_message) - sizeof(long)), 0)) < 0)
								{
									if(errno != EAGAIN) //MS message queue is full
									{
										fprintf(stderr,"--ERROR--\treading from Slave_Master M-queue failed\n");
										//close msqs
										exit(EXIT_FAILURE);
									}
								}
							}
		/***************************** PARENT ***********************************************/
			default:		/*no error + not inside child ->must be inside parent*/
				break;
		}
		#if FS_DEBUG
			printf("inside for-loop\n");
		#endif
	}

//for again ->master_send and master_revc
	pid_t master_snd = 0;
	master_snd = fork();
	switch(master_snd)
	{
		/**************************** ERROR **************************************************/
					case -1:		/*fork() failed*/
						fprintf (stderr, "master->master fork failed\n");
						break;
		/**************************** Master snd **************************************************/
					case 0:			/*we are inside child*/
						if(FS_DEBUG)
							printf ("#master_snd\t**DEBUG**\tmaster_sendd; PID = %ld\n", i, (long) getpid());
							//send packages
							//enter for loop to send/recieve packages to/from children
							framed_ppm_array = framed_ppm_array+(img_width+2)+1; //begin at first pixel inside frame
							int col_cnt = 0;
							for (int i = 0; i < (img_width * img_height); i++)
							{

									if(col_cnt == img_width)
									{
										col_cnt = 0; //reset col_cnt
										framed_ppm_array = framed_ppm_array + 2; //skip frame pixels
									}
									col_cnt++;
								//load up package to send to children
									MS_message.mtype = 1;
									MS_message.pixel_index = i;
									MS_message.pixel_pkg[0] = *(framed_ppm_array - (img_width+2) -1);
									MS_message.pixel_pkg[1] = *(framed_ppm_array - (img_width+2) );
									MS_message.pixel_pkg[2] = *(framed_ppm_array - (img_width+2) +1);
									MS_message.pixel_pkg[3] = *(framed_ppm_array -1);
									MS_message.pixel_pkg[4] = *(framed_ppm_array);
									MS_message.pixel_pkg[5] = *(framed_ppm_array + 1);
									MS_message.pixel_pkg[6] = *(framed_ppm_array + (img_width+2) -1);
									MS_message.pixel_pkg[7] = *(framed_ppm_array + (img_width+2) );
									MS_message.pixel_pkg[8] = *(framed_ppm_array + (img_width+2) +1);

							//msgsnd in blocking-mode
									if( (msgsnd(Master_Slave_Queue_ID, &MS_message, (sizeof(MS_message) - sizeof(long)), 0)) < 0)
									{
												printf("ABBORT sending\n");
												exit(EXIT_FAILURE);
									}
									else{
										printf("pixel sent to child\n");
									}

									framed_ppm_array++; //jump to next pixel
								//read pixel-packages from Slave_Master -msq till MQ is empty

							}
							printf("master_snd terminates\n");
							//master_send terminates succesfull if all packages were sent
							exit(EXIT_SUCCESS);
		/***************************** Master Receive ***********************************************/
			default:		/*no error + not inside master_snd->must be inside master_rvc*/
				break;
	}


for(int i = 0; i < img_width*img_height; i++) //get all filtered pixels
{
	if( (msgrcv(Slave_Master_Queue_ID, &SM_message, (sizeof(SM_message) - sizeof(long)),0, 0)) < 0)
	{
		printf("ABBORT MQ master-rcv\n");
		exit(EXIT_FAILURE);
	}
	else{
		printf("pixel received from child\n");
		*(ppm_array + SM_message.pixel_index) = SM_message.filtered_pixel;
	}
}
//send SIGTERM to children and wait for their termination + check termination-status
//loop for No_procs
//kill(*(child_PID + i, SIGTERM);
//waitpid(); ->check if terminated proper and term cause was SIGTERM


//save content of filtered image to output_ppm
/**print_ppm(FILE *output_ppm, s_pixel *ppm_array, int img_depth, int img_width, height, kernel);**/
print_ppm (output_ppm, ppm_array, img_depth, img_width, img_height, &kernel[0]);

//close message Queues
	if( (msgctl(Master_Slave_Queue_ID, IPC_RMID, NULL)) < 0)
	{
		fprintf(stderr,"closing Master-Slave M-queue failed\n");
		exit(EXIT_FAILURE);
	}
	if( (msgctl(Slave_Master_Queue_ID, IPC_RMID, NULL)) < 0)
	{
		fprintf(stderr,"closing Slave-Master M-queue failed\n");
		exit(EXIT_FAILURE);
	}

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
