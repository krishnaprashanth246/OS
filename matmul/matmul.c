/*
 * Program: Speedup calculation of matrix multiplication with
 *          multi-processing and multi-threading.
 * Author:  Krishna Prashanth
 * Roll# :  CS18TECH11045
 */

#include <stdlib.h> /* for exit, atoi */
#include <stdio.h>  /* for fprintf */
#include <errno.h>  /* for error code eg. E2BIG */
#include <getopt.h> /* for getopt */
#include <assert.h> /* for assert */
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
/*
 * Forward declarations
 */

void usage(int argc, char *argv[]);
void input_matrix(int *mat, int rows, int cols);
void output_matrix(int *mat, int rows, int cols);
void init_matrix(int *mat, int rows, int cols);
unsigned long long single_thread_mm();
unsigned long long multi_thread_mm();
unsigned long long multi_process_mm();
void * thread_do(void* input);

int *A, *B, *C;
int crows, ccols;
int arows, acols, brows, bcols;
char interactive = 0;
int k = 20;//varied; 
//a small k x k sub-matrix of C will be evaluated by each thread/process

int main(int argc, char *argv[])
{
	int c;

	/* Loop through each option (and its's arguments) and populate variables */
	while (1) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] = {
			{"help",	no_argument,		0, 'h'},
			{"ar",		required_argument,	0, '1'},
			{"ac",		required_argument,	0, '2'},
			{"br",		required_argument,	0, '3'},
			{"bc",		required_argument,	0, '4'},
			{"interactive",	no_argument, 		0, '5'},
			{0,		0,			0,  0 }
		};

		c = getopt_long(argc, argv, "h1:2:3:4:5", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 0:
			fprintf(stdout, "option %s", long_options[option_index].name);
			if (optarg)
				fprintf(stdout, " with arg %s", optarg);
				fprintf(stdout, "\n");
			break;

		case '1':
			arows = atoi(optarg);
			break;

		case '2':
			acols = atoi(optarg);
			break;

		case '3':
			brows = atoi(optarg);
			break;

		case '4':
			bcols = atoi(optarg);
			break;

		case '5':
			interactive = 1;
			break;

		case 'h':
		case '?':
			usage(argc, argv);

		default:
			fprintf(stdout, "?? getopt returned character code 0%o ??\n", c);
			usage(argc, argv);
		}
	}

	if (optind != argc) {
		fprintf(stderr, "Unexpected arguments\n");
		usage(argc, argv);
	}

	unsigned long long time_single, time_multi_process, time_multi_thread;
	/*times for different procedures*/
	time_single = single_thread_mm();
	time_multi_thread = multi_thread_mm();
	time_multi_process = multi_process_mm();

	fprintf(stdout, "Time taken for single threaded: %llu us\n",
			time_single);
	fprintf(stdout, "Time taken for multi process: %llu us\n",
			time_multi_process);
	fprintf(stdout, "Time taken for multi threaded: %llu us\n",
			time_multi_thread);
	fprintf(stdout, "Speedup for multi process : %4.2f x\n",
			(double)time_single/time_multi_process);
	fprintf(stdout, "Speedup for multi threaded : %4.2f x\n",
			(double)time_single/time_multi_thread);

	exit(EXIT_SUCCESS);
}

/*
 * Show usage of the program
 */
void usage(int argc, char *argv[])
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "%s --ar <rows_in_A>  --ac <cols_in_A>"
			" --br <rows_in_B>  --bc <cols_in_B>"
			" [--interactive]",
			argv[0]);
	exit(EXIT_FAILURE);
}

/*
 * Input a given 2D matrix
 */
void input_matrix(int *mat, int rows, int cols)
{
	for (int i=0; i<rows; i++) {
		for (int j=0; j<cols; j++) {
			fscanf(stdin, "%d", mat+(i*cols+j));
		}
	}
}

/*
 * Output a given 2D matrix
 */
void output_matrix(int *mat, int rows, int cols)
{
	for (int i=0; i<rows; i++) {
		for (int j=0; j<cols; j++) {
			fprintf(stdout, "%d ", *(mat+(i*cols+j)));
		}
		fprintf(stdout, "\n");
	}
}

