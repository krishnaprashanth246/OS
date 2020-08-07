/*
 * Program: Musical chairs game with n players and m intervals.
 * Author:  G.Vishal Siva Kumar,  T.Krishna Prashanth
 * Roll# :  CS18BTECH11013, CS18BTECH11045
 */

#include <stdlib.h>  /* for exit, atoi */
#include <iostream>  /* for fprintf */
#include <errno.h>   /* for error code eg. E2BIG */
#include <getopt.h>  /* for getopt */
#include <assert.h>  /* for assert */
#include <chrono>   /* for timers */
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>

using namespace std;

/*
 * Forward declarations
 */
int n, nplayers;
int i;  //global variable used for signalling, as analogue for music
int count;  //used for synchronization at lap_stop
int sleep_time[2000] = {0}; //global sleep time array

void usage(int argc, char *argv[]);
unsigned long long musical_chairs(int nplayers);

mutex k,m,s;
condition_variable laps;
condition_variable music;


int main(int argc, char *argv[])
{
    int c;

    /* Loop through each option (and its's arguments) and populate variables */
    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"help",            no_argument,        0, 'h'},
            {"nplayers",         required_argument,    0, '1'},
            {0,        0,            0,  0 }
        };

        c = getopt_long(argc, argv, "h1:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 0:
            cerr << "option " << long_options[option_index].name;
            if (optarg)
                cerr << " with arg " << optarg << endl;
            break;

        case '1':
            nplayers = atoi(optarg);
            break;

        case 'h':
        case '?':
            usage(argc, argv);

        default:
            cerr << "?? getopt returned character code 0%o ??n" << c << endl;
            usage(argc, argv);
        }
    }

    if (optind != argc) {
        cerr << "Unexpected arguments.\n";
        usage(argc, argv);
    }


    if (nplayers == 0) {
        cerr << "Invalid nplayers argument." << endl;
        return EXIT_FAILURE;
    }

    unsigned long long game_time;
    game_time = musical_chairs(nplayers);

    cout << "Time taken for the game: " << game_time << " us" << endl;

    exit(EXIT_SUCCESS);
}

/*
 * Show usage of the program
 */
void usage(int argc, char *argv[])
{
    cerr << "Usage:\n";
    cerr << argv[0] << "--nplayers <n>" << endl;
    exit(EXIT_FAILURE);
}

void umpire_main (int nplayers)
{
    /* Add your code here */
    /* read stdin only from umpire */
    while (n != 0)
    {
        string temp;

        cin>>temp;

        if (temp == "lap_start")
        {
            /*Already synchronized and checked in lapstop*/
        }

        if (temp == "player_sleep")
        {
            int plid,sleeptime;

            cin>>plid>>sleeptime;

            std::unique_lock<std::mutex> sleeplock (k); //acquire unique lock
            
            sleep_time[plid] = sleeptime;

            sleeplock.unlock ();//atomically release unique lock
        }//for taking player sleep input

        if (temp == "music_start")
        {
            /*Players are running while music is playing,
            they try to aquire chairs when it's stopped*/
        }

        if (temp == "umpire_sleep")
        {
            int sleeptime;

            cin>>sleeptime;

            this_thread::sleep_for (chrono::microseconds (sleeptime));//sleep
        }//umpire sleeps as soon as this input is taken

        if (temp == "music_stop")
        {
            std::unique_lock<std::mutex> look (m);

            i = 1;

            music.wait (look);  //wait till all players acquire chairs

            i = 0;  //reset

            continue;
        }

        if (temp == "lap_stop")
        {
            laps.notify_all (); //release laps conditional variable
        }

        if (n == 1)
        {
            std::unique_lock<std::mutex> look (m);

            i = 1;

            music.wait (look);
            break;
        }

    }

}

void player_main(int plid)
{
    /* Add your code here */
    /* synchronize stdouts coming from multiple players */
    while (1)
    {
        if (sleep_time[plid])
        {
            this_thread::sleep_for (chrono::microseconds (sleep_time[plid]));//sleep

            sleep_time[plid] = 0;
        }

        if (i)  //analogue of music start
        {
            std::unique_lock<std::mutex> locklap (s);   //acquire unique lock

            if (count == 1 && n != 1)
            {
                cout<<"======= lap#"<<" "<< (nplayers - n + 1) <<" "<<"======="<<endl;
                cout<< plid <<" could not get chair"<<endl;
                cout<<"**********************"<<endl;

                n--;    //decrese n as this is last thread
                count = n;//reset count

                music.notify_one ();    //wake up umpire which waits till the last player executes
                break;                  //break out of loop and exit
            }/*Executed by last player in a lap*/

            else if (n == 1)
            {
                cout<<"Winner is"<<" "<<plid<<endl;

                music.notify_one ();
                break;
            }/*Executed by winner of the game*/

            else
            {
                count--;

                laps.wait (locklap);    //atomically release unique lock
            }/*acquiring chair and waiting for completion of lap*/
        }
    }
}

unsigned long long musical_chairs(int nplayers)
{
    auto t1 = chrono::steady_clock::now();

    n = nplayers;   //n is the pressent number of running threads at any given instant of the program
    count = n;      //count is used for synchronization at lap_stop

    cout<<"Musical Chairs: "<<nplayers<<" player game with "<<(nplayers-1)<<" laps."<<endl;

    std::vector<std::thread> thread_pool;   //pool of threads for creation

    thread_pool.reserve (n);
    // Spawn umpire thread.
    /* Add your code here */
    thread umpire (umpire_main,nplayers);

    // Spawn n player threads.
    /* Add your code here */
    for (int i = 0; i < nplayers; i++)
    {   
        thread_pool.push_back(std::thread (player_main, i));
    }//each thread has its unique identifier passed by value through i

    for (int i = 0; i < nplayers; i++)
    {   
        thread_pool[i].join ();
    }
    /*Main thread waits for all player threads and umpire thread to be completed*/
    umpire.join ();

    auto t2 = chrono::steady_clock::now();

    auto d1 = chrono::duration_cast<chrono::microseconds>(t2 - t1);

    return d1.count();
}/*Return time taken for the musical chairs in us*/