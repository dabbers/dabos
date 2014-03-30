typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

typedef struct 
{
	u16	i_mode;		/* File mode */
	u16	i_uid;		/* Owner Uid */
	u32	i_size;		/* Size in bytes */
	u32	i_atime;	/* Access time */
	u32	i_ctime;	/* Creation time */
	u32	i_mtime;	/* Modification time */
	u32	i_dtime;	/* Deletion Time */
	u16	i_gid;		/* Group Id */
	u16	i_links_count;	/* Links count */
	u32	i_blocks;	/* Blocks count */
	u32	i_flags;	/* File flags */
    u32 dummy;
	u32	i_block[15];    /* Pointers to blocks */
    u32 pad[7];         /* inode size MUST be 128 bytes */
} ext2_inode;

typedef struct
{
	u32	inode;			/* Inode number */
	u16	rec_len;		/* Directory entry length */
	u8	name_len;		/* Name length */
	u8	file_type;
	char	name[255];      	/* File name */
} ext2_dir_entry_2;

#define BLOCK_SIZE 1024
#define INODE_BLOCK   5    // First block of inodes
#define ROOT_INDEX    2    // root starts at inode 2

char indirect[1024];    // buffer for indirect block
char filename[255];
char pathstore[255];
char str[255];
char blockBuffer[BLOCK_SIZE];
ext2_inode inodeStore[8];

void my_strcpy( char* dest, char* src)
{
	while(*dest++ = *src++);
}

prints(char *s)
{
  while(*s)
    putc(*s++);
}

my_gets(char *s)
{
  char c;
  while ( (c = getc()) != '\r'){
    *s++ = c;
    putc(c);
  }
  *s = 0;
}

void get_block(int blk, char buf[])
{
	readfd(blk/18, (blk/9)%2, (blk*2)%18, buf);
}

ext2_inode* get_inode( int inodeIndex )
{
	inodeIndex--;
	get_block( (inodeIndex/8) + INODE_BLOCK, &inodeStore );
	return &inodeStore[inodeIndex%8]; //Return pointer to inode in inode store based on i.
}

int search( char* pathname, int inodeIndex )
{
	int i;
	int bytesLeft;
	int dataBlock;
	ext2_inode* inode;
	char *cp;
	char *dirname;
	char *next;
	ext2_dir_entry_2* dir;

	cp = pathstore;

	if ( pathname[0] == '/' ) pathname++;

	// Loop until the next path part
	while( *pathname != '/' && *pathname != '\0' )
	{
		*cp = *pathname;
		cp++;
		pathname++;
	}

	// Add null at end of string so it is a proper string
	*cp = '\0';
	
	inode = get_inode(inodeIndex);

	for( i = 0; i < 13; i++ )
	{
		dataBlock = inode->i_block[i];

		if ( dataBlock == 0) error();

		get_block(dataBlock, &blockBuffer );

		dir = (ext2_dir_entry_2*)blockBuffer;

		bytesLeft = BLOCK_SIZE;
		while( bytesLeft > 0 )
		{
			dirname = dir->name;
			next = pathstore;

			while( *dirname == *next && dir->name_len > 0 )
			{
				if ( *(next+1) == '\0' )
				{
					if ( pathname[0] == '\0' )
					{
						return dir->inode;
					}
					else
					{
						return search(pathname, dir->inode);
					}
				}
				dirname++;
				next++;
			}

			bytesLeft -= dir->rec_len;
			dir = &blockBuffer[BLOCK_SIZE-bytesLeft];
		}

	}

	return 0;
}

main()
{
	ext2_inode* inode;
	int i;
	long* longptr;

	prints("Select a file to boot:\n\r");
	my_gets(filename);

	if ( filename[0] == '\0' )
	{
		my_strcpy( filename, "/boot/mtx" );
	}

	inode = get_inode( search( filename, ROOT_INDEX ) );

	i = inode->i_block[12];

	if ( i != 0 )
	{
		get_block(i, indirect);
	}

	setes(0x1000);

	longptr = inode->i_block;
	for( i = 0; i < 12; i++ )
	{
		if ( longptr[i] == 0 )
		{
			break;
		}

		get_block( longptr[i], 0 );
		inces();
	}

	longptr = indirect;
	while( *longptr != 0 )
	{
		get_block( *longptr, 0 );
		inces();
		longptr++;
	}

	return 1;
}
  
