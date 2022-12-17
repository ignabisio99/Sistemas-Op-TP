#include "protocolo.h"

/*
 *      Buffer & package functions
 *  
 *      Buffer create       t_buffer* buffer_create()
 *      Buffer destroy      void buffer_destroy(t_buffer* buffer) 
 *      Buffer add          void buffer_add(t_buffer* buffer, void* stream, uint32_t size)
 *  
 *      Package create      t_package* package_create(t_msg_header msg_header)
 *      Package destroy     void package_destroy(t_package* package)
 *      Package send        int package_send(t_package* package, int fd)
 *      Package receive     int package_recv(t_package* package, int fd)
 * 
 */

/*
 *      Buffer
 */

// Buffer create
t_buffer* buffer_create() {

    t_buffer* buffer = malloc(sizeof(t_buffer));
    
    buffer->size = 0;
    buffer->stream = NULL;

    return buffer;

}

// Buffer destroy
void buffer_destroy(t_buffer* buffer) {
    if(buffer->stream != 0){
        free(buffer->stream);
    }
    free(buffer);

}

// Buffer add
void buffer_add(t_buffer* buffer, void* stream, uint32_t size) {

    buffer->stream = realloc(buffer->stream, buffer->size + size + sizeof(uint32_t));

    memcpy(buffer->stream + buffer->size, &size, sizeof(uint32_t));
    memcpy(buffer->stream + buffer->size + sizeof(uint32_t), stream, size);

    buffer->size += size + sizeof(uint32_t);

}

/*
 *      Package
 */

// Package create 
t_package* package_create(t_msg_header msg_header) {

    t_package* package = malloc(sizeof(t_package));

    package->msg_header = msg_header;
    package->buffer = buffer_create();

    return package;

}

// Package destroy
void package_destroy(t_package* package) {
    if(package->buffer != 0){
        buffer_destroy(package->buffer);
    }
    
    free(package);
}

