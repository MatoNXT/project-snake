//      ______   ____  _____       _       ___  ____   ________  
//    .' ____ \ |_   \|_   _|     / \     |_  ||_  _| |_   __  | 
//    | (___ \_|  |   \ | |      / _ \      | |_/ /     | |_ \_| 
//     _.____`.   | |\ \| |     / ___ \     |  __'.     |  _| _  
//    | \____) | _| |_\   |_  _/ /   \ \_  _| |  \ \_  _| |__/ | 
//     \______.'|_____|\____||____| |____||____||____||________|                                                          
//  
//  TODO:
//  - suppress new lines while entering name/surname
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

#define KB_ARROW_UP 72
#define KB_ARROW_LEFT 75
#define KB_ARROW_RIGHT 77
#define KB_ARROW_DOWN 80
#define KB_ENTER 13
#define KB_ESC 27 //ESC
#define KB_P 112 //P

#define S_PLAYERS 6
#define S_DEFAULT_SPEED 250
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

#define S_BODY_MAX_LENGTH 500
#define S_BODY_LINE (char)178
#define S_BODY_HEAD (char)219
#define S_BODY_LINE_COL S_SCR_COL_MONOCHROME

#define S_FOOD_COUNT 10
#define S_FOOD (char)234
#define S_FOOD_COL LIGHT_GREEN+BG_BLACK

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

struct SNAKE_BODY
{
    int col;
    int lin;
    char chr;
    int att;
};

struct FOOD
{
    int col;
    int lin;
    char chr;
    int att;
};


struct SNAKE
{
    struct SNAKE_BODY body[S_BODY_MAX_LENGTH+2];
    int speed;
    int length;
    int direction;
};

struct SCREEN screen;
struct STATS players[S_PLAYERS];
struct SNAKE snake;
struct FOOD food[S_FOOD_COUNT];
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

void s_printf_xy(int x, int y, const char *s)
{
    COORD coord = { x, y};
    SetConsoleCursorPosition(screen.h_active, coord);
    printf("%s",s);
}

