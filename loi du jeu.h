#include <stdio.h>
#include <stdlib.h>

#define J1 '1'// Joueur 1
#define J2 '2'// Joueur 2
#define P1 1   // Pion 1
#define P2 2   // Pion 2
#define G 'q' //Mouvement � gauche
#define D 'd' // Mouvement � droit
#define H 'z' //Mouvement vers l'haut
#define BB 's' //Mouvement vers le bas
#define N 'n' //Aucun mouevement



typedef struct{
    int T[3][8];
    char joueur;
    int NU;//  Le nombre des pions que le joueur J2 avait utilis�.
    int NJ1;// Le nombre des pions restants � J1.
    int NJ2;// Le nombre des pions restants � J2.
    int possible;//Cette valeur indique si le joueur humain peut faire un certain mouvement(elle va changer de 0 � -1).
    int manger;  //Indique si le joueur a le droit de manger.
    int dern_mvt[2];//Le dernier couple (i, j) qu'on a jou�.
    int position_mangee[2];//Le dernier couple (i, j) qu'on a mang� dans le cas de l'int�lligence aftificielle.
    int PasSortir;     /*quand NU = 9, si on va manger quelque chose, il va se produire un probl�me qu'on est d�ja dans la phase 2, donc
                on utilise ce nombre pour pouvoir entrer dans la boucle GenereSuccesseur if(Phase(et) == 1) pour une seule fois.*/
    int h;//La faleur de h(n) qu'on va utiliser pour l'intelligence artificielle.
}etat;

typedef struct Noeud{
    struct Noeud *suivant;
    struct Noeud *successeur;
    etat Et;
}noeud;

typedef struct Liste{
    noeud *debut, *fin;
}liste;



int m, n, i, j, ii, jj, k;
char J, dep;
const char Tdep[4] = {G, D, H, BB};//Tableau contenant les 4 d&placements possibles
etat et1;
liste chemin;

void Inserer(etat et,liste *listeNoeuds){
    noeud *Noeud=NULL;
    Noeud = malloc(sizeof(noeud));
    Noeud->Et = et;
    if(listeNoeuds->fin==NULL){
        listeNoeuds->fin=Noeud;
        listeNoeuds->debut=Noeud;
        Noeud->suivant=NULL;
    }
    else if(listeNoeuds->debut==listeNoeuds->fin){
        listeNoeuds->debut=Noeud;
        listeNoeuds->debut->suivant=listeNoeuds->fin;
    }
    else{
        Noeud->suivant=listeNoeuds->debut;
        listeNoeuds->debut=Noeud;
    }
}

etat Extraire(liste *listeNoeuds){
    noeud *Noeud=listeNoeuds->debut;
    if(listeNoeuds->debut!=listeNoeuds->fin){
        listeNoeuds->debut=listeNoeuds->debut->suivant;
        free(Noeud);
        return listeNoeuds->debut->Et;
    }
}

int etatSolution(etat Et,etat etatFinal){
    for(i = 0; i<3; i++){
        for(j = 0; j<3; j++){
            if(Et.T[i][j] != etatFinal.T[i][j])
                return 0;
        }
    }
    return 1;
}

int Pion(etat et){
    if(et.joueur == J1)
        return P1;
    else if(et.joueur == J2)
        return P2;
}

char joueur_inverse(etat et){
    if(et.joueur == J1)
        return J2;
    else if(et.joueur == J2)
        return J1;
}

int Phase(etat et){//Phase 1(on pose les 9 pions), phase 2(On d�place les pions), phase 3(Il reste 3 pions � J et pas � l'autre) ou phase 4(Il ne reste que 2 pions � l'un des deux).
    if(et.NU != 9)// NU peut prendre que 9 ou une valeur inf�rieur, et si on est arriv� � l'�tat 2, elle stabilise � 9 pour le reste.
        return 1;
    else if(et.joueur == J1){
        if(et.NJ1 == 3)
            return 3;
        else
            return 2;
    }
    else{
        if(et.NJ2 == 3)
            return 3;
        return 2;
    }
}


