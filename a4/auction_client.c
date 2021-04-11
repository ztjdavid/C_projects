#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#define BUF_SIZE 128

#define MAX_AUCTIONS 5
#ifndef VERBOSE
#define VERBOSE 0
#endif

#define ADD 0
#define SHOW 1
#define BID 2
#define QUIT 3

/* Auction struct - this is different than the struct in the server program
 */
struct auction_data {
    int sock_fd;
    char item[BUF_SIZE];
    int current_bid;
};

/* Displays the command options available for the user.
 * The user will type these commands on stdin.
 */

void print_menu() {
    printf("The following operations are available:\n");
    printf("    show\n");
    printf("    add <server address> <port number>\n");
    printf("    bid <item index> <bid value>\n");
    printf("    quit\n");
}

/* Prompt the user for the next command 
 */
void print_prompt() {
    printf("Enter new command: ");
    fflush(stdout);
}


/* Unpack buf which contains the input entered by the user.
 * Return the command that is found as the first word in the line, or -1
 * for an invalid command.
 * If the command has arguments (add and bid), then copy these values to
 * arg1 and arg2.
 */
int parse_command(char *buf, int size, char *arg1, char *arg2) {
    int result = -1;
    char *ptr = NULL;
    if(strncmp(buf, "show", strlen("show")) == 0) {
        return SHOW;
    } else if(strncmp(buf, "quit", strlen("quit")) == 0) {
        return QUIT;
    } else if(strncmp(buf, "add", strlen("add")) == 0) {
        result = ADD;
    } else if(strncmp(buf, "bid", strlen("bid")) == 0) {
        result = BID;
    } 
    ptr = strtok(buf, " "); // first word in buf

    ptr = strtok(NULL, " "); // second word in buf
    if(ptr != NULL) {
        strncpy(arg1, ptr, BUF_SIZE);
    } else {
        return -1;
    }
    ptr = strtok(NULL, " "); // third word in buf
    if(ptr != NULL) {
        strncpy(arg2, ptr, BUF_SIZE);
        return result;
    } else {
        return -1;
    }
    return -1;
}

/* Connect to a server given a hostname and port number.
 * Return the socket for this server
 */
int add_server(char *hostname, int port) {
        // Create the socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("client: socket");
        exit(1);
    }
    
    // Set the IP and port of the server to connect to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    struct addrinfo *ai;
    
    /* this call declares memory and populates ailist */
    if(getaddrinfo(hostname, NULL, NULL, &ai) != 0) {
        close(sock_fd);
        return -1;
    }
    /* we only make use of the first element in the list */
    server.sin_addr = ((struct sockaddr_in *) ai->ai_addr)->sin_addr;

    // free the memory that was allocated by getaddrinfo for this list
    freeaddrinfo(ai);

    // Connect to the server.
    if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("client: connect");
        close(sock_fd);
        return -1;
    }
    if(VERBOSE){
        fprintf(stderr, "\nDebug: New server connected on socket %d.  Awaiting item\n", sock_fd);
    }
    return sock_fd;
}
/* ========================= Add helper functions below ========================
 * Please add helper functions below to make it easier for the TAs to find the 
 * work that you have done.  Helper functions that you need to complete are also
 * given below.
 */

void slice(int start, int end, char *buf, const char *str){
    int index = 0;
    for(int i = start; i < end; i++){
        buf[index] = str[i];
        index++;
    }
    buf[index] = 0;
}

/* Print to standard output information about the auction
 */
void print_auctions(struct auction_data *a, int size) {
    printf("Current Auctions:\n");

    for(int i = 0; i < size; i++){
        if(a[i].sock_fd != -1) {
            int bid = a[i].current_bid;
            printf("(%d) %s bid = %d\n", i, a[i].item, bid);
        }
    }

     /*
     * server.  Use the follow string format string:
     *     "(%d) %s bid = %d\n", index, item, current bid
     * The array may have some elements where the auction has closed and
     * should not be printed.
     */

}



/* Process the input that was sent from the auction server at a[index].
 * If it is the first message from the server, then copy the item name
 * to the item field.  (Note that an item cannot have a space character in it.)
 */
void update_auction(char *buf, int size, struct auction_data *a, int index) {
    int space_count = 0;
    char item[BUF_SIZE];
    int highest_bid = 0;
    int time = 0;
    int start_index = 0;
    for(int i = 0; i<strlen(buf); i++){
        if(buf[i] == ' '){
            if(space_count == 0){
                slice(start_index, i, item, buf);
                item[i] = '\0';
                start_index = i+1;
                space_count++;
            }else if(space_count == 1){
                char temp[BUF_SIZE];
                slice(start_index, i, temp, buf);
                highest_bid = strtol(temp, NULL, 10);
                start_index = i+1;
                space_count++;
            }
        }
    }
    if(space_count ==2){
        char temp2[128];
        slice(start_index, strlen(buf), temp2, buf);
        time = strtol(temp2, NULL, 10);
    }else {
        fprintf(stderr, "ERROR malformed bid: %s", buf);
        return;
    }

    if(a[index].current_bid == -2){
        strncpy(a[index].item, item, strlen(item));
        a[index].current_bid = highest_bid;
    }else{
        a[index].current_bid = highest_bid;
        printf("\nNew bid for %s [%d] is %d (%d seconds left)\n", a[index].item, index, a[index].current_bid, time);
    }

}





