/**
 * @brief Header file for parse_args.c
 * @file parse_args.h
 * @author Adam Zvara, xzvara01
 * @date 22.4.2021
 */

#include <stdio.h>
#include <string.h>

#ifndef __PARSE__ARGS_H__
#define __PARSE__ARGS_H__

/**
 * @struct args_t
 * @brief Structure to encapsulate args of main process
 */
typedef struct arguments {
    long int NE;            ///<Number of elves
    long int NR;            ///<Number of reindeers
    long int TE;            ///<Max elves working time
    long int TR;            ///<Max reindeer vacation time
} args_t;

/**
 * @brief Function to extract arguments from argv to args structure
 *
 * @param argc Number of arguments
 * @param argv Actual arguments - string array
 * @param args Pointer to args_t structure
 *
 * @return 0 If all arguments were "extracted" without any fault
 * @return 1 When the arguments couldn't be "extracted"
 */
int parse_arguments(int argc, char *argv[], args_t *args);

/**
 * @brief Print usage message
 */ 
void usage(void);

#endif
