#!/bin/sh
ipcs -a
~/bin/sem_unlink 8765
~/bin/sem_unlink 5432
~/bin/sem_unlink 7654
~/bin/sem_unlink 6789
ipcs -a
