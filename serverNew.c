#include "serverNew.h"
#include "Labyrinthe.c"
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

char PORTS_USED[255];
char IP_USED[255];

int NOTSTARTED=1; //true

Partie liste[255];
uint8_t nb_partie = 0;
uint8_t compteurID = 0; //Pour mettre un id diff a toute les games

int id_liste_joueur=0;
Joueur liste_clients[255]; // clients enregistres par le serveur 

int id_liste_joueur_enrolled=0;
Joueur liste_clients_enrolled[255]; // clients inscrits a une partie par le serveur

uint16_t *WIDTH;
uint16_t *HEIGHT;
char *MAZE_GLOBAL;

/* UDP */
int envoiUDPToPlayer(char *mess,char *pseudoFrom, char *pseudoTo, uint8_t partie){
    //check si le pseudoTo existe dans la partie
    if(isInsidePartie(pseudoTo,partie)==0){
        printf("pseudoTo is %s\n",pseudoTo);
        printf("[func:envoiUDPToPlayer] pseudoTo nest pas dans la partie \n");
        return 0;
    }
    int sock=socket(PF_INET,SOCK_DGRAM,0);
    struct addrinfo *first_info;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    char *port = malloc(sizeof(char)*4);
    int portUdpTo = getPortUdpFromJoueur(pseudoTo);
    sprintf(port, "PORT UDP POUR ENVOI UDP is %d\n", portUdpTo);
  
    int r=getaddrinfo("localhost",port,&hints,&first_info); 
    if(r==0){
        if(first_info!=NULL){
            struct sockaddr *saddr=first_info->ai_addr;
            sendto(sock,mess,strlen(mess),0,saddr,(socklen_t)sizeof(struct sockaddr_in));
            return 1;
        }else{
            printf("[func:envoiUDPToPlayer] firstINFO is NULL \n");
            return 0;
        }
    }else{
        printf("[func:envoiUDPToPlayer] getaddrinfo ERREUR\n");

        return 0;
    }
}
/* UDP */

/* MULTIDIFFUSION */
 
void diffuserGhostMove(uint8_t partie, char *x, char *y){
    /// envoyer [GHOST␣x␣y+++] := (x,y) la position d'arrivee du fantome qui se deplace actuellement
    printf("[func: diffuserGhostMove] inside multidiffusion fantome qui se deplace \n");
    int sock=socket(PF_INET,SOCK_DGRAM,0);
    struct addrinfo *first_info;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    //printf("liste[partie].ip_multidiffusion: %s\n", liste[partie].ip_multidiffusion);
    //printf("liste[partie].port_multidiffusion: %s\n",liste[partie].port_multidiffusion);
    int r=getaddrinfo(liste[partie].ip_multidiffusion,liste[partie].port_multidiffusion,&hints,&first_info);
    if(r==0){
        if(first_info!=NULL){
            struct sockaddr *saddr=first_info->ai_addr;
            char *final = malloc(sizeof(char)*100);
            strcpy(final,"GHOST "); 
            strcat(final," ");          
            strcat(final,x);
            strcat(final," ");
            strcat(final,y);
            strcat(final,"+++");
            int si = sendto(sock,final,strlen(final),0,saddr,(socklen_t)sizeof(struct sockaddr_in));
        }
    }
}

void diffuserGhostCaught(Joueur *actuel, int x, int y, int toAdd, int partie){
    /// si un joueur capture un fantome , envoyer [SCORE␣id␣p␣x␣y+++] := (x,y) position du ghost attrape
    printf("[func: diffuserGhostCaught] inside multidiffusion fantome attrapé \n");
    char *pseudo = malloc(sizeof(char)*8);
    memmove(pseudo,actuel->pseudo,8);
    int points;
    for(int k=0;k<255;k++){
        if(strcmp(liste_clients[k].pseudo,actuel->pseudo) == 0){
            points = liste_clients[k].points;
            break;
        }
    }
    points += toAdd;
    int sock=socket(PF_INET,SOCK_DGRAM,0);
    struct addrinfo *first_info;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    //printf("liste[partie].ip_multidiffusion: %s\n", liste[partie].ip_multidiffusion);
    //printf("liste[partie].port_multidiffusion: %s\n",liste[partie].port_multidiffusion);
    int r=getaddrinfo(liste[partie].ip_multidiffusion,liste[partie].port_multidiffusion,&hints,&first_info);
    if(r==0){
        if(first_info!=NULL){

            struct sockaddr *saddr=first_info->ai_addr;
            char *final = malloc(sizeof(char)*100);
            strcpy(final,"SCORE "); 
            strcat(final,pseudo);
            strcat(final," ");
            char *pointStr = malloc(sizeof(char)*100);
            sprintf(pointStr, "%d", points);
            strcat(final,pointStr);
            strcat(final," ");
            char *xStr = malloc(sizeof(char)*100);
            sprintf(xStr, "%d", x);
            strcat(final,xStr);
            strcat(final," ");
            char *yStr = malloc(sizeof(char)*100);
            sprintf(yStr, "%d", y);
            strcat(final,yStr);
            strcat(final,"+++");
            //printf("FANTOME va etre envoye DANS LE MULTICAST! \n");

            //printf("strlen is %lu octets\n",strlen(final));
            int si = sendto(sock,final,strlen(final),0,saddr,(socklen_t)sizeof(struct sockaddr_in));
            //printf("nbr octets envoyes : %d\n",si);
        }
    }
    
}
void multiDiffuseToPlayers(char *mess,char *idPlayer,uint8_t partie){
    // le serveur multi-diffuse le message [MESSA␣id␣mess+++] a tout le monde
    printf("[func: multiDiffuseToPlayers] inside multidiffusion message envoi pour tous \n");
    int sock=socket(PF_INET,SOCK_DGRAM,0);
    struct addrinfo *first_info;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    int r=getaddrinfo(liste[partie].ip_multidiffusion,liste[partie].port_multidiffusion,&hints,&first_info);
    if(r==0){
        if(first_info!=NULL){
            struct sockaddr *saddr=first_info->ai_addr;
            char *final = malloc(sizeof(char)*100);
            strcpy(final,"MESSA "); 
            strcat(final,idPlayer);
            strcat(final," ");
            strcat(final,mess);
            strcat(final,"+++");
            sendto(sock,final,strlen(final),0,saddr,(socklen_t)sizeof(struct sockaddr_in));
        }
    }
}
void multiDiffuseScoreFinal(int idPartie, Joueur *j){
    printf("[func: multiDiffuseScoreFinal] inside multidiffusion score final de la partie \n");
    int sock=socket(PF_INET,SOCK_DGRAM,0);
    struct addrinfo *first_info;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype=SOCK_DGRAM;
    int r=getaddrinfo(liste[idPartie].ip_multidiffusion,liste[idPartie].port_multidiffusion,&hints,&first_info);
    if(r==0){
        if(first_info!=NULL){
            struct sockaddr *saddr=first_info->ai_addr;
            char *final = malloc(sizeof(char)*100);
            strcpy(final,"ENGA ");
            char *s = malloc(sizeof(char)*8);
            memmove(s,j->pseudo,8); 
            strcat(final,s);
            strcat(final," ");
            char *s1 = malloc(sizeof(char)*10);
            int u1 = sprintf(s1, "%u",j->points); 
            strcat(final,s1);
            strcat(final,"+++");
            sendto(sock,final,strlen(final),0,saddr,(socklen_t)sizeof(struct sockaddr_in));
        }
    }
}

/* MULTIDIFFUSION */


/* GETTERS */

int getPortUdpFromJoueur(char *pseudoTo){
    for(int k=0;k<255;k++){
        if(strcmp(liste_clients[k].pseudo,pseudoTo) == 0){
            return liste_clients[k].fd_udp;
        }
    }
    return 0;
}

Joueur * getGagnant(uint8_t idPartie){
    Joueur *winner = malloc(sizeof(Joueur));
    int max = 0;
    for(int i=0;i<liste[idPartie].nb_joueur;i++){
        printf("[func: getGagnant] %s possede %d points\n",liste[idPartie].liste[i].pseudo,liste[idPartie].liste[i].points);
        if(liste[idPartie].liste[i].points >= max){
            max = liste[idPartie].liste[i].points;
            winner = &liste[idPartie].liste[i];
        }
    }
    return winner;
}

