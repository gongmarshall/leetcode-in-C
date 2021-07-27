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

gmtime(const time_t *timep); //glibc/time/gmtime.c

```c
/* Convert `time_t' to `struct tm' in UTC.
   Copyright (C) 1991-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
#include <time.h>
/* Return the `struct tm' representation of *T in UTC,
   using *TP to store the result.  */
struct tm *
__gmtime64_r (const __time64_t *t, struct tm *tp)
{
  return __tz_convert (*t, 0, tp);
}
/* Provide a 32-bit variant if needed.  */
#if __TIMESIZE != 64
libc_hidden_def (__gmtime64_r)
struct tm *
__gmtime_r (const time_t *t, struct tm *tp)
{
  __time64_t t64 = *t;
  return __gmtime64_r (&t64, tp);
}
#endif
libc_hidden_def (__gmtime_r)
weak_alias (__gmtime_r, gmtime_r)
/* Return the `struct tm' representation of *T in UTC.  */
struct tm *
__gmtime64 (const __time64_t *t)
{
  return __tz_convert (*t, 0, &_tmbuf);
}
/* Provide a 32-bit variant if needed.  */
#if __TIMESIZE != 64
libc_hidden_def (__gmtime64)
struct tm *
gmtime (const time_t *t)  
{
  __time64_t t64 = *t;
  return __gmtime64 (&t64);
}
#endif
__tz_convert (__time64_t timer, int use_localtime, struct tm *tp)
{
  long int leap_correction;
  int leap_extra_secs;
  __libc_lock_lock (tzset_lock);
  /* Update internal database according to current TZ setting.
     POSIX.1 8.3.7.2 says that localtime_r is not required to set tzname.
     This is a good idea since this allows at least a bit more parallelism.  */
  tzset_internal (tp == &_tmbuf && use_localtime);
  if (__use_tzfile)
    __tzfile_compute (timer, use_localtime, &leap_correction,
                      &leap_extra_secs, tp);
  else
    {
      if (! __offtime (timer, 0, tp))
        tp = NULL;
      else
        __tz_compute (timer, tp, use_localtime);
      leap_correction = 0L;
      leap_extra_secs = 0;
    }
  __libc_lock_unlock (tzset_lock);
  if (tp)
    {
      if (! use_localtime)
        {
          tp->tm_isdst = 0;
          tp->tm_zone = "GMT";
          tp->tm_gmtoff = 0L;
        }
      if (__offtime (timer, tp->tm_gmtoff - leap_correction, tp))
        tp->tm_sec += leap_extra_secs;
      else
        tp = NULL;
    }
  return tp;
}



```

# 系统和进程信息

## 获取与进程有关的信息：/proc/PID







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

