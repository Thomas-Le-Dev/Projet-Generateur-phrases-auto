Projet Générateur automatique de phrases


Par Thomas TOURNIER, Mehdy MICHALAK et Adem DEBBAHI (L2 - D)


Le projet se constitue de différents programmes :
    -main.c
    -fonctions.c
    -fonctions.h

    -dictionnaire.txt


Avant l'exécution, l'utilisateur doit mettre tous les fichiers et programmes dans
le même répertoire. Il doit aussi imperativement copier son path depuis la racine de son ordinateur et
l'inserer à la ligne 19 du fichier fonctions.h dans le define.
Par exemple : #define DICTIONNAIRE "C:\\Thomas\\Travail\\EFREI\\L2\\Informatique\\Language C\\Projet C\\Generateur automatique de phrases\\dictionnaire.txt"
Attention à bien mettre des doubles back slash "\\" sous windows pour que le programme s'éxecute.


Pour exécuter le programme, il faut se placer dans ce répertoire et exécuter
le programme "main.c", puis saisir dans la console ce qui est demandé.

Le programme propose 4 fonctionnalités :
1 : Rechercher un mot
2 : Extraire un mot au hasard
3 : Generer une phrase au hasard selon un modele
4 : Rechercher un mot parmi les formes flechies

Lors du lancement, l'utilisateur se trouve dans le menu principal dans lequel il peut
choisir quelle fonctionnalité il va executer, puis le programme va afficher
un sous-menu dans chaque cas et l'utilisateur pourra choisir une action à effectuer.

Parfois il sera demandé d'entrer une chaîne de caractères au lieu d'un entier afin 
d'effectuer certaines actions.

Pour quitter le programme veuillez saisir "0" dans le menu ou les sous-menus.

Option : Si vous voulez activer le mode DEBUG pour mieux comprendre l'execution du programme,
Enlever le commentaire devant le "#define DEBUG 1" dans le fichier fonctions.h.