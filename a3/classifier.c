#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>      
#include <sys/types.h>  
#include <sys/wait.h>  
#include <string.h>
#include "knn.h"
#include <math.h>

/*****************************************************************************/
/* Do not add anything outside the main function here. Any core logic other  */
/*   than what is described below should go in `knn.c`. You've been warned!  */
/*****************************************************************************/

/**
 * main() takes in the following command line arguments.
 *   -K <num>:  K value for kNN (default is 1)
 *   -d <distance metric>: a string for the distance function to use
 *          euclidean or cosine (or initial substring such as "eucl", or "cos")
 *   -p <num_procs>: The number of processes to use to test images
 *   -v : If this argument is provided, then print additional debugging information
 *        (You are welcome to add print statements that only print with the verbose
 *         option.  We will not be running tests with -v )
 *   training_data: A binary file containing training image / label data
 *   testing_data: A binary file containing testing image / label data
 *   (Note that the first three "option" arguments (-K <num>, -d <distance metric>,
 *   and -p <num_procs>) may appear in any order, but the two dataset files must
 *   be the last two arguments.
 * 
 * You need to do the following:
 *   - Parse the command line arguments, call `load_dataset()` appropriately.
 *   - Create the pipes to communicate to and from children
 *   - Fork and create children, close ends of pipes as needed
 *   - All child processes should call `child_handler()`, and exit after.
 *   - Parent distributes the test dataset among children by writing:
 *        (1) start_idx: The index of the image the child should start at
 *        (2)    N:      Number of images to process (starting at start_idx)
 *     Each child should get at most N = ceil(test_set_size / num_procs) images
 *      (The last child might get fewer if the numbers don't divide perfectly)
 *   - Parent waits for children to exit, reads results through pipes and keeps
 *      the total sum.
 *   - Print out (only) one integer to stdout representing the number of test 
 *      images that were correctly classified by all children.
 *   - Free all the data allocated and exit.
 *   - Handle all relevant errors, exiting as appropriate and printing error message to stderr
 */
void usage(char *name) {
    fprintf(stderr, "Usage: %s -v -K <num> -d <distance metric> -p <num_procs> training_list testing_list\n", name);
}

int main(int argc, char *argv[]) {

    int opt;
    int K = 1;             // default value for K
    char *dist_metric = "euclidean"; // default distant metric
    int num_procs = 1;     // default number of children to create
    int verbose = 0;       // if verbose is 1, print extra debugging statements
    int total_correct = 0; // Number of correct predictions

    while((opt = getopt(argc, argv, "vK:d:p:")) != -1) {
        switch(opt) {
        case 'v':
            verbose = 1;
            break;
        case 'K':
            K = atoi(optarg);
            break;
        case 'd':
            dist_metric = optarg;
            break;
        case 'p':
            num_procs = atoi(optarg);
            break;
        default:
            usage(argv[0]);
            exit(1);
        }
    }

    if(optind >= argc) {
        fprintf(stderr, "Expecting training images file and test images file\n");
        exit(1);
    } 

    char *training_file = argv[optind];
    optind++;
    char *testing_file = argv[optind];


  
    // Set which distance function to use
    /* You can use the following string comparison which will allow
     * prefix substrings to match:
     * 
     * If the condition below is true then the string matches
     * if (strncmp(dist_metric, "euclidean", strlen(dist_metric)) == 0){
     *      //found a match
     * }
     * (Edited)
     */ 

    double (*fptr)(Image *, Image *);
    if (strncmp(dist_metric, "euclidean", strlen(dist_metric)) == 0){
        fptr = distance_euclidean;
    }else if(strncmp(dist_metric, "cosine", strlen(dist_metric)) == 0){
        fptr = distance_cosine;
    }else{
        fprintf(stderr, "Distance function is neither 'euclidean' nor 'cosine'!\n");
    }


    // Load data sets
    if(verbose) {
        fprintf(stderr,"- Loading datasets...\n");
    }
    
    Dataset *training = load_dataset(training_file);
    if ( training == NULL ) {
        fprintf(stderr, "The data set in %s could not be loaded\n", training_file);
        exit(1);
    }

    Dataset *testing = load_dataset(testing_file);
    if ( testing == NULL ) {
        fprintf(stderr, "The data set in %s could not be loaded\n", testing_file);
        exit(1);
    }

    // Create the pipes
    if(verbose) {
        printf("- Creating children ...\n");
    }
    int fd_in[num_procs][2];
    for(int i = 0; i< num_procs;i++) {
        if (pipe(fd_in[i]) == -1) {
            perror("pipe");
        }
    }
    int fd_out[num_procs][2];
    for(int i = 0; i< num_procs;i++) {
        if (pipe(fd_out[i]) == -1) {
            perror("pipe");
        }
    }


    //Create the child processes
    pid_t pid_arr[num_procs];
    for(int i = 0; i< num_procs;i++) {
        pid_t pid = fork();
        pid_arr[i] = pid;
    }

    //Write the index and N into the pipes
    for(int i = 0; i< num_procs;i++) {
        if(pid_arr[i] > 0){
            close(fd_in[i][0]);
            int index = 0;
            if(i != num_procs -1){
                int N = ceil(testing->num_items / num_procs);
                write(fd_in[i][1], &index, sizeof(int));
                write(fd_in[i][1], &N, sizeof(int));
                index += N;
            }else if(i == num_procs -1){
                int N = testing->num_items - index;
                write(fd_in[i][1], &index, sizeof(int));
                write(fd_in[i][1], &N, sizeof(int));
            }
            close(fd_in[i][1]);
        }else if(pid_arr[i] == 0){
            close(fd_in[i][1]);
            close(fd_out[i][0]);
            child_handler(training, testing, K, fptr, fd_in[i][0], fd_out[i][1]);
            close(fd_in[i][0]);
            close(fd_out[i][1]);
        }
    }

    int status;
    for(int i = 0;i<num_procs;i++){
        wait(&status);
        if(!WIFEXITED(status)){
            fprintf(stderr, "Child process did not exit normally!");
        }
    }
    // Wait for children to finish
    if(verbose) {
        printf("- Waiting for children...\n");
    }

    // When the children have finished, read their results from their pipe
    for(int i = 0; i<num_procs;i++){
        close(fd_out[i][1]);
        int result;
        read(fd_out[i][0], &result, sizeof(int));
        total_correct += result;
        close(fd_out[i][0]);
    }


    if(verbose) {
        printf("Number of correct predictions: %d\n", total_correct);
    }

    // This is the only print statement that can occur outside the verbose check
    printf("%d\n", total_correct);

    // Clean up any memory, open files, or open pipes
    free_dataset(training);
    free_dataset(testing);


    return 0;
}
