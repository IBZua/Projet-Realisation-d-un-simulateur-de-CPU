#include "Memoire.h"

MemoryHandler *memory_init(int size){
	//Alloc handler
	MemoryHandler *handler = malloc(sizeof(MemoryHandler));
	if (!handler) return NULL;
	//Alloc Tableau de pointeurs vers la memoire allouee
	handler->memory = calloc(size, sizeof(void *));
	if (!handler->memory) {
		free(handler);
		return NULL;
	}
	//Init Taille totale de la memoire geree
	handler->total_size = size;
	//Creation liste des segments de mem libre
	Segment *segment = malloc(sizeof(Segment));
	if (!segment) {
		free(handler->memory);
		free(handler);
		return NULL;
	}
	segment->start = 0;
	segment->size = size;
	segment->next = NULL;
	handler->free_list = segment;

	//Init table de hachage pour les segments alloués
	handler->allocated = hashmap_create();
	if (!handler->allocated) {
		free(segment);
		free(handler->memory);
		free(handler);
		return NULL;
	}
	return handler;
}

Segment *find_free_segment(MemoryHandler* handler, int start, int size, Segment** prev){
	*prev = NULL;
	Segment *tmp = handler->free_list;
	while(tmp){
		if(tmp->start <= start && (tmp->start+tmp->size) >= (start+size)){
			return tmp;
		}
		*prev = tmp;
		tmp = tmp->next;
	}
	return NULL;
}

int create_segment(MemoryHandler *handler, const char *name, int start, int size){
	//1
	Segment *prev = NULL;
	Segment *free_seg = find_free_segment(handler, start, size, &prev);
	if(!free_seg) return -1;
	//2
	Segment *new_seg = malloc(sizeof(Segment));
	new_seg->start = start;
	new_seg->size = size;
	new_seg->next = NULL;
	if(hashmap_insert(handler->allocated, name, new_seg) != 0){
		free(new_seg);
		return -1;
	}
	//3
	//retirer sgment
	//ajouter deux segments
	if(!prev){
		handler->free_list = free_seg->next;
	}else{
		prev->next = free_seg->next;
	}

	int libre_start = free_seg->start;	//debut seg init
	int libre_end   = free_seg->start + free_seg->size;	//fin seg init
	int left_size  = start - libre_start;	//fin bloc gauche
	int right_size = libre_end - (start + size); //debut bloc droit

	//Creation des seg
	Segment *left_seg  = NULL;
    Segment *right_seg = NULL;

	if(left_size > 0){
		left_seg = malloc(sizeof(Segment));
		left_seg->start = start;
		left_seg->size = left_size;
		left_seg->next = NULL;
	}
	if(right_seg > 0){
		right_seg = malloc(sizeof(Segment));
		right_seg->start = libre_end;
		right_seg->size = right_size;
		right_seg->next = NULL;
	}

	//Chainage des seg
	if(left_seg && right_seg){
		left_seg->next = right_seg;
		right_seg->next = free_seg->next;
	}else if(right_seg){
		right_seg->next = free_seg->next;
	}

	//insertion dans la liste
	handler->free_list = left_seg;
	left_seg->next = right_seg;

	return 0;
}
int remove_segment(MemoryHandler *handler, const char *name){
	//touver le segment
	Segment *seg = hashmap_get(handler->allocated, name);
	//stocker ses donnees dans une variable tmp
	Segment *free = malloc(sizeof(Segment));
	free->start = seg->start;
	free->size = seg->size;
	//le liberer
	hashmap_remove(handler->allocated, name);
	//reallouer dans les segments libre
	Segment *tmp = handler->free_list;
	//On cherche l'emplacement de l'insertion
	while(tmp->next){
		int fin = tmp->next->start + tmp->next->size;
		if(fin == (free->start)){
			free->next = tmp->next->next;
			tmp->next = free;
			return 0;
		}
		tmp = tmp->next;
	}
	return 1;
}