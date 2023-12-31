#include <xinu.h>
#include <kernel.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef FS
#include <fs.h>

static fsystem_t fsd;
int dev0_numblocks;
int dev0_blocksize;
char *dev0_blocks;

extern int dev0;
int global_inode_creation_id=0;
char block_cache[512];

#define SB_BLK 0 // Superblock
#define BM_BLK 1 // Bitmapblock

#define NUM_FD 16

filetable_t oft[NUM_FD]; // open file table
#define isbadfd(fd) (fd < 0 || fd >= NUM_FD || oft[fd].in.id == EMPTY)

//Number of inodes in a index block
#define INODES_PER_BLOCK (fsd.blocksz / sizeof(inode_t))
//Number of blocks in an inode
#define NUM_INODE_BLOCKS (( (fsd.ninodes % INODES_PER_BLOCK) == 0) ? fsd.ninodes / INODES_PER_BLOCK : (fsd.ninodes / INODES_PER_BLOCK) + 1)
#define FIRST_INODE_BLOCK 2

/**
 * Helper functions
 */
int _fs_fileblock_to_diskblock(int dev, int fd, int fileblock) {
  int diskblock;

  if (fileblock >= INODEDIRECTBLOCKS) {
    errormsg("No indirect block support! (%d >= %d)\n", fileblock, INODEBLOCKS - 2);
    return SYSERR;
  }

  // Get the logical block address
  diskblock = oft[fd].in.blocks[fileblock];

  return diskblock;
}

/**
 * Filesystem functions
 */
int _fs_get_inode_by_num(int dev, int inode_number, inode_t *out) {
  int bl, inn;
  int inode_off;

  if (dev != dev0) {
    errormsg("Unsupported device: %d\n", dev);
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    errormsg("inode %d out of range (> %s)\n", inode_number, fsd.ninodes);
    return SYSERR;
  }

  bl  = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  inode_off = inn * sizeof(inode_t);

  bs_bread(dev0, bl, 0, &block_cache[0], fsd.blocksz);
  memcpy(out, &block_cache[inode_off], sizeof(inode_t));

  return OK;

}

int _fs_put_inode_by_num(int dev, int inode_number, inode_t *in) {
  int bl, inn;

  if (dev != dev0) {
    errormsg("Unsupported device: %d\n", dev);
    return SYSERR;
  }
  if (inode_number > fsd.ninodes) {
    errormsg("inode %d out of range (> %d)\n", inode_number, fsd.ninodes);
    return SYSERR;
  }

  bl = inode_number / INODES_PER_BLOCK;
  inn = inode_number % INODES_PER_BLOCK;
  bl += FIRST_INODE_BLOCK;

  bs_bread(dev0, bl, 0, block_cache, fsd.blocksz);
  // coping nodes of times of offset into block_cache from in
  memcpy(&block_cache[(inn*sizeof(inode_t))], in, sizeof(inode_t));

  bs_bwrite(dev0, bl, 0, block_cache, fsd.blocksz);

  return OK;
}