void s_screen_printxy_chr(int col, int lin, const char s, int attr)
{
    int position = (lin-1)*80+(col-1);
    screen.buffer[position].Char.AsciiChar=s;
    screen.buffer[position].Attributes=attr;
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
    s_screen_printxy(1,25," Coder: Martin Musec, 2a1                                              (c) 2023 ",GRAY+BG_WHITE);
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

int s_menu_selection(int sel_from, int sel_to)
{
    int pressed;
    
    while(1)
    {
        if(kbhit())
        {
            pressed=getch();
            if (pressed == KB_ESC)
            {
                s_screen_clear();
                pressed = 0;
                break;
            }
            if (pressed >= sel_from+48 && pressed <= sel_to+48)
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
    scanf(" %s", players[p_id].name);                           // leading space in formatter should suppress spaces and new lines
    sprintf(tmp_str,"Name:    %-20s", players[p_id].name);      // print entered name
    s_screen_printxy(15,15,tmp_str,S_SCR_COL_MONOCHROME);       // print Name to screen buffer
    s_screen_hide_cursor();                                     
    s_screen_buffer_flush();                                    // display screen buffer

    // enter new player surname
    SetConsoleCursorPosition(screen.h_active, (COORD){23,15});
    s_screen_show_cursor();
    scanf(" %s", players[p_id].surname);                        // leading space in formatter should suppress spaces and new lines
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

int s_food_collision(int col, int lin)
{
    // check collision with snake body
    for(int s = 0; s<= snake.length; s++)
    {
        if (snake.body[s].col==col && snake.body[s].lin==lin)
        {
            return 1;
        }
    }
    // check collision with existing food
    for(int f = 0; f< S_FOOD_COUNT; f++)
    {
        if (food[f].col != 0 && food[f].lin != 0) // do not check eaten food
        {
            if (food[f].col==col && food[f].lin==lin)
            {
                return 1;
            }
        }
    }
    return 0;
};

void s_food_generate()
{
	int r_lin,r_col;
	for (int f = 0; f < S_FOOD_COUNT; f++)
    {
        // only generate food for not existing or eaten one
        if (food[f].col == 0 && food[f].lin == 0)
        {
            do // keep generate position for food unril there is no collision
            {
                r_col = rand() % (S_SCR_COLUMNS-2) + 2;
                r_lin = rand() % (S_SCR_LINES-7) + 2;
            } while (s_food_collision(r_col,r_lin));
            food[f].col = r_col;
            food[f].lin = r_lin;        
            s_screen_printxy_chr(food[f].col,food[f].lin,food[f].chr,food[f].att);
        }
    };
    s_screen_buffer_flush();
};

void s_snake_prepare(int col, int lin, int length, int direction)
{
    snake.length = length;
    for(int s = 0; s < length; s++ )
    {
        snake.body[s].col = col-s;
        snake.body[s].lin = lin;
        snake.body[s].chr = (char)S_BODY_LINE;
        snake.body[s].att = S_BODY_LINE_COL;
    };
    snake.body[0].chr = (char)S_BODY_HEAD;
    snake.speed = 0;
    snake.direction = direction;
    // prepare tail - tail will be clearing last part of snake whem moving
    snake.body[length].col = col-length;
    snake.body[length].lin = lin;
    snake.body[length].chr = (char)S_SCR_SPACE;
    snake.body[length].att = S_SCR_COL_MONOCHROME;
    // prepare food
    for(int f = 0; f< S_FOOD_COUNT; f++)
    {
        food[f].col=0;
        food[f].lin=0;
        food[f].chr=S_FOOD;
        food[f].att=S_FOOD_COL;
    }
};

void s_snake_show()
{
    for(int s = 0; s <= snake.length; s++)
    {
        s_screen_printxy_chr(
            snake.body[s].col,
            snake.body[s].lin,
            snake.body[s].chr,
            snake.body[s].att
        );
    }
};

int s_snake_food_collision()
{
    int collision = 0;

    for(int f = 0; f< S_FOOD_COUNT; f++)
    {
        if (food[f].col != 0 && food[f].lin != 0) // do not check eaten food
        {
            if (food[f].col==snake.body[0].col && food[f].lin==snake.body[0].lin)
            {
                // expand body only if not maximum length
                if (snake.length<S_BODY_MAX_LENGTH)
                {
                    collision=1;
                    snake.length+=1;
                    if (snake.speed<=S_DEFAULT_SPEED-5){
                        snake.speed+=10;
                    }
                }
                // mark food as eaten
                food[f].col=0;
                food[f].lin=0;
            }
        }
    }
    s_food_generate();
    return collision;
};

int s_snake_body_collision()
{
    for(int s = 1; s<= snake.length; s++)
    {
        if (snake.body[s].col==snake.body[0].col && snake.body[s].lin==snake.body[0].lin)
        {
            return 1;
        }
    }
    return 0;
};

int s_snake_wall_collision()
{
    if (snake.body[0].col==1 || snake.body[0].col==S_SCR_COLUMNS)
    {
        return 1;
    }
    if (snake.body[0].lin==1 || snake.body[0].lin==S_SCR_LINES-5)
    {
        return 1;
    }
    return 0;
};

int s_snake_collision()
{
    if (s_snake_body_collision())
    {
        return 1;
    }
    if (s_snake_wall_collision())
    {
        return 1;
    }
    if (s_snake_food_collision())
    {
        return 2;
    }
    return 0;
};

int s_snake_move()
{
    int collision = 0;
    struct SNAKE_BODY head;
    
    // remember old head position for later
    head=snake.body[0];
    // move head to new position
    switch (snake.direction)
    {
        case KB_ARROW_RIGHT:
                snake.body[0].col+=1;
            break;
        case KB_ARROW_LEFT:
                snake.body[0].col-=1;
            break;
        case KB_ARROW_UP:
                snake.body[0].lin-=1;
            break;
        case KB_ARROW_DOWN:
                snake.body[0].lin+=1;
            break;
        default:
            break;
    }
    // now check where is head
    collision = s_snake_collision();
    if (collision==0)
        {
        // move whole body - start from tail
        for(int s = snake.length; s > 1; s--)
        {
            snake.body[s].col = snake.body[s-1].col;
            snake.body[s].lin = snake.body[s-1].lin;
        }
        // move fist body part to head old position
        snake.body[1].col = head.col;
        snake.body[1].lin = head.lin;
    }
    else if (collision==2) // food eaten
    {
        // expand body 1
        for(int s = snake.length; s > 1; s--)
        {
            snake.body[s] = snake.body[s-1];
        }
        // move new body part to head old position
        snake.body[1].col = head.col;
        snake.body[1].lin = head.lin;
        snake.body[1].chr=S_BODY_LINE;
        snake.body[1].att=S_BODY_LINE_COL;
        // reset colision as this was just food so continue in game
        collision=0;
    }
    return collision;
};

int s_snake_key_pressed()
{
	int pressed;
	
	if(kbhit())
	{
		while (kbhit()) // this loop is to clear keyboaard buffer
        {
            pressed=getch();
        }
        if (pressed==KB_ESC)
            {
                pressed=-1;
            }
        else
        {
            // if getch return 224 or 0, read keyboard once more time
            if (pressed == 224 || pressed == 0)
            {
                pressed=getch();
            }
            if (snake.direction != pressed)
            {
                if(pressed == KB_ARROW_DOWN && snake.direction != KB_ARROW_UP)
                {
                    snake.direction = pressed;
                }
                else if (pressed == KB_ARROW_UP && snake.direction != KB_ARROW_DOWN)
                {
                    snake.direction = pressed;
                }
                else if (pressed == KB_ARROW_LEFT && snake.direction != KB_ARROW_RIGHT)
                {
                    snake.direction = pressed;
                }
                else if (pressed == KB_ARROW_RIGHT && snake.direction != KB_ARROW_LEFT)
                {
                    snake.direction = pressed;
                }
                // else if (pressed == KB_P)
                // 	pauseMenu();
            }
        }
	}
    return pressed;
};

void s_game_start()
{
    int game_over = 0;
    int pressed;

    do
    {
        pressed = s_snake_key_pressed();
        game_over=s_snake_move();
        if (!game_over)
        {
            s_snake_show();
            s_show_status();
            s_screen_buffer_flush();
            Sleep(S_DEFAULT_SPEED-snake.speed);
            if (pressed == KB_ESC)
            {
                //game_over = 1;
            }
        }
    } while (!game_over);
};

void s_load_game()
{
    s_screen_clear();
    if (current_player == -1 || players[current_player].empty )
    {
        s_screen_printxy(6,15,"Please select player profile first...",S_SCR_COL_MONOCHROME);
        s_screen_buffer_flush();
        s_wait_for_any_key();
    }
    else
    {
        s_snake_prepare(40,10,20,KB_ARROW_RIGHT);
        s_snake_show();
        s_food_generate();
        s_show_status();
        s_screen_buffer_flush();
        s_game_start();
    }
    return;
}

int s_exit(void)
{
    char pressed;

    s_screen_printxy(23,15,"Are you sure you want to exit(Y/N)",S_SCR_COL_MONOCHROME);
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
