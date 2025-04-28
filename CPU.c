#include "CPU.h"
CPU *cpu_init(int memory_size){
	CPU *cpu = malloc(sizeof(CPU));
	cpu->memory_handler = memory_init(memory_size);
	cpu->context = hashmap_create();
	int *reg_ax = malloc(sizeof(int));*reg_ax = 0;hashmap_insert(cpu->context, "AX", reg_ax);
	int *reg_bx = malloc(sizeof(int));*reg_bx = 0;hashmap_insert(cpu->context, "BX", reg_bx);
	int *reg_cx = malloc(sizeof(int));*reg_cx = 0;hashmap_insert(cpu->context, "CX", reg_cx);
	int *reg_dx = malloc(sizeof(int));*reg_dx = 0;hashmap_insert(cpu->context, "DX", reg_dx);
	int *reg_ip = malloc(sizeof(int));*reg_ip = 0;hashmap_insert(cpu->context, "IP", reg_ip);
	int *reg_zf = malloc(sizeof(int));*reg_zf = 0;hashmap_insert(cpu->context, "ZF", reg_zf);
	int *reg_sf = malloc(sizeof(int));*reg_sf = 0;hashmap_insert(cpu->context, "SF", reg_sf);
	int *reg_sp = malloc(sizeof(int));*reg_sp = cpu->memory_handler->total_size - 1;hashmap_insert(cpu->context, "SP", reg_sp);
    int *reg_bp = malloc(sizeof(int));*reg_bp = cpu->memory_handler->total_size - 1;hashmap_insert(cpu->context, "BP", reg_bp);
	int ss_start = cpu->memory_handler->total_size - 128;
    create_segment(cpu->memory_handler, "SS", ss_start, 128);
    int *reg_es = malloc(sizeof(int));
    if (reg_es) {
        *reg_es = -1;
        hashmap_insert(cpu->context, "ES", reg_es);
    }
	cpu->constant_pool = hashmap_create();
	return cpu;
}

void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data){
	Segment *seg_allocated = (Segment*) hashmap_get(handler->allocated, segment_name);
	//Check existance segment
	if (!seg_allocated) {
        printf("segment non trouvé\n");
        return NULL;
    }
	//Check bornes segment
	if (pos < 0 || pos >= seg_allocated->size) {
        printf("hors segment\n");
        return NULL;
    }
	handler->memory[seg_allocated->start + pos] = data;
	return handler->memory[seg_allocated->start + pos];
}

void* load(MemoryHandler *handler, const char *segment_name, int pos){
	if(!handler) {
        printf("erreur parametre handler invalide\n");
        return NULL;
    }
	Segment *seg_allocated = (Segment*) hashmap_get(handler->allocated, segment_name);
	if (!seg_allocated) {
        printf("segment non trouvé\n");
        return NULL;
    }
	//Check bornes segment
	if (pos < 0 || pos >= seg_allocated->size) {
        printf("hors segment\n");
        return NULL;
    }
	return handler->memory[seg_allocated->start + pos];
}

void allocate_variables(CPU *cpu, Instruction** data_instructions, int data_count){
	/*Calcul de l'espace de stockage*/
	//on parcourt chaque Instruction .DATA pour compte le nombre total d'éléments (1 valeur + 1 par virgule)
	int total_size = 0;
	//Check data
	if(!data_instructions || data_count <= 0) return;
	for(int i = 0; i < data_count; i++){
		//si la data n'existe pas ou est une chaine vide on skip
		if(!data_instructions[i]->operand2 || data_instructions[i]->operand2[0] == '\0') continue;
		//on a pas skip donc +1
		total_size++;
		//on parcout la chaine de caractere lettre par lettre
		int lettre = 0;
		while(data_instructions[i]->operand2[lettre] != '\0'){
			if(data_instructions[i]->operand2[lettre] == ','){
				total_size++;
			}
			lettre++;
		}
	}
	/*Segment DS dans le MemoryHandler*/
	if (!hashmap_get(cpu->memory_handler->allocated, "DS")) if (create_segment(cpu->memory_handler, "DS", 0, total_size)) return;
	/*Remplissage du segment de donnees avec .DATA*/
	int pos = 0;
	for(int i = 0; i < data_count; i++){
		//on duplique la chaîne pour strtok
		char *copy = strdup(data_instructions[i]->operand2);
		if(!copy) return;
		//on decoupe sur la virgule
		char *el = strtok(copy, ",");
		while(el){
			//conversion string en int (toutes les data sont des int)
			int val = atoi(el);
			int *cell = malloc(sizeof(int));
			if(!cell){
				free(copy);
				return;
			}
			*cell = val;
			//stockage DS à pos
			int *storecheck = store(cpu->memory_handler, "DS", pos, cell);
			if(!storecheck){
				free(copy);
				return;
			}
			//case suivante
			pos++;
			el = strtok(NULL, ",");
		}
		free(copy);
	}
}

