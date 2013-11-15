

typedef sturct pofs_superblock posuperblock

struct pofs_superblock
{
char* nameP;

POMutex sbdescMutex;

SuperBlockDesc sbDesc;

fsmgrData  fsmgrData;

char *mountOptions;

char *mountPoint;

JournalFile* jfP;

Disk* diskP[MAX_DISKS_PER_POFS];

Boolean panicked;

Boolean mountedOnLocal;  //the file system is mounted on this local node

Boolean noMtime;   //if nomtime mount option is setup



}