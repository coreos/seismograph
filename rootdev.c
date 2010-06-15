/* Taken from util-linux source. GPLv2.
 * Emits the current rootfs device.
 * Works by searching /dev recursively for a BLK device with the same device
 * number as '/'.
 */

#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>


static int
find_dev_recursive(char *dirnamebuf, int number) {
        DIR *dp;
        struct dirent *dir;
        struct stat s;
        int dirnamelen = 0;

        if ((dp = opendir(dirnamebuf)) == NULL)
                err(1, "can't read directory %s", dirnamebuf);
        dirnamelen = strlen(dirnamebuf);
        while ((dir = readdir(dp)) != NULL) {
                if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, ".."))
                        continue;
                if (dirnamelen + 1 + strlen(dir->d_name) > PATH_MAX)
                        continue;
                dirnamebuf[dirnamelen] = '/';
                strcpy(dirnamebuf+dirnamelen+1, dir->d_name);
                if (lstat(dirnamebuf, &s) < 0)
                        continue;
                if ((s.st_mode & S_IFMT) == S_IFBLK && s.st_rdev == number)
                        return 1;
                if ((s.st_mode & S_IFMT) == S_IFDIR &&
                    find_dev_recursive(dirnamebuf, number))
                        return 1;
        }
        dirnamebuf[dirnamelen] = 0;
        closedir(dp);
        return 0;
}

int main(int argc, char *argv[]) {
        struct stat s;
        char *file = "/";
        static char name[PATH_MAX+1];
        
        if (argc > 1)
                file = argv[1];

        if (stat(file, &s) < 0)
                err(1, "unable to stat %s", file);
            
        if (!s.st_dev)
                err(1, "unknown device number 0");

        strcpy(name, "/dev");

        if (!find_dev_recursive(name, s.st_dev)) {
                fprintf(stderr, "unable to find match\n");
                return 1;
        }

        printf("%s\n", name);
    
        return 0;
}