int fs_mkfs(int dev, int num_inodes) {
  int i;

  if (dev == dev0) {
    fsd.nblocks = dev0_numblocks;
    fsd.blocksz = dev0_blocksize;
  } else {
    errormsg("Unsupported device: %d\n", dev);
    return SYSERR;
  }

  if (num_inodes < 1) {
    fsd.ninodes = DEFAULT_NUM_INODES;
  } else {
    fsd.ninodes = num_inodes;
  }

  i = fsd.nblocks;
  while ( (i % 8) != 0) { i++; }
  fsd.freemaskbytes = i / 8;

  if ((fsd.freemask = getmem(fsd.freemaskbytes)) == (void *) SYSERR) {
    errormsg("fs_mkfs memget failed\n");
    return SYSERR;
  }

  /* zero the free mask */
  for(i = 0; i < fsd.freemaskbytes; i++) {
    fsd.freemask[i] = '\0';
  }

  fsd.inodes_used = 0;

  /* write the fsystem block to SB_BLK, mark block used */
  fs_setmaskbit(SB_BLK);
  bs_bwrite(dev0, SB_BLK, 0, &fsd, sizeof(fsystem_t));

  /* write the free block bitmask in BM_BLK, mark block used */
  fs_setmaskbit(BM_BLK);
  bs_bwrite(dev0, BM_BLK, 0, fsd.freemask, fsd.freemaskbytes);

  // Initialize all inode IDs to EMPTY
  inode_t tmp_in;
  for (i = 0; i < fsd.ninodes; i++) {
    _fs_get_inode_by_num(dev0, i, &tmp_in);
    tmp_in.id = EMPTY;
    _fs_put_inode_by_num(dev0, i, &tmp_in);
  }
  fsd.root_dir.numentries = 0;
  for (i = 0; i < DIRECTORY_SIZE; i++) {
    fsd.root_dir.entry[i].inode_num = EMPTY;
    memset(fsd.root_dir.entry[i].name, 0, FILENAMELEN);
  }

  for (i = 0; i < NUM_FD; i++) {
    oft[i].state     = 0;
    oft[i].fileptr   = 0;
    oft[i].de        = NULL;
    oft[i].in.id     = EMPTY;
    oft[i].in.type   = 0;
    oft[i].in.nlink  = 0;
    oft[i].in.device = 0;
    oft[i].in.size   = 0;
    memset(oft[i].in.blocks, 0, sizeof(oft[i].in.blocks));
    oft[i].flag      = 0;
  }
  // printf("Inodes per  block: %d", INODES_PER_BLOCK);
  // printf("FSD block size: %d", fsd.blocksz); //512
  // printf("Number of nodes: %d", fsd.ninodes); //128
  // printf("Number of blocks: %d", fsd.nblocks); //512
  // printf("Used inodes: %d", fsd.inodes_used); //0
  return OK;
}

int fs_freefs(int dev) {
  if (freemem(fsd.freemask, fsd.freemaskbytes) == SYSERR) {
    return SYSERR;
  }

  return OK;
}

/**
 * Debugging functions
 */
void fs_print_oft(void) {
  int i;

  printf ("\n\033[35moft[]\033[39m\n");
  printf ("%3s  %5s  %7s  %8s  %6s  %5s  %4s  %s\n", "Num", "state", "fileptr", "de", "de.num", "in.id", "flag", "de.name");
  for (i = 0; i < NUM_FD; i++) {
    if (oft[i].de != NULL) printf ("%3d  %5d  %7d  %8d  %6d  %5d  %4d  %s\n", i, oft[i].state, oft[i].fileptr, oft[i].de, oft[i].de->inode_num, oft[i].in.id, oft[i].flag, oft[i].de->name);
  }

  printf ("\n\033[35mfsd.root_dir.entry[] (numentries: %d)\033[39m\n", fsd.root_dir.numentries);
  printf ("%3s  %3s  %s\n", "ID", "id", "filename");
  for (i = 0; i < DIRECTORY_SIZE; i++) {
    if (fsd.root_dir.entry[i].inode_num != EMPTY) printf("%3d  %3d  %s\n", i, fsd.root_dir.entry[i].inode_num, fsd.root_dir.entry[i].name);
  }
  printf("\n");
}

void fs_print_inode(int fd) {
  int i;

  printf("\n\033[35mInode FS=%d\033[39m\n", fd);
  printf("Name:    %s\n", oft[fd].de->name);
  printf("State:   %d\n", oft[fd].state);
  printf("Flag:    %d\n", oft[fd].flag);
  printf("Fileptr: %d\n", oft[fd].fileptr);
  printf("Type:    %d\n", oft[fd].in.type);
  printf("nlink:   %d\n", oft[fd].in.nlink);
  printf("device:  %d\n", oft[fd].in.device);
  printf("size:    %d\n", oft[fd].in.size);
  printf("blocks: ");
  for (i = 0; i < INODEBLOCKS; i++) {
    printf(" %d", oft[fd].in.blocks[i]);
  }
  printf("\n");
  return;
}