//Ligne verticale c�d quand on varie les i, et horizontale quand on varie les j.
int aligne(etat et, int i, int j){//Elle cherche si le pion de la position (i, j) forme une ligne avec une ligne(il faut qu'il ait le m�me type de pions!!).
    //On va changer les valeurs de i et j durant le programme, et fixer leurs valeurs initiales par ii et jj que l'on ne va pas changer.
    int jj;
    jj = j;
    if(j%2 == 0){// Le cas o� une ligne verticale v�rifie ce qu'on veut.
        if(j%2 == 0 && et.T[i][j] == Pion(et) && et.T[0][j] == et.T[1][j] && et.T[1][j] == et.T[2][j])// C'est comme si on donne � i la valeur 0.
            return 1;
    }
    if(j%2 == 0){
        j += 7;//Il faut rendre j dans la t�te de la ligne. + 7 equivalent � - 1.
        j = j%8;
    }
    if(et.T[i][j%8] == Pion(et) && et.T[i][j%8] == et.T[i][(j + 1)%8] && et.T[i][(j + 1)%8] == et.T[i][(j + 2)%8])
        return 1;//Le cas de la ligne horizontale qui suit la t�te
    j = jj;
    if(j%2 != 0){//Le cas de la ligne horizontale qui pr�c�de la t�te si jj == la t�te.
        j += 6;// Au lien de faire -= 2.
        if(et.T[i][j%8] == Pion(et) && et.T[i][j%8] == et.T[i][(j + 1)%8] && et.T[i][(j + 1)%8] == et.T[i][(j + 2)%8])
            return 1;
    }
    return 0;
}

etat transition(etat et, liste *L, int iiii, int jjjj){//Apr�s qu'on place ou on fait un mouvement � un pion, qu'est ce qui va se passer(Quelqu'un va �tre mang� et si c'est le cas, qui ??. Il faut qu'on donne � iiii la valeur -1 quand on veut extraire des successeurs de fa�on g�n�rale.
    int i, j, ii, jj, m = 0, n = 0;
    etat et1;
    for(i = 0; i<3; i++){
        j = 7;
        while(j == 7 || j%8 != 7){//j%8 != 7 c�d que que j arrive � 15, on va tomber dans la valeur 7, et donc on va traiter 7 2 fois.
            m = (et.dern_mvt[0] == i && (et.dern_mvt[1] == j%8 || et.dern_mvt[1] == (j + 1)%8 || et.dern_mvt[1] == (j + 2)%8));
            if(m == 1 && et.T[i][j%8] != 0 && et.T[i][j%8] != Pion(et) && et.T[i][j%8] == et.T[i][(j + 1)%8] && et.T[i][(j + 1)%8] == et.T[i][(j + 2)%8]){//Jusqu'ici, les conditions traitent le cas o� on est tomb� dans le cas o� le joueur fait une ligne.
                for(ii = 0; ii<3; ii++){
                    for(jj = 0; jj<8; jj++){//(i, j) d�signe la ligne du pion qui nous a amen� � prendre un pion, et (ii, jj) est le pion qu'on veut manger.
                        if((iiii == -1 || (ii == iiii && jj == jjjj)) && et.T[ii][jj] == Pion(et)){
                            if(!aligne(et, ii, jj)){
                                et1 = et;
                                et1.T[ii][jj] = 0;
                                if(et.possible == 0){//La premi�re fois qu'on entre en ne voulant pas remplir la liste.
                                    et.manger = 1;
                                    et1.manger = 1;
                                    if(et.PasSortir == 0 && et.NU == 9)//Le cas o� on fait le dernier mvt dans la phase 1, si on va manger quelque chose, qui est ce cas, alors il faut entrer dans la boucle if(Phase(et) == 1) m�me si on n'est plus dans cette �tat, donc on joue par PasSortir.
                                        et.PasSortir = 1;//PasSortit = 0 signifie qu'on est dans la phase 1, 1 signifie qu'on est dans le cas du commentaire qui est au dessus et 2 signifie qu'on est dans l'�tat 2.
                                    if(L == NULL)
                                        return et;
                                }
                                if(L != NULL){//Si on veut remplir la liste.
                                    et1.position_mangee[0] = ii, et1.position_mangee[1] = jj;
                                    Inserer(et1, L);
                                }
                                if(iiii != -1){// le seul cas o� iiii est diff�rent de -1, c'est quand on veut manger
                                    if(et1.joueur == J1)
                                        et1.NJ1 -= 1;
                                    else
                                        et1.NJ2 -= 1;
                                    et1.possible = 2;
                                    return et1;
                                }
                                n = 1;
                            }
                        }
                    }
                }
            }
            j += 2;
        }
    }
    for(j = 0; j<8; j += 2){
        if(et.dern_mvt[1] == j && et.T[0][j] != 0 && et.T[0][j] != Pion(et) && et.T[0][j] == et.T[1][j] && et.T[1][j] == et.T[2][j]){
            for(ii = 0; ii<3; ii++){
                for(jj = 0; jj<8; jj++){
                    if((iiii == -1 || (ii == iiii && jj == jjjj)) && et.T[ii][jj] == Pion(et)){
                        if(!aligne(et, ii, jj)){
                            et1 = et;
                            et1.T[ii][jj] = 0;
                            if(et.possible == 0){
                                et.manger = 1;
                                et1.manger = 1;
                                if(et.PasSortir == 0 && et.NU == 9)
                                    et.PasSortir = 1;
                                if(L == NULL)
                                    return et;
                            }
                            if(L != NULL){
                                et1.position_mangee[0] = ii, et1.position_mangee[1] = jj;
                                Inserer(et1, L);
                            }
                            if(iiii != -1){
                                if(et1.joueur == J1)
                                    et1.NJ1 -= 1;
                                else
                                    et1.NJ2 -= 1;
                                et1.possible = 2;
                                return et1;
                            }
                            n = 1;
                        }
                    }
                }
            }
        }
    }
    if(n == 0){
        if(iiii != -1)//Le cas o� on doit manger, mais on donne une mauvaise position � manger. Et dans ce cas, et.possible = 1 et donc elle n'a pas encore obtenu le nombre 2.
            return et;
        if(L == NULL && et.possible == 0)
            return et;
        Inserer(et, L);
    }
}

