#include "dir.h"

bool print_dir(){
    DIR* dir = opendir(".");
    struct dirent *ent;
    struct stat stat;
    char* ent_dname;
    char* format;
    char path[1025];
    int i = 0;

    if(!dir){
        fprintf(stderr, "[ERROR] Can't open directory");
        return false;
    }
    ent = readdir(dir);
    while (ent){
        ent_dname = ent->d_name;
        lstat(ent_dname, &stat);

        if(S_ISDIR(stat.st_mode)) format = "%s/";
        else if(S_IXUSR & stat.st_mode) format = "%s*";
        else format = "%s ";

        sprintf(path, format, ent->d_name);
        printf("%-25s", path);

        if(++i % 6 == 0) printf("\n");
        ent = readdir(dir);
    }
    if (i % 6 != 0) printf ("\n");
    closedir(dir);
    return true;
}