# Simulateur de CPU en C

[English](README.md) · Français

Un projet universitaire pour explorer un processeur simplifié : registres, mémoire segmentée, modes d'adressage et traitement des instructions. L'implémentation est écrite en C et utilise une table de hachage pour les états nommés du CPU.

## Contenu du dépôt

| Composant | Responsabilité |
| --- | --- |
| [CPU.c](CPU.c), [CPU.h](CPU.h) | Initialisation, résolution des adresses, traitement des instructions, pile et allocation d'un segment supplémentaire. |
| [Memoire.c](Memoire.c), [Memoire.h](Memoire.h) | Segments mémoire et gestion des allocations. |
| [Hachage.c](Hachage.c), [Hachage.h](Hachage.h) | Table de hachage utilisée pour les registres et les états nommés. |
| [Parser.c](Parser.c), [Parser.h](Parser.h) | Analyse de l'assembleur simplifié. |
| [main.c](main.c), [input.txt](input.txt) | Point d'entrée de démonstration et fichier d'exemple. |

Le CPU représente notamment `AX`, `BX`, `CX`, `DX`, `IP` et des indicateurs d'état. Les fonctions d'adressage couvrent les valeurs immédiates, registres, accès directs à la mémoire, accès indirects par registre et substitutions de segment. Les traitements présents comprennent `MOV`, `ADD`, `CMP`, `JMP`, `JZ`, `JNZ`, `HALT`, `ALLOC` et `FREE`. Les fonctions d'empilement et de dépilement sont démontrées séparément.

## Compiler et exécuter

Utiliser un environnement POSIX avec **GCC et GNU Make**, par exemple Linux ou Debian sous WSL. Le code utilise les expressions régulières POSIX (`regex.h`). Depuis la racine du dépôt :

```sh
make
./main input.txt
```

Le programme affiche des exemples d'adressage, l'état des registres et de la mémoire, une démonstration de pile et une allocation/libération. Extraits du passage vérifié :

```text
Test environment initialized.
POP => 42, SP = 1023
```

Pour retirer les fichiers de compilation :

```sh
make clean
```

## Fonctionnement de la démonstration

Le point d'entrée analyse le fichier passé en premier argument, puis libère le résultat de cette analyse. Il crée un CPU de test distinct et manipule l'adressage, les transferts mémoire, la pile et l'allocation de segments.

**Le point d'entrée actuel ne charge pas les instructions analysées dans le segment de code du CPU.** Ainsi, `./main input.txt` lance les exemples intégrés ; il n'exécute pas ce fichier assembleur comme un programme complet. Les traitements d'instructions et une boucle de lecture/exécution existent dans les sources, mais le point d'entrée ne raccorde pas tout le parcours.

## Vérification et limites

Le **17 novembre 2026**, `make` et `./main input.txt` se sont terminés avec un code de sortie 0 sous Debian/WSL. La compilation n'a émis aucun avertissement lors de ce passage. L'exemple inclut l'empilement puis le dépilement réussi de `42`.

Il s'agit de vérifications de compilation et de démonstration, et non d'une suite d'assertions automatisées ou d'une validation de chaque instruction. Le point d'entrée attend un fichier existant et ne vérifie pas le nombre d'arguments avant d'utiliser `argv[1]` : fournir le nom de fichier indiqué. Le simulateur est un modèle pédagogique, pas l'implémentation du jeu d'instructions d'un processeur réel.

[Autres projets d'Illia](https://github.com/IBZua/project-showcase/blob/main/README.fr.md)
