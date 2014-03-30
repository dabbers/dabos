/* David Barajas 11329861 */
/************ t.c file **********************************/
#define NPROC     9        
#define SSIZE  1024                /* kstack int size */

#define DEAD      0                /* proc status     */
#define READY     1      
#define FREE      2      
#define SLEEP     3
#define ZOMBIE    4

// Used for Pointers
#define NULL 0

typedef struct proc
{
	struct proc *next;   
    int  ksp;               /* saved sp; offset = 2 */
    int  pid;
    int  ppid;
	int  evt;
	int  exit;
	struct proc *parentPtr;
    int  status;            /* READY|DEAD, etc */
    int  kstack[SSIZE];     // kmode stack of task
	int  priority;
} PROC;

#include "io.c"  /* <===== use YOUR OWN io.c with printf() ****/
#include "pri_queue.c"

PROC proc[NPROC], *running, *freeList, *readyQueue, *sleepList;

int  procSize = sizeof(PROC);

/****************************************************************
 Initialize the proc's as shown:
        running ---> proc[0] -> proc[1];

        proc[1] to proc[N-1] form a circular list:

        proc[1] --> proc[2] ... --> proc[NPROC-1] -->
          ^                                         |
          |<---------------------------------------<-

        Each proc's kstack contains:
        retPC, ax, bx, cx, dx, bp, si, di, flag;  all 2 bytes
*****************************************************************/

int body();  

int initialize()
{
	int i, j;
	PROC *p;

	freeList = NULL;
	readyQueue = NULL;
	sleepList = NULL;

	for (i=1; i < NPROC; i++)
	{
		p = &proc[i];
		p->next = NULL;
		p->pid = i;
		p->ppid = -1;
		p->status = FREE;
		p->priority = 1;
		p->parentPtr = NULL;
		enqueue(p, &freeList);
	}

	running = &proc[0];
	//proc[NPROC-1].next = &proc[1];
  
	// Setup proc0
	running->next = &proc[1];
	running->pid = 0;
	running->ppid = -1;
	running->status = READY;
	running->priority = 0;

	printf("initialization complete\n"); 
}

int kfork()
{
	PROC * newProc;
	int j;

	if ( freeList == NULL )
	{
		printf("FREE LIST FULL!\n");
		return -1;
	}
	
	newProc = dequeue(&freeList);
	
    for (j=1; j<10; j++)
        newProc->kstack[SSIZE - j] = 0;          // all saved registers = 0

    newProc->kstack[SSIZE-1]=(int)body;          // called tswitch() from body
    newProc->ksp = &(newProc->kstack[SSIZE-9]);        // ksp -> kstack top
	newProc->ppid = running->pid;
	newProc->parentPtr = running;
	newProc->status = READY;
	
	printf( "readyQueue after fetching: %d\n", newProc->pid);
	printQueue( &readyQueue );

	enqueue(newProc, &readyQueue);

	printf( "readyQueue\n ");
	printQueue( &readyQueue );

	return newProc->pid;
}

void wakeup( int evt )
{
	PROC *past, *temp;

	if ( sleepList == NULL )
	{
		return;
	}
	printf("sleepList:");
	printQueue(&sleepList);
	temp = sleepList;
	past = temp;

	// In case any are in front, reassign sleeplist to next proc.
	while( temp != NULL && temp->evt == evt )
	{
		temp->status = READY;
		sleepList = temp->next;
		enqueue( temp, &readyQueue );

		temp = sleepList;
		printf("sleepList:");
		printQueue(&sleepList);
	}
	
	temp = sleepList->next;
	past = temp;
	while(temp != NULL)
	{
		if (temp->evt == evt)
		{
			temp->status = READY;
			enqueue( temp, &readyQueue );
			past->next = temp->next;
		}

		past = temp;
		temp = temp->next;
	}

}

void sleep(int evt)
{
	running->evt = evt;
	running->status = SLEEP;
	enqueue(running, &sleepList);
	tswitch();
}

int wait( int *status )
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
				*status = proc[i].exit;
				proc[i].status = DEAD;
				enqueue(&proc[i], &freeList);
				return i;
			}
		}
		sleep(running);
	}

	return -1;
}

void kexit()
{
	char c;
	int exitval, i;

	running->status = ZOMBIE;
	printf("Enter an exit value:" );
	c = getc();
	exitval = c-'0';
	running->exit = exitval;

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

char *gasp[NPROC] =
{
     "Oh! You are killing me .......",
     "Oh! I am dying ...............", 
     "Oh! I am a goner .............", 
     "Bye! Bye! World...............",      
};

int grave()
{
	printf("\n*****************************************\n"); 
	printf("Task %d %s\n", running->pid,gasp[(running->pid) % 4]);
	printf("*****************************************\n");
	running->status = DEAD;

	tswitch();   /* journey of no return */        
}

int ps()
{
	PROC *p;

	printf("running = %d\n", running->pid);

	p = running;
	p = p->next;
	printf("readyProcs = ");
	while(p != running && p->status==READY)
	{
		printf("%d -> ", p->pid);
		p = p->next;
	}

	printf("\n");
}

int body()
{
	char c;
	int pid, status;
	while(1)
	{
		ps();
		printf("I am Proc %d in body()\n", running->pid);
		printf("Input a char : [s|q|f|w] ");
		c=getc();
		switch(c)
		{
			case 's': tswitch(); break;
			case 'q': kexit(); break;
			case 'f': kfork();   break;
			case 'w':
				pid = wait(&status);
				if (pid < 1) printf("\nNo children to wait for\n");
				else printf("\nChild %d, died with: %d\n", pid, status );
				break;
			default :            break;  
		}
	}
}


main()
{
	printf("\nWelcome to the 460 Multitasking System\n");
	initialize();

	printf( "\nKFORK\n" );


	kfork();
	tswitch();

	printf("P0 resumes: all dead, happy ending\n");
}


int scheduler()
{
	if( running->status == READY )
	{
		enqueue( running, &readyQueue );
	}
	printQueue(&readyQueue);
	running = dequeue( &readyQueue );

    printf("\n-----------------------------\n");
    printf("next running proc = %d\n", running->pid);
    printf("-----------------------------\n");
}
