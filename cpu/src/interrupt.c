#include "interrupt.h"

int fd_server_interrupt;
int fd_kernel_interrupt;
uint32_t pid_interrupted;

void interrupt_server() {

    fd_kernel_interrupt = esperar_cliente(cpu_logger, SERVER_INTERRUPT, fd_server_interrupt);

    for (;;) {

        t_package* package = package_create(NULL_HEADER);
        package_recv(package, fd_kernel_interrupt);

        if (package->msg_header == MSG_KERNEL_CPU_INTERRUPT) {

            recv_msg_kernel_cpu_interrupt(package->buffer, &pid_interrupted);
            
            pthread_mutex_lock(&mutex_interrupt);
            interrupt = true;
            pthread_mutex_unlock(&mutex_interrupt);
           
        }
        else {
            log_error(cpu_logger, "Código de mensaje no esperado: %s",
                get_string_from_msg_header(package->msg_header));
        }

        package_destroy(package);       // ToDo: eliminar con fix línea 13: t_package* package = package_create()
    }

}