#include "tlb.h"

t_TLB* TLB;

int32_t ultima_referencia = -1;

int32_t _proxima_referencia_tlb();

bool obtener_marco(uint32_t pid, uint32_t segment, uint32_t page, uint32_t* frame) {
    

	log_error(cpu_logger,"PID: %d - QUE BUSCO EN TLB?- Segmento: %d - Pagina: %d",
					pid,segment, page);
    for (int i=0; i < cpu_config->ENTRADAS_TLB; i++) {

        if (TLB[i].page == page && TLB[i].pid == pid && TLB[i].segment == segment) {
            *frame = TLB[i].frame; //TLB HIT
			log_info(cpu_logger,"PID: %d - TLB HIT - Segmento: %d - Pagina: %d",
						pid,segment, page);
			
			if(strcmp(cpu_config->REEMPLAZO_TLB,"LRU") == 0){
				TLB[i].cont_referencia = _proxima_referencia_tlb();
				}
            return true;
        }
    }

	log_info(cpu_logger,"PID: %d - TLB MISS - Segmento: %d - Pagina: %d",
						pid,segment, page);

    return false; //TLB MISS
} 


void reemplazar(uint32_t pid,uint32_t segment,uint32_t page,uint32_t frame){
	int32_t cant_entradas = cpu_config->ENTRADAS_TLB;
	int32_t referencia_actual = TLB[0].cont_referencia;
	int32_t victima_a_reemplazar = 0;

	for(int i=1; i<cant_entradas; i++){
		
		if(referencia_actual > TLB[i].cont_referencia){
			referencia_actual = TLB[i].cont_referencia;
			victima_a_reemplazar = i;
		}

	}

	//Se reemplaza la seleccionada
	TLB[victima_a_reemplazar].pid = pid;
	TLB[victima_a_reemplazar].segment = segment;
	TLB[victima_a_reemplazar].page	= page;
	TLB[victima_a_reemplazar].frame = frame;
	TLB[victima_a_reemplazar].cont_referencia = _proxima_referencia_tlb();

	imprimir_tlb();
}

int32_t _proxima_referencia_tlb(){
	ultima_referencia++;
	return ultima_referencia;
}

void limpiar_proceso_TLB(uint32_t pid){
	
	for(int i = 0; i< cpu_config->ENTRADAS_TLB; i++){
		if(pid == TLB[i].pid){
			TLB[i].pid = -1;
			TLB[i].segment = -1;
			TLB[i].frame = -1;
			TLB[i].page = -1;
			TLB[i].cont_referencia = -1;
		}
	}

}

void imprimir_tlb(){
	char *string = string_new();
	string_append(&string, "=========================SEGMENTOS===============================\n");
	for(int i =0; i< cpu_config->ENTRADAS_TLB; i++){
		char* entrada_string = string_from_format("| PID: %i | SEGMENTO: %i | PAGINA: %i | MARCO: %i | contador: %i\n", 
				TLB[i].pid,TLB[i].segment,TLB[i].page, TLB[i].frame,TLB[i].cont_referencia );
		string_append(&string, entrada_string);
		free(entrada_string);
	}
	string_append(&string,"=================================================================");
	log_info(cpu_logger, "\n%s\n", string);
	free(string);
}

void inicializar_TLB(){

    TLB = malloc(sizeof(t_TLB)* cpu_config->ENTRADAS_TLB);

    for(int i = 0; i<cpu_config->ENTRADAS_TLB;i++){
        TLB[i].pid = -1;
        TLB[i].segment = -1;
        TLB[i].page = -1;
        TLB[i].frame = -1;
        TLB[i].cont_referencia = -1;
    }

    return;
}