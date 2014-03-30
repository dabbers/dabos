#include "ucode.c"

main(int argc, char* argv[])
{
	char c;
	int source, destination;

	if (argc != 3)
	{
		printf("Usage: cp file1 file2\n");
		return;
	}

	if (strcmp(argv[1], argv[2]) == 0)
	{
		printf("Source and destination are the same\n");
		return;
	}

	source = open(argv[1], READ);
	if (source == -1)
	{
		printf("Source does not exist\n");
		return;
	}

	creat(argv[2]);
	destination = open(argv[2], WRITE);

	while(read(source, &c, 1))
		write(destination, &c, 1);
	

}