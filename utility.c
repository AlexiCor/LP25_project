#include <defines.h>
#include <string.h>

/*!
 * @brief concat_path concatenates suffix to prefix into result
 * It checks if prefix ends by / and adds this token if necessary
 * It also checks that result will fit into PATH_SIZE length
 * @param result the result of the concatenation
 * @param prefix the first part of the resulting path
 * @param suffix the second part of the resulting path
 * @return a pointer to the resulting path, NULL when concatenation failed
 */
char *concat_path(char *result, char *prefix, char *suffix) {
    if (!result || !prefix || !suffix) {
        return NULL; // Checks if any of the pointers is NULL
    }

    size_t prefix_len = strlen(prefix);
    size_t suffix_len = strlen(suffix);

    // Check if the total length is within PATH_SIZE, including the potential '/' and null terminator
    if (prefix_len + suffix_len + 2 > PATH_SIZE) {
        return NULL; // Total length exceeds PATH_SIZE
    }

    strcpy(result, prefix); // Copy the prefix to the result

    // Check if the last character of the prefix is not '/' and append it
    if (prefix[prefix_len - 1] != '/') {
        strcat(result, "/");
    }

    strcat(result, suffix); // Append the suffix to the result

    return result; // Return the concatenated path
}