void  print_data_segment(CPU *cpu){
	if(!cpu || !cpu->memory_handler || !cpu->memory_handler->allocated) return;
	Segment *seg = (Segment*) hashmap_get(cpu->memory_handler->allocated, "DS");
	if(!seg) return;
	for(int i = 0; i < seg->size; i++){
		int *val = (int*) load(cpu->memory_handler, "DS", i);
		if(!val){
			printf("pos %d vide\n", i);
			continue;
		}
		printf("pos %d val = %d\n", seg->start+i, *val);
	}
	printf("fin du segment\n");
}

int matches (const char *pattern, const char *string ){
	regex_t regex;
	int result = regcomp(&regex, pattern, REG_EXTENDED);
	if (result) {
		fprintf (stderr, "Regex compilation failed for pattern : %s \n", pattern);
		return 0;
	}
	result = regexec (&regex, string, 0, NULL, 0);
	regfree (&regex);
	return result == 0;
}

void *immediate_addressing(CPU *cpu, const char *operand){
	if(!cpu || !cpu->constant_pool) return NULL;
	if(matches("^[0-9]+$", operand)){
		void *val = hashmap_get(cpu->constant_pool, operand);
		if(!val){
			int *p = malloc(sizeof(int));
			if(!p) return NULL;
			*p = atoi(operand);
			if(hashmap_insert(cpu->constant_pool, operand, p)){
				free(p);
				return NULL;
			}
			val = p;
		}
		return val;
	}
	return NULL;
}

void *register_addressing(CPU *cpu, const char *operand){
	if(!cpu || !cpu->context) return NULL;
	//verifier format operand
	if(!matches("^(AX|BX|CX|DX)$", operand)) return NULL;
	void *val = hashmap_get(cpu->context, operand);
	return val;
}

void *memory_direct_addressing(CPU *cpu, const char *operand){
	if(!cpu || !cpu->memory_handler) return NULL;
	//verifier format operand
	if(!matches("^\\[[0-9]+\\]$", operand)) return NULL;
	//extraire
	int i = atoi(operand+1);
	return load(cpu->memory_handler, "DS", i);
}

void *register_indirect_addressing(CPU *cpu, const char *operand){
	if(!cpu || !cpu->memory_handler) return NULL;
	//verifier format operand
	if(!matches("^\\[(AX|BX|CX|DX)\\]$", operand)) return NULL;
	//suppression des []
	char *copy = strdup(operand+1);
	copy[strlen(copy) - 1] = '\0';
	int *p = (int*)register_addressing(cpu, copy);
	free(copy);
	if (!p) return NULL;
	int adresse = *p;
	return load(cpu->memory_handler, "DS", adresse);
}

void handle_MOV(CPU* cpu, void* src, void* dest){
	if(!cpu || !src || !dest) return;
	int value = *(int*)src;
	*(int*)dest = value;
}

CPU *setup_test_environment(){
	// Initialiser le CPU
	CPU *cpu = cpu_init(1024) ;
	if (!cpu) {
		printf ("Error : CPU initialization failed\n") ;
		return NULL;
	}
	// Initialiser les registres avec des valeurs specifiques
	int *ax = (int *) hashmap_get(cpu->context, "AX") ;
	int *bx = (int *) hashmap_get(cpu->context, "BX") ;
	int *cx = (int *) hashmap_get(cpu->context, "CX") ;
	int *dx = (int *) hashmap_get(cpu->context, "DX") ;

	*ax = 3;
	*bx = 6;
	*cx = 100;
	*dx = 0;

	//Creer et initialiser le segment de donnees
	if(!hashmap_get(cpu->memory_handler->allocated , "DS")){
	create_segment (cpu->memory_handler, "DS", 0 , 20);
	//Initialiser le segment de donnes avec des valeurs de test
		for(int i = 0; i < 10; i++) {
			int *value = (int *) malloc(sizeof(int));
			*value = i * 10 + 5; // Valeurs 5, 15, 25, 35...
			store ( cpu-> memory_handler , "DS", i , value) ;
		}
	}
	printf ("Test environment initialized.\n") ;
	return cpu ;
}

