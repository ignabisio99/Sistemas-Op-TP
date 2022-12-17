#include "memoria.h"

int main(int argc, char* argv[]){
    initializeMemoria(argv[1]);

    memory_server();

    signal(SIGINT, handle_exit);
    sem_wait(&BLOQUEADOR);
}



