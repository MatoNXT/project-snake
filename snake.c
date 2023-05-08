//
// 
//
//
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <conio.h>
#include <windows.h> 

#define UP_ARROW 72
#define LEFT_ARROW 75
#define RIGHT_ARROW 77
#define DOWN_ARROW 80
#define ENTER_KEY 13
#define EXIT_BUTTON 27 //ESC
#define PAUSE_BUTTON 112 //P
#define _PLAYERS 6
#define SC_WALL (char)219
#define SC_UP_CORNER (char)223
#define SC_DOWN_CORNER (char)220
#define SC_LINE (char)196
#define SC_SPACE (char)32

struct SCORE
{
	int round[3];
	int total;
};

struct STATS
{
	int empty;
	char name[20];
	char surname[20];
	int id;
	struct SCORE score;
};

struct STATS players[_PLAYERS];
int ndx[_PLAYERS];
FILE* log_file;
FILE* player_stats;
int current_player;

char *to_lower(const char *str)
{
    size_t len = strlen(str);
    char *lower_str = malloc(len + 1);
    if (lower_str == NULL)
	{
        fprintf(stderr, "Error: memory allocation in to_lower failed\n");
        return NULL;
    }
    for (size_t i = 0; i < len; i++)
	{
        lower_str[i] = tolower(str[i]);
    }
    lower_str[len] = '\0';
    return lower_str;
}

void s_log(const char *message)
{
	time_t now = time(NULL);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    if (log_file != NULL)
	{ 
		fprintf(log_file, "[%s] %s\n", timestamp, message);
	}
} 

void printxy(int x, int y, const char *s)
{
  COORD coord = { x, y};
  SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
  printf("%s",s);
}

char waitForAnyKey(void)
{
	int pressed;
	
	while(!kbhit());
	
	pressed = getch();
	//pressed = tolower(pressed);
	return((char)pressed);
}

void s_clear()
{
	int x; 
	char tmp_str[80];

	//sprintf(tmp_str,"%*s",80,WALL);

	memset(tmp_str,SC_UP_CORNER,80); tmp_str[0]=SC_WALL; tmp_str[79]=SC_WALL; tmp_str[80]='\0';
	printxy(1,1,tmp_str);
	memset(tmp_str,SC_SPACE,80); tmp_str[0]=SC_WALL; tmp_str[79]=SC_WALL;tmp_str[80]='\0';
	for(x = 2; x<=19; x++)
	{
		printxy(1,x,tmp_str);
	}
	memset(tmp_str,SC_DOWN_CORNER,80); tmp_str[0]=SC_WALL; tmp_str[79]=SC_WALL; tmp_str[80]='\0';
	printxy(1,20,tmp_str); 
}
void s_reset_ndx()
{
	for(int i =0; i<_PLAYERS; i++)
	{
		ndx[i]=i;
	}	
}

void s_initialize()
{
	log_file = fopen("mylog.txt", "a");
	player_stats = fopen("playerstats.dat","rb+");
	if(player_stats == NULL)
	{
		player_stats = fopen("playerstats.dat","wb");
	}
    s_reset_ndx();
	s_log("********************************************************************************");
}

void s_dispose()
{
	fclose(log_file);
	fclose(player_stats);
}

int s_save()
{
	if (player_stats == NULL) 
	{
        printf("Error opening file\n");
        return (1);
    }
	//fwrite(&players, sizeof(struct STATS), PLAYERS, player_stats);
	fseek(player_stats,0,SEEK_SET);
	fwrite(players, sizeof(struct STATS), _PLAYERS, player_stats);
}


void s_load()
{
    fread(players, sizeof(struct STATS), _PLAYERS, player_stats);
	for(int i =0; i <= 5; i++)
	{
		players[i].id = i + 1;
	}
    
}

void s_show_profiles(char *title)
{
	int i;
	char tmp_str[80];
	s_clear();
	printxy(34,3,title);
	printxy(8,4,"                                              Total  Round         ");
	printxy(8,5,"ID  Name                 Surname              Score  1     2     3 ");
	memset(tmp_str,SC_LINE,66); tmp_str[66]='\0';
	printxy(8,6,tmp_str);
	for(i = 0; i<_PLAYERS; i++)
	{
		if(players[ndx[i]].empty == 0)
		{
			sprintf(tmp_str,"%-3d %-20s",players[ndx[i]].id, "EMPTY");
		}
		else
		{
			sprintf(tmp_str,"%-3d %-20s %-20s %-5d  %-5d %-5d %-5d",players[ndx[i]].id, players[ndx[i]].name, players[ndx[i]].surname, players[ndx[i]].score.total, players[ndx[i]].score.round[0], players[ndx[i]].score.round[1], players[ndx[i]].score.round[2]);
		}
		printxy(8,7+i,tmp_str);
		s_log(tmp_str);
	}
	
	//printxy(17,13,"(You can select from profiles by pressing 1-6)");
	//waitForAnyKey();
	return;
}