void fs_print_fsd(void) {
  int i;

  printf("\033[35mfsystem_t fsd\033[39m\n");
  printf("fsd.nblocks:       %d\n", fsd.nblocks);
  printf("fsd.blocksz:       %d\n", fsd.blocksz);
  printf("fsd.ninodes:       %d\n", fsd.ninodes);
  printf("fsd.inodes_used:   %d\n", fsd.inodes_used);
  printf("fsd.freemaskbytes  %d\n", fsd.freemaskbytes);
  printf("sizeof(inode_t):   %d\n", sizeof(inode_t));
  printf("INODES_PER_BLOCK:  %d\n", INODES_PER_BLOCK);
  printf("NUM_INODE_BLOCKS:  %d\n", NUM_INODE_BLOCKS);

  inode_t tmp_in;
  printf ("\n\033[35mBlocks\033[39m\n");
  printf ("%3s  %3s  %4s  %4s  %3s  %4s\n", "Num", "id", "type", "nlnk", "dev", "size");
  for (i = 0; i < NUM_FD; i++) {
    _fs_get_inode_by_num(dev0, i, &tmp_in);
    if (tmp_in.id != EMPTY) printf("%3d  %3d  %4d  %4d  %3d  %4d\n", i, tmp_in.id, tmp_in.type, tmp_in.nlink, tmp_in.device, tmp_in.size);
  }
  for (i = NUM_FD; i < fsd.ninodes; i++) {
    _fs_get_inode_by_num(dev0, i, &tmp_in);
    if (tmp_in.id != EMPTY) {
      printf("%3d:", i);
      int j;
      for (j = 0; j < 64; j++) {
        printf(" %3d", *(((char *) &tmp_in) + j));
      }
      printf("\n");
    }
  }
  printf("\n");
}

void fs_print_dir(void) {
  int i;

  printf("%22s  %9s  %s\n", "DirectoryEntry", "inode_num", "name");
  for (i = 0; i < DIRECTORY_SIZE; i++) {
    printf("fsd.root_dir.entry[%2d]  %9d  %s\n", i, fsd.root_dir.entry[i].inode_num, fsd.root_dir.entry[i].name);
  }
}

int fs_setmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;
  //0x80>>mbit(mbit=0=>128; if b=0)
  // |= mean addition
  //mbyte=0 (if b=0)
  fsd.freemask[mbyte] |= (0x80 >> mbit);
  return OK;
}

int fs_getmaskbit(int b) {
  int mbyte, mbit;
  mbyte = b / 8;
  mbit = b % 8;

  return( ( (fsd.freemask[mbyte] << mbit) & 0x80 ) >> 7);
}

int fs_clearmaskbit(int b) {
  int mbyte, mbit, invb;
  mbyte = b / 8;
  mbit = b % 8;

  invb = ~(0x80 >> mbit);
  invb &= 0xFF;

  fsd.freemask[mbyte] &= invb;
  return OK;
}

/**
 * This is maybe a little overcomplicated since the lowest-numbered
 * block is indicated in the high-order bit.  Shift the byte by j
 * positions to make the match in bit7 (the 8th bit) and then shift
 * that value 7 times to the low-order bit to print.  Yes, it could be
 * the other way...
 */
void fs_printfreemask(void) { // print block bitmask
  int i, j;

  for (i = 0; i < fsd.freemaskbytes; i++) {
    for (j = 0; j < 8; j++) {
      printf("%d", ((fsd.freemask[i] << j) & 0x80) >> 7);
    }
    printf(" ");
    if ( (i % 8) == 7) {
      printf("\n");
    }
  }
  printf("\n");
}
/**
 * TODO: implement the functions below
 */
int fs_open(char *filename, int flags) 
{
  if (flags > 2 || flags < 2 ||\
   fsd.root_dir.numentries < 1) // file & permission
    return SYSERR;

  int fd = 0;
  while(fd < fsd.ninodes)
  {
    if (fd > fsd.ninodes)
      return SYSERR;
    if(strcmp(fsd.root_dir.entry[fd].name, filename) == 0) // filename match
    {
      if(oft[fd].state == 1)
        return SYSERR;
      oft[fd].state = 1; // file open
      oft[fd].flag = flags; // permission
      oft[fd].de = &fsd.root_dir.entry[fd]; //dir entry-name
      inode_t nd;
      int get_valid_inode=_fs_get_inode_by_num(dev0,\
       fsd.root_dir.entry[fd].inode_num, &nd);
      if (get_valid_inode<0)
        return SYSERR;
      if (nd.id > -1)
        {
          oft[fd].in = nd;
          int update_inode = _fs_put_inode_by_num(dev0,\
           fsd.root_dir.entry[fd].inode_num, &nd); // writing oft
          if (update_inode < 0)
            return SYSERR;
        }
      else 
        return SYSERR;
      return fd;
    }    
    fd++;
  }
  return SYSERR;
}

