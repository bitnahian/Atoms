#include <stdio.h>
#include "atoms.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>


// This node always points to the head
move_t *head = NULL;

void help();
bool isEmpty();
move_t* createMove(int x, int y, int move_no, player_t* player);
grid_t** initializeGame(int width, int height);
void InsertAtTail(int x, int y, int move_no, player_t* player);
void forwardPrint();
void freeList();
void fillGrid(int width, int height, grid_t**, int x, int y, player_t* player);
void expand(int width, int height, grid_t** grid, int x, int y, player_t* player);
save_file_t* saveFile(int width, int height, int pl_no);
int* place(int width, int height, int x, int y, grid_t** grid, int pl_pos, int lim, int pl_remaining, int move_no,
	       	player_t** player, player_t* current_pl, bool* lost);
void deleteNode();
int nodeLength();



int main(int argc, char** argv) {

	int width = 0; // This is width -> value of x
	int height = 0; // This is height -> value of y
	int pl_no = 0;
	bool lost[6] = {false, false, false, false, false, false};
	bool saved = false;
	bool loaded = false;
	save_file_t* saved_file;
	int pl_pos = 0;
	int lim;
	int pl_remaining;
	int move_no;
	int* result;
	player_t* current_pl;
	int* xC;
	int* yC;
	int no_of_moves;
	int n = 0;
	bool undo = false;

	// Deal with number of players here
	// Set number of players to as specified

	player_t** player = (player_t**) malloc(sizeof(player_t*)*6);
	for(int i = 0; i < 6; ++i)
	{
		player[i] = (player_t*)malloc(sizeof(player_t));
	}
	player[0]->colour = "Red";
	player[1]->colour = "Green";
	player[2]->colour = "Purple";
	player[3]->colour = "Blue";
	player[4]->colour = "Yellow";
	player[5]->colour = "White";

	for(int i = 0; i < 6; ++i)
	{
		player[i]->grids_owned = 0;
	}

	grid_t **grid;


	while(1)
	{
		// Input command
		char command[256];
		char *execute[5];
		// String validation for command checking
		fgets(command, 100, stdin);
		command[strlen(command)-1] = '\0';
		// printf("%s", command);
		
		int args = 0;
   		char *p = strtok (command, " ");

    	while (p != NULL && args < 5)
    	{
       		execute[args++] = p;
       		p = strtok (NULL, " ");
    	}
		
		// First only 3 commands are valid: QUIT, HELP and START
		
		// START command
		if((strcmp(execute[0], "START") == 0) && loaded == false)
		{	
			char *endptrX;
			char *endptrY;
			char *endptrZ;
			
			width = (int) strtol(execute[2], &endptrX, 10);
			height = (int) strtol(execute[3], &endptrY, 10);
			pl_no = (int) strtol(execute[1], &endptrZ, 10);
			if(endptrX == execute[2] || endptrY == execute[3] || endptrZ == execute[1] 
			   || width < MIN_WIDTH || width > MAX_WIDTH || height < MIN_HEIGHT || height > MAX_HEIGHT 
			   || pl_no < MIN_PLAYERS || pl_no > MAX_PLAYERS)
			{
				fprintf(stderr, "Invalid command arguments\n");
				continue;
			}
			else if (pl_no > (width*height))
			{
				fprintf(stderr, "Cannot Start Game\n");
				continue;
			}
			else if (args < 3)
			{
				fprintf(stderr, "Missing Argument");
				continue;
			}
			else if (args > 4)
			{
				fprintf(stderr, "Too Many Arguments\n");
				continue;
			}

			printf("Game Ready\n");

			grid = initializeGame(width, height); 	// This function should make the game grid
			for(int i = 0; i < height; ++i)		// For every row
			{
				for(int j = 0; j < width; ++j)	// For every column
				{
					grid[i][j].atom_count = 0;
					grid[i][j].owner = NULL;
				}

			}
			break;
		}
		// HELP command
		else if((strcmp(execute[0], "HELP") == 0) && loaded == false)
		{
			help();
		}
		// QUIT command
		else if (strcmp(execute[0], "QUIT") == 0)
		{
			if(loaded == true)
			{
				free(xC);
				free(yC);
				for(int i = 0; i < height; i++)
				{
					grid_t* currentPtr = grid[i];
					free(currentPtr);
					//printf("no error");
				}
					//printf("no error");
				free(grid);
				//printf("no error");
				freeList();
				//printf("no error");
				
			}
			
			for(int i = 0; i < 6; ++i)
				{
					free(player[i]);
				}
			free(player);


			printf("Bye!\n");
			return 0;
		}
		// STAT command
		else if ((strcmp(execute[0], "STAT") == 0) && loaded == false)
		{
			printf("Game Not In Progress\n");
		}
		else if ((strcmp(execute[0], "UNDO") == 0) && loaded == false)
		{
			printf("Cannot Undo\n");
		}
		// LOAD command
		else if ((strcmp(execute[0], "LOAD") == 0) && loaded == false)
		{
			if(args < 1)
			{
				fprintf(stderr, "Enter File Name\n");
				continue;	
			}
			if( access( execute[1], F_OK ) == -1 ) 
			{
				// File does not exist
				fprintf(stderr, "Cannot Load Save\n");
				continue;

			}
	
			FILE * fp = fopen(execute[1], "rb");

			if (fp == NULL)
			{
				fprintf(stderr, "Cannot Load Save\n");
				continue;
			}
			else
			{
				loaded = true;
				fread(&width, sizeof(uint8_t), 1, fp);
				fread(&height, sizeof(uint8_t), 1, fp);
				fread(&pl_no, sizeof(uint8_t), 1, fp);

				grid = initializeGame(width, height);  // Make the grid using the dimensions from the load file
				for(int i = 0; i < height; ++i)		// For every row
				{
					for(int j = 0; j < width; ++j)	// For every column
					{
						grid[i][j].atom_count = 0;
						grid[i][j].owner = NULL;
					}

				}
				int fileSize;
				FILE * fs = fopen(execute[1], "rb");
				fseek(fs, 0, SEEK_END);
				fileSize = ftell(fs) - 3*sizeof(uint8_t);
				no_of_moves = (fileSize/sizeof(uint32_t));
				// printf("no of moves: %d\n", no_of_moves);

				xC = (int*)calloc(no_of_moves, sizeof(int));	
				yC = (int*)calloc(no_of_moves, sizeof(int));

				int i = 0;
				while(!feof(fp) && i < no_of_moves)
				{
					uint32_t move_data = 0;
					int output = fread(&move_data, sizeof(uint32_t), 1, fp);
					if(output == 1)
					{
						int p = move_data & 0xff;
						int q = ((move_data & 0xff00) >> 8);
						xC[i] = p;
						yC[i] = q;
						 //printf("x: %d\n", p);
						 //printf("y: %d\n", q);
					}
					++i;

				}
				fclose(fs);
				fclose(fp);
				printf("Game Loaded\n\n");
				
			}

		}
		// PLAYFROM command
		else if ((strcmp(execute[0], "PLAYFROM") == 0) && loaded == true)
		{
			
			char *endptrX;
			
			if (strcmp(execute[1], "END") == 0)
			{
				n = no_of_moves - 1;
			}
			else
			{
				n = (int) strtol(execute[1], &endptrX, 10) - 1;
				// printf("%d\n", n);
				// Do some error checking for n
				if ( n > no_of_moves )
				{
					n = no_of_moves - 1;
				}
				else if(endptrX == execute[1] ) 
				{
					fprintf(stderr, "Invalid Arguments\n");
					continue;

				}
				else if( n < -1)
				{
					fprintf(stderr, "Invalid Turn Number\n");
					continue;
				}

			}

			pl_pos = 0;
			lim = pl_no;
			pl_remaining = pl_no;
			move_no = 1;
			current_pl = player[pl_pos];

			//printf("width: %d\n", width);
			//printf("height: %d\n", height);
			//printf("number of players: %d\n", pl_no);


			for(int i = 0; i <= n; ++i)
			{	
					
				// Algorithm to keep tracking previous owner and user's turn
				if(pl_pos == lim)
				{
					pl_pos = 0;
				}
				if(current_pl != player[pl_pos])
				{
					if(pl_pos == lim)
					{
						pl_pos = 0;
					}
					current_pl = player[pl_pos];
					if(move_no > lim && current_pl->grids_owned == 0)
					{
						pl_pos++;
						if(pl_pos == lim)
						{
							pl_pos = 0;
						}
						continue;
					}
				}
				
				int p = xC[i];
				int q = yC[i];
				// printf("x: %d\n", (p));
				// printf("y: %d\n", (q));
					
				result = place(width, height, p, q, grid, pl_pos, lim, pl_remaining, move_no,
					       	player, current_pl, lost);
				if(result[2] ==  0)
				{
					free(result);
					break;
				}
					pl_pos = result[0];
					move_no = result[1];
					free(result);

			}
			printf("Game Ready\n");
			//printf("%s's Turn\n\n", current_pl->colour);
			free(xC);
			free(yC);
			break;
		
		}
		else
		{
			printf("Invalid Command\n");
		}
	}
	
	
	if(loaded == false)
	{
		pl_pos = 0;
		lim = pl_no;
		pl_remaining = pl_no;
		move_no = 1;
		current_pl = player[pl_pos];
		printf("Red's Turn\n\n");

	}
	else if( loaded == true && n != no_of_moves - 1)
	{
		printf("%s's Turn\n\n", current_pl->colour);
	}
	

	while(1)
	{	
		// Algorithm to keep tracking previous owner and user's turn
		if(pl_pos == lim)
		{
			pl_pos = 0;
		}
		if(undo == true && current_pl == player[pl_pos])
		{
			printf("%s's Turn\n\n", current_pl->colour);
			undo = false;
		}
		if(current_pl != player[pl_pos])
		{
			if(pl_pos == lim)
			{
				pl_pos = 0;
			}
			current_pl = player[pl_pos];
			if(move_no > lim && current_pl->grids_owned == 0)
			{
				pl_pos++;
				if(pl_pos == lim)
				{
					pl_pos = 0;
				}
				continue;
			}
			printf("%s's Turn\n\n", current_pl->colour);	
		}
		// Input command
		char command[256];
		char *execute[4];
		// String validation for command checking
		fgets(command, 256, stdin);
		command[strlen(command)-1] = '\0';
		// printf("%s", command);
		
		int args = 0;
   		char *p = strtok (command, " ");

    		while (p != NULL && args < 4)
    		{
		
	       		execute[args++] = p;
       			p = strtok (NULL, " ");
    		}
	
		// START command
		if(strcmp(execute[0], "START") == 0)
		{	
			printf("Game already started");
			undo = false;
		}
		// HELP command
		else if(strcmp(execute[0], "HELP") == 0)
		{
			help();
			undo = false;
		}
		// QUIT command
		else if (strcmp(execute[0], "QUIT") == 0)
		{
			printf("Bye!\n");
			break;
		}
		// STAT command
		else if (strcmp(execute[0], "STAT") == 0)
		{
			for(int i = 0; i < lim; ++i)
			{
				printf("Player %s:\n", player[i]->colour);
				if(player[i]->grids_owned == 0 & move_no > lim)
				{
					printf("Lost\n");
				}
				else 
					printf("Grid Count: %d\n", player[i]->grids_owned);
			
				if( i != lim)
				{
					printf("\n");
				}
				
			}
			undo = false;
		}
		// PLACE command
		else if (strcmp(execute[0], "PLACE") == 0)
		{
			char *endptrX;
			char *endptrY;
			
			int x = (int) strtol(execute[1], &endptrX, 10);
			int y = (int) strtol(execute[2], &endptrY, 10);
			if(endptrX == execute[1] || endptrY == execute[2] )
			{
				fprintf(stderr, "Invalid Arguments\n");
				continue;
			}
			else if(args > 3)
			{
				fprintf(stderr, "Too Many Arguments\n");
				continue;

			}
			else if(args < 3)
			{
				fprintf(stderr, "Missing Argument\n");
				continue;

			}
			else if( x < 0 || x > width - 1 || y < 0 || y > height - 1)
			{
				fprintf(stderr, "Invalid Coordinates\n");
				continue;
			}

			result = place(width, height, x, y, grid, pl_pos, lim, pl_remaining, move_no, player, current_pl, lost);
			if(result[2] ==  0)
			{
				free(result);
				break;
			}
			pl_pos = result[0];
			move_no = result[1];
			free(result);
			undo = false;
		
		}
		// DISPLAY command
		else if(strcmp(execute[0], "DISPLAY") == 0)
		{
			printf("\n");
			for(int i = 0; i < width*3 + 1; ++i)
			{
				if(i == 0 || i == (width*3))
					printf("+");
				else
					printf("-");
			}

			printf("\n");

			for(int i = 0; i < height; ++i)
			{
				for(int j = 0; j < width; ++j)
				{
					if(grid[i][j].owner == NULL)
					{	
						printf("|  ");
					}
					else
					{
						printf("|%c%d", grid[i][j].owner->colour[0], grid[i][j].atom_count);
					}
					if( j == width -1)
						printf("|");
				}
				printf("\n");

			}		

			for(int i = 0; i < width*3 + 1; ++i)
			{
				if(i == 0 || i == (width*3))
					printf("+");
				else
					printf("-");
			}
			printf("\n\n");
			undo = false;

		}
		// UNDO command
		else if(strcmp(execute[0], "UNDO") == 0)
		{
			undo = true;
			if(move_no < 2)
			{
				printf("Cannot Undo\n");
				undo = false;
				continue;
			}
			for(int i = 0; i < height; i++)
			{
				for(int j = 0; j < width; ++j)
				{
					grid[i][j].owner = NULL;
					grid[i][j].atom_count = 0;
				}

			}

					//printf("no error");
			deleteNode();
			int node_length = nodeLength();
			
			xC = (int*)calloc(node_length, sizeof(int));	
			yC = (int*)calloc(node_length, sizeof(int));

			move_t* temp = head;

			int jk = 0;

			while(temp != NULL)
			{
				xC[jk] = temp->x;
				yC[jk] = temp->y;
				++jk;
				temp = temp->next;

			}
			// forwardPrint();

			pl_pos = 0;
			lim = pl_no;
			pl_remaining = pl_no;
			move_no = 1;
			current_pl = player[pl_pos];
			for(int i = 0; i < 6; ++i)
			{
				lost[i] = false;
			}

			//printf("width: %d\n", width);
			//printf("height: %d\n", height);
			//printf("number of players: %d\n", pl_no);

			jk = 0;
			freeList();
			
			while( jk < node_length)
			{	
					
				// Algorithm to keep tracking previous owner and user's turn
				if(pl_pos == lim)
				{
					pl_pos = 0;
				}
				if(current_pl != player[pl_pos])
				{
					if(pl_pos == lim)
					{
						pl_pos = 0;
					}
					current_pl = player[pl_pos];
					if(move_no > lim && current_pl->grids_owned == 0)
					{
						pl_pos++;
						if(pl_pos == lim)
						{
							pl_pos = 0;
						}
						continue;
					}
				}
				
				int p = xC[jk];
				int q = yC[jk];

				//printf("x: %d\n", (p));
				//printf("y: %d\n", (q));
					
				result = place(width, height, p, q, grid, pl_pos, lim, pl_remaining, move_no,
					       	player, current_pl, lost);
				if(result[2] ==  0)
				{
					free(result);
					break;
				}
				pl_pos = result[0];
				move_no = result[1];
				free(result);
				++jk;
				

			}
			free(xC);
			free(yC);
			//printf("%s's Turn\n", current_pl->colour);
		}
		// SAVE command
			else if(strcmp(execute[-0], "SAVE") == 0)
		{

			// Write a function that iterates through the linked list
			// And stores the moves
			
			if(args < 1)
			{
				fprintf(stderr, "Enter File Name\n");
				continue;	
			}
			if( access( execute[1], F_OK ) != -1 ) 
			{
				// File already exists exist
				fprintf(stderr, "File Already Exists\n");
				continue;

			}
			move_t* temp = head;
			int count = 0;
			while(temp != NULL)
			{
				count++;
				temp = temp->next;

			}

			saved_file = saveFile(width, height, pl_no);
			saved = true;

			FILE * fp = fopen(execute[1], "wb+");
				
			if(fp == NULL)
			{	
				exit(EXIT_FAILURE);
			}
			else
			{
				
				fwrite(&saved_file->width, sizeof(uint8_t), 1, fp);
				fwrite(&saved_file->height, sizeof(uint8_t), 1, fp);
				fwrite(&saved_file->no_players, sizeof(uint8_t), 1, fp);
				
				for(int i = 0; i < count; ++i)
				{
					fwrite(&saved_file->raw_move_data[i], sizeof(uint32_t), 1, fp);
					//printf("count: %d\n", i);
				}
				
				fclose(fp);
			
			}
			printf("Game Saved\n\n");
			undo = false;
			continue;
		}
		else if(strcmp(execute[0], "LOAD") == 0)
		{
			printf("Restart Application To Load Save\n");
			undo = false;
		}

		else
		{
			printf("Invalid Command\n");
			undo = false;
		}
	}

	// forwardPrint();
	
	for(int i = 0; i < height; i++)
	{
		grid_t* currentPtr = grid[i];
		free(currentPtr);
	//	printf("no error");
	}
	//printf("no error");
	free(grid);
	//printf("no error");
	freeList();
	//printf("no error");
	for(int i = 0; i < 6; ++i)
	{
		free(player[i]);
	}
	free(player);
	if(saved == true)
	{
		//printf("width: %u\n", saved_file->width);
		//printf("height: %u\n", saved_file->height);
		//int p = saved_file->raw_move_data[1] & 0xff;
		//int q = (saved_file->raw_move_data[1] & 0xff00) >> 8;
		//printf("x: %d\n", (p));
		//printf("y: %d\n", (q));
		free(saved_file->raw_move_data);
		free(saved_file);
	}	
	return 0;
	
}
void help()
{
	printf("\n");
	printf("HELP displays this help message\n");
	printf("QUIT quits the current game\n");
	printf("\n");
	printf("DISPLAY draws the game board in terminal\n");
	printf("START <number of players> <width> <height> starts the game\n");
	printf("PLACE <x> <y> places an atom in a grid space\n");
	printf("UNDO undoes the last move made\n");
	printf("STAT displays game statistics\n");
	printf("\n");
	printf("SAVE <filename> saves the state of the game\n");
	printf("LOAD <filename> loads a save file\n");
	printf("PLAYFROM <turn> plays from n steps into the game\n");
	printf("\n");
}