int s_menu_selection(int x, int y)
{
	int pressed;
	
	while(1)
	{
		if(kbhit())
		{
			pressed=getch();
			if (pressed == 27)
			{
				s_clear();
				pressed = 0;
				break;
			}
			if (pressed >= x+48 && pressed <= y+48)
			{
				pressed = pressed-48;
				break;
			}
		}
	}
	return pressed;
}

void s_sort(int attr)
{
	int change = 1;
	int tmp;

	s_reset_ndx();
	if (attr == 1) // sort by Surnmae
	{
		while(change)
		{
			change = 0;
			for (int i=0; i < _PLAYERS-1; i++)
			{
				if (players[ndx[i]].empty!=0 && players[ndx[i+1]].empty!=0)
				{
					if (strcmp(to_lower(players[ndx[i]].surname),to_lower(players[ndx[i+1]].surname))>0)
					{
							tmp = ndx[i];
							ndx[i] = ndx[i+1];
							ndx[i+1] = tmp;
							change = 1;
					}
				}
			}
		}
	}
	else if (attr == 2)  // Sort by Name
	{
		while(change)
		{
			change = 0;
			for (int i=0; i < _PLAYERS-1; i++)
			{
				if (players[ndx[i]].empty!=0 && players[ndx[i+1]].empty!=0)
				{
					if (strcmp(to_lower(players[ndx[i]].name),to_lower(players[ndx[i+1]].name))>0)
					{
							tmp = ndx[i];
							ndx[i] = ndx[i+1];
							ndx[i+1] = tmp;
							change = 1;
					}
				}
			}
		}
	}
	else if (attr == 3) // Sort by ID
	{
		s_reset_ndx();
	}
	else if (attr >= 41 && attr <=43) // Sort by Roud
	{
		int x = attr-41;
		while(change)
		{
			change = 0;
			for (int i=0; i < _PLAYERS-1; i++)
			{
				if (players[ndx[i]].empty!=0 && players[ndx[i+1]].empty!=0)
				{
					if (players[ndx[i]].score.round[x] > players[ndx[i+1]].score.round[x])
					{
							tmp = ndx[i];
							ndx[i] = ndx[i+1];
							ndx[i+1] = tmp;
							change = 1;
					}
				}
			}
		}
	}
	else if (attr == 5) // Sort by Total Score
	{
		while(change)
		{
			change = 0;
			for (int i=0; i < _PLAYERS-1; i++)
			{
				if (players[ndx[i]].empty!=0 && players[ndx[i+1]].empty!=0)
				{
					if (players[ndx[i]].score.total > players[ndx[i+1]].score.total)
					{
							tmp = ndx[i];
							ndx[i] = ndx[i+1];
							ndx[i+1] = tmp;
							change = 1;
					}
				}
			}
		}
	}
	else
	{
		s_reset_ndx();
	}
}

int s_main_menu(void)
{
	int selected;
	char *t;
	
	s_clear();
	printxy(10,5,"1. New Game");
	printxy(10,6,"2. High Scores");
	printxy(10,7,"3. Select Profile");
	printxy(10,9,"0. Exit");
	selected = s_menu_selection(0, 3);
	return(selected);
} 

int s_enter_player(int p_id)
{
	char tmp_str[10];
	sprintf(tmp_str,"ID: %-3d", p_id+1);
	printxy(15,14,tmp_str);
	printxy(15,15,"Name:");
	scanf("%s", players[p_id].name);
	printxy(15,16,"Surname:");
	scanf("%s", players[p_id].surname);
	players[p_id].id = p_id+1;
	players[p_id].empty = 1;
	players[p_id].score.total = 0;
	for(int i =0; i <= 2; i++)
	{
		players[p_id].score.round[i] = 0;
	}
	return 0;
}