etat GenererSuccesseurs(etat et, liste *L, int iii, int jjj, int iiii, int jjjj, int iiiii, int jjjjj, char dep){// On donne � i la valeur -1 dans le cas o� on n'est pas int�ress� par ce que veut le joueur humain faire mouvoir.
    //L'utilisation des valeurs iiiii et jjjjj : Dans la phase 3 on a (iii, jjj) qui repr�sente le pion qu'on veut mouvoir, (iiiii, jjjjj) la position de destination, et (iiii, jjjj) la position qu'on veut manger si c'est le cas. Quand on n'est pas dans la phase 3, on donne � iiiii et jjjjj la valeur -1 par d�faut.
    int Pas = 0;//Il va nous aider dans le cas du remplissage de la liste, on va utiliser cette valeur pour ne pas tomber dans le cycle de goto ici!!!
    if(L != NULL){
        for(i = 0; i<3; i++){
            for(j = 0; j<8; j++){
                if(i == iii && j == jjj){
                traiter_seulement:
                    i = iii, j = jjj;
                }
                et1 = et;
                if(Phase(et) == 1 || et.PasSortir == 1 || et.possible == 0){
                    if(et.possible == 0){
                        et.possible = 1;
                        et1 = et;
                    }
                    if(Phase(et) == 1 && et.T[i][j] == 0 && et.possible == -1){
                        et1 = et;
                        et1.possible = 0;
                        et1.T[i][j] = Pion(et);
                        if(et.joueur == J1)
                            et1.NJ1 += 1;
                        else{
                            et1.NJ2 += 1;
                            et1.NU += 1;
                        }
                        et1.joueur = joueur_inverse(et);//Si on peut manger quelque chose, il ne faut pas encore inverser le joueur, sinon on va rencontrer des probl�mes. On inverse toujours le joueur quand et.possible = -1, c�d quand un �tat entre dans cet endroit pour la premi�re fois.

                        if(L != NULL){
                            et1.dern_mvt[0] = i;
                            et1.dern_mvt[1] = j;
                            transition(et1, L, -1, -1);
                        }
                    }
                    if(iii != -1 && et1.possible == 0)
                        return transition(et1, NULL, -1, -1);
                    if(iii != -1 && et.possible == 1){//On ne peut pas faire directement == 0 et changer la valeur en 1 ici, parceque si on a la valeur -1 au d�but, elle va changer en 0, et puisqu'on pose ici la condition 0, alors elle va v�rifier la condition, c'est pourquoi on a travaill� comme �a.
                        if(et.PasSortir == 1)
                            et.PasSortir = 2;
                        return transition(et, NULL, iiii, jjjj);
                    }
                }
                else if(Phase(et) == 2){
                    et.PasSortir = 2;
                    if(et.T[i][j] == Pion(et)){
                        k = (et.T[i][(j + 1)%8] == 0 && (L != NULL || dep == D));
                        if(k == 1){
                            et1 = et;//On revient aux valeurs initiales de et, mais pour les autres cas(k != 1), il ne faut pas revenir par et1 car on a d�ja chang� pas mal de choses.
                            Pas = 0;
                        ici:
                            if(et1.possible == -1){//Sa premi�re fois( il va faire seulement un mouvement)
                                et1.possible = 0;
                                et1.T[i][j] = 0;
                                if(k == 1){
                                    et1.T[i][(j + 1)%8] = et.T[i][j];
                                    et1.dern_mvt[0] = i;
                                    et1.dern_mvt[1] = (j + 1)%8;
                                }
                                et1.joueur = joueur_inverse(et);//On renverse le joueur juste avant qu'on essaye de manger l'un des pions de l'adversaire, sinon Phase(et) ne va pas �tre calcul�e correctement.
                            }
                            if(L != NULL)
                                transition(et1, L, -1, -1);
                            else if(et1.possible == 0)
                                return transition(et1, NULL, -1, -1);
                            k = 0;
                            if(Pas == 1)
                                goto pas1;
                            else if(Pas == 2)
                                goto pas2;
                            else if(Pas == 3)
                                goto pas3;
                        }
                        if((L != NULL || dep == G) && et1.T[i][(j + 7)%8] == 0){//Au lien de faire -1 on fait +7.
                            et1 = et;
                            if(et1.possible == -1){
                                et1.T[i][(j + 7)%8] = et.T[i][j];//On fait cette op�ration seulement dans la premi�re fois.
                            et1.dern_mvt[0] = i;
                            et1.dern_mvt[1] = (j + 7)%8;
                            }
                            if(L != NULL)
                                Pas = 1;
                            goto ici;
                        }
                    pas1:
                        if(j%2 == 0){
                            if((L != NULL || dep == BB) && i <= 1 && et1.T[i + 1][j] == 0){
                                et1 = et;
                                if(et1.possible == -1){
                                    et1.T[i + 1][j] = et.T[i][j];
                                    et1.dern_mvt[0] = i + 1;
                                    et1.dern_mvt[1] = j;
                                }
                                if(L != NULL)
                                    Pas = 2;
                                goto ici;
                            }
                        pas2:
                            if((L != NULL || dep == H) && i >= 1 && et1.T[i - 1][j] == 0){
                                et1 = et;
                                if(et1.possible == -1){
                                    et1.T[i - 1][j] = et.T[i][j];
                                    et1.dern_mvt[0] = i - 1;
                                    et1.dern_mvt[1] = j;
                                }
                                if(L != NULL)
                                    Pas = 3;
                                goto ici;
                            }// et.possible =
                        }//                   0 signifie que le pion n'a pas encore fiat son mouvement.
                    }//                       1 signifie qu'il a d�ja fait le mouvement et il vient encore pour manger un pion, mais il n'avait pas de chance.
                }//                           2 signifie qu'il a d�ja fait le mouvement et il vient encore pour manger un pion, et il a eu de la chance.
                else if(Phase(et) == 3 && et.T[i][j] == Pion(et)){//On n'a pas besoin de pr�ciser le pion.
                    if(et.T[i][j] == Pion(et)){
                        for(ii = 0; ii<3; ii++){
                            for(jj = 0; jj<8; jj++){
                                if((L != NULL || (i == iii && j == jjj && ii == iiiii && jj == jjjjj)) && et.T[ii][jj] == 0){
                                    et1 = et;
                                    if(et1.possible == -1){
                                        et1.possible = 0;
                                        et1.T[ii][jj] = et.T[i][j];
                                        et1.T[i][j] = 0;
                                        et1.joueur = joueur_inverse(et);
                                    }
                                    if(L != NULL){
                                        et1.dern_mvt[0] = ii;
                                        et1.dern_mvt[1] = jj;
                                        transition(et1, L, -1, -1);
                                    }
                                    if(L == NULL && et1.possible == 0){
                                        return transition(et1, NULL, -1, -1);
                                    }
                                }
                            }
                        }
                    }
                }
            pas3:
                if(iii != -1)
                    return et1;// Dans ce cas o� le joueur humain veut avoir ce changement sur son �tat, il va sortir de la fonction, (et pas continuer en augmentant les valeurs de i et j et faire des calcules ind�sirables) gra�e � "return".
            }
        }
    }
    else
        goto traiter_seulement;
}

