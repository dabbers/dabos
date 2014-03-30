#include "type.h"

/********* in type.h ***********
typedef unsigned char   u8;
typedef unsigned short u16;
typedef unsigned long  u32;

#define NULL     0
#define NPROC    9
#define SSIZE 1024

//******* PROC status ********
#define FREE     0
#define READY    1
#define RUNNING  2
#define STOPPED  3
#define SLEEP    4
#define ZOMBIE   5

typedef struct proc{
    struct proc *next;
    int    *ksp;

    int    uss, usp;           // at offsets 4, 6 
    int    pid;                // add pid for identify the proc
    int    status;             // status = FREE|READY|RUNNING|SLEEP|ZOMBIE    
    int    ppid;               // parent pid
    struct proc *parent;
    int    priority;
    int    event;
    int    exitCode;

    char   name[32];
    int    kstack[SSIZE];      // per proc stack area
}PROC;
*******************************/

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;
int procSize = sizeof(PROC);
int nproc = 0;

int body();
char *pname[]={"P0", "P1", "P2", "P3",  "P4", "P5","P6", "P7", "P8" };

/**************************************************
  bio.o, queue.o loader.o are in mtxlib
**************************************************/
/* #include "bio.c" */
/* #include "queue.c" */
/* #include "loader.c" */

/**********
#include "wait.c"
#include "kernel.c"
#include "int.c"
************/


void ksleep(int evt)
{
	PROC * q;

	running->event = evt;
	running->status = SLEEP;
	if (sleepList == 0)
		sleepList = running;
	else
	{
		q = sleepList;
		while (q->next)
			q = q->next;
		q->next = running;
	}
	running->next = 0;
	tswitch();
}

void wakeup( int evt )
{
  PROC *p, *q; int i;

  p = q = sleepList;

  while( p )
  {
    if ( p->event != evt )
	{
      q = p;
      p = p->next;
      continue;
    }
 
    if ( p==sleepList )
	{
		 sleepList = p->next;
         p->status = READY;
         p->event = 0;
         enqueue(&readyQueue, p);
         p = q = sleepList;
         continue;
    }

    q->next = p->next;
    p->event = 0;
    p->status = READY;
    enqueue( &readyQueue, p );
    p = q->next;
  }   
}
int do_tswitch()
{ 
	return tswitch();
}

int do_kfork()
{
	if (! kfork("/bin/u1") )
	{
		printf("FORK FAILED!\n");
	}
}

void freeproc(PROC*p)
{
	PROC *q;
	if (freeList == 0)
		freeList = p;
	else
	{
		q = freeList;
		while (q->next)
		{
			q = q->next;
		}
		q->next = p;
	}

	p->next = 0;
}

int do_wait( int *status )
{
	int children, i;
	
	children = 0;

	for( i = 0; i < NPROC; i++ )
	{
		if ( proc[i].ppid == running->pid )
		{
			children++;
		}
	}

	while( children )
	{
		for (i = 0; i < NPROC; i++)
		{
			if (proc[i].status == ZOMBIE && proc[i].ppid == running->pid)
			{
				*status = proc[i].exitCode;
				proc[i].status = FREE;
				freeproc(&proc[i]);
				return i;
			}
		}
		ksleep(running);
	}

	return -1;
}


int do_exit(int exitval)
{	char c;
	int i;
	
	// Prevent PROC 1 from dying if it has ANY children.
	if ( running->pid == 1 )
	{
		for ( i = 0; i < NPROC; i++ )
		{
			if (proc[i].ppid == running->pid)
			{
				return -1;
			}
		}
	}

	running->status = ZOMBIE;

	running->exitCode = exitval;

	for ( i = 0; i < NPROC; i++ )
	{
		if (proc[i].ppid == running->pid)
		{
			proc[i].ppid = 1;
		}
	}

	wakeup(&proc[running->ppid]);
	tswitch();
}


int do_ps()
{
	int i;
	printf("PID\tPPID\tNAME\tSTATUS\n");

	for (i = 0; i < NPROC; ++i)
	{
		PROC* p = &proc[i];
		printf("%d\t%d\t%s\t", p->pid, p->ppid, p->name);
		switch(p->status) 
		{
			case READY:   printf("\tREADY");  break;
			case SLEEP:   printf("\tSLEEP");	break;
			case ZOMBIE:  printf("\tZOMBIE"); break;
			case FREE:    printf("\tFREE");   break;
			default:      printf("\t???");    break;
		}
		printf("\n");
	}
	printf("\n");
	return 1;
}

