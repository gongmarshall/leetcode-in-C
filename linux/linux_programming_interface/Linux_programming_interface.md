# 4. 文件I/O:通用I/O模型

* open()

```c
#include <sys/stat.h>
#include <fcntl.h>

int open(const char *pathname, int flags, ... /* mode_t mode */);
```

要打开的文件有参数pathname来标识，如果是一个符号链接，会对其解引用。打开成功返回文件描述符。

文件访问模式：O_RDONLY, O_WRONLY, O_RDWR。

* open()调用的flags

  | 标志        | 用途                                                         |
  | ----------- | ------------------------------------------------------------ |
  | O_RDONLY    | 只读打开                                                     |
  | O_WRONLY    | 只写打开                                                     |
  | O_RDWR      | 以读写打开                                                   |
  | O_CLOEXEC   | 设置为close-on-exec标志                                      |
  | O_CREAT     | 若文件不存在则创建之                                         |
  | O_DIRECT    | 无缓冲的输入输出                                             |
  | O_DIRECTORY | 如果pathname不是目录，则失败                                 |
  | O_EXCL      | 结合O_CREAT参数使用，专门用于创建文件。                      |
  |             |                                                              |
  |             |                                                              |
  |             |                                                              |
  |             |                                                              |
  |             |                                                              |
  |             |                                                              |
  |             |                                                              |
  |             |                                                              |
  |             |                                                              |
  |             |                                                              |
  | O_TRUNC     | 如果文件已经存在且为普通文件，那么将清空文件内容，将其长度置0.在Linux使用此标志，无论是以读、写方式打开文件，都可以清空文件内容。 |

  对于已打开的文件，这些标志可以使用fcntl()F_GETFL和F_SETFL操作来检索和修改。

* read

  ssize_t read(int fd, void *buffer, size_t count)

  read函数读取的数据若要使用printf打印则需要在读取数据的末尾加上'\0'

  numRead = read(STDIN_FILENO, buffer, MAX_READ);

  buffer[numRead] = '\0';

* write

  ssize write(int fd, void *buffer, size_t count);

  返回为写入文件的字节数，可能小于count，原因可能是磁盘满了，或者进程资源对文件大小的限制。

* lseek()

  ```c
  #include <unistd.h>
  
  off_t lssek(int fd, off_t offset, int whence);
  ```

  whence参数表明参照哪个基点来解释offset参数：

  SEEK_SET

  ​	将文件偏移量设置为从文件头部起始点开始的offset个字节。

  SEEK_CUR

  ​	相当于当前文件偏移量，将文件偏移量调整offset个字节

  SEEK_END

  ​	将文件偏移量设置为起始于文件尾部的offset个字节，从尾部开始算。

  但SEEK_SET时，offset必须为非负数。

  lseek()调用会返回新的文件偏移量。

  lseek(fd, 0, SEEK_CUR)返回当前偏移量。

  不允许lseek()应用于管道，FIFO, socket或者终端。



练习：

