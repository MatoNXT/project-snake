//      ______   ____  _____       _       ___  ____   ________  
//    .' ____ \ |_   \|_   _|     / \     |_  ||_  _| |_   __  | 
//    | (___ \_|  |   \ | |      / _ \      | |_/ /     | |_ \_| 
//     _.____`.   | |\ \| |     / ___ \     |  __'.     |  _| _  
//    | \____) | _| |_\   |_  _/ /   \ \_  _| |  \ \_  _| |__/ | 
//     \______.'|_____|\____||____| |____||____||____||________|                                                          
//  
//  TODO:
//  - supress new lines while entering name/surname
//  - kill buffered keypressed.
//   
//  REFERENCES:
//  this project          - https://github.com/MatoNXT/project-snake/blob/main/snake.c
//  original snake source - https://github.com/Contagious06/console-snake-game/blob/master/src/snake.c
//  console functions     - https://learn.microsoft.com/en-us/windows/console/using-the-console
//  tuning functions      - https://chat.openai.com/chat
//  ascii art             - https://patorjk.com/software/taag/
//

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <conio.h>
#include <windows.h> 

//text colors
#define BLACK              0
#define BLUE               1
#define GREEN              2
#define CYAN               3
#define RED                4
#define MAGENTA            5
#define BROWN              6
#define WHITE              7
#define GRAY               8
#define LIGHT_BLUE         9
#define LIGHT_GREEN       10
#define LIGHT_CYAN        11
#define LIGHT_RED         12
#define LIGHT_MAGENTA     13
#define YELLOW            14
#define BRIGHT_WHITE      15
// background colors
#define BG_BLACK           0
#define BG_BLUE           16
#define BG_GREEN          32
#define BG_CYAN           48
#define BG_RED            64
#define BG_MAGENTA        80
#define BG_BROWN          96
#define BG_WHITE         112
#define BG_GRAY          128
#define BG_LIGHT_BLUE    144
#define BG_LIGHT_GREEN   160
#define BG_LIGHT_CYAN    176
#define BG_LIGHT_RED     192
#define BG_LIGHT_MAGENTA 208
#define BG_YELLOW        224
#define BG_BRIGHT_WHITE  240


#define KB_UP_ARROW 72
#define KB_LEFT_ARROW 75
#define KB_RIGHT_ARROW 77
#define KB_DOWN_ARROW 80
#define KB_ENTER_KEY 13
#define KB_EXIT_BUTTON 27 //ESC
#define KB_PAUSE_BUTTON 112 //P
#define S_PLAYERS 6
#define S_SCR_COL_MONOCHROME WHITE+BG_BLACK
#define S_SCR_COL_STATUS BRIGHT_WHITE+BG_BLACK
#define S_SCR_WALL (char)219
#define S_SCR_COL_WALL RED+BG_BLACK
#define S_SCR_WALL_UP_CORNER (char)223
#define S_SCR_WALL_DOWN_CORNER (char)220
#define S_SCR_LINE (char)196
#define S_SCR_SPACE (char)32
#define S_SCR_LINES 25
#define S_SCR_COLUMNS 80
#define S_SCR_BUFFER_SIZE S_SCR_LINES*S_SCR_COLUMNS

struct SCREEN
{
    HANDLE h_stdout, h_active;           // screen handle
    SMALL_RECT rectangle;
    COORD size;
    COORD buffer_coord;
    CHAR_INFO buffer[S_SCR_BUFFER_SIZE];    // screen buffer [25][80];
    BOOL f_success;
};

struct SCORE
{
    int round[3];
    int total;
};

struct STATS
{
    int id;
    int empty;
    int current;
    char name[21];
    char surname[21];
    struct SCORE score;
};

struct SNAKE
{
    int speed;
    int length;
};

struct SCREEN screen;
struct STATS players[S_PLAYERS];
struct SNAKE snake;
int ndx[S_PLAYERS];
FILE* f_log_file;
FILE* f_player_stats;
int current_player = -1;

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
    if (f_log_file != NULL)
    { 
        fprintf(f_log_file, "[%s] %s\n", timestamp, message);
    }
} 

void printxy(int x, int y, const char *s)
{
    COORD coord = { x, y};
    SetConsoleCursorPosition(screen.h_active, coord);
    printf("%s",s);
}

void s_screen_printxy(int col, int lin, const char *s, int attr)
{
    int position = (lin-1)*80+(col-1);
    for (int i = 0; s[i]!=0; i++){
        screen.buffer[position+i].Char.AsciiChar=s[i];
        screen.buffer[position+i].Attributes=attr;
    }
}

