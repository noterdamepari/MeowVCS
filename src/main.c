#include "meow.h"

int main(int argc, char** argv){
    const char* default_dir = "/.meow";
    switch (argc){
    case 2:{
        if (!strcmp(argv[1], "init")){
            char buf[100];
            getcwd(buf, 100);
            strcat(buf, default_dir);
            mkdir(buf, 0777);
        }
        break;
    } 
    default:
        puts("Err: No arguments");
        break;
    }
    return 0;
}
