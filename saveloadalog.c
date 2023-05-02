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

FILE* log_file;
FILE* player_stats;

int current_player;

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

typedef struct
{
	int round[3];
	int total;
}SCORE;

typedef struct 
{
	int empty;
	char name[20];
	char surname[20];
	int id;
	SCORE score;
} STATS;
STATS players[6];

void s_initialize()
{
	log_file = fopen("mylog.txt", "a");
	player_stats = fopen("playerstats.txt","w");
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
	fwrite(&players, sizeof(STATS), 6, player_stats);
}

void s_load()
{
    fread(&players, sizeof(STATS), 6, player_stats);
    
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

void s_show_profiles()
{
	int i;
	char tmp_str[50];
	s_clear();
	printxy(34,4,"Select profile:");
	printxy(15,5,"ID  Name                 Surname              Score");
	for(i = 0; i <=5; i++)
	{
		if(players[i].empty == 0)
		{
			printxy(19,6+i,"EMPTY");
		}
		else
		{
			sprintf(tmp_str,"%-3d %-20s %-20s %-5d",players[i].id, players[i].name, players[i].surname, players[i].score);
			printxy(15,6+i,tmp_str);
		}
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

void s_select_profile()
{
	int selection = 0;

	s_show_profiles();
    
	selection = s_menu_selection(0,5);

	switch(selection)
	{
		case 1:
			break;
		case 2:
			break;		
		case 0:
			break;			
	}	
}

void s_load_game()
{

}

void s_exit(void)
{
	char pressed;

	printxy(10,5,"Are you sure you want to exit(Y/N)");
	
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
	memcpy(players[0].name, "Name", 4);
	memcpy(players[0].surname, "Surname", 7);
	players[0].id = 1;
	players[0].empty=1;
	players[0].score.round[0] = 2;
	players[0].score.total = 3;
	memcpy(players[1].name, "Name1", 5);
	memcpy(players[1].surname, "Surname1", 8);
	players[1].id = 2;
	players[1].empty=1;
	players[1].score.round[0] = 4;
	players[1].score.total = 6;
      
	do
	{	
		switch(s_main_menu())
		{
			case 1:
				s_load_game();
				break;
			case 2:
				s_show_profiles();
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