int main(void) {

    char name[BUF_SIZE];

    // Declare and initialize necessary variables
    char response[BUF_SIZE];
    int sizeof_response;
    char arg1[BUF_SIZE];
    char arg2[BUF_SIZE];
    int choice;
    char server_buf[BUF_SIZE];
    fd_set read_fds;
    FD_ZERO(&read_fds);
    int max_fd = STDIN_FILENO;

    struct auction_data auction[MAX_AUCTIONS];
    for(int i = 0; i < MAX_AUCTIONS; i++){
        auction[i].sock_fd = -1;
        auction[i].current_bid = -2;
    }

    // Get the user to provide a name.
    printf("Please enter a username: ");
    fflush(stdout);
    int num_read = read(STDIN_FILENO, name, BUF_SIZE);
    if(num_read <= 0){
        fprintf(stderr, "ERROR: read from stdin failed\n");
        exit(1);
    }
    print_menu();



    while(1) {
        //Print please enter a command
        print_prompt();

        FD_SET(STDIN_FILENO, &read_fds);
        for(int i = 0; i < MAX_AUCTIONS; i++){
            if(auction[i].sock_fd != -1){
                FD_SET(auction[i].sock_fd, &read_fds);
            }
        }
        if (select(max_fd+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("ERROR: select");
            exit(1);
        }

        //checking message from the stander input
        if(FD_ISSET(STDIN_FILENO, &read_fds)){
            sizeof_response = read(STDIN_FILENO, response, BUF_SIZE);
            if(sizeof_response <= 0){
                fprintf(stderr, "ERROR: read from stdin failed\n");
                exit(1);
            }
            choice = parse_command(response, sizeof_response, arg1, arg2);

            //If User enters: "show"
            if(choice == SHOW){
                print_auctions(auction, MAX_AUCTIONS);
            }

            //If User enters: "add <server address> <port number>"
            else if(choice == ADD){
                int available_index = -1;

                //find an  index in auction struct with empty space for the new server
                for(int i = 0; i<MAX_AUCTIONS; i++){
                    if(auction[i].sock_fd == -1){
                        available_index = i;
                        break;
                    }
                }
                if(available_index == -1){
                    fprintf(stderr, "Max number server reached, adding failed!");
                    continue;
                }
                int port = strtol(arg2, NULL, 10);
                int fd = add_server(arg1, port);
                auction[available_index].sock_fd = fd;
                FD_SET(fd, &read_fds);
                if(fd > max_fd){
                    max_fd = fd;
                }
                if(write(auction[available_index].sock_fd, name, strlen(name)) != strlen(name)){
                    fprintf(stderr, "ERROR: writing name to server\n");
                    exit(1);
                }
            }

            //If User enters: "bid <item index> <bid value>"
            else if(choice == BID){
                int index = strtol(arg1, NULL, 10);
                if(write(auction[index].sock_fd, arg2, strlen(arg2)) != strlen(arg2)){
                    fprintf(stderr, "ERROR: writing bid to server\n");
                    exit(1);
                }
            }

            //If User enters: "quit"
            else if(choice == QUIT){
                for(int i = 0; i<MAX_AUCTIONS; i++){
                    if(auction[i].sock_fd != -1) {
                        close(auction[i].sock_fd);
                    }
                }
                return 0;
            }
        }

        //checking message from servers
        for(int i = 0; i < MAX_AUCTIONS; i++) {
            if(auction[i].sock_fd > -1){
                if (FD_ISSET(auction[i].sock_fd, &read_fds)) {
                    num_read = read(auction[i].sock_fd, server_buf, BUF_SIZE);
                    if (num_read <= 0) {
                        fprintf(stderr, "ERROR: read from server failed\n");
                        exit(1);
                    }

                    //Is the auction closed?
                    if (strncmp("Auction closed", server_buf, 13) == 0) {
                        printf("%s", server_buf);
                        close(auction[i].sock_fd);
                        auction[i].sock_fd = -1;
                    }


                    //If the auction is not closed, update auction struct
                    else {
                        update_auction(server_buf, MAX_AUCTIONS, auction, i);
                    }
                }
            }
        }

    }
    return 0; // Shouldn't never get here
}
