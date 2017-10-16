#ifdef __cplusplus
extern "C" {
#endif

#define SHMSZ     4096//the size of our shared memory space in characters
#define CONTROL_PANEL_CHANNEL 0
#define ENGINE_CHANNEL 1
#define BALLOON_CHANNEL 1

//these are just remnants from the SDk
void SayHello(void);
typedef void (*SayHelloPtr)(void);
//end of remnants


#if TARGET_OS_WIN32
//define the structure for windows
typedef struct SMblock
{
	char semaphoreName[255];
	int* semaphoreID;
	int size;
	int memoryKey;
	char *address;
	LPVOID winMemoryPointer;
	LPCTSTR winMemoryName;
	HANDLE winSemaphoreHandle;
}SharedMemBlock;

#else
// #include <semaphore.h>

//define the structure for OSX
typedef struct SMblock
{
	char semaphoreName[255];
	int* semaphoreID;
	int size;
	int memoryKey;
	char *address;
}SharedMemBlock;

#endif


int CreateSharedMemory(SharedMemBlock *block);
typedef int (*CreateSharedMemoryPtr)(SharedMemBlock *block);

int LockSharedMemory(SharedMemBlock *block);
typedef int (*LockSharedMemoryPtr)(SharedMemBlock *block);

int UnlockSharedMemory(SharedMemBlock *block);
typedef int (*UnlockSharedMemoryPtr)(SharedMemBlock *block);

int WriteSharedMemory(SharedMemBlock *block,int offset, char *data);
typedef int (*WriteSharedMemoryPtr)(SharedMemBlock *block,int offset, char *data);

int WriteSharedMemoryByte(SharedMemBlock *block,int offset, int byteVal);
typedef int (*WriteSharedMemoryBytePtr)(SharedMemBlock *block,int offset, int byteVal);

void ReadSharedMemory(SharedMemBlock *block,int offset,char *myString);
typedef void (*ReadSharedMemoryPtr)(SharedMemBlock *block,int offset,char *myString);

int ReadSharedMemoryByte(SharedMemBlock *block,int offset);
typedef int (*ReadSharedMemoryBytePtr)(SharedMemBlock *block,int offset);

int DestroySharedMemory(SharedMemBlock *block);
typedef int (*DestroySharedMemoryPtr)(SharedMemBlock *block);

int ClearSharedMemory(SharedMemBlock *block);
typedef int (*ClearSharedMemoryPtr)(SharedMemBlock *block);



#ifdef __cplusplus
}
#endif
void InitializeSharedMemory(void);

