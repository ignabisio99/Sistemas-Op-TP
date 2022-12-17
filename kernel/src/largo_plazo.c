#include "largo_plazo.h"

u_int32_t pcb_ids;

void planificadorLargoPlazo()
 {
	 pthread_t hilo_newAReady;
	 pthread_t hilo_exit;
	 pthread_t hilo_conexion_consola;
	 pthread_create(&hilo_newAReady,NULL,(void*)enviarProcesosAReady,NULL);
	 pthread_create(&hilo_exit,NULL,(void*) terminarProcesos,NULL);
	 pthread_create(&hilo_conexion_consola,NULL,(void*) conexionConConsola,NULL);
	 pthread_detach(hilo_newAReady);
	 pthread_detach(hilo_exit);
	 pthread_detach(hilo_conexion_consola);
 }  

void conexionConConsola(){
	while(recibirAConsola());
}

void procesar_conexion(void* args){
	int args_consola_fd = ((t_args*) args)->fd;
	free(args);

	t_package* package = package_create(NULL_HEADER);
	package_recv(package, args_consola_fd);
	
	send_msg_kernel_consola_init(args_consola_fd);
	t_list* instructions = list_create();
    t_list* segments = list_create();
    
    recv_msg_consola_kernel_init(package->buffer, instructions, segments);
	log_info(kernel_logger, "Recibimos el PCB desde consola");
	
    
	
	// y fijarse que el PID tiene que tener el mutex
	pthread_mutex_lock(&MUTEX_PIDS);
	uint32_t id_pcb = ++pcb_ids;
	pthread_mutex_unlock(&MUTEX_PIDS);
	
	t_PCB* pcb = pcb_create(id_pcb,instructions,segments);
	
	t_consola_pid* consola_pid = malloc(sizeof(t_consola_pid));

	consola_pid->pid = pcb->pid;
	consola_pid->consola_socket = args_consola_fd;

	pthread_mutex_lock(&MUTEX_CONSOLAS);
	list_add(LISTA_CONSOLAS,consola_pid);
	pthread_mutex_unlock(&MUTEX_CONSOLAS);


	pthread_mutex_lock(&MUTEX_NEW);
	queue_push(COLA_NEW,pcb);
	pthread_mutex_unlock(&MUTEX_NEW);

	log_info(kernel_logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, "-", "NEW");

	sem_post(&SEM_NEW);
	

	package_destroy(package);

}



int recibirAConsola(){
	int fd_console = esperar_cliente(kernel_logger, SERVERNAME, fd_server);
	log_info(kernel_logger, "Nuevo cliente-consola recibido");

	pthread_t hilo;
	t_args* args = (t_args*) malloc(sizeof(t_args));
	args->fd = fd_console;
	pthread_create(&hilo, NULL, (void*) procesar_conexion, (void*) args);
	pthread_detach(hilo);
	return 1;	
}

void enviarProcesosAReady()
 {
	char* puerto_memoria = string_itoa(kernel_config->PUERTO_MEMORIA);
    fd_memoria_init = crear_conexion (
        kernel_logger, 
        "MEMORIA", 
        kernel_config->IP_MEMORIA, 
        puerto_memoria
    );

	fd_memoria_page_fault = crear_conexion (
        kernel_logger, 
        "MEMORIA", 
        kernel_config->IP_MEMORIA, 
        puerto_memoria
    );

	fd_memoria_exit = crear_conexion (
        kernel_logger, 
        "MEMORIA", 
        kernel_config->IP_MEMORIA, 
        puerto_memoria
    );

    while(1){
		sem_wait(&SEM_NEW);
		sem_wait(&SEM_MULTIPROGRAMACION);

		t_PCB* pcb;

		pthread_mutex_lock(&MUTEX_NEW);
			pcb = queue_pop(COLA_NEW);
		pthread_mutex_unlock(&MUTEX_NEW);

		t_list* segments = list_create();
		for(int i = 0; i < list_size(pcb->segments); i++){
			list_add(segments, list_get(pcb->segments, i));
		}

		send_msg_kernel_memoria_init(pcb->pid, segments, fd_memoria_init);
		
		list_destroy(segments);
		
		t_package* packageMemoria = package_create(NULL_HEADER);
        package_recv(packageMemoria, fd_memoria_init);
		if(packageMemoria->msg_header != MSG_MEMORIA_KERNEL_INIT){
        	log_debug(kernel_logger, "ERROR AL RECIBIR DESDE MEMORIA LA CONFIRMACION DE CREACION");
			exit(EXIT_FAILURE);
		}

		package_destroy(packageMemoria);
		
		log_info(kernel_logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, "NEW", "READY");
		agregar_de_new_a_ready(pcb);
		

	}
	
 }

t_consola_pid* find_pid(uint32_t pid, t_list* lista) {
                    int _is_the_one(t_consola_pid* p) {
                        return pid == p->pid;
                    }

                    return list_find(lista, (void*) _is_the_one);
                }

void borrar_consola_pid(uint32_t pid, t_list* lista) {
                    int _is_the_one(t_consola_pid* p) {
                        return pid == p->pid;
                    }
                list_remove_by_condition(lista, (void*) _is_the_one);
				}

                
void terminarProcesos()
 {
	while(1){
		sem_wait(&SEM_EXIT);
		t_PCB*  pcb_exit;

		log_info(kernel_logger,"empieza a terminar el proceso");

		pthread_mutex_lock(&MUTEX_EXIT);
			pcb_exit = queue_pop(COLA_EXIT);
		pthread_mutex_unlock(&MUTEX_EXIT);
		sem_post(&SEM_MULTIPROGRAMACION);
		
		pthread_mutex_lock(&MUTEX_CONSOLAS);
			t_consola_pid* consola_pid = find_pid(pcb_exit->pid,LISTA_CONSOLAS);
			borrar_consola_pid(pcb_exit->pid,LISTA_CONSOLAS);
		pthread_mutex_unlock(&MUTEX_CONSOLAS);

		send_msg_kernel_memoria_exit(pcb_exit->pid, fd_memoria_exit);

		t_package* packageMemoria = package_create(NULL_HEADER);
        package_recv(packageMemoria, fd_memoria_exit);
		if(packageMemoria->msg_header != MSG_MEMORIA_KERNEL_EXIT){
        	log_debug(kernel_logger, "ERROR AL RECIBIR DESDE MEMORIA LA CONFIRMACION DE EXIT");
			exit(EXIT_FAILURE);
		}
		package_destroy(packageMemoria);

		send_msg_kernel_consola_exit(consola_pid->consola_socket);

		liberar_conexion(consola_pid->consola_socket);
		free(consola_pid);
		pcb_destroy(pcb_exit);
	}
 }