uint8_t getIdPartieFromJoueur(char idPlayer[]){
    for(int i=0;i<nb_partie;i++){
        for(int j=0;j<liste[i].nb_joueur;j++){
            printf("[func: getIdPartieFromJoueur] Joueur numéro %d is %s\n",j,liste[i].liste[j].pseudo);
            if(strcmp(liste[i].liste[j].pseudo,idPlayer)==0){
                return liste[i].id;
            }
        }
    }
    return 200;
}

char *getRandomIp(){
    char *ip= malloc(sizeof(char)*15);

    char *one = malloc(sizeof(char)*3);
    int r = (rand() % (239 + 1 - 224)) + 224;
    sprintf(one, "%d", r);

    char *second = malloc(sizeof(char)*3);
    r = (rand() % (255 + 1 - 100)) + 100;
    sprintf(second, "%d", r);

    char *third = malloc(sizeof(char)*3);
    r = (rand() % (255 + 1 - 100)) + 100;
    sprintf(third, "%d", r);

    char *fourth = malloc(sizeof(char)*3);
    r = (rand() % (255 + 1 - 100)) + 100;
    sprintf(fourth, "%d", r);

    strcat(ip,one);
    strcat(ip,".");
    strcat(ip,second);
    strcat(ip,".");
    strcat(ip,third);
    strcat(ip,".");
    strcat(ip,fourth);

    //todo add a test for uniqueness with an array
    return ip;
}

char *getRandomAvailablePort(int lower, int upper){
    char *port= malloc(sizeof(char)*4);
    int r = (rand() % (upper + 1 - lower)) + lower;
    sprintf(port, "%d", r);

    //TODO add a test for uniqueness with an array
    return port;
}

uint8_t getPartiesNotYetStarted(){
    uint8_t cpt=0;
    for(int i=0;i<nb_partie;i++){
        if(liste[i].started == 0){
            cpt++;
        }
    }
    return cpt;
}

Joueur* getJoueur(char id[]){
    for(int i=0;i<id_liste_joueur;i++){
        //(" [func: getJoueur] Joueur trouve est : %s\n",liste_clients[i].pseudo);

        if(strcmp(liste_clients[i].pseudo,id)==0){
            printf("[func: getJoueur] Joueur trouve\n");
            return &liste_clients[i];
        }
    }
    printf("[func: getJoueur] Joueur not found\n");

    return NULL;
}