// Package send
int package_send(t_package* package, int fd) {

    size_t size     =   sizeof(t_msg_header)    // Message header
                    +   sizeof(uint32_t)        // Buffer size
                    +   package->buffer->size;  // Buffer
    
    void* stream = malloc(size);
    int offset = 0;
    
    memcpy(stream + offset, &(package->msg_header), sizeof(t_msg_header));
    offset += sizeof(t_msg_header);
    memcpy(stream + offset, &(package->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, package->buffer->stream, package->buffer->size);

    send(fd, stream, size, 0);

    free(stream);

    return EXIT_SUCCESS;
}

// Abstraction of sockets recv method. It should check if the connection has ended
void _recv_and_check_response(int fd, void* buf,size_t size,int flags){
    ssize_t recv_response = recv(fd, buf, size, flags);
    if(recv_response == 0){
        printf("SE HA DESCONECTADO EL SOCKET. ABORTANDO PROCESO\n");
        exit(1); //TODO para resolver memoryleaks se debería hacer un graceful-exit
    }
}

// Package receive
// El package debe ser creado con package_create
int package_recv(t_package* package, int fd) {

    _recv_and_check_response(fd, &(package->msg_header), sizeof(t_msg_header), 0);
    
    uint32_t size;
    _recv_and_check_response(fd, &size, sizeof(uint32_t), 0);

    if(size == 0){
        return EXIT_SUCCESS;
    }

    void* stream = malloc(size);
    _recv_and_check_response(fd, stream, size, 0);

    package->buffer->size = size;
    package->buffer->stream = stream;

    return EXIT_SUCCESS;

}


/*
 *      Serialize & deserialize: General functions
 *      
 *      Serialize
 *      Instruction list    void serialize_instruction_list(t_buffer* buffer, t_list* instructions)
 *      Instruction         void serialize_instruction(t_buffer* buffer, t_instruction* instruction) 
 *      Segment list        void serialize_segment_list(t_buffer* buffer, t_list* segments)
 *      CPU registers       void serialize_cpu_registers(t_buffer* buffer, t_cpu_registers* cpu_registers)
 *      PCB                 void serialize_pcb(t_buffer* buffer, t_PCB* pcb)
 * 
 *      Deserialize
 *      Instruction list    void deserialize_instruction_list(t_buffer* buffer, t_list* instructions)
 *      Instruction         void deserialize_instruction(t_buffer* buffer, t_instruction* instruction) 
 *      Segment list        void deserialize_segment_list(t_buffer* buffer, t_list* segments)
 *      CPU registers       void deserialize_cpu_registers(t_buffer* buffer, t_cpu_registers* cpu_registers)
 *      PCB                 void deserialize_pcb(t_buffer* buffer, t_PCB* pcb)
 * 
 */

/*
 *  Serialize
 */

// Instruction list
void serialize_instruction_list(t_buffer* buffer, t_list* instructions) {

    t_instruction* instruction;
    size_t size_list = list_size(instructions);

    for (int i = 0; i < size_list; i++) {
        
        t_buffer* buffer_aux = buffer_create();
        
        instruction = list_get(instructions, i);
        serialize_instruction(buffer_aux, instruction);
        buffer_add(buffer, buffer_aux->stream, buffer_aux->size);
        buffer_destroy(buffer_aux);
    }
        
}

// Instruction
void serialize_instruction(t_buffer* buffer, t_instruction* instruction) {

    uint32_t length_param_1 = strlen(instruction->param_1) + 1;
    uint32_t length_param_2 = strlen(instruction->param_2) + 1;

    size_t size =   sizeof(t_instruction_code)  // Instruction code
                +   sizeof(uint32_t) * 2        // Length parameters
                +   length_param_1              // Parameter 1
                +   length_param_2;             // Parameter 2

    void* stream = malloc(size);
    int offset = 0;

    memcpy(stream + offset, &(instruction->instruction_code), sizeof(t_instruction_code));
    offset += sizeof(t_instruction_code);
    memcpy(stream + offset, &length_param_1, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, instruction->param_1, length_param_1);
    offset += length_param_1;
    memcpy(stream + offset, &length_param_2, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, instruction->param_2, length_param_2);

    buffer->size = size;
    buffer->stream = stream;

}

// Segment list
void serialize_segment_list(t_buffer* buffer, t_list* segments) {

    size_t size_list = list_size(segments);
    t_segment* segment;

    void* stream = malloc(sizeof(t_segment) * size_list);    
    int offset = 0;

    for (int i = 0; i < size_list; i++) {
        
        segment = list_get(segments, i);
        memcpy(stream + offset, &(segment->segment), sizeof(uint32_t));
        offset += sizeof(uint32_t);
        memcpy(stream + offset, &(segment->page), sizeof(uint32_t));
        offset += sizeof(uint32_t);
        
    }
    buffer->stream = stream;
    buffer->size = offset;
}

// Segment
void serialize_segment(t_buffer* buffer, t_segment* segment) {

    buffer->size = sizeof(t_segment);
    void* stream = malloc(buffer->size);
    
    memcpy(stream, &(segment->segment), sizeof(uint32_t));
    int offset = sizeof(uint32_t);
    memcpy(stream + offset, &(segment->page), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    buffer->stream = stream;

}

// CPU registers
void serialize_cpu_registers(t_buffer* buffer, t_cpu_registers* cpu_registers) {

    // CPU registers: AX, BX, CX, DX
    buffer->size = sizeof(t_cpu_registers);
    void* stream = malloc(sizeof(t_cpu_registers));
        
    memcpy(stream, &(cpu_registers->ax), sizeof(uint32_t));
    int offset = sizeof(uint32_t);
    memcpy(stream + offset, &(cpu_registers->bx), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &(cpu_registers->cx), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(stream + offset, &(cpu_registers->dx), sizeof(uint32_t));

    buffer->stream = stream;

}

// PCB
void serialize_pcb(t_buffer* buffer, t_PCB* pcb) {

    void* stream = malloc(sizeof(t_PCB));

    // PID
    memcpy(stream, &(pcb->pid), sizeof(uint32_t));
    int offset = sizeof(uint32_t);
 
    // PC
    memcpy(stream + offset, &(pcb->pc), sizeof(uint32_t));
    offset += sizeof(uint32_t);

    // CPU registers
    t_buffer* buffer_cpu_registers = buffer_create();
    serialize_cpu_registers(buffer_cpu_registers, pcb->cpu_registers);
    memcpy(stream + offset, buffer_cpu_registers->stream, sizeof(t_cpu_registers));
    offset += sizeof(t_cpu_registers);
  
    buffer->stream = stream;
    buffer->size = offset;

    // Segments
    t_buffer* buffer_segments = buffer_create();
    serialize_segment_list(buffer_segments, pcb->segments);
    buffer_add(buffer, buffer_segments->stream, buffer_segments->size);
 
    // Instructions
    t_buffer* buffer_instructions  = buffer_create();
    serialize_instruction_list(buffer_instructions, pcb->instructions);
    buffer_add(buffer, buffer_instructions->stream, buffer_instructions->size);


    buffer_destroy(buffer_cpu_registers);
    buffer_destroy(buffer_segments);
    buffer_destroy(buffer_instructions);
}


/*
 *  Deserialize
 */

// Instruction list
void deserialize_instruction_list(t_buffer* buffer, t_list* instructions) {

    void* stream = buffer->stream;
    uint32_t size = buffer->size;

    void* stream_aux = NULL;
    uint32_t size_aux = 0;

    for (; size > 0;) {

        t_instruction* instruction = malloc(sizeof(t_instruction));

        // Buffer size
        memcpy(&size_aux, stream, sizeof(uint32_t));
        stream += sizeof(uint32_t);
        size -= sizeof(uint32_t);

        // Buffer stream
        stream_aux = malloc(size_aux);
        memcpy(stream_aux, stream, size_aux);
        stream += size_aux;
        size -= size_aux;
        
        // Buffer aux
        t_buffer* buffer_aux = buffer_create();
        buffer_aux->stream = stream_aux;
        buffer_aux->size = size_aux;

        // Deserialize instrucion & add to instructions list
        deserialize_instruction(buffer_aux, instruction);
        list_add(instructions, instruction);

        buffer_destroy(buffer_aux);
    }

}

// Instruction
void deserialize_instruction(t_buffer* buffer, t_instruction* instruction) {
    
    void* stream = buffer->stream;
    uint32_t length_param = 0;

    // Instruction code
    memcpy(&(instruction->instruction_code), stream, sizeof(t_instruction_code));
    stream += sizeof(t_instruction_code);
    
    // Parameter 1
    memcpy(&length_param, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    instruction->param_1 = malloc(length_param);
    memcpy(instruction->param_1, stream, length_param);
    stream += length_param;

    // Parameter 2
    memcpy(&length_param, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    instruction->param_2 = malloc(length_param);
    memcpy(instruction->param_2, stream, length_param);
    stream += length_param;

}

// Segment list
void deserialize_segment_list(t_buffer* buffer, t_list* segments) {

    size_t size_list = buffer->size / sizeof(t_segment);
    void* stream = buffer->stream;    
           
    for (int i = 0; i < size_list; i++) {

        t_segment* segment = segment_create();

        memcpy(&(segment->segment), stream, sizeof(uint32_t));
        stream += sizeof(uint32_t);
        memcpy(&(segment->page), stream, sizeof(uint32_t));
        stream += sizeof(uint32_t);
        list_add(segments, segment);
    }
}

// Segment
void deserialize_segment(t_buffer* buffer, t_segment* segment) {

    void* stream = buffer->stream;

    memcpy(&(segment->segment), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(segment->page), stream, sizeof(uint32_t));

}

// CPU registers
void deserialize_cpu_registers(t_buffer* buffer, t_cpu_registers* cpu_registers) {

    // CPU registers: AX, BX, CX, DX
    void* stream = buffer->stream;

    memcpy(&(cpu_registers->ax), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(cpu_registers->bx), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(&(cpu_registers->cx), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);    
    memcpy(&(cpu_registers->dx), stream, sizeof(uint32_t));
    
}

// PCB
void deserialize_pcb(t_buffer* buffer, t_PCB* pcb) {

    t_buffer* buffer_cpu_registers = buffer_create();
    t_buffer* buffer_instructions = buffer_create();
    t_buffer* buffer_segments = buffer_create();
    uint32_t size_aux = 0;

    void* stream = buffer->stream;

    // PID
    memcpy(&(pcb->pid), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    // PC
    memcpy(&(pcb->pc), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
 
    // CPU registers
    void* stream_aux = malloc(sizeof(t_cpu_registers));
    memcpy(stream_aux, stream, (sizeof(t_cpu_registers)));
    stream += sizeof(t_cpu_registers);

    buffer_cpu_registers->stream = stream_aux;
    buffer_cpu_registers->size = sizeof(t_cpu_registers);

    pcb->cpu_registers = malloc(sizeof(t_cpu_registers));
    deserialize_cpu_registers(buffer_cpu_registers, pcb->cpu_registers);
    buffer_destroy(buffer_cpu_registers);

    // Segments
    memcpy(&size_aux, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    
    stream_aux = malloc(size_aux);
    memcpy(stream_aux, stream, size_aux);
    stream += size_aux;
    
    buffer_segments->stream = stream_aux;
    buffer_segments->size = size_aux;

    pcb->segments = list_create();
    deserialize_segment_list(buffer_segments, pcb->segments);
    buffer_destroy(buffer_segments);

    // Instructions
    memcpy(&size_aux, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    
    stream_aux = malloc(size_aux);
    memcpy(stream_aux, stream, size_aux);

    buffer_instructions->stream = stream_aux;
    buffer_instructions->size = size_aux;

    pcb->instructions = list_create();
    deserialize_instruction_list(buffer_instructions, pcb->instructions);
    buffer_destroy(buffer_instructions);

}

// Serialize t_list* uint32_t 
void serialize_list_uint32_t(t_buffer* buffer, t_list* values) {

    size_t size_list = list_size(values);
    buffer->size = size_list * sizeof(uint32_t);
    void* stream = malloc(buffer->size);

    int offset = 0;
    uint32_t* segment;
    
    for (int i = 0; i < size_list; i++) {

        segment = list_get(values, i); 
        memcpy(stream + offset, segment, sizeof(uint32_t));
        offset += sizeof(uint32_t);

    }

    buffer->stream = stream;
}

// Deserialize t_list* uint32_t
void deserialize_list_uint32_t(t_buffer* buffer, t_list* values) {

    size_t size_list = buffer->size / sizeof(uint32_t);
    void* stream = buffer->stream;
       
    for (int i = 0; i < size_list; i++) {
       
        uint32_t* aux = malloc(sizeof(uint32_t));
        memcpy(aux, stream, sizeof(uint32_t));
        stream += sizeof(uint32_t);
        list_add(values, aux);

    }
}

// Serialize uint32_t with a variable number of arguments
void serialize_uint32_t(t_buffer* buffer, int args_qty, ...) {

    buffer->size = sizeof(uint32_t) * args_qty;
    void* stream = malloc(buffer->size);
    
    va_list valist;
    va_start(valist, args_qty);             // Inicializa valist con la cantidad de argumentos recibidos
   
    int offset = 0;
    uint32_t value = 0;
    
    for (int i = 0; i < args_qty; i++) {    // Procesa argumentos de valist

        value = va_arg(valist, uint32_t);
        memcpy(stream + offset, &value, sizeof(uint32_t));
        offset += sizeof(uint32_t);

    }

    va_end(valist);                         // Libera memoria reservada para valist

    buffer->stream = stream;

}

// Deserialize uint32_t with a variable number of arguments
void deserialize_uint32_t(t_buffer* buffer, int args_qty, ...) {

    void* stream = buffer->stream;
    
    va_list valist;
    va_start(valist, args_qty);             // Inicializa valist con la cantidad de argumentos recibidos

    uint32_t* ptr_value;
   
    for (int i = 0; i < args_qty; i++) {    // Procesa argumentos de valist

        ptr_value = va_arg(valist, uint32_t*);
        memcpy(ptr_value, stream, sizeof(uint32_t));        
        stream += sizeof(uint32_t);
    
    }

    va_end(valist);                         // Libera memoria reservada para valist

}

/*
 *      Consola & Kernel: serialize, deserialize, send & receive messages
 *      
 *      Serialize messages:
 *      void serialize_msg_consola_kernel_init(t_buffer* buffer, t_list* instructions, t_list* segments)
 *      void serialize_msg_kernel_consola_print(t_buffer* buffer, uint32_t value)
 *      void serialize_msg_consola_kernel_input(t_buffer* buffer, uint32_t value) 
 *      
 *      Deserialize messages: 
 *      void deserialize_msg_consola_kernel_init(t_buffer* buffer, t_list* instructions, t_list* segments)
 *      void deserialize_msg_kernel_consola_print(t_buffer* buffer, uint32_t* value)
 *      void deserialize_msg_consola_kernel_input(t_buffer* buffer, uint32_t* value)
 * 
 *      Send messages: 
 *      int send_msg_consola_kernel_init(t_list* instructions, t_list* segments, int fd)
 *      int send_msg_kernel_consola_init(int fd)
 *      int send_msg_kernel_consola_print(uint32_t value, int fd)
 *      int send_msg_consola_kernel_print(int fd)
 *      int send_msg_kernel_consola_input(int fd)
 *      int send_msg_consola_kernel_input(uint32_t value, int fd)
 *      int send_msg_kernel_consola_exit(int fd)
 *      
 *      Receive messages:
 *      int recv_msg_consola_kernel_init(t_buffer* buffer, t_list* instructions, t_list* segments)
 *      int recv_msg_kernel_consola_print(t_buffer* buffer, uint32_t* value)
 *      int recv_msg_consola_kernel_input(t_buffer* buffer, uint32_t* value)
 *
 */  


/*
 *      Consola & Kernel: serialize messages
 */

// MSG_CONSOLA_KERNEL_INIT
void serialize_msg_consola_kernel_init(t_buffer* buffer, t_list* instructions, t_list* segments) {

    t_buffer* buffer_instructions = buffer_create();    
    serialize_instruction_list(buffer_instructions, instructions);
    buffer_add(buffer, buffer_instructions->stream, buffer_instructions->size);

    t_buffer* buffer_aux = buffer_create();
    serialize_segment_list(buffer_aux, segments);
    buffer_add(buffer, buffer_aux->stream, buffer_aux->size);
    
    buffer_destroy(buffer_instructions);
    buffer_destroy(buffer_aux);

}

// MSG_KERNEL_CONSOLA_PRINT
void serialize_msg_kernel_consola_print(t_buffer* buffer, uint32_t value) {

    size_t size = sizeof(uint32_t);
    
    buffer->size = size;
    buffer->stream = malloc(size);
    memcpy(buffer->stream, &value, sizeof(uint32_t));
}

// MSG_CONSOLA_KERNEL_INPUT
void serialize_msg_consola_kernel_input(t_buffer* buffer, uint32_t value) {
        
    size_t size = sizeof(uint32_t);
    
    buffer->size = size;
    buffer->stream = malloc(size);
    memcpy(buffer->stream, &value, sizeof(uint32_t));

}


/*
 *      Consola & Kernel: deserialize messages
 */

// MSG_CONSOLA_KERNEL_INIT
void deserialize_msg_consola_kernel_init(t_buffer* buffer, t_list* instructions, t_list* segments) {
    
    t_buffer* buffer_instructions = buffer_create();
    t_buffer* buffer_segments = buffer_create();
    uint32_t size_aux = 0;

    void* stream = buffer->stream;

    // Instructions
    memcpy(&size_aux, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    
    void* stream_aux = malloc(size_aux);
    memcpy(stream_aux, stream, size_aux);
    stream += size_aux;

    buffer_instructions->stream = stream_aux;
    buffer_instructions->size = size_aux;
    
    deserialize_instruction_list(buffer_instructions, instructions);
    buffer_destroy(buffer_instructions);

    // Segments
    memcpy(&size_aux, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    
    stream_aux = malloc(size_aux);
    memcpy(stream_aux, stream, size_aux);
    
    buffer_segments->stream = stream_aux;
    buffer_segments->size = size_aux;

    deserialize_segment_list(buffer_segments, segments);
    buffer_destroy(buffer_segments);
}

// MSG_KERNEL_CONSOLA_PRINT 
void deserialize_msg_kernel_consola_print(t_buffer* buffer, uint32_t* value) {

    memcpy(value, buffer->stream, sizeof(uint32_t));

}

// MSG_CONSOLA_KERNEL_INPUT
void deserialize_msg_consola_kernel_input(t_buffer* buffer, uint32_t* value) {

    memcpy(value, buffer->stream, sizeof(uint32_t));

}


/*
 *      Consola & Kernel: send messages
 */

// MSG_CONSOLA_KERNEL_INIT
int send_msg_consola_kernel_init(t_list* instructions, t_list* segments, int fd) {

    t_package* package = package_create(MSG_CONSOLA_KERNEL_INIT);

    serialize_msg_consola_kernel_init(package->buffer, instructions, segments);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;

}

// MSG_KERNEL_CONSOLA_INIT
int send_msg_kernel_consola_init(int fd) {

    t_package* package = package_create(MSG_KERNEL_CONSOLA_INIT);

    package_send(package, fd);
 
    package_destroy(package);

    return EXIT_SUCCESS;
}

// MSG_KERNEL_CONSOLA_PRINT
int send_msg_kernel_consola_print(uint32_t value, int fd) {

    t_package* package = package_create(MSG_KERNEL_CONSOLA_PRINT);

    serialize_msg_kernel_consola_print(package->buffer, value);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}

// MSG_CONSOLA_KERNEL_PRINT
int send_msg_consola_kernel_print(int fd) {

    t_package* package = package_create(MSG_CONSOLA_KERNEL_PRINT);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}

// MSG_KERNEL_CONSOLA_INPUT
int send_msg_kernel_consola_input(int fd) {

    t_package* package = package_create(MSG_KERNEL_CONSOLA_INPUT);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}

// MSG_CONSOLA_KERNEL_INPUT
int send_msg_consola_kernel_input(uint32_t value, int fd) {

    t_package* package = package_create(MSG_CONSOLA_KERNEL_INPUT);

    serialize_msg_consola_kernel_input(package->buffer, value);
    
    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}

// MSG_KERNEL_CONSOLA_EXIT
int send_msg_kernel_consola_exit(int fd) {

    t_package* package = package_create(MSG_KERNEL_CONSOLA_EXIT);

    package_send(package, fd);
 
    package_destroy(package);

    return EXIT_SUCCESS;
}


/*
 *      Consola & Kernel: receive messages
 */

// MSG_CONSOLA_KERNEL_INIT
int recv_msg_consola_kernel_init(t_buffer* buffer, t_list* instructions, t_list* segments) {

    deserialize_msg_consola_kernel_init(buffer, instructions, segments);
        
    return EXIT_SUCCESS;

}

// MSG_KERNEL_CONSOLA_PRINT
int recv_msg_kernel_consola_print(t_buffer* buffer, uint32_t* value) {

    deserialize_msg_kernel_consola_print(buffer, value);

    return EXIT_SUCCESS;

}

// MSG_KERNEL_CONSOLA_INPUT
int recv_msg_consola_kernel_input(t_buffer* buffer, uint32_t* value) {

    deserialize_msg_consola_kernel_input(buffer, value);

    return EXIT_SUCCESS;

}


/*
 *      Kernel & CPU: serialize, deserialize, send & receive messages
 *      
 *   
 *      Serialize messages:
 *      void serialize_msg_kernel_cpu_interrupt(t_buffer* buffer, uint32_t pid)
 *      void serialize_msg_cpu_kernel_interrupt(t_buffer* buffer, t_PCB* pcb)
 *      void serialize_msg_kernel_cpu_exec(t_buffer* buffer, t_PCB* pcb)
 *      void serialize_msg_cpu_kernel_io(t_buffer* buffer, t_PCB* pcb)
 *      void serialize_msg_cpu_kernel_exit(t_buffer* buffer, t_PCB* pcb)
 *      void serialize_msg_cpu_kernel_page_fault(t_buffer* buffer, t_PCB* pcb, uint32_t segment, uint32_t page)
 *      void serialize_msg_cpu_kernel_sigsegv(t_buffer* buffer, t_PCB* pcb)
 * 
 *      Deserialize messages:
 *      void deserialize_msg_kernel_cpu_interrupt(t_buffer* buffer, uint32_t* pid) 
 *      void deserialize_msg_cpu_kernel_interrupt(t_buffer* buffer, t_PCB* pcb)
 *      void deserialize_msg_kernel_cpu_exec(t_buffer* buffer, t_PCB* pcb)
 *      void deserialize_msg_cpu_kernel_io(t_buffer* buffer, t_PCB* pcb)
 *      void deserialize_msg_cpu_kernel_exit(t_buffer* buffer, t_PCB* pcb)
 *      void deserialize_msg_cpu_kernel_page_fault(t_buffer* buffer, t_PCB* pcb, uint32_t* segment, uint32_t* page)
 *      void deserialize_msg_cpu_kernel_sigsegv(t_buffer* buffer, t_PCB* pcb)
 * 
 *      Send messages:
 *      int send_msg_kernel_cpu_interrupt(uint32_t pid, int fd)
 *      int send_msg_cpu_kernel_interrupt(t_PCB* pcb, int fd)
 *      int send_msg_kernel_cpu_exec(t_PCB* pcb, int fd)
 *      int send_msg_cpu_kernel_io(t_PCB* pcb, int fd)
 *      int send_msg_cpu_kernel_exit(t_PCB* pcb, int fd)
 *      int send_msg_cpu_kernel_page_fault(t_PCB* pcb, uint32_t segment, uint32_t page, int fd)
 *      int send_msg_cpu_kernel_sigsegv(t_PCB* pcb, int fd)
 *      
 *      Receive messages:
 *      int recv_msg_kernel_cpu_interrupt(t_buffer* buffer, uint32_t* pid)
 *      int recv_msg_cpu_kernel_interrupt(t_buffer* buffer, t_PCB* pcb)
 *      int recv_msg_kernel_cpu_exec(t_buffer* buffer, t_PCB* pcb)
 *      int recv_msg_cpu_kernel_io(t_buffer* buffer, t_PCB* pcb)
 *      int recv_msg_cpu_kernel_exit(t_buffer* buffer, t_PCB* pcb)
 *      int recv_msg_cpu_kernel_page_fault(t_buffer* buffer, t_PCB* pcb, uint32_t* segment, uint32_t* page)
 *      int recv_msg_cpu_kernel_sigsegv(t_buffer* buffer, t_PCB* pcb)
 * 
 */

/*
 *      Kernel & CPU: serialize messages
 */

// MSG_KERNEL_CPU_INTERRUPT
void serialize_msg_kernel_cpu_interrupt(t_buffer* buffer, uint32_t pid) {

    buffer->size = sizeof(uint32_t);
    buffer->stream = malloc(buffer->size);

    memcpy(buffer->stream, &pid, sizeof(uint32_t));

}

// MSG_CPU_KERNEL_INTERRUPT
void serialize_msg_cpu_kernel_interrupt(t_buffer* buffer, t_PCB* pcb) {

    buffer->size = sizeof(t_PCB);
    buffer->stream = malloc(buffer->size);

    serialize_pcb(buffer, pcb);

}

// MSG_KERNEL_CPU_EXEC
void serialize_msg_kernel_cpu_exec(t_buffer* buffer, t_PCB* pcb) {

    buffer->size = sizeof(t_PCB);

    serialize_pcb(buffer, pcb);

}

// MSG_CPU_KERNEL_IO
void serialize_msg_cpu_kernel_io(t_buffer* buffer, t_PCB* pcb) {

    buffer->size = sizeof(t_PCB);

    serialize_pcb(buffer, pcb);
    
}

// MSG_CPU_KERNEL_EXIT
void serialize_msg_cpu_kernel_exit(t_buffer* buffer, t_PCB* pcb) {

    buffer->size = sizeof(t_PCB);

    serialize_pcb(buffer, pcb);

}

// MSG_CPU_KERNEL_PAGE_FAULT
void serialize_msg_cpu_kernel_page_fault(t_buffer* buffer, t_PCB* pcb, uint32_t segment, uint32_t page) {

    // Segment & page
    buffer->size = sizeof(uint32_t) * 2;
    void* stream = malloc(buffer->size);    
    memcpy(stream, &segment, sizeof(uint32_t));
    memcpy(stream + sizeof(uint32_t), &page, sizeof(uint32_t));    
    buffer->stream = stream;

    // PCB
    t_buffer* buffer_pcb = buffer_create();
    serialize_pcb(buffer_pcb, pcb);
    buffer_add(buffer, buffer_pcb->stream, buffer_pcb->size);
    buffer_destroy(buffer_pcb);
}

// MSG_CPU_KERNEL_SIGSEGV
void serialize_msg_cpu_kernel_sigsegv(t_buffer* buffer, t_PCB* pcb) {

    buffer->size = sizeof(t_PCB);
    buffer->stream = malloc(buffer->size);

    serialize_pcb(buffer, pcb);

}


/*
 *      Kernel & CPU: deserialize messages
 */

// MSG_KERNEL_CPU_INTERRUPT
void deserialize_msg_kernel_cpu_interrupt(t_buffer* buffer, uint32_t* pid) {

    memcpy(pid, buffer->stream, sizeof(uint32_t));

}

// MSG_CPU_KERNEL_INTERRUPT
void deserialize_msg_cpu_kernel_interrupt(t_buffer* buffer, t_PCB* pcb) {

    deserialize_pcb(buffer, pcb);

}

// MSG_KERNEL_CPU_EXEC
void deserialize_msg_kernel_cpu_exec(t_buffer* buffer, t_PCB* pcb) {

    deserialize_pcb(buffer, pcb);

}

// MSG_CPU_KERNEL_IO
void deserialize_msg_cpu_kernel_io(t_buffer* buffer, t_PCB* pcb) {

    deserialize_pcb(buffer, pcb);
    
}

// MSG_CPU_KERNEL_EXIT
void deserialize_msg_cpu_kernel_exit(t_buffer* buffer, t_PCB* pcb) {

    deserialize_pcb(buffer, pcb);

}

// MSG_CPU_KERNEL_PAGE_FAULT
void deserialize_msg_cpu_kernel_page_fault(t_buffer* buffer, t_PCB* pcb, uint32_t* segment, uint32_t* page) {

    void* stream = buffer->stream;

    // Segment & page
    memcpy(segment, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    memcpy(page, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);
    
    // PCB
    t_buffer* pcb_buffer = buffer_create();
    uint32_t pcb_size = 0;
    memcpy(&pcb_size, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    pcb_buffer->stream = stream;
    pcb_buffer->size = pcb_size;

    deserialize_pcb(pcb_buffer, pcb);
    free(pcb_buffer);
}

// MSG_CPU_KERNEL_SIGSEGV
void deserialize_msg_cpu_kernel_sigsegv(t_buffer* buffer, t_PCB* pcb) {

    deserialize_pcb(buffer, pcb);

}

/*
 *      Kernel & CPU: send messages
 */

// MSG_KERNEL_CPU_INTERRUPT
int send_msg_kernel_cpu_interrupt(uint32_t pid, int fd) {
    
    t_package* package = package_create(MSG_KERNEL_CPU_INTERRUPT);

    serialize_msg_kernel_cpu_interrupt(package->buffer, pid);

    package_send(package, fd);
 
    package_destroy(package);

    return EXIT_SUCCESS;
}

// MSG_CPU_KERNEL_INTERRUPT
int send_msg_cpu_kernel_interrupt(t_PCB* pcb, int fd) {

    t_package* package = package_create(MSG_CPU_KERNEL_INTERRUPT);

    serialize_msg_cpu_kernel_interrupt(package->buffer, pcb);

    package_send(package, fd);
 
    package_destroy(package);

    return EXIT_SUCCESS;
}

// MSG_KERNEL_CPU_EXEC
int send_msg_kernel_cpu_exec(t_PCB* pcb, int fd) {

    t_package* package = package_create(MSG_KERNEL_CPU_EXEC);

    serialize_msg_kernel_cpu_exec(package->buffer, pcb);

    package_send(package, fd);
 
    package_destroy(package);

    return EXIT_SUCCESS;
}

// MSG_CPU_KERNEL_IO
int send_msg_cpu_kernel_io(t_PCB* pcb, int fd) {

    t_package* package = package_create(MSG_CPU_KERNEL_IO);

    serialize_msg_cpu_kernel_io(package->buffer, pcb);

    package_send(package, fd);
 
    package_destroy(package);

    return EXIT_SUCCESS;
}

// MSG_CPU_KERNEL_EXIT
int send_msg_cpu_kernel_exit(t_PCB* pcb, int fd) {

    t_package* package = package_create(MSG_CPU_KERNEL_EXIT);

    serialize_msg_cpu_kernel_exit(package->buffer, pcb);

    package_send(package, fd);
 
    package_destroy(package);

    return EXIT_SUCCESS;
}

// MSG_CPU_KERNEL_PAGE_FAULT
int send_msg_cpu_kernel_page_fault(t_PCB* pcb, uint32_t page_tabled_id, uint32_t page, int fd) {

    t_package* package = package_create(MSG_CPU_KERNEL_PAGE_FAULT);

    serialize_msg_cpu_kernel_page_fault(package->buffer, pcb, page_tabled_id, page);

    package_send(package, fd);
 
    package_destroy(package);

    return EXIT_SUCCESS;
}

// MSG_CPU_KERNEL_SIGSEGV
int send_msg_cpu_kernel_sigsegv(t_PCB* pcb, int fd) {

    t_package* package = package_create(MSG_CPU_KERNEL_SIGSEGV);

    serialize_msg_cpu_kernel_sigsegv(package->buffer, pcb);

    package_send(package, fd);
 
    package_destroy(package);

    return EXIT_SUCCESS;
}

/*
 *      Kernel & CPU: receive messages
 */

// MSG_KERNEL_CPU_INTERRUPT
int recv_msg_kernel_cpu_interrupt(t_buffer* buffer, uint32_t* pid) {

    deserialize_msg_kernel_cpu_interrupt(buffer, pid);

    return EXIT_SUCCESS;
}

// MSG_CPU_KERNEL_INTERRUPT
int recv_msg_cpu_kernel_interrupt(t_buffer* buffer, t_PCB* pcb) {

    deserialize_msg_cpu_kernel_interrupt(buffer, pcb);

    return EXIT_SUCCESS;
}

// MSG_KERNEL_CPU_EXEC
int recv_msg_kernel_cpu_exec(t_buffer* buffer, t_PCB* pcb) {

    deserialize_msg_kernel_cpu_exec(buffer, pcb);

    return EXIT_SUCCESS;
}

// MSG_CPU_KERNEL_IO
int recv_msg_cpu_kernel_io(t_buffer* buffer, t_PCB* pcb) {

    deserialize_msg_cpu_kernel_io(buffer, pcb);
 
    return EXIT_SUCCESS;
}

// MSG_CPU_KERNEL_EXIT
int recv_msg_cpu_kernel_exit(t_buffer* buffer, t_PCB* pcb) {

    deserialize_msg_cpu_kernel_exit(buffer, pcb);

    return EXIT_SUCCESS;
}

// MSG_CPU_KERNEL_PAGE_FAULT
int recv_msg_cpu_kernel_page_fault(t_buffer* buffer, t_PCB* pcb, uint32_t* page_table_id, uint32_t* page) {

    deserialize_msg_cpu_kernel_page_fault(buffer, pcb, page_table_id, page);

    return EXIT_SUCCESS;
}

// MSG_CPU_KERNEL_SIGSEGV
int recv_msg_cpu_kernel_sigsegv(t_buffer* buffer, t_PCB* pcb) {

    deserialize_msg_cpu_kernel_sigsegv(buffer, pcb);

    return EXIT_SUCCESS;
}


/*
 *
 *      KERNEL & MEMORY: SEND & RECEIVE MESSAGES
 * 
 */


// -- -- SEND MESSAGES -- --


// --   INIT    --

// KERNEL -> MEMORIA :: MSG_KERNEL_MEMORIA_INIT
int send_msg_kernel_memoria_init(uint32_t pid, t_list* segments, int fd) {

    t_package* package = package_create(MSG_KERNEL_MEMORIA_INIT);

    serialize_uint32_t(package->buffer, 1, pid);

    t_buffer* buffer_segments = buffer_create();
    serialize_list_uint32_t(buffer_segments, segments);
    
    buffer_add(package->buffer, buffer_segments->stream, buffer_segments->size);
    buffer_destroy(buffer_segments);
    
    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}

// MEMORIA -> KERNEL :: MSG_MEMORIA_KERNEL_INIT
int send_msg_memoria_kernel_init(int fd) {

    t_package* package = package_create(MSG_MEMORIA_KERNEL_INIT);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}


// --   PAGE FAULT    --

// KERNEL -> MEMORIA :: MSG_KERNEL_MEMORIA_PAGE_FAULT
int send_msg_kernel_memoria_page_fault(uint32_t pid, uint32_t page_table_id, uint32_t page_number, int fd) {

    t_package* package = package_create(MSG_KERNEL_MEMORIA_PAGE_FAULT);

    serialize_uint32_t(package->buffer, 3, pid, page_table_id, page_number);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}

// MEMORIA -> KERNEL :: MSG_MEMORIA_KERNEL_PAGE_FAULT_DONE
int send_msg_memoria_kernel_page_fault_done(int fd) {

    t_package* package = package_create(MSG_MEMORIA_KERNEL_PAGE_FAULT);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}


// --   EXIT    --

// KERNEL -> MEMORIA :: MSG_KERNEL_MEMORIA_EXIT
int send_msg_kernel_memoria_exit(uint32_t pid, int fd) {

    t_package* package = package_create(MSG_KERNEL_MEMORIA_EXIT);

    serialize_uint32_t(package->buffer, 1, pid);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;

}

/*
// KERNEL -> MEMORIA :: MSG_KERNEL_MEMORIA_EXIT ALT. VERSION
int send_msg_kernel_memoria_exit(t_list* page_table_ids, int fd) {

    t_package* package = package_create(MSG_KERNEL_MEMORIA_EXIT);

    serialize_msg_list_uint32_t(package->buffer, page_table_ids);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}
*/

// MEMORIA -> KERNEL :: MSG_MEMORIA_KERNEL_EXIT
int send_msg_memoria_kernel_exit(int fd) {

    t_package* package = package_create(MSG_MEMORIA_KERNEL_EXIT);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}


// -- -- RECEIVE MESSAGES -- --


// --   INIT    --

// KERNEL -> MEMORIA :: MSG_KERNEL_MEMORIA_INIT 
int recv_msg_kernel_memoria_init(t_buffer* buffer, uint32_t* pid, t_list* segments) {

    deserialize_uint32_t(buffer, 1, pid);

    void* stream = buffer->stream;
    stream += sizeof(uint32_t);
    size_t size = 0;

    memcpy(&size, stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

    t_buffer* buffer_segments = buffer_create();
    buffer_segments->stream = stream;
    buffer_segments->size = size;

    deserialize_list_uint32_t(buffer_segments, segments);
    free(buffer_segments);
    return EXIT_SUCCESS;
}


// --   PAGE FAULT    --

// MSG_KERNEL_MEMORIA_PAGE_FAULT
int recv_msg_kernel_memoria_page_fault(t_buffer* buffer, uint32_t* pid, uint32_t* page_table_id, uint32_t* page_number) {

    deserialize_uint32_t(buffer, 3, pid, page_table_id, page_number);

    return EXIT_SUCCESS;
}


// --   EXIT    --

// KERNEL -> MEMORIA :: MSG_KERNEL_MEMORIA_EXIT
int recv_msg_kernel_memoria_exit(t_buffer* buffer, uint32_t* pid) {
    
    deserialize_uint32_t(buffer, 1, pid);
    
    return EXIT_SUCCESS;
} 

/*
// KERNEL -> MEMORIA :: MSG_KERNEL_MEMORIA_EXIT ALT. VERSION
int recv_msg_kernel_memoria_exit(t_buffer* buffer, t_list* page_table_ids) {
    
    deserialize_msg_list_uint32_t(buffer, page_table_ids);

    buffer_destroy(buffer);

    return EXIT_SUCCESS;
}
*/


/*
 *
 *      CPU & MEMORY: SEND & RECEIVE MESSAGES
 * 
 */


// -- -- SEND MESSAGES -- --


// --   INIT    --

// CPU -> MEMORIA :: MSG_CPU_MEMORIA_INIT
int send_msg_cpu_memoria_init(int fd) {

    t_package* package = package_create(MSG_CPU_MEMORIA_INIT);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}

// MEMORIA -> CPU :: MSG_MEMORIA_CPU_INIT
int send_msg_memoria_cpu_init(uint32_t page_table_entries_qty, uint32_t page_size, int fd) {

    t_package* package = package_create(MSG_MEMORIA_CPU_INIT);

    serialize_uint32_t(package->buffer, 2, page_table_entries_qty, page_size);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}


// --   PAGE & FRAME    --

// CPU -> MEMORIA :: MSG_CPU_MEMORIA_PAGE
int send_msg_cpu_memoria_page(uint32_t pid, uint32_t page_table_id, uint32_t page, int fd) {

    t_package* package = package_create(MSG_CPU_MEMORIA_PAGE);

    serialize_uint32_t(package->buffer, 3, pid, page_table_id, page);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}

// MEMORIA -> CPU :: MSG_MEMORIA_CPU_FRAME
int send_msg_memoria_cpu_frame(uint32_t frame, int fd) {

    t_package* package = package_create(MSG_MEMORIA_CPU_FRAME);

    serialize_uint32_t(package->buffer, 1, frame);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}


// --   READ DATA   --

// CPU -> MEMORIA :: MSG_CPU_MEMORIA_DATA_READ
int send_msg_cpu_memoria_data_read(uint32_t pid, uint32_t segment, uint32_t page, uint32_t frame, uint32_t offset, int fd) {

    t_package* package = package_create(MSG_CPU_MEMORIA_DATA_READ);

    serialize_uint32_t(package->buffer, 5, pid, segment, page, frame, offset);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}

// MEMORIA -> CPU :: MSG_MEMORIA_CPU_DATA_READ
int send_msg_memoria_cpu_data_read(uint32_t value, int fd) {

    t_package* package = package_create(MSG_MEMORIA_CPU_DATA_READ);

    serialize_uint32_t(package->buffer, 1, value);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}


// --   WRITE DATA   --

// CPU -> MEMORIA :: MSG_CPU_MEMORIA_DATA_WRITE
int send_msg_cpu_memoria_data_write(uint32_t pid, uint32_t segment, uint32_t page, uint32_t frame, uint32_t offset, uint32_t value, int fd) {

    t_package* package = package_create(MSG_CPU_MEMORIA_DATA_WRITE);

    serialize_uint32_t(package->buffer, 6, pid, segment, page, frame, offset, value);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}

// MEMORIA -> CPU :: MSG_MEMORIA_CPU_DATA_WRITE
int send_msg_memoria_cpu_data_write(int fd) {

    t_package* package = package_create(MSG_MEMORIA_CPU_DATA_WRITE);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}


// --   PAGE FAULT    --

// MSG_MEMORIA_CPU_PAGE_FAULT
int send_msg_memoria_cpu_page_fault(int fd) {

    t_package* package = package_create(MSG_MEMORIA_CPU_PAGE_FAULT);

    package_send(package, fd);

    package_destroy(package);

    return EXIT_SUCCESS;
}


// -- -- RECEIVE MESSAGES -- --


// --   INIT   --

// MEMORIA -> CPU :: MSG_MEMORIA_CPU_INIT
int recv_msg_memoria_cpu_init(t_buffer* buffer, uint32_t* page_table_entries_qty, uint32_t* page_size) {

    deserialize_uint32_t(buffer, 2, page_table_entries_qty, page_size);
    
    return EXIT_SUCCESS;
}


// --   PAGE & FRAME   --

// CPU -> MEMORIA :: MSG_CPU_MEMORIA_PAGE
int recv_msg_cpu_memoria_page(t_buffer* buffer, uint32_t* pid, uint32_t* page_table_id, uint32_t* page) {

    deserialize_uint32_t(buffer, 3, pid, page_table_id, page);
    
    return EXIT_SUCCESS;
}

// MEMORIA -> CPU :: MSG_MEMORIA_CPU_FRAME
int recv_msg_memoria_cpu_frame(t_buffer* buffer, uint32_t* frame) {
    
    deserialize_uint32_t(buffer, 1, frame);
    
    return EXIT_SUCCESS;
}


// --   READ DATA    --

// CPU -> MEMORIA :: MSG_CPU_MEMORIA_DATA_READ
int recv_msg_cpu_memoria_data_read(t_buffer* buffer, uint32_t* pid, uint32_t* segment, uint32_t* page, uint32_t* frame, uint32_t* offset) {

    deserialize_uint32_t(buffer, 5, pid, segment, page, frame, offset);
    
    return EXIT_SUCCESS;
}

// MEMORIA -> CPU :: MSG_MEMORIA_CPU_DATA_READ
int recv_msg_memoria_cpu_data(t_buffer* buffer, uint32_t* value) {

    deserialize_uint32_t(buffer, 1, value);
    
    return EXIT_SUCCESS;
}


// --   WRITE DATA    --

// CPU -> MEMORIA :: MSG_MEMORIA_CPU_DATA_WRITE
int recv_msg_cpu_memoria_data_write(t_buffer* buffer, uint32_t* pid, uint32_t* segment, uint32_t* page, uint32_t* frame, uint32_t* offset, uint32_t* value) {

    deserialize_uint32_t(buffer, 6, pid, segment, page, frame, offset, value);
    
    return EXIT_SUCCESS;
}
