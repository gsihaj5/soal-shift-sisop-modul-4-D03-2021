#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <stdbool.h>

char buffer[1000];
static const char *dirpath = "/home/gerry/Downloads";

bool isLowerCase(char character){
    return (character >= 97 && character <= 122);
}

bool isUpperCase(char character){
    return (character >= 65 && character <= 90);
}

void mirrorCipher(const char *filename)
{
    int counter = 0;
    while(filename[counter] != '\0'){
        char temp = filename[counter];

        int new_ascii;
        if(isUpperCase(temp)) new_ascii = 90 - (temp-'A');
        else if(isLowerCase(temp)) new_ascii = 122 - (temp-'a');
        else new_ascii = temp;

        buffer[counter] = new_ascii;
        counter++;
    }
    buffer[counter] = '\0';
}

static  int  xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
    res = lstat(path, stbuf);

    if (res == -1) return -errno;
    return 0;
}

bool isATOZ(const char *path){
    if(strlen(path) < 4) return false;

    char atoz[5] = "AtoZ_";

    int counter;
    for(counter = 0; counter <= 4; counter++){
        if(atoz[counter] != path[counter]) return false;
    }
    return true;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
char fullPath[1000];

    if(strcmp(path,"/") == 0) {
        path=dirpath;
        sprintf(fullPath,"%s",path);
    } else
        sprintf(fullPath, "%s%s",dirpath,path);

    int res = 0;

    DIR *dp;
    struct dirent *de;
    (void) offset;
    (void) fi;

    dp = opendir(fullPath);

    if (dp == NULL) return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));

        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        res = (filler(buf, de->d_name, &st, 0));

        if(res!=0) break;
    }

    closedir(dp);

    return 0;}



static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int fd;
    int res;
    (void) fi;

    fd = open(path, O_RDONLY);

    if (fd == -1) return -errno;

    res = pread(fd, buf, size, offset);

    if (res == -1) res = -errno;

    close(fd);

    return res;
}

static int xmp_rename(const char *from, const char *to) {
    printf("renaming from %s to %s \n", from, to);

    return 0;
}

static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .read = xmp_read,
    .rename = xmp_rename,
};


int  main(int  argc, char *argv[])
{
    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
