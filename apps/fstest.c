#include <xinu.h>
#include <fs.h>

#ifdef FS

/**
 * TEST
 * MACRO to run a function and print whether it was successful or not
 */
#define TEST(function)                                         \
  if (nargs == 1 || strcmp(args[1], #function) == 0) {         \
    if ((function)() != SYSERR) {                              \
      printf("%-35s: [\033[32mPASS\033[39m]\n", #function);    \
    } else {                                                   \
      printf("%-35s: [\033[31mFAIL\033[39m]\n", #function);    \
    }                                                          \
  }


/* #define FSTEST_DEBUG */

/**
 * MACROs for unit testing
 */

/**
 * ASSERT_TEST
 * Run a function and report the return code
 */
#define ASSERT_TEST(function)                             printf("\033[33mTEST  %20s:%-3d %30s:\033[39m %d\n", __FILE__, __LINE__, #function, (function));

#ifdef FSTEST_DEBUG

#define ASSERT_TRUE(expr)     if (!(expr))              { printf("\033[31mERROR %20s:%-3d %30s()\033[39m '%s' != TRUE\n", __FILE__, __LINE__, __func__, #expr);       return SYSERR; }
#define ASSERT_PASS(function) if ((function) == SYSERR) { printf("\033[31mERROR %20s:%-3d %30s()\033[39m '%s' == SYSERR\n", __FILE__, __LINE__, __func__, #function); return SYSERR; }
#define ASSERT_FAIL(function) if ((function) != SYSERR) { printf("\033[31mERROR %20s:%-3d %30s()\033[39m '%s' != SYSERR\n", __FILE__, __LINE__, __func__, #function); return SYSERR; }

#else

/**
 * ASSERT_TRUE
 * Evaluate an expression and return SYSERR if false
 */
#define ASSERT_TRUE(expr)     if (!(expr))              { return SYSERR; }

/**
 * ASSERT_PASS
 * Run a function and return SYSERR if its return code is SYSERR
 */
#define ASSERT_PASS(function) if ((function) == SYSERR) { return SYSERR; }

/**
 * ASSERT_FAIL
 * Run a function and return SYSERR if its return code is not SYSERR
 */
#define ASSERT_FAIL(function) if ((function) != SYSERR) { return SYSERR; }

#endif


int fstest_testbitmask(void) {

  bs_mkdev(0, MDEV_BLOCK_SIZE, MDEV_NUM_BLOCKS);
  fs_mkfs(0, DEFAULT_NUM_INODES);

  fs_setmaskbit(31);
  fs_setmaskbit(95);
  fs_setmaskbit(159);
  fs_setmaskbit(223);
  fs_setmaskbit(287);
  fs_setmaskbit(351);
  fs_setmaskbit(415);
  fs_setmaskbit(479);
  fs_setmaskbit(90);
  fs_setmaskbit(154);
  fs_setmaskbit(218);
  fs_setmaskbit(282);
  fs_setmaskbit(346);
  fs_setmaskbit(347);
  fs_setmaskbit(348);
  fs_setmaskbit(349);
  fs_setmaskbit(350);
  fs_setmaskbit(100);
  fs_setmaskbit(164);
  fs_setmaskbit(228);
  fs_setmaskbit(292);
  fs_setmaskbit(356);
  fs_setmaskbit(355);
  fs_setmaskbit(354);
  fs_setmaskbit(353);
  fs_setmaskbit(352);

  fs_printfreemask();

  fs_clearmaskbit(31);
  fs_clearmaskbit(95);
  fs_clearmaskbit(159);
  fs_clearmaskbit(223);
  fs_clearmaskbit(287);
  fs_clearmaskbit(351);
  fs_clearmaskbit(415);
  fs_clearmaskbit(479);
  fs_clearmaskbit(90);
  fs_clearmaskbit(154);
  fs_clearmaskbit(218);
  fs_clearmaskbit(282);
  fs_clearmaskbit(346);
  fs_clearmaskbit(347);
  fs_clearmaskbit(348);
  fs_clearmaskbit(349);
  fs_clearmaskbit(350);
  fs_clearmaskbit(100);
  fs_clearmaskbit(164);
  fs_clearmaskbit(228);
  fs_clearmaskbit(292);
  fs_clearmaskbit(356);
  fs_clearmaskbit(355);
  fs_clearmaskbit(354);
  fs_clearmaskbit(353);
  fs_clearmaskbit(352);

  fs_printfreemask();

  fs_freefs(0);
  bs_freedev(0);

  return OK;
}

/**
 * Try to
 * - (re-)create and free the block device,
 * - (re-)create and free the file system, and
 * - create your first file "test"
 */
int fstest_mkdev() {

  int i;

  for (i = 0; i < 10; i++) {
    ASSERT_PASS(bs_mkdev(0, MDEV_BLOCK_SIZE, MDEV_NUM_BLOCKS))
    ASSERT_PASS(fs_mkfs(0, DEFAULT_NUM_INODES))
    ASSERT_PASS(fs_create("test", O_CREAT))
    ASSERT_PASS(fs_freefs(0))
    ASSERT_PASS(bs_freedev(0))
  }
  int fd1 = fs_create("kannan", O_CREAT);
  fs_write(fd1, "ab", 2);
  fs_close(fd1);
  fs_print_inode(fd1);
  // ASSERT_PASS(fs_create("sharrath", O_CREAT))
  return OK;
}
#endif


int fstest(int nargs, char *args[]) {

  /* Output help, if '--help' argument was supplied */
  if (nargs == 2 && strncmp(args[1], "--help", 7) == 0) {
    printf("Usage: %s [TEST]\n\n", args[0]);
    printf("Description:\n");
    printf("\tFilesystem Test\n");
    printf("Options:\n");
    printf("\t--help\tdisplay this help and exit\n");
    return OK;
  }

  /* Check for correct number of arguments */
  if (nargs > 2) {
    fprintf(stderr, "%s: too many arguments\n", args[0]);
    fprintf(stderr, "Try '%s --help' for more information\n",
            args[0]);
    return SYSERR;
  }

#ifdef FS

  printf("\n\n\n");
  TEST(fstest_testbitmask)
  TEST(fstest_mkdev)
  
  // fs_print_dir();
  // fs_print_fsd();
  // fs_print_oft();
  // int fd=fs_create("kannan.txt", O_CREAT);
  // int a=65;
  // int bytes_written=fs_write(fd,(char)a,1);
  // printf("FD Bytes written: %d\n", bytes_written);
  // char *to_read=getmem(10*sizeof(char));
  // // for(int i=0;i<10;i++)
  // //   printf("To_read[%d]: %c\n",i,to_read[i]);
  // fs_seek(fd, 0);
  // int read_fs=fs_read(fd,to_read,1);
  // printf("FD Bytes read: %d\n", read_fs);
  // printf("FD Chars read: %c\n", (char)to_read);
  // // fs_print_inode(fd);
  // fs_close(fd);
  // fs_print_inode(fd);
  // int fd2=fs_create("test.txt", O_CREAT);
  // bytes_written=fs_write(fd2,"abcdefghijk",11);
  // printf("FD2 Bytes written: %d\n", bytes_written);
  // // fs_seek(fd2,2);
  // read_fs=fs_read(fd2,to_read,4);
  // printf("FD2 Bytes read: %d\n", read_fs);
  // for(int i=0;i<4;i++)
  //   printf("FD2 Chars read: %d\n", to_read[i]);

  // int fd1=fs_create("kannan.txt", O_CREAT);
  // fs_close(fd);
  // fs_print_fsd();
  // fs_print_oft();
  // fs_print_inode(fd);
  // fs_close(fd);
  // fs_print_inode(fd);
  // fs_print_inode(fd);
  // fs_print_inode(fd1);
  // fs_freefs(0);
  // fs_print_fsd();
  // fs_print_oft();
  // int fd1=fs_create("kannan.txt", O_CREAT);

  // // fs_close(fd);
  // fs_open("kannan.txt", O_RDONLY);
  // int fd2=fs_create("kannan1.txt", O_CREAT);
  // fs_close(fd2);
  // fs_close(fd2);
  // fs_open("kannan1.txt", O_RDONLY);

#else
  printf("No filesystem support\n");
#endif

  return OK;
}