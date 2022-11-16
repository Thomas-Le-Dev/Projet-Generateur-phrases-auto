/*******************************************************************************
* Projet Generateur automatique de phrases                                     *
* Description du fichier : Fichier main                                        *
*                                                                              *
* Auteurs : Thomas TOURNIER, Mehdy MICHALAK, Adem DEBBAHI                       *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Sous windows la fonction srandom() s'appelle srand()
// Et random() s'appelle rand()
#if defined(_WIN32) || defined(_WIN64)
# define srandom srand
# define random rand
#endif

#include "fonctions.h"

extern NOEUD* verbes;
extern NOEUD* noms;
extern NOEUD* adjectifs;
extern NOEUD* adverbes;

////////////////////////////////////////////////////////////////////////////////
void charger_dictionnaire()
{
    FILE *fp = fopen(DICTIONNAIRE, "r");
    
    if (fp == NULL)
    {
        fprintf(stderr, "ERREUR : fopen(%s) a echoue ! \n", DICTIONNAIRE);
        exit(1);
    }

    while (! feof(fp))
    {
        char ligne[TAILLE_LIGNE];

	    if (fgets(ligne, TAILLE_LIGNE, fp) != NULL)
	    {
            int taille_ligne;
	        char* forme_flechie;
	        char* forme_de_base;
	        char* description;
            char* categorie;
            char* formes;
            static char* adverbe = "Adv";

            // On elimine le saut de ligne que conserve fgets()
            taille_ligne = strlen(ligne);
            if (taille_ligne > 0 && (ligne[taille_ligne - 1] == '\n' || ligne[taille_ligne - 1] == '\r'))
                ligne[taille_ligne - 1] = 0;
            if (taille_ligne > 1 && (ligne[taille_ligne - 2] == '\n' || ligne[taille_ligne - 2] == '\r'))
                ligne[taille_ligne - 2] = 0;

            // Les lignes des fichiers de dictionnaire sont composees de champs separes par une tabulation
            forme_flechie = strtok(ligne, "\t");
    	    forme_de_base = strtok(NULL, "\t");
            description = strtok(NULL, "\t");
    
            if (! strcmp(description, "Adv"))
            {
                categorie = adverbe;
                formes = NULL;
            }
            else
            {
                categorie = strtok(description, ":");
                formes = strtok(NULL, "\t");
            }
            
            if (strstr(categorie, "Nom") != NULL) 
            {
#if DEBUG > 1
                printf("%s %s %s %s\n", forme_flechie, forme_de_base, categorie, formes);
#endif
                ajouter_mot(noms, forme_de_base, forme_flechie, NOM, formes);
            }
            if (strstr(categorie, "Ver") != NULL) 
            {
#if DEBUG > 1
                printf("%s %s %s %s\n", forme_flechie, forme_de_base, categorie, formes);
#endif
                ajouter_mot(verbes, forme_de_base, forme_flechie, VERBE, formes);
            }
            if (strstr(categorie, "Adv") != NULL) 
            {
#if DEBUG > 1
                printf("%s %s %s\n", forme_flechie, forme_de_base, categorie);
#endif
                ajouter_mot(adverbes, forme_de_base, forme_flechie, ADVERBE, formes);
            }
            if (strstr(categorie, "Adj") != NULL) 
            {
#if DEBUG > 1
                printf("%s %s %s %s\n", forme_flechie, forme_de_base, categorie, formes);
#endif
                ajouter_mot(adjectifs, forme_de_base, forme_flechie, ADJECTIF, formes);
            }
        }
    }
    fclose(fp);
}

////////////////////////////////////////////////////////////////////////////////
void initialiser_nombres_aleatoires()
{
    unsigned int graine;

    // Initialisation du generateur de nombres aleatoires
    graine = (unsigned int) time(NULL);
    srandom(graine);
}

////////////////////////////////////////////////////////////////////////////////
// Initialisation d'un noeud d'arbre de dictionnaire
NOEUD* creer_noeud()
{
    NOEUD* noeud = NULL;

    noeud = malloc(sizeof(NOEUD));
    if (noeud == NULL)
    {
        fprintf(stderr, "ERREUR : malloc(%zu) a echoue !\n", sizeof(NOEUD));
        exit(1);
    }

    noeud -> nombre_formes_flechies = 0;
    noeud -> u.pointeur = NULL;
    for (int i = 0; i < 256 ; i++)
        noeud -> caractere[i] = NULL;

    return noeud;
}

////////////////////////////////////////////////////////////////////////////////
// Ajout d'un mot dans un arbre de dictionnaire
void ajouter_mot(NOEUD* arbre, char* forme_base, char* forme_flechie, CATEGORIE categorie, char* formes)
{
    int i = 0;
    NOEUD* feuille = arbre;
    char* chaine;
    char* forme;
    char* positionDansFormes;
    int forme_retenue = 0;

    while (forme_base[i] != 0)
    {
        if (feuille -> caractere[(unsigned char) forme_base[i]] == NULL) 
            // La lettre n'existe pas donc il faut creer le reste de l'arbre.
            feuille -> caractere[(unsigned char) forme_base[i]] = creer_noeud();

        // Soit la lettre existait deja soit on vient de la creer, on descend dans la feuille.
        feuille = feuille -> caractere[(unsigned char) forme_base[i]];
        i++;
    }

    // On alloue de l'espace pour stocker cette nouvelle forme flechie
    // peut-etre pour rien, par exemple si c'est un verbe d'un temps non utilise
    chaine = malloc(strlen(forme_flechie) + 1);
    if (chaine == NULL)
    {
        fprintf(stderr, "ERREUR : malloc(%zu) a echoue !\n", strlen(forme_flechie) + 1);
        exit(1);
    }
    strcpy(chaine, forme_flechie);

    // On analyse les differentes formes rencontrees
    // Par exemple :
    //     formes = IPre+SG+P1:IPre+SG+P3:SPre+SG+P1:SPre+SG+P3:ImPre+SG+P2
    // Puis par bloc separe par ':'
    //     forme = IPre+SG+P3
    //     element = IPre
    //     element = SG
    //     element = P3
    forme = strtok_r(formes, ":", &positionDansFormes);
    while (forme != NULL)
    {
        char* element;
        char* positionDansForme = NULL;
        GENRE genre = GENRE_INDEFINI;
        NOMBRE nombre = NOMBRE_INDEFINI;
        CONJUGAISON conjugaison = CONJUGAISON_INDEFINIE;
        int personne = 0;

        element = strtok_r(forme, "+", &positionDansForme);
        while (element != NULL)
        {
            if (! strcmp(element, "SG"))
                nombre = SINGULIER;
            else if (! strcmp(element, "PL"))
                nombre = PLURIEL;
            else if (! strcmp(element, "InvPL"))
                nombre = NOMBRE_INVARIABLE;
            else if (! strcmp(element, "Mas"))
                genre = MASCULIN;
            else if (! strcmp(element, "Fem"))
                genre = FEMININ;
            else if (! strcmp(element, "InvGen"))
                genre = INVARIABLE;
            else if (! strcmp(element, "Inf"))
                conjugaison = INFINITIF;
            else if (! strcmp(element, "IPre"))
                conjugaison = INDICATIF_PRESENT;
            else if (! strcmp(element, "IImp"))
                conjugaison = INDICATIF_IMPARFAIT;
            else if (! strcmp(element, "SPre"))
                conjugaison = SUBJONCTIF_PRESENT;
            else if (! strcmp(element, "P1"))
                personne = 1;
            else if (! strcmp(element, "P2"))
                personne = 2;
            else if (! strcmp(element, "P3"))
                personne = 3;

            element = strtok_r(NULL, "+", &positionDansForme);
        }

        if (categorie == NOM)
        {
            if (genre != GENRE_INDEFINI && nombre != NOMBRE_INDEFINI)
            {
                forme_retenue = 1;

                if (feuille -> u.nom == NULL)
                {
                    feuille -> u.nom = malloc(sizeof(FF_NOM));
                    if (feuille -> u.nom == NULL)
                    {
                        fprintf(stderr, "ERREUR : malloc(%zu) a echoue !\n", sizeof(FF_NOM));
                        exit(1);
                    }
                }

                feuille -> u.nom -> genre = genre;

                if ((genre == MASCULIN || genre == INVARIABLE) && (nombre == SINGULIER || nombre == NOMBRE_INVARIABLE))
                    feuille -> u.nom -> SingulierMasculin = chaine;
                if ((genre == FEMININ || genre == INVARIABLE) && (nombre == SINGULIER || nombre == NOMBRE_INVARIABLE))
                    feuille -> u.nom -> SingulierFeminin = chaine;
                if ((genre == MASCULIN || genre == INVARIABLE) && (nombre == PLURIEL || nombre == NOMBRE_INVARIABLE))
                    feuille -> u.nom -> PlurielMasculin = chaine;
                if ((genre == FEMININ || genre == INVARIABLE) && (nombre == PLURIEL || nombre == NOMBRE_INVARIABLE))
                    feuille -> u.nom -> PlurielFeminin = chaine;
            }
        }
        else if (categorie == ADJECTIF)
        {
            if (genre != GENRE_INDEFINI && nombre != NOMBRE_INDEFINI)
            {
                forme_retenue = 1;

                if (feuille -> u.adjectif == NULL)
                {
                    feuille -> u.adjectif = malloc(sizeof(FF_ADJECTIF));
                    if (feuille -> u.adjectif == NULL)
                    {
                        fprintf(stderr, "ERREUR : malloc(%zu) a echoue !\n", sizeof(FF_ADJECTIF));
                        exit(1);
                    }
                }

                if ((genre == MASCULIN || genre == INVARIABLE) && (nombre == SINGULIER || nombre == NOMBRE_INVARIABLE))
                    feuille -> u.adjectif -> SingulierMasculin = chaine;
                if ((genre == FEMININ || genre == INVARIABLE) && (nombre == SINGULIER || nombre == NOMBRE_INVARIABLE))
                    feuille -> u.adjectif -> SingulierFeminin = chaine;
                if ((genre == MASCULIN || genre == INVARIABLE) && (nombre == PLURIEL || nombre == NOMBRE_INVARIABLE))
                    feuille -> u.adjectif -> PlurielMasculin = chaine;
                if ((genre == FEMININ || genre == INVARIABLE) && (nombre == PLURIEL || nombre == NOMBRE_INVARIABLE))
                    feuille -> u.adjectif -> PlurielFeminin = chaine;
            }
        }
        else if (categorie == VERBE)
        {
            if (conjugaison != CONJUGAISON_INDEFINIE)
            {
                forme_retenue = 1;

                if (feuille -> u.verbe == NULL)
                {
                    feuille -> u.verbe = malloc(sizeof(FF_VERBE));
                    if (feuille -> u.verbe == NULL)
                    {
                        fprintf(stderr, "ERREUR : malloc(%zu) a echoue !\n", sizeof(FF_VERBE));
                        exit(1);
                    }
                }
        
                if (conjugaison == INFINITIF)
                    feuille -> u.verbe -> InfinitifPresent = chaine;
                else if (conjugaison == INDICATIF_PRESENT)
                {
                    if (nombre == SINGULIER)
                    {
                        if (personne == 1)
                            feuille -> u.verbe -> IndicatifPresent.SingulierP1 = chaine;
                        else if (personne == 2)
                            feuille -> u.verbe -> IndicatifPresent.SingulierP2 = chaine;
                        else if (personne == 3)
                            feuille -> u.verbe -> IndicatifPresent.SingulierP3 = chaine;
                    }
                    else if (nombre == PLURIEL)
                    {
                        if (personne == 1)
                            feuille -> u.verbe -> IndicatifPresent.PlurielP1 = chaine;
                        else if (personne == 2)
                            feuille -> u.verbe -> IndicatifPresent.PlurielP2 = chaine;
                        else if (personne == 3)
                            feuille -> u.verbe -> IndicatifPresent.PlurielP3 = chaine;
                    }
                }
                else if (conjugaison == INDICATIF_IMPARFAIT)
                {
                    if (nombre == SINGULIER)
                    {
                        if (personne == 1)
                            feuille -> u.verbe -> IndicatifImparfait.SingulierP1 = chaine;
                        else if (personne == 2)
                            feuille -> u.verbe -> IndicatifImparfait.SingulierP2 = chaine;
                        else if (personne == 3)
                            feuille -> u.verbe -> IndicatifImparfait.SingulierP3 = chaine;
                    }
                    else if (nombre == PLURIEL)
                    {
                        if (personne == 1)
                            feuille -> u.verbe -> IndicatifImparfait.PlurielP1 = chaine;
                        else if (personne == 2)
                            feuille -> u.verbe -> IndicatifImparfait.PlurielP2 = chaine;
                        else if (personne == 3)
                            feuille -> u.verbe -> IndicatifImparfait.PlurielP3 = chaine;
                    }
                }
                else if (conjugaison == SUBJONCTIF_PRESENT)
                {
                    if (nombre == SINGULIER)
                    {
                        if (personne == 1)
                            feuille -> u.verbe -> SubjonctifPresent.SingulierP1 = chaine;
                        else if (personne == 2)
                            feuille -> u.verbe -> SubjonctifPresent.SingulierP2 = chaine;
                        else if (personne == 3)
                            feuille -> u.verbe -> SubjonctifPresent.SingulierP3 = chaine;
                    }
                    else if (nombre == PLURIEL)
                    {
                        if (personne == 1)
                            feuille -> u.verbe -> SubjonctifPresent.PlurielP1 = chaine;
                        else if (personne == 2)
                            feuille -> u.verbe -> SubjonctifPresent.PlurielP2 = chaine;
                        else if (personne == 3)
                            feuille -> u.verbe -> SubjonctifPresent.PlurielP3 = chaine;
                    }
                }
            }
        }
    
        forme = strtok_r(NULL, ":", &positionDansFormes);
    }

    if (forme_retenue)
        feuille -> nombre_formes_flechies += 1;
    else if (categorie == ADVERBE)
    {
        feuille -> nombre_formes_flechies += 1;
        free(chaine);
    }
    else
        free(chaine);
}

////////////////////////////////////////////////////////////////////////////////
int menu_principal()
{
    char choix[81];

    printf("\n\nGenerateur automatique de phrases\n");
    printf("=================================\n\n");
    printf("Menu principal :\n");
    printf("----------------\n");
    printf("1 : Rechercher un mot\n");
    printf("2 : Extraire un mot au hasard\n");
    printf("3 : Generer une phrase au hasard selon un modele\n");
    printf("4 : Rechercher un mot parmi les formes flechies\n");
#ifdef DEBUG
    printf("\n");
    printf("5 : Afficher l'arbre des noms\n");
    printf("6 : Afficher l'arbre des adjectifs\n");
    printf("7 : Afficher l'arbre des adverbes\n");
    printf("8 : Afficher l'arbre des verbes\n");
#endif
    printf("0 : Quitter\n\n");

    do
    {
        printf("Quelle action voulez-vous faire ? --> ");
        scanf("%80s", choix); // %80s permet de saisir une chaîne d'au plus 80 caracteres
    }
    while (! (! strcmp(choix, "0")
              || ! strcmp(choix, "1")
              || ! strcmp(choix, "2")
              || ! strcmp(choix, "3")
              || ! strcmp(choix, "4")
#ifdef DEBUG
              || ! strcmp(choix, "5")
              || ! strcmp(choix, "6")
              || ! strcmp(choix, "7")
              || ! strcmp(choix, "8")
#endif
          )  );
    // strcmp() renvoie 0 si les chaînes de caracteres sont identiques

    return atoi(choix);
}

////////////////////////////////////////////////////////////////////////////////
CATEGORIE menu_categorie()
{
    char choix[81];

    printf("\nCategorie grammaticale souhaitee :\n");
    printf("----------------------------------\n");
    printf("1 : Nom\n");
    printf("2 : Adjectif\n");
    printf("3 : Adverbe\n");
    printf("4 : Verbe\n");
    printf("0 : Quitter\n\n");

    do
    {
        printf("Quelle action voulez vous faire ? --> ");
        scanf("%80s", choix); // %80s permet de saisir une chaîne d'au plus 80 caracteres
    }
    while (! (! strcmp(choix, "0") || ! strcmp(choix, "1") || ! strcmp(choix, "2") || ! strcmp(choix, "3") || ! strcmp(choix, "4")));
    // strcmp() renvoie 0 si les chaînes de caracteres sont identiques 

    return (CATEGORIE) atoi(choix);
}

////////////////////////////////////////////////////////////////////////////////
NOEUD* rechercher_mot(char* mot, NOEUD* arbre, CATEGORIE categorie)
{
    int i = 0;
    NOEUD* feuille = arbre;

    while (mot[i] != 0)
    {
        if (feuille -> caractere[(unsigned char) mot[i]] == NULL) 
            return NULL;
        else
        {
            feuille = feuille -> caractere[(unsigned char) mot[i]];
            i++;
        }
    }

    if (feuille -> nombre_formes_flechies)
        return feuille;

    // On n'a pas trouve le mot recherche
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
void afficher_forme_flechie_nom(NOEUD* feuille)
{
    if (feuille -> u.nom -> SingulierMasculin != NULL)
        printf("	'%s', nom masculin, singulier\n", feuille -> u.nom -> SingulierMasculin);
    if (feuille -> u.nom -> SingulierFeminin != NULL)
        printf("	'%s', nom feminin, singulier\n", feuille -> u.nom -> SingulierFeminin);
    if (feuille -> u.nom -> PlurielMasculin != NULL)
        printf("	'%s', nom masculin, pluriel\n", feuille -> u.nom -> PlurielMasculin);
    if (feuille -> u.nom -> PlurielFeminin != NULL)
        printf("	'%s', nom feminin, pluriel\n", feuille -> u.nom -> PlurielFeminin);
}   

////////////////////////////////////////////////////////////////////////////////
void afficher_forme_flechie_adjectif(NOEUD* feuille)
{
    if (feuille -> u.adjectif -> SingulierMasculin != NULL)
        printf("	'%s', adjectif au masculin, singulier\n", feuille -> u.adjectif -> SingulierMasculin);
    if (feuille -> u.adjectif -> SingulierFeminin != NULL)
        printf("	'%s', adjectif au feminin, singulier\n", feuille -> u.adjectif -> SingulierFeminin);
    if (feuille -> u.adjectif -> PlurielMasculin != NULL)
        printf("	'%s', adjectif au masculin, pluriel\n", feuille -> u.adjectif -> PlurielMasculin);
    if (feuille -> u.adjectif -> PlurielFeminin != NULL)
        printf("	'%s', adjectif au feminin, pluriel\n", feuille -> u.adjectif -> PlurielFeminin);
}

////////////////////////////////////////////////////////////////////////////////
void afficher_forme_flechie_adverbe(char* mot)
{
    printf("	'%s', adverbe\n", mot);
}

////////////////////////////////////////////////////////////////////////////////
void afficher_forme_flechie_verbe(NOEUD* feuille, char* verbe)
{
    printf("Conjugaison du verbe '%s':\n", verbe);
    printf("	Infinitif:\n");
    if (feuille -> u.verbe -> InfinitifPresent != NULL)
        printf("		%s\n", feuille -> u.verbe -> InfinitifPresent);
    printf("	Indicatif:\n");
    printf("		Present:\n");
    if (feuille -> u.verbe -> IndicatifPresent.SingulierP1 != NULL)
        printf("			1ere personne du singulier:  '%s'\n", feuille -> u.verbe -> IndicatifPresent.SingulierP1);
    if (feuille -> u.verbe -> IndicatifPresent.SingulierP2 != NULL)
        printf("			2eme personne du singulier:  '%s'\n", feuille -> u.verbe -> IndicatifPresent.SingulierP2);
    if (feuille -> u.verbe -> IndicatifPresent.SingulierP3 != NULL)
        printf("			3eme personne du singulier:  '%s'\n", feuille -> u.verbe -> IndicatifPresent.SingulierP3);
    if (feuille -> u.verbe -> IndicatifPresent.PlurielP1 != NULL)
        printf("			1ere personne du pluriel:  '%s'\n", feuille -> u.verbe -> IndicatifPresent.PlurielP1);
    if (feuille -> u.verbe -> IndicatifPresent.PlurielP2 != NULL)
        printf("			2eme personne du pluriel:  '%s'\n", feuille -> u.verbe -> IndicatifPresent.PlurielP2);
    if (feuille -> u.verbe -> IndicatifPresent.PlurielP3 != NULL)
        printf("			3eme personne du pluriel:  '%s'\n", feuille -> u.verbe -> IndicatifPresent.PlurielP3);
    printf("		Imparfait:\n");
    if (feuille -> u.verbe -> IndicatifImparfait.SingulierP1 != NULL)
        printf("			1ere personne du singulier:  '%s'\n", feuille -> u.verbe -> IndicatifImparfait.SingulierP1);
    if (feuille -> u.verbe -> IndicatifImparfait.SingulierP2 != NULL)
        printf("			2eme personne du singulier:  '%s'\n", feuille -> u.verbe -> IndicatifImparfait.SingulierP2);
    if (feuille -> u.verbe -> IndicatifImparfait.SingulierP3 != NULL)
        printf("			3eme personne du singulier:  '%s'\n", feuille -> u.verbe -> IndicatifImparfait.SingulierP3);
    if (feuille -> u.verbe -> IndicatifImparfait.PlurielP1 != NULL)
        printf("			1ere personne du pluriel:  '%s'\n", feuille -> u.verbe -> IndicatifImparfait.PlurielP1);
    if (feuille -> u.verbe -> IndicatifImparfait.PlurielP2 != NULL)
        printf("			2eme personne du pluriel:  '%s'\n", feuille -> u.verbe -> IndicatifImparfait.PlurielP2);
    if (feuille -> u.verbe -> IndicatifImparfait.PlurielP3 != NULL)
        printf("			3eme personne du pluriel:  '%s'\n", feuille -> u.verbe -> IndicatifImparfait.PlurielP3);
    printf("	Subjonctif:\n");
    printf("		Present:\n");
    if (feuille -> u.verbe -> SubjonctifPresent.SingulierP1 != NULL)
        printf("			1ere personne du singulier:  '%s'\n", feuille -> u.verbe -> SubjonctifPresent.SingulierP1);
    if (feuille -> u.verbe -> SubjonctifPresent.SingulierP2 != NULL)
        printf("			2eme personne du singulier:  '%s'\n", feuille -> u.verbe -> SubjonctifPresent.SingulierP2);
    if (feuille -> u.verbe -> SubjonctifPresent.SingulierP3 != NULL)
        printf("			3eme personne du singulier:  '%s'\n", feuille -> u.verbe -> SubjonctifPresent.SingulierP3);
    if (feuille -> u.verbe -> SubjonctifPresent.PlurielP1 != NULL)
        printf("			1ere personne du pluriel:  '%s'\n", feuille -> u.verbe -> SubjonctifPresent.PlurielP1);
    if (feuille -> u.verbe -> SubjonctifPresent.PlurielP2 != NULL)
        printf("			2eme personne du pluriel:  '%s'\n", feuille -> u.verbe -> SubjonctifPresent.PlurielP2);
    if (feuille -> u.verbe -> SubjonctifPresent.PlurielP3 != NULL)
        printf("			3eme personne du pluriel:  '%s'\n", feuille -> u.verbe -> SubjonctifPresent.PlurielP3);
}

////////////////////////////////////////////////////////////////////////////////
int compter_feuilles(NOEUD* feuille)
{
    int nombres_feuilles = 0;

    for (int i = 0; i < 256; i++)
        if (feuille -> caractere[i] !=  NULL)
            nombres_feuilles++;

    return nombres_feuilles;
}

////////////////////////////////////////////////////////////////////////////////
// Le mot va etre reconstitue progressivement en partant de la racine de l'arbre
// Il doit etre vide initialement
char* extraire_forme_base_aleatoirement(NOEUD* feuille, CATEGORIE categorie, char* mot, char* dernier_mot)
{
    int n; // nombre aleatoire

#ifdef DEBUG
printf("categorie=%d mot='%s' nombre_formes_flechies=%d\n", categorie, mot, feuille -> nombre_formes_flechies);
#endif

    // Tirage a pile ou face
    // pour savoir si on prend une forme flechie (n = 1)
    // ou si on descend dans un des fils (n = 0)
    n = random() % 2;

    if (feuille -> nombre_formes_flechies && n)
        return mot;
    else
    {
        int nombre_de_feuilles = compter_feuilles(feuille);

        if (feuille -> nombre_formes_flechies)
            dernier_mot = mot;

        if (nombre_de_feuilles == 1)
        {
            // On cherche la lettre possible
            for (int i = 0; i < 256; i++)
                if (feuille -> caractere[i] !=  NULL)
                {
                    // On ajoute le caractere du noeud ou l'on va descendre dans le mot
                    int taille_mot = strlen(mot);
                    mot[taille_mot] = (unsigned char) i;
                    mot[taille_mot + 1] = 0; 

#ifdef DEBUG
printf("On descend dans LA lettre possible='%c' mot='%s'\n", (char) i, mot);
#endif
                    return extraire_forme_base_aleatoirement(feuille -> caractere[i], categorie, mot, dernier_mot);
                }

            // Normalement on a deja trouve la lettre possible
            // Si ce n'est pas le cas on retourne la derniere forme de base
#ifdef DEBUG
printf("On est dans un cul de sac. On retourne le dernier mot='%s'\n", dernier_mot);
#endif
            return dernier_mot;
        }
        else if (nombre_de_feuilles > 1)
        {
            int i, j, n;
            int taille_mot;

            // On tire le numero du fils ou on voudrait descendre
            n = random() % nombre_de_feuilles;

            // Et on cherche ce fils
            for (i = 0, j = -1; j != n; i++)
                if (feuille -> caractere[i] != NULL)
                    j++;
            
            // On ajoute le caractere du noeud ou l'on va descendre dans le mot
            taille_mot = strlen(mot);
            mot[taille_mot] = (char) (i - 1);
            mot[taille_mot + 1] = 0; 

#ifdef DEBUG
printf("On descend dans la lettre='%c' mot='%s'\n", (char) (i - 1), mot);
#endif
            return extraire_forme_base_aleatoirement(feuille -> caractere[i - 1], categorie, mot, dernier_mot);
        }
        else // if (nombre_de_feuilles == 0)
            // Finalement on retourne quand meme la forme flechie
            return mot;
    }   

    // Normalement on a deja retourne une forme de base
    // Si ce n'est pas le cas on retourne la derniere forme de base
#ifdef DEBUG
printf("On est dans un cul de sac. On retourne le dernier mot='%s'\n", dernier_mot);
#endif
    return dernier_mot;
}

////////////////////////////////////////////////////////////////////////////////
int menu_generateur_phrase()
{
    char choix[81];

    printf("\nGenerateur automatique de phrases\n");
    printf("---------------------------------\n");
    printf("1 : Forme base modele 1\n");
    printf("2 : Forme base modele 2\n");
    printf("3 : Forme base modele 3\n");
    printf("4 : Forme flechie modele 1\n");
    printf("5 : Forme flechie modele 2\n");
    printf("6 : Forme flechie modele 3\n");
    printf("0 : Quitter\n\n");

    do
    {
        printf("Quelle action voulez vous faire ? --> ");
        scanf("%80s", choix); // %80s permet de saisir une chaîne d'au plus 80 caracteres
    }
    while (! (! strcmp(choix, "0") || ! strcmp(choix, "1") || ! strcmp(choix, "2") || ! strcmp(choix, "3") || ! strcmp(choix, "4") || ! strcmp(choix, "5") || ! strcmp(choix, "6")));
    // strcmp() renvoie 0 si les chaînes de caracteres sont identiques

    return atoi(choix);
}

////////////////////////////////////////////////////////////////////////////////
void generateur_phrase_base_modele_1()
{
    char phrase[TAILLE_PHRASE];
    char mot[TAILLE_PHRASE];
    char* forme_de_base;

    phrase[0] = 0;

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(noms, NOM, mot, NULL);
    strcat(phrase, forme_de_base);
    strcat(phrase, " ");

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(adjectifs, ADJECTIF, mot, NULL);
    strcat(phrase, forme_de_base);
    strcat(phrase, " ");

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(verbes, VERBE, mot, NULL);
    strcat(phrase, forme_de_base);
    strcat(phrase, " ");

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(noms, NOM, mot, NULL);
    strcat(phrase, forme_de_base);
    
    printf("La phrase generee du modele 1 avec les formes de base est :\n");
    printf("%s\n", phrase);
}

////////////////////////////////////////////////////////////////////////////////
void generateur_phrase_base_modele_2()
{
    char phrase[TAILLE_PHRASE];
    char mot[TAILLE_PHRASE];
    char* forme_de_base;

    phrase[0] = 0;

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(noms, NOM, mot, NULL);
    strcat(phrase, forme_de_base);
    strcat(phrase, " ");

    strcat(phrase, "qui ");

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(verbes, VERBE, mot, NULL);
    strcat(phrase, forme_de_base);
    strcat(phrase, " ");

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(verbes, VERBE, mot, NULL);
    strcat(phrase, forme_de_base);
    strcat(phrase, " ");
    
    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(noms, NOM, mot, NULL);
    strcat(phrase, forme_de_base);
    strcat(phrase, " ");

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(adjectifs, ADJECTIF, mot, NULL);
    strcat(phrase, forme_de_base);
    
    printf("La phrase generee du modele 2 avec les formes de base est :\n");
    printf("%s\n", phrase);
}

////////////////////////////////////////////////////////////////////////////////
void generateur_phrase_base_modele_3()
{
    char phrase[TAILLE_PHRASE];
    char mot[TAILLE_PHRASE];
    char* forme_de_base;
    
    phrase[0] = 0;

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(verbes, VERBE, mot, NULL);
    strcat(phrase, forme_de_base);
    strcat(phrase, " ");

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(noms, NOM, mot, NULL);
    strcat(phrase, forme_de_base);
    strcat(phrase, " ");

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(adjectifs, ADJECTIF, mot, NULL);
    strcat(phrase, forme_de_base);
    
    printf("La phrase generee du modele 3 avec les formes de base est :\n");
    printf("%s\n", phrase);
}

////////////////////////////////////////////////////////////////////////////////
void ajouter_nom(char* phrase, GENRE* pt_genre, int* pt_nombre, int article)
{
    char mot[TAILLE_PHRASE];
    char* forme_de_base;
    NOEUD* feuille;

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(noms, NOM, mot, NULL);
    feuille = rechercher_mot(forme_de_base, noms, NOM);
    *pt_genre = feuille -> u.nom -> genre;
    *pt_nombre = random() % 2;
    if (*pt_nombre)
    {
        // Singulier
        if (*pt_genre == MASCULIN || *pt_genre == INVARIABLE)
        {
            if (article == 1)
                strcat(phrase, "Le ");
            else
                strcat(phrase, "un ");

            if (feuille -> u.nom -> SingulierMasculin != NULL)
                strcat(phrase, feuille -> u.nom -> SingulierMasculin);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Nom:Mas+SG manquant)");
            }
        }
        else
        {
            if (article == 1)
                strcat(phrase, "La ");
            else
                strcat(phrase, "une ");

            if (feuille -> u.nom -> SingulierFeminin != NULL)
                strcat(phrase, feuille -> u.nom -> SingulierFeminin);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Nom:Fem+SG manquant)");
            }
        }
    }
    else
    {
        // Pluriel
        if (*pt_genre == MASCULIN || *pt_genre == INVARIABLE)
        {
            if (article == 1)
                strcat(phrase, "Les ");
            else
                strcat(phrase, "des ");

            if (feuille -> u.nom -> PlurielMasculin != NULL)
                strcat(phrase, feuille -> u.nom -> PlurielMasculin);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Nom:Mas+PL manquant)");
            }
        }   
        else 
        {
            if (article == 1)
                strcat(phrase, "Les ");
            else
                strcat(phrase, "des ");

            if (feuille -> u.nom -> PlurielFeminin != NULL)
                strcat(phrase, feuille -> u.nom -> PlurielFeminin);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Nom:Fem+PL manquant)");
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void ajouter_adjectif(char* phrase, GENRE* pt_genre, int* pt_nombre)
{
    char mot[TAILLE_PHRASE];
    char* forme_de_base;
    NOEUD* feuille;

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(adjectifs, ADJECTIF, mot, NULL);
    feuille = rechercher_mot(forme_de_base, adjectifs, ADJECTIF);
    if (*pt_nombre)
    {
        // Singulier
        if (*pt_genre == MASCULIN || *pt_genre == INVARIABLE)
        {
            if (feuille -> u.adjectif -> SingulierMasculin != NULL)
                strcat(phrase, feuille -> u.adjectif -> SingulierMasculin);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Adj:Mas+SG manquant)");
            }
        }
        else
        {
            if (feuille -> u.adjectif -> SingulierFeminin != NULL)
                strcat(phrase, feuille -> u.adjectif -> SingulierFeminin);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Adj:Fem+SG manquant)");
            }
        }
    }
    else
    {
        // Pluriel
        if (*pt_genre == MASCULIN || *pt_genre == INVARIABLE)
        {
            if (feuille -> u.adjectif -> PlurielMasculin != NULL)
                strcat(phrase, feuille -> u.adjectif -> PlurielMasculin);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Adj:Mas+PL manquant)");
            }
        }   
        else 
        {
            if (feuille -> u.adjectif -> PlurielFeminin != NULL)
                strcat(phrase, feuille -> u.adjectif -> PlurielFeminin);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Adj:Fem+PL manquant)");
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void ajouter_verbe_p3(char* phrase, int* pt_nombre, int* pt_temps)
{
    char mot[TAILLE_PHRASE];
    char* forme_de_base;
    NOEUD* feuille;

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(verbes, VERBE, mot, NULL);
    feuille = rechercher_mot(forme_de_base, verbes, VERBE);
    *pt_temps = random() % 2;
    if (*pt_temps)
    {
        // Present de l'indicatif
        if (*pt_nombre)
        {
            // Singulier
            if (feuille -> u.verbe -> IndicatifPresent.SingulierP3 != NULL)
                strcat(phrase, feuille -> u.verbe -> IndicatifPresent.SingulierP3);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Ver:IPre+SG+P3 manquant)");
            }
        }
        else
        {
            // Pluriel
            if (feuille -> u.verbe -> IndicatifPresent.PlurielP3 != NULL)
                strcat(phrase, feuille -> u.verbe -> IndicatifPresent.PlurielP3);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Ver:IPre+PL+P3 manquant)");
            }
        }
    }
    else
    {
        // Imparfait de l'indicatif
        if (*pt_nombre)
        {
            // Singulier
            if (feuille -> u.verbe -> IndicatifImparfait.SingulierP3 != NULL)
                strcat(phrase, feuille -> u.verbe -> IndicatifImparfait.SingulierP3);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Ver:IImp+SG+P3 manquant)");
            }
        }
        else
        {
            // Pluriel
            if (feuille -> u.verbe -> IndicatifImparfait.PlurielP3 != NULL)
                strcat(phrase, feuille -> u.verbe -> IndicatifImparfait.PlurielP3);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Ver:IImp+PL+P3 manquant)");
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void ajouter_verbe_p1p2(char* phrase, int* pt_nombre, int* pt_temps)
{
    char mot[TAILLE_PHRASE];
    char* forme_de_base;
    NOEUD* feuille;

    mot[0] = 0;
    forme_de_base = extraire_forme_base_aleatoirement(verbes, VERBE, mot, NULL);
    feuille = rechercher_mot(forme_de_base, verbes, VERBE);
    *pt_temps = random() % 2;
    if (*pt_temps)
    {
        // Present de l'indicatif
        if (*pt_nombre == 0)
        {
            if (feuille -> u.verbe -> IndicatifPresent.SingulierP1 != NULL)
                strcat(phrase, feuille -> u.verbe -> IndicatifPresent.SingulierP1);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Ver:IPre+SG+P1 manquant)");
            }
        }
        else if (*pt_nombre == 1)
        {
            if (feuille -> u.verbe -> IndicatifPresent.SingulierP2 != NULL)
                strcat(phrase, feuille -> u.verbe -> IndicatifPresent.SingulierP2);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Ver:IPre+SG+P2 manquant)");
            }
        }
        else if (*pt_nombre == 2)
        {
            if (feuille -> u.verbe -> IndicatifPresent.PlurielP1 != NULL)
                strcat(phrase, feuille -> u.verbe -> IndicatifPresent.PlurielP1);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Ver:IPre+PL+P1 manquant)");
            }
        }
        else if (*pt_nombre == 3)
        {
            if (feuille -> u.verbe -> IndicatifPresent.PlurielP2 != NULL)
                strcat(phrase, feuille -> u.verbe -> IndicatifPresent.PlurielP2);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Ver:IPre+PL+P2 manquant)");
            }
        }
    }
    else
    {
        // Imparfait de l'indicatif
        if (*pt_nombre == 0)
        {
            if (feuille -> u.verbe -> IndicatifImparfait.SingulierP1 != NULL)
                strcat(phrase, feuille -> u.verbe -> IndicatifImparfait.SingulierP1);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Ver:IImp+SG+P1 manquant)");
            }
        }
        else if (*pt_nombre == 1)
        {
            if (feuille -> u.verbe -> IndicatifImparfait.SingulierP2 != NULL)
                strcat(phrase, feuille -> u.verbe -> IndicatifImparfait.SingulierP2);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Ver:IImp+SG+P2 manquant)");
            }
        }
        else if (*pt_nombre == 2)
        {
            if (feuille -> u.verbe -> IndicatifImparfait.PlurielP1 != NULL)
                strcat(phrase, feuille -> u.verbe -> IndicatifImparfait.PlurielP1);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Ver:IImp+PL+P1 manquant)");
            }
        }
        else if (*pt_nombre == 3)
        {
            if (feuille -> u.verbe -> IndicatifImparfait.PlurielP2 != NULL)
                strcat(phrase, feuille -> u.verbe -> IndicatifImparfait.PlurielP2);
            else
            {
                strcat(phrase, "(");
                strcat(phrase, forme_de_base);
                strcat(phrase, " Ver:IImp+PL+P2 manquant)");
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
void generateur_phrase_flechie_modele_1()
{
    char phrase[TAILLE_PHRASE];
    GENRE genre;
    int nombre;
    int temps;

    phrase[0] = 0;

    ajouter_nom(phrase, &genre, &nombre, 1);
    strcat(phrase, " ");

    ajouter_adjectif(phrase, &genre, &nombre);
    strcat(phrase, " ");

    ajouter_verbe_p3(phrase, &nombre, &temps);
    strcat(phrase, " ");

    ajouter_nom(phrase, &genre, &nombre, 2);

    printf("La phrase generee du modele 1 avec les formes flechies est : \n");
    printf("%s\n", phrase);
}

////////////////////////////////////////////////////////////////////////////////
void generateur_phrase_flechie_modele_2()
{
    char phrase[TAILLE_PHRASE];
    GENRE genre;
    int nombre;
    int temps;

    phrase[0] = 0;

    ajouter_nom(phrase, &genre, &nombre, 1);
    strcat(phrase, " ");

    strcat(phrase, "qui ");

    ajouter_verbe_p3(phrase, &nombre, &temps);
    strcat(phrase, " ");

    ajouter_verbe_p3(phrase, &nombre, &temps);
    strcat(phrase, " ");

    ajouter_nom(phrase, &genre, &nombre, 2);
    strcat(phrase, " ");

    ajouter_adjectif(phrase, &genre, &nombre);
    
    printf("La phrase generee du modele 2 avec les formes flechies est : \n");
    printf("%s\n", phrase);
}

////////////////////////////////////////////////////////////////////////////////
void generateur_phrase_flechie_modele_3()
{
    char phrase[TAILLE_PHRASE];
    GENRE genre;
    int nombre;
    int temps;

    phrase[0] = 0;

    nombre = random() % 4;
    if (nombre == 0)
        strcat(phrase, "Je ");
    else if (nombre == 1)
        strcat(phrase, "Tu ");
    else if (nombre == 2)
        strcat(phrase, "Nous ");
    else if (nombre == 3)
        strcat(phrase, "Vous ");

    ajouter_verbe_p1p2(phrase, &nombre, &temps);
    strcat(phrase, " ");

    ajouter_nom(phrase, &genre, &nombre, 2);
    strcat(phrase, " ");

    ajouter_adjectif(phrase, &genre, &nombre);
    
    printf("La phrase generee du modele 3 avec les formes flechies est : \n");
    printf("%s\n", phrase);
}

////////////////////////////////////////////////////////////////////////////////
int rechercher_forme_flechie(char* mot_recherche, NOEUD* feuille, CATEGORIE categorie, char* mot_courant)
{
    int trouve = 0;
    char nouveau_mot_courant[TAILLE_PHRASE];
    int resultat;

#ifdef DEBUG
    printf("rechercher_forme_flechie('%s', pointeur, %d, '%s')\n", mot_recherche, categorie, mot_courant);
#endif
    if (feuille -> nombre_formes_flechies)
    {
        if (categorie == NOM)
        {
            if (feuille -> u.nom -> SingulierMasculin != NULL && ! strcmp(mot_recherche, feuille -> u.nom -> SingulierMasculin))
            {    
                printf("%s: nom %s, masculin, singulier\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.nom -> SingulierFeminin != NULL && ! strcmp(mot_recherche, feuille -> u.nom -> SingulierFeminin))
            {    
                printf("%s: nom %s, feminin, singulier\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.nom ->  PlurielMasculin != NULL && ! strcmp(mot_recherche, feuille -> u.nom -> PlurielMasculin))
            {    
                printf("%s: nom %s, masculin, pluriel\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.nom -> PlurielFeminin != NULL && ! strcmp(mot_recherche, feuille -> u.nom -> PlurielFeminin))
            {    
                printf("%s: nom %s, feminin, pluriel\n", mot_recherche, mot_courant);
                trouve = 1;
            }
        }
        else if (categorie == ADJECTIF)
        {
            if (feuille -> u.adjectif -> SingulierMasculin != NULL && ! strcmp(mot_recherche, feuille -> u.adjectif -> SingulierMasculin))
            {    
                printf("%s: adjectif %s, masculin, singulier\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            else if (feuille -> u.adjectif -> SingulierFeminin != NULL && ! strcmp(mot_recherche, feuille -> u.adjectif -> SingulierFeminin))
            {    
                printf("%s: adjectif %s, feminin, singulier\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            else if (feuille -> u.adjectif -> PlurielMasculin != NULL && ! strcmp(mot_recherche, feuille -> u.adjectif -> PlurielMasculin))
            {    
                printf("%s: adjectif %s, masculin, pluriel\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            else if (feuille -> u.adjectif -> PlurielFeminin != NULL && ! strcmp(mot_recherche, feuille -> u.adjectif -> PlurielFeminin))
            {    
                printf("%s: adjectif %s, feminin, pluriel\n", mot_recherche, mot_courant);
                trouve = 1;
            }
        }
        else if (categorie == VERBE)
        {
            if (feuille -> u.verbe -> InfinitifPresent != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> InfinitifPresent)) 
            {    
                printf("%s: verbe %s, present de l'infinitif\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> IndicatifPresent.SingulierP1 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> IndicatifPresent.SingulierP1)) 
            {    
                printf("%s: verbe %s, present de l'indicatif, 1ere personne du singulier\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> IndicatifPresent.SingulierP2 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> IndicatifPresent.SingulierP2)) 
            {    
                printf("%s: verbe %s, present de l'indicatif, 2eme personne du singulier\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> IndicatifPresent.SingulierP3 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> IndicatifPresent.SingulierP3)) 
            {    
                printf("%s: verbe %s, present de l'indicatif, 3eme personne du singulier\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> IndicatifPresent.PlurielP1 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> IndicatifPresent.PlurielP1)) 
            {    
                printf("%s: verbe %s, present de l'indicatif, 1ere personne du pluriel\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> IndicatifPresent.PlurielP2 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> IndicatifPresent.PlurielP2)) 
            {    
               printf("%s: verbe %s, present de l'indicatif, 2eme personne du pluriel\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> IndicatifPresent.PlurielP3 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> IndicatifPresent.PlurielP3)) 
            {    
                printf("%s: verbe %s, present de l'indicatif, 3eme personne du pluriel\n", mot_recherche, mot_courant);
                trouve = 1;
            }
    
            if (feuille -> u.verbe -> IndicatifImparfait.SingulierP1 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> IndicatifImparfait.SingulierP1)) 
            {    
                printf("%s: verbe %s, imparfait de l'indicatif, 1ere personne du singulier\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> IndicatifImparfait.SingulierP2 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> IndicatifImparfait.SingulierP2)) 
            {    
                printf("%s: verbe %s, imparfait de l'indicatif, 2eme personne du singulier\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> IndicatifImparfait.SingulierP3 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> IndicatifImparfait.SingulierP3)) 
            {    
                printf("%s: verbe %s, imparfait de l'indicatif, 3eme personne du singulier\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> IndicatifImparfait.PlurielP1 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> IndicatifImparfait.PlurielP1)) 
            {    
                printf("%s: verbe %s, imparfait de l'indicatif, 1ere personne du pluriel\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> IndicatifImparfait.PlurielP2 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> IndicatifImparfait.PlurielP2)) 
            {    
                printf("%s: verbe %s, imparfait de l'indicatif, 2eme personne du pluriel\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> IndicatifImparfait.PlurielP3 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> IndicatifImparfait.PlurielP3)) 
            {    
                printf("%s: verbe %s, imparfait de l'indicatif, 3eme personne du pluriel\n", mot_recherche, mot_courant);
                trouve = 1;
            }
    
            if (feuille -> u.verbe -> SubjonctifPresent.SingulierP1 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> SubjonctifPresent.SingulierP1)) 
            {    
                printf("%s: verbe %s, present de l'indicatif, 1ere personne du singulier\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> SubjonctifPresent.SingulierP2 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> SubjonctifPresent.SingulierP2)) 
            {    
                printf("%s: verbe %s, present de l'indicatif, 2eme personne du singulier\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> SubjonctifPresent.SingulierP3 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> SubjonctifPresent.SingulierP3)) 
            {    
                printf("%s: verbe %s, present de l'indicatif, 3eme personne du singulier\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> SubjonctifPresent.PlurielP1 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> SubjonctifPresent.PlurielP1)) 
            {    
                printf("%s: verbe %s, present de l'indicatif, 1ere personne du pluriel\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> SubjonctifPresent.PlurielP2 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> SubjonctifPresent.PlurielP2)) 
            {    
                printf("%s: verbe %s, present de l'indicatif, 2eme personne du pluriel\n", mot_recherche, mot_courant);
                trouve = 1;
            }
            if (feuille -> u.verbe -> SubjonctifPresent.PlurielP3 != NULL && ! strcmp(mot_recherche, feuille -> u.verbe -> SubjonctifPresent.PlurielP3)) 
            {    
                printf("%s: verbe %s, present de l'indicatif, 3eme personne du pluriel\n", mot_recherche, mot_courant);
                trouve = 1;
            }
        }
        else if (categorie == ADVERBE)
        {
            if (! strcmp(mot_recherche, mot_courant))
            {
                printf("%s: adverbe\n", mot_recherche);
                trouve = 1;
            }
        }
    }
    
    if (trouve)
    {
        return trouve;
    }
    else if (feuille -> caractere[(unsigned char) mot_recherche[strlen(mot_courant)]] != NULL)
    {
        strcpy(nouveau_mot_courant, mot_courant);
        nouveau_mot_courant[strlen(mot_courant)] = mot_recherche[strlen(mot_courant)];
        nouveau_mot_courant[strlen(mot_courant) + 1] = 0;

        // On descend d'abord au plus pres de la forme flechie recherchee
        resultat = rechercher_forme_flechie(mot_recherche, feuille -> caractere[(unsigned char) mot_recherche[strlen(mot_courant)]], categorie, nouveau_mot_courant);

        if (resultat)
            return 1;
        else    
        {
            // On n'a pas trouve de forme de base dont les formes flechies correspondent a celle recherchee
            for (int i = 0; i < 256 ; i++)
            {
                // On teste les autres fils au niveau actuel
                if (mot_recherche[strlen(mot_courant)] != (char) i)
                {
                    if (feuille -> caractere[i] != NULL)
                    {
#ifdef DEBUG
printf("On teste la lettre '%c'\n", (char) i);
#endif
                        strcpy(nouveau_mot_courant, mot_courant);
                        nouveau_mot_courant[strlen(mot_courant)] = (char) i;
                        nouveau_mot_courant[strlen(mot_courant) + 1] = 0;

                        resultat = rechercher_forme_flechie(mot_recherche, feuille -> caractere[i], categorie, nouveau_mot_courant);
                        if (resultat)
                            return 1;
                    }
                }
            }
            return 0;
        }
    }
    else
    {
        // On n'a pas trouve de forme de base dont les formes flechies correspondent a celle recherchee
        for (int i = 0; i < 256 ; i++)
        {
            // On teste les autres fils au niveau actuel
            if (mot_recherche[strlen(mot_courant)] != (char) i)
            {
                if (feuille -> caractere[i] != NULL)
                {
#ifdef DEBUG
printf("On teste la lettre '%c'\n", (char) i);
#endif
                    strcpy(nouveau_mot_courant, mot_courant);
                    nouveau_mot_courant[strlen(mot_courant)] = (char) i;
                    nouveau_mot_courant[strlen(mot_courant) + 1] = 0;

                    resultat = rechercher_forme_flechie(mot_recherche, feuille -> caractere[i], categorie, nouveau_mot_courant);
                    if (resultat)
                        return 1;
                }
            }
        }
        return 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
void afficher_arbre(NOEUD* feuille, CATEGORIE categorie, char* forme_de_base, int* pt_nb_formes_de_base, int* pt_nb_formes_flechies, int* pt_nb_feuilles)
{
    int nombre_feuilles = compter_feuilles(feuille);

    *pt_nb_feuilles += 1;

    if (feuille -> nombre_formes_flechies)
    {
        *pt_nb_formes_de_base += 1;
        *pt_nb_formes_flechies += feuille -> nombre_formes_flechies;

        printf(
            "'%s': %d forme%s fléchie%s, %d feuille%s\n",
            forme_de_base,
            feuille -> nombre_formes_flechies,
            (feuille -> nombre_formes_flechies == 1)?"":"s",
            (feuille -> nombre_formes_flechies == 1)?"":"s",
            nombre_feuilles,
            (nombre_feuilles == 1)?"":"s"
        );
    }

    for (int i = 0; i < 256; i++)
        if (feuille -> caractere[i] != NULL)
        {
            char nouvelle_forme_de_base[TAILLE_PHRASE];
            int longueur = strlen(forme_de_base);

            strcpy(nouvelle_forme_de_base, forme_de_base);
            nouvelle_forme_de_base[longueur] = (unsigned char) i;
            nouvelle_forme_de_base[longueur + 1] = 0;

            afficher_arbre(feuille -> caractere[i], categorie, nouvelle_forme_de_base, pt_nb_formes_de_base, pt_nb_formes_flechies, pt_nb_feuilles);
        }
}