int fs_close(int fd) {
  
  if (oft[fd].state == 1 && !isbadfd(fd))
  {
    oft[fd].state = 0;
    return OK;
  }
  else if (oft[fd].state == 0 || isbadfd(fd))
    return SYSERR;
}

int fs_create(char *filename, int mode) 
{
  int fd = 0;
  while(fd < fsd.root_dir.numentries)
  {
    if (fsd.inodes_used >= fsd.ninodes || fd >= DIRECTORY_SIZE || \
    fsd.root_dir.numentries >= DIRECTORY_SIZE ||\
    strcmp(filename, fsd.root_dir.entry[fd].name) == 0 ||\
    fd >= fsd.root_dir.numentries || mode != 11)
    {
      return SYSERR;
    }
    fd++;
  }
  if(fsd.inodes_used < fsd.ninodes &&  fd == fsd.root_dir.numentries &&\
   mode == 11 && fsd.root_dir.numentries < DIRECTORY_SIZE )
  { 
    inode_t nd;
    int get_inode = _fs_get_inode_by_num(dev0, global_inode_creation_id, &nd);
    if(get_inode < 0)
      return SYSERR;
    fsd.root_dir.entry[fd].inode_num = global_inode_creation_id;//update inode id
    if(nd.id == -1 && nd.nlink == 0 && nd.type == 0)
    {
      nd.id = global_inode_creation_id;
      nd.type = 1; // file-type
      nd.nlink = 1; // block linking
      int update_inode = _fs_put_inode_by_num(dev0, global_inode_creation_id, &nd); // writing oft
      if (update_inode < 0)
        return SYSERR; 
      fsd.inodes_used = global_inode_creation_id++ + 1; // new creations increment
      strcpy(fsd.root_dir.entry[fd].name, filename); // updating file-name
      fsd.root_dir.numentries = ++fd;
      return fs_open(filename, 2);
    }
    else
      return SYSERR;
  }
  else
    return SYSERR;
}

int fs_seek(int fd, int offset) {

  if (isbadfd(fd) || offset > oft[fd].in.size ||\
   oft[fd].in.type != 1 || oft[fd].state != FSTATE_OPEN ||\
   oft[fd].flag < 2 || oft[fd].flag > 2 ||\
   offset < 0)
    return SYSERR;

  oft[fd].fileptr = offset;
  if(oft[fd].fileptr > oft[fd].in.size || oft[fd].fileptr < 0)
    {
      oft[fd].fileptr -= offset;
      return SYSERR;
    }
  else
    return OK;
}

int fs_read(int fd, void *buf, int nbytes) {
    
  if(oft[fd].state == FSTATE_CLOSED || isbadfd(fd) || nbytes == 0 ||\
  oft[fd].flag == O_WRONLY || fd > fsd.root_dir.numentries)
    return SYSERR;

  int temp_bytes = 0;
  int inodeblocks_index = oft[fd].fileptr/fsd.blocksz;
  int current_datablock = 0;

  if(inodeblocks_index > -1)
  {    
    if(oft[fd].in.blocks[inodeblocks_index] < fsd.blocksz)
    {
      while(nbytes > 0 && oft[fd].in.blocks[inodeblocks_index] <= 511 \
      && oft[fd].fileptr <= oft[fd].in.size) 
      {
        int inodeblock_offset = oft[fd].fileptr % fsd.blocksz;
        int to_read_datablock = fsd.blocksz - (oft[fd].fileptr % fsd.blocksz);
    
        if((oft[fd].fileptr/fsd.blocksz) > inodeblocks_index)
        {
          inodeblock_offset+=1;
          if(inodeblock_offset==11)
          {
            return temp_bytes;
          }
        }
        else if(nbytes>0)
        {
          bs_bread(dev0, inodeblocks_index,inodeblock_offset,buf,to_read_datablock);
          oft[fd].fileptr += to_read_datablock;
          buf += to_read_datablock;
          nbytes -= to_read_datablock;
          temp_bytes += to_read_datablock;
        }
        else if(nbytes<=-2)
        {
          // printf("Entered nbytes < datablock_size_to_read\n");
          int fileptr_offset=oft[fd].fileptr%fsd.blocksz;
          bs_bread(dev0, current_datablock, fileptr_offset, buf, nbytes);
          oft[fd].fileptr+=nbytes;
          // read_until_fileptr+=nbytes;
          // oft[fd].in.size+=nbytes;
          buf+=nbytes;
          // oft[fd].in.blocks[current_datablock]+=nbytes;
          temp_bytes+=nbytes;
          nbytes-=nbytes;
          return temp_bytes;
        }
      
      } 
    }
  }
  return temp_bytes;
}

