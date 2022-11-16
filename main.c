/*******************************************************************************
* Projet Generateur automatique de phrases                                     *
* Description du fichier : Fichier main                                        *
*                                                                              *
* Auteurs : Thomas TOURNIER, Mehdy MICHALAK, Adem DEBAHI                       *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fonctions.h"
# define random rand

// Les arbres pour chaque categorie grammaticale du dictionnaire
NOEUD* verbes = NULL;
NOEUD* noms = NULL;
NOEUD* adjectifs = NULL;
NOEUD* adverbes = NULL;

int main()
{
    int menu;
    
    // On cree la racine des arbres du dictionnaire
    verbes = creer_noeud();
    noms = creer_noeud();
    adjectifs = creer_noeud();
    adverbes = creer_noeud();

    charger_dictionnaire();
    initialiser_nombres_aleatoires();

    do
    {
        menu = menu_principal();

        if (menu == 1) // Rechercher un mot parmi les formes de base
        {
            char mot[TAILLE_PHRASE];
            CATEGORIE categorie;
            NOEUD* feuille;

            printf("Quelle forme de base voulez-vous rechercher ? --> ");
            scanf("%1024s", mot);

            categorie = menu_categorie();
            printf("\n");
            if (categorie == NOM)
            {
                feuille = rechercher_mot(mot, noms, categorie);
                if (feuille != NULL)
                {
                    printf("La forme de base '%s' a ete trouvee. Voici ses formes flechies :\n", mot);
                    afficher_forme_flechie_nom(feuille); 
                }
            }
            else if (categorie == ADJECTIF)
            {
                feuille = rechercher_mot(mot, adjectifs, categorie);
                if (feuille != NULL)
                {
                    printf("La forme de base '%s' a ete trouvee. Voici ses formes flechies :\n", mot);
                    afficher_forme_flechie_adjectif(feuille); 
                }
            }
            else if (categorie == ADVERBE)
            {
                feuille = rechercher_mot(mot, adverbes, categorie);
                if (feuille != NULL)
                {
                    printf("La forme de base '%s' a ete trouvee. Voici ses formes flechies :\n", mot);
                    afficher_forme_flechie_adverbe(mot); 
                }
            }
            else if (categorie == VERBE)
            {
                feuille = rechercher_mot(mot, verbes, categorie);
                if (feuille != NULL)
                {
                    printf("La forme de base '%s' a ete trouvee. Voici ses formes flechies :\n", mot);
                    afficher_forme_flechie_verbe(feuille, mot); 
                }
            }

            if (feuille == NULL)
                printf("La forme de base '%s' n'a PAS ete trouvee.\n", mot);
        }

        else if (menu == 2) // Extraire une forme de base au hasard
        {
            CATEGORIE categorie;
            char mot[TAILLE_PHRASE];
            char* forme_de_base;

            // Les categories vont de 1 a 4. On en tire une au hasard
            categorie = (random() % 4) + 1;

            mot[0] = 0;
            if (categorie == NOM)
                forme_de_base = extraire_forme_base_aleatoirement(noms, categorie, mot, NULL);
            else if (categorie == ADJECTIF)
                forme_de_base = extraire_forme_base_aleatoirement(adjectifs, categorie, mot, NULL);
            else if (categorie == ADVERBE)
                forme_de_base = extraire_forme_base_aleatoirement(adverbes, categorie, mot, NULL);
            else if (categorie == VERBE)
                forme_de_base = extraire_forme_base_aleatoirement(verbes, categorie, mot, NULL);
    
            printf("\nForme de base extraite : '%s'\n", forme_de_base);
        }

        else if (menu == 3) // Generer des phrases au hasard selon plusieurs modeles
        {
            int menu1 = menu_generateur_phrase();

            printf("\n");
            if (menu1 == 1)
                generateur_phrase_base_modele_1();
            else if (menu1 == 2)
                generateur_phrase_base_modele_2();
            else if (menu1 == 3)
                generateur_phrase_base_modele_3();
            else if (menu1 == 4)
                generateur_phrase_flechie_modele_1();
            else if (menu1 == 5)
                generateur_phrase_flechie_modele_2();
            else if (menu1 == 6)
                generateur_phrase_flechie_modele_3();
            else if (menu1 == 0)
                return 0;
        }

        else if (menu == 4)
        {
            char mot[TAILLE_PHRASE];
            char mot_courant[TAILLE_PHRASE];
            CATEGORIE categorie;
            int resultat = 9;

            printf("Quelle forme flechie voulez-vous rechercher ? --> ");
            scanf("%1024s", mot);
            mot_courant[0] = 0;

            categorie = menu_categorie();
            printf("\n");
            if (categorie == NOM)
                resultat = rechercher_forme_flechie(mot, noms, categorie, mot_courant);
            else if (categorie == ADJECTIF)
                resultat = rechercher_forme_flechie(mot, adjectifs, categorie, mot_courant);
            else if (categorie == ADVERBE)
                resultat = rechercher_forme_flechie(mot, adverbes, categorie, mot_courant);
            else if (categorie == VERBE)
                resultat = rechercher_forme_flechie(mot, verbes, categorie, mot_courant);

            if (! resultat)
                printf("Forme flechie '%s' NON trouvee !\n", mot);
        }

#ifdef DEBUG
        else if (menu >= 5 && menu <= 8)
        {
            char forme_de_base[TAILLE_PHRASE];
            int nb_formes_de_base = 0;
            int nb_formes_flechies = 0;
            int nb_feuilles = 0;

            printf("\n");

            forme_de_base[0] = 0;
            if (menu == 5)
                afficher_arbre(noms, NOM, forme_de_base, &nb_formes_de_base, &nb_formes_flechies, &nb_feuilles);
            else if (menu == 6)
                afficher_arbre(adjectifs, ADJECTIF, forme_de_base, &nb_formes_de_base, &nb_formes_flechies, &nb_feuilles);
            else if (menu == 7)
                afficher_arbre(adverbes, ADVERBE, forme_de_base, &nb_formes_de_base, &nb_formes_flechies, &nb_feuilles);
            else if (menu == 8)
                afficher_arbre(verbes, VERBE, forme_de_base, &nb_formes_de_base, &nb_formes_flechies, &nb_feuilles);

            printf("===== TOTAL =====\n");
            printf("%d formes de base\n", nb_formes_de_base);
            printf("%d formes flechies\n", nb_formes_flechies);
            printf("%d feuilles\n", nb_feuilles);
        }
#endif
    } while (menu);

    return 0;
}

