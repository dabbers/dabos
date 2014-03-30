/* David Barajas 11329861 */

/* io.c */
typedef unsigned int u16;
typedef unsigned long u32;

char *ctable = "0123456789ABCDEF";
u16  BASE = 10;

void putc(char c)
{
	printf("%c", c);
}
void rpu(u16 x)
{
    char c;
    if (x){ 
        c = ctable[x % BASE]; 
        rpu(x / BASE);
         putc(c);
    }
}

int printu(u16 x)
{   
    if (x==0) 
         putc('0');
    else 
        rpu(x);
     putc(' ');
} 

int printd(int x)
{   
	if (x < 0 )
	{
		putc('-');
		x *= -1;
		rpu(x);
	}
	else if (x==0) 
         putc('0');
    else 
        rpu(x);
     putc(' ');
} 


int printl(u32 x)
{   
	if (x==0) 
         putc('0');
    else 
        rpu(x);
     putc(' ');
} 
void prints(char *s)
{
  while(*s)
    putc(*s++);
}

/*
void printf(char* fmt, ...)
{
	char *cp = fmt;
	u16 *ip = (u16 *)&fmt + 1;
	u32 *up;

	while( *cp != '\0' )
	{
		if (*cp == '%')
		{
			cp++;

			switch( *cp )
			{
			case 'c':
				putc((char)*ip);
				ip += 1;
				break;

			case 's':
				prints((char*)*ip);
				ip += 1;
				break;
			case 'x':
				prints("0x");
				BASE = 16;
				printu(*ip);
				ip += 1;
				break;
			case 'd':
				BASE = 10;
				printd((int)*ip);
				ip += 1;
				break;
			case 'u':
				BASE = 10;
				printu(*ip);
				ip += 1;
				break;
			case 'l':
				BASE = 10;
				printu(*ip);
				ip += 2;
				break;
			}
		}
		else
		{
			putc(*cp);
		}

		cp++;
	}

}*/