void cpu_destroy(CPU *cpu){
	if (!cpu) return;
	//free DS
	if (cpu->memory_handler) {
        MemoryHandler *mh = cpu->memory_handler;
        Segment *ds = (Segment*) hashmap_get(mh->allocated, "DS");
        if (ds) {
            for (int i = 0; i < ds->size; i++) {
                int *cell = (int*) load(mh, "DS", i);
                if (cell) free(cell);
            }
        }
		//free allocated
		if (mh->allocated) {
            for (int i = 0; i < TABLE_SIZE; i++) {
				HashEntry *e = &mh->allocated->table[i];
				if (e->key && e->value && e->value != TOMBSTONE) {
					free(e->value);
				}
			}
			hashmap_destroy(mh->allocated);
        }
		//free freelist
		Segment *cur = mh->free_list;
        while (cur) {
            Segment *next = cur->next;
            free(cur);
            cur = next;
        }
		free(mh->memory);
        free(mh);
		
	}
	//free context
	if (cpu->context) {
        for (int i = 0; i < TABLE_SIZE; i++) {
			HashEntry *e = &cpu->context->table[i];
			if (e->key && e->value && e->value != TOMBSTONE) {
				free(e->value);
			}
		}
		hashmap_destroy(cpu->context);
    }
	//free constant pool
	if (cpu->constant_pool) {
        for (int i = 0; i < TABLE_SIZE; i++) {
            HashEntry *e = &cpu->constant_pool->table[i];
            if (e->key && e->value && e->value != TOMBSTONE) {
                free(e->value);  // l’int* stocké pour chaque constante
            }
        }
        hashmap_destroy(cpu->constant_pool);
    }
	//free CPU
	free(cpu);
}

void *resolve_addressing(CPU *cpu, const char *operand) {
    void *res = NULL;

    //Immediate
    res = immediate_addressing(cpu, operand);
    if (res) return res;

    //Register direct
    res = register_addressing(cpu, operand);
    if (res) return res;

    //Memory direct (DS segment)
    res = memory_direct_addressing(cpu, operand);
    if (res) return res;

    //Register indirect
    res = register_indirect_addressing(cpu, operand);
    if (res) return res;

	//Segment override
	res = segment_override_addressing(cpu, operand);
    if (res) return res;

    //Aucun mode ne correspond
    return res;
}

char *trim(char *str) {
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }
    char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        *end = '\0';
        end--;
    }
    return str;
}

int search_and_replace(char **str, HashMap *values) {
    if (!str || !*str || !values) return 0;
    int replaced = 0;
    char *input = *str;

    for (int i = 0; i < values->size; i++) {
        if (values->table[i].key && values->table[i].key != (void *)-1) {
            char *key = values->table[i].key;
            int value = (int)(long)values->table[i].value;
            char *substr = strstr(input, key);
            if (substr) {
                char replacement[64];
                snprintf(replacement, sizeof(replacement), "%d", value);

                int key_len = strlen(key);
                int repl_len = strlen(replacement);

                char *new_str = malloc(strlen(input) - key_len + repl_len + 1);
                strncpy(new_str, input, substr - input);
                new_str[substr - input] = '\0';
                strcat(new_str, replacement);
                strcat(new_str, substr + key_len);

                free(input);
                *str = new_str;
                input = new_str;
                replaced = 1;
            }
        }
    }

    if (replaced) {
        char *trimmed = trim(input);
        if (trimmed != input) {
            memmove(input, trimmed, strlen(trimmed) + 1);
        }
    }

    return replaced;
}

int resolve_constants(ParserResult *result) {
    if (!result) return 0;
    int replaced = 0;

    // 1) Remplacement noms de variables par adresses dans DS
    for (int i = 0; i < result->code_count; i++) {
        Instruction *instr = result->code_instructions[i];
        if (instr->operand1)
            replaced |= search_and_replace(&instr->operand1, result->memory_locations);
        if (instr->operand2)
            replaced |= search_and_replace(&instr->operand2, result->memory_locations);
    }

    // 2) Remplacement étiquettes par  adresses dans segment code
    for (int i = 0; i < result->code_count; i++) {
        Instruction *instr = result->code_instructions[i];
        if (instr->operand1)
            replaced |= search_and_replace(&instr->operand1, result->labels);
        if (instr->operand2)
            replaced |= search_and_replace(&instr->operand2, result->labels);
    }

    return replaced;
}