void init_matrix(int *mat, int rows, int cols)
{
	for (int i=0; i<rows; i++) {
		for (int j=0; j<cols; j++) {
			*(mat+(i*cols+j)) = rand();
		}
	}
}//initialize with random values

unsigned long long single_thread_mm()
{
	if (acols != brows) {
		exit(EXIT_FAILURE);
	} else {
		crows = arows;
		ccols = bcols;
	}//matix multiplication feasibility
	//Allocate memory for each matrix
	A = (int* )malloc(sizeof(int)*arows*acols);
	B = (int* )malloc(sizeof(int)*brows*bcols);
	C = (int* )malloc(sizeof(int)*crows*ccols);

	struct timeval start, end; //for calculation of time

	if (interactive == 1) {//take input
		fprintf(stdout, "Enter A:\n");
		fflush(stdout);
		input_matrix (A,arows,acols);

		fprintf(stdout, "Enter B:\n");
		fflush(stdout);
		input_matrix (B,brows,bcols);
	} else {//initialize with rand();
		init_matrix (A,arows,acols);
		init_matrix (B,brows,bcols);
	}

	gettimeofday (&start, NULL);//start time

	for (int i=0; i<arows; i++) {
		for (int j=0; j<bcols; j++) {
			int s = 0;
			for (int p=0; p<acols; p++) {
				s += (*(A+(i*acols+p))) * (*(B+(p*bcols+j)));
			}
			*(C+(i*ccols+j)) = s;
		}
	}//matrix multiplication using single thread

	gettimeofday (&end, NULL);//end time

	if (interactive == 1) {
		fprintf(stdout, "Result:\n");
		fflush(stdout);

		output_matrix (C,crows,ccols);
		fflush(stdout);
	}//output matrix only if interactive

	free(A);
	free(B);
	free(C);//free all the allocated memory
	//calculate time
	double time_taken = (end.tv_sec - start.tv_sec) * 1e6; 
	time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;
	return time_taken*1000000;
}//return time taken for single thread procedure in micro seconds

unsigned long long multi_thread_mm()
{
	if (acols != brows) {
		exit(EXIT_FAILURE);
	} else {
		crows = arows;
		ccols = bcols;
	}//matix multiplication feasibility
	//Allocate memory for each matrix
	A = (int* )malloc(sizeof(int)*arows*acols);
	B = (int* )malloc(sizeof(int)*brows*bcols);
	C = (int* )malloc(sizeof(int)*crows*ccols);

	if (interactive == 1) {
		fprintf(stdout, "Enter A:\n");
		fflush(stdout);
		input_matrix (A,arows,acols);

		fprintf(stdout, "Enter B:\n");
		fflush(stdout);
		input_matrix (B,brows,bcols);
	} else {
		init_matrix (A,arows,acols);
		init_matrix (B,brows,bcols);//initialize with rand();
	}

	struct timeval start, end; //for calculation of time

	int t = 0, rc;

	int n = (arows/k +1)*(bcols/k +1);//get n, number of threads based on k

	pthread_t threads[n];

	int *taskids[n];//for safe argument passing to threads

	void* status;

	gettimeofday(&start, NULL);//start time

	for (t=0;t<n;t++) {
		taskids[t] = (int*) malloc ( sizeof(int) );
		*taskids[t] = t;
		rc = pthread_create(&threads[t],NULL,thread_do,(void*)taskids[t]);
		if (rc) {//if thread creation failed
			exit(EXIT_FAILURE);
		}
	}

	for (int j=0;j<n;j++) {
		pthread_join(threads[j],&status);
	}//wait for all the threads to complete

	gettimeofday(&end, NULL);//end time

	if (interactive == 1){//output matrix only if interactive
		fprintf(stdout, "Result:\n");
		fflush(stdout);
		output_matrix (C,crows,ccols);
		fflush(stdout);
	}
	free(A);
	free(B);
	free(C);//free the allocated memory
	//calculate time
	double time_taken = (end.tv_sec - start.tv_sec) * 1e6; 
	time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;
	return time_taken*1000000;
}//return time taken for multi thread procedure in micro seconds

