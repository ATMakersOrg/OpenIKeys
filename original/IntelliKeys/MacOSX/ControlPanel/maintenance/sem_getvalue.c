#include "sem_common.h"
#include <sys/sem.h>
#define PROGNAME "sem_getvalue"
int
main(int argc, char **argv)
{
    int    ret = 0;
    sem_t *sem;
    int   sval = 0;
    semun_t sem_data;
    
    CHECK_ARGS(2, "<path>");
    sem = sem_open(argv[1], 0);
    if (sem == (sem_t *)SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }
    if ((ret = semctl(sem, &sem_data, GETVAL)) < 0)
        perror("semctl");
    printf("successful %d\n", ret);
    sem_close(sem);
    exit(ret);
}