void s_select_profile()
{
	int selection = 0;

	s_show_profiles("Select profile:");
    
	selection = s_menu_selection(0,_PLAYERS);
	if(selection != 0)
	{
		s_enter_player(selection-1);
	}
}

int s_sort_round()
{
	int round = 1;
	printxy(6,17,"Sort by round number [1-3]:");
	round = s_menu_selection(1,3)+40;
	return round;
}

void s_show_leaderboard()
{
	int selection = 0;
	while (1)
	{
		s_show_profiles("Leaderboard");
		printxy(6,15,"Sort by:");
		printxy(6,16,"1. Surname   2. Name      3. ID        4. Round     5. Total score");
		printxy(6,17,"0. Back                                                           ");
		selection = s_menu_selection(0,5);
		if (selection !=0 )
		{
			if (selection == 4 )
			{
				selection == s_sort_round();
			}
			s_sort(selection);
		} 
		else
		{
			break;
		}
	}
}

// int s_generate_food(int foodXY[], int width, int height, int snakeXY[][SNAKE_ARRAY_SIZE], int snakeLength)
// {
// 	int i;
	
// 	do
// 	{
// 		srand ( time(NULL) );
// 		foodXY[0] = rand() % (width-2) + 2;
// 		srand ( time(NULL) );
// 		foodXY[1] = rand() % (height-6) + 2;
// 	} while (collisionSnake(foodXY[0], foodXY[1], snakeXY, snakeLength, 0)); //This should prevent the "Food" from being created on top of the snake. - However the food has a chance to be created ontop of the snake, in which case the snake should eat it...

// 	gotoxy(foodXY[0] ,foodXY[1]);
// 	printf("%c", FOOD);
	
// 	return(0);
// }

// int s_get_game_speed(void)
// {
// 	int speed;
// 	s_clear();
	
// 	do
// 	{
// 		gotoxy(10,5);
// 		printf("Select The game speed between 1 and 9.");
// 		speed = waitForAnyKey()-48;
// 	} while(speed < 1 || speed > 9);
// 	return(speed);
// }

// void s_load_game()
// {
// 	int snakeXY[2][SNAKE_ARRAY_SIZE]; //Two Dimentional Array, the first array is for the X coordinates and the second array for the Y coordinates
	
// 	int snakeLength = 4; //Starting Length
	
// 	int direction = LEFT_ARROW; //DO NOT CHANGE THIS TO RIGHT ARROW, THE GAME WILL INSTANTLY BE OVER IF YOU DO!!! <- Unless the prepairSnakeArray function is changed to take into account the direction....
	
// 	int foodXY[] = {5,5};// Stores the location of the food
	
// 	int score = 0;
// 	//int level = 1;
	
// 	//Window Width * Height - at some point find a way to get the actual dimensions of the console... <- Also somethings that display dont take this dimentions into account.. need to fix this...
// 	int consoleWidth = 80;
// 	int consoleHeight = 25;
	
// 	int speed = s_get_game_speed();
	
// 	//The starting location of the snake
// 	snakeXY[0][0] = 40; 
// 	snakeXY[1][0] = 10;
	
// 	loadEnviroment(consoleWidth, consoleHeight); //borders
// 	prepairSnakeArray(snakeXY, snakeLength);
// 	loadSnake(snakeXY, snakeLength);
// 	s_generate_food( foodXY, consoleWidth, consoleHeight, snakeXY, snakeLength);
// 	refreshInfoBar(score, speed); //Bottom info bar. Score, Level etc
// 	startGame(snakeXY, foodXY, consoleWidth, consoleHeight, snakeLength, direction, score, speed);

// 	return;
// }

void s_exit(void)
{
	char pressed;

	printxy(10,15,"Are you sure you want to exit(Y/N)");
	
	do
	{
		pressed = waitForAnyKey();
		pressed = tolower(pressed);
	} while (!(pressed == 'y' || pressed == 'n'));
	
	if (pressed == 'y')
	{
		s_save();
		s_dispose();
		s_clear(); //clear the console
		exit(0);
	}
	return;
}

int main()
{ 
	s_initialize();
	s_load();
	//waitForAnyKey(); 
	do
	{	
		switch(s_main_menu())
		{
			case 1:
				//s_load_game();
				break;
			case 2:
				s_show_leaderboard();
				break;		
			case 3:
				s_select_profile();
				break;		
			case 0:
				s_exit(); 
				break;			
		}		
	} while(1);
	//waitForAnyKey();  
	return 0;
}