/* David Barajas 11329861 */

/* pri_queue.c */
void enqueue( PROC *p, PROC **queue )
{
	PROC* past;
	PROC* temp; 

	past = *queue;
	if ( past == NULL || p->priority > past->priority )
	{
		p->next = past;
		*queue = p;
		return;
	}


	temp = past->next;

	while(temp != NULL && temp->priority >= p->priority )
	{
		past = past->next;
		temp = temp->next;
	}

	past->next = p;
	p->next = temp;
}

PROC* dequeue( PROC **queue )
{
	PROC* p = *queue;
	printf("dequeue()\n");
	
	if (*queue != NULL)
	*queue = (*queue)->next;

	p->next = NULL;
	return p;
}

void printQueue( PROC** queue )
{
	PROC* temp = *queue;
	printf("\n");
	while( temp != NULL  )
	{
		printf("[%d] PRI: %d, STATUS: %d, Next: %d, EVT: %x\n", temp->pid, temp->priority, temp->status, (temp->next!=NULL?temp->next->pid:-1), temp->evt );
		temp = temp->next;

		if ( *queue == temp ) break;
	}

}