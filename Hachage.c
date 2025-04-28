#include "Hachage.h"
unsigned long simple_hash(const char *str){
	unsigned long hash = 0;
	while(*str){
		hash += (unsigned long)*str++;
	}
	return  hash % TABLE_SIZE;
}

HashMap *hashmap_create(){
	HashMap *map = malloc(sizeof(HashMap));
	if(!map) return NULL;
	map->size = 0;
	map->table = calloc(TABLE_SIZE, sizeof(HashEntry));	// Initialise à NULL
	return map;
}

int hashmap_insert(HashMap *map, const char *key, void *value){
	if (!map || !key) return -1;	// Check des paramètres
	unsigned long i = simple_hash(key);
	unsigned long start = i;	// Sauvegarde du point de départ
	do{
		// Cas 1 : Case vide ou supprimée -> insertion
		if(map->table[i].key == NULL || map->table[i].value == TOMBSTONE){
			map->table[i].key = strdup(key);	// Copie de la clé
			map->table[i].value = value;	// Insertion de la valeur
			map->size++;
			return 0;
		}
		// Cas 2 : Clé déjà existante -> mise à jour de la valeur
		if(strcmp(map->table[i].key, key) == 0){
			map->table[i].value = value;
			return 0;
		}
		//Cas 3 : Collision
		i = (i + 1) % TABLE_SIZE;
	}while (i != start);
	return -1;  // Table pleine, insertion impossible
}

void *hashmap_get(HashMap *map, const char *key){
	if (!map || !key) return NULL;	// Vérification des paramètres
	unsigned long i = simple_hash(key);
	unsigned long start = i;	// Sauvegarde du point de départ
	do {
		//Cas 1 : Clé n'existe pas
		if(map->table[i].key == NULL){
			return NULL;
		}
		//Cas 2 : Clé trouvé
		if(strcmp(map->table[i].key, key) == 0 && map->table[i].value != TOMBSTONE){
			return map->table[i].value;
		}
		//Cas 3 : Collision
		i = (i + 1) % TABLE_SIZE;
	} while (i != start);

	return NULL;
}
int hashmap_remove(HashMap *map, const char *key){
	if (!map || !key) return 0;	// Vérification des paramètres
	unsigned long i = simple_hash(key);
	unsigned long start = i;	// Sauvegarde du point de départ
	do{
		//Cas 1 : Clé n'existe pas
		if(map->table[i].key == NULL){
			return 0;
		}
		//Cas 2 : Clé trouvé
		if(strcmp(map->table[i].key, key) == 0 && map->table[i].value != TOMBSTONE){
			map->table[i].value = TOMBSTONE;
			return 0;
		}
		//Cas 3 : Collision
		i = (i + 1) % TABLE_SIZE;
	} while (i != start);  // Boucle jusqu’à revenir au point de départ
	
	return 0;
}

void hashmap_destroy(HashMap *map){
	if (!map) return ;	// Vérification des paramètres
	for (int i = 0; i < TABLE_SIZE; i++) {
        // Si la case contient une vraie clé (différente de NULL et de TOMBSTONE),
        // on la libère
        if (map->table[i].key && map->table[i].key != TOMBSTONE) {
            free(map->table[i].key);
        }
    }
	free(map->table);
	free(map);
}