int fs_write(int fd, void *buf, int nbytes) {
  if(oft[fd].state == 0 || oft[fd].flag != O_RDWR ||\
   oft[fd].flag != O_WRONLY || nbytes == 0 || isbadfd(fd) ||\
   fd > fsd.root_dir.numentries)
    return SYSERR;

  int temp_bytes = 0;
  int current_datablock = oft[fd].fileptr/fsd.blocksz;
  int inodeblocks_index = -1;
  if (oft[fd].in.size <= 0)
    inodeblocks_index = oft[fd].in.size/fsd.blocksz;
  else
    {
      inodeblocks_index = oft[fd].in.size/fsd.blocksz;
      ++inodeblocks_index;
    }

  int next_datablock = -1;

  while(nbytes > 0)
  {
    if (current_datablock >= INODEDIRECTBLOCKS ||\
    oft[fd].in.blocks[current_datablock] >= fsd.blocksz)
      return SYSERR;
    if (inodeblocks_index <= 10 && inodeblocks_index >= 1)
      next_datablock = oft[fd].in.blocks[inodeblocks_index];
    else
    {
      int free_datablock = 18;
      while(free_datablock < fsd.nblocks)
      {
        
        if (fs_getmaskbit(free_datablock) == 0)
          {
            fs_setmaskbit(free_datablock);
            next_datablock = free_datablock;
            if(inodeblocks_index != -1)
              oft[fd].in.blocks[inodeblocks_index] = next_datablock;
          }
        ++free_datablock;
      }
    }
    int datablock_offset = oft[fd].fileptr % fsd.blocksz;
    int datablock_size_left = fsd.blocksz - (oft[fd].fileptr % fsd.blocksz);
    if(datablock_size_left < -1)
    {
      current_datablock+=1;
      if(current_datablock == 10)
      {
        return temp_bytes;
      }
    }
    else if(nbytes<=datablock_size_left)
    {
      bs_bwrite(dev0, next_datablock, datablock_offset,\
        buf,nbytes);
      oft[fd].fileptr+=nbytes;
      oft[fd].in.size+=nbytes;
      buf+=nbytes;
      temp_bytes+=nbytes;
      nbytes-=nbytes;
      int update_inode=_fs_put_inode_by_num(dev0, oft[fd].in.id, &oft[fd].in);
      if (update_inode<0)
        return SYSERR;
      return temp_bytes;
    }
    else if(nbytes>datablock_size_left)
    {
      bs_bwrite(dev0, next_datablock, datablock_offset,\
        buf, datablock_size_left);
      oft[fd].fileptr += datablock_size_left;
      oft[fd].in.size += datablock_size_left;
      buf += datablock_size_left;
      nbytes -= datablock_size_left;
      temp_bytes += datablock_size_left;
      int update_inode = _fs_put_inode_by_num(dev0, oft[fd].in.id, &oft[fd].in);
      if (update_inode<0)
        return SYSERR;
    }
  } 
  return SYSERR;
}

int fs_link(char *src_filename, char* dst_filename) {
  return SYSERR;
}

int fs_unlink(char *filename) {
  return SYSERR;
}

#endif /* FS */