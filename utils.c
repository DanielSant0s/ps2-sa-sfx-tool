
bool fexists(const char *fname) {
    FILE *file;

    file = fopen(fname, "r");

    if (file != NULL) {
        fclose(file);
        return true;
    }

    return false;
}

#if defined(WIN32)
#include <direct.h>

void create_folder(const char* folderName) {
    // Remove existing folder
    if (_rmdir(folderName) != 0) {
        // Error might be due to non-existent folder, ignore
    }

    // Create a new folder
    if (_mkdir(folderName) != 0) {
        perror("Error creating folder");
        exit(EXIT_FAILURE);
    }
    printf("Folder created or reset successfully.\n");
}
#else
void create_folder(const char* dname) {
    struct stat st;

    // Check if the folder exists
    if (stat(dname, &st) == -1) {
        // Folder doesn't exist, create it
        if (mkdir(dname, 0777) != 0) {
            perror("Error creating folder");
            exit(EXIT_FAILURE);
        }
        printf("Folder created successfully.\n");
    } else if (S_ISDIR(st.st_mode)) {
        // Folder exists, delete it and create a new one
        char command[256];
        snprintf(command, sizeof(command), "rm -r %s", dname);

        if (system(command) != 0) {
            perror("Error deleting folder");
            exit(EXIT_FAILURE);
        }

        if (mkdir(dname, 0777) != 0) {
            perror("Error creating folder");
            exit(EXIT_FAILURE);
        }
        printf("Folder reset successfully.\n");
    } else {
        printf("A file with the same name exists. Please remove it or choose a different name.\n");
    }
}
#endif

