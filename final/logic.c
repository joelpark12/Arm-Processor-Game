#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

//Don't forget the continue thing!
int main(){
  int lives = 3;
  int wins = 0;
  int round = 1;
  int pattern[10]; //holds the sequence of numbers
  srand(time(0));
  while(lives && wins != 10){
    int current =  rand() % 4; //random number between 0 and 3
    // 0 = red 1 = blue 2 = green 3 = yellow
    int input[10]; //holds the user inputted sequence of numbers
    pattern[round-1] = current;
    int i;
    int j;
    int k;
    for(i=0; i<round; i++){ //shows pattern to user
      if(pattern[i] == 0){
	printf("red\n");
      }
      else if(pattern[i] == 1){
	printf("blue\n");
      }
      else if(pattern[i] == 2){
	printf("green\n");
      }
      else if(pattern[i] == 3){
	printf("yellow\n");
      }

    }
    int success = 1;
    for(j=0; j<round; j++){ //user inputs their pattern
      int n;
      scanf("%d", &n);
      input[j] = n;
      if(pattern[j] != n){
	success = 0;
	lives --;
	memset(pattern, 0, 10);
	round = 1;
	printf("That's wrong!\n");
	break;
    }
    }
    if(success){
      round++;
      wins++;
    }
    if(wins == 10){
      printf("You win!\n");
      return 0;
    }
    if (lives == 0){
      printf("GAME OVER\n");
      return 0;
    }
     memset(input, 0, 10);
  }
  return 0;
}

