#include "cpu.h"

int fd_memoria;

int main(int argc, char* argv[]) {

    // Inicializar CPU
    initializeCpu(argv[1]);

    // INTERRUPT Server
    // -- Thread en que escuchará los mensajes INTERRUPT enviados por KERNEL
    pthread_t THREAD_INTERRUPT;

    if (!pthread_create(&THREAD_INTERRUPT, NULL, (void*) interrupt_server, NULL))
        pthread_detach(THREAD_INTERRUPT);
    else {
        close_program();
        return EXIT_FAILURE;
    }

    // DISPATCH Server
    // -- Thread en que escuchará los mensajes DISPATCH enviados por KERNEL
    pthread_t THREAD_DISPATCH;
    
    if (!pthread_create(&THREAD_DISPATCH, NULL, (void*) dispatch_server, NULL))
        pthread_detach(THREAD_DISPATCH);
    else {
        close_program();
        return EXIT_FAILURE;
    }


    signal(SIGINT, handle_exit);
    sem_wait(&BLOQUEADOR);

}
