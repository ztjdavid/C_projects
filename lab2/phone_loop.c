#include <stdio.h>
#include <stdlib.h>

int main() {
    char phone[11];
    int choice;

    scanf("%s", phone);
    int flag = 0;
    while(scanf("%d", &choice) != EOF){
        if (choice == -1) {
            printf("%s\n", phone);
        } else if (choice > -1 && choice < 10) {
            printf("%c\n", phone[choice]);
        } else {
            printf("ERROR\n");
            flag = 1;
        }
    }

    if (flag == 1){
        return 1;
    }
    else{
        return 0;
    }

}