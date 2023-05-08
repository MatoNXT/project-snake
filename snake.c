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

struct STATS players[6];
FILE* log_file;
FILE* player_stats;
int current_player;

void s_bubble_sort_name(char names[MAX_NAMES][MAX_NAME_LENGTH], int n) {
    int i, j;
    char temp[MAX_NAME_LENGTH];

    for (i = 0; i < n-1; i++) {
        for (j = 0; j < n-i-1; j++) {
            if (strcmp(player[j].name, player[j+1].name) > 0) {
                strcpy(temp, player[j].name);
                strcpy(player[j].name, player[j+1].name);
                strcpy(player[j+1].name, temp);
            }
        }
    }
}
//iojoijoijoij
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
	printxy(1,1,"********************************************************************************");
	for(x = 2; x<=19; x++)
	{
		printxy(1,x,"*                                                                              *");//80
	}
	printxy(1,20,"********************************************************************************");//80
}

void s_initialize()
{
	log_file = fopen("mylog.txt", "a");
	player_stats = fopen("playerstats.dat","rb+");
	if(player_stats == NULL)
	{
		player_stats = fopen("playerstats.dat","wb");
	}
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
	//fwrite(&players, sizeof(struct STATS), 6, player_stats);
	fseek(player_stats,0,SEEK_SET);
	fwrite(players, sizeof(struct STATS), 6, player_stats);
}


void s_load()
{
    fread(players, sizeof(struct STATS), 6, player_stats);
	for(int i =0; i <= 5; i++)
	{
		players[i].id = i + 1;
	}
    
}

void s_log(char message)
{
	time_t now = time(NULL);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    if (log_file != NULL)
	{ 
		fprintf(log_file, "[%s] %s\n", timestamp, message);
	}
} 

void s_show_profiles(char *title)
{
	int i;
	char tmp_str[50];
	s_clear();
	printxy(34,3,title);
	printxy(15,5,"ID  Name                 Surname              Score");
	for(i = 0; i <=5; i++)
	{
		if(players[i].empty == 0)
		{
			sprintf(tmp_str,"%-3d %-20s",players[i].id, "EMPTY");
		}
		else
		{
			sprintf(tmp_str,"%-3d %-20s %-20s %-5d",players[i].id, players[i].name, players[i].surname, players[i].score.total);
		}
		printxy(15,6+i,tmp_str);
	}
	
	//printxy(17,13,"(You can select from profiles by pressing 1-6)");
	waitForAnyKey();
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
			if(pressed >= x+48 && pressed <= y+48)
			{
				pressed = pressed-48;
				break;
			}
		}
	}
	return pressed;
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
    
	selection = s_menu_selection(1,6);
	if(selection != 0)
	{
		s_enter_player(selection-1);
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
	//waitForAnyKey();  
	s_initialize();
	s_load();
	// memcpy(players[0].name, "Name", 4);
	// memcpy(players[0].surname, "Surname", 7);
	// players[0].id = 1;
	// players[0].empty=1;
	// players[0].score.round[0] = 2;
	// players[0].score.total = 3;
	// memcpy(players[1].name, "Name1", 5);
	// memcpy(players[1].surname, "Surname1", 8);
	// players[1].id = 2;
	// players[1].empty=1;
	// players[1].score.round[0] = 4;
	// players[1].score.total = 6;
      
	do
	{	
		switch(s_main_menu())
		{
			case 1:
				//s_load_game();
				break;
			case 2:
				s_show_profiles("Leaderboard");
				break;		
			case 3:
				s_select_profile();
				break;		
			case 0:
				s_exit(); 
				break;			
		}		
	} while(1);

	waitForAnyKey();  
	return 0;
}