etat placer_un_pion(etat et, int i, int j, int ii, int jj, char dep){
    et.possible = -1;//Le cas par d�faut, on le donne ici.
    int n;
    n = (i >= 0 && i<3 && j >= 0 && j < 8);
        et = GenererSuccesseurs(et, NULL, i, j, -1, -1, ii, jj, dep);
    if(n == 1){
        if(et.possible != -1)
            return et;
    }//Il ne faut pas faire une printf() ici, parcequ'on utilise cette fonction dans la fonction tournoa_termine() et donc ce qui est dans printf() va �tre affich�.
    et.possible = -2;
    return et;
}

etat choisir_apres_le_deplacement_du_pion(etat et, int i, int j, int ii, int jj, char dep){
    etat et1 = et;
    int n;
    n = (ii >= 0 && ii<3 && jj >= 0 && jj < 8);
    if(n == 1){
        et1 = GenererSuccesseurs(et1, NULL, i, j, ii, jj, -1, -1, dep);
        if(et1.possible == 2){//On a mang� l'un des pions de l'adversaire.
            return et1;
        }
    }
    return et;
}

int tournoa_termine(etat et){
    etat et1;
    if(Phase(et) == 1 || Phase(et) == 3)//Il peut �tre que le nombre de pions du joueur est 2, mais on n'a pas encore termin� de la phase 1, c'est pourquoi qu'on l'a isol� d�s le d�but !!. Si la phase == 3, on peut avoir un cas de pions qui ne peuvent pas d�placer ordinairement, mais en r�ali� ils peuvent se d�placer  gra�e � leur privil�ge, donc il faut les traiter seuls, sinon on va tomber dans des erreurs si on les traite avec le cas o� phase = 2.
        return 0;
    else{// Maintenant, c'est le tour de l'un des deux joururs, et on v�rifie si le tournoa se termine � son tour, et si elle se termine � son tour alors automatiquement il est le perant!!
        if(et.NJ1 == 2 || et.NJ2 == 2)
            return 1;// 1 signifie que le tournoa se termine, et 0 signifie l'inverse.
        else{
            for(i = 0; i<3; i++){
                for(j = 0; j<8; j++){
                    if((et.T[i][j] == P1 && et.joueur == J1) || (et.T[i][j] == P2 && et.joueur == J2)){
                        for(n = 0; n<4; n++){
                            et1 = placer_un_pion(et, i, j, -1, -1, Tdep[n]);
                            if(et1.possible != -2){//Si on trouve seulement un d�placement possible(!= -1), alors le tournoa n'est pas encore termin�e.
                                return 0;
                            }
                        }
                    }
                }
            }
            return 1;//Si on n'a trouv� aucun d�placement possible.
        }
    }
}