grid_t** initializeGame(int width, int height)
{
	grid_t** grid = (grid_t**) malloc(sizeof(grid_t*) * height);
	for(int i = 0; i < height; ++i)
	{
		grid[i] = (grid_t*)malloc(sizeof(grid_t) * width);
	}
	
	return grid;
}

move_t* createMove(int x, int y, int move_no, player_t* player)
{
	move_t* move = (move_t*)malloc(sizeof(move_t));
	move -> next = NULL;
	move -> prev = NULL;
	move -> x = x;
	move -> y = y;
	if(move_no != 1)
		move -> old_owner = player;
	else
		move -> old_owner = NULL;
	move -> move_no = move_no;
	
	return move;

}

// Is list empty
bool isEmpty() {
	return head == NULL;
}

//Inserts a Node at tail of Doubly linked list
void InsertAtTail(int x, int y, int move_no, player_t* player) {
	move_t* temp = head;
	move_t* newMove = createMove(x, y, move_no, player);
	if(head == NULL) {
		head = newMove;
		return;
	}
	while(temp->next != NULL) temp = temp->next; // Go To last Node
	temp->next = newMove;
	newMove->prev = temp;
}

void deleteEndNode()
{
	move_t* temp = head;
	while(temp != NULL)
	{
		temp = temp->next;
	}

	// Now at last node
	
}
void forwardPrint()
{
	move_t* temp = head;
	int count = 0;
	while(temp != NULL)
	{
		printf("For linked list[%d]:\n", count);
		printf("x: %d\n", temp->x);
		printf("y: %d\n", temp->y);
		printf("move number : %d\n", temp->move_no);
		if(temp->old_owner != NULL)
			printf("old owner: %s\n", temp->old_owner->colour);
		temp = temp->next;
		count++;
	}
}
void freeList()
{
	move_t* tmp;

	while (head != NULL)
	{
		tmp = head;
		head = head->next;
		free(tmp);
	}

}

