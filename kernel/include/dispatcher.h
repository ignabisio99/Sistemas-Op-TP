#ifndef DISPATCHER_H
#define DISPATCHER_H


#include "global.h"
#include "sockets.h"
#include "protocolo.h"
#include <semaphore.h>
#include <pthread.h>

t_PCB* get_next_pcb_to_exec();

#endif