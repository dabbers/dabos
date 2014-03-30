/*
David Barajas 11329861
*/

// ucode.c file
char *cmd[]={"getpid", "ps", "chname", "kmode", "switch", "wait", "exit", 
             "fork", "exec", "pipe", "pfd", "read", "write", "close", 0};


int show_menu()
{
   printf("******************** Menu ***************************\n");
   printf("*  ps  chname  kmode  switch  wait  exit  fork  exec *\n");
          //   1     2      3       4      5     6    7     8 
   printf("*  pipe  pfd   read   write   close                 *\n");
	  //   9     10    11      12     13    
   printf("*****************************************************\n");

}

int find_cmd(name) char *name;
{
   int i=0;   
   char *p=cmd[0];

   while (p){
         if (strcmp(p, name)==0)
            return i;
         i++;  
         p = cmd[i];
   } 
   return(-1);
}

int getpid()
{
   return syscall(0,0,0);
}

int ps()
{
   syscall(1, 0, 0);
}

int chname()
{
    char s[64];
    printf("input new name : ");
    gets(s);
    syscall(2, s, 0);
}

int kmode()
{
    printf("kmode : syscall #3 to enter Kmode\n");
    printf("proc %d going K mode ....\n", getpid());
    syscall(3, 0, 0);
    printf("proc %d back from Kernel\n", getpid());
}    

int kswitch()
{
    printf("proc %d enter Kernel to switch proc\n", getpid());
    syscall(4,0,0);
    printf("proc %d back from Kernel\n", getpid());
}

int wait()
{
    int child, exitValue;
    printf("proc %d enter Kernel to wait for a child to die\n", getpid());
    child = syscall(5, &exitValue, 0);
    printf("proc %d back from wait, dead child=%d", getpid(), child);
    if (child>=0)
        printf("exitValue=%d", exitValue);
    printf("\n"); 
} 

int fork()
{
  int r;
  printf("enter kernel to ufork...\n");
  r = syscall(7, 0, 0);
}

int exec()
{
  char s[128];
  printf("\nenter filename:");
  gets(s);
  syscall(8,s,0);
  // no return, image changed
}

int exit()
{
   int exitValue;
   printf("\nenter an exitValue (0-9) : ");
   exitValue=(getc()&0x7F) - '0';
   printf("enter kernel to die with exitValue=%d\n", exitValue);
   _exit(exitValue);
}

int _exit(exitValue) int exitValue;
{
  syscall(6,exitValue,0);
}

int pd[2];

int pipe()
{
   printf("pipe syscall\n");
   syscall(30, pd, 0);
   printf("proc %d created a pipe with fd = %d %d\n", 
           getpid(), pd[0], pd[1]);
}

int pfd()
{
  syscall(34,0,0,0);
}
  
int read_pipe()
{
  char fds[32], buf[1024]; 
  int fd, n, nbytes;
  pfd();
  
  
  printf("read : enter fd: ");
  gets(fds);
  fd = atoi(fds);
  
  printf("read : enter # bytes: ");
  gets(buf);
  nbytes = atoi(buf);

  printf("fd=%d  nbytes=%d\n", fd, nbytes); 

  n = syscall(31, fd, buf, nbytes);

  if (n>=0){
     printf("proc %d back to Umode, read %d bytes from pipe : ",
             getpid(), n);
     buf[n]=0;
	 for(nbytes = 0; nbytes < n; nbytes++)
	 {
		 printf("'%c'", buf[nbytes]);
	 }
     printf("%s\n", buf);
  }
  else
    printf("read pipe failed\n");
}

int write_pipe()
{
  char fds[16], buf[1024]; 
  int fd, n, nbytes;
  char* ptr;
  pfd();
  
  printf("write : enter fd: ");
  gets(fds);
  fd = atoi(fds);
  
  printf("write : enter text: ");
  gets(buf);

  nbytes = strlen(buf);

  printf("fd=%d nbytes=%d : %s\n", fd, nbytes, buf);

  n = syscall(32,fd,buf,nbytes);

  if (n>=0){
     printf("\nproc %d back to Umode, wrote %d bytes to pipe\n", getpid(),n);
  }
  else
    printf("write pipe failed\n");
}

int close_pipe()
{
  char s[16]; 
  int fd;
  printf("enter fd to close : ");
  gets(s);
  fd = atoi(s);
  syscall(33, fd);
}


int invalid(name) char *name;
{
    printf("Invalid command : %s\n", name);
}
