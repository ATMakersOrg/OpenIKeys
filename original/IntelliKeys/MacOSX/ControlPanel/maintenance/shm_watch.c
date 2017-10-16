#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <string.h>	

#define CLAIM_OFFSET 0
#define COMMAND_OFFSET 4
#define RESPONSE_OFFSET 2050

int main(int argc, char **argv)
{
    int    ret = 0;
    int   sval = 0;
    int memoryKey = atoi (argv[1]);
    
    int shmid;	//the id of our shared memory
    char *shm;	//the address of our shared memory
          
    errno = 0;
    if ((shmid = shmget(memoryKey, 4096, IPC_CREAT | 0777)) < 0) 
    {
      if (errno)
      {
        char *x = strerror(errno);
        printf("shmget error %s\n", x);
      }
    }
    printf("shmget successful %d\n", shmid);
        
    if ((shm = (char *)shmat(shmid, NULL, 0)) == (char *) -1) 
    {
        perror("shmat");
        exit (1);
    }        
  
    while (1) {
      int i = 0;
      printf ("memory at:");
      printf (" (%4.4d): %x, %x, %x, %x : ", CLAIM_OFFSET, shm[CLAIM_OFFSET+0], shm[CLAIM_OFFSET+1], shm[CLAIM_OFFSET+2], shm[CLAIM_OFFSET+3]);
      printf (" (%4.4d): %x, %x, %x, %x, %x : ", COMMAND_OFFSET, shm[COMMAND_OFFSET+0], shm[COMMAND_OFFSET+1], shm[COMMAND_OFFSET+2], shm[COMMAND_OFFSET+3], shm[COMMAND_OFFSET+4]);
      printf (" (%4.4d): %x, %x, %x, %x, %x\n", RESPONSE_OFFSET, shm[RESPONSE_OFFSET+0], shm[RESPONSE_OFFSET+1], shm[RESPONSE_OFFSET+2], shm[RESPONSE_OFFSET+3], shm[RESPONSE_OFFSET+4]);
      sleep (1);
    }

    exit (0);
}