void allocate_code_segment(CPU *cpu, Instruction **code_instructions, int code_count) {
    if (!cpu || !cpu->memory_handler || !code_instructions || code_count <= 0) return;
    // alloc CS taille code_count adresse 0
    if (create_segment(cpu->memory_handler, "CS", 0, code_count) != 0) return;

    //init IP à 0
    int *ip = (int*)hashmap_get(cpu->context, "IP");
    if (ip) *ip = 0;

    //stick instr dans CS
    for (int i = 0; i < code_count; i++) {
        store(cpu->memory_handler, "CS", i, code_instructions[i]);
    }
}

Instruction* fetch_next_instruction(CPU *cpu) {
    if (!cpu || !cpu->memory_handler || !cpu->context) return NULL;

    // Récupérer le segment CS
    Segment *cs = (Segment*)hashmap_get(cpu->memory_handler->allocated, "CS");
    if (!cs) return NULL;

    // Récupérer le pointeur d'instruction
    int *ip = (int*)hashmap_get(cpu->context, "IP");
    if (!ip) return NULL;

    // Vérifier bornes
    if (*ip < 0 || *ip >= cs->size) return NULL;

    // Charger l'instruction dans CS à l'index IP
    void *p = load(cpu->memory_handler, "CS", *ip);
    Instruction *instr = (Instruction*)p;

    // Incrémenter IP pour la prochaine lecture
    (*ip)++;

    return instr;
}

int run_program(CPU *cpu) {
    if (!cpu) return -1;

    // Affichage de l'état initial
    printf("=== État initial ===\n");
    print_data_segment(cpu);
    printf("Registres : AX=%d BX=%d CX=%d DX=%d IP=%d ZF=%d SF=%d\n",
           *(int*)hashmap_get(cpu->context,"AX"),
           *(int*)hashmap_get(cpu->context,"BX"),
           *(int*)hashmap_get(cpu->context,"CX"),
           *(int*)hashmap_get(cpu->context,"DX"),
           *(int*)hashmap_get(cpu->context,"IP"),
           *(int*)hashmap_get(cpu->context,"ZF"),
           *(int*)hashmap_get(cpu->context,"SF"));

    // Boucle d'exécution pas à pas
    while (1) {
        Instruction *instr = fetch_next_instruction(cpu);
        if (!instr) break;
        printf("\nInstruction suivante : %s %s%s%s\n",
               instr->mnemonic,
               instr->operand1 ? instr->operand1 : "",
               instr->operand2 ? ", " : "",
               instr->operand2 ? instr->operand2 : "");

        printf("[Entrée=exécuter, q=quitter] ");
        int c = getchar();
        if (c == 'q') break;
        while (c != '\n' && c != EOF) c = getchar();

        // Exécution
        execute_instruction(cpu, instr);

        // Affichage des flags et IP après exécution
        printf(" -> IP=%d ZF=%d SF=%d\n",
               *(int*)hashmap_get(cpu->context,"IP"),
               *(int*)hashmap_get(cpu->context,"ZF"),
               *(int*)hashmap_get(cpu->context,"SF"));
    }

    // Affichage de l'état final
    printf("\n=== État final ===\n");
    print_data_segment(cpu);
    printf("Registres : AX=%d BX=%d CX=%d DX=%d IP=%d ZF=%d SF=%d\n",
           *(int*)hashmap_get(cpu->context,"AX"),
           *(int*)hashmap_get(cpu->context,"BX"),
           *(int*)hashmap_get(cpu->context,"CX"),
           *(int*)hashmap_get(cpu->context,"DX"),
           *(int*)hashmap_get(cpu->context,"IP"),
           *(int*)hashmap_get(cpu->context,"ZF"),
           *(int*)hashmap_get(cpu->context,"SF"));

    return 0;
}

