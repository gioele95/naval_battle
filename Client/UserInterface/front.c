#include "front.h"
#include <stdio.h>

void printMap(CellStatus *b){
	int i=0;
	int riga=0;
	printf("  A B C D E F");
	for(;i<36;i++){
		if((i%6)==0)
			printf("\n%d ",++riga);
		switch(b[i]){
			case OCCUPIED:
				printf("o ");
				break;
			case MISSED: 
				printf("~ ");
				break;			
			case HIT:
				printf("x ");
				break;
			case EMPTY:
				printf("? ");
				break;
		}
	}
	printf("\n");
	printf("\n");
}
void resetGrid(CellStatus*b){
	int i;
	for ( i = 0; i < 36; ++i)
	{
		b[i]=EMPTY;
	}
}