void s_screen_buffer_flush()
{
    screen.f_success = WriteConsoleOutput(
        screen.h_active,          // screen hand3le
        screen.buffer,            // buffer to copy from
        screen.size,              // col-row size of screen buffer
        screen.buffer_coord,      // 
        &screen.rectangle         // top left src cell in screen buffer 
    );  
    if (! screen.f_success)
    {
        printf("WriteConsoleOutput failed - (%d)\n", GetLastError());
    }
}

char s_wait_for_any_key(void)
{
    int pressed;

    while(!kbhit());
    
    pressed = getch();
    //pressed = tolower(pressed);
    return((char)pressed);
}

void s_screen_clear()
{
    char tmp_str[80];

    memset(tmp_str,S_SCR_WALL_UP_CORNER,80); tmp_str[0]=S_SCR_WALL; tmp_str[79]=S_SCR_WALL; tmp_str[80]='\0';
    s_screen_printxy(1,1,tmp_str,S_SCR_COL_WALL);
    memset(tmp_str,S_SCR_SPACE,80); tmp_str[0]=S_SCR_WALL; tmp_str[79]=S_SCR_WALL;tmp_str[80]='\0';
    for(int x = 2; x<=19; x++)
    {
        s_screen_printxy(1,x,tmp_str,S_SCR_COL_WALL);
    }
    memset(tmp_str,S_SCR_WALL_DOWN_CORNER,80); tmp_str[0]=S_SCR_WALL; tmp_str[79]=S_SCR_WALL; tmp_str[80]='\0';
    s_screen_printxy(1,20,tmp_str,S_SCR_COL_WALL);
    memset(tmp_str,S_SCR_SPACE,80); tmp_str[80]='\0';
    for(int x = 21; x<=S_SCR_LINES; x++)
    {
        s_screen_printxy(1,x,tmp_str,S_SCR_COL_WALL);
    }
    s_screen_buffer_flush();
}
void s_reset_ndx()
{
    for(int i =0; i<S_PLAYERS; i++)
    {
        ndx[i]=i;
    }    
}

void s_screen_init()
{
    screen.h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    screen.h_active = CreateConsoleScreenBuffer(
       GENERIC_READ |           // read/write access
       GENERIC_WRITE,
       FILE_SHARE_READ |
       FILE_SHARE_WRITE,        // shared
       NULL,                    // default security attributes
       CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE
       NULL);                   // reserved; must be NULL
    if (screen.h_stdout == INVALID_HANDLE_VALUE || screen.h_active == INVALID_HANDLE_VALUE)
    {
        printf("CreateConsoleScreenBuffer failed - (%d)\n", GetLastError());
        exit(1);
    }
    // Make the new screen buffer the active screen buffer.
    if (! SetConsoleActiveScreenBuffer(screen.h_active) )
    {
        printf("SetConsoleActiveScreenBuffer failed - (%d)\n", GetLastError());
        exit(1);
    }
    //screen setup
    screen.rectangle.Top=0;      // screen coordinates 
    screen.rectangle.Left=0;     // screen coordinates
    screen.rectangle.Bottom=24;  // screen coordinates
    screen.rectangle.Right=79;   // screen coordinates
    screen.size.Y=S_SCR_LINES;     // number of screen lines
    screen.size.X=S_SCR_COLUMNS;   // number of screen columns
    screen.buffer_coord.X=0;
    screen.buffer_coord.Y=0;
}

void s_screen_hide_cursor()
{
   CONSOLE_CURSOR_INFO info;
   info.dwSize = 100;
   info.bVisible = FALSE;
   SetConsoleCursorInfo(screen.h_active, &info);
}

void s_screen_show_cursor()
{
   CONSOLE_CURSOR_INFO info;
   info.dwSize = 100;
   info.bVisible = TRUE;
   SetConsoleCursorInfo(screen.h_active, &info);
}

int s_save()
{
    f_player_stats = fopen("playerstats.dat","wb");
    if (f_player_stats != NULL) 
    {
        //fwrite(&players, sizeof(struct STATS), PLAYERS, player_stats);
        fseek(f_player_stats,0,SEEK_SET);
        fwrite(players, sizeof(struct STATS), S_PLAYERS, f_player_stats);
        fclose(f_player_stats);
    }
    else
    {
        printf("Error opening file\n");
        return (1);
    }
}