int push_value(CPU *cpu, int value) {
    if (!cpu) return -1;
    //Récup SP
    int *sp = (int*)hashmap_get(cpu->context, "SP");
    if (!sp) return -1;

    //Récup SS
    Segment *ss = (Segment*)hashmap_get(cpu->memory_handler->allocated, "SS");
    if (!ss) return -1;

    //Test overflow : si SP au début de SS, on ne peut pas descendre
    if (*sp <= ss->start) return -1;

    //Decrementer SP
    (*sp)--;

    //Calcul offset SS
    int offset = *sp - ss->start;

    //Alloc et stock val
    int *cell = malloc(sizeof(int));
    if (!cell) { (*sp)++; return -1; }
    *cell = value;
    if (!store(cpu->memory_handler, "SS", offset, cell)) {
        free(cell);
        (*sp)++;
        return -1;
    }

    return 0;
}

int pop_value(CPU *cpu, int *dest) {
    if (!cpu || !dest) return -1;
    //Recup SP
    int *sp = (int*)hashmap_get(cpu->context, "SP");
    if (!sp) return -1;
    //Recup SS
    Segment *ss = (Segment*)hashmap_get(cpu->memory_handler->allocated, "SS");
    if (!ss) return -1;
    //Underflow : SP au sommet initial (aucune valeur)
    if (*sp >= ss->start + ss->size - 1) return -1;
    //Lire el à SP
    int offset = *sp - ss->start;
    int *cell = (int*)load(cpu->memory_handler, "SS", offset);
    if (!cell) return -1;
    *dest = *cell;
    free(cell);
    //Incrémenter SP pour depiler
    (*sp)++;
    return 0;
}

void *segment_override_addressing(CPU *cpu, const char *operand) {
    if (!cpu || !cpu->memory_handler) return NULL;

    //Check format
    if (!matches("^\\[(DS|CS|SS|ES):(AX|BX|CX|DX)\\]$", operand)) return NULL;

    //Extraire nom seg et registre
    char seg[3] = { operand[1], operand[2], '\0' };
    char reg[3] = { operand[4], operand[5], '\0' };

    //Recup la valeur du registre (adresse)
    int *reg_ptr = (int *)register_addressing(cpu, reg);
    if (!reg_ptr) return NULL;
    int offset = *reg_ptr;

    //load donnee dans seg
    return load(cpu->memory_handler, seg, offset);
}

int find_free_address_strategy(MemoryHandler *handler, int size, int strategy) {
    if (!handler || size <= 0) return -1;

    Segment *curr = handler->free_list;
    int best_addr = -1;
    int best_diff;

    switch (strategy) {
        case 0:  // First Fit
            while (curr) {
                if (curr->size >= size) {
                    return curr->start;
                }
                curr = curr->next;
            }
            return -1;

        case 1:  // Best Fit
            best_diff = INT_MAX;
            while (curr) {
                if (curr->size >= size) {
                    int diff = curr->size - size;
                    if (diff < best_diff) {
                        best_diff = diff;
                        best_addr = curr->start;
                    }
                }
                curr = curr->next;
            }
            return best_addr;

        case 2:  // Worst Fit
            best_diff = -1;
            while (curr) {
                if (curr->size >= size) {
                    int diff = curr->size - size;
                    if (diff > best_diff) {
                        best_diff = diff;
                        best_addr = curr->start;
                    }
                }
                curr = curr->next;
            }
            return best_addr;

        default:
            return -1;
    }
}

int alloc_es_segment(CPU *cpu){
    if (!cpu) return -1;

    // 1) Récupérer les registres AX (taille), BX (stratégie), ZF (flag) et ES (destination)
    int *ax = (int*)hashmap_get(cpu->context, "AX");
    int *bx = (int*)hashmap_get(cpu->context, "BX");
    int *zf = (int*)hashmap_get(cpu->context, "ZF");
    int *es = (int*)hashmap_get(cpu->context, "ES");
    if (!ax || !bx || !zf || !es) return -1;

    int size     = *ax;
    int strategy = *bx;

    // 2) Trouver l'adresse libre selon la stratégie
    int start = find_free_address_strategy(cpu->memory_handler, size, strategy);
    if (start < 0) {
        *zf = 1;   // échec
        return -1;
    }

    // 3) Créer le segment "ES"
    if (create_segment(cpu->memory_handler, "ES", start, size) != 0) {
        *zf = 1;   // échec
        return -1;
    }

    // 4) Initialiser chaque case du segment à 0
    for (int i = 0; i < size; i++) {
        int *cell = malloc(sizeof(int));
        if (!cell) {
            *zf = 1;
            return -1;
        }
        *cell = 0;
        store(cpu->memory_handler, "ES", i, cell);
    }

    // 5) Mettre à jour ES et ZF
    *es = start;
    *zf = 0;
    return 0;
}

