#include "kernel.h"


int kernelDispatchFd;
int kernelInterruptFd;
int fd_memoria_init;
int fd_memoria_page_fault;
int fd_memoria_exit;

void handle_exit(int signal){
    log_info(kernel_logger,"FINALIZAMOS EL PROCESO MAIN");
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    initializeKernel(argv[1]);
    inicializar_planificadores();
    
    signal(SIGINT, handle_exit);
    sem_wait(&BLOQUEADOR);
    
}