#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <conio.h>
#include <windows.h> 

FILE* log_file;
FILE* player_stats;

typedef struct
{
	int round[3];
	int total;
}SCORE;

typedef struct 
{
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

int main()
{
	s_initialize();
	s_load();
	memcpy(players[0].name, "Name", 4);
	memcpy(players[0].surname, "Surname", 7);
	players[0].id = 1;
	players[0].score.round[0] = 2;
	players[0].score.total = 3;
	memcpy(players[1].name, "Name1", 5);
	memcpy(players[1].surname, "Surname1", 8);
	s_save();
	s_dispose();
	return 0;
}