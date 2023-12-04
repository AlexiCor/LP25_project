#include <configuration.h>
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

typedef enum {DATE_SIZE_ONLY, NO_PARALLEL} long_opt_values;

/*!
 * @brief function display_help displays a brief manual for the program usage
 * @param my_name is the name of the binary file
 * This function is provided with its code, you don't have to implement nor modify it.
 */
void display_help(char *my_name) {
    printf("%s [options] source_dir destination_dir\n", my_name);
    printf("Options: \t-n <processes count>\tnumber of processes for file calculations\n");
    printf("         \t-h display help (this text)\n");
    printf("         \t--date_size_only disables MD5 calculation for files\n");
    printf("         \t--no-parallel disables parallel computing (cancels values of option -n)\n");
}

/*!
 * @brief init_configuration initializes the configuration with default values
 * @param the_config is a pointer to the configuration to be initialized
 */
void init_configuration(configuration_t *the_config) {
    //Initialisation de source
    char source_init[1024] = "";
    *the_config->source = *source_init;

    //Initialisation de destination
    char destination_init[1024] = "";
    *the_config->destination = *destination_init;

    //Initialisation de processes_count
    uint8_t processes_count_init = 0;
    the_config->processes_count = processes_count_init;

    //Initialisation de is_parallel
    bool is_parallel_init = 0;
    the_config->is_parallel = is_parallel_init;

    //Initialisation de uses_md5
    bool uses_md58_init = 0;
    the_config->uses_md5 = uses_md58_init;

}

/*!
 * @brief set_configuration updates a configuration based on options and parameters passed to the program CLI
 * @param the_config is a pointer to the configuration to update
 * @param argc is the number of arguments to be processed
 * @param argv is an array of strings with the program parameters
 * @return -1 if configuration cannot succeed, 0 when ok
 */
int set_configuration(configuration_t *the_config, int argc, char *argv[]) {

    if (argc < 3){
        printf("too few arguments!\n");
    }
    else {
        //source
        *the_config->source = *argv[1];

        //destination
        *the_config->destination = *argv[2];


        //Conditions à rajouter pour vérifications (voir TD/TP)
        //processes_count
        uint8_t processes_count_init = 0;
        the_config->processes_count = processes_count_init;

        //is_parallel
        bool is_parallel_init = 0;
        the_config->is_parallel = is_parallel_init;

        //uses_md5
        bool uses_md58_init = 0;
        the_config->uses_md5 = uses_md58_init;
    }
}
