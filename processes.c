#include "processes.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <stdio.h>
#include <messages.h>
#include <file-properties.h>
#include <sync.h>
#include <string.h>
#include <errno.h>

/*!
 * @brief prepare prepares (only when parallel is enabled) the processes used for the synchronization.
 * @param the_config is a pointer to the program configuration
 * @param p_context is a pointer to the program processes context
 * @return 0 if all went good, -1 else
 */
int prepare(configuration_t *the_config, process_context_t *p_context) {
       if (!the_config || !p_context) return -1;

    if (!the_config->is_parallel) return 0; // Only prepare if parallel is enabled

    // Initialize necessary resources for parallel processing
    // This could include setting up message queues, allocating memory for process IDs, etc.

    return 0; // Return 0 if successful, -1 otherwise
}

/*!
 * @brief make_process creates a process and returns its PID to the parent
 * @param p_context is a pointer to the processes context
 * @param func is the function executed by the new process
 * @param parameters is a pointer to the parameters of func
 * @return the PID of the child process (it never returns in the child process)
 */
int make_process(process_context_t *p_context, process_loop_t func, void *parameters) {
    if (!p_context || !func) return -1;

    pid_t pid = fork();
    if (pid == -1) {
        // Handle error in fork
        return -1;
    } else if (pid == 0) {
        // In child process
        func(parameters);
        exit(0);
    }

    // In parent process
    return pid;
}

/*!
 * @brief lister_process_loop is the lister process function (@see make_process)
 * @param parameters is a pointer to its parameters, to be cast to a lister_configuration_t
 */
void lister_process_loop(void *parameters) {
    lister_configuration_t* config = (lister_configuration_t*) parameters;
    any_message_t msg;
    files_list_t list;
    list.head = NULL;
    list.tail = NULL;

    int msg_id = msgget(config->mq_key, 0666);

    while (msg.list_entry.op_code != COMMAND_CODE_TERMINATE){
        if (msgrcv(msg_id, &msg, sizeof(any_message_t), config->my_receiver_id, 0) != -1) {
            if (msg.analyze_file_command.op_code == COMMAND_CODE_ANALYZE_DIR) {
                make_list(&list, msg.analyze_dir_command.target);
            }

        }
    }
    send_terminate_confirm(msg_id, MSG_TYPE_TO_MAIN);
}

/*!
 * @brief analyzer_process_loop is the analyzer process function
 * @param parameters is a pointer to its parameters, to be cast to an analyzer_configuration_t
 */
void analyzer_process_loop(void *parameters) {
    lister_configuration_t* config = (lister_configuration_t*) parameters;
    any_message_t msg;

    int msg_id = msgget(config->mq_key, 0666);

    while (msg.list_entry.op_code != COMMAND_CODE_TERMINATE){
        if (msgrcv(msg_id, &msg, sizeof(any_message_t), config->my_receiver_id, 0) != -1) {
            if (msg.analyze_file_command.op_code == COMMAND_CODE_ANALYZE_DIR) {
                if ((get_file_stats(&msg.list_entry.payload)) == -1) {
                    printf("Error in the function fill_entry of the file files-list.c\n");
                    printf("The get_file_stats function failed\n");
                }
                send_analyze_file_response(msg_id, config->my_recipient_id, &msg.list_entry.payload);
            }
        }
    }


    send_terminate_confirm(msg_id, MSG_TYPE_TO_MAIN);
}

/*!
 * @brief clean_processes cleans the processes by sending them a terminate command and waiting to the confirmation
 * @param the_config is a pointer to the program configuration
 * @param p_context is a pointer to the processes context
 */
void clean_processes(configuration_t *the_config, process_context_t *p_context) {
    
    // Do nothing if not parallel
    // Send terminate
    // Wait for responses
    // Free allocated memory
    // Free the MQ
}
