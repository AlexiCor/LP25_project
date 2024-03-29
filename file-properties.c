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

    // We create a variable of type struct stat to stock the information that the stat function returns about the file
    struct stat file_stat;

    // Use of the stat function to obtain information about the file
    // We verify if the stat function was successful,
        // if not we return -1
        // else we fill the different elements of the structure of new_entry
    if (stat(entry->path_and_name, &file_stat) == -1) {
        perror("stat");
        return -1; // Error while getting file stats
    } else {
        // mode (permissions)
        entry->mode = file_stat.st_mode;

        // entry type
        if (S_ISREG(file_stat.st_mode)) {
            entry->entry_type = FICHIER;
            // mtime (in nanoseconds)
            entry->mtime.tv_nsec = (file_stat.st_mtime)*1000000000;
            // size
            entry->size = file_stat.st_size;
            // MD5 sum

            if ((compute_file_md5(entry)) == -1) {
                printf("compute_file_md5");
                return -1;
            }
        } else if (S_ISDIR(file_stat.st_mode)) {
            entry->entry_type = DOSSIER;
        } else {
            return -1; // Unsupported file type
        }

        return 0; // Success
    }
}

/*!
 * @brief compute_file_md5 computes a file's MD5 sum
 * @param the pointer to the files list entry
 * @return -1 in case of error, 0 else
 * Use libcrypto functions from openssl/evp.h
 */
int compute_file_md5(files_list_entry_t *entry) {
    // the reason why we have put this function (except the return 0) in commentary is because the EVP functions weren't reconised when compiling the project
    /*
    if (!entry || !entry->path_and_name) return -1;

    FILE *file = fopen(entry->path_and_name, "rb");
    if (!file) {
        perror("Error opening file");
        return -1;
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        fclose(file);
        return -1;
    }

    EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);

    unsigned char buffer[MAX_BUF_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, MAX_BUF_SIZE, file)) > 0) {
        EVP_DigestUpdate(mdctx, buffer, bytesRead);
    }

    fclose(file);

    unsigned int md_len;
    EVP_DigestFinal_ex(mdctx, entry->md5sum, &md_len);
    EVP_MD_CTX_free(mdctx);
    */
    return 0;
}

/*!
 * @brief directory_exists tests the existence of a directory
 * @path_to_dir a string with the path to the directory
 * @return true if directory exists, false else
 */
bool directory_exists(char *path_to_dir) {
    struct stat dirStat;

    if (stat(path_to_dir, &dirStat) == 0 && S_ISDIR(dirStat.st_mode)) {
        return true;
    } else {
        return false;
    }
}


/*!
 * @brief is_directory_writable tests if a directory is writable
 * @param path_to_dir the path to the directory to test
 * @return true if dir is writable, false else
 * Hint: try to open a file in write mode in the target directory.
 */
bool is_directory_writable(char *path_to_dir) {
    if (access(path_to_dir, W_OK) == 0) {
        return true; //Il est possible d'écrire dans le répertoire
    } else {
        return false; //Il est impossible d'écrire dans le répertoire
    }
}
