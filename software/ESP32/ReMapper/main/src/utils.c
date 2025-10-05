#include "utils.h"

int files_in_dir(const char *path, char *output[255], int output_size) {

    DIR *dp = opendir(path);
    if (!dp) {
        printf("Failed to open dir: %s\n", path);
        return 0;
    }

    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dp)) != NULL && count < output_size) {
        // skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (strlen(entry->d_name) >= 255)
            continue;

        // no dirs
        if (entry->d_type == DT_DIR)
            continue;

        strcpy(output[count], entry->d_name);

        count++;
    }

    closedir(dp);

    return count;
}