void fillGrid(int width, int height, grid_t** grid, int x, int y, player_t* player)
{
	// write a function to find limit based on position
	int limit = 0;
	if(grid[y][x].owner != NULL)
	{
		// grid[y][x].owner->grids_owned -= grid[y][x].atom_count;
		grid[y][x].owner -> grids_owned--;
	}
	grid[y][x].owner = player;
	grid[y][x].atom_count++;
	//player->grids_owned += grid[y][x].atom_count;
	player->grids_owned++;
	
	if((x == 0 && y == 0) || (x == 0 && y == height - 1) || (x == width-1 && y == 0) || (x == width-1 && y == height-1))
	{
		limit = 2;
	}
	else if( (x == 0 || x == width-1) || (y == 0 || y == height-1))
	{
		limit = 3;
	}
	else
	{
		limit = 4;
	}
	if(grid[y][x].atom_count == limit)
	{
		grid[y][x].atom_count = 0;
		grid[y][x].owner = NULL;
		player->grids_owned--;
		expand(width, height, grid, x, y, player);
	}
	return;

}

void expand(int width, int height, grid_t** grid, int x, int y, player_t* player)
{
	// Write an algorithm which does clockwise depth first recursion
	// p is the row
	// q is the column
	
	// First go up
	if(!((y-1) < 0))
	{
		fillGrid(width, height, grid, x, y-1, player);
	}
	// Then go right
	if(!((x+1) > (width-1)))
	{
		fillGrid(width, height, grid, x+1, y, player);
	}
	// Then go down	
	if(!((y+1) > (height-1)))
	{
		fillGrid(width, height, grid, x, y+1, player);
	}
	// Then go left
	if(!((x-1) < 0))
	{
		fillGrid(width, height, grid, x-1, y, player);
	}
}
// Creates a save_file_t* struct which contains the move data
save_file_t* saveFile(int width, int height, int pl_no)
{
	save_file_t* file = (save_file_t*)malloc(sizeof(save_file_t));

	file->width = (uint8_t) width;
	file->height = (uint8_t) height;
	file->no_players = (uint8_t) pl_no ;

	move_t* temp = head;
	int count = 0;
	while(temp != NULL)
	{
		count++;
		temp = temp->next;
	}
	
	file->raw_move_data = (uint32_t*) malloc((sizeof(uint32_t)*count));

	temp = head;
	int i = 0;

	while(temp != NULL)
	{
		// Packing raw move data
	
		file->raw_move_data[i] = 0x00000000;

		// Write X at first byte
		
		file->raw_move_data[i] |= ((uint8_t)temp->x);

		// Write Y at second byte

		file->raw_move_data[i] |= ((uint8_t)temp->y << 8);
		temp = temp->next;
		// printf("%d\n", i);
		++i;
	}
	return file;

}

