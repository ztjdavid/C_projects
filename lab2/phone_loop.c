#include <stdio.h>
#include <stdlib.h>

int main() {
    char phone[11];
    int choice;

    printf("Please enter the number:\n");
    for(int i = 0; i<10; i++){
        scanf("%s", &phone[i]);
    }

    printf("Please choose the digit:\n");
    scanf("%d", &choice);

    if(choice == -1){
        printf("%s", phone);
        return 0;
    }else if(choice > -1 && choice < 10){
        printf("%c", phone[choice]);
        return 0;
    }else{
        printf("ERROR");
        return 1;
    }

}