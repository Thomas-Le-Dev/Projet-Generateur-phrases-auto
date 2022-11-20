/*******************************************************************************
* Projet Générateur automatique de phrases                                     *
* Description du fichier : Fichier main                                        *
*                                                                              *
* Auteurs : Thomas TOURNIER, Mehdy MICHALAK, Adem DEBBAHI                      *
*******************************************************************************/

#ifndef FONCTIONS_H
#define FONCTIONS_H

// Decommentez une des lignes suivantes pour activer le mode debug
// (Niveau 2 = affichage du dictionnaire lu)
//#define DEBUG 1
//#define DEBUG 2

// Choisissez l'un des dictionnaires
//#define DICTIONNAIRE "C:\\Thomas\\Travail\\EFREI\\L2\\Informatique\\Language C\\Projet C\\Generateur automatique de phrases\\minidico.txt"
#define DICTIONNAIRE "C:\\Thomas\\Travail\\EFREI\\L2\\Informatique\\Language C\\Projet C\\Generateur automatique de phrases\\dictionnaire_non_accentue.txt"
//#define DICTIONNAIRE "C:\\Thomas\\Travail\\EFREI\\L2\\Informatique\\Language C\\Projet C\\Generateur automatique de phrases\\dictionnaire.txt"

#define TAILLE_LIGNE 1024   // une ligne d'un fichier de dictionnaire
#define TAILLE_PHRASE 1024  // une chaîne de caracteres

// Catégories grammaticales
typedef enum
{
    NOM = 1,
    ADJECTIF = 2,
    ADVERBE = 3,
    VERBE = 4
} CATEGORIE;

// Genres
typedef enum
{
    GENRE_INDEFINI,
    MASCULIN,
    FEMININ,
    INVARIABLE
} GENRE;

// Nombres
typedef enum
{
    NOMBRE_INDEFINI,
    SINGULIER,
    PLURIEL,
    NOMBRE_INVARIABLE
} NOMBRE;

// Conjugaisons
typedef enum
{
    CONJUGAISON_INDEFINIE,
    INFINITIF,
    INDICATIF_PRESENT,
    INDICATIF_IMPARFAIT,
    SUBJONCTIF_PRESENT
} CONJUGAISON;

// Forme fléchie pour un nom
// - Les différentes chaînes sont NULL si non définies
// - Elles peuvent pointer vers la même valeur
typedef struct
{
    GENRE genre;
    char* SingulierMasculin;
    char* SingulierFeminin;
    char* PlurielMasculin;
    char* PlurielFeminin;
} FF_NOM;

// Forme fléchie pour un adjectif
// - Les différentes chaînes sont NULL si non définies
// - Elles peuvent pointer vers la même valeur
typedef struct
{
    char* SingulierMasculin;
    char* SingulierFeminin;
    char* PlurielMasculin;
    char* PlurielFeminin;
} FF_ADJECTIF;

// Forme fléchie pour le temps d'un verbe
// - Les différentes chaînes sont NULL si non définies
// - Elles peuvent pointer vers la même valeur
typedef struct
{
    char* SingulierP1;
    char* SingulierP2;
    char* SingulierP3;
    char* PlurielP1;
    char* PlurielP2;
    char* PlurielP3;
} TEMPS;

// Forme fléchie pour un verbe
// - Les différentes chaînes sont NULL si non définies
// - Elles peuvent pointer vers la même valeur
typedef struct
{
    char* InfinitifPresent;
    TEMPS IndicatifPresent;
    TEMPS IndicatifImparfait;
    TEMPS SubjonctifPresent;
} FF_VERBE;

// Il n'y a pas de forme fléchie pour un adverbe
// Ceux-ci n'ont rien d'autre à stocker !

// Un noeud d'un arbre de dictionnaire
typedef struct noeud
{
    int nombre_formes_flechies;

    // On n'aura qu'une seule catégorie présente à la fois car il y aura un arbre par catégorie grammaticale
    union
    {
        void* pointeur;
        FF_NOM* nom;
        FF_ADJECTIF* adjectif;
        FF_VERBE* verbe;
    } u;

    // Chaque noeud contient 256 pointeurs vers tous les caractères possibles
    // La plupart de ces pointeurs vaudront NULL ce qui indiquera que le caractère
    // correspondant n'existera pas à l'étage inférieur
    struct noeud* caractere[256];
} NOEUD;

// Prototype des fonctions
extern void charger_dictionnaire();
extern void initialiser_nombres_aleatoires();
extern NOEUD* creer_noeud();
extern void ajouter_mot(NOEUD* arbre, char* forme_base, char* forme_flechie, CATEGORIE categorie, char* formes);
extern int menu_principal();
extern CATEGORIE menu_categorie();
extern void afficher_forme_flechie_nom(NOEUD* feuille);
extern void afficher_forme_flechie_verbe(NOEUD* feuille, char* verbe);
extern void afficher_forme_flechie_adjectif(NOEUD* feuille);
extern void afficher_forme_flechie_adverbe(char* mot);
extern NOEUD* rechercher_mot(char* mot, NOEUD* arbre, CATEGORIE categorie);
extern int compter_feuilles(NOEUD* feuille);
extern char* extraire_forme_base_aleatoirement(NOEUD* arbre, CATEGORIE categorie, char* mot, char* dernier_mot);
extern void generateur_phrase_base_modele_1();
extern void generateur_phrase_base_modele_2();
extern void generateur_phrase_base_modele_3();
extern void generateur_phrase_flechie_modele_1();
extern void generateur_phrase_flechie_modele_2();
extern void generateur_phrase_flechie_modele_3();
extern int menu_generateur_phrase();
extern void ajouter_nom(char* phrase, GENRE* pt_genre, int* pt_nombre, int article);
extern void ajouter_adjectif(char* phrase, GENRE* pt_genre, int* pt_nombre);
extern void ajouter_verbe_p3(char* phrase, int* pt_nombre, int* pt_temps);
extern void ajouter_verbe_p1p2(char* phrase, int* pt_nombre, int* pt_temps);
extern int rechercher_forme_flechie(char* mot_recherche, NOEUD* feuille, CATEGORIE categorie, char* mot_courant);
extern void afficher_arbre(NOEUD* feuille, CATEGORIE categorie, char* forme_de_base, int* pt_nb_formes_de_base, int* pt_nb_formes_flechies, int* pt_nb_feuilles);

#endif //FONCTIONS_H
