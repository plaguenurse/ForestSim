/*
* A forest simulation based on a concept on reddit's dailyprogrammer
* and reposted on the codegolf forums of stackexchange
* Copyright (C) 2014 Kate Barr
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
//Structs
typedef struct coords
{
	int x;
	int y;
} coords;
typedef struct worldElement
{
	char displayImg;
	int value;
} worldElement;

typedef struct worldMetaElement
{
	worldElement character;
	coords position;
	struct worldMetaElement* next;
} worldMetaElement;

//Constants
const int FOREST_SIZE = 20;
const int TIME_MAX = 4800;
const worldElement BLANK = {'.',0};
const worldElement SAPLING = {',',1};
const worldElement TREE = {'|',1};
const worldElement ELDERTREE = {'%',1};
const worldElement LUMBERJACK = {'$',1};
const worldElement BEAR = {'B',1};
const struct timespec SLEEP = {0, 80000000};

//Functions
void addElement(worldMetaElement**,worldMetaElement*,worldElement,double);
void addSingleElement(worldMetaElement**,worldMetaElement*,worldElement);
void age(worldElement[][FOREST_SIZE],worldElement,worldElement,int);
int collision(coords,worldMetaElement*);
void clearList(worldMetaElement*);
void moveBears(worldMetaElement*, worldMetaElement**,int*);
void moveLumberjacks(worldMetaElement**,worldMetaElement*, worldElement[][FOREST_SIZE], int*);
int notEmpty(worldElement[][FOREST_SIZE]);
int numberOf(worldMetaElement*);
void printBoard(worldElement[][FOREST_SIZE]);
void printMetaElements(worldMetaElement*);
void propagateTrees(worldElement[][FOREST_SIZE],worldElement,int);
coords randomMovement(int,int);
void removeSingleElement(worldMetaElement*);
void seedForest(worldElement [][FOREST_SIZE]);
int totalLumber(worldMetaElement*);

int main(void)
{
	//Initialize variables
	int character,i, bearAttacks = 0, j,amountToAdd;
	worldElement forest[FOREST_SIZE][FOREST_SIZE];
	worldMetaElement* lumberjacks,* bears;
	//Initialize ncurses and seed RNG
	initscr();
	srand(time(NULL));
	keypad(stdscr, TRUE);
	noecho();
	//Color
	start_color();
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	
	//Initialize simulation elements & etcetera
	character = ERR;
	seedForest(forest);
	addElement(&lumberjacks,NULL,LUMBERJACK,.1);
	addElement(&bears,lumberjacks,BEAR,.02);
	
	//Simulation loop
	for(i = 1; i <= TIME_MAX && notEmpty(forest);i++)
	{
		age(forest, SAPLING, TREE, 12);
		age(forest, TREE, ELDERTREE, 120);
		propagateTrees(forest,TREE,10);
		propagateTrees(forest,ELDERTREE,5);
		
		moveLumberjacks(&lumberjacks,bears,forest,&bearAttacks);
		moveBears(bears,&lumberjacks,&bearAttacks);
		if(!lumberjacks)
			addSingleElement(&lumberjacks,bears,LUMBERJACK);
		if(!(i%12))
		{
			if(bearAttacks)
			{
				bearAttacks=0;
				removeSingleElement(bears);
			}
			else
			{
				addSingleElement(&bears,lumberjacks,BEAR);
			}
			if(totalLumber(lumberjacks) > numberOf(lumberjacks))
			{
				amountToAdd = totalLumber(lumberjacks) / numberOf(lumberjacks);
				for(j = 0; j< amountToAdd; j++)
					addSingleElement(&lumberjacks,bears,LUMBERJACK);
			}
			else
			{
				removeSingleElement(lumberjacks);
			}
		}
		
		clear();
		printBoard(forest);
		attron(COLOR_PAIR(2));		
		printMetaElements(lumberjacks);
		attroff(COLOR_PAIR(2));
		attron(COLOR_PAIR(3));
		printMetaElements(bears);
		attroff(COLOR_PAIR(3));
		mvaddch(FOREST_SIZE,FOREST_SIZE,' ');
		refresh();
		
		nanosleep(&SLEEP, NULL);
	}
		
	//End
	getch();
	endwin();
	clearList(lumberjacks);
	clearList(bears);
	return 0;
}

void addElement(worldMetaElement** firstelement, worldMetaElement* check, worldElement type, double percent)
{
	int i;
	coords position;
	worldMetaElement* current;
	*firstelement = malloc(sizeof(worldMetaElement));
	current = *firstelement;
	if(current == NULL)
		exit(1);

	current->character = type;
	current->next = NULL;
	position.x = rand() % FOREST_SIZE;
	position.y = rand() % FOREST_SIZE;
	while(collision(position, *firstelement)|| collision(position, current))
	{
		position.x = rand() % FOREST_SIZE;
		position.y = rand() % FOREST_SIZE;
	}
	current->position.x = rand() % FOREST_SIZE;
	current->position.y = rand() % FOREST_SIZE;
	
	for(i=1; i< FOREST_SIZE * FOREST_SIZE * percent;i++)
	{
		current->next = malloc(sizeof(worldMetaElement));
		current = current->next;
		current->character = type;
		current->next = NULL;
		position.x = rand() % FOREST_SIZE;
		position.y = rand() % FOREST_SIZE;
		while(collision(position, *firstelement)|| collision(position, current))
		{
			position.x = rand() % FOREST_SIZE;
			position.y = rand() % FOREST_SIZE;
		}
		current->position.x = rand() % FOREST_SIZE;
		current->position.y = rand() % FOREST_SIZE;
	}
}

void addSingleElement(worldMetaElement** firstElement,worldMetaElement* secondList,worldElement type)
{
	worldMetaElement* current = *firstElement;
	coords position;
	if(!current)
	{
		*firstElement=malloc(sizeof(worldMetaElement));
		if(!*firstElement)
			exit(1);
		(*firstElement)->next=NULL;
		(*firstElement)->character=type;
		position.x=rand() % FOREST_SIZE;
		position.y=rand() % FOREST_SIZE;
		while(collision(position,secondList))
		{
			position.x=rand() % FOREST_SIZE;
			position.y=rand() % FOREST_SIZE;
		}
		(*firstElement)->position=position;
	}
	else
	{
		while (current->next)
			current = current->next;
		current->next=malloc(sizeof(worldMetaElement));
		current=current->next;
		if(!current)
			exit(1);
		current->next = NULL;
		current->character=type;
		current->position.x=-1;
		current->position.y=-1;
		position.x=rand() % FOREST_SIZE;
		position.y=rand() % FOREST_SIZE;
		while(collision(position,*firstElement) || collision(position,secondList))
		{
			position.x=rand() % FOREST_SIZE;
			position.y=rand() % FOREST_SIZE;
		}
		current->position=position;
	}
}

void age(worldElement forest[][FOREST_SIZE],worldElement start,worldElement end,int time)
{
	int i;
	for(i = 0; i < FOREST_SIZE * FOREST_SIZE; i++)
	{
		if(forest[i/FOREST_SIZE][i%FOREST_SIZE].displayImg == start.displayImg)
		{
			if(forest[i/FOREST_SIZE][i%FOREST_SIZE].value == time)
			{
				forest[i/FOREST_SIZE][i%FOREST_SIZE] = end;
			}
			else
			{
				forest[i/FOREST_SIZE][i%FOREST_SIZE].value++;
			}
		}
	}
}

void clearList(worldMetaElement* link)
{
        worldMetaElement* current = link;
       
        while (current != NULL)
        {
                current = link->next;
                free(link);
                link = current;
        }
}

int collision(coords coordinates,worldMetaElement* linkList)
{
	while(linkList)
	{
		if(linkList->position.x == coordinates.x && linkList->position.y == coordinates.y)
			return 1;
		linkList = linkList->next;
	}
	return 0;
}

void moveBears(worldMetaElement* bears,worldMetaElement** lumberjacks, int* bearAttack)
{
	worldMetaElement* current= bears;
	worldMetaElement*currentLumberjacks;
	worldMetaElement*previousLumberjacks;
	int i,flag = 0;
	coords position;
	while (current)
	{
		for(i = 0;i<5 && !flag;i++)
		{
			currentLumberjacks = *lumberjacks;
			flag = 0;
			position = randomMovement(current->position.x,current->position.y);
			while(collision(position,bears))
			{
				position = randomMovement(current->position.x,current->position.y);
			}
			current->position=position;
			if(collision(position,*lumberjacks))
			{
				if((current->position.x == (*lumberjacks)->position.x) && (current->position.y == (*lumberjacks)->position.y))
				{
					*lumberjacks = currentLumberjacks->next;
					free(currentLumberjacks);
					currentLumberjacks = *lumberjacks;
				}
				else
				{
					previousLumberjacks = currentLumberjacks;
					currentLumberjacks = currentLumberjacks->next;
					while((current->position.x != currentLumberjacks->position.x) && (current->position.y != currentLumberjacks->position.y))
						currentLumberjacks = currentLumberjacks->next;
					previousLumberjacks->next = currentLumberjacks->next;
					free(currentLumberjacks);
					currentLumberjacks = previousLumberjacks->next;
				}
				flag = 1;
				(*bearAttack)++;
			}
		}
		current = current->next;
	}
}
void moveLumberjacks(worldMetaElement** lumberjacks,worldMetaElement* bears, worldElement forest[][FOREST_SIZE], int* bearAttack)
{
	worldMetaElement* current = *lumberjacks;
	worldMetaElement* previous = NULL;
	coords position;
	int i = 0, flag = 0;
	while (current)
	{
		flag = 0;
		for(i = 0;i<3 && !flag;i++)
		{
			position = randomMovement(current->position.x,current->position.y);
			while(collision(position,*lumberjacks))
				position = randomMovement(current->position.x,current->position.y);
			current->position = position;
			if(collision(position,bears))
			{
				(*bearAttack)++;
				if(current == *lumberjacks)
				{
					*lumberjacks = current->next;
					free(current);
					current = *lumberjacks;
				}
				else
				{
					previous->next = current->next;
					free(current);
					current = previous->next;
				}
				flag = 1;
			}
			else if(forest[current->position.x][current->position.y].displayImg == TREE.displayImg)
			{
				current->character.value++;
				forest[current->position.x][current->position.y]=BLANK;
				flag = 1;
			}
			else if(forest[current->position.x][current->position.y].displayImg == ELDERTREE.displayImg)
			{
				current->character.value+=2;
				forest[current->position.x][current->position.y]=BLANK;
				flag = 1;
			}
			
		}
		previous = current;
		if (current)
			current = current->next;
	}
	
}

int notEmpty(worldElement forest[][FOREST_SIZE])
{
	int i = 0;
	for(i = 0; i < FOREST_SIZE * FOREST_SIZE; i++)
	{
		if(forest[i/FOREST_SIZE][i%FOREST_SIZE].displayImg != BLANK.displayImg)
			return 1;
	}
	return 0;
}

int numberOf(worldMetaElement* list)
{
	int total = 0;
	while(list)
	{
		list=list->next;
		total++;
	}
	return total;
}

void printBoard(worldElement forest[][FOREST_SIZE])
{
	int i = 0;
	for(i = 0; i < FOREST_SIZE * FOREST_SIZE; i++)
	{
		mvaddch(i / FOREST_SIZE, i % FOREST_SIZE, forest[i/FOREST_SIZE][i%FOREST_SIZE].displayImg | (forest[i/FOREST_SIZE][i%FOREST_SIZE].displayImg == BLANK.displayImg?COLOR_PAIR(3):COLOR_PAIR(1)));
	}
}

void printMetaElements(worldMetaElement* character)
{
	while(character)
	{
	mvaddch(character->position.x, character->position.y, character->character.displayImg);
	character = character->next;
	}
}

void propagateTrees(worldElement forest[][FOREST_SIZE],worldElement treeType, int chance)
{
	coords new;
	int i;
	for(i = 0; i < FOREST_SIZE * FOREST_SIZE; i++)
	{
		if(forest[i/FOREST_SIZE][i%FOREST_SIZE].displayImg == treeType.displayImg)
		{
			if(!(rand()%chance))
			{
				new = randomMovement(i%FOREST_SIZE,i/FOREST_SIZE);
				if(forest[new.y][new.x].displayImg == BLANK.displayImg)
				{
					forest[new.y][new.x] = SAPLING;
				}
			}
		}
	}
}

coords randomMovement(int startX,int startY)
{
	coords new;
	new.x = startX - 1 + rand() % 3;
	new.y = startY - 1 + rand() % 3;
	while(new.x<0 || new.x >= FOREST_SIZE || new.y<0 || new.y >= FOREST_SIZE || 
		(new.x == startX && new.y == startY))
	{
		new.x = startX - 1 + rand() % 3;
		new.y = startY - 1 + rand() % 3;
	}
	return new;
}

void removeSingleElement(worldMetaElement* list)
{
	worldMetaElement*previous, * current = list;
	
	if(current && current->next)
	{
		while(current->next)
		{
			previous=current;
			current=current->next;
		}
		free(current);
		previous->next=NULL;
	}
}

void seedForest(worldElement forest[][FOREST_SIZE])
{
	int i,randX,randY;
	//Blank out map
	for(i = 0; i < FOREST_SIZE * FOREST_SIZE; i++)
	{
		forest[i/FOREST_SIZE][i%FOREST_SIZE] = BLANK;
	}
	//Add trees
	i = 0;
	while (i < FOREST_SIZE * FOREST_SIZE * .5)
	{
		randX = rand() % FOREST_SIZE;
		randY = rand() % FOREST_SIZE;
		if(forest[randX][randY].displayImg == BLANK.displayImg)
		{
			forest[randX][randY] = TREE;
			i++;
		}
	} 
}

int totalLumber(worldMetaElement* list)
{
	int total = 0;
	while(list)
	{
		total+=list->character.value;
		list=list->next;
	}
	return total;
	
}
