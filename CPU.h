#include "Parser.h"
#include <regex.h>
#include <limits.h>

typedef struct {
    MemoryHandler *memory_handler ; // Gestionnaire de memoire
    HashMap *context; // Registres (AX, BX, CX, DX)
    HashMap * constant_pool ; // Table de hachage pour stocker les valeurs immediates
}CPU;
CPU *cpu_init(int memory_size);
void* store(MemoryHandler *handler, const char *segment_name, int pos, void *data);
void* load(MemoryHandler *handler, const char *segment_name, int pos);
void cpu_destroy(CPU *cpu);
void *immediate_addressing(CPU *cpu, const char *operand);
void *register_addressing(CPU *cpu, const char *operand);
void *memory_direct_addressing(CPU *cpu, const char *operand);
void *register_indirect_addressing(CPU *cpu, const char *operand);
void handle_MOV(CPU* cpu, void* src, void* dest);
CPU *setup_test_environment();
int resolve_constants(ParserResult *result);
int execute_instruction(CPU *cpu, Instruction *instr);
Instruction* fetch_next_instruction(CPU *cpu);
int run_program(CPU *cpu);
int push_value(CPU *cpu, int value);
int pop_value(CPU *cpu, int *dest);
void *segment_override_addressing(CPU *cpu, const char *operand);
int find_free_address_strategy(MemoryHandler *handler, int size, int strategy);
int alloc_es_segment(CPU *cpu);
int free_es_segment(CPU *cpu);