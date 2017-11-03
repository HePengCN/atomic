#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*only for loading small bin file*/
std::string loadBinFile(const char* filePath)
{
    std::string str;
    struct stat statBuf;
    if(0 != stat(filePath, &statBuf)) {
        printf("%s, %d, Error: stat file %s fail\n", __FUNCTION__, __LINE__, filePath);
        return str;
    }
    str.resize(statBuf.st_size);

    FILE *fd = fopen(filePath, "r");
    if(!fd) {
        printf("%s, %d, Error: open file %s fail\n", __FUNCTION__, __LINE__, filePath);
        return str;
    }

    fread(const_cast<char*>(str.data()), 1, statBuf.st_size, fd);
    fclose(fd);
    return str;
}

#if 0
int main(int argc, char* argv[])
{
    std::string str = loadBinFile(argv[1]);

    printf("str: %s\n", str.c_str());
    return 0;
}
#endif
