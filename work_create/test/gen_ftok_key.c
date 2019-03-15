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
ͨ��ִ�н���ɿ�����ftok��ȡ�ļ�ֵ����ftok()�����ĵڶ��������ĺ�8��bit��st_dev�ĺ���λ��st_ino�ĺ���λ���ɵġ�

*/

/*
https://blog.csdn.net/u013485792/article/details/50764224
*/