4-1. tee命令是从标准输入中读取数据，直至文件结尾，随后将数据写入标准输出和命令行参数所指定的文件。使用I/O系统调用实现tee命令。默认情况下，若已存在于命令行参数指定文件同名的文件，tee命令会将其覆盖。如文件已存在，请实现-a命令行选项（tee -a file）在文件结尾处追加数据。

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int flag=0;
    int i;
    int file_flags=O_CREAT |O_WRONLY;
    char buffer[1024+1];
    int ch = getopt(argc, argv, "a");
    int fd_in;
    int fd_out;
    int nums;
    if(ch == "a")
        flag = 1;
    else
        /* remove the content of the file */
        file_flags |= O_TRUNC;
    /* read data from stdin to buffer */
    while((nums=read(STDIN_FILENO, buffer, 1024))>0)
    {
        /* write buffer data to stdout */
        if(write(STDOUT_FILENO, buffer, nums)!=nums)
        {
            fprintf(stderr, "write stdout error\n");
            return -3;
        }
        /* oprate all files */
        for(i=flag+1; i<argc; i++)
        {
            /* open file with read ability and write ability */
            if((fd_out=open(argv[i], file_flags, S_IRUSR|S_IWUSR))<0)
            {
                fprintf(stderr, "open %s error\n", argv[i]);
                return -2;
            }
            /* write buffer data to file */
            if(flag)
                lseek(fd_out, 0, SEEK_END);
            if(write(fd_out, buffer, nums) < 0)
            {
                fprintf(stderr, "write file error\n");
                close(fd_out);
                return -2;
            }
            close(fd_out);
        }
        
        
    }
    return 0;
}
```



# 5. 深入探究文件I/O

(NFS系统不支持O_APPEND标志)

* 文件控制操作：fcntl()

  ```c
  #include <fcntl.h>
  
  int fcntl(int fd, int cmd, ...);
  ```

  cmd:

  ​	F_GETFL: 	针对一个打开的文件，获取其访问模式和状态标志（open函数中的flag）

  ​	F_SETFL: 	修改打开文件的某些状态。允许更改的标志有O_APPEND, O_NONBLOCK, O_NOATIME, O_ASYNC和O_DIRECT.

   	F_DUPFD： newfd= fcntl(oldfd, F_DUPFD, startfd)

  ​			为oldfd创建一个副本，

  dup(oldfd) 返回一个新的文件描述符，两个文件描述符都指向同一个打开的文件句柄。

  dup2(oldfd, newfd) 首先会将newfd文件描述符关闭，然后再将newfd的文件描述符指向oldfd指向的文件句柄，eg.:

  ​	dup2(fd_file, 2) stderr的文件描述符将会指向fd_file打开的文件句柄，也就说如果再调用fprintf(stderr, "");之类的函数会将字符串输入到fd_file描述符所指向的文件中。

  dup3(int oldfd, int newfd, int flags)在dup2的基础上支持对标志O_CLOEXEC

  

* 在文件特定偏移量处的I/O: pread和pwrite

  ```c
  #include <unistd.h>
  
  ssize_t pread(int fd, void *buf, size_t count, off_t offset);
  
  ssize_t pwrite(int fd, void *buf, size_t count, off_t offset);
  ```

  

  pread()调用等同于将如下纳入同一原子操作：

  ```c
  off_t orig;
  
  orig = lseek(fd, 0, SEEK_CUR);
  lseek(fd, offset, SEEK_SET);
  s = read(fd, buf, len);
  lseek(fd, orig, SEEK_SET);
  ```

* 分散输入和集中输出：readv()和writev()

  ```c
  #include <sys/uio.h>
  
  ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
  
  ssize_t wirtev(int fd, const struct iovec *iov, int iovcnt);
  
  struct iovec {
      void *iov_base; /* start address of buffer */
      size_t iov_len; /* Number of bytes to transfer to/from buffer */
  };
  ```

  这些系统调用并非只对单个缓冲区进行读写操作，而是一次即可传输多个缓冲区的数据。数组iov定义了一组用来传输数据的缓冲区。整型数iovcnt则指定了iov的成员个数。iov中的每个成员都是如下形式

  

* 截断文件：truncate()和ftruncate()

  系统调用将文件大小设置为length参数指定的值
  
* exercises

  2. 用O_APPEND打开后，write操作是一个原子操作，所以每次都会自动把偏移量移到文件末尾，lseek不能任意位置write。但是可以在任意位置开始读。
  3. 没有O_APPEND标志的lseek和write有可能被中断而导致lseek没有把偏移量移到文件尾，那么再写会覆盖已经写了的文件。


# 6 进程

* 环境变量

  extern char **environ;来访问环境列表。

  char *getenv(const char *name)

  可以从进程环境中检索单个值。

  putenv(char *string)

  ​	可以向调用进程的环境中添加一个新变量或修改一个已经存在的变量值。

  ​	string为一指针，指向name=value形式的字符串。

  setenv()可以替代putenv

  ```c
  #include <stdlib.h>
  
  int setenv(const char *name, const char *value, int overwrite)；
   
  int unsetenv(const char *name);
  
  int clearenv(void);
  ```

  让环境变量中已经存在这个变量，overwrite为0，则不添加，若非0则添加。

  unsetenv(const char *name)将移除由name参数标识的变量。

  清除整个环境变量

# 7. 内存分配

## 7.1 在堆上分配内存

| 栈（向下生长）    |
| ----------------- |
| 堆（向上生长）    |
| 未初始化数据(bss) |
| 初始化的数据      |
| 文本（程序代码）  |
|                   |

最初program break正好位于未初始化数据末，program break的位置向上抬后，程序可以访问新分配区域内的任何内存地址。

```c
#include <unistd.h>

