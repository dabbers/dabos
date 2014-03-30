/*
David Barajas 11329861
*/

char *MODE[ ]={"READ_PIPE ","WRITE_PIPE"};

show_pipe(PIPE *p)
{
   int i, j;
   printf("------------ PIPE CONTENETS ------------\n");     
   i = p->head;
   j = p->tail;
   
   while(i != j && p->buf[i] != NULL)
   {
	   printf("%c", p->buf[i]);
	   
	   i = ((i+1)%PSIZE);
   }

   printf("\n----------------------------------------\n");
}


int pfd()
{
	int i = 0;

	printf("ID\tTYPE\tMODE\n");
	for( i = 0; i < NFD; i++ )
	{
		if (running->fd[i]->refCount != NULL)
			printf("%d\tPIPE\t%s\n", i, MODE[running->fd[i]->mode] );
	}
}

int read_pipe(int fd, char *buf, int n)
{
	int r = 0;
	PIPE* p;

	if (n<=0) return 0;

	if ( running->fd[fd]->mode == 1 ) return 0;

	p = running->fd[fd]->pipe_ptr;
	show_pipe(p);
	
	printf("N: %d\n", n);

	while(n)
	{
		while(p->data)
		{
			printf("HEAD: %d\n", p->head);
			printf(":%c\n", p->buf[ p->head ] );
			put_word(p->buf[ p->head++ ], running->uss, buf+r);
			p->head = p->head % PSIZE;
			n--; r++; p->data--; p->room++;
			if (n==0) break;              
		}

		if (n==0)
		{                  // has read some data
			wakeup(&p->room);
			return r;
		}

		if (p->nwriter)
		{
			wakeup(&p->room);             // wakeup ALL writers, if any.
			ksleep(&p->data);              // sleep for data
			continue;
		}

		// pipe has no writer and no data
		return 0;
	}
}

int write_pipe(int fd, char *buf, int n)
{
	int r = 0;
	char tmp;

	PIPE* p;
	
	if (n<=0)
	{
		printf("<=0!!\n");
		return 0;
	}

	if ( running->fd[fd]->mode == 0 ) return 0;
	p = running->fd[fd]->pipe_ptr;
	show_pipe(p);
	while (n)
	{
		if (!p->nreader)                 // no more readers
		{
			printf("BROKEN PIPE\n");
			close_pipe(fd);
			return 0;
		}

		while(p->room && n)
		{
			tmp = get_word(running->uss, buf+r);

			p->buf[ p->tail++ ] = tmp;
			printf("'%c'", p->buf[p->tail-1]);
			p->tail = (p->tail % PSIZE);
			r++; p->data++; p->room--; n--;
		}

		printf("R: %d\n", r );
		wakeup(&p->data);             // wakeup ALL readers, if any.
		if (n==0) return r;           // finished writing n bytes 
		// still has data to write but pipe has no room
		ksleep(&p->room);              // sleep for room   
	}
}

int kpipe(int pd[2])
{
	int i = 0;
	int fd_read, fd_write;
	fd_read = fd_write = -1;

  // create a pipe; fill pd[0] pd[1] (in USER mode!!!) with descriptors
	for( i = 0; i < NOFT; i++ )
	{
		// Check for unused OFT entry
		if (oft[i].refCount == 0)
		{
			oft[i].pipe_ptr = NULL;
			oft[i].refCount++;

			if ( fd_read == -1)
			{
				fd_read = i;
				oft[i].mode = 0;
				
			}
			else if( fd_write == -1 )
			{
				fd_write = i;
				oft[i].mode = 1;
			}
			else
				break;
		}
	}

	for (i = 0; i < NPIPE; i++ )
	{
		// FInd free pipe
		if (pipe[i].busy == 0)
		{
			oft[fd_read].pipe_ptr = &pipe[i];
			oft[fd_write].pipe_ptr = &pipe[i];
			pipe[i].nreader++;
			pipe[i].nwriter++;
			pipe[i].busy = 1;
			pipe[i].room = PSIZE;
			pipe[i].head = pipe[i].tail = pipe[i].data = 0;
		}
	}

	for( i = 0; i < NFD; i++ )
	{
		if (running->fd[i] == NULL)
		{
			running->fd[i] = &oft[fd_read];
			fd_read = i;
			break;

		}
	}
	for( i = 0; i < NFD; i++ )
	{
		if (running->fd[i] == NULL)
		{
			running->fd[i] = &oft[fd_write];
			fd_write = i;
			break;

		}
	}

	put_word( fd_read, running->uss, pd);
	put_word( fd_write, running->uss, pd + 1);
}

int close_pipe(int fd)
{
  OFT *op; PIPE *pp;

  printf("proc %d close_pipe: fd=%d\n", running->pid, fd);

  op = running->fd[fd];
  running->fd[fd] = 0;                 // clear fd[fd] entry 

  if (op->mode == READ_PIPE){
      pp = op->pipe_ptr;
      pp->nreader--;                   // dec n reader by 1

      if (--op->refCount == 0){        // last reader
		if (pp->nwriter <= 0){         // no more writers
	     pp->busy = 0;             // free the pipe   
             return;
        }
      }
      wakeup(&pp->room); 
      return;
  }

  if (op->mode == WRITE_PIPE){
      pp = op->pipe_ptr;
      pp->nwriter--;                   // dec nwriter by 1

      if (--op->refCount == 0){        // last writer 
	if (pp->nreader <= 0){         // no more readers 
	    pp->busy = 0;              // free pipe also 
            return;
        }
      }
      wakeup(&pp->data);
      return;
  }
}
