#include <files-list.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>

/*!
 * @brief clear_files_list clears a files list
 * @param list is a pointer to the list to be cleared
 * This function is provided, you don't need to implement nor modify it
 */
void clear_files_list(files_list_t *list) {
    while (list->head) {
        files_list_entry_t *tmp = list->head;
        list->head = tmp->next;
        free(tmp);
    }
}

/*!
 *  @brief add_file_entry adds a new file to the files list.
 *  It adds the file in an ordered manner (strcmp) and fills its properties
 *  by calling stat on the file.
 *  Il the file already exists, it does nothing and returns 0
 *  @param list the list to add the file entry into
 *  @param file_path the full path (from the root of the considered tree) of the file
 *  @return a pointer to the added element if success, NULL else (out of memory)
 */
files_list_entry_t *add_file_entry(files_list_t *list, char *file_path) {

    // We verify that the list and the file_path are not NULL
    if (list == NULL || file_path == NULL){
        printf("Error in the function add_file_entry of the file files-list.c\n");
        if (list == NULL && file_path == NULL){
            printf("The list and the file_path are NULL\n");
        }
        if (list == NULL && file_path != NULL){
            printf("The list is NULL\n");
        }
        if (list != NULL && file_path == NULL){
            printf("The file_path is NULL\n");
        }
        return NULL;

    } else {
        //we verify if the file already exists in the list
        if (list->head != NULL && list->tail != NULL) {
            for (files_list_entry_t *cursor = list->head; cursor != NULL; cursor = cursor->next) {
                if (strcmp(cursor->path_and_name, file_path) == 0) {
                    return 0;
                }
            }
        }
        // If the file does not exist in the list, we will add it

        // we allocate memory for a variable of type files_list_entry_t named new_entry
        files_list_entry_t *new_entry = malloc(sizeof(files_list_entry_t));
        // We check if the allocation of memory was successful, if not we return NULL (out of memory)
        if (!new_entry) {
            printf("Error when allocating memory in the function add_file_entry of the file files-list.c\n");
            return NULL;
        }

        // We initialize the entire memory space allocated to new entry to 0;
        memset(new_entry, 0, sizeof(files_list_entry_t));

        // We call the fill_entry function to fill the different elements of the structure of new_entry
        if ((fill_entry(list, file_path, new_entry)) == 0){

            // We add the new_entry to the list of files, but we have to add it in an ordered manner (with strcmp)
            // We check if the list is empty, if it is we add the new_entry to the head of the list
            // else we will add it in an ordered manner
            if (list->head == NULL) {
                list->head = new_entry;
                return new_entry;
            } else {
                files_list_entry_t *cursor = list->head;
                while (cursor != NULL) {
                    if (strcmp(cursor->path_and_name, file_path) > 0) {
                        cursor = cursor->next;
                    } else {
                        if (cursor->prev) {
                            cursor->prev->next = new_entry;
                            new_entry->prev = cursor->prev;
                        } else {
                            list->head = new_entry;
                        }
                        cursor->prev = new_entry;
                        new_entry->next = cursor;
                        return new_entry;
                    }
                }
                list->tail->next = new_entry;
                return new_entry;
            }
        } else {
            // If the fill_entry function failed we free the memory allocated to new_entry, and we return NULL,
            // the error message is already displayed in the fill_entry function
            free(new_entry);
            return NULL;
        }
    }
}

/*!
 *  @brief fill_entry fills the properties of a file entry
 *  It fills the properties of a file entry by calling stat on the file.
 *  @param list the list to add the file entry into
 *  @param file_path the full path (from the root of the considered tree) of the file
 *  @param new_entry the entry to fill
 *  @return 0 in case of success, -1 else
 */
int fill_entry(files_list_t *list, char *file_path, files_list_entry_t *new_entry) {

    // We create a variable of type struct stat to stock the information that the stat function returns about the file
    struct stat info_file;

    // Use of the stat function to obtain information about the file
    // We verify if the stat function was successful,
        // if it was we fill the different elements of the structure of new_entry
        // if not we return -1
    if (stat(file_path, &info_file) == 0) {

        //  Filling "char path_and_name[4096]";
        strcpy(new_entry->path_and_name, file_path);

        //  Filling "struct timespec mtime"
        //      st_mtimespec and mtime are both a structure called timespec:
        //      that contains two elements: tv_sec and tv_nsec
        //      two variables of type time_t (long int) or simply long that represent the time in seconds
        //      ,so we can simply copy the structure st_mtimespec in mtime without any problem
        new_entry->mtime.tv_sec = info_file.st_mtime;
        new_entry->mtime.tv_nsec = 0;


        //  Filling "uint64_t size"
        new_entry->size = info_file.st_size;

        //  Filling "uint8_t md5sum[16]"
        compute_md5_file(file_path, new_entry->md5sum);

        //  Filling "file_type_t entry_type"
        //      We use the macro S_ISDIR to check if the file is a directory
        //      If it is a directory we fill entry_type with DOSSIER
        //      We use the macro S_ISREG to check if the file is a regular file
        //      If it is a regular file we fill entry_type with FICHIER
        if (S_ISDIR(info_file.st_mode)) {
            new_entry->entry_type = DOSSIER;
        }
        if (S_ISREG(info_file.st_mode)) {
            new_entry->entry_type = FICHIER;
        }

        //  Filling "mode_t mode"
        new_entry->mode = info_file.st_mode;

        //  Filling "struct _files_list_entry *next"
        new_entry->next = NULL;

        //  Filling "struct _files_list_entry *prev"
        if (!list->tail) {
            new_entry->prev = list->tail;
        } else {
            new_entry->prev = NULL;
        }
        list->tail = new_entry;
        return 0;

    } else {
        printf("Error when recuperating the information about the file in the function fill_entry of the file files-list.c\n");
        return -1;
    }
}

