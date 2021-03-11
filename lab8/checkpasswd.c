#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256
#define MAX_PASSWORD 11

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
  char user_id[MAXLINE];
  char password[MAXLINE];

  /* The user will type in a user name on one line followed by a password 
     on the next.
     DO NOT add any prompts.  The only output of this program will be one 
	 of the messages defined above.
     Please read the comments in validate carefully.
   */

  if(fgets(user_id, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  if(fgets(password, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  if(strlen(user_id)>MAX_PASSWORD){
      printf(NO_USER);
      exit(1);
  }
  if(strlen(password)>MAX_PASSWORD){
      printf(INVALID);
      exit(1);
  }

  int fd[2], r;
  int result = -1;

  pipe(fd);

  r = fork();

  if(r>0){


      if ((dup2(fd[1], STDIN_FILENO)) == -1) {
          perror("dup2");
          exit(1);
      }
      if(write(fd[1],user_id,MAX_PASSWORD)== -1){
          perror("write to pipe");
      }
      if(write(fd[1],password,MAX_PASSWORD)== -1){
          perror("write to pipe");
      }

      if ((close(fd[0])) == -1) {
          perror("close");
      }

      if ((close(fd[1])) == -1) {
          perror("close");
      }

      int status;
      if (wait(&status) != -1) {
          if (WIFEXITED(status)) {
              result = WEXITSTATUS(status);
          } else {
              fprintf(stderr, "validate.c terminated abnormally!");
          }
      }


      if (result == 0) {
          printf(SUCCESS);
      } else if (result == 1) {
          fprintf(stderr, "There is an error while validate.c is executing!");
      } else if (result == 2) {
          printf(INVALID);
      } else if (result == 3) {
          printf(NO_USER);
      }


}else if(r==0){

      if ((close(fd[1])) == -1) {
          perror("close");
      }
      if ((dup2(fd[0], STDIN_FILENO)) == -1) {
          perror("dup2");
          exit(1);
      }

      if ((close(fd[0])) == -1) {
          perror("close");
      }

      execl("./validate","validate",NULL);


  }else {
      perror("fork");
      exit(1);
  }


  return 0;
}
