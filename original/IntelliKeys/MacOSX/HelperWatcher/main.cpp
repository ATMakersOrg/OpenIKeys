extern "C"
{
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sysctl.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
}

typedef struct kinfo_proc kinfo_proc;

static int GetBSDProcessList(kinfo_proc **procList, size_t *procCount)
// Returns a list of all BSD processes on the system.  This routine
// allocates the list and puts it in *procList and a count of the
// number of entries in *procCount.  You are responsible for freeing
// this list (use "free" from System framework).
// On success, the function returns 0.
// On error, the function returns a BSD errno value.
{
    int                 err;
    kinfo_proc *        result;
    bool                done;
    static const int    name[] = { CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0 };
    // Declaring name as const requires us to cast it when passing it to
    // sysctl because the prototype doesn't include the const modifier.
    size_t              length;
	
    assert( procList != NULL);
    assert(*procList == NULL);
    assert(procCount != NULL);
	
    *procCount = 0;
	
    // We start by calling sysctl with result == NULL and length == 0.
    // That will succeed, and set length to the appropriate length.
    // We then allocate a buffer of that size and call sysctl again
    // with that buffer.  If that succeeds, we're done.  If that fails
    // with ENOMEM, we have to throw away our buffer and loop.  Note
    // that the loop causes use to call sysctl with NULL again; this
    // is necessary because the ENOMEM failure case sets length to
    // the amount of data returned, not the amount of data that
    // could have been returned.
	
    result = NULL;
    done = false;
    do {
        assert(result == NULL);
		
        // Call sysctl with a NULL buffer.
		
        length = 0;
        err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
                      NULL, &length,
                      NULL, 0);
        if (err == -1) {
            err = errno;
        }
		
        // Allocate an appropriately sized buffer based on the results
        // from the previous call.
		
        if (err == 0) {
            result = (kinfo_proc *) malloc(length);
            if (result == NULL) {
                err = ENOMEM;
            }
        }
		
        // Call sysctl again with the new buffer.  If we get an ENOMEM
        // error, toss away our buffer and start again.
		
        if (err == 0) {
            err = sysctl( (int *) name, (sizeof(name) / sizeof(*name)) - 1,
                          result, &length,
                          NULL, 0);
            if (err == -1) {
                err = errno;
            }
            if (err == 0) {
                done = true;
            } else if (err == ENOMEM) {
                assert(result != NULL);
                free(result);
                result = NULL;
                err = 0;
            }
        }
    } while (err == 0 && ! done);
	
    // Clean up and establish post conditions.
	
    if (err != 0 && result != NULL) {
        free(result);
        result = NULL;
    }
    *procList = result;
    if (err == 0) {
        *procCount = length / sizeof(kinfo_proc);
    }
	
    assert( (err == 0) == (*procList != NULL) );
	
    return err;
}

static void get_app_name ( char *arg, char *appname )
{
	//  find the last backslash
	size_t i = strlen(arg)-1;
	while (i > 0)
	{
		if (arg[i]=='/')
			break;
		i--;
	}
	
	//  give it up
	if (arg[i]=='/')
	{
		i++;
		strcpy ( appname, &(arg[i]) );
	}
	else
		strcpy ( appname, "" );
}

static int
get_args (int pid, char *cbuf, int csize)
{
	//  allocate a space to hold arguments
	char *arguments;
	int arguments_size = csize;
	arguments = (char *) malloc(arguments_size);

	//  get the arguments
	int mib[4];
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROCARGS;
	mib[2] = pid;
	mib[3] = 0;
	if (sysctl(mib, 3, arguments, (size_t *)&arguments_size, NULL, 0) < 0) 
	{
	    free(arguments);
		cbuf[0] = 0;
		return(0);
	}
	
	//  prevent buffer overrun
	arguments[arguments_size] = 0;
        
	//  replace all of the nulls with bars
	for (int i=0; i<arguments_size-1; i++)
		if (arguments[i] == 0)
                    arguments[i] = '|';
	
	//  give it back
	strcpy ( cbuf, arguments );

	//  all done.
	free (arguments);
	return (1);
}


static bool MacIsAppRunning ( const char *appname )
{
    bool bFound = false;

    size_t count = 0;
    kinfo_proc *procList = NULL;
    int result = GetBSDProcessList(&procList, &count);
    if (result==0)
    {        
        static char args[4096];
        
        for (int i = 0; i < count; i++)
        {
            get_args (procList[i].kp_proc.p_pid, args, sizeof(args)-1);
            if (strstr(args,appname) != NULL)
            {
                bFound = true;
                break;
            }
            if (strstr(procList[i].kp_proc.p_comm,appname) != NULL)
            {
                bFound = true;
                break;
            }
        }
    }
    
    if (procList != NULL) {
        free(procList);
    }
    
    return bFound;
}

#define shortName2 "USBMenu|"
#define longName2  "/usr/bin/open \"/Applications/IntelliTools/IntelliKeys USB/Private/USBMenu.app\""
#define PERIOD 1000000  //  one second

//
//  This program checks for the existence of
//  a certain program, and if it's missing, launches it.
//

int main (int argc, const char * argv[]) 
{
    //  do this forever
    while (1==1)
    {        
		char *login = getlogin();
		if(login)
		{
			if (strlen(login)>0)
			{
				bool bRunning = MacIsAppRunning ( shortName2 );
				if (!bRunning)
				{
					//  launch it
					char command[255];
					strcpy(command,"");
					strcat(command,longName2);
					system(command);
				}
			}
		}
        
        //  sleep a while
        usleep(PERIOD);
    }

    return 0;
}
