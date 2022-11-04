// Structure de mots


typedef struct s_letter_node
{
    char letter;
    struct s_letter_node * sons [ALPHABET_SIZE] ;
} t_letter_node, *p_letter_node ;


typedef struct mots {
    int values;
    struct mots* next;
} mots;


  typedef struct NomFleche {
      singulier nom;
  };



typedef struct Adjectif{

};


typedef struct Adverbe {

        };


typedef struct Verbe {

        };


//arbre


typedef struct s_tree
{
    p_letter_node root ;

} t_tree, *p_tree;


t_tree Verbe_tree;

t_tree Nom_tree;

t_tree Adjectif_tree;

t_tree Adverbe_tree;
