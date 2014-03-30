#include "ucode.c"
char *tty;

main(int argc, char *argv[])   // invoked by exec("login /dev/ttyxx")
{
	char username[64];
	char password[64];
	char file[1024];
	char *tokenize;
	int fd, i, uid, gid;
	tty =  argv[1];
	
	close(0); close(1); close(2); // login process may run on different terms
	
	// open its own tty (passed in by INIT) as stdin, stdout, stderr
	open(tty, READ);
	open(tty, WRITE);
	open(tty, WRITE);

	fixtty(tty);   // store tty string in PROC.tty[] for putc()

	// NOW we can use printf, which calls putc() to our tty
	printf("DABLOGIN : open %s as stdin, stdout, stderr\n", tty);

	signal(2,1);  // ignore Control-C interrupts so that 
	              // Control-C KILLs other procs on this tty but not the main sh

	while(1)
	{
		printf("login: ");
		gets(username);
		printf("Login: %s\n", username);

		printf("pass: ");
		gets(password);
		
		fd = open("/etc/passwd", READ);
		read(fd, file, 1024);

		tokenize = strtok(file, ":\n");
		printf("tok: %s\n", tokenize);
		printf("U Entered: %s, P entered: '%s'\n", username, password );
		while (tokenize != 0)
		{
			printf("tok: %s\n", tokenize);
			if (strcmp(username, tokenize ) != 0 )
			{
				for(i = 0; i < 6; i++ )
				{
					tokenize = strtok(0, ":\n");
				}
			}
			else
			{
				tokenize = strtok(0, ":\n");
				
				printf("pwtok: '%s'\n", tokenize);
				if (strcmp(password, tokenize) == 0 )
				{
					// Password check here
					tokenize = strtok(0, ":\n");
					printf("uid: %s\n", tokenize);
					uid = atoi(tokenize);
					tokenize = strtok(0, ":\n");
					printf("gid: %s\n", tokenize);
					gid = atoi(tokenize);

					chuid(uid, gid);
					tokenize = strtok(0, ":\n");
					tokenize = strtok(0, ":\n");
					printf("chdir: %s\n", tokenize);
					chdir(tokenize);
					tokenize = strtok(0, ":\n");
					printf("exec: %s\n", tokenize);
					exec(tokenize);
				}
			
				for(i = 0; i < 5; i++ )
				{
					tokenize = strtok(0, ":\n");
				}
			}
		
			tokenize = strtok(0, ":\n");
		}
	/*5. verify user name and passwd from /etc/passwd file

		6. if (user account valid){
		setuid to user uid.
		chdir to user HOME directory.
		exec to the program in users's account
		
		}
		*/
		printf("login failed, try again\n");
	}
}
