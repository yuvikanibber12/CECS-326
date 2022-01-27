/* shmp3.cpp */

#include "booking3.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <memory.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

// there are 3 child processes being created
#define NCHILD  3

//pre-defining functions before they are called. This is done to avoid error.
int     shm_init( void * );
void    wait_and_wrap_up( int [], void *, int );
void    rpterror( char *, char * );

int main(int argc, char *argv[]) {
   // storing child PIDs. i = number of process and shmid = Shared memory segment ID. 
    int     child[NCHILD], i, shmid;
    //shared memory pointer
    void    *shm_ptr;
    //character array of shared memory segment and process name
    char    ascshmid[10], pname[14];
    //initializer
    sem_init(&(mybus.sem1),1,1);
    // initializes shared memory segment and assigns its identifier to shmid
    shmid = shm_init(shm_ptr);
    //storing shmid to char array ascshmid
    sprintf(ascshmid, "%d", shmid);
    //printing the bus number, title, date, seats left and when the booking begins
    cout << "Bus " << mybus.tour1.bus_number << " for "
        << mybus.tour1.title << " on " << mybus.tour1.date << ", "
        << mybus.tour1.seats_left << " seats available. " << endl;
    cout << "Booking begins: " << endl << endl;
    //creating NCHILD amount of processes
    for (i = 0; i < NCHILD; i++) {
        //creating new child process
        child[i] = fork();
        switch (child[i]) {
            //failed creation of child process
            case -1:
                sprintf (pname, "child%d", i+1);
                //printing out error message
                rpterror ((char *)"fork failed", pname);
                //exits the process with error
                exit(1);
                //successful creation of child process
            case 0:
            //Stores the process in the format "shmc%d" into pname
                sprintf (pname, "shmc%d", i+1);
                execl("shmc3", pname, ascshmid, (char *)0);
                //error message printing
                rpterror ((char *)"execl failed", pname);
                //exits the process with error 
                exit (2);
        }
    }
    //Waits for all the child processes in child to terminate before detaching the shared memory segment from shm_ptr and destroys the shared memory segment from the system
    wait_and_wrap_up (child, shm_ptr, shmid);
    //destroys semaphore after it's done being used
    sem_destroy(&(mybus.sem1)); 
	
}
// shared memory segment is initialized and assigns to the pointer
//returns shared memory segment ID
int shm_init(void *shm_ptr) {
    //shared memory segment ID
    int shmid;
/*
     creates shared memory segment as the size of the BUS structure
     */
    shmid = shmget(ftok(".",'u'), sizeof(BUS), 0600 | IPC_CREAT);
    //acquiring shared memory segment has failed
    if (shmid == -1) {
        //error message
        perror ("shmget failed");
        //exits with the correct error message
        exit(3);
    }
    //shm_ptr is attatched the created shared memory segment
    shm_ptr = shmat(shmid, (void * ) 0, 0);
    //indicates that attatchment failed
    if (shm_ptr == (void *) -1) {
        //error message
        perror ("shmat failed");
        //exits with error
        exit(4);
    }
    //data of the created BUS structure is copied into shm_ptr
	//size of data = bytes
    memcpy (shm_ptr, (void *) &mybus, sizeof(BUS) );
    return (shmid);
}
/*
 Waits for all child processes to terminate before detaching the shared memory segment from the pointer *shm_ptr and removing the shared memory segment associated with the identifier shmid
 */
void wait_and_wrap_up(int child[], void *shm_ptr, int shmid) {
    //wait_rtn = flag that indicates the status of the child
     //w = index of child process
     //ch_active = # of child processes that are active
    int wait_rtn, w, ch_active = NCHILD;
//runs until there are no child processes active
    while (ch_active > 0) {
        //waits for child process to terminate
        wait_rtn = wait( (int *)0 );
        //decrements the amount of child processes running
        for (w = 0; w < NCHILD; w++)
        if (child[w] == wait_rtn) {
            //decrementing amount of child processes that are active
            ch_active--;
            break;
        }
    }
    //states which shm was removed
    cout << "Parent removing shm" << endl;
    
   //detaches the shared memory segment associated with the pointer
    shmdt (shm_ptr);
    //Removes the shared memory segment identifier specified with shmid and destroys the shared memory segment and data structure associated
    shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0);
    exit (0);
}
/*
 printing out message that states which parameters were passed
 also prints out error message
 */
void rpterror(char *string, char *pname) {
    //declaring array size
    char errline[50];
    //storing passed parameters
    sprintf (errline, "%s %s", string, pname);
    //printing out error message
    perror (errline);
}
