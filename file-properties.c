#include <file-properties.h>

#include <sys/stat.h>
#include <dirent.h>
#include <openssl/evp.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <defines.h>
#include <fcntl.h>
#include <stdio.h>
#include <utility.h>

/*!
 * @brief get_file_stats gets all of the required information for a file (inc. directories)
 * @param the files list entry
 * You must get:
 * - for files:
 *   - mode (permissions)
 *   - mtime (in nanoseconds)
 *   - size
 *   - entry type (FICHIER)
 *   - MD5 sum
 * - for directories:
 *   - mode
 *   - entry type (DOSSIER)
 * @return -1 in case of error, 0 else
 */
int get_file_stats(files_list_entry_t *entry) {
    struct stat fileStat;

    if (stat(entry->path, &fileStat) < 0)
    {
        return -1; // Échec de la récupération des statistiques
    }

    entry->mode = fileStat.st_mode;                                                  // Permissions
    entry->mtime = fileStat.st_mtim.tv_sec * 1000000000L + fileStat.st_mtim.tv_nsec; // Heure de modification en nanosecondes
    entry->size = fileStat.st_size;                                                  // Taille du fichier

    if (S_ISREG(fileStat.st_mode))
    {
        entry->type = FICHIER; // Type de fichier régulier
    }
    else if (S_ISDIR(fileStat.st_mode))
    {
        entry->type = DOSSIER; // Type de dossier
    }
    else
    {
        return -1; // Type de fichier non pris en charge
    }

    return 0;
}

/*!
 * @brief compute_file_md5 computes a file's MD5 sum
 * @param the pointer to the files list entry
 * @return -1 in case of error, 0 else
 * Use libcrypto functions from openssl/evp.h
 */
int compute_file_md5(files_list_entry_t *entry) {
}

/*!
 * @brief directory_exists tests the existence of a directory
 * @path_to_dir a string with the path to the directory
 * @return true if directory exists, false else
 */
bool directory_exists(char *path_to_dir) {
    struct stat dir;

    if (stat(path_to_dir, &dir) == 0 && S_ISDIR(dir.st_mode)) {
        return 0; //Le repertoire existe.
    } else {
        return -1; //Le repertoire n'existe pas.
    }
}

/*!
 * @brief is_directory_writable tests if a directory is writable
 * @param path_to_dir the path to the directory to test
 * @return true if dir is writable, false else
 * Hint: try to open a file in write mode in the target directory.
 */
bool is_directory_writable(char *path_to_dir) {
}
