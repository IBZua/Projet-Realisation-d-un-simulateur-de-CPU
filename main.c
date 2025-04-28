#include "CPU.h"
int main(int argc, char **argv){
    //Test du Parser
    ParserResult *input = parse(argv[1]);
    free_parser_result(input);

    CPU *cpu = setup_test_environment();

    //Test de l'adressage immédiat
    int *imm = (int *) immediate_addressing(cpu, "123");
    printf("Immédiat \"123\" => %d\n", imm ? *imm : -1);

    //Test de l'adressage par registre
    int *rax = (int *) register_addressing(cpu, "AX");
    printf("Registre AX initial = %d\n", rax ? *rax : -1);

    //MOV imm → AX
    handle_MOV(cpu, imm, rax);
    printf("Après MOV imm=>AX, AX = %d\n", *rax);

    // Test de l'adressage mémoire direct
    int *mem5 = (int *) memory_direct_addressing(cpu, "[5]");
    printf("Direct [5] = %d\n", mem5 ? *mem5 : -1);

    // MOV AX => [5]
    handle_MOV(cpu, rax, mem5);
    printf("Après MOV AX=>[5], [5] = %d\n", *mem5);

    // Test de l'adressage indirect par registre
    //BX à l'indice 7 puis on lit [BX]
    int *rbx = (int *) register_addressing(cpu, "BX");
    *rbx = 7;
    int *ind7 = (int *) register_indirect_addressing(cpu, "[BX]");
    printf("Indirect [BX] (BX=7) = %d\n", ind7 ? *ind7 : -1);

    // MOV imm(55) → [BX]
    int *imm2 = (int *) immediate_addressing(cpu, "55");
    handle_MOV(cpu, imm2, ind7);
    //relecture mem
    int *mem7 = (int *) memory_direct_addressing(cpu, "[7]");
    printf("Après MOV imm2=>[BX], DS[7] = %d\n", mem7 ? *mem7 : -1);
    run_program(cpu);

     // === Tests PUSH / POP ===
     printf("\n== Test PUSH / POP ==\n");
     // On part de SP initial
     int *sp = (int*)hashmap_get(cpu->context, "SP");
     printf("SP initial = %d\n", *sp);
     // PUSH immédiat 42
     int *imm3 = (int*)immediate_addressing(cpu, "42");
     push_value(cpu, *imm3);
     printf("Après PUSH 42, SP = %d\n", *sp);
     // POP dans BX
     int popped;
     pop_value(cpu, &popped);
     printf("POP => %d, SP = %d\n", popped, *sp);
 
     // === Tests ALLOC / FREE ===
     printf("\n== Test ALLOC / FREE (ES) ==\n");
     // Préparer AX (taille) et BX (stratégie)
     *(int*)hashmap_get(cpu->context, "AX") = 5;
     *(int*)hashmap_get(cpu->context, "BX") = 0;  // First Fit
     alloc_es_segment(cpu);
     printf("ALLOC 5 en ES, ES = %d, ZF = %d\n",
         *(int*)hashmap_get(cpu->context, "ES"),
         *(int*)hashmap_get(cpu->context, "ZF"));
     // Remplissage du segment
     for(int i = 0; i < 5; i++){
         int *v = (int*)load(cpu->memory_handler, "ES", i);
         printf("  ES[%d] = %d\n", i, v ? *v : -1);
     }
     // FREE
     free_es_segment(cpu);
     printf("Après FREE, ES = %d, ZF (non modifié) = %d\n",
         *(int*)hashmap_get(cpu->context, "ES"),
         *(int*)hashmap_get(cpu->context, "ZF"));
         
    //free
    cpu_destroy(cpu);
    return 0;
}