#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#define verb 1 // permet le debugage

typedef struct{
    /*uint8_t id;*/
    char pseudo[9];
    int fd_udp;
    int fd_tcp;
    int hasCalledStart;
    u_int32_t points;
    char *x;
    char *y;
    int estDejaInscrit;
}Joueur;

typedef struct{ 
    uint8_t id; 
    uint8_t nb_joueur;
    int mx_nb_joueur;
    int nb_joueur_pret;
    uint16_t width;
    uint16_t height;
    int created;
    int started;
    Joueur *liste;
    uint8_t ghosts;
    char *ip_multidiffusion;
    char *port_multidiffusion;
}Partie;

// Thread pour les accept
// creer socket dans thread
// creer nouveau thread pour le client
int join_Partie(Joueur *j, uint8_t numPartie);
void main_boucle(int fd);
char* pseudo(int fd); // Demande le pseudo en boucle tant que != 8 caracteres
uint8_t getIdPartieFromJoueur(char idPlayer[]);
void diffuserGhostCaught(Joueur *actuel, int x, int y, int toAdd, int partie);
void multiDiffuseToPlayers(char *mess,char *idPlayer,uint8_t partie);
void multiDiffuseScoreFinal(int idPartie, Joueur *j);
int checkIfGhostHere(char *maze, int width,int i, int j);
void updatePositionJoueur(Joueur *actuel, int x,int y, int add,uint8_t partie);
int checkWall(char *maze,int width,int x,int y);
void diffuserGhostMove(uint8_t partie, char *x, char *y);
int envoiUDPToPlayer(char *mess,char *pseudoFrom, char *pseudoTo,u_int8_t partie);
void initPositionPlayer(Joueur *j, char *maze, int width, int height, uint8_t idPartie);
Joueur * getGagnant(uint8_t idPartie);
int startsWith(const char *haystack, const char *needle) ;
int probablyLancePartie(char idPlayer[],int fd);
void lancePartie(uint8_t partie,int fd,char idJoueur[]);
void create_client(Joueur * joueur, int fd, char *pseudo); //uint8_t id
int create_Partie(uint8_t id, Joueur *j);
int checkIDExist(uint8_t id);
uint8_t getPartiesNotYetStarted();
Joueur* getJoueur(char id[]);
void enrollPlayer(Joueur *j);
void updateJoueurUDP(Joueur *j);
uint8_t desinscription(char id[]);
void getListePlayers(uint8_t numPartie, int fd);
void getSizeLaby(uint8_t idPartie, int fd);
void getPartiesSpecials(int fd);
void afficheGame(int fd);
void waitClient(int fd);
int getRandomNumber(int lower,int upper);
int getPortUdpFromJoueur(char *pseudoTo);
int isRegistered(char *id_actual_player);
void saveClient(int fd,char* id_from_client);
int isInsidePartie(char *pseudoTo,uint8_t partie);

void *comm(void *arg); // fonction thread