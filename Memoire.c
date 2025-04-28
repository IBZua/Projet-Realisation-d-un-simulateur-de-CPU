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
		left_seg->start = libre_start;
		left_seg->size = left_size;
		left_seg->next = NULL;
	}
	if(right_size > 0){
		right_seg = malloc(sizeof(Segment));
		right_seg->start = start + size;
		right_seg->size = right_size;
		right_seg->next = NULL;
	}

	//Chainage des seg
	if(left_seg){
		if(prev) prev->next = left_seg;
		else handler->free_list = left_seg;
	}else if(right_seg){
		if(prev) prev->next = right_seg;
		else handler->free_list = right_seg;
	}

	//insertion dans la liste

	if (right_seg)right_seg->next = free_seg->next;
	if (left_seg){
		if(prev) prev->next = left_seg;
		else handler->free_list = left_seg;
	}else if(right_seg){
		if(prev) prev->next = right_seg;
		else handler->free_list = right_seg;
	}
	free(free_seg);

	return 0;

}

int remove_segment(MemoryHandler *handler, const char *name) {
    if (!handler || !name) return -1;

    // 1) Récupère et retire le segment alloué
    Segment *seg = (Segment*) hashmap_get(handler->allocated, name);
    if (!seg) return -1;
    hashmap_remove(handler->allocated, name);

    // 2) Parcours la free_list pour trouver où insérer seg (en gardant l’ordre croissant)
    Segment *prev = NULL, *curr = handler->free_list;
    while (curr && curr->start + curr->size < seg->start) {
        prev = curr;
        curr = curr->next;
    }

    // 3) Tenter fusion avec le bloc de gauche
    if (prev && prev->start + prev->size == seg->start) {
        prev->size += seg->size;
        // 3a) Fusion possible avec le bloc de droite aussi ?
        if (curr && seg->start + seg->size == curr->start) {
            prev->size += curr->size;
            prev->next = curr->next;
            free(curr);
        }
        free(seg);
        return 0;
    }

    // 4) Fusion seulement avec le bloc de droite
    if (curr && seg->start + seg->size == curr->start) {
        curr->start  = seg->start;
        curr->size  += seg->size;
        free(seg);
        return 0;
    }

    // 5) Pas de fusion : on insère seg entre prev et curr
    seg->next = curr;
    if (prev) prev->next = seg;
    else        handler->free_list = seg;

    return 0;
}