int kmode()
{
	body();
}
int chname(char* buffer)
{
	int i;
	char tmp = get_byte(running->uss, buffer);

	if ( tmp == '\0' )
		return -1;

	running->name[0] = tmp;

	for (i = 1; i < 32; ++i) 
	{
		running->name[i] = get_byte(running->uss, buffer+i);
		if (running->name[i] == '\0')
			break;
	}
    running->name[31] == '\0';

	return 0;
}

int init()
{
    PROC *p; int i;
    printf("init ....");
    for (i=0; i<NPROC; i++){   // initialize all procs
        p = &proc[i];
        p->pid = i;
        p->status = FREE;
        p->priority = 0;  
        strcpy(proc[i].name, pname[i]);
   
        p->next = &proc[i+1];
    }
    freeList = &proc[0];      // all procs are in freeList
    proc[NPROC-1].next = 0;
    readyQueue = sleepList = 0;

    /**** create P0 as running ******/
    p = get_proc(&freeList);
    p->status = READY;
    p->ppid   = 0;
    p->parent = p;
    running = p;
    nproc = 1;
    printf("done\n");
} 

int int80h();

int set_vec(u16 vector, u16 handler)
{
     // put_word(word, segment, offset) in mtxlib
     put_word(handler, 0, vector<<2);
     put_word(0x1000,  0, (vector<<2) + 2);
}
            
main()
{
    printf("MTX starts in main()\n");
    init();      // initialize and create P0 as running
    set_vec(80, int80h);

    kfork("/bin/u1");     // P0 kfork() P1

    while(1){
      printf("P0 running\n");
      while(!readyQueue);
      printf("P0 switch process\n");
      tswitch();         // P0 switch to run P1
   }
}

int scheduler()
{
    if (running->status == READY)
        enqueue(&readyQueue, running);
    running = dequeue(&readyQueue);
}

int body()
{
  char c;
  printf("proc %d resumes to body()\n", running->pid);
  while(1){
    printf("-----------------------------------------\n");
    printList("freelist  ", freeList);
    printList("readyQueue", readyQueue);
    printList("sleepList ", sleepList);
    printf("-----------------------------------------\n");

    printf("proc %d running: parent = %d  enter a char [s|f|w|q|u] : ", 
	   running->pid, running->parent->pid);

    c = getc(); printf("%c\n", c);
    switch(c){
       case 's' : do_tswitch();   break;
       case 'f' : do_kfork();     break;
       case 'w' : do_wait();      break;
       case 'q' : do_exit();      break;

       case 'u' : goUmode();      break;
    }
  }
}
    
int kfork(char *filename)
{
  PROC *p;
  int  i, child;
  u16 word;
  u16  segment;

  /*** get a PROC for child process: ***/
  if ( (p = get_proc(&freeList)) == 0){
       printf("no more proc\n");
       return(-1);
  }

  /* initialize the new proc and its stack */
  p->status = READY;
  p->ppid = running->pid;
  p->parent = running;
  p->priority  = 1;                 // all of the same priority 1

  /******* write C code to to do THIS YOURSELF ********************
     Initialize p's kstack AS IF it had called tswitch() 
     from the entry address of body():

   HI   -1  -2    -3  -4   -5   -6   -7   -8    -9                LOW
      -------------------------------------------------------------
      |body| ax | bx | cx | dx | bp | si | di |flag|
      ------------------------------------------------------------
                                                ^
                                    PROC.ksp ---|

  ******************************************************************/

  // fill in resume address
  p->kstack[SSIZE-1] = (int)body;

  // save stack TOP address in PROC
  p->ksp = &(p->kstack[SSIZE - 9]);


  enqueue(&readyQueue, p);
 
  // make Umode image by loading /bin/u1 into segment
  segment = (p->pid + 1)*0x1000;
  load(filename, segment);

  /*************** WRITE C CODE TO DO THESE ******************
     Initialize new proc's ustak AS IF it had done INT 80
     from virtual address 0:

    HI  -1  -2  -3  -4  -5  -6  -7  -8  -9 -10 -11 -12
       flag uCS uPC uax ubx ucx udx ubp usi udi ues uds
     0x0200 seg  0   0   0   0   0   0   0   0  seg seg
                                                     ^
    PROC.uss = segment;           PROC.usp ----------|
 
  ***********************************************************/

	for (i = 1; i < 13; ++i)
	{
		switch(i) 
		{
			case 1:   word = 0x0200;  break;  // uFlag
			case 2:
			case 11:
			case 12:  word = segment; break;  // uCS, uES, uDS
			default:  word = 0;       break;  // pretty much everything else
		}

		put_word(word, segment, 0x1000-i*2);  // stack starts at highest end of segment
	}
	
    p->uss = segment;
    p->usp = 0x1000 - 24; // usp is byte address, x2

  printf("Proc%d forked a child %d segment=%x\n", running->pid,p->pid,segment);
  return(p->pid);
}




