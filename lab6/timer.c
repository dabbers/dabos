/* timer parameters. */
#define LATCH_COUNT     0x00	/* cc00xxxx, c = channel, x = any */
#define SQUARE_WAVE     0x36	/* ccaammmb, a = access, m = mode, b = BCD */

/************************ NOTICE THE DIVISOR VALUE ***********************/
#define TIMER_FREQ   1193182L	/* timer frequency for timer in PC and AT */
#define TIMER_COUNT ((unsigned) (TIMER_FREQ/60)) /* initial value for counter*/

#define TIMER0       0x40
#define TIMER_MODE   0x43
#define TIMER_IRQ       0


int enable_irq(irq_nr) unsigned irq_nr;
{
  lock();
    out_byte(0x21, in_byte(0x21) & ~(1 << irq_nr));

}


/*===========================================================================*
 *				timer_init				     *
 *===========================================================================*/

ushort tick;
int timer_sec, timer_hr, timer_min;

int timer_init()
{
  /* Initialize channel 0 of the 8253A timer to e.g. 60 Hz. */

  printf("timer init\n");
  tick = timer_hr = timer_sec = timer_min = 0;

  out_byte(TIMER_MODE, SQUARE_WAVE);	/* set timer to run continuously */
  out_byte(TIMER0, TIMER_COUNT);	/* load timer low byte */
  out_byte(TIMER0, TIMER_COUNT >> 8);	/* load timer high byte */
  enable_irq(TIMER_IRQ); 
}

/*===========================================================================*
 *				timer_handler				     *
 *===========================================================================*/
int tmpbool = 0;
int fdcount = 5;

int thandler()
{
	int temprow, tempcol;
	PROC* p, *p2, *a, *b;

    tick++; 

    tick %= 60;


    out_byte(0x20, 0x20);  
	
    if (tick % 60 == 0)
	{
		temprow = row;
		tempcol = column;
		row = 24;
		column = 66;
		
		++timer_sec;
		timer_min = (timer_sec / 60);
		timer_hr = (timer_sec / 3600);

	
		if (timer_hr < 10)
		{
			putc('0');
		}
		printf("%d:", timer_hr);

		if (timer_min < 10)
		{
			putc('0');
		}
		printf("%d:", timer_min);
	
		if ((timer_sec%60) < 10)
		{
			putc('0');
		}
		printf("%d", (timer_sec%60));

		row = temprow;
		column = tempcol;
		
		if (fdcount-- <= 0)
		{
			if (!tmpbool)
			{
				out_byte(0x0C, 0x3F2);
			}
			else
			{
				out_byte(0x1C, 0x3F2);
			}

			tmpbool = !tmpbool;
			fdcount = 5;
		}

		// Handle Sleeping Processess
		p = p2 = sleepList;

		// handle edge case of first in line
		if (p && p->time - 1 <= 0)
		{
			printf("Waking up first item %d %x, queue: %d\n", p->pid, p, rqueue[1].priority);
			wakeup(&proc[p->pid]);
			//p = p2 = sleepList = p->next;
			//	p->status = READY;
			a = b = rqueue[1].queue;
			if (rqueue[1].queue == 0)
			{
				rqueue[1].queue = p;
				p->next = 0;
			}
			else
			{
				a = b =rqueue[1].queue;
				while(a)
				{
					b = a;
					a = a->next;
				}

				b->next = p;
				p->next = a;
			}
			printQ();
			printsleep();
			p = sleepList;
		}
		while(p)
		{
			p->time--;
			printf("TIME: %d\n", p->time);
			if (p->time == 0)
			{
				wakeup(p);
				a = b = rqueue[1].queue;
				printf("QUEUE: %x\n", rqueue[1].queue);
				a = b = rqueue[1].queue;
				if (rqueue[1].queue == 0)
				{
					rqueue[1].queue = p;
					p->next = 0;
				}
				else
				{
					a = b =rqueue[1].queue;
					while(a)
					{
						b = a;
						a = a->next;
					}

					b->next = p;
					p->next = a;
				}
			}

			p2 = p;
			p = p->next;

		}

		// Switch if time is up
		if (inkmode == 1)
		{  
			running->time--;  
			if (running->time<=0)
			{
				running->time = 5;

				tswitch();
			}
		}
	}

}