void s_load()
{
    f_player_stats = fopen("playerstats.dat","rb+");
    if (f_player_stats != NULL)
    {
        fread(players, sizeof(struct STATS), S_PLAYERS, f_player_stats);
        fclose(f_player_stats);
        for(int i =0; i < S_PLAYERS; i++)
        {
            players[i].id = i + 1;
            if (players[i].current == 1)
            {
                current_player = i;
            }
            if ( players[i].empty )  // fill empty profile with '~' so sorting will place it to the end
            {
                strcpy(players[i].name,"~~~~~~~~~~~~~~~~~~~~");
                strcpy(players[i].surname,"~~~~~~~~~~~~~~~~~~~~");
            }
        }
    } 
}

void s_initialize()
{
    // initialize player profiles
    current_player=-1;
    for(int i =0; i < S_PLAYERS; i++)
    {
        players[i].id = i + 1;
        players[i].empty=1;
        strcpy(players[i].name,"~~~~~~~~~~~~~~~~~~~~");
        strcpy(players[i].surname,"~~~~~~~~~~~~~~~~~~~~");
        players[i].current=0;
        players[i].score.total=0;
        for(int r=0; r<3; r++)
        {
            players[i].score.round[r]=0;
        }
    } 
    // open log files
    f_log_file = fopen("mylog.txt", "a");//
    // check if data file exists
    f_player_stats = fopen("playerstats.dat","rb+");
    if(f_player_stats == NULL)
    { // data file does not exists so generate new one
        f_player_stats = fopen("playerstats.dat","wb");
        if(f_player_stats != NULL) 
        {
            s_save();
            fclose(f_player_stats);
        }
        else
        {
            printf("Unable to create data file.");
            exit(1);
        }
    }
    else
    {  // data file exist so close it for now
        fclose(f_player_stats);
    }
    // reset sorting index
    s_reset_ndx();
    // mark new start in log file
    s_log("********************************************************************************");
    // initialize screen
    s_screen_init();
    s_screen_hide_cursor();
}

void s_close()
{
    fclose(f_log_file);
    // Restore the original active screen buffer.
    s_screen_show_cursor();
    if (! SetConsoleActiveScreenBuffer(screen.h_stdout))
    {
        printf("SetConsoleActiveScreenBuffer failed - (%d)\n", GetLastError());
        exit(1);
    }    
}

void s_show_status()
{
    char tmp_str[81];
    s_screen_printxy(2,21,"Profile:                                                            Speed:    ",S_SCR_COL_STATUS);
    s_screen_printxy(2,22,"Score:                                                             Length:    ",S_SCR_COL_STATUS);
    s_screen_printxy(2,23,"                                                                              ",S_SCR_COL_STATUS);
    s_screen_printxy(2,24,"                                                                              ",S_SCR_COL_STATUS);
    s_screen_printxy(1,25," Coder: Martin Musec, 2A1                                              (c) 2023 ",GRAY+BG_WHITE);
    if(current_player != -1)
    {
        if ( !players[current_player].empty )
        {
            sprintf(tmp_str,"%s %s", players[current_player].name, players[current_player].surname);
            s_screen_printxy(11,21,tmp_str,S_SCR_COL_STATUS);
            sprintf(tmp_str,"%5d", players[current_player].score.total);
            s_screen_printxy(11,22,tmp_str,S_SCR_COL_STATUS);
            sprintf(tmp_str,"%3d", snake.speed);
            s_screen_printxy(77,21,tmp_str,S_SCR_COL_STATUS);
            sprintf(tmp_str,"%3d", snake.length);
            s_screen_printxy(77,22,tmp_str,S_SCR_COL_STATUS);
        }
     }
    s_screen_buffer_flush();
}


