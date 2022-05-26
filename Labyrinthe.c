#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Display the maze. */
void ShowMaze(const char *maze, int width, int height) {
   int x, y;
   for(y = 0; y < height; y++) {
      for(x = 0; x < width; x++) {
         switch(maze[y * width + x]) {
         case 1:  printf("*");  break; //mur trouve
         case 2:  printf("<>");  break;
         case 3:  printf("P");  break; // JOUEUR trouve
         case 5:  printf("F");  break; // ghost trouve
         default: printf(" ");  break;
         }
      }
      printf("\n");
   }
}

/*  Carve the maze starting at x, y. */
void CarveMaze(char *maze, int width, int height, int x, int y) {

   int x1, y1;
   int x2, y2;
   int dx, dy;
   int dir, count;

   dir = rand() % 4;
   count = 0;
   while(count < 4) {
      dx = 0; dy = 0;
      switch(dir) {
      case 0:  dx = 1;  break;
      case 1:  dy = 1;  break;
      case 2:  dx = -1; break;
      default: dy = -1; break;
      }
      x1 = x + dx;
      y1 = y + dy;
      x2 = x1 + dx;
      y2 = y1 + dy;
      if(   x2 > 0 && x2 < width && y2 > 0 && y2 < height
         && maze[y1 * width + x1] == 1 && maze[y2 * width + x2] == 1) {
         maze[y1 * width + x1] = 0;
         maze[y2 * width + x2] = 0;
         x = x2; y = y2;
         dir = rand() % 4;
         count = 0;
      } else {
         dir = (dir + 1) % 4;
         count += 1;
      }
   }

}

/* Generate maze in matrix maze with size width, height and init walls on the board*/
void GenerateMaze(char *maze, int width, int height) {

   int x, y;

   /* Initialize the maze. */
   for(x = 0; x < width * height; x++) {
      maze[x] = 1;
   }
   maze[1 * width + 1] = 0;

   /* Seed the random number generator. */
   srand(time(0));

   /* Carve the maze. */
   for(y = 1; y < height; y += 2) {
      for(x = 1; x < width; x += 2) {
         CarveMaze(maze, width, height, x, y);
      }
   }

   /* Set up the entry and exit. */
   maze[0 * width + 1] = 0;
   maze[(height - 1) * width + (width - 2)] = 0;

}

/* Solve the maze. */
void SolveMaze(char *maze, int width, int height) {

   int dir, count;
   int x, y;
   int dx, dy;
   int forward;

   /* Remove the entry and exit. */
   maze[0 * width + 1] = 1;
   maze[(height - 1) * width + (width - 2)] = 1;

   forward = 1;
   dir = 0;
   count = 0;
   x = 1;
   y = 1;
   while(x != width - 2 || y != height - 2) {
      dx = 0; dy = 0;
      switch(dir) {
      case 0:  dx = 1;  break;
      case 1:  dy = 1;  break;
      case 2:  dx = -1; break;
      default: dy = -1; break;
      }
      if(   (forward  && maze[(y + dy) * width + (x + dx)] == 0)
         || (!forward && maze[(y + dy) * width + (x + dx)] == 2)) {
         maze[y * width + x] = forward ? 2 : 3;
         x += dx;
         y += dy;
         forward = 1;
         count = 0;
         dir = 0;
      } else {
         dir = (dir + 1) % 4;
         count += 1;
         if(count > 3) {
            forward = 0;
            count = 0;
         }
      }
   }

   /* Replace the entry and exit. */
   maze[(height - 2) * width + (width - 2)] = 2;
   maze[(height - 1) * width + (width - 2)] = 2;

}


// 1 if wall, 0 otherwise
int checkWall(char *maze,int width,int x,int y){
   if(maze[width*x+y]==1){
      return 1;
   }
   return 0;
}

int checkIfGhostHere(char *maze, int width,int i, int j){
   // if maze[i][j] == fantome (5 symbole du fantome) => return 1 else return 0
   if(maze[i*width+j]==5){
      return 1;
   }else{
      return 0;
   }
}

char *moveGhostNotCaught(char *maze,int width,int height){
   char *pos = malloc(sizeof(char)*2);
   for(int i=0;i<height;i++){
      for(int j=0;j<width;j++){
         // on pourrait recup une case de fantome aleatoire avec getRandomGhostCase 
         //car ici on prend tjrs la premiere case occupee par un fantome cest pas tres original
         if(maze[width*i+j]==5){
            // bouger le ghost sur une case libre sans mur et fantome
            int x=0;
            int y=0;

            while(checkWall(maze,width,x,y)==1 || checkIfGhostHere(maze,width,x,y)==1){
               x = getRandomNumber(0,height-1); //get a random x in the board
               y = getRandomNumber(0,width-1);; //get a random y in the board
            }
            maze[width*i+j]=0; // la case devient libre car le fantome se deplace
            maze[width*x+y]=5; // le fantome atterit sur la nouvelle case (x,y) qui est libre et mtn occupee par le fantome
            pos[0] = x;
            pos[1] = y; 
            break;
         }
      }
   }
   return pos;
}

int getRandomNumber(int lower,int upper){
   int num = (rand() % (upper - lower + 1)) + lower;
   return num;
}

void initGhosts(char *maze,int width, int height,int ghostsToAdd){ // => placer les fantomes sur le labyrinthe
   
   while(ghostsToAdd > 0){ //PROBLEME +-1 ghost
      
      int x=0;
      int y=0;

      while(checkWall(maze,width,x,y)==1){
         x = getRandomNumber(0,height-1); //get a random x in the board
         y = getRandomNumber(0,width-1);; //get a random y in the board
      }
      maze[x *width +y] = 5; // ajout du fantome avec symbole 5
      ghostsToAdd--;
   }
}

/*
int main(int argc,char *argv[]) {

   int width, height;
   char *maze;

   printf("Maze by Charly Sonneville\n");
   if(argc != 3 && argc != 4) {
      printf("usage: maze <width> <height> [s]\n");
      exit(EXIT_FAILURE);
   }

   // Get and validate the size. //
   width = atoi(argv[1]) * 2 + 3;
   height = atoi(argv[2]) * 2 + 3;
   if(width < 7 || height < 7) {
      printf("error: illegal maze size\n");
      exit(EXIT_FAILURE);
   }
   if(argc == 4 && argv[3][0] != 's') {
      printf("error: invalid argument\n");
      exit(EXIT_FAILURE);
   }

   // Allocate the maze array. //
   maze = (char*)malloc(width * height * sizeof(char));
   if(maze == NULL) {
      printf("error: not enough memory\n");
      exit(EXIT_FAILURE);
   }

   // Generate and display the maze. //
   GenerateMaze(maze, width, height);
   printf("%s\n",maze);
   ShowMaze(maze, width, height);

   // Solve the maze if requested. //
   if(argc == 4) {
      SolveMaze(maze, width, height);
      ShowMaze(maze, width, height);
   }

   // Clean up. //
   free(maze);
   exit(EXIT_SUCCESS);

}*/