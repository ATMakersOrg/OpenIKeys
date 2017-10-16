//#define DEBUG 1

#if TARGET_OS_WIN32

	#include "IKCommon.h"
	#include <aclapi.h>
#include <stdio.h>

#else

extern "C" 
{
	#include <errno.h>
	#include <sys/types.h>
	#include <sys/ipc.h>
	#include <sys/shm.h>
	#include <semaphore.h>
	#include <string.h>	
}

#endif

#include "sharedMemory.h"


#if !TARGET_OS_WIN32
#pragma export on
#endif

//create the shared memory block (memory and semaphore)
extern "C" int CreateSharedMemory(SharedMemBlock *block)
{

#if TARGET_OS_WIN32

	//  set up security descriptors for full access

	PSECURITY_DESCRIPTOR pSD = NULL;
	EXPLICIT_ACCESS ea;
	PACL pACL = NULL;

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof (SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = FALSE;

    PSID pEveryoneSID = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;

    if(AllocateAndInitializeSid(&SIDAuthWorld, 1,
                     SECURITY_WORLD_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pEveryoneSID))
	{
		ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
		ea.grfAccessPermissions = KEY_ALL_ACCESS;
		ea.grfAccessMode = SET_ACCESS;
		ea.grfInheritance= NO_INHERITANCE;
		ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ea.Trustee.ptstrName  = (LPTSTR) pEveryoneSID;

		DWORD dwRes = SetEntriesInAcl(1, &ea, NULL, &pACL);
		if (ERROR_SUCCESS == dwRes) 
		{
			pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
									 SECURITY_DESCRIPTOR_MIN_LENGTH); 
			if (pSD) 
			{ 
				if (InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION)) 
				{  
					if (SetSecurityDescriptorDacl(pSD, 
							TRUE,     // bDaclPresent flag   
							pACL, 
							FALSE))   // not a default DACL 
					{  
						sa.lpSecurityDescriptor = pSD;
					} 
				} 
			} 
		}
	}

	//  assume good
	int result = 1;

	//  create shared mem segment

	LPVOID ptr = NULL;
	HANDLE hMap;
	hMap = CreateFileMapping(
		(HANDLE) 0xFFFFFFFF, /* use paging file */
		&sa,                                       /* no security attr.  */
		PAGE_READWRITE,   /* read/write access         */
		0,                                                /* size: high 32-bits */
		SHMSZ,   /* size: low 32-bits  */
		block->winMemoryName);  /* name of map object */
	if (hMap)
	{
		//  Are we the first process to attach?
		BOOL fInit = (GetLastError() != ERROR_ALREADY_EXISTS);

		/* Get a pointer to the file-mapped shared memory. */
		ptr = MapViewOfFile(
			hMap,   /* object to map view of  */
			FILE_MAP_WRITE, /* read/write access              */
			0,                                   /* high offset:   map from  */
			0,                                   /* low offset:    beginning */
			0);                     /* default: map entire file */
		if (ptr)
		{
			//  save the pointer
			block->winMemoryPointer = ptr;

			//  zero memory if we're first
			if (fInit)
			{
				DWORD i;
				for (i=0;i<SHMSZ;i++)
					((char *)ptr)[i] = 0;
			}

			//now create the semaphore

			PSECURITY_DESCRIPTOR psd2 = (PSECURITY_DESCRIPTOR) malloc (SECURITY_DESCRIPTOR_MIN_LENGTH);
			SECURITY_ATTRIBUTES sa2 = {sizeof(SECURITY_ATTRIBUTES), psd2, TRUE};
			InitializeSecurityDescriptor ( psd2, SECURITY_DESCRIPTOR_REVISION );
			SetSecurityDescriptorDacl ( psd2, TRUE, (PACL) NULL, FALSE );

			HANDLE hSemaphore;
			LONG cMax = 1;
			hSemaphore = CreateMutex( 
				&sa2,   // security attributes
				//cMax,   // initial count
				//cMax,   // maximum count
				false, //the mutex is not locked by default
				block->semaphoreName);  // semaphore name

			free(psd2);

			if (hSemaphore) 
			{
				block->winSemaphoreHandle = hSemaphore;
			}
			else
				result = -2;
		}
		else
			result = -1;
	}
	else
		result = 0;

	//  clean up

    if (pSD) 
        LocalFree(pSD);

	if (pACL) 
		LocalFree(pACL);

	if (pEveryoneSID) 
		FreeSid(pEveryoneSID);

	return result;
	

#else

	int shmid;	//the id of our shared memory
	char *shm;	//the address of our shared memory
        	
    errno = 0;
	if ((shmid = shmget(block->memoryKey, SHMSZ, IPC_CREAT | 0777)) < 0) 
	{
		if (errno)
		{
			char *x = strerror(errno);
		}
		return shmid;
	}
        
	if ((shm = (char *)shmat(shmid, NULL, 0)) == (char *) -1) 
	{
		return -1;      
	}        
        
	block->address = shm;

	//create the semaphore for our shared memory stucture

	block->semaphoreID = sem_open(block->semaphoreName,O_CREAT, 0777,1);
    int sem_open_err = errno;
	if(block->semaphoreID == (sem_t *)-1)
		return 0;

