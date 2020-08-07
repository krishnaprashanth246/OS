MATRIX MULTIPLICATION 
USING MULTI-THREAD AND MULTI-PROCESS PROCEDURES
---------------------------------------------------------------------
AUTHOR: T. KRISHNA PRASHANTH
ROLL NO.: CS18BTECH11045
---------------------------------------------------------------------
FILES SUBMITTED:
	matmul.c
	report.txt
	readme.txt

Program:
	This program measures the speedup given by using multiple
threads and multiple processes for parallelization.

Compilation:
	The C program is compiled on the linux terminal by the GNU 
GCC Compiler by using a flag '-lpthread'.
	$ gcc matmul.c -lpthread
Running the program:
	After compilation, the resulting binary file is executed on 
the terminal by providing command-line options

The format for running on the command-line is:
	matmul --ar <rows_in_A>  --ac <cols_in_A>  --br <rows_in_B> 
				 --bc <cols_in_B> [--interactive]
	The dimensions of each input matrix is given in the terminal.

The option [--interactive] is to be specified if matrices are to be
given by the user from stdin. If not specified, the program takes
random numbers to fill the matrices using the pseudo random number 
generator (rand();) available in standard C library in <stdlib.h>.
At the end, it prints the time taken for the matrix multiplication
through single-thread, multi-process and multi-thread fashions.
It also gives the speedup achieved from multi-process and multi-
threaded executions. The output matrix is only printed if interactive
option is given.

Sample input: 
	$ ./a.out --ar 200 --ac 200 --br 200 --bc 200
Sample output:
	Time taken for single threaded: 32119 us
	Time taken for multi process: 11252 us
	Time taken for multi threaded: 9670 us
	Speedup for multi process : 2.85 x
	Speedup for multi threaded : 3.32 x
---------------------------------------------------------------------