int ufork()
{
	PROC *p;
	int  i, child;
	u16  segment;
    u16 wurd;

	int from_segment = (running->pid + 1) * 0x1000;

	p = get_proc(&freeList);

	if ( p == 0 )
	{
		return -1;
	}
	printf("ufork:2\n");
	p->status = READY;
	p->ppid = running->pid;
	p->parent = running;
	p->priority = running->priority;

	for ( i=1; i<10; i++ )
	{
		p->kstack[SSIZE-i] = 0;
	}

	p->ksp = &(p->kstack[SSIZE - 9]);

	p->kstack[SSIZE-1] = (int)goUmode;

	enqueue(&readyQueue, p);
	nproc++;

	segment = (p->pid + 1) * 0x1000;

	for(i = 0; i < 0x1000; i++) 
	{
		put_word( get_word( from_segment, i ), segment, i );
	}


	for (i = 1; i < 13; ++i)
	{
		switch(i) 
		{
			case 1:   wurd = 0x0200;  break;  // uFlag
			case 2:
			case 11:
			case 12:  wurd = segment; break;  // uCS, uES, uDS
			case 10: put_word(0, segment, 0x1000-i*2); continue;
			default:  wurd = 0;       break;  // pretty much everything else
		}

		put_word(wurd, segment, 0x1000-i*2);  // stack starts at highest end of segment
	}

	p->uss = segment;
	p->usp = 0x1000-24;

	put_word(0, segment, running->usp + 8*2);

	printf("[%d] forked [%d]: %x\n", running->pid, p->pid, segment);

	return p->pid;
}

int exec(char* filepath)
{
	char path[128];
	int i;
	u16 word;
	u16 seg_size = 0x1000;
	u16 segment;
	segment =  (running->pid + 1) * seg_size;

	for (i = 0; i < 128; ++i)
	{
		path[i] = get_byte(segment, filepath + i);
		if (path[i] == '\0')
			break;
	}
	printf("path: '%s'\n", path);
	load(path, segment);


	for (i = 1; i < 13; ++i)
	{
		switch(i) 
		{
			case 1:   word = 0x0200;  break;  // uFlag
			case 2:
			case 11:
			case 12:  word = segment; break;  // uCS, uES, uDS
			case 10: put_word(0, segment, 0x1000-i*2); continue;
			default:  word = 0;       break;  // pretty much everything else
		}

		put_word(word, segment, 0x1000-i*2);  // stack starts at highest end of segment
	}

	running->uss = segment;
	running->usp = 0x1000-24;

	put_word(0, segment, running->usp + 8*2);
	return;
}



/*************************************************************************
  usp  1   2   3   4   5   6   7   8   9  10   11   12    13  14  15  16
----------------------------------------------------------------------------
 |uds|ues|udi|usi|ubp|udx|ucx|ubx|uax|upc|ucs|uflag|retPC| a | b | c | d |
----------------------------------------------------------------------------
***************************************************************************/
#define PA 13
#define PB 14
#define PC 15
#define PD 16
#define AX  8

/****************** syscall handler in C ***************************/
int kcinth()
{
   u16    segment, offset;
   int    a,b,c,d, r;
   segment = running->uss; 
   offset = running->usp;

   /** get syscall parameters from ustack **/
   a = get_word(segment, offset + 2*PA);
   b = get_word(segment, offset + 2*PB);
   c = get_word(segment, offset + 2*PC);
   d = get_word(segment, offset + 2*PD);

   // UNCOMMENT TO SEE syscalls into kernel
   // printf("proc%d syscall a=%d b=%d c=%d d=%d\n", running->pid, a,b,c,d);

   switch(a){
       case 0 : r = running->pid;     break;
       case 1 : r = do_ps();          break;
       case 2 : r = chname(b);        break;
       case 3 : r = kmode();          break;
       case 4 : r = tswitch();        break;
       case 5 : r = do_wait(b);       break;
       case 6 : r = do_exit(b);       break;
       case 7 : r = ufork();          break;
       case 8 : r = exec(b);          break;
       
       case 90: r =  getc();          break;
       case 91: r =  putc(b);         break;       
       case 99: do_exit(b);           break;
       default: printf("invalid syscall # : %d\n", a); 
   }
   put_word(r, segment, offset + 2*AX);
}
