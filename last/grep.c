#include "ucode.c"

main(int argc, char* argv[])
{
	int sourcePos, i, fd;
	char checkLine[2048];
	char c;

	if (argc > 2)
	{
		for(i = 2; i < argc; i++)
		{
			fd = open(argv[i], READ);

			if (fd == -1) continue;

			while(read(fd, &c, 1))
			{
				if (c != '\n' && c != '\r')
				{
					sourcePos++;
					checkLine[sourcePos] = c;
				}
				else
				{
					sourcePos++;
					checkLine[sourcePos] = '\0';

					if (strstr(checkLine, argv[1]) != 0)
					{
						printf("'%s'\n", checkLine);
					}
					sourcePos = -1;
				}	
			}
			close(fd);
			sourcePos = -1;
		}
		return;
	}
	
	sourcePos = -1;
	while(read(0, &c, 1))
	{
		if (c != '\n' && c != '\r')
		{
			sourcePos++;
			checkLine[sourcePos] = c;
		}
		else
		{
			sourcePos++;
			checkLine[sourcePos] = '\0';

			if (strstr(checkLine, argv[1]) != 0)
			{
				printf("'%s'\n", checkLine);
			}
			sourcePos = -1;
		}	
	}
}