void h(liste *L){
    int i, j;
    noeud *Noeud;
    etat et;
    for(Noeud = L->debut; Noeud != NULL; Noeud = Noeud->suivant){//On ajoute 3 � h quand il y'a 2 pion et seulement 2 pions dans une ligne.
        et = Noeud->Et;
        Noeud->Et.h = 0;
        if(et.manger == 0){
            i = et.dern_mvt[0], j = et.dern_mvt[1];
            j += 8;
            if(j%2 == 1){
                if(et.T[i%3][j%8] == et.T[i%3][(j + 1)%8] && et.T[i%3][(j + 2)%8] == 0)//[j = 1, 1, 0]
                    Noeud->Et.h += 3;
                if(et.T[i%3][j%8] == et.T[i%3][(j + 2)%8] && et.T[i%3][(j + 1)%8] == 0)//[j = 1, 0, 1]
                    Noeud->Et.h += 3;
                j -= 2;
                if(et.T[i%3][(j + 1)%8] == et.T[i%3][(j + 2)%8] && et.T[i%3][j%8] == 0)//[0, 1, j = 1]
                    Noeud->Et.h += 3;
                if(et.T[i%3][j%8] == et.T[i%3][(j + 2)%8] && et.T[i%3][(j + 1)%8] == 0)//[1, 0, j = 1]
                    Noeud->Et.h += 3;
                j += 2;
            }
            else{
                if(et.T[i%3][j%8] == et.T[i%3][(j + 1)%8] && et.T[i%3][(j - 1)%8] == 0)//[0, j = 1, 1]
                    Noeud->Et.h += 3;
                if(et.T[i%3][j%8] == et.T[i%3][(j - 1)%8] && et.T[i%3][(j + 1)%8] == 0)//[1, j = 1, 0]
                    Noeud->Et.h += 3;
                i += 3;
                if(et.T[i%3][j%8] == et.T[(i + 1)%3][j%8] && et.T[(i + 2)%3][j%8] == 0)//[i = 1, 1, 0] ou [0, i = 1, 1] ou [1, 0, i = 1] verticalement(cad franchir et varier les i).
                    Noeud->Et.h += 3;
                if(et.T[i%3][j%8] == et.T[(i - 1)%3][j%8] && et.T[(i - 2)%3][j%8] == 0)//[i = 1, 0, 1] ou [1, i = 1, 0] ou [0, 1, i = 1].
                    Noeud->Et.h += 3;
            }
        }
        else{
            Noeud->Et.h += 14;//Il a fait un alignement, donc il m�rite �a.
            i = et.position_mangee[0], j = et.position_mangee[1];
            j += 8;
            int x =Pion(et);//Le pion inverse
            if(j % 2 == 1){
                if(x == et.T[i][(j + 1)%8] && et.T[i][(j + 2)%8] == 0)//On supprima (i, j) tq [j = 1, 1, 0]
                    Noeud->Et.h += 3;
                if(x == et.T[i][(j - 1)%8] && et.T[i][(j - 2)%8] == 0)//On supprima (i, j) tq [0, 1, j = 1]
                    Noeud->Et.h += 3;
                if(x == et.T[i][(j + 2)%8] && et.T[i][(j + 1)%8] == 0)//On supprima (i, j) tq [j = 1, 0, 1]
                    Noeud->Et.h += 3;
                if(x == et.T[i][(j - 2)%8] && et.T[i][(j - 1)%8] == 0)//On supprima (i, j) tq [1, 0, j = 1]
                    Noeud->Et.h += 3;
            }
            else{
                if(x == et.T[i][(j + 1)%8] && et.T[i][(j - 1)%8] == 0)//On supprima (i, j) tq [j = 1, 1, 0]
                    Noeud->Et.h += 3;
                if(x == et.T[i][(j - 1)%8] && et.T[i][(j + 1)%8] == 0)//On supprima (i, j) tq [j = 1, 1, 0]
                    Noeud->Et.h += 3;
                i += 3;
                if(x == et.T[(i + 1)%3][j%8] && et.T[(i + 2)%3][j%8] == 0)
                    Noeud->Et.h += 3;
                if(x == et.T[(i - 1)%3][j%8] && et.T[(i - 2)%3][j%8] == 0)
                    Noeud->Et.h += 3;
            }
        }
    }
}

