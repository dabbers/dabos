#include "ucode.c"


char* commands[64];

void split(line) char* line;
{
	int i;
	
	commands[0] = strtok(line, " ");
	
	for(i = 1; commands[i - 1] != 0; i++)
	{
		commands[i] = strtok(0, " ");
	}
}

char shellInput[128];

main(int argc, char* argv[])
{
	int i;
	char* cp;
	int pipes[2];
	char temp[128];

	while(1)
	{
		printf("dabsh # : ");
		gets(shellInput);

		if (shellInput[0] == 0)
			continue;
		if (strcmp("die", shellInput) == 0)
		{
			exit();
		}

		if (shellInput[0] == 'c' && shellInput[1] == 'd')
		{
			chdir(shellInput+3);
			continue;
		}
		if (fork())
		{
			// sh parent thread waiting for children to execute.
			wait();
		}
		else 
		{

			// child

			strcpy(temp, shellInput);
			split(temp);

			cp = shellInput;

			for (i = 0; commands[i] != 0; i++)
			{
				printf("%d", i);
				switch (commands[i][0])
				{
					case '|':
						pipe(pipes);

						if (fork())
						{
							commands[i + 1] = '\0';
							close(pipes[0]);
							close(1);
							dup2(pipes[1], 1);

						}
						else
						{
							cp = shellInput;
							close(pipes[1]);
							close(0);
							dup2(pipes[0], 0);
						}

						break;
					case '>':
						creat(commands[i+1]);
						close(1);
						// open for write or append depending on if we used > or >>
						if (open(commands[i+1], (commands[i][1] == '>' ? APPEND : WRITE)) == -1)
						{
							write(2, "Failed to create file\n", 23);
							return;
						}

						i++;

						break;

					case '<':
						close(0);

						if (open(commands[i+1], 0) == -1)
						{
							printf("Invalid file!\n");
							return;
						}
						i++;
						break;

					default: // Rebuild string from tokenization
						strcpy(cp, commands[i]);
						cp += strlen(commands[i]);
						*(cp++) = ' ';
				}
			}

			*(cp-1) = 0;

			printf("child process %d exec to %s\n", getpid(), commands[0]);

			exec(shellInput);
		}
	}

}