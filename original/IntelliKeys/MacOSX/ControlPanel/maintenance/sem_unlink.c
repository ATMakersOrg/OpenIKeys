// sem_unlink.c
#include "sem_common.h"
#include <sys/shm.h>

#define PROGNAME "sem_unlink"
int
main(int argc, char **argv)
{
    int ret = 0;
    
    CHECK_ARGS(2, "<path>");
    if ((ret = sem_unlink(argv[1])) < 0) 
    {
        perror("sem_unlink");
    }

    int shmkey = atoi (argv[1]);
    
    printf ("shmkey = %d\n", shmkey);
    
    int shmid = shmget(shmkey, 4096, 0);
    if (shmid < 0) {
        perror("shmget");
    }

    printf ("shmid = %d\n", shmid);
    
    ret = shmctl(shmid, IPC_RMID, NULL);
    if (ret < 0) {
        perror("shmctl");
    }
    
    exit(ret);
}