noeud* MiniMax(noeud *Noeud, int p, int n, char J){//p : le profondeur, n : Le profondeur qu'on choisit au d�but(La premi�re valeur de p).
    int hh;
    noeud *NN, *MAX;//NN : noeud qui va devenir � chaque fois le noeud qu'on pense qu'il est le plus grand.
    liste L;
    L.debut = L.fin = NULL;
    Noeud->Et.possible = -1;
    if(p == n && Phase(Noeud->Et) == 3)
        p -= 1, n -= 1;
    if((n != 1 && tournoa_termine(Noeud->Et)) || p == 0)//si p == 0, alors on est arriv� au profondeur qu'on veut.
        return Noeud;
    else{
        GenererSuccesseurs(Noeud->Et, &L, -1, -1, -1, -1, -1, -1, N);
        h(&L);
        for(noeud *KKK = L.debut; KKK != NULL; KKK = KKK->suivant)
            KKK->Et.h += 2 * p;
        if(Noeud->Et.joueur == J){
            for(NN = L.debut; NN != NULL; NN = NN->suivant){
                if(p == n && NN->Et.h >= 14){
                    NN->Et.h += (NN->Et.h)/3;
                }
                if(p == n && tournoa_termine(NN->Et)){//si cette condition est v�rifi�e, alors l'un des successeurs principales va gagner(On va gagner d�s le premier pas).
                    Noeud->successeur = NN;
                    return Noeud;
                }
                NN->Et.h += Noeud->Et.h;
                NN = MiniMax(NN, p - 1, n, J);
                if(NN == L.debut || MAX->Et.h < NN->Et.h){
                    MAX = NN;//S'il y'a aussi un noeud qui arrive � la valeur de h = - 1000, alors on doit conclure que ce noeud repr�sente aussi un �tat final, et donc on doit choisir entre les 2 �tats finaux de l'adversaire(-1005 ou -1010)(S'il n y'a aucune valeur qui est ordinaire, alors tous ce qu'on va choisir va nous amener � la perte).
                }
            }
        }
        else{
            for(NN = L.debut; NN != NULL; NN = NN->suivant){
                NN->Et.h = - NN->Et.h;
                if(p == n - 1 && tournoa_termine(NN->Et)){//si cette condition est v�rifi�e, alors l'un des successeurs principales va gagner(On va gagner d�s le premier pas).
                    Noeud->successeur = NN;
                    return Noeud;
                }
                NN->Et.h += Noeud->Et.h;
                NN = MiniMax(NN, p - 1, n, J);
                if(NN == L.debut || MAX->Et.h > NN->Et.h)
                    MAX = NN;//S'il y'a aussi un noeud qui arrive � la valeur de h = - 1000, alors on doit conclure que ce noeud repr�sente aussi un �tat final, et donc on doit choisir entre les 2 �tats finaux de l'adversaire(-1005 ou -1010)(S'il n y'a aucune valeur qui est ordinaire, alors tous ce qu'on va choisir va nous amener � la perte).
            }
        }
        hh = MAX->Et.h;
        Noeud->Et.h = hh;
        Noeud->successeur = MAX;
        return Noeud;
    }
}

