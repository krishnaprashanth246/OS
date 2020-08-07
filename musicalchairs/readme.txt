Implementation of Musical chairs using threads
---------------------------------------------------------------------
AUTHOR: T. KRISHNA PRASHANTH, G. VISHAL SIVA KUMAR
ROLL NO.: CS18BTECH11045, CS18BTECH11013
---------------------------------------------------------------------
FILES SUBMITTED:
	C++ source file(.cpp file)
	Report(.pdf file)
	Readme file(.txt file)
AIM: Understanding the classical IPC synchronization problems in 
	developing concurrent programs.
Compilation: The code is compiled using the command-
		  “ g++ <file_name.cpp> -pthread ”
Running: To run the code, we use the command-
“ ./a.out --np <number of players> < input.txt "
The input is taken from input.txt file.

Sample:
$ g++ uncharted.cpp -pthread -o musicalchairs

$ cat input4deterministic.txt

lap_start
player_sleep 0 1000
player_sleep 1 2000
player_sleep 2 3000
player_sleep 3 4000
music_start
umpire_sleep 200
music_stop
lap_stop
lap_start
player_sleep 0 1000
player_sleep 1 2000
player_sleep 2 3000
music_start
umpire_sleep 200000
music_stop
lap_stop
lap_start
player_sleep 0 1000
player_sleep 1 2000
music_start
umpire_sleep 800000
music_stop
lap_stop

$ ./musicalchairs --np 4 < input4deterministic.txt

Musical Chairs: 4 player game with 3 laps.
======= lap# 1 =======
3 could not get chair
**********************
======= lap# 2 =======
2 could not get chair
**********************
======= lap# 3 =======
1 could not get chair
**********************
Winner is 0
Time taken for the game: 1005148 us

$ cat input4rand.txt
lap_start
music_start
umpire_sleep 200
music_stop
lap_stop
lap_start
music_start
umpire_sleep 200000
music_stop
lap_stop
lap_start
music_start
umpire_sleep 800000
music_stop
lap_stop

$ ./musicalchairs --np 4 < input4rand.txt
Musical Chairs: 4 player game with 3 laps.
======= lap# 1 =======
1 could not get chair
**********************
======= lap# 2 =======
3 could not get chair
**********************
======= lap# 3 =======
0 could not get chair
**********************
Winner is 2
Time taken for the game: 1001594 us