int brk(void *end_data_segment);
void *sbrk(intptr_t increment);
```

系统调用brk()会将program break设置为参数end_data_segment所指定的位置。由于虚拟内存以页为单位进行分配，end_data_segment会到下一个边界处。

sbrk()将program break在原有地址上增加参数increment传入的大小。

free函数的过程：

free()一般并不会降低program break的位置，而是将这块内存添加到空闲内存列表中，供后续的malloc()函数循环使用。

malloc函数的过程：

首先扫描free()所释放的空闲内存列表，以寻找尺寸大于或等于要求的一块内存。如果没有则调用sbrk()以分配更多内存.

* 在堆上分配内存的其它方法

  calloc()和realloc()分配内存

  ```c
  #include <stdlib.h>
  
  void *calloc(size_t numitems, size_t size);
  
  void *realloc(void *ptr, size_t size);
  
  ```

  calloc()用于给一组相同对象分配内存，参数numitems指定分配对象的数量，size指定每个对象的大小。calloc()会将已分配的内存初始化为0.

  realloc()函数用来调整（通常是增加）一块内存的大小，而此块内存应是之前由malloc包中函数所分配的。

  realloc()返回指向大小调整后内存块的指针。与调整前的指针相比，二者指向的位置可能不同。

  当增大已分配内存时，realloc()会试图去合并在空闲列表中紧随其后且大小满足要求的内存块。若原内存位于堆的顶部，那么realloc()将对堆空间进行扩展。如果这块内存位于堆的中部，且紧邻的空闲空间大小不足，realloc()会分配一块新内存，并将原有数组复制到新内存块中。

## 7.2 在堆栈上分配内存

```c
#incldue <alloca.h>

void *alloca(size_t size);
```

alloca从堆栈上分配内存，不需要free，也不能使用realloca.

alloca分配内存的速度要快于malloc(),因为编译器将alloca()作为内联代码处理，并直接调用堆栈指针来实现，此外，alloca()也不需要维护空闲内存块列表。

另一个优势是，由alloca()分配的内存随栈帧的移除而自动释放，即调用alloca的函数的返回时。

# 8 时间

* int gettimeofday(struct timeval *tv, sturct timezone *tz)

  tz已废弃，应始终设置为NULL

* time_t time(time_t *timep)

  返回自Epoch以来的秒数，与gettimeofday中tv参数中tv_sec字段数值相同。如果timep参数不为NULL那么还会将自Epoch以来的秒数置于timep所指向的位置。

```c
struct timeval {
    time_t tv_sec; /* Seconds since 00:00:00, 1 jan 1970 UTC */
    suseconds_t tv_usec; /* Additional microseconds (long int) */
}


```

* time_t 转换格式

  * char *ctime(const time_t *timep)

    timestamp-> wed Jun 8 14:22:34 2021格式

* time_t和时间之间的转换

  ```c
  struct tm {
      int tm_sec;
      int tm_min;
      int tm_hour;
      int tm_mday;
      int tm_mon; /* 0-11 */
      int tm_year; /* year since 1900 */
      int tm_wday; /* Day of the week (Sunday = 0) */
      int tm_yday; /* Day in the year (0-365; 1 Jan = 0) */
      int tm_isdst; /* Daylight saving time flag
      				>0: DST is in effect
      				=0: DST is not effect;
      				<0: DST information not available*/
  }
  
  struct tm *gmtime(const time_t *timep);
  struct tm *localtime(const time_t *timep);
  time_t mktime(struct tm *timeptr);
  ```

  gmtime()和localtime()可将time_t值转换为一个所谓的分解时间。localtime()需要考虑时区和夏令时设置，对应系统本地时间的分解时间。

  mktime()将本地时区的分解时间翻译为time_t值

# 系统和进程信息

## 获取与进程有关的信息：/proc/PID



# 文件I/O缓冲

* 设置一个stdio流的缓冲模式

  调用setvbuf()函数，可以控制stdio的使用缓冲形式。

  ```c
  int setvbuf(FILE *stream, char *buf, int inode, size_t size)
  ```

  stream标识将要修改哪个文件流的缓冲。buf和size为缓冲区和大小

* 刷新stdio缓冲区

  ```c
  int fflush(FILE *stream);
  ```

  若stream为NULL，则fflush()将刷新所有的stdio缓冲区

* 用于控制文件I/O内核缓冲的系统调用

  fsync()系统调用将使缓冲数据和与打开文件描述符fd相关的所有元数据都刷新到磁盘上。调用fsync()会强制使文件处于Synchronized I/O file integrity completion状态。

  ```c
  #include <unistd.h>
  
  int fsync(int fd);
  ```

  sync()系统调用会使包含更新文件信息的所有内核缓冲区（即数据块、指针块、元数据等）刷新到磁盘上。

  ```c
  #include <unistd.h>
  
  void sync(void);
  ```

* 使所有写入同步：O_SYNC

  调用open()函数时如指定O_SYNC标志，则会使所有后续输出同步。

  fd=open(pathname, O_WRONLY|O_SYNC)

  调用open()后，每个write()调用会自动将文件数据和元数据刷新到磁盘上

* 绕过缓冲区高速缓存：直接I/O

  open()打开文件或设备时指定O_DIRECT标志

# 文件系统

* 文件系统结构

  * 引导块

    总是作为文件系统的首块。引导块不为文件系统所用，只是包含用来引导操作系统的信息。

  * 超级块

    紧随引导块，包含与文件系统有关的参数信息，其中包括：

    * i节点表容量
    * 文件系统中逻辑块大小
    * 以逻辑块计，文件系统的大小

  * i节点表

    文件系统中的每个文件或目录在i节点表中都会对应唯一一条记录。

  * 数据块

    文件系统的大部分空间都用于存放数据，以构成驻留在文件系统上的文件和目录

# 信号

信号到达后，进程视具体情况执行如下默认操作之一。

* 忽略信号
* 终止进程
* 产生核心转储文件，同时进程终止
* 停止进程：暂停进程的进行
* 与之前暂停后再度恢复进程的执行
* 采取默认行为
* 执行信号处理函数

## 发送信号

```c
#include <signal.h>

