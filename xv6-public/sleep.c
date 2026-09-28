#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
    // check if the user provided a sleep duration argument
    if(argc != 2){
        printf(2, "Error: Usage: sleep <duration>\n");
        exit();
    }

    // convert the argument to an integer
    int duration = atoi(argv[1]);

    // call the sleep system call
    sleep(duration);

    exit();

}