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
    printf("         \t--dry-run lists the changes that would need to be synchronized but doesn't perform them\n");
    printf("         \t-v enables verbose mode\n");
}

/*!
 * @brief init_configuration initializes the configuration with default values
 * @param the_config is a pointer to the configuration to be initialized
 */
void init_configuration(configuration_t *the_config) {
    //Initialisation de source
    strcpy(the_config->source, "");

    //Initialisation de destination
    strcpy(the_config->destination, "");

    //Initialisation de processes_count
    the_config->processes_count = 1;

    //Initialisation de is_parallel
    the_config->is_parallel = true;

    //Initialisation de uses_md5
    the_config->uses_md5 = true;

    //Initialisation de is_verbose
    the_config->is_verbose = false;

    //Initialisation de is_dry_run
    the_config->is_dry_run = false;

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
        return -1;
    } else {
        //source
        strcpy(the_config->source, argv[1]);

        //destination
        strcpy(the_config->destination, argv[2]);

        //Vérification des options
        int opt = 0;
        struct option my_opts[] = {
                {.name="date-size-only",.has_arg=0,.flag=0,.val='d'},
                {.name="no-parallel",.has_arg=0,.flag=0,.val='p'},
                {.name="dry-run",.has_arg=0,.flag=0,.val='r'},
                {.name=0,.has_arg=0,.flag=0,.val=0}, // last element must be zero
        };
        while((opt = getopt_long(argc, argv, "n:v", my_opts, NULL)) != -1) {
            switch (opt) {
                case 'n':
                    the_config->processes_count = *optarg;
                    break;
                case 'v':
                    the_config->is_verbose = true;
                    break;
                case 'd':
                    the_config->uses_md5 = false;
                    break;
                case 'p':
                    the_config->is_parallel = false;
                    break;
                case 'r':
                    the_config->is_dry_run = true;
                    break;
                case 'h':
                    printf("This function analyze two repertories and synchronizes them in one direction\n"
                           "Usage : backup source destination\nPossible options : n v r d/date-size-only p/no-parallel r/dry-run\n");
                default:
                    printf("Wrong option or missing argument for option\n");
            }
        }
        if (optind < argc) {
            printf("Remaining program arguments:");
            for (int i=optind; i<argc; ++i) {
                printf(" %s", argv[i]);
            }
            printf("\n");
        }
        return 0;

    }
}
