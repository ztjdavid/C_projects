#include <stdio.h>
#include <stdlib.h>

int main() {
    char phone[11];
    int choice;

    for(int i = 0; i<10; i++){
        scanf("%s", &phone[i]);
    }

    scanf("%d", &choice);

    if(choice == -1){
        printf("%s\n", phone);
        return 0;
    }else if(choice > -1 && choice < 10){
        printf("%c\n", phone[choice]);
        return 0;
    }else{
        printf("ERROR\n");
        return 1;
    }

}