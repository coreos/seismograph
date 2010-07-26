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
find_dev_recursive(char *dirnamebuf, int number, int deviceOnly) {
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
                if ((s.st_mode & S_IFMT) == S_IFBLK && s.st_rdev == number){
                        if (deviceOnly) {
                                int len = strlen(dirnamebuf);
                                char c = 0;
                                do {
                                        c = dirnamebuf[len-1];
                                        --len;
                                }while(c > 0 && c < 9 && len > 0);
                                /* arm has "p" for partition */
                                if (dirnamebuf[len-1] == 'p')
                                        --len;
                                dirnamebuf[len]='\0';
                        }
                        return 1;
                }
                if ((s.st_mode & S_IFMT) == S_IFDIR &&
                    find_dev_recursive(dirnamebuf, number, deviceOnly))
                        return 1;
        }
        dirnamebuf[dirnamelen] = 0;
        closedir(dp);
        return 0;
}

void usage(){
   printf ("rootdev \n\t-d (for device only)\n");
}

int main(int argc, char *argv[]) {
        struct stat s;
        char *file = "/";
        static char name[PATH_MAX+1];
        int deviceOnly=0;
        int c;
        extern char *optarg;
        extern int optind, optopt;
        while ((c = getopt(argc, argv, "hd")) != -1) {
                switch(c) {
                case 'd':
                        deviceOnly=1;
                        break;
                case 'h':
                default:
                        usage();
                        return 1;
                }
        }
        if (argc - optind >= 1)
                file = argv[optind];

        if (stat(file, &s) < 0)
                err(1, "unable to stat %s", file);

        if (!s.st_dev)
                err(1, "unknown device number 0");

        strcpy(name, "/dev");

        if (!find_dev_recursive(name, s.st_dev, deviceOnly)) {
                fprintf(stderr, "unable to find match\n");
                return 1;
        }

        printf("%s\n", name);

        return 0;
}