void s_show_profiles(char *title)
{
    int i;
    char tmp_str[80];
    s_screen_clear();
    s_screen_printxy(34,3,title,S_SCR_COL_MONOCHROME);
    s_screen_printxy(8,4,"                                              Total  Round         ",S_SCR_COL_MONOCHROME);
    s_screen_printxy(8,5,"ID  Name                 Surname              Score  1     2     3 ",S_SCR_COL_MONOCHROME);
    memset(tmp_str,S_SCR_LINE,66); tmp_str[66]='\0';
    s_screen_printxy(8,6,tmp_str,S_SCR_COL_MONOCHROME);
    for(i = 0; i<S_PLAYERS; i++)
    {
        sprintf(tmp_str,"%-3d %-20s %-20s %-5d  %-5d %-5d %-5d",players[ndx[i]].id, players[ndx[i]].name, players[ndx[i]].surname, players[ndx[i]].score.total, players[ndx[i]].score.round[0], players[ndx[i]].score.round[1], players[ndx[i]].score.round[2]);
        s_screen_printxy(8,7+i,tmp_str,S_SCR_COL_MONOCHROME);
        s_log(tmp_str);
    }
    s_show_status();
    s_screen_buffer_flush();
    //printxy(17,13,"(You can select from profiles by pressing 1-6)");
    //s_wait_for_any_key();
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
                s_screen_clear();
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
            for (int i=0; i < S_PLAYERS-1; i++)
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
    else if (attr == 2)  // Sort by Name
    {
        while(change)
        {
            change = 0;
            for (int i=0; i < S_PLAYERS-1; i++)
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
            for (int i=0; i < S_PLAYERS-1; i++)
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
    else if (attr == 5) // Sort by Total Score
    {
        while(change)
        {
            change = 0;
            for (int i=0; i < S_PLAYERS-1; i++)
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
    else
    {
        s_reset_ndx();
    }
}

int s_main_menu(void)
{
    int selected;
    char *t;
    
    s_screen_clear();
    //printxy(0,0,"Pixel united");
    s_screen_printxy(36,4,"SNAKE",S_SCR_COL_MONOCHROME);
    s_screen_printxy(10,5,"1. Start game",S_SCR_COL_MONOCHROME);
    s_screen_printxy(10,6,"2. High Scores",S_SCR_COL_MONOCHROME);
    s_screen_printxy(10,7,"3. Select Profile",S_SCR_COL_MONOCHROME);
    s_screen_printxy(10,9,"0. Exit",S_SCR_COL_MONOCHROME);
    s_screen_buffer_flush();
    s_show_status();
    selected = s_menu_selection(0, 3);
    return(selected);
} 

int s_enter_player(int p_id)
{
    char tmp_str[10];
    sprintf(tmp_str,"ID: %-3d", p_id+1);
    s_screen_printxy(15,14,tmp_str,S_SCR_COL_MONOCHROME);
    s_screen_printxy(15,16,"Surname: ",S_SCR_COL_MONOCHROME);
    s_screen_printxy(15,15,"Name:    ",S_SCR_COL_MONOCHROME);
    s_screen_buffer_flush();
    
    // enter new player name 
    SetConsoleCursorPosition(screen.h_active, (COORD){23,14});
    s_screen_show_cursor();
    scanf(" %s", players[p_id].name);                           // leading sero in formatter supress spaces and new lines
    sprintf(tmp_str,"Name:    %-20s", players[p_id].name);      // print entered name
    s_screen_printxy(15,15,tmp_str,S_SCR_COL_MONOCHROME);       // print Name to screen buffer
    s_screen_hide_cursor();                                     
    s_screen_buffer_flush();                                    // display screen buffer

    // enter new player surname
    SetConsoleCursorPosition(screen.h_active, (COORD){23,15});
    s_screen_show_cursor();
    scanf(" %s", players[p_id].surname);                        // leading sero in formatter supress spaces and new lines
    sprintf(tmp_str,"Surname: %-20s", players[p_id].surname);   // print entered surname
    s_screen_printxy(15,16,tmp_str,S_SCR_COL_MONOCHROME);       // print Name to screen buffer
    s_screen_hide_cursor();                                     
    s_screen_buffer_flush();                                    // display screen buffer

    players[p_id].id = p_id+1;
    players[p_id].empty = 0;
    players[p_id].score.total = 0;
    for(int i =0; i <= 2; i++)
    {
        players[p_id].score.round[i] = 0;
    }

    s_screen_printxy(15,17,"Press any key to continue...",S_SCR_COL_MONOCHROME);
    s_screen_buffer_flush();
    s_wait_for_any_key();
    return 0;
}

void s_select_profile()
{
    int selection = 0;
    char pressed;

    s_show_profiles("Select profile:");
    
    selection = s_menu_selection(0,S_PLAYERS);
    if(selection != 0)
    {
        // check if selected profile is empty
        if (players[selection-1].empty)
        {
            s_enter_player(selection-1);
        }
        else // if profile is not empty ask if profile will be overwritten
        {
            s_screen_printxy(6,15,"Overwrite existing profile? [Y/N]",S_SCR_COL_MONOCHROME);
            s_screen_buffer_flush();
            do
            {
                pressed = s_wait_for_any_key();
                pressed = tolower(pressed);
            } while (!(pressed == 'y' || pressed == 'n'));
            s_screen_printxy(6,15,"                                 ",S_SCR_COL_MONOCHROME);
            s_screen_buffer_flush();
            if (pressed == 'y')
            {
                s_enter_player(selection-1);
            }
        }
        players[current_player].current = 0;  // unselect current pplayer profile
        current_player=selection-1;           // set selected profile as current profile
        players[selection-1].current = 1;     // set 'current' flag on selected player profile
        s_show_status();             // show profile
    }
}

int s_sort_round()
{
    int round = 1;
    s_screen_printxy(6,17,"Sort by round number [1-3]:",S_SCR_COL_MONOCHROME);
    s_screen_buffer_flush();
    round = s_menu_selection(1,3)+40;
    return round;
}

void s_show_leaderboard()
{
    int selection = 0;
    while (1)
    {
        s_show_profiles("Leaderboard");
        s_screen_printxy(6,15,"Sort by:",S_SCR_COL_MONOCHROME);
        s_screen_printxy(6,16,"1. Surname   2. Name      3. ID        4. Round     5. Total score",S_SCR_COL_MONOCHROME);
        s_screen_printxy(6,17,"0. Back                                                           ",S_SCR_COL_MONOCHROME);
        s_show_status();
        s_screen_buffer_flush();
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
//     int i;
    
//     do
//     {
//         srand ( time(NULL) );
//         foodXY[0] = rand() % (width-2) + 2;
//         srand ( time(NULL) );
//         foodXY[1] = rand() % (height-6) + 2;
//     } while (collisionSnake(foodXY[0], foodXY[1], snakeXY, snakeLength, 0)); //This should prevent the "Food" from being created on top of the snake. - However the food has a chance to be created ontop of the snake, in which case the snake should eat it...

//     gotoxy(foodXY[0] ,foodXY[1]);
//     printf("%c", FOOD);
    
//     return(0);
// }

// int s_get_game_speed(void)
// {
//     int speed;
//     s_screen_clear();
    
//     do
//     {
//         gotoxy(10,5);
//         printf("Select The game speed between 1 and 9.");
//         speed = s_wait_for_any_key()-48;
//     } while(speed < 1 || speed > 9);
//     return(speed);
// }

void s_load_game()
{
    // int snakeXY[2][SNAKE_ARRAY_SIZE]; //Two Dimentional Array, the first array is for the X coordinates and the second array for the Y coordinates
    
    // int snakeLength = 4; //Starting Length
    
    // int direction = LEFT_ARROW; //DO NOT CHANGE THIS TO RIGHT ARROW, THE GAME WILL INSTANTLY BE OVER IF YOU DO!!! <- Unless the prepairSnakeArray function is changed to take into account the direction....
    
    // int foodXY[] = {5,5};// Stores the location of the food
    
    // int score = 0;
    //int level = 1;
    
    //Window Width * Height - at some point find a way to get the actual dimensions of the console... <- Also somethings that display dont take this dimentions into account.. need to fix this...
    // int consoleWidth = 80;
    // int consoleHeight = 25;
    
    // int speed = s_get_game_speed();
    
    //The starting location of the snake
    // snakeXY[0][0] = 40; 
    // snakeXY[1][0] = 10;
    s_screen_clear();
    if (current_player == -1 || players[current_player].empty )
    {
        s_screen_printxy(6,15,"Please select player profile first...",S_SCR_COL_MONOCHROME);
        s_screen_buffer_flush();
        s_wait_for_any_key();
    }
    else
    {
        // loadEnviroment(consoleWidth, consoleHeight); //borders
        // prepairSnakeArray(snakeXY, snakeLength);
        // loadSnake(snakeXY, snakeLength);
        // s_generate_food( foodXY, consoleWidth, consoleHeight, snakeXY, snakeLength);
        // refreshInfoBar(score, speed); //Bottom info bar. Score, Level etc
        // startGame(snakeXY, foodXY, consoleWidth, consoleHeight, snakeLength, direction, score, speed);
    }
    return;
}

int s_exit(void)
{
    char pressed;

    s_screen_printxy(10,15,"Are you sure you want to exit(Y/N)",S_SCR_COL_MONOCHROME);
    s_screen_buffer_flush();
    
    do
    {
        pressed = s_wait_for_any_key();
        pressed = tolower(pressed);
    } while (!(pressed == 'y' || pressed == 'n'));
    
    if (pressed == 'y')
    {
        s_save();
        s_close();
        exit(0);
    }
}

int main()
{
    s_initialize();
    s_load();
    //s_wait_for_any_key(); 
    do
    {    
        switch(s_main_menu())
        {
            case 1:
                s_load_game();
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
    //s_wait_for_any_key();  
    return 0;
}
