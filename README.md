# Readme_Groupe_M4

## Algorithme de Bellman-Ford en C

***

Le programme de ce projet implemente l'algorithme de Bellman-Ford, il nous permet de trouver le chemin le plus court / moins coûteux entre 2 noeuds d'un graphe. 
Il est basé sur un code python qui nous a été fourni et vise à améliorer les performances de ce dernier avec une approche multi-thread.

### Comment lancer le programme

Il faut avoir les outils/librairies suivantes pour éviter tout soucis de compilation : CUnit, Valgrind, GCC 

Pour lancer le programme, il faut d'abord le compiler, voici quelques commandes utiles pour le faire :
 - **make/make sp** : Permet de compiler le programme de façon à ce qu'il puisse être utilisé, cela crée un executable 'main' que vous pouvez executer, pour plus d'informations sur le programme main utiliser ./main -h
 - **make tests** : Permet d'executer une serie de tests pour s'assurer que le programme fonctionne bien comme attendu, ceci compile les tests CUnit et les fait passer par valgrind pour vous assurer qu'aucune fuite de memoire est présente, ensuite le programme main est compile si ce n'est pas encore fait et fait passer main par valgrind pour vérifier les fuites de mémoire ainsi que la synchronisation entre threads.
 - **make clean** : Supprime tout les fichiers supplémentaires qui ont étés créés lors de l'utilisation des 2 commandes précedentes 

Pour des soucis de clareté utiliser toujours **make clean** après avoir fini d'utiliser le programme, le makefile fait en sorte que peu importe l'ordre des commandes, il ne soit pas nécessaire de faire autre chose pour faire fonctionner ce que vous voulez.  

### Utilisation de la fonction main 

La fonction main prend comme arguments : 
 - *-n* : Le nombre de threads que le programme va utiliser. 
 - *-v* : Affichage des messages d'erreur, false par défaut.
 - *-f* : Le fichier dans lequel les données seront sauvegardées.
 - *-?* / *-h* : Affiche une fenêtre avec cette explication.
 - Le nom du fichier avec le graph.

### Utilisation du Makefile 

Le Makefile a été fortemment modifié afin de faire face à toutes les fonctionnalités que nous avons jugés utiles, il est donc important de savoir comment l'utiliser.

Premièrement, si à tout moments vous avez besoin d'aide, vous pouvez utiliser la commande **make help** pour avoir une liste des commandes disponibles.
Celles-ci sont les suivantes :
 - **make sp** : compile le programme principal.
 - **make run** : compile et exécute le programme principal avec les paramètres par défaut.
 - **make time** : compile et exécute le programme principal avec plusieurs nombres de threads et affiche le temps d'exécution pour chacun d'entre eux.
 - **make myfile** : compile et exécute le programme principal avec un fichier d'entrée personnalisé (voir plus bas).
 - **make mygraph** : compile et exécute le programme principal avec un graphe personnalisé (voir plus bas).
 - **make tests** : compile et exécute les tests unitaires.
 - **make clean** : supprime les fichiers compilés et les fichiers de sortie.
 - **make help** : affiche l'aide.

### Utilisation de graph personnels

Ce projet vous permet de créer votre propre graphe, d'executer le programme sur ce graph et de vérifier que cette réponse est correcte. Vous avez 2 possibilités pour le faire :
 - Vous avez déja votre fichier binaire ? 
    Utilisez simplement la commande **make myfile**, celle-ci vous demandera le nom / chemin du fichier binaire que vous voulez utiliser, puis elle exécutera le programme sur ce fichier et enregistrera le résultat dans le fichier myGraphOut.bin qui sera stocké dans le dossier outputs. 

 - Si vous n'avez pas de fichier binaire ? 
    Mettez votre graphe dans un fichier ntf avec la bonne structure (pour plus d'infos, voir examples dans le dossier ntf_files) et utiliser la commande **make mygraph**. Cette commande vous demandera le nom /chemin de votre fichier et se chargera de tout le reste, elle crée le fichier binaire, exécute le programme et enregistre le résultat dans le fichier myGraphOut.bin qui sera stocké dans le dossier outputs. 

Remarques : 
 - Vos fichiers ne seront pas supprimés en utilisant la commande **make clean** s'ils restent dans le dossier outputs (comportement par défaut de **make mygrpah** et **make myfile**). Si vous voulez les supprimer, spécifiez juste le nom du fichier de sortie quand vous le passez au programme.
 - Une limitation des 2 commandes (**make mygraph** et **make myfile**) est que le programme sera toujours executé avec le nombre de threads par défaut (4). Si vous voulez changer cela, vous devez exécuter le programme manuellement avec la commande **./main** en spécifiant tout ce qui est nécessaire (voir plus haut).

### Explication rapide des fonctions principales 
Les fonction principales qui sont utilisées dans le programme sont les suivantes :
 - la fonction **get_file_info** qui va ouvrir le fichier, le lire et retourner le nombre de noeuds et de liens.
 - la fonction **bellman_ford** qui à partir d'un noeud source 's' va retourner un tableau avec tous les autres noeuds et la distance pour y parvenir. Si le cycle est négatif, la distance retournée sera infinie. Si le noeud est isolé, la distance isolée sera infinie. Pour ce faire, elle initialise la distance à 0 et pour chaque noeud va tester tous les chemins avec lesquels il est possible d'y arriver. Dès qu'un chemin meilleur que le précedent sera trouvé, elle va remplacer l'ancien par celui là. Elle va faire ça pour tous les noeuds et pouvoir dresser le tableau.
 - la fonction **get_max** prend come argument un noeud source et retourne l'indice du noeud dont il existe un chemin vers ce noeud et le coût de ce chemin est le plus élevé parmis tous les noeuds ayant un chemin vers 's'.
 - la fonction **get_path** prend en argument le noeud source, le noeud auquel on veut arriver ainsi que le tableau retourné dans la fonction belmanford.c et va retourner une liste décrivant le plus court chemin pour relier ces deux noeuds.
 - la fonction **write_to_file** permet d'écrire les informations qui lui sont passées dans le fichier de sortie.
 - la fonction **main** qui permet de relier toutes les fonctions entre elles assurer le bon fonctionnement du programme.

Pour plus d'informations sur les fonctions tels que les types précis d'arguments, des valeurs de retour, etc. vous pouvez vous référer à la docstring dans le code (fichier *graph.c/graph.h* et *mtgraph.c/mtgraph.h*).