#ifdef DEBUG_CONTROL_PANEL
    NSLog (@"CreateSharedMemory: key (%d); address (%p);", block->memoryKey, block->address);
#endif
    
	return 1;

#endif	
}

//lock our block
extern "C" int LockSharedMemory(SharedMemBlock *block){
#if TARGET_OS_WIN32
		DWORD dwWaitResult = WaitForSingleObject ( block->winSemaphoreHandle, 0L); //  timeout = 0
		switch (dwWaitResult){ 
			 case WAIT_OBJECT_0: 
			// The semaphore object was signaled.  Good.
			{
				return 1;
			}
			break; 
			case WAIT_TIMEOUT: 
			// Semaphore was nonsignaled, so a time-out occurred. Bad.
			{
				return 0;
			}
			break; 
		}
	return 0;

#else//mac side
	if(sem_trywait(block->semaphoreID)==0){
		return 1;
	}else{
		return 0;
	}
#endif	
}

//unlock our block
extern "C" int UnlockSharedMemory(SharedMemBlock *block){
#if TARGET_OS_WIN32
	if (ReleaseMutex(block->winSemaphoreHandle)){//,1,NULL)){
		return 1;
	}else{
		return 0;
	}
#else
	sem_post(block->semaphoreID);
	return 1;
#endif
}

//write to our block
extern "C" int WriteSharedMemory(SharedMemBlock *block,int offset, char *data){
#if TARGET_OS_WIN32
	DWORD i;
	for(i=0;i<strlen(data);i++){
		((unsigned char *)(block->winMemoryPointer))[i+offset]=data[i];
	}
	((unsigned char *)(block->winMemoryPointer))[i+offset]= 0;
	return i;
#else
	int i;
	for(i=0;i<strlen(data);i++){
		((unsigned char*)(block->address))[i+offset]=data[i];
	
	}
	((unsigned char*)(block->address))[i+offset]='\0';
	return 1;
#endif
}
//write a byte to our block
extern "C" int WriteSharedMemoryByte(SharedMemBlock *block,int offset, int byteVal){
#if TARGET_OS_WIN32
	
		((unsigned char *)(block->winMemoryPointer))[offset]=byteVal;
	
		return 1;
#else
	
#ifdef DEBUG_CONTROL_PANEL
    if (byteVal != 0) {
        NSLog (@"WriteSharedMemoryByte: key (%d); address (%p); offset (%d); byteVal (%d, %c)", 
               block->memoryKey, block->address, offset, byteVal, byteVal);
    }
#endif

    ((unsigned char*)(block->address))[offset]=byteVal;
	
		return 1;
#endif
}

//read from our block
extern "C" void ReadSharedMemory(SharedMemBlock *block,int offset,char *myString){
#if TARGET_OS_WIN32

	for(DWORD i=0;;i++){
		myString[i]=((char *)(block->winMemoryPointer))[i+offset];
		if(((unsigned char *)(block->winMemoryPointer))[i+offset]=='\0' || i> 1500)
	break;
	}
#else
	int i;
	for(i=0;;i++){
		myString[i]=(block->address)[i+offset];
		if(((unsigned char*)(block->address))[i+offset]=='\0' || i> 1500)
		break;
	}
#endif
}

//read a byte from our block
extern "C" int ReadSharedMemoryByte(SharedMemBlock *block,int offset){
#if TARGET_OS_WIN32
	

    unsigned char dataByte = ((unsigned char *)(block->winMemoryPointer))[offset];
	return dataByte;
	
#else
	
    unsigned char dataByte = ((unsigned char*)(block->address))[offset];

#ifdef DEBUG_CONTROL_PANEL
    if (dataByte != 0) {
        NSLog (@"ReadSharedMemoryByte: key (%d); address (%p); offset (%d); value (%d, %c)", 
               block->memoryKey, block->address, offset, dataByte, dataByte);
    }
#endif
    
	return ((unsigned char*)(block->address))[offset];
	return dataByte;
		
	
#endif
}

//destroy our block
extern "C" int DestroySharedMemory(SharedMemBlock *block){
#if TARGET_OS_WIN32
	//I don't need to destroy this on windows
	return 1;
#else
	return sem_unlink(block->semaphoreName);
	
#endif
}

//clear our block
extern "C" int ClearSharedMemory(SharedMemBlock *block){
#if TARGET_OS_WIN32
	DWORD i;
	for(i=4;i<4096;i++){
		((char *)(block->winMemoryPointer))[i]=0;
	}
	return i;
#else
	int i;
	for(i=4;i<4096;i++){
		(block->address)[i]=0;
	
	}
	return 1;
#endif
}

#if !TARGET_OS_WIN32
#pragma export off
#endif

