/* shmc2.cpp */
#include "booking.h" //including booking file from which the BUS structure is called from
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <memory.h>
/* added include statements to fix the code*/
#include <semaphore.h> //POSIX semaphore instantiation
#include <pthread.h> //used to make the threads

using namespace std;
//referencing header file that has a structure called BUS
BUS *bus_ptr; 
//pointer for shared memory
void *memptr;
//pointer for process name
char *pname;
//shared memory ID and return value that detahces from shared memory segment
int shmid, ret;
//functions that are used throughput the program
void rpterror(char *), srand(), perror(), sleep();
//function
void sell_seats();

/*Added variables*/
sem_t *sem;
char *semName;

int main(int argc, char* argv[])
{
    //verifies that at least 3 arguments are passed
if (argc < 3) {
    //printing out usage message
fprintf (stderr, "Usage:, %s shmid\n", argv[0]);
//exiting the process
exit(1);
}
//first name of the process. Initializing process name. 
pname = argv[0];
//second argument has their ID stored as a decimal integer
sscanf (argv[1], "%d", &shmid);
semName = argv[2]; //new
/* shared memory segment is attatched to the shared memory identifier through the shmid 
second parameter is given a null pointer */

memptr = shmat (shmid, (void *)0, 0);
//if memptr = -1 then, shared memory segment attachment has failed.
if (memptr == (char *)-1 ) { 
    //error message
rpterror ("shmat failed"); 
//exits the system
exit(2); 
}
bus_ptr = (struct BUS *)memptr;
//function called to sell the seats
sell_seats(); 
//detaches shared memory segment associated w/ memptr and ret is initialized to 0 w/ successful completetion
ret = shmdt(memptr); 
//successful exiting of the program
exit(0); 
}
//function that sells all the seats in a BUS
void sell_seats() 
{
    //remaining seats
int all_out = 0; 
//psuedo random number is initialized to get pid
srand ( (unsigned) getpid() ); 
while ( !all_out) { /* loop to sell all seats */

/*doesn't allow semaphore to have value of 0 NEW*/
sem_wait(sem);

//makes sure there are seats left before they start selling
if (bus_ptr->seats_left > 0) { 
    //sleeps for a random number of seconds
sleep ( (unsigned)rand()%2 + 1); 
bus_ptr->seats_left--; //seat is removed
//sleeps for a random number of seconds
sleep ( (unsigned)rand()%5 + 1); 
//outputs the amount of seats left
cout << pname << " SOLD SEAT -- " 
 << bus_ptr->seats_left << " left" << endl;
}
else { //lets the user know that there are no seats left
all_out++; //changed to 1 to let them know that the seats are unavailable
cout << pname << " sees no seats left" << endl;
}

/*incremenets semaphore from 0 to 1.
         locks semaphore*/
        sem_post(sem);
        
sleep ( (unsigned)rand()%5 + 1); //sleeps for a random number of seconds
}
}
void rpterror(char* string)
{
char errline[50]; //char array for error message
sprintf (errline, "%s %s", string, pname); //%s means string
perror (errline); //prints out custom error message stored in errline
}