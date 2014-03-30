#include "ucode.c"

int main(int argc, char* argv[])
{
	int i, j, read_size, fd;
	char c;
	char buf[1024];

	printf("DABCAT\n");


	if (argc == 1)
	{
		while(1)
		{
			c = getc();

			if (c == 255) exit(0);

			switch(c)
			{
				case '\r': putc('\n');  putc('\r');	break;
				default  : putc(c);  	break;
			}
		}
	}
	else 
	{
		for(i = 1; i < argc; i++)
		{
			fd = open(argv[i], READ);
			if (fd == -1) continue;

			while(read_size = read(fd, buf, 1024))
			{
				for(j = 0; j < read_size; j++)
				{
					c = buf[j];
					switch(c)
					{
						case '\n': putc('\n');  putc('\r');	break;
						default  : putc(c);  	break;
					}
				}
			}
			close(fd);
		}
	}
}