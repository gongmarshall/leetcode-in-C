1. 第一种方式重定向stdout或者stderr, 使用freopen

   a. 使用freopen("file_path", "w+", stdout), 将stdout的输出重定向到file_path中

   ​	在ls -l /proc/pid/fd 中可以看到2->file_path， pid为调用freopen的进程pid。

   b. 在调用freopen之前调用ttyname_r(1, ttyName, 128);可以将获取标准输出原本输出的设备到ttyName中。

   ​	恢复时： freopen(ttyName, "w+", 1); 将标准输出重定向到之前的设备中。

   但这样并不能完全恢复，还是存在一些问题。

   > On POSIX systems, you can do it as follows:
   >
   > ```c
   > int bak, new;
   > 
   > fflush(stdout);
   > bak = dup(1);
   > new = open("/dev/null", O_WRONLY);
   > dup2(new, 1);
   > close(new);
   > 
   > /* your code here ... */
   > 
   > fflush(stdout);
   > dup2(bak, 1);
   > close(bak);
   > ```
   >
   > What you want is not possible in further generality.
   >
   > Any solution using `freopen` is wrong, as it does not allow you to restore the original `stdout`. Any solution by assignment to `stdout` is wrong, as `stdout` is not an lvalue (it's a macro that expands to an expression of type `FILE *`).

2. 使用dup, dup2

   a. int bak = dup(1); 获取stdout的赋值文件描述符

   ​	fd = open("test2.log", O_WRONLY|O_CREAT, 0644)

   ​	dup2(fd, 1); 这样fd和1共享同一个文件表项也就相当于将1文件描述符复制到fd中

   b. 恢复时

   ​	dup(bak, 1);将1重新复制到bak中

   eg: 默认情况下： dup(1) 3为dup(1)返回文件描述符

   lrwx------ 1 marshall marshall 64 3月  18 14:22 0 -> /dev/pts/2
   lrwx------ 1 marshall marshall 64 3月  18 14:22 1 -> /dev/pts/2
   lrwx------ 1 marshall marshall 64 3月  18 14:22 2 -> /dev/pts/2
   lrwx------ 1 marshall marshall 64 3月  18 14:22 3 -> /dev/pts/2

   

   fd=open("test2.txt","w+", 0644), dup2(fd, 1) 4为fd, 1的指向的文件表项指向了fd指向的。

   lrwx------ 1 marshall marshall 64 3月  18 14:22 0 -> /dev/pts/2                         
   l-wx------ 1 marshall marshall 64 3月  18 14:22 1 -> /home/marshall/study/log/test2.log 
   lrwx------ 1 marshall marshall 64 3月  18 14:22 2 -> /dev/pts/2                         
   lrwx------ 1 marshall marshall 64 3月  18 14:22 3 -> /dev/pts/2                         
   l-wx------ 1 marshall marshall 64 3月  18 14:22 4 -> /home/marshall/study/log/test2.log 

   

   close(fd):

   lrwx------ 1 marshall marshall 64 3月  18 14:22 0 -> /dev/pts/2
   l-wx------ 1 marshall marshall 64 3月  18 14:22 1 -> /home/marshall/study/log/test2.log
   lrwx------ 1 marshall marshall 64 3月  18 14:22 2 -> /dev/pts/2
   lrwx------ 1 marshall marshall 64 3月  18 14:22 3 -> /dev/pts/2

   

   dup2(bak, 1);1指向bak原本指向的表项，并close(bak):

   lrwx------ 1 marshall marshall 64 3月  18 14:22 0 -> /dev/pts/2
   lrwx------ 1 marshall marshall 64 3月  18 14:22 1 -> /dev/pts/2
   lrwx------ 1 marshall marshall 64 3月  18 14:22 2 -> /dev/pts/2

* fflush(stdout)可以刷新printf的缓存
* rc = lseek(fd, 0, SEEK_END)返回文件末尾的偏移量单位为字节。