void * thread_do(void* thread_arg)
{
	int t = *((int*)thread_arg);//acquire argument
	//t-th thread
	//acquire bounds
	int a = (t/((arows/k)+1))*k;
	int b = (t%((arows/k)+1))*k;

	for (int i = a; (i < arows) && (i < a + k); i++) {
		for (int j = b; (j < bcols) && (j < b + k); j++) {
			int s = 0;
			for (int p = 0; p < acols; p++) {
				s += (*(A+(i*acols+p))) * (*(B+(p*bcols+j)));
			}
			*(C+(i*ccols+j)) = s;
		}
	}
}//procedure for each thread

unsigned long long multi_process_mm()
{
	if (acols != brows) {
		exit(EXIT_FAILURE);
	} else {
		crows = arows;
		ccols = bcols;
	}
	const int sizeA = sizeof(int)*arows*acols;
	const int sizeB = sizeof(int)*brows*bcols;
	const int sizeC = sizeof(int)*crows*ccols;
	//Allocate shared memory for each matrix
	int segment_id1 = shmget(IPC_PRIVATE, sizeA, IPC_CREAT | 0666);
	int segment_id2 = shmget(IPC_PRIVATE, sizeB, IPC_CREAT | 0666);
	int segment_id3 = shmget(IPC_PRIVATE, sizeC, IPC_CREAT | 0666);

	int n = (arows/k +1)*(bcols/k +1);//get n, number of processes based on k

	A = (int*) shmat(segment_id1, NULL, 0);//attach
	B = (int*) shmat(segment_id2, NULL, 0);//attach

	if (interactive == 1) {
		fprintf(stdout, "Enter A:\n");
		fflush(stdout);
		input_matrix(A,arows,acols);
		fprintf(stdout, "Enter B:\n");
		fflush(stdout);
		input_matrix(B,brows,bcols);
	} else {
		init_matrix(A,arows,acols);
		init_matrix(B,brows,bcols);
	}

	struct timeval start, end; //for calculation of time

	gettimeofday(&start, NULL);//start time

	for (int p = 0; p < n; p++) {
		int new_process = fork();
		if (new_process == 0) {//procedure for each process
			int t = p;//p-th process

			A = (int*) shmat(segment_id1, NULL, 0);//attach
			B = (int*) shmat(segment_id2, NULL, 0);//attach
			C = (int*) shmat(segment_id3, NULL, 0);//attach
			//acquire bounds
			int a = (t/((arows/k)+1))*k;
			int b = (t%((arows/k)+1))*k;

			for (int i = a; (i < arows) && (i < a + k); i++) {
				for (int j = b; (j < bcols) && (j < b + k); j++) {
					int s = 0;
					for (int p = 0; p < acols; p++) {
						s += (*(A+(i*acols+p))) * (*(B+(p*bcols+j)));
					}
					*(C+(i*ccols+j)) = s;
				}
			}

			shmdt(A);//detach
			shmdt(B);//detach
			shmdt(C);//detach

			exit(0);
		} else if (new_process < 0) {//process creation error
			exit(EXIT_FAILURE);
		}
	}

	for (int p = 0; p < n; p++)//wait for all processes to complete
		wait(NULL);

	gettimeofday(&end, NULL);//end time

	if (interactive == 1) {//output matrix only if interactive
		C = (int*) shmat(segment_id3, NULL, 0);//attach

		fprintf(stdout,"Result:\n");
		fflush(stdout);

		output_matrix(C,crows,ccols);
		fflush(stdout);

		shmdt(C);//detach
	}

	shmdt(A);//detach
	shmdt(B);//detach

	shmctl(segment_id1, IPC_RMID, NULL);//remove segment
	shmctl(segment_id2, IPC_RMID, NULL);//remove segment
	shmctl(segment_id3, IPC_RMID, NULL);//remove segment

	//calculate time
	double time_taken = (end.tv_sec - start.tv_sec) * 1e6; 
	time_taken = (time_taken + (end.tv_usec - start.tv_usec)) * 1e-6;
	return time_taken*1000000;
}//return time taken for multi process procedure in micro seconds
