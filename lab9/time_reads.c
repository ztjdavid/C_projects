/* The purpose of this program is to practice writing signal handling
 * functions and observing the behaviour of signals.
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

/* Message to print in the signal handling function. */
#define MESSAGE "%ld reads were done in %ld seconds.\n"

/* Global variables to store number of read operations and seconds elapsed. 
 */
long num_reads, seconds;


/* The first command-line argument is the number of seconds to set a timer to run.
 * The second argument is the name of a binary file containing 100 ints.
 * Assume both of these arguments are correct.
 */

void handler(int code){
    printf(MESSAGE, num_reads, seconds);
    exit(0);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: time_reads s filename\n");
        exit(1);
    }
    seconds = strtol(argv[1], NULL, 10);

    FILE *fp;
    if ((fp = fopen(argv[2], "r")) == NULL) {
      perror("fopen");
      exit(1);
    }
    struct sigaction new;
    new.sa_handler = handler;
    new.sa_flags = 0;

    struct itimerval timer, old;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    timer.it_value.tv_sec = seconds;
    timer.it_value.tv_usec = 0;
    setitimer(ITIMER_PROF, &timer, &old);



    /* In an infinite loop, read an int from a random location in the file,
     * and print it to stderr.
     */
    for (;;) {
        sigemptyset(&new.sa_mask);
        int num = rand()%100;
        int result;
        fseek(fp, num*sizeof(int), SEEK_SET);
        fread(&result, sizeof(int), 1, fp);
        num_reads++;
        printf("%d ", result);
        sigaction(SIGPROF, &new, NULL);

    }
    return 1; // something is wrong if we ever get here!
}
