PROC * get_proc(PROC **list){
  return (dequeue(list));
}
PROC * getproc(PROC **list){
  return  get_proc(list);
}
int goUmode();

int kexec(char filepath)
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



PROC * kfork(char * filename){
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
  p->pri  = 1;                 // all of the same priority 1

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


  enqueue(p, &readyQueue);
 
  // make Umode image by loading /bin/u1 into segment
  segment = (p->pid + 1)*0x2000;
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

		put_word(word, segment, 0x2000-i*2);  // stack starts at highest end of segment
	}
	
    p->uss = segment;
    p->usp = 0x2000 - 24; // usp is byte address, x2

  printf("Proc%d forked a child %d segment=%x\n", running->pid,p->pid,segment);
  return(p->pid);
}

int fork()
{
	PROC *p;
	int  i, child;
	u16  segment;
    u16 wurd;

	int from_segment = (running->pid + 1) * 0x2000;

	p = get_proc(&freeList);

	if ( p == 0 )
	{
		return -1;
	}

	p->status = READY;
	p->ppid = running->pid;
	p->pri = running->pri;

	for ( i=1; i<10; i++ )
	{
		p->kstack[SSIZE-i] = 0;
	}

	p->ksp = &(p->kstack[SSIZE - 9]);

	p->kstack[SSIZE-1] = (int)goUmode;

	enqueue(p, &readyQueue);
	segment = (p->pid + 1) * 0x2000;

	for(i = 0; i < 0x2000; i++) 
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
			case 10: put_word(0, segment, 0x2000-i*2); continue;
			default:  wurd = 0;       break;  // pretty much everything else
		}

		put_word(wurd, segment, 0x2000-i*2);  // stack starts at highest end of segment
	}

	p->uss = segment;
	p->usp = 0x2000-24;

	put_word(0, segment, running->usp + 8*2);

	printf("[%d] forked [%d]: %x\n", running->pid, p->pid, segment);

	return p->pid;
}



int printQueue(PROC * list){
  PROC * temp = list;

  while(temp!=0){
    printf("[ %d]-> ", temp->pid);
    temp = temp->next;
  }

  printf("NULL\n");
}