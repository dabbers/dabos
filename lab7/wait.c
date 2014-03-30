
PROC * dequeue(PROC **queue) 
{

	PROC* p = *queue;
	if (*queue != NULL)
	*queue = (*queue)->next;

	p->next = NULL;
	return p;
}

void enqueue(PROC *p, PROC **queue)
{
	PROC* past;
	PROC* temp; 

	past = *queue;
	if ( past == NULL || p->pri > past->pri )
	{
		p->next = past;
		*queue = p;
		return;
	}


	temp = past->next;

	while(temp != NULL && temp->pri >= p->pri )
	{
		past = past->next;
		temp = temp->next;
	}

	past->next = p;
	p->next = temp;
 }



int ksleep(int event)
{
	PROC * q;

	running->event = event;
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

/* wake up ALL procs sleeping on event */
int kwakeup(int event)
{
  PROC *p, *q; int i;

  p = q = sleepList;

  while( p )
  {
    if ( p->event != event )
	{
      q = p;
      p = p->next;
      continue;
    }
 
    if ( p==sleepList )
	{
		 sleepList = p->next;
         p->status = READY;
         p->event = event;
         enqueue(&readyQueue, p);
         p = q = sleepList;
         continue;
    }

    q->next = p->next;
    p->event = event;
    p->status = READY;
    enqueue( &readyQueue, p );
    p = q->next;
  }   
}


int kexit(int value)
{
	char c;
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

	running->exitCode = value;

	for ( i = 0; i < NPROC; i++ )
	{
		if (proc[i].ppid == running->pid)
		{
			proc[i].ppid = 1;
		}
	}

	kwakeup(&proc[running->ppid]);
	tswitch();
}


int kwait(status) int *status;
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
				proc[i].parent = 0;
				enqueue(&proc[i], &freeList);
				return i;
			}
		}
		ksleep(running);
	}

	return -1;
}
