#include "ucode.c"



main(int argc, char* argv[])
{
	char tty[64];
	char c, input;
	int i, colCount, rowCount;
	int fd;
	
	if (argc == 1)
	{
		fd = dup(0);
		
		gettty(tty);
		close(0);
		open(tty, READ);
	}
	else
	{
		fd =  open(argv[1], READ);
	}

	printf("fd: %d\n", fd);

	colCount = 0;
	rowCount = 0;
	while (read(fd, &c, 1))
	{
		if (colCount >= 80)
			printf("\r\n");
		if ( c== '\n' || colCount >= 80)
		{
			colCount = 0;
			rowCount++;
		}

		printf("%c", c);
		colCount++;

		if (rowCount >= 23)
		{
			input = getc();
			if (input == ' ')
			{
				rowCount = 0;
			}
			else if (input == '\r')
			{
				rowCount--;
			}
		}
	}
	close(fd);
}