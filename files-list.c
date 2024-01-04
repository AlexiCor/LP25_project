#include <files-list.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <file-properties.h>

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
    if (list == NULL || file_path == NULL) {
        printf("Error in the function add_file_entry of the file files-list.c\n");
        if (list == NULL) {
            printf("The list is NULL\n");
        }
        if (file_path == NULL) {
            printf("The file_path is NULL\n");
        }
        return NULL;
    } else if (list->head == NULL || list->tail == NULL) {
        if (list->head == NULL) {
            printf("The head of the list is NULL\n");
        }
        if (list->tail == NULL) {
            printf("The tail of the list is NULL\n");
        }
        return NULL;
    } else {
        //we verify if the file already exists in the list
            for (files_list_entry_t *cursor = list->head; cursor != NULL; cursor = cursor->next) {
                if (strcmp(cursor->path_and_name, file_path) == 0) {
                    return 0;
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

    //  Filling "char path_and_name[4096]";
    strcpy(new_entry->path_and_name, file_path);

    if (get_file_stats(new_entry)) == -1){
        printf("Error in the function fill_entry of the file files-list.c\n");
        printf("The get_file_stats function failed\n");
        return -1;
    }

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
    // We check if the list or the entry are NULL or not
    if (list == NULL || entry == NULL) {
        printf("Error in the function add_entry_to_tail of the file files-list.c\n");
        if (list == NULL) {
            printf("The list is NULL\n");
        }
        if (entry == NULL) {
            printf("The entry is NULL\n");
        }
        return -1;
    } else {
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

    // We check if the list or file_path is NULL or not
    if (list == NULL || file_path == NULL) {
        printf("Error in the function find_entry_by_name of the file files-list.c\n");
        if (list == NULL) {
            printf("The list is NULL\n");
        }
        if (file_path == NULL) {
            printf("The file_path is NULL\n");
        }
        return NULL;
    } else if (list->head == NULL) {
        printf("The head of the list is NULL\n");
        return NULL;
    } else {
        // We create a variable of type files_list_entry_t named cursor, and we initialize it with the head of the list
        files_list_entry_t *cursor = list->head;
        while (cursor != NULL) {
            // We check if the file_path is the same as the path_and_name of the cursor
            // if it is we return the cursor
            if (strcmp(file_path, cursor->path_and_name) == 0) {
                return cursor;
            } else {
                cursor = cursor->next;
            }
        }
        // If we did not find the file_path in the list we return NULL
        printf("Error in the function find_entry_by_name of the file files-list.c\n");
        printf("The file_path is not in the list\n");
        return NULL;
    }
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