int kill(pid_t pid, int sig);
```

sig为指定要发送的信号。

pid要视情况而定：

* 如果pid大于0，那么会发送信号给由pid指定的进程。

* 如果pid等于0，那么会发送信号给与调用进程同组的每个进程，包括调用进程自身。

* 如果pid小于-1，那么会向组ID等于该绝对值的进程组内所有下属进程发送信号。

* 如果pid等于-1，发送范围为：调用进程有权将信号发往的每个目标进程，除去init和调用进程自身。

* 检查进程的存在

  kill函数的sig为0时，则无信号发送。kill()会去执行错误检查，查看是否可以向目标进程发送信号。这意味着，可以使用空信号来检测具有特定进程ID的进程是否存在。若发送空信号失败，且errno为ESRCH,则表明目标进程不存在。如果调用失败，且errno为EPERM则表示进程存在,但无权向目标进程发送信号。如果返回值为0表示进程存在。

raise(int sig):对自身发送信号

* 显示信号描述

  char *strsignal(int sig);

* 信号集

  ```c
  int sigemptyset(sigset_t *set);
  int sigfillset(sigset_t *set);
  int sigaddset(sigset_t *set, int sig);
  int sigdelset(sigset_t *set, int sig);
  int sigismember(sigset_t *set, int sig)
  ```

  sigemptyset()函数初始化一个未包含任何成员的信号集。sigfillset()函数则初始化一个信号集，使其包含所有的信号（包括实时信号）.

  如果sig是set的一个成员，那么sigismember()函数返回1（true），否则返回0.

* 信号掩码

```c
int sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
```

how:

​	SIG_BLOCK

​		将set指向信号集内的指定信号添加到信号掩码中。将信号掩码设置为当前值和set的并集。

​	SIG_UNBLOCK

​		移除信号从信号掩码中

​	SIG_SETMASK

​		将set指向的信号集赋给信号掩码

oldset返回旧的信号掩码

* 处于等待信号

  ```c
  int sigpending(sigset_t *set);
  ```

  sigpending()系统调用为调用进程返回处于等待状态的信号集。

* 改变信号处置：sigaction()

  ```c
  int sigaction(int sig, const struct sigaction *act, struct sigaction *oldact);
  
  struct sigaction {
      void (*sa_handler)(int); /* Address of handler */
      sigset_t sa_mask; /* Signals blocked during handler invocation */
      int sa_flags; /* Flags controlling handler invocation */
      void (*sa_restorer)(void); /* Not for application use */
  };
  ```

  sa_mask字段定义了一组信号，在调用由sa_handler所定义的处理器程序时将阻塞该信号。

  sa_flags:

  SA_NODEFER

  ​	捕获该信号时，不会在执行处理器程序时将该信号自动添加到进程掩码中。如果再收到该信号那么正在运行的处理函数会暂停，接着另一个处理函数会开始运行。

  SA_RESTART

  ​	自动重启由信号处理函数程序中断的系统调用

  SA_RESETHAND

  ​	捕获该信号时，会在调用处理器函数之前将信号处置为默认值（即SIG_DFL）(默认情况下，信号处理函数保持建立状态，直至进一步调用sigaction()将其显式解除).也就是下次捕获到该信号时不会调用信号处理函数

* 实时信号

  标准信号中的实时信号只有两个：SIGUSR1和SIGUSR2

  LINUX中扩展了32个不同的实时信号32~63 SIGRTMIN~SIGRTMAX

  * 发送实时信号

    ```c
    int sigqueue(pid_t pid, int sig, const union sigval value)
    ```

* 使用掩码来等待信号：sigsuspend()

  