int* place(int width, int height, int x, int y, grid_t** grid, int pl_pos, int lim, int pl_remaining, int move_no,
	       	player_t** player, player_t* current_pl, bool* lost)
{
	int* arr = (int*)malloc(sizeof(int)*3);
	if(grid[y][x].owner == NULL || grid[y][x].owner == current_pl)
	{
		// Write an algo for insertion
		int i = pl_pos - 1;
		if(pl_pos == 0)
			i = lim - 1;
		while((i  >= 0) && (i < lim))
		{
			if(lost[i] == true)
			{
				--i;
				if(i < 0)
				{
					i = lim - 1;
				}
				continue;
			}
			InsertAtTail(x,y,move_no,player[i]);
			break;
		}
		
		fillGrid(width, height, grid, x, y, current_pl);
		pl_pos++;
		move_no++;
		// Check to see how many people lost
		if(move_no > lim)
		{
			int i = 0;
			while(i < lim)
			{
				if(player[i]->grids_owned == 0 && lost[i] == false)
				{
					pl_remaining--;
					lost[i] = true;
				}
				++i;
			}
		}
		if(pl_remaining == 1 && move_no > lim)
		{
			for(int i = 0; i < lim; ++i)
			{
				if(player[i]->grids_owned != 0)
				{
					printf("%s Wins!\n", player[i]->colour);
					arr[0] = pl_pos;
					arr[1] = move_no;
					arr[2] = 0;
					return arr;
				}
			}
			
		}
		arr[0] = pl_pos;
		arr[1] = move_no;
		arr[2] = 1;
		return arr;
	}
	else
	{
		printf("Cannot Place Atom Here\n");
		arr[0] = pl_pos;
		arr[1] = move_no;
		arr[2] = 1;

		return arr;
	}
	// Have to return pl_pos, move_no, result

}

void deleteNode()
{
	  move_t* temp = head;
	  move_t* t;
	  if(head->next==NULL)
  	  {
      	free(head);
      	head=NULL;
		return;
      }
	  else
	  {
	  	while(temp->next != NULL)
	  	{
		  	t=temp;
		  	temp=temp->next;
	  	}
	  	free(t->next);
	  	t->next=NULL;
      	return;
	  }
}

int nodeLength()
{
	int node_length = 0;
	move_t* temp = head;
	while(temp != NULL)
	{
		node_length++;
		temp=temp->next;
	}
	
	return node_length;

}