int free_es_segment(CPU *cpu) {
    if (!cpu) return -1;

    MemoryHandler *mh = cpu->memory_handler;
    if (!mh) return -1;

    // 1) Récupérer le segment ES
    Segment *seg = (Segment*)hashmap_get(mh->allocated, "ES");
    if (!seg) return -1;

    // 2) Libérer individuellement chaque valeur allouée dans ES
    for (int i = 0; i < seg->size; i++) {
        int *cell = (int*)load(mh, "ES", i);
        if (cell) {
            free(cell);
        }
    }

    // 3) Retirer le segment de la table des segments alloués et le réinsérer dans free_list
    remove_segment(mh, "ES");

    // 4) Remettre le registre ES à -1
    int *es = (int*)hashmap_get(cpu->context, "ES");
    if (es) *es = -1;

    return 0;
}

int handle_instruction(CPU *cpu, Instruction *instr, void *src, void *dest){
	if (!cpu || !instr) return -1;

    // MOV dst, src
    if (strcmp(instr->mnemonic, "MOV") == 0) {
        handle_MOV(cpu, src, dest);
        return 0;
    }

    // ADD dst, src
    if (strcmp(instr->mnemonic, "ADD") == 0) {
        int sum = *(int*)dest + *(int*)src;
        *(int*)dest = sum;
        int *zf = (int*)hashmap_get(cpu->context, "ZF");
        int *sf = (int*)hashmap_get(cpu->context, "SF");
        if (zf) *zf = (sum == 0);
        if (sf) *sf = (sum < 0);
        return 0;
    }

    // CMP dst, src
    if (strcmp(instr->mnemonic, "CMP") == 0) {
        int dv = *(int*)dest;
        int sv = *(int*)src;
        int diff = dv - sv;
        int *zf = (int*)hashmap_get(cpu->context, "ZF");
        int *sf = (int*)hashmap_get(cpu->context, "SF");
        if (zf) *zf = (diff == 0);
        if (sf) *sf = (diff < 0);
        return 0;
    }

    // JMP address
    if (strcmp(instr->mnemonic, "JMP") == 0) {
        int addr = *(int*)src;
        int *ip = (int*)hashmap_get(cpu->context, "IP");
        if (ip) *ip = addr;
        return 0;
    }

    // JZ address
    if (strcmp(instr->mnemonic, "JZ") == 0) {
        int *zf = (int*)hashmap_get(cpu->context, "ZF");
        int *ip = (int*)hashmap_get(cpu->context, "IP");
        if (zf && *zf == 1 && ip) *ip = *(int*)src;
        return 0;
    }

    // JNZ address
    if (strcmp(instr->mnemonic, "JNZ") == 0) {
        int *zf = (int*)hashmap_get(cpu->context, "ZF");
        int *ip = (int*)hashmap_get(cpu->context, "IP");
        if (zf && *zf == 0 && ip) *ip = *(int*)src;
        return 0;
    }

    // HALT
    if (strcmp(instr->mnemonic, "HALT") == 0) {
        Segment *cs = (Segment*)hashmap_get(cpu->memory_handler->allocated, "CS");
        int *ip = (int*)hashmap_get(cpu->context, "IP");
        if (cs && ip) *ip = cs->size;
        return 0;
    }

	// ALLOC : utilise AX (taille) et BX (stratégie) → ES, ZF
	if (strcmp(instr->mnemonic, "ALLOC") == 0) {
		return alloc_es_segment(cpu) == 0 ? 0 : -1;
	}

	// FREE : libère ES, met ES à -1
	if (strcmp(instr->mnemonic, "FREE") == 0) {
		return free_es_segment(cpu) == 0 ? 0 : -1;
	}

    // Instruction non reconnue
    return -1;
}

int execute_instruction(CPU *cpu, Instruction *instr) {
    if (!cpu || !instr || !instr->mnemonic) return -1;

    // Résolution des opérandes : operand2 => src, operand1 → dest
    void *src  = NULL;
    void *dest = NULL;

    if (instr->operand2 && instr->operand2[0] != '\0') {
        src = resolve_addressing(cpu, instr->operand2);
    }
    if (instr->operand1 && instr->operand1[0] != '\0') {
        dest = resolve_addressing(cpu, instr->operand1);
    }

    return handle_instruction(cpu, instr, src, dest);
}