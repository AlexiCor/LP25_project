#include <sync.h>
#include <dirent.h>
#include <string.h>
#include <processes.h>
#include <utility.h>
#include <messages.h>
#include <file-properties.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <sys/msg.h>

#include <stdio.h>

/*!
 * @brief synchronize is the main function for synchronization
 * It will build the lists (source and destination), then make a third list with differences, and apply differences to the destination
 * It must adapt to the parallel or not operation of the program.
 * @param the_config is a pointer to the configuration
 * @param p_context is a pointer to the processes context
 */
void synchronize(configuration_t *the_config, process_context_t *p_context) {
    if (!the_config || !p_context) return;

    // Initialize file lists for source and destination
    files_list_t src_list = {0}, dst_list = {0};

    // Building the file lists: the approach changes based on parallel or non-parallel operation
    if (the_config->is_parallel) {
        // Parallel list building (this function needs to be implemented based on your parallel processing strategy)
        make_files_lists_parallel(&src_list, &dst_list, the_config, p_context->message_queue_id);
    } else {
        // Non-parallel list building
        make_files_list(&src_list, the_config->source);
        make_files_list(&dst_list, the_config->destination);
    }

    // Iterate through the source list to find differences and apply them to the destination
    for (files_list_entry_t *src_entry = src_list.head; src_entry != NULL; src_entry = src_entry->next) {
        // Find corresponding entry in destination list
        files_list_entry_t *dst_entry = find_entry_by_name(&dst_list, src_entry->path_and_name, 0, 0);

        // If there is a mismatch or the file doesn't exist in the destination, copy/update it
        if (!dst_entry || mismatch(src_entry, dst_entry, the_config->uses_md5)) {
            copy_entry_to_destination(src_entry, the_config);
        }
    }

    // Clean up file lists after processing
    clear_files_list(&src_list);
    clear_files_list(&dst_list);
}

/*!
 * @brief mismatch tests if two files with the same name (one in source, one in destination) are equal
 * @param lhd a files list entry from the source
 * @param rhd a files list entry from the destination
 * @has_md5 a value to enable or disable MD5 sum check
 * @return true if both files are not equal, false else
 */
bool mismatch(files_list_entry_t *lhd, files_list_entry_t *rhd, bool has_md5) {
    if (lhd->mode != rhd->mode || lhd->mtime.tv_sec != rhd->mtime.tv_sec || lhd->mtime.tv_nsec != rhd->mtime.tv_nsec || lhd->size != rhd->size || (lhd->entry_type == FICHIER && rhd->entry_type != FICHIER)) {
        return true;
    }

    if (has_md5) {
        for (int i = 0; i < 16; i++) {
            if (lhd->md5sum[i] != rhd->md5sum[i]) {
                return true;
            }
        }
    }

    return false;
}
/*!
 * @brief make_files_list builds a files list in no parallel mode
 * @param list is a pointer to the list that will be built
 * @param target_path is the path whose files to list
 */
void make_files_list(files_list_t *list, char *target_path) {
    if (!list || !target_path) {
        return;
    }

    DIR *dir = opendir(target_path);
    if (!dir) {
        return;
    }

    struct dirent *entry;
    while ((entry = get_next_entry(dir)) != NULL) {
        // Construire le chemin complet
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", target_path, entry->d_name);

        // Créer une nouvelle entrée dans la liste
        files_list_entry_t *new_entry = add_file_entry(list, full_path);
        // Ici, vous pouvez définir des propriétés supplémentaires pour new_entry si nécessaire
        // Si vous voulez inclure les sous-répertoires, appelez récursivement make_files_list pour les parcourir
    }

    closedir(dir);
}


/*!
 * @brief make_files_lists_parallel makes both (src and dest) files list with parallel processing
 * @param src_list is a pointer to the source list to build
 * @param dst_list is a pointer to the destination list to build
 * @param the_config is a pointer to the program configuration
 * @param msg_queue is the id of the MQ used for communication
 */
void make_files_lists_parallel(files_list_t *src_list, files_list_t *dst_list, configuration_t *the_config, int msg_queue) {
}

/*!
 * @brief copy_entry_to_destination copies a file from the source to the destination
 * It keeps access modes and mtime (@see utimensat)
 * Pay attention to the path so that the prefixes are not repeated from the source to the destination
 * Use sendfile to copy the file, mkdir to create the directory
 */

void copy_entry_to_destination(files_list_entry_t *source_entry, configuration_t *the_config) {
    if (!source_entry || !the_config) return;

    // Ouvrir le fichier source
    int source_fd = open(source_entry->path_and_name, O_RDONLY);
    if (source_fd == -1) {
        return;
    }

    // Créer le chemin du fichier de destination
    char destination_path[4096];
    // En supposant que concat_path soit implémentée correctement
    concat_path(destination_path, the_config->destination, source_entry->path_and_name);

    // Ouvrir ou créer le fichier de destination
    int dest_fd = open(destination_path, O_WRONLY | O_CREAT, source_entry->mode);
    if (dest_fd == -1) {
        close(source_fd);
        return;
    }

    // Copier les données du fichier
    struct stat file_stat;
    fstat(source_fd, &file_stat);
    sendfile(dest_fd, source_fd, NULL, file_stat.st_size);

    // Conserver l'heure de modification
    struct timespec times[2];
    times[0] = source_entry->mtime; // atime
    times[1] = source_entry->mtime; // mtime
    futimens(dest_fd, times);

    close(source_fd);
    close(dest_fd);
}

/*!
 * @brief make_list lists files in a location (it recurses in directories)
 * It doesn't get files properties, only a list of paths
 * This function is used by make_files_list and make_files_list_parallel
 * @param list is a pointer to the list that will be built
 * @param target is the target dir whose content must be listed
 */
void make_list(files_list_t *list, char *target) {
    DIR *dir = open_dir(target);
    if (!dir) {
        return;
    }

    struct dirent *entry;
    while ((entry = get_next_entry(dir)) != NULL) {
        // Construire le chemin complet
        char full_path[4096];
        snprintf(full_path, sizeof(full_path), "%s/%s", target, entry->d_name);

        // Créer une nouvelle entrée dans la liste
        files_list_entry_t *new_entry = add_file_entry(list, full_path);
        // Ici, vous pouvez définir des propriétés supplémentaires pour new_entry si nécessaire

        // Si l'entrée est un répertoire, appelez récursivement make_list pour explorer son contenu
        if (new_entry->entry_type == DOSSIER) {
            make_list(list, new_entry->path_and_name);
        }
    }

    closedir(dir);
}


/*!
 * @brief open_dir opens a dir
 * @param path is the path to the dir
 * @return a pointer to a dir, NULL if it cannot be opened
 */
DIR *open_dir(char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("Erreur lors de l'ouverture du répertoire");
    }
    return dir;
}


/*!
 * @brief get_next_entry returns the next entry in an already opened dir
 * @param dir is a pointer to the dir (as a result of opendir, @see open_dir)
 * @return a struct dirent pointer to the next relevant entry, NULL if none found (use it to stop iterating)
 * Relevant entries are all regular files and dir, except . and ..
 */
struct dirent *get_next_entry(DIR *dir) {
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            return entry;
        }
    }
    return NULL;
}