void getListePlayers(uint8_t numPartie, int fd){
    if(numPartie>nb_partie || numPartie<0 || liste[numPartie].created!=1){
        int r = send(fd,"DUNNO***",8,0);
        if(r<0){
            perror("error in sending DUNNO\n");
            exit(EXIT_FAILURE);
        }
    }else{
        int s = liste[numPartie].nb_joueur;
        //envoyer  LIST! numPartie s***
        char envoy[100];

        strcpy(envoy,"LIST! ");
        char str[10];
        sprintf(str, "%hhu",numPartie); 
        strcat(envoy,str);
        strcat(envoy," ");

        char str1[10];
        sprintf(str1, "%d",s); 
        strcat(envoy,str1); 
        strcat(envoy,"***"); 
        int r = send(fd,envoy,strlen(envoy),0);
        if(r<0){
            perror("error in sending LIST!\n");
            exit(EXIT_FAILURE);
        }

        for(int i=0;i<s;i++){
            //envoyer PLAYR liste[numPartie].liste[i].id***
            char envoyPlay[100];
            strcpy(envoyPlay,"PLAYR ");
            char *ps = liste[numPartie].liste[i].pseudo;
            strcat(envoyPlay,ps);
            strcat(envoyPlay,"***"); 
            
            int r = send(fd,envoyPlay,strlen(envoyPlay),0);
            if(r<0){
                perror("error in sending PLAYR\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void getSizeLaby(uint8_t idPartie, int fd){
    if(idPartie>nb_partie || idPartie<0 || liste[idPartie].created!=1){
        int r = send(fd,"DUNNO***",8,0);
        if(r<0){
            perror("error in sending DUNNO\n");
            exit(EXIT_FAILURE);
        }
    }else{
        //envoyer SIZE! idPartie liste[idPartie].h liste[idPartie].w
        char envoySize[100];

        strcpy(envoySize,"SIZE! ");
        char str[10];
        sprintf(str, "%hhu", idPartie);
        strcat(envoySize,str);
        strcat(envoySize," ");

        char str1[10];
        sprintf(str1, "%hu", liste[idPartie].height);
        strcat(envoySize,str1);
        strcat(envoySize," ");
        char str2[10];
        sprintf(str2, "%hu", liste[idPartie].width); 
        strcat(envoySize,str2); 
        strcat(envoySize,"***"); 

        int r = send(fd,envoySize,strlen(envoySize),0);
        if(r<0){
            perror("error in sending SIZE\n");
            exit(EXIT_FAILURE);
        }
    }
}

void getPartiesSpecials(int fd){
    uint8_t n = getPartiesNotYetStarted();
    char envoyMess[100];
    strcpy(envoyMess,"GAMES ");
    char str[10];
    sprintf(str, "%hhu", n);
    strcat(envoyMess,str);
    strcat(envoyMess, "***\0");
    int r = send(fd,envoyMess,strlen(envoyMess),0);
    if(r<0){
        perror("error in sending GAMES\n");
        exit(EXIT_FAILURE);
    }
    // envoyer OGAME m s***
    for(int i=0;i<nb_partie;i++){
        if(liste[i].started == 0){
            uint8_t m = liste[i].id;
            uint8_t s = liste[i].nb_joueur;
            //envoi OGAME m s***
            char envoyM[100];
            strcpy(envoyM,"OGAME ");
            char str[10];
            sprintf(str, "%hhu", m);
            strcat(envoyM,str);
            strcat(envoyM," ");
            char str1[10];
            sprintf(str1, "%hhu", s);
            strcat(envoyM,str1); 
            strcat(envoyM, "***\0");

            int r = send(fd,envoyM,strlen(envoyM),0);
            if(r<0){
                perror("error in sending OGAME\n");
                exit(EXIT_FAILURE);
            }
            

        }
    }
}

void afficheGame(int fd){
    printf("[func: afficheGame] inside the func ! \n");
    char res[15];
    for(int i=0;i<nb_partie;i++){
        if(liste[i].started == 0){
            uint8_t m = liste[i].id;
            uint8_t s = liste[i].nb_joueur;
            //envoi OGAME m s***
            char envoyM[100];
            strcpy(envoyM,"OGAME ");
            char str[10];
            sprintf(str, "%hhu", m);
            strcat(envoyM,str);
            strcat(envoyM," ");
            char str1[10];
            sprintf(str1, "%hhu", s);
            strcat(envoyM,str1); 
            strcat(envoyM, "***\0");

            int r = send(fd,envoyM,strlen(envoyM),0);
            if(r<0){
                perror("error in sending OGAME\n");
                exit(EXIT_FAILURE);
            }
            

        }
    }
}

int getRandomHeight(){
    return (rand() % (35 + 1 - 4)) + 4;
}

int getRandomWidth(){
    return (rand() % (35 + 1 - 4)) + 4;
}

int getRandomGhosts(){
    return (rand() % (8 + 1 - 2)) + 2;
}

/* GETTERS */

/* SETTERS */

void updatePositionJoueur(Joueur *actuel, int x,int y, int add,uint8_t partie){
    for(int k=0;k<id_liste_joueur;k++){
        if(strcmp(liste_clients[k].pseudo,actuel->pseudo) == 0){
            liste_clients[k].points += add;
            char *s = malloc(sizeof(char)*10);
            sprintf(s,"%d", x);
            printf("[func: getJoueur] Position x actualisée pour le joueur %s\n",s);
            memmove(liste_clients[k].x,s,3);

            char *s1 = malloc(sizeof(char)*10);
            sprintf(s1,"%d", y);
            printf("[func: getJoueur] Position y actualisée pour le joueur %s\n",s1);
            memmove(liste_clients[k].y,s1,3);

        }
    }
    for(int k=0;k<liste[partie].nb_joueur;k++){
        if(strcmp(liste[partie].liste[k].pseudo,actuel->pseudo) == 0){
            liste[partie].liste[k].points += add;
            char *s = malloc(sizeof(char)*10);
            sprintf(s,"%d", x);
            //printf("position x actualisee du joueur %s\n",s);
            memmove(liste[partie].liste[k].x,s,3);

            char *s1 = malloc(sizeof(char)*10);
            sprintf(s1,"%d", y);
            //printf("position y actualisee du joueur %s\n",s1);
            memmove(liste[partie].liste[k].y,s1,3);

        }
    }
}

void enrollPlayer(Joueur *j){
    liste_clients_enrolled[id_liste_joueur_enrolled] = *j;
    id_liste_joueur_enrolled++;
}

void updateJoueurUDP(Joueur *j){
    for(int i=0;i<id_liste_joueur;i++){
        if(strcmp(liste_clients[i].pseudo,j->pseudo)==0){
            liste_clients[i].fd_udp = j->fd_udp;
            printf("[func: updateJoueurUDP] Port UDP du client est mis a jour : %d\n",liste_clients[i].fd_udp);
        }
    }
    for(int i=0;i<nb_partie;i++){
        for(int k=0;k<liste[i].nb_joueur;k++){
            if(strcmp(liste[i].liste[k].pseudo,j->pseudo)==0){
                liste[i].liste[k].fd_udp = j->fd_udp;
            }
        }
    }
    
}

void saveClient(int fd,char* id_from_client){
    Joueur *joueur_information = malloc(sizeof(Joueur));
    // enregistrer les infos du client dans joueur info
    //creer le joueur associe aux infos recues
    create_client(joueur_information,fd,id_from_client);
    if (verb)
        printf("[func: saveClient] PSEUDO CLIENT IS  %s AND FD_TCP IS %d\n",joueur_information->pseudo, joueur_information->fd_tcp);
    
    // ajouter joueur_information a la liste des joueurs qui ont interragi avec le serveur mais pas encore inscrits
    liste_clients[id_liste_joueur] = *joueur_information;
    id_liste_joueur++;
    //free(joueur_information);
}

/* HELPERS */

void initPositionPlayer(Joueur *j, char *maze, int width, int height, uint8_t idPartie){
    
    //random int between width and height for both x and y 
    int x=0;
    int y=0;

    while(checkWall(maze,width,x,y)==1 || checkIfGhostHere(maze,width,x,y)==1){
        x = getRandomNumber(0,height-1); //get a random x in the board
        y = getRandomNumber(0,width-1); //get a random y in the board
    }
    char *s = malloc(sizeof(char)*10);
    sprintf(s,"%d", x);
    printf("[func: initPositionPlayer] Position x initialisée pour le joueur : %s\n",s);
    memmove(j->x,s,3);

    char *s1 = malloc(sizeof(char)*10);
    sprintf(s1,"%d", y);
    printf("[func: initPositionPlayer] Position y initialisée pour le joueur : %s\n",s1);
    memmove(j->y,s1,3);

    maze[x *width +y] = 3;
    
}

int checkIDExist(uint8_t id){
    for(int i = 0; i < nb_partie; i ++){
        if(liste[i].id == id){
            return 1;
        }
    }
    return 0;
}

char* pseudo(int fd) {
    int rc = 0;
    char *pseudo = malloc(sizeof(char)*10);
    while (rc != 8){
        rc = recv(fd, pseudo, 8, 0);
    }
    pseudo[8] = '\0';
    return pseudo;
}

uint8_t desinscription(char id[]){

    if(strlen(id)==0) return 200;
    for(int i=0;i<nb_partie;i++){
        for(int j=0;j<liste[i].nb_joueur;j++){
            //printf("id is %s \n",liste[i].liste[j].pseudo);

            if(strcmp(liste[i].liste[j].pseudo,id)==0){


                for(int k=j;k<liste[i].nb_joueur-1;k++){
                    liste[i].liste[k] = liste[i].liste[k+1];

                }
                liste[i].nb_joueur -= 1;

                //boucle to check each user
                for(int l=0;l<liste[i].nb_joueur;l++){
                    printf("[func: desinscription] Joueur encore dans la partie : %s \n",liste[i].liste[l].pseudo);
                }
            
                return liste[i].id;
            }
        }
    }
    return 200;
}

int isRegistered(char *id_actual_player){
    if(strlen(id_actual_player)==0) return 0;
    for(int i=0;i<nb_partie;i++){
        for(int j=0;j<liste[i].nb_joueur;j++){
            if(strcmp(liste[i].liste[j].pseudo,id_actual_player)==0){
                printf("[func: isRegistered] %s est deja enregistre dans une partie !\n",id_actual_player);
                return 1;
            }
        }
    }
    return 0;
}

int isInsidePartie(char *pseudoTo, uint8_t partie){
    //printf("[func:isInsidePartie] PSEUDO TO is %s\n",pseudoTo);

    for(int j=0;j<liste[partie].nb_joueur;j++){
        //printf("[func:isInsidePartie] joueur %d is %s\n",j,liste[partie].liste[j].pseudo);
        //printf("[func:isInsidePartie] COMPARAISON %d is %d for %s / (pseudoTO) %s \n",j,strcmp(liste[partie].liste[j].pseudo,pseudoTo), liste[partie].liste[j].pseudo,pseudoTo);
        if(strcmp(liste[partie].liste[j].pseudo,pseudoTo)==0){
            return 1;
        }
    }
    return 0;
}
/* HELPERS */

/* PARTIE FUNCTIONS */

void create_client(Joueur * joueur, int fd, char *pseudo){ //uint8_t id
    //joueur->id = id;
    strcpy(joueur->pseudo,pseudo);
    joueur->fd_tcp = fd;
    joueur->hasCalledStart = 0;
    joueur->points = 0;
    joueur->x = malloc(sizeof(char)*3);
    joueur->y = malloc(sizeof(char)*3);
    joueur->estDejaInscrit = 0;
}

int create_Partie(uint8_t id, Joueur *j){
    Partie p;
    p.nb_joueur = 0;
    p.height = 4 ; //getRandomHeight();  
    p.width = 4; //getRandomWidth(); 
    p.mx_nb_joueur = 10;
    p.nb_joueur_pret = 0;
    p.id = id;
    p.started = 0;
    p.ghosts = 2;//getRandomGhosts(); 
    p.ip_multidiffusion = malloc(sizeof(char)*15);
    p.port_multidiffusion = malloc(sizeof(char)*4);
    char *tmpIP = getRandomIp();
    memmove(p.ip_multidiffusion,tmpIP,15);
    char *tmp = getRandomAvailablePort(1000,9999);
    memmove(p.port_multidiffusion,tmp,4);
    p.created = 1;
    p.liste = (Joueur*) malloc(p.mx_nb_joueur * sizeof(Joueur));
    compteurID++;
    liste[nb_partie] = p; 
    nb_partie++;
    int a = join_Partie(j,id);
    //return 1;
    return a;
}

int join_Partie(Joueur *j, uint8_t numPartie){
    if(!checkIDExist(numPartie)){
        printf("[func: joinPartie] La partie %d n'existe pas ! \n",numPartie);
        return 0;
    }else if(j->estDejaInscrit == 1){
        return 0;
    }else{
        // numpartie existe
        j->estDejaInscrit = 1;
        printf("[func: joinPartie] Adding joueur to partie \n");
        liste[numPartie].liste[liste[numPartie].nb_joueur] = *j;
        
        pthread_mutex_lock(&mutex);
        liste[numPartie].nb_joueur++;
        pthread_mutex_unlock(&mutex);

        /*for(int i=0;i<liste[numPartie].nb_joueur;i++){
            printf("(INSIDE JOIN PARTIE AFTER ADDING THE PLAYER) joueur %d is %s\n",i,liste[numPartie].liste[i].pseudo);
        }*/
        enrollPlayer(j); //liste_joueurs_enrolled contient mtn le joueur j
        return 1;
    }
}

int probablyLancePartie(char idPlayer[],int fd){
    //lancer la partie que si tous les joueurs inscrits ont appele START***
    //step 1 : get game tied to idPlayer
    //step 2 : check if all players enrolled in that game called START
    //step 3: launch the game based on the latter check
    uint8_t idPartie = getIdPartieFromJoueur(idPlayer);
    if(liste[idPartie].nb_joueur_pret==liste[idPartie].nb_joueur){
        return 1;
    }else{
        printf("[func: probablyLancePartie] Id de la partie : %d\n",idPartie);
        printf("[func: probablyLancePartie] Nbr de joueurs de la partie : %d\n",liste[idPartie].nb_joueur);
        printf("[func: probablyLancePartie] Nbr  de joueurs prets : %d\n",liste[idPartie].nb_joueur_pret);
        return 0;
    }
}

void setPartieWELCOME(uint8_t partie){
    char envoy[100];
   
    strcpy(envoy,"WELCO ");

    char str[10];
    sprintf(str, "%hhu",partie); 

    strcat(envoy,str);
    strcat(envoy," ");
    char str1[10];
    sprintf(str1, "%hu",liste[partie].height); 

    strcat(envoy,str1); 

    strcat(envoy," ");


    char str2[10];
    sprintf(str2, "%hu",liste[partie].width); 

    strcat(envoy,str2);  

    strcat(envoy," ");
    
    char str3[10];
    sprintf(str3, "%hu",liste[partie].ghosts); 
    strcat(envoy,str3); 

    strcat(envoy," ");
    
    char *str4 = malloc(sizeof(char)*15);
    printf("[func: setPartieWELCOME] Ip Multidiffusion est : %s\n",liste[partie].ip_multidiffusion);

    sprintf(str4, "%s",liste[partie].ip_multidiffusion); 
    strcat(envoy,str4); 

    strcat(envoy," ");
    
    char str5[10];
    printf("[func: setPartieWELCOME] Port Multidiffusion est : %s\n",liste[partie].port_multidiffusion);

    sprintf(str5, "%s",liste[partie].port_multidiffusion); 

    strcat(envoy,str5); 

    strcat(envoy,"***"); 

    // associer un labyrinthe a la partie
    
    WIDTH = malloc(sizeof(uint16_t));
    HEIGHT = malloc(sizeof(uint16_t));
    *WIDTH = liste[partie].width;
    *HEIGHT = liste[partie].height;

    MAZE_GLOBAL = (char*)malloc( (*WIDTH) * (*HEIGHT) * sizeof(char));

    if(MAZE_GLOBAL == NULL) {
      printf("[func: setPartieWelcome] Error Maze: not enough memory\n");
      exit(EXIT_FAILURE);
    }
    GenerateMaze(MAZE_GLOBAL,*WIDTH,*HEIGHT);
    initGhosts(MAZE_GLOBAL,*WIDTH,*HEIGHT,liste[partie].ghosts);
    
    //envoyer le message a tous les joueurs de la partie
    for(int i=0;i<liste[partie].nb_joueur;i++){
        int r = send(liste[partie].liste[i].fd_tcp,envoy,strlen(envoy),0);
        if(r<0){
            perror("error in sending WELCO\n");
            exit(EXIT_FAILURE);
        }
    }


    for(int i=0;i<liste[partie].nb_joueur_pret;i++){

        initPositionPlayer(&liste[partie].liste[i],MAZE_GLOBAL,*WIDTH,*HEIGHT,partie); // initialiser les positions de chaque joueur aleatoirement
        char envoyPlay[100];
        strcpy(envoyPlay,"POSIT ");
        char *ps = liste[partie].liste[i].pseudo;
        strcat(envoyPlay,ps);

        strcat(envoyPlay," ");

        char su[10];
        sprintf(su, "%s",liste[partie].liste[i].x);
        strcat(envoyPlay,su); 

        strcat(envoyPlay," ");

        char su1[10];
        sprintf(su1, "%s",liste[partie].liste[i].y);
        strcat(envoyPlay,su1);

        strcat(envoyPlay,"***"); 
        
        int r = send(liste[partie].liste[i].fd_tcp,envoyPlay,strlen(envoyPlay),0);
        if(r<0){
            perror("error in sending POSIT\n");
            exit(EXIT_FAILURE);
        }

    }

}

void lancePartie(uint8_t partie,int fd,char idJoueur[]){
    liste[partie].started = 1;
    // launch the actual game with ghosts etc
    // envoyer aux joueurs  
    // 1. [WELCO␣idPartie␣h␣w␣f␣ip␣port***] f=>nbr de fantomes, ip/port=> ip/port de multidiffusion
    // 2. [POSIT␣id␣x␣y***] id du joueur, (x,y) position du joueur aleatoire au debut tant que c valide


    while(liste[partie].ghosts>0 && liste[partie].nb_joueur>0){
        // TODO : add randomMove pour les ghosts
        /*
        char *pos = moveGhostNotCaught(MAZE_GLOBAL,*WIDTH,*HEIGHT);
        sleep(1200);
        diffuserGhostMove(partie,pos[0],pos[1]);
        */
        printf("[func: lancePartie] Nombre de fantomes dans la partie est : %d\n", liste[partie].ghosts);
        //printf("LABYRINTHE AVEC FANTOMES ET JOUEUR\n");
        printf("\n");
        printf("\n");
        ShowMaze(MAZE_GLOBAL,*WIDTH,*HEIGHT);
        printf("\n");
        printf("\n");
        printf("[func: lancePartie] In the big party listening events... \n");
        // tant que la partie est en cours
        char getMsg[100];
        int r;
        for(int i=0;i<liste[partie].nb_joueur;i++){
            r = recv(liste[partie].liste[i].fd_tcp,getMsg,100,0);
            if(r<0){
                perror("error in sending WELCO\n");
                exit(EXIT_FAILURE);
            }
            getMsg[r] = '\0';
                // [UPMOV␣d***]
            char *distance = malloc(sizeof(char)*3);

            if(strncmp("UPMOV", getMsg, 5) == 0){
                //printf(" [func: lancePartie] inside UPMOV\n");
            //recup la distance d
                memmove(distance,&getMsg[6],1);
                memmove(distance+1,&getMsg[7],1);
                memmove(distance+2,&getMsg[8],1);

            Joueur *actuel = getJoueur(idJoueur);
            //printf("joueur actuel is %s\n",actuel->pseudo);
            int d = atoi(distance);
            //printf("distance is %d\n",d);
            int x = atoi(actuel->x); //ERREUR
            //printf("x (joueur) in char is %s\n",actuel->x);
            //printf("x (joueur) is %d\n",x);

            int y = atoi(actuel->y);
            //printf("y (joueur) in char is %s\n",actuel->y);
            //printf("y (joueur) is %d\n",y);

            //check for potential out of bounds
            if(x-d <0){
                    int r = send(liste[partie].liste[i].fd_tcp,"out of bounds, please change the number\n",40,0);
                    if(r<0){
                        perror("error in sending POSIT\n");
                        exit(EXIT_FAILURE);
                    }
                    printf("[func: lancePartie] Out of bounds, please change the number\n"); 
            }
            int FLAG = 1;

            int i=0;
            int tmpX = x; 
            int toAdd = 0; // nbr de points du joueur a ajouter si ghost croise
            while(checkWall(MAZE_GLOBAL,*WIDTH,tmpX-1,y) == 0 && i<d){
                    // boucle vers le haut de d cases
                    //printf("x for fantom check is %d\n",tmpX-1);
                    //printf("y for fantom check is %d\n",y);

                    if(checkIfGhostHere(MAZE_GLOBAL,*WIDTH,tmpX-1,y)==1){
                        printf("[func: lancePartie] Fantome attrape ! \n");
                        liste[partie].ghosts -=1;
                        printf("[func: lancePartie] Nouveau nombre de fantomes est : %d \n",liste[partie].ghosts);
                        FLAG = 0;
                        //faire disparaitre le fantome 
                        MAZE_GLOBAL[(tmpX-1)* (*WIDTH) +y] = 8; 
                        // update le nbr de points du client
                        toAdd+=200;
                        diffuserGhostCaught(actuel,tmpX-1,y,toAdd,partie); 
                    }
                    tmpX-=1; // update la position temporaire x du joueur
                    i++;
                }
                updatePositionJoueur(actuel,tmpX,y,toAdd,partie); //update position (x,y) and score du joueur
                
                MAZE_GLOBAL[*WIDTH*tmpX + y] = 3; // modifie sur le plateau la pos du joueur
                MAZE_GLOBAL[x* (*WIDTH) +y] = 8; // valeur par defaut pr dire que la case est libre

                if(FLAG==1){
                    // envoyer le msg MOVE!␣x␣y*** au client si aucun ghost croise avc (x,y) nvlle position

                    char env[100];
                    strcpy(env,"MOVE! ");

                    strcat(env,actuel->x); 
                    strcat(env," ");

                    strcat(env,actuel->y);

                    strcat(env,"***");
                    //printf("test for env (MOVE!) is %s \n",env);
                    int r = send(liste[partie].liste[i].fd_tcp,env,strlen(env),0);
                    if(r<0){
                        perror("error in sending POSIT\n");
                        exit(EXIT_FAILURE);
                    }
                }else{
                    // envoyer le msg MOVEF x y p*** au client avec p le nv nbr de points
                    char env[100];
                    strcpy(env,"MOVE! ");
                    char p1[10];
                    sprintf(p1, "%s",actuel->x);
                    strcat(env,p1);

                    strcat(env," ");

                    char p2[10];
                    sprintf(p2, "%s",actuel->y);
                    strcat(env,p2);

                    strcat(env," ");

                    char *po = malloc(sizeof(char)*10);
                    sprintf(po,"%u", actuel->points);
                    strcat(env,po); 


                    strcat(env,"***");
                    //printf("test for env (MOVE avec points!) is %s \n",env);
                    int r = send(liste[partie].liste[i].fd_tcp,env,strlen(env),0);
                    if(r<0){
                        perror("error in sending POSIT\n");
                        exit(EXIT_FAILURE);
                    }

                }

            }

            // [DOMOV␣d***]

            if(strncmp("DOMOV", getMsg, 5) == 0){
                //TODO : parcourir le labyrinthe pour chaque (i,j) check fantome lors du mouvement := checkIfGhostHere()
                //printf("actually in DOMOV\n");
            //recup la distance d
                memmove(distance,&getMsg[6],1);
                memmove(distance+1,&getMsg[7],1);
                memmove(distance+2,&getMsg[8],1);

            Joueur *actuel = getJoueur(idJoueur);
            //printf("joueur actuel is %s\n",actuel->pseudo);
            int d = atoi(distance);
            //printf("distance is %d\n",d);
            int x = atoi(actuel->x); //ERREUR
            //printf("x (joueur) in char is %s\n",actuel->x);
            //printf("x (joueur) is %d\n",x);

            int y = atoi(actuel->y);
            //printf("y (joueur) in char is %s\n",actuel->y);
            //printf("y (joueur) is %d\n",y);

            //check for potential out of bounds
            if(x+d <0){
                    int r = send(liste[partie].liste[i].fd_tcp,"out of bounds, please change the number\n",40,0);
                    if(r<0){
                        perror("error in sending POSIT\n");
                        exit(EXIT_FAILURE);
                    }
                    printf("[func: lancePartie] Out of bounds, please change the number\n"); 
            }
            int FLAG = 1;

            int i=0;
            int tmpX = x; 
            int toAdd = 0; // nbr de points du joueur a ajouter si ghost croise
            while(checkWall(MAZE_GLOBAL,*WIDTH,tmpX+1,y) == 0 && i<d){
                    // boucle vers le haut de d cases

                    //erreur ici la fonction renvoie une erreur on ne rentre jamais dans le if
                    //printf("x for fantom check is %d\n",tmpX+1);
                    //printf("y for fantom check is %d\n",y);

                    if(checkIfGhostHere(MAZE_GLOBAL,*WIDTH,tmpX+1,y)==1){
                        printf("[func: lancePartie] Fantome attrape ! \n");
                        liste[partie].ghosts -=1;
                        printf("[func: lancePartie] Nouveau nombre de fantomes est : %d \n",liste[partie].ghosts);
                        FLAG = 0;
                        //faire disparaitre le fantome 
                        MAZE_GLOBAL[(tmpX+1)* (*WIDTH) +y] = 8; 
                        // update le nbr de points du client
                        toAdd+=200;
                        diffuserGhostCaught(actuel,tmpX+1,y,toAdd,partie);
                    }
                    tmpX+=1; // update la position temporaire x du joueur
                    i++;
                }
                updatePositionJoueur(actuel,tmpX,y,toAdd,partie); //update position (x,y) and score du joueur

                MAZE_GLOBAL[*WIDTH*tmpX + y] = 3; // modifie sur le plateau la pos du joueur
                MAZE_GLOBAL[x* (*WIDTH)+y] = 8; // valeur par defaut pr dire que la case est libre


                if(FLAG==1){
                    // envoyer le msg MOVE!␣x␣y*** au client si aucun ghost croise avc (x,y) nvlle position

                    char env[100];
                    strcpy(env,"MOVE! ");

                    strcat(env,actuel->x); 
                    strcat(env," ");

                    strcat(env,actuel->y);

                    strcat(env,"***");
                    //printf("test for env (MOVE!) is %s \n",env);
                    int r = send(liste[partie].liste[i].fd_tcp,env,strlen(env),0);
                    if(r<0){
                        perror("error in sending POSIT\n");
                        exit(EXIT_FAILURE);
                    }
                }else{
                    // envoyer le msg MOVEF x y p*** au client avec p le nv nbr de points
                    char env[100];
                    strcpy(env,"MOVE! ");
                    char p1[10];
                    sprintf(p1, "%s",actuel->x);
                    strcat(env,p1);

                    strcat(env," ");

                    char p2[10];
                    sprintf(p2, "%s",actuel->y);
                    strcat(env,p2);

                    strcat(env," ");

                    char *po = malloc(sizeof(char)*10);
                    sprintf(po,"%u", actuel->points);
                    strcat(env,po); 


                    strcat(env,"***");
                    //printf("test for env (MOVE avec points!) is %s \n",env);
                    int r = send(liste[partie].liste[i].fd_tcp,env,strlen(env),0);
                    if(r<0){
                        perror("error in sending POSIT\n");
                        exit(EXIT_FAILURE);
                    }

                }      
            }
            
            // [LEMOV␣d***]

            if(strncmp("LEMOV", getMsg, 5) == 0){
                //printf("actually in LEMOV\n");
            //recup la distance d
                memmove(distance,&getMsg[6],1);
                memmove(distance+1,&getMsg[7],1);
                memmove(distance+2,&getMsg[8],1);

            Joueur *actuel = getJoueur(idJoueur);
            //printf("joueur actuel is %s\n",actuel->pseudo);
            int d = atoi(distance);
            //printf("distance is %d\n",d);
            int x = atoi(actuel->x); //ERREUR
            //printf("x (joueur) in char is %s\n",actuel->x);
            //printf("x (joueur) is %d\n",x);

            int y = atoi(actuel->y);
            //printf("y (joueur) in char is %s\n",actuel->y);
            //printf("y (joueur) is %d\n",y);

            //check for potential out of bounds
            if(y-d <0){
                    int r = send(liste[partie].liste[i].fd_tcp,"out of bounds, please change the number\n",40,0);
                    if(r<0){
                        perror("error in sending POSIT\n");
                        exit(EXIT_FAILURE);
                    }
                    printf("[func: lancePartie] Out of bounds, please change the number\n"); 
            }
            int FLAG = 1;

            int i=0;
            int tmpY = y; 
            int toAdd = 0; // nbr de points du joueur a ajouter si ghost croise
            while(checkWall(MAZE_GLOBAL,*WIDTH ,x,tmpY-1) == 0 && i<d){
                    // boucle vers le haut de d cases

                    //printf("x for fantom check is %d\n",x);
                    //printf("y for fantom check is %d\n",tmpY-1);

                    if(checkIfGhostHere(MAZE_GLOBAL,*WIDTH,x,tmpY-1)==1){
                        printf("[func: lancePartie] Fantome attrape ! \n");
                        liste[partie].ghosts -=1;
                        printf("[func: lancePartie] Nouveau nombre de fantomes est : %d \n",liste[partie].ghosts);
                        FLAG = 0;
                        //faire disparaitre le fantome 
                        MAZE_GLOBAL[x* (*WIDTH) +(tmpY-1)] = 8; 
                        // update le nbr de points du client
                        toAdd+=200;
                        diffuserGhostCaught(actuel,x,tmpY-1,toAdd,partie);
                    }
                    tmpY-=1; // update la position temporaire x du joueur
                    i++;
                }
                updatePositionJoueur(actuel,x,tmpY,toAdd,partie); //update position (x,y) and score du joueur

                MAZE_GLOBAL[*WIDTH *x + tmpY] = 3; // modifie sur le plateau la pos du joueur
                MAZE_GLOBAL[x* (*WIDTH) +y] = 8; // valeur par defaut pr dire que la case est libre


                if(FLAG==1){
                    // envoyer le msg MOVE!␣x␣y*** au client si aucun ghost croise avc (x,y) nvlle position

                    char env[100];
                    strcpy(env,"MOVE! ");

                    strcat(env,actuel->x); 
                    strcat(env," ");

                    strcat(env,actuel->y);

                    strcat(env,"***");
                    //printf("test for env (MOVE!) is %s \n",env);
                    int r = send(liste[partie].liste[i].fd_tcp,env,strlen(env),0);
                    if(r<0){
                        perror("error in sending POSIT\n");
                        exit(EXIT_FAILURE);
                    }
                }else{
                    // envoyer le msg MOVEF x y p*** au client avec p le nv nbr de points
                    char env[100];
                    strcpy(env,"MOVE! ");
                    char p1[10];
                    sprintf(p1, "%s",actuel->x);
                    strcat(env,p1);

                    strcat(env," ");

                    char p2[10];
                    sprintf(p2, "%s",actuel->y);
                    strcat(env,p2);

                    strcat(env," ");

                    char *po = malloc(sizeof(char)*10);
                    sprintf(po,"%u", actuel->points);
                    strcat(env,po); 


                    strcat(env,"***");
                    //printf("test for env (MOVE avec points!) is %s \n",env);
                    int r = send(liste[partie].liste[i].fd_tcp,env,strlen(env),0);
                    if(r<0){
                        perror("error in sending POSIT\n");
                        exit(EXIT_FAILURE);
                    }

                }
            }       
            
            // [RIMOV␣d***]

            if(strncmp("RIMOV", getMsg, 5) == 0){
                //printf("actually in RIMOV\n");
            //recup la distance d
                memmove(distance,&getMsg[6],1);
                memmove(distance+1,&getMsg[7],1);
                memmove(distance+2,&getMsg[8],1);

            Joueur *actuel = getJoueur(idJoueur);
            //printf("joueur actuel is %s\n",actuel->pseudo);
            int d = atoi(distance);
            //printf("distance is %d\n",d);
            int x = atoi(actuel->x); //ERREUR
            //printf("x (joueur) in char is %s\n",actuel->x);
            //printf("x (joueur) is %d\n",x);

            int y = atoi(actuel->y);
            //printf("y (joueur) in char is %s\n",actuel->y);
            //printf("y (joueur) is %d\n",y);

            //check for potential out of bounds
            if(y+d <0){
                    int r = send(liste[partie].liste[i].fd_tcp,"out of bounds, please change the number\n",40,0);
                    if(r<0){
                        perror("error in sending POSIT\n");
                        exit(EXIT_FAILURE);
                    }
                    printf("[func: lancePartie] Out of bounds, please change the number\n"); 
            }
            int FLAG = 1;

            int i=0;
            int tmpY = y; 
            int toAdd = 0; // nbr de points du joueur a ajouter si ghost croise
            while(checkWall(MAZE_GLOBAL,*WIDTH,x,tmpY+1) == 0 && i<d){
                    // boucle vers le haut de d cases
                    //printf("x for fantom check is %d\n",x);
                    //printf("y for fantom check is %d\n",tmpY+1);

                    if(checkIfGhostHere(MAZE_GLOBAL,*WIDTH,x,tmpY+1)==1){
                        printf("[func: lancePartie] Fantome attrape ! \n");
                        liste[partie].ghosts -=1;
                        printf("[func: lancePartie] Nouveau nombre de fantomes est : %d \n",liste[partie].ghosts);
                        FLAG = 0;
                        MAZE_GLOBAL[x* (*WIDTH) +(tmpY+1)] = 8; 
                        // update le nbr de points du client
                        toAdd+=200;
                        diffuserGhostCaught(actuel,x,tmpY+1,toAdd,partie);
                    }
                    tmpY+=1; // update la position temporaire x du joueur
                    i++;
                }
                updatePositionJoueur(actuel,x,tmpY,toAdd,partie); //update position (x,y) and score du joueur

                MAZE_GLOBAL[*WIDTH *x + tmpY] = 3; // modifie sur le plateau la pos du joueur
                MAZE_GLOBAL[x* (*WIDTH) +y] = 8; // valeur par defaut pr dire que la case est libre

                if(FLAG==1){
                    // envoyer le msg MOVE!␣x␣y*** au client si aucun ghost croise avc (x,y) nvlle position

                    char env[100];
                    strcpy(env,"MOVE! ");

                    strcat(env,actuel->x); 
                    strcat(env," ");

                    strcat(env,actuel->y);

                    strcat(env,"***");
                    //printf("test for env (MOVE!) is %s \n",env);
                    int r = send(liste[partie].liste[i].fd_tcp,env,strlen(env),0);
                    if(r<0){
                        perror("error in sending POSIT\n");
                        exit(EXIT_FAILURE);
                    }
                }else{
                    // envoyer le msg MOVEF x y p*** au client avec p le nv nbr de points
                    char env[100];
                    strcpy(env,"MOVE! ");
                    char p1[10];
                    sprintf(p1, "%s",actuel->x);
                    strcat(env,p1);

                    strcat(env," ");

                    char p2[10];
                    sprintf(p2, "%s",actuel->y);
                    strcat(env,p2);

                    strcat(env," ");

                    char *po = malloc(sizeof(char)*10);
                    sprintf(po,"%u", actuel->points);
                    strcat(env,po); 


                    strcat(env,"***");
                    //printf("test for env (MOVE avec points!) is %s \n",env);
                    int r = send(liste[partie].liste[i].fd_tcp,env,strlen(env),0);
                    if(r<0){
                        perror("error in sending POSIT\n");
                        exit(EXIT_FAILURE);
                    }

                }
            }     
            // [IQUIT***]
            if(strcmp(getMsg,"IQUIT***\n")==0){
                printf("[func: lancePartie] Inside IQUIT\n");
                int res = desinscription(idJoueur); //shouldnt be 200 if successful call
                // todo actualiser le board 
                int r = send(liste[partie].liste[i].fd_tcp,"GOBYE***",8,0);
                if(r<0){
                    perror("error in sending GOBYE\n");
                    exit(EXIT_FAILURE);
                }
                close(liste[partie].liste[i].fd_tcp); // todo :  close le fd du client 
            }

            if(strcmp("GLIS?***",getMsg)==0){
                // ENVOYER GLIS
                char *env = malloc(sizeof(char)*100);
                strcpy(env,"GLIS! ");
                char *su1 = malloc(sizeof(char)*10);
                sprintf(su1, "%hu",liste[partie].nb_joueur); 
                strcat(env,su1);
                strcat(env,"***");
                int r = send(liste[partie].liste[i].fd_tcp,env,strlen(env),0);
                if(r<0){
                    perror("error in sending GLIS\n");
                    exit(EXIT_FAILURE);
                }
                // ENVOYER  [GPLYR␣id␣x␣y␣p***]
                for(int i=0;i<liste[partie].nb_joueur;i++){
                    char *env1 = malloc(sizeof(char)*100);
                    strcpy(env1,"GPLYR ");
                    strcat(env1,liste[partie].liste[i].pseudo);

                    char *k1 = malloc(sizeof(char)*10);
                    sprintf(k1, "%s",liste[partie].liste[i].x); 
                    strcat(env1," ");
                    strcat(env1,k1);

                    char *k2 = malloc(sizeof(char)*10);
                    sprintf(k2, "%s",liste[partie].liste[i].y); 
                    strcat(env1," ");
                    strcat(env1,k2);

                    char *k3 = malloc(sizeof(char)*10);
                    sprintf(k3, "%d", liste[partie].liste[i].points);
                    strcat(env1," ");
                    strcat(env1,k3);
                    strcat(env1,"***");

                    int r = send(liste[partie].liste[i].fd_tcp,env1,strlen(env1),0);
                    if(r<0){
                        perror("error in sending GPLYR\n");
                        exit(EXIT_FAILURE);
                    }
                }

            }
            if(strncmp("MALL", getMsg, 4)==0){
                // get message mess
                char *mess = malloc(sizeof(char)*200);
                int i=6;
                int j=0;

                while(getMsg[i]!='*'){
                    memmove(mess+j,&getMsg[i],1);
                    i++;
                    j++;
                }

                mess[j]='\0';

                multiDiffuseToPlayers(mess,idJoueur,partie); // multidiffuse le message aux autres users : [MESSA␣id␣mess+++] avec id le joueur qui a envoye le message
                int r = send(liste[partie].liste[i].fd_tcp,"MALL!***",8,0);
                if(r<0){
                    perror("error in sending MALL\n");
                    exit(EXIT_FAILURE);
                }
            }
            if(strncmp("SEND", getMsg, 4) == 0){
                // get message mess from [SEND?␣id␣mess***]
                printf("[func: lancePartie] inside SEND\n");
                char *mess = malloc(sizeof(char)*200);            
                int i=15;
                int j=0;
                while(getMsg[i]!='*'){
                    memmove(mess+j,&getMsg[i],1);
                    i++;
                    j++;
                }
                mess[j]='\0';  
                printf("[func: lancePartie] FROM message is  : %s \n", mess);

                char *idM = malloc(sizeof(char)*200);            
                int k=6;
                int l=0;
                while(k!=14){
                    memmove(idM+l,&getMsg[k],1);
                    k++;
                    l++;
                }
                idM[l]='\0';    
                
                // => send le message MESSP␣id2␣mess+++ en UDP vers id avec id2 = le joueur qui a envoye le message
                char *pseudoFrom = idJoueur; 
                //printf("[func: lancePartie] FROM : %s \n", pseudoFrom);

                char *pseudoTo = idM; 
                //printf("[func: lancePartie] TO : %s \n", pseudoTo);

                int res = envoiUDPToPlayer(mess,pseudoFrom,pseudoTo,partie);
                //printf("[func: lancePartie] IS RES REACHED ?? : %d \n", res);

                if(res==0){
                    int r = send(liste[partie].liste[i].fd_tcp,"NSEND!***",8,0);
                    if(r<0){
                        perror("error in sending NSEND\n");
                        exit(EXIT_FAILURE);
                    }
                }else{
                    int r = send(liste[partie].liste[i].fd_tcp,"SEND!***",8,0);
                    if(r<0){
                        perror("error in sending SEND!\n");
                        exit(EXIT_FAILURE);
                    }
                }
            }

        }
        //int r = recv(fd,getMsg,100,0);
        /*if(r<0){
            perror("error in receiving Msg for partie interaction\n");
            exit(EXIT_FAILURE);
        }*/

        

    }
    liste[partie].started = 0; // la partie est finie 

    // envoi du message [ENDGA␣id␣p+++]
    Joueur *j = getGagnant(partie);
    printf("[func: lancePartie]  Partie Finie, le gagnant est : %s !\n", j->pseudo);
    multiDiffuseScoreFinal(partie,j);

    //waitClient(fd);

}

/* PARTIE FUNCTIONS */

void waitPartieToStart(int fd,char *id_actual_player){
    // client en attente soit la partie commence ou il doit attendre que les autres joueurs start
    printf("[func: waitPartieToStart] Start lance, joueur en attente... \n");
    char *RECEIVE_C = malloc(sizeof(char)*100);
    int rc = 0;
    
    //probleme erreur de descripteurs fd != fd_user donc il ne recoit pas le WELCO
    while(NOTSTARTED==1){
        if(fd>=0){
            rc = recv(fd,RECEIVE_C,100*sizeof(char),0);
            printf("[func: waitPartieToStart] Message recu client en attente de game %s\n",RECEIVE_C);
            if(rc<0){
                printf("error in receiving structure\n");
                exit(EXIT_FAILURE);
            }
            RECEIVE_C[rc]='\0';
            break;
        }
    }
    printf("[func: waitPartieToStart] La partie peut start \n");
    uint8_t idPartie = getIdPartieFromJoueur(id_actual_player);
    lancePartie(idPartie,fd,id_actual_player);
}


void main_boucle(int fd){
    //valeur 4 par defaut
    int resCreatePartie=-1;
    int resJoinPartie=-1;
    int acceptedByServer = 4;

    int STARTED = 0;
    uint8_t PARTIE_REJOINTE;
    char *id_actual_player =  malloc(sizeof(char)*8);
   

    while(STARTED == 0 || isRegistered(id_actual_player) == 0){
        // MAIN ACTION TOUJOURS POSSIBLE := le joueur peut envoyer START*** qd il est pret
        char buff[100];
        // recevoir n'importe quelle option 
        int r = recv(fd,buff,100,0);
        buff[r] = '\0';
        if(r<0){
            perror("error in receiving option\n");
            exit(EXIT_FAILURE);
        }

        if(strncmp("REGIS",buff,5)==0 || strncmp("NEWPL",buff,5)==0){
            // HANDLE [REGIS]  ou [NEW]
            if (strncmp("REGIS",buff,5)==0){
                //enroll a player in existed game
                uint8_t m = atoi(&buff[20]); // get id partie from client
                PARTIE_REJOINTE = m;
                // add a todo check si la partie a deja commence erreur
                if(liste[PARTIE_REJOINTE].started == 1){
                    resJoinPartie = 0;
                }else{
                    char *idTEST = malloc(sizeof(char)*8);
                    char *port_udp = malloc(sizeof(char)*4);


                    //printf("%s\n",&mess[14]);
                    memmove(idTEST,&buff[6],8);
                    //printf("id is from REGIS %s \n",idTEST);
                    memmove(port_udp,&buff[15],4);
                    memmove(id_actual_player,idTEST,8);
                    //printf("id actualplayer in REGIS if %s\n",id_actual_player);

                    saveClient(fd,idTEST);
                    Joueur *j = malloc(sizeof(Joueur));

                    j = getJoueur(idTEST);
                    printf("[func: main_boucle] Id joueur qui veut rejoindre est : %s\n",j->pseudo);
            
                    j->fd_udp = atoi(port_udp);
                    printf("[func: main_boucle] Port UDP du joueur qui veut rejoindre est : %d\n",j->fd_udp); 

                    //j->estDejaInscrit = 1;
                    updateJoueurUDP(j); 

                    resJoinPartie = join_Partie(j,m);

                    //free(idTEST);
                    //free(port_udp);
                    //free(j);
                }
                
            }
            
            if(strncmp("NEWPL",buff,5)==0){
                //creer une nvlle partie
                char *idTEST2 = malloc(sizeof(char)*8); 
                char *port_udp2 = malloc(sizeof(char)*4);

                memmove(idTEST2,&buff[6],8);
                printf("[func: main_boucle] Actual player id after NEWPL is : %s\n",idTEST2);
                memmove(port_udp2,&buff[15],4);
                memmove(id_actual_player,idTEST2,8); 
            
                saveClient(fd,idTEST2);

                Joueur *j = malloc(sizeof(Joueur));
                j = getJoueur(idTEST2);
                if(j==NULL){
                    printf("[func: main_boucle] Joueur (from getJOUEUR) is NULL\n");
                }
                //printf("%s\n",j->pseudo);

                j->fd_udp = atoi(port_udp2);
                //printf("%d\n",j->fd_udp);

                // update le joueur j avec son port UDP
                updateJoueurUDP(j);

                resCreatePartie = create_Partie(compteurID,j);
                //printf(" [func: main_boucle] rescreatepartie is %d\n",resCreatePartie);
                
            }

             // check pr savoir si user a ete ajoute ou non a une partie avec success
            if(resCreatePartie==0 || resJoinPartie==0){
                int r = send(fd,"REGNO***",8,0);
                if(r<0){
                    perror("error in sending REGNO\n");
                    exit(EXIT_FAILURE);
                }else{
                    acceptedByServer = 0;
                }
            }else{
            if(resCreatePartie==1){
                uint8_t m = nb_partie;
                char *toSend = malloc(sizeof(char)*20);
                // envoi du message [REGOK m***] au client
                strcpy(toSend, "REGOK ");
                char *str1 = malloc(sizeof(char)*10);
                sprintf(str1, "%d", compteurID-1);
                //printf("id inside str1 is %s\n",str1);
                strcat(toSend, str1); 
                strcat(toSend, "***\0");
                //printf("whole toSend chain is %s\n",toSend);


                int r = send(fd,toSend,strlen(toSend),0);
                if(r<0){
                    perror("error in sending REGOK\n");
                    exit(EXIT_FAILURE);
                }else{
                    acceptedByServer = 1;
                }
                
            }
            if(resJoinPartie==1){
                //get la partie que l'utlisiteur a rejoint
                char *toSend = malloc(sizeof(char)*20);
                // envoi du message [REGOK m***] au client
                strcpy(toSend, "REGOK ");
                char *str = malloc(sizeof(char)*10);
                sprintf(str, "%hhu", PARTIE_REJOINTE);
                //printf("id inside str is %s\n",str);
                strcat(toSend, str); 
                strcat(toSend, "***\0");
                //printf("whole toSend chain (joinPartie after regis) is %s\n",toSend);
                int r = send(fd,toSend,strlen(toSend),0);
                if(r<0){
                    perror("error in sending REGOK(join_partie)\n");
                    exit(EXIT_FAILURE);
                }else{
                    acceptedByServer = 1;
                }
            }
        }

        }
        
        if(strncmp("MALL", buff, 4) == 0 || strncmp("SEND", buff, 4) == 0){
            int r = send(fd,"GOBYE***",8,0);
            if(r<0){
                perror("erreur sending MALL/SEND error message\n");
                exit(EXIT_FAILURE);
            }
            close(fd); // todo: close le descripteur du client en question
        }


        
        // UNREG

        if(strcmp("UNREG***",buff)==0){
            printf("[func: main_boucle] inside UNREG \n");
            //printf("actual player to unreg is %s\n",id_actual_player);

            uint8_t a = desinscription(id_actual_player);
            if(a==200){ // on suppose qu'il ya moins de 200 joueurs 
                // envoyer DUNNO
                int r = send(fd,"DUNNO***",8,0);
                if(r<0){
                    perror("error in sending DUNNO\n");
                    exit(EXIT_FAILURE);
                }
            }else{
                // envoyer UNROK a***
                if(resCreatePartie!=-1) resCreatePartie = -1;
                char repUn[20];

                strcpy(repUn,"UNROK ");
                char str[10];
                sprintf(str, "%d", a);
                strcat(repUn,str);
                strcat(repUn,"***\0");
                int r = send(fd,repUn,strlen(repUn),0);
                if(r<0){
                    perror("erreur UNROK\n");
                    exit(EXIT_FAILURE);
                }
            }
        }

        // GET SIZE OF MAZE

        if(strncmp("SIZE", buff, 4) == 0){
            printf("[func: main_boucle] inside SIZE\n");
            uint8_t num_partie;
            char *tmp = malloc(sizeof(char)*10);
            int i=6;
            int j=0;

            while(buff[i]!='*'){
                memmove(tmp+j,&buff[i],1);
                i++;
                j++;
            }
            //printf("%s\n",tmp);
            num_partie = atoi(tmp);
            //printf("numpartie is %d\n", num_partie);
            getSizeLaby(num_partie,fd);
        }
        
        // GET PLAYERS LIST

        if(strncmp("LIST", buff, 4) == 0){
            printf("[func: main_boucle] inside LIST\n");

            uint8_t num_partie;
            char *tmp = malloc(sizeof(char)*10);
            int i=6;
            int j=0;
            while(buff[i]!='*'){
                memmove(tmp+j,&buff[i],1);
                i++;
                j++;
            }
            num_partie = atoi(tmp);
            getListePlayers(num_partie,fd);
        }
        
        // GET PARTIES THAT DIDNT STARTED YET BUT WITH PLAYERS ENROLLED

        if(strcmp(buff,"GAME?***")==0){
            // DONE
            getPartiesSpecials(fd);
        }

        if(strcmp("START***",buff)==0){
            printf("[func: main_boucle] inside START\n");
            STARTED = 1;

            //TODO : check si le joueur est inscrit
            for(int i=0;i<id_liste_joueur;i++){
                if(strcmp(liste_clients[i].pseudo,id_actual_player)==0){
                    liste_clients[i].hasCalledStart = 1;
                    //printf("nbjoueurs PRETS is %d\n",liste[getIdPartieFromJoueur(id_actual_player)].nb_joueur_pret);
                    pthread_mutex_lock(&mutex);
                    liste[getIdPartieFromJoueur(id_actual_player)].nb_joueur_pret+=1;
                    pthread_mutex_unlock(&mutex);
                    break;
                }
            }
            int res = probablyLancePartie(id_actual_player,fd);
            if(res==1) {
                NOTSTARTED = 0;
                printf("[func: main_boucle] La partie peut commencer car tous les joueurs sont prets !\n");
                uint8_t idPartie = getIdPartieFromJoueur(id_actual_player);
                setPartieWELCOME(idPartie);
                
                lancePartie(idPartie,fd,id_actual_player);
            }
            else{
                waitPartieToStart(fd,id_actual_player);
            }

        }
        
    }

    free(id_actual_player);
    
}

void *comm(void *arg){
    char buff[15];
    int fd = *(int *) arg; // socket from caller

    // MESSAGE DE DEBUT D'INTERACTION AVEC LE CLIENT
    char games[11];
    // envoi du message [GAMES␣n***] au client
    memmove(games, "GAMES ", 6);

    //parties non commencees et avec joueurs inscrits
    uint8_t n = getPartiesNotYetStarted();
    char str[10];
    sprintf(str, "%d", n);
    //printf("%s",str);
    
    memmove(games+6,str,1); 

    memmove(games+7, "***\0",3);
    //printf("%s\n",games);

    int sd = send(fd, games, strlen(games), 0);
    if(sd<0){
        perror("error in sending welcoming msg : start of the game\n");
        exit(EXIT_FAILURE);
    }

    if(nb_partie==0){
        //printf("yes its 0\n");
        main_boucle(fd);
    }else{
        //printf("no its not 0\n");
        // envoi de [OGAME␣m␣s***] : s -> nombre de joueurs par partie au client et m = n -> nbr de parties
        afficheGame(fd);
        main_boucle(fd);
    }
   
    close(fd);
    return NULL;
}

int main(int argc, char *argv[]){
    srand(time(NULL));

    int sock=socket(PF_INET,SOCK_STREAM,0);
    if (sock == -1){
        perror("problem socket");
        exit(1);
    }
    if (argc < 2){
        printf("[func: main] Entrer numero de port en argument\n");
        exit(1);
    }
    struct sockaddr_in sockaddress;
    sockaddress.sin_family=AF_INET;
    sockaddress.sin_port=htons(atoi(argv[1]));
    sockaddress.sin_addr.s_addr=htonl(INADDR_ANY);
    int r=bind(sock,(struct sockaddr *)&sockaddress,sizeof(struct sockaddr_in));
    if(r==0){
        r=listen(sock,0);
        if (r == 0){
            struct sockaddr_in caller;
            socklen_t size=sizeof(caller);
            while(1){
                int *sock2=(int *)malloc(sizeof(int));
                *sock2=accept(sock,(struct sockaddr *)&caller,&size);
                if(sock2>=0){
                    printf("[func: main] Sock accepted\n");
                    pthread_t th;
                    pthread_create(&th,NULL,comm, sock2);
                    //pthread_join(th, NULL);
                }else{
                    perror("accept");
                }
            }
        }else{
            perror("problem listen");
        }
    }else{
        perror("problem bind ");
    }
    pthread_exit(NULL);
    return 0;
}
