# Projet Reseau

Ce projet est un jeu à plusieurs reprenant le concept de Pac-Man mais avec des changements non négligeables.

## Démarrage du jeu

Tout d'abord, lorsque vous lancez le client, vous allez recevoir le nombre de parties disponibles suivi de la liste des parties disponibles.

Ensuite vous avez la possibilité de :

1. Rejoindre une partie avec l'opcode [REGIS id_client port_udp num_partie*** ]
   => reponse du serveur :  [REGOK␣m***] avec m le num_partie sinon erreur avec  [REGNO***]

2. Créer une partie avec l'opcode [NEWPL id_client port_udp***]
   => reponse du serveur :  [REGOK␣m***] avec m le num_partie sinon erreur avec  [REGNO***]

3. Demander la taille du labyrinthe d'une partie avec l'opcode [SIZE? num_partie***]
   => reponse du serveur :  [SIZE!␣m␣h␣w***] avec m le num_partie, h la hauteur et w la largeur sinon erreur avec  [DUNNO***]

4. La liste des joueurs d'une partie avec l'opcode [LIST? num_partie***]
   => reponse du serveur :  [LIST!␣m␣s***] avec m le num_partie, s le nombre de joueurs suivi de s opcodes [PLAYR␣id***] avec id le nom du joueur sinon erreur avec  [DUNNO***]

5. La liste des parties non commencées avec joueurs inscrits avec l'opcode [GAME?***]
   => reponse du serveur :  [GAMES␣n***] avec n le nombre de parties suivi de n opcodes [OGAME␣m␣s***] avec m le numero de la partie et s le nombre de joueurs dans cette partie
6. Se desinscrire de la partie à laquelle il est inscrit avec l'opcode [UNREG***]
   => reponse du serveur :  [UNROK␣m***] avec m le num_partie sinon erreur avec  [DUNNO***]


## Règles du jeu 

Lorsque le client est entrée dans une partie et qu'il a envoyé l'opcode [START***] pour commencer, il est en attente des autres joueurs et lorsque tous les joueurs présents dans une partie ont aussi envoyé l'opcode [START***], 
alors la partie comence et chaque joueur recoit l'opcode [WELCO num_partie width_partie length_partie number_of_ghosts addresse_multidiffusion port_multidiffusion***]
Chacun recoit aussi sa position initiale sur le plateau avec l'opcode [POSIT id_joueur x_pos y_pos***]


Chaque joueur peut alors demander un mouvelent sur le plateau du labyrinthe :

[UPMOV n***] [pour bouger vers le haut de n cases]
[DOMOV n***] [pour bouger vers le bas de n cases]
[LEMOV n***] [pour bouger vers la gauche de n cases]
[RIMOV n***] [pour bouger vers la droite de n cases]

Le but du jeu est d'attraper le plus de fantomes possible.
Chaque joueur gagne un certain nombre de points apres chaque capture, et cela eclenche l'envoi d'un message multidiffusé à tous les autres joueurs de la partie.
Le jeu s'arrete lorsqu'il n'y a plus de joueurs ou bien lorsque tous les fantomes sont attrapés.

## Pour faire fonctionner le projet


Il vous faut un compilateur java et c installé

```cd reseau```

``` make ```
1. Ouvrir un terminal dans lequel le serveur sera lancé
   
```./serveur <port_tcp>```

2. Ouvrir p clients dans p terminaux différents afin e faire jouer les joueurs entre eux

``` java Client localhost <port_tcp> ```
