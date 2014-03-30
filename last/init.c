
int pid, status;
int stdin, stdout;
char* consoles[] = { "login /dev/tty0", "login /dev/ttyS0" };
int children[] = { 0, 0 };
#define CONSOLES 2
#include "ucode.c"

main(int argc, char *argv[])
{
	int count = 0;
	//1. // open /dev/tty0 as 0 (READ) and 1 (WRTIE) in order to display messages
	open("/dev/tty0", O_RDONLY);
	open("/dev/tty0", WRITE);
	printf("1");

	//2. // Now we can use printf, which calls putc(), which writes to stdout
    printf("KCINIT : fork a login task on console\n"); 
    
	for (count = 0; count < CONSOLES; count++)
	{
		children[count] = fork();

		if (!children[count])
		{
			login(consoles[count]);
		}
	}
	
	parent();
} 

login(char* toexec)
{
	exec(toexec);
}
    
int parent()
{
	int count = 0;
  while(1){
    printf("KCINIT : waiting .....\n");

    pid = wait(&status);
	for( count = 0; count < CONSOLES; count++)
	{
		if (pid == children[count])
		{
			children[count] = fork();
			if (!children[count])
			{
				login(consoles[count]);
			}
		}
	}
  }
}