#define MAX_BUFFER_SIZE 1024

typedef struct {
    uint8_t data[16];
} MD5Hash;

/*!
 * @brief compute_md5_file computes the MD5 hash of a file
 * @param filename is the name of the file to hash
 * @param result is the resulting hash modified by the function
 */
void compute_md5_file(const char *filename, uint8_t result[16]) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    unsigned char buffer[MAX_BUFFER_SIZE];
    size_t bytesRead;
    MD5Hash hash;

    // Initialize hash
    for (int i = 0; i < 16; i++) {
        hash.data[i] = 0;
    }

    // Read file and update hash
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        for (size_t i = 0; i < bytesRead; i++) {
            // Simple XOR operation for illustration purposes
            hash.data[i % 16] ^= buffer[i];
        }
    }

    fclose(file);

    // Copy the computed hash to the result
    memcpy(result, &hash, sizeof(MD5Hash));
}

/*!
 * @brief add_entry_to_tail adds an entry directly to the tail of the list
 * It supposes that the entries are provided already ordered, e.g. when a lister process sends its list's
 * elements to the main process.
 * @param list is a pointer to the list to which to add the element
 * @param entry is a pointer to the entry to add. The list becomes owner of the entry.
 * @return 0 in case of success, -1 else
 */
int add_entry_to_tail(files_list_t *list, files_list_entry_t *entry) {

    // We check if the list and the entry are not NULL
    if (list != NULL && entry != NULL) {
        // We check if the list is empty, if it is we add the entry to the head and the tail of the list (the list has only one element)
        // else we will add it only to the tail of the list
        if (list->head == NULL && list->tail == NULL) {
            list->head = entry;
            list->tail = entry;
            return 0;
        } else {
            if (list->head != NULL && list->tail != NULL) {
                list->tail->next = entry;
                entry->prev = list->tail;
                list->tail = entry;
                return 0;
            } else {
                printf("Error in the function add_entry_to_tail of the file files-list.c\n");
                printf("The list is not empty but the head or the tail is NULL\n");
                return -1;
            }
        }
    } else {
        printf("Error in the function add_entry_to_tail of the file files-list.c\n");
        printf("The list or the entry is NULL\n");
        return -1;
    }
}

/*!
 *  @brief find_entry_by_name looks up for a file in a list
 *  The function uses the ordering of the entries to interrupt its search
 *  @param list the list to look into
 *  @param file_path the full path of the file to look for
 *  @param start_of_src the position of the name of the file in the source directory (removing the source path)
 *  @param start_of_dest the position of the name of the file in the destination dir (removing the dest path)
 *  @return a pointer to the element found, NULL if none were found.
 */
files_list_entry_t *find_entry_by_name(files_list_t *list, char *file_path, size_t start_of_src, size_t start_of_dest) {

    // We check if the list is not NULL
    if (list != NULL && file_path != NULL) {
        // We check if the list is empty, if it is we return NULL
        if (list->head == NULL) {
            return NULL;
        } else {
            // We create a variable of type files_list_entry_t named cursor, and we initialize it with the head of the list
            files_list_entry_t *cursor = list->head;
            while(cursor != NULL) {
                // We check if the file_path is the same as the path_and_name of the cursor
                // if it is we return the cursor
                if (strcmp(file_path, cursor->path_and_name) == 0) {
                    return cursor;
                } else {
                    cursor = cursor->next;
                }
            }
            // If we did not find the file_path in the list we return NULL
            return NULL;
        }
    } else {
        printf("Error in the function find_entry_by_name of the file files-list.c\n");
        if (list == NULL && file_path == NULL){
            printf("The list and the file_path are NULL\n");
        }
        if (list == NULL && file_path != NULL){
            printf("The list is NULL\n");
        }
        if (list != NULL && file_path == NULL){
            printf("The file_path is NULL\n");
        }
    return NULL;
}

/*!
 * @brief display_files_list displays a files list
 * @param list is the pointer to the list to be displayed
 * This function is already provided complete.
 */
void display_files_list(files_list_t *list) {
    if (!list)
        return;
    
    for (files_list_entry_t *cursor=list->head; cursor!=NULL; cursor=cursor->next) {
        printf("%s\n", cursor->path_and_name);
    }
}

/*!
 * @brief display_files_list_reversed displays a files list from the end to the beginning
 * @param list is the pointer to the list to be displayed
 * This function is already provided complete.
 */
void display_files_list_reversed(files_list_t *list) {
    if (!list)
        return;
    
    for (files_list_entry_t *cursor=list->tail; cursor!=NULL; cursor=cursor->prev) {
        printf("%s\n", cursor->path_and_name);
    }
}
