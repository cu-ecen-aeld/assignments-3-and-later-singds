#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>

int main(int argc, char *argv[])
{
    const char *writefile, *writestr, *filename;

    openlog(NULL, 0, LOG_USER);
    if (argc != 3) {
        syslog(LOG_ERR, "Incompatible number of arguments");

        printf("error: Two parameters must be provided\n");
        printf("    arg1 : a full path to a file (including filename) on the filesystem\n");
        printf("    arg2 : a text string which will be written within this file\n");
        exit(1);
    }

    writefile = argv[1];
    writestr = argv[2];
    filename = writefile;
    for (const char *p = writefile; *p != 0; p++) {
        if (*p == '/') {
            filename = p + 1;
        }
    }

    syslog(LOG_DEBUG, "Writing %s to %s", writestr, filename);
    FILE *f = fopen (writefile, "wb");
    if (f == NULL) {
        syslog(LOG_ERR, "Can't open file %s", writefile);
        exit(1);
    }
    if (fwrite(writestr, 1, strlen(writestr), f) < 0) {
        syslog(LOG_ERR, "Can't write to file %s", writefile);
        exit(1);
    }
    fclose(f);

    closelog();
    exit(0);
}
