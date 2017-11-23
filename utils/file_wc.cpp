#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#define MAXBSIZE 65536
u_long file_wc(char *file)
{
    register u_char *p;
    register short gotsp;
    register int ch, len;
    register u_long linect, charct;
    int fd;
    u_char buf[MAXBSIZE];
    if (file) {
        if ((fd = open(file, O_RDONLY, 0)) < 0)
            return -1;
        for (gotsp = 1; len = read(fd, buf, MAXBSIZE);) {
            if (len == -1)
                return -1;
            charct += len;
            for (p = buf; len--;) {
                ch = *p++;
                if (ch == '\n')
                    ++linect;
                if (isspace(ch))
                    gotsp = 1;
                else if (gotsp) {
                    gotsp = 0;
                }
            }
        }
    }
    return linect;
}


#if 1

int main(int argc, char* argv[])
{
    if(2 != argc) {
        return 0;
    }
    printf("%s: total line: %lu\n", argv[1], file_wc(argv[1]));
    return 0;
}

#endif
