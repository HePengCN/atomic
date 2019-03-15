#include <stdio.h>       
#include <stdlib.h>  
#include <sys/stat.h>
 
int main()
{
        char    filename[50];
        struct stat     buf;
        int     ret;
        strcpy( filename, "/home/satellite/" );
        ret = stat( filename, &buf );
        if( ret )
        {
                printf( "stat error\n" );
                return -1;
        }
 
        printf( "the file info: ftok( filename, 0x27 ) = %x, st_ino = %x, st_dev= %x\n", ftok( filename, 0x27 ), buf.st_ino, buf.st_dev );
 
        return 0;
}

/*
satellite@ubuntu:~/test$ ./wxyuanthe file info: ftok( filename, 0x27 ) = 27012eef, st_ino = e2eef, st_dev= 801
通过执行结果可看出，ftok获取的键值是由ftok()函数的第二个参数的后8个bit，st_dev的后两位，st_ino的后四位构成的。

*/

/*
https://blog.csdn.net/u013485792/article/details/50764224
*/
