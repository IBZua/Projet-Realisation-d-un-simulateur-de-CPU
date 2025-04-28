#include "Parser.h"

// offset courant pour la section .DATA
static int data_offset = 0;
//fonction auxiliaire de calcul taille op2
int size(char *operand2){
	int res = 1;
	int i = 0;
	while(operand2[i]){
		if(operand2[i] == ','){
			res++;
		}
		i++;
	}
	return res;
}

Instruction *parse_data_instruction(const char *line, HashMap *memory_locations){
	char mnemonic[64], operand1[64], operand2[256] ;
	if(sscanf(line, "%63s %63s %255[^\n]", mnemonic, operand1, operand2) == 3){
		//nb el
		int nb_el = size(operand2);
		//insertion
		int current_address = data_offset;
		//
		hashmap_insert(memory_locations, mnemonic,  (void*)(long)current_address);
		//
		data_offset += nb_el;
		//Creation insturction
		Instruction *inst = malloc(sizeof(Instruction));
		inst->mnemonic = strdup(mnemonic);
		inst->operand1 = strdup(operand1);
		inst->operand2 = strdup(operand2);
		return inst;
	}else{
		printf("erreur format DATA:la ligné doit être au format mnemonic op1 op2\n");
		printf("%s", line);
		return NULL;
	}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
}

Instruction *parse_code_instruction(const char *line, HashMap *labels, int code_count){
	
	char label[64] = "";
	char mnemonic[64] = "";
	char operand1[64] = "";
	char operand2[128] = "";
	char remainder[512] = "";
	//chercher l'absence de label
	if(!strchr(line, ':')){    
		if(sscanf(line, "%63s %63[^,],%127[^\n]", mnemonic, operand1, operand2) == 3){
			//Creation insturction
			Instruction *inst = malloc(sizeof(Instruction));
			inst->mnemonic = strdup(mnemonic);
			inst->operand1 = strdup(operand1);
			inst->operand2 = strdup(operand2);
			return inst;
		}else{
			printf("erreur format CODE :la ligné doit être au format mnemonic op1 op2\n");
			printf("%s", line);
			return NULL;
		}
	}else{
		//l'inserer dans la table hash
		// Si label se termine par ':', coupes
		if(sscanf(line, "%63s %511[^\n]", label, remainder) < 1) return NULL;
		char *pos = strrchr(label, ':');
		if (pos) *pos = '\0';
		hashmap_insert(labels, label, (void*)(long)code_count);
		//recup le mnemo et les op
		if(sscanf(remainder, " %63s %63[^,],%127[^\n]", mnemonic, operand1, operand2) == 3){
			//Creation insturction
			Instruction *inst = malloc(sizeof(Instruction));
			inst->mnemonic = strdup(mnemonic);
			inst->operand1 = strdup(operand1);
			inst->operand2 = strdup(operand2);
			return inst;
		}else{
			printf("errreur:la ligné doit être au format mnemonic op1 op2");
			return NULL;
		}
	}
}

ParserResult *parse(const char *filename){
	FILE *f = fopen(filename, "r");
	if(!f) return NULL;
	char line[256];
	// On alloue le ParserResult
	ParserResult *result = malloc(sizeof(ParserResult));
	if (!result) {
        fclose(f);
        return NULL;
    }
	//Initialisations
	result->data_instructions = NULL;
    result->code_instructions = NULL;
	result->code_count = 0;
	result->data_count = 0;
	result->labels = hashmap_create();
	result->memory_locations= hashmap_create();

	//flag
	int in_code = 0;
	int in_data = 0;
	Instruction *instr = NULL;
	while(fgets(line, sizeof(line), f)){
		if(!strncmp(line, ".CODE",5)){
			in_code = 1;
			in_data = 0;
			continue;
		}else if(!strncmp(line, ".DATA",5)){
			in_data = 1;
			in_code = 0;
			continue;
		}
		//On agrandit le tableau d'instructions et on ajoute l'instruction à son emplacement
		if(in_code){
			instr = parse_code_instruction(line, result->labels, result->code_count);
			result->code_instructions = realloc(result->code_instructions, sizeof(Instruction*) * (result->code_count + 1));
			result->code_instructions[result->code_count] = instr;
			result->code_count++;
		}else if(in_data){
			instr = parse_data_instruction(line, result->memory_locations);
			result->data_instructions = realloc(result->data_instructions, sizeof(Instruction*) * (result->data_count + 1));
			result->data_instructions[result->data_count] = instr;
			result->data_count++;
		}
	}
	fclose(f);
	return result;
}

void free_parser_result(ParserResult *result){
	if (!result) return;

    // 1) Libérer chaque instruction .DATA car chaque instruction contient des champs qu’on a dupliqués avec strdup
    for (int i = 0; i < result->data_count; i++) {
        Instruction *inst = result->data_instructions[i];
        if (inst) {
            // free les champs de l’instruction
            free(inst->mnemonic);
            free(inst->operand1);
            free(inst->operand2);
            // free la structure elle-même
            free(inst);
        }
    }
    // Puis libérer le tableau data_instructions
	free(result->data_instructions);
	 // 2) free chaque instruction .CODE
	 for (int i = 0; i < result->code_count; i++) {
        Instruction *inst = result->code_instructions[i];
        if (inst) {
            free(inst->mnemonic);
            free(inst->operand1);
            free(inst->operand2);
            free(inst);
        }
    }
	free(result->code_instructions);
	hashmap_destroy(result->labels);
	hashmap_destroy(result->memory_locations);
	free(result);
}