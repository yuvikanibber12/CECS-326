#include "booking.h" //including booking file from which the BUS structure is called from
#include <sys/types.h>
#include <sys/ipc.h>
#include <semaphore.h>        //    Defines the POSIX
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <memory.h>
#include <fcntl.h>            //    File control options, used for creating semaphore
#include <pthread.h>        //    Used for threads

using namespace std;

/*declaring new BUS structure
 bus number = 4321
 date = 11-26-21
 title = Grand Canyon Tour
 amount of seats left = 20 */
BUS mybus = { "4321", "11262021", "Grand Canyon Tour", 20 };

// there are 3 child processes being created
#define NCHILD	3

#define SNAME "shmpSem" //name of semaphore (new)

//pre-defining functions before they are called. This is done to avoid error. 
int	shm_init( void * );
void	wait_and_wrap_up( int [], void *, int, sem_t * );
void	rpterror( char *, char * );

int main(int argc, char *argv[])
{
	// storing child PIDs. i = number of process and shmid = Shared memory segment ID. 
	int 	child[NCHILD], i, shmid;

	/*New variable created*/
    sem_t *sem;

	//shared memory pointer
	void	*shm_ptr;
	//character array of shared memory segment and process name
	char	ascshmid[10], pname[14];

	strcpy (pname, argv[0]); //new

	//initializes shared memory segment and assigns its identifier to shmid
	shmid = shm_init(shm_ptr);
	//storing shmid to char array ascshmid
	sprintf (ascshmid, "%d", shmid);

    //printing the bus number, title, date, seats left and when the booking begins
	cout << "Bus " << mybus.bus_number << " for " 
	     << mybus.title << " on " << mybus.date << ", " 
	     << mybus.seats_left << " seats available. " << endl;
	cout << "Booking begins: " << endl << endl;


	/*
     assigns to sem and initializes semaphore
     */
    sem = sem_open(SNAME, O_CREAT, 0644, 1);
    if(sem == SEM_FAILED) {
        perror("Semaphore Initialization");
        exit(1);
    }

    //creating NCHILD amount of processes
	for (i = 0; i < NCHILD; i++) {
		//creating new child process
		child[i] = fork();
		switch (child[i]) {
			//failed creation of child process
		case -1:
			sprintf (pname, "child%d", i+1);
			//printing out error message
			rpterror ("fork failed", pname);
			//exits the process with error
			exit(1);

			//successful creation of child process
		case 0:
		//Stores the process in the format "shmc%d" into pname
			sprintf (pname, "shmc%d", i+1);
			//replaces the program code with shmc1 and passes the pname and the PID as the arguments
			execl("shmc2", pname, ascshmid, (char *)0);
			//error message printing
			rpterror ("execl failed", pname);
			//exits the process with error 
			exit (2);
		}
	}
	
     //Waits for all the child processes in child to terminate before detaching the shared memory segment from shm_ptr and destroys the shared memory segment from the system
     wait_and_wrap_up (child, shm_ptr, shmid, sem);
	 return 0;
}

// shared memory segment is initialized and assigns to the pointer
//returns shared memory segment ID
int shm_init(void *shm_ptr)
{
	//shared memory segment ID
	int	shmid;
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
void wait_and_wrap_up(int child[], void *shm_ptr, int shmid, sem_t *sem)
{
	
     //wait_rtn = flag that indicates the status of the child
     //w = index of child process
     //ch_active = # of child processes that are active
     
	int wait_rtn, w, ch_active = NCHILD;
	wait_rtn = NCHILD;
    w = NCHILD;

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
	/*disconnects and closes the semaphore*/
    sem_unlink(SNAME);
    sem_close(sem);
	
	exit (0);
}
/*
 printing out message that states which parameters were passed
 also prints out error message
 */
void rpterror(char *string, char *pname)
{
	//declaring array size
	char errline[50];

    //storing passed parameters
	sprintf (errline, "%s %s", string, pname);
	//printing out error message
	perror (errline);
}