noeud* AlphaBeta(noeud *Noeud, int p, int n, char J, int a, int b){//p : le profondeur, n : Le profondeur qu'on choisit au d�but(La premi�re valeur de p).
    int hh;
    noeud *NN, *MAX;//NN : noeud qui va devenir � chaque fois le noeud qu'on pense qu'il est le plus grand.
    liste L;
    L.debut = L.fin = NULL;
    Noeud->Et.possible = -1;
    if((n != 1 && tournoa_termine(Noeud->Et)) || p == 0){//si p == 0, alors on est arriv� au profondeur qu'on veut.
        return Noeud;
    }
    else{
        if(p == n)
            Noeud->Et.h = 0;
        GenererSuccesseurs(Noeud->Et, &L, -1, -1, -1, -1, -1, -1, N);
        h(&L);
        int v;
        if(Noeud->Et.joueur == J){
            v = INT_MIN;
            hh = - Noeud->Et.h;
            Noeud->successeur = L.debut;
            for(NN = L.debut; NN != NULL; NN = NN->suivant){
                if(p == n && NN->Et.h >= 14){
                    NN->Et.h += (NN->Et.h)/2;
                }
                if(p == n && tournoa_termine(NN->Et)){//si cette condition est v�rifi�e, alors l'un des successeurs principales va gagner(On va gagner d�s le premier pas).
                    Noeud->successeur = NN;
                    return Noeud;
                }
                NN->Et.h += Noeud->Et.h;
                NN = AlphaBeta(NN, p - 1, n, J, a, b);
                if(NN == L.debut || v < NN->Et.h){//Max
                    Noeud->successeur = NN;
                    v = NN->Et.h;
                }
                if(b < v){
                    Noeud->Et.h = v;
                    return Noeud;
                }
                a = fmax(a, v);
            }
        }
        else{
            v = INT_MAX;
            hh = Noeud->Et.h;
            Noeud->successeur = L.debut;
            for(NN = L.debut; NN != NULL; NN = NN->suivant){
                NN->Et.h = - NN->Et.h;
                if(p == n - 1 && tournoa_termine(NN->Et)){//si cette condition est v�rifi�e, alors l'un des successeurs principales va gagner(On va gagner d�s le premier pas).
                    Noeud->successeur = NN;
                    return Noeud;
                }
                NN->Et.h += Noeud->Et.h;
                NN = AlphaBeta(NN, p - 1, n, J, a, b);
                if(NN == L.debut || v > NN->Et.h){//Min
                    Noeud->successeur = NN;
                    v = NN->Et.h;
                }
                if(a > v){
                    Noeud->Et.h = v;
                    return Noeud;
                }
                b = fmin(b, v);
            }

        }
        Noeud->Et.h = v;
        return Noeud;
    }
}
