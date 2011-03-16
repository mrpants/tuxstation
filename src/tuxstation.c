/*******************************************************************************
*                                                                              *
*  TuxStation - an emulator frontend                                           *
*                                                                              *
*  Copyright (C) 2004 Mr. Pants                                                *
*                                                                              *
*  This program is free software; you can redistribute it and/or modify        *
*  it under the terms of the GNU General Public License as published by        *
*  the Free Software Foundation; either version 2, or (at your option)         *
*  any later version.                                                          *
*                                                                              *
*  This program is distributed in the hope that it will be useful,             *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of              *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
*  GNU General Public License for more details.                                *
*                                                                              *
*  You should have received a copy of the GNU General Public License           *
*  along with this program; if not, write to the                               *
*  Free Software Foundation, Inc., 59 Temple Place - Suite 330,                *
*  Boston, MA 02111-1307, USA.                                                 *
*                                                                              *

TO DO:

int cursorPos[10] - anyway to make that empty?

*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>

#include "tuxconfig.h"

#define VERSION "0.1"

/* Variable declarations */
SDL_Surface *mainScreen;
SDL_Surface *mainScreenBack;
SDL_Surface *border;
SDL_Surface *cursor;
SDL_Surface *title;
SDL_Surface *console_games;
SDL_Joystick *joystick1;
SDL_Joystick *joystick2;
SDL_Color fontColor;
SDL_Color *forecol = &fontColor;
SDL_Rect title_drect;
SDL_Rect cursor_drect;
SDL_Rect console_games_drect;
SDL_PixelFormat *fmt;
SDL_Event event;
TTF_Font *font;
Mix_Music *music = NULL;

int cursorPos[10];
int numCursors;
int distance;
int currentPosition = 0;
int tempPosition = 0;
int systemNumber;
int done = 0;
int maxCursor;
int minCursor = 0;
int maxGames;
int minGames = 0;
char games[1000][1000];
char gamenames[1000][1000];
int onMenu;
int menuCounter = 0;
int gameCounter = 0;
int tempCount;

/* Function declarations */
int init_frontend(void);
void init_video(void);
void init_ttf(void);
void init_input(void);
void init_sound(void);
void toggleMusic(void);
void genLists(void);
void loadConsoles(void);
void render(void);
void clearApp(void);
void loadGames(void);
void playGame(void);
void mainLoop(void);
void handleUp(void);
void handleDown(void);
void handleLeft(void);
void handleRight(void);
void handleFwd(void);
void handleBack(void);
void scrollDown(void);
void scrollUp(void);
void pageDown(void);
void pageUp(void);
void handleExit(void);
int quit(void);


/*******************************************************************************
*                                                                              *
*  init_frontend()                                                             *
*                                                                              *
*    Initializes SDL, calls the rest of the init_* subs, and renders the app.  *
*                                                                              *
*******************************************************************************/

int init_frontend()
{
    printf("TuxStation v%s\n\n", VERSION);

    printf("Initializing SDL.. ");

    /* Initialize defaults, Video, Audio and Joystick */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) == -1)
    {
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
        exit(-1);
    }

    printf("SDL initialized.\n");

    init_video();

    init_ttf();

    init_input();

    if (cfg_music_bool == 1)
    {
        init_sound();
    }
    else
    {
        printf("Not initializing sound.\n");
    }

    onMenu = 1;
    menuCounter = 0;
    
    /* Generate lists */
    if (cfg_autoscan_bool == 1)
    {
        genLists();
    }
    else
    {
        printf("Not generating ROM lists.\n");
    }

    if (cfg_music_bool == 1)
    {
        toggleMusic();
    }
    
    /* Load consoles */
    loadConsoles();

    /* Make a call to render() to get all of this rendered */
    render();
}

/*******************************************************************************
*                                                                              *
*  init_video()                                                                *
*                                                                              *
*    Initializes SDL's video, loads the border and cursor graphics, and inits  *
*    cursor positions.                                                         *
*                                                                              *
*******************************************************************************/

void init_video()
{
    printf("Initializing video.. ");
    
    /* Set up main surface */
    if (cfg_fullscreen_bool == 1)
    {
        mainScreen = SDL_SetVideoMode(cfg_hres_int,cfg_vres_int,cfg_bpp_int,
                                                 SDL_SWSURFACE |
                                                 SDL_DOUBLEBUF |
                                                 SDL_FULLSCREEN);
    }
    else
    {
        mainScreen = SDL_SetVideoMode(cfg_hres_int,cfg_vres_int,cfg_bpp_int,
                                                 SDL_SWSURFACE |
                                                 SDL_DOUBLEBUF);
    }

    if (mainScreen == NULL)
    {
        fprintf(stderr, "Couldn't set %ix%ix%i video mode: %s\n",
                        cfg_hres_int,
                        cfg_vres_int,
                        cfg_bpp_int,
                        SDL_GetError());
        exit(-1);
    }

    /* Set up border surface */
    border = IMG_Load(cfg_border_file_str);
    
    /* Set up cursor surface */
    cursor = IMG_Load("gfx/cursor.png");
    
    printf("video initialized.\n");
}

/*******************************************************************************
*                                                                              *
*  init_ttf()                                                                  *
*                                                                              *
*    Initializes SDL's TTF lib, loads the font.                                *
*                                                                              *
*******************************************************************************/

void init_ttf()
{
    int i;
    
    printf("Initializing fonts.. ");

    /* Initialize the TTF library */
    if (TTF_Init() < 0)
    {
        printf("\nCouldn't initialize TTF: %s\n",SDL_GetError());
    }

    /* Open the font file with the requested point size */
    font = TTF_OpenFont(cfg_font_file_str, cfg_font_size_int);
    
    if (font == NULL)
    {
        printf("\nCouldn't load %i pt font from %s: %s\n",
                cfg_font_size_int, cfg_font_file_str, SDL_GetError());
    }

    fontColor.r = cfg_font_color_r_uint;
    fontColor.g = cfg_font_color_g_uint;
    fontColor.b = cfg_font_color_b_uint;
    fontColor.unused = 0;

    /* Get size of font, and set up cursorPos[] and X,Y distances. */
    if (cfg_font_size_int >= 20 && cfg_font_size_int < 30)
    {
        numCursors = 7;
        distance = 60;
    }
    else if (cfg_font_size_int >= 30 && cfg_font_size_int < 40)
    {
        numCursors = 6;
        distance = 70;
    }
    else if (cfg_font_size_int >= 40 && cfg_font_size_int <= 50)
    {
        numCursors = 5;
        distance = 80;
    }
    else
    {
        printf("\n\nBad font size.  Please choose one from 20 to 50.\n");
        quit();
    }

    /* Set up cursor positions. */
    cursorPos[0] = 100;
    
    for (i = 1; i <= numCursors; i++)
    {
        cursorPos[i] = cursorPos[i-1] + distance;
    }

    printf("fonts initialized.\n");
}

/*******************************************************************************
*                                                                              *
*  init_input()                                                                *
*                                                                              *
*    Initializes SDL's joystick, loads joystick(s), and hides mouse cursor.    *
*                                                                              *
*******************************************************************************/

void init_input()
{
    int numJoy = SDL_NumJoysticks();
    
    printf("Initializing input.. ");

    /* Hide mouse cursor */
    SDL_ShowCursor(SDL_DISABLE);

    if (numJoy == 0)
    {
        printf("no joysticks found, using keyboard.\n");
    }
    else if (numJoy == 1)
    {
        printf("%i joystick(s) found.. ", numJoy);

        SDL_JoystickEventState(SDL_ENABLE);
        
        joystick1 = SDL_JoystickOpen(0);

        printf("input initialized.\n");
    }
    else if (numJoy == 2)
    {
        printf("%i joystick(s) found.. ", numJoy);

        SDL_JoystickEventState(SDL_ENABLE);

        joystick1 = SDL_JoystickOpen(0);
        joystick2 = SDL_JoystickOpen(1);

        printf("input initialized.\n");
    }
    else
    {
        printf("%i joystick(s) found.. unsupported number.\n", numJoy);
    }
}

/*******************************************************************************
*                                                                              *
*  init_sound()                                                                *
*                                                                              *
*    Initializes SDL's mixer lib, and sets default values.  Also calls the     *
*    toggleMusic() function to start the music playing.                        *
*                                                                              *
*******************************************************************************/

void init_sound()
{
    int audio_rate = 22050;
    /* int audio_rate = 44100; */
    Uint16 audio_format = AUDIO_S16LSB;
    /*Uint16 audio_format = MIX_DEFAULT_FORMAT;*/
    int audio_channels = 2;
    int audio_buffers = 4096;

    printf("Initializing sound.. ");
    
    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers))
    {
        printf("no sound device available!\n");
    }
    else
    {
        printf("sound initialized.\n");
    }
}

/*******************************************************************************
*                                                                              *
*  toggleMusic()                                                               *
*                                                                              *
*    Loads / unloads the OGG file and starts / stops playing the music.        *
*                                                                              *
*******************************************************************************/

void toggleMusic()
{
    int numtimesopened, frequency, channels, counter;
    Uint16 format;
    
    if (music == NULL)
    {
        /* Call init_sound() again.
           This is needed after shutting off the music and
           freeing the sound device. */
        init_sound();
        
        music = Mix_LoadMUS(cfg_music_file_str);
        if (music == NULL)
          printf("Can't load sound file!\n");
        else
          Mix_PlayMusic(music, 99);
    }
    else
    {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        music = NULL;
        
        /* Need to close the mixer to free the sound device. */
        numtimesopened = Mix_QuerySpec(&frequency, &format, &channels);
        for(counter = 0; counter < numtimesopened; counter++)
            Mix_CloseAudio();
    }
}

/*******************************************************************************
*                                                                              *
*  genLists()                                                                  *
*                                                                              *
*    If set in tuxstation.conf, this will auto-scan ROM directories, and       *
*    create lists of games for each system.                                    *
*                                                                              *
*******************************************************************************/

void genLists()
{
    int i;
    char cmd[1024];
    char *str;

    printf("Generating ROM lists.. ");

    /* This is in case the ~/.tuxstation/data directory doesn't exist. */
    str = (char *)malloc(strlen(homedir) + 6);
    strcpy(str, homedir);
    strcat(str, "/data");
    mkdir(str, 0755);
    
    for (i = 0; i < ini_num_sections; i++)
    {
        if (ini_system_autogen_int[i] == 1)
        {
            sprintf(cmd, "ls %s > %s",
                    ini_system_romdir_str[i],
                    ini_system_gamelist_str[i]);
            system(cmd);
        }
    }

    printf("done.\n");
}

/*******************************************************************************
*                                                                              *
*  loadConsoles()                                                              *
*                                                                              *
*    Using the tuxstation.conf file, this will print all systems to the main   *
*    screen.                                                                   *
*                                                                              *
*******************************************************************************/

void loadConsoles()
{
    int i;
    int x_pos = 80;
    int y_pos = 100;
    int font_renderstyle;
    static int systemCount;
    static int for_counter;

    onMenu = 1;
    
    clearApp();

    /* In case number of consoles is greater than number of cursors. */
    if (menuCounter == 0)
    {
        if ((ini_num_sections - 1) < numCursors)
            systemCount = ini_num_sections - 1;
        else
            systemCount = numCursors;

        tempCount = systemCount;
        for_counter = 0;
    }

    /* In case number of consoles is greater than number of cursors AND
       we've already scrolled down the list. */
    if (menuCounter > systemCount || menuCounter < for_counter)
    {
        for_counter = menuCounter - numCursors;
        systemCount = menuCounter;
        
        if (for_counter < 0)
        {
            for_counter = 0;
            systemCount = tempCount;
        }
    }

    /* Print out the consoles, or at least the first batch of them. */
    for (i = for_counter; i <= systemCount; i++)
    {
        font_renderstyle = TTF_STYLE_NORMAL;
        TTF_SetFontStyle(font, font_renderstyle);
        console_games = TTF_RenderText_Solid(font, ini_system_name_str[i],
	                                     *forecol);
        if (console_games == NULL)
        {
            fprintf(stderr, "Couldn't render text: %s\n", SDL_GetError());
            TTF_CloseFont(font);
        }	
        else
        {
            console_games_drect.x = x_pos;
            console_games_drect.y = y_pos;
            console_games_drect.w = console_games->w;
            console_games_drect.h = console_games->h;
								        
            if (SDL_BlitSurface(console_games, NULL, mainScreen,
	                        &console_games_drect) < 0)
            {
                fprintf(stderr, "Couldn't blit text to display: %s\n",
                                 SDL_GetError());
                TTF_CloseFont(font);
            }
            SDL_FreeSurface(console_games);
        }
        y_pos = y_pos + distance;
    }

    if (numCursors > (ini_num_sections - 1))
    {
        maxCursor = ini_num_sections - 1;
    }
    else
    {
        maxCursor = numCursors;
    }
    
    render();
}

/*******************************************************************************
*                                                                              *
*  render()                                                                    *
*                                                                              *
*    All of the video blitting is done here.  The border is blitted, then the  *
*    title of the window (after setting the font style and location), then the *
*    cursor is redrawn, and finally the whole app is updated.                  *
*                                                                              *
*******************************************************************************/

void render()
{
    int font_renderstyle;
    char *title_text;

    /* Blit the border */
    SDL_BlitSurface(border, NULL, mainScreen, NULL);
    /*SDL_FreeSurface(border);*/

    /* Check if we're on the console menu or not */

    /* Print the title */
    title_text = "TuxStation";
    font_renderstyle = TTF_STYLE_UNDERLINE;

    TTF_SetFontStyle(font, font_renderstyle);

    title = TTF_RenderText_Solid(font, title_text, *forecol);
    if (title == NULL)
    {
        fprintf(stderr, "Couldn't render text: %s\n", SDL_GetError());
        TTF_CloseFont(font);
    }
    else
    {
        title_drect.x = mainScreen->w / 2 - title->w / 2;
        title_drect.y = 20;
        title_drect.w = title->w;
        title_drect.h = title->h;
        
        if (SDL_BlitSurface(title, NULL, mainScreen, &title_drect) < 0)
        {
            fprintf(stderr, "Couldn't blit text to display: %s\n",
                             SDL_GetError());
            TTF_CloseFont(font);
        }
        SDL_FreeSurface(title);
    }

    /* Start setting up to blit the cursor */
    fmt = mainScreen->format;
    SDL_FillRect(mainScreen, &cursor_drect, SDL_MapRGB(
                                            fmt,
                                            cfg_background_color_r_uint,
                                            cfg_background_color_g_uint,
                                            cfg_background_color_b_uint));

    cursor_drect.w = cursor->w;
    cursor_drect.h = cursor->h;
    cursor_drect.x = 40;
    cursor_drect.y = cursorPos[currentPosition];

    /* Blit the cursor */
    SDL_BlitSurface(cursor, NULL, mainScreen, &cursor_drect);
    /*SDL_FreeSurface(cursor);*/

    /* Update everything on the mainScreen */
    /*SDL_UpdateRect(mainScreen,0,0,0,0);*/
    SDL_Flip(mainScreen);
}

/*******************************************************************************
*                                                                              *
*  clearApp()                                                                  *
*                                                                              *
*    Clears the entire app window.  This is needed when changing from the main *
*    menu to the system menu.                                                  *
*                                                                              *
*******************************************************************************/

void clearApp()
{
    fmt = mainScreen->format;
    SDL_FillRect(mainScreen, NULL, SDL_MapRGB(
                                   fmt,
                                   cfg_background_color_r_uint,
                                   cfg_background_color_g_uint,
                                   cfg_background_color_b_uint));
    render();
}

/*******************************************************************************
*                                                                              *
*  loadGames()                                                                 *
*                                                                              *
*    This will load up the list of ROMs for the selected console.              *
*                                                                              *
*******************************************************************************/

void loadGames()
{
    int buf = 255;
    int count = 0;
    int i;
    int x_pos = 80;
    int y_pos = 100;
    int font_renderstyle;
    FILE *gamelist;
    char *newline;
    char *extention;

    clearApp();

    onMenu = 0;
    gameCounter = 0;

    /* Open the *.lst file for the selected system. */
    gamelist = fopen(ini_system_gamelist_str[systemNumber], "r");

    if (gamelist == NULL)
    {
        printf("Error opening file: %s\n",
               ini_system_gamelist_str[systemNumber]);
    }

    /* Read in the ROMs. */
    while (fgets(games[count], buf, gamelist) != NULL)
    {
        /* Remove any carriage returns or newlines from the string. */
        newline = strrchr(games[count], '\r');
        if (newline)
            *newline = '\0';
        newline = strrchr(games[count], '\n');
        if (newline)
            *newline = '\0';

        count++;
    }
    
    /* Close the file. */
    fclose(gamelist);
    
    /* Count the number of games here. */
    maxGames = count - 1;
    
    /* Load up gamenames[], and also strip off the extention. */
    for (i = 0; i < count; i++)
    {
        strcpy(gamenames[i],games[i]);
        
        /* Start by stripping the extention off the ROM name. */
        extention = strrchr(gamenames[i], '.');
        if (extention)
            *extention = '\0';
    }

    if (count <= maxCursor)
    {
        maxCursor = count - 1;
    }
    else
    {
        maxCursor = numCursors;
    }
    
    /* Now print the first batch of ROM names to the screen. */
    for (i = 0; i <= maxCursor; i++)
    {
        font_renderstyle = TTF_STYLE_NORMAL;
        TTF_SetFontStyle(font, font_renderstyle);
        console_games = TTF_RenderText_Solid(font, gamenames[i], *forecol);
        if (console_games == NULL)
        {
            fprintf(stderr, "Couldn't render text: %s\n", SDL_GetError());
            TTF_CloseFont(font);
        }
        else
        {
            console_games_drect.x = x_pos;
            console_games_drect.y = y_pos;
            console_games_drect.w = console_games->w;
            console_games_drect.h = console_games->h;

            if (SDL_BlitSurface(console_games, NULL, mainScreen,
                                &console_games_drect) < 0)
            {
                fprintf(stderr, "Couldn't blit text to display: %s\n",
                                 SDL_GetError());
                TTF_CloseFont(font);
            }
            SDL_FreeSurface(console_games);
        }
        y_pos = y_pos + distance;
    }

    /* Reset the cursor to the top of the screen. */
    currentPosition = 0;

    gameCounter = 0;

    render();
}

/*******************************************************************************
*                                                                              *
*  playGame()                                                                  *
*                                                                              *
*    This will load up the selected game with its emulator settings.           *
*                                                                              *
*******************************************************************************/

void playGame()
{
    char cmd[1024];
    SDL_Event dumpEvent;
    
    printf("Now playing game..\n");

    /* Turn off the music, if running. */
    if (cfg_music_bool == 1)
    {
        printf("Shutting off music.. ");
        toggleMusic();
        printf("done.\n");
    }

    /* Turn off input. */
    /*SDL_WM_GrabInput(SDL_GRAB_OFF);*/
    
    /* Minimize fullscreen, if on. */
    if (cfg_fullscreen_bool == 1)
    {
        printf("Minimizing fullscreen... ");
        SDL_WM_ToggleFullScreen(mainScreen);
        printf("done.\n");
    }
    
    /* Run the emulator. */
    sprintf(cmd, "%s %s \"%s/%s\" %s",
            ini_system_emu_str[systemNumber],
            ini_system_opts_before_str[systemNumber],
            ini_system_romdir_str[systemNumber],
            games[gameCounter],
            ini_system_opts_after_str[systemNumber]);
    
    system(cmd);
    
    /* Maximize fullscreen, if on. */
    if (cfg_fullscreen_bool == 1)
    {
        printf("Going fullscreen... ");
        SDL_WM_ToggleFullScreen(mainScreen);
        printf("done.\n");
    }
    
    /* Turn on input. */
    /*SDL_WM_GrabInput(SDL_GRAB_ON);*/
    
    /* Render the screen again. */
    render();

    /* Dump the extra events. */
    SDL_Delay(1000);
    SDL_PumpEvents();
    
    while (SDL_PeepEvents(&dumpEvent, 1, SDL_GETEVENT, 0xFFFFFFFF) > 0)
        SDL_PumpEvents();

    /* Tun the music back on, if running before. */
    if (cfg_music_bool == 1)
    {
        printf("Restarting music... ");
        toggleMusic();
        printf("done.\n");
    }
}

/*******************************************************************************
*                                                                              *
*  mainLoop()                                                                  *
*                                                                              *
*    The main loop of the program.  This function waits for events, and then   *
*    chooses which handler to use based on the event.                          *
*                                                                              *
*******************************************************************************/

void mainLoop()
{
    done = 0;

    printf("\nStarting TuxStation\n");
    
    while (!done)
    {
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_JOYAXISMOTION:
                    if ((event.jaxis.value < -3200) || (event.jaxis.value > 3200))
                    {
                        /* Y axis (up and down) */
                        if (event.jaxis.axis == 1) 
                        {
                            /* Up pressed */
                            if (event.jaxis.value < -16384)
                            {
                                /* Check for Joystick 1 ONLY */
                                if (event.jaxis.which == 0)
                                {
                                    handleUp();
                                }
                            }
                            
                            /* Down pressed */
                            else if (event.jaxis.value > 16384)
                            {
                                /* Check for Joystick 1 ONLY */
                                if (event.jaxis.which == 0)
                                {
                                    handleDown();
                                }
                            }
                        }
                        
                        /* X axis (left and right) */
                        if (event.jaxis.axis == 0) 
                        {
                            /* Left pressed */
                            if (event.jaxis.value < -16384)
                            {
                                /* Check for Joystick 1 ONLY */
                                if (event.jaxis.which == 0)
                                {
                                    handleLeft();
                                }
                            }
                            
                            /* Right pressed */
                            else if (event.jaxis.value > 16384)
                            {
                                /* Check for Joystick 1 ONLY */
                                if (event.jaxis.which == 0)
                                {
                                    handleRight();
                                }
                            }
                        }
                    }
                    break;
		    
                case SDL_JOYBUTTONDOWN:
                    /* "A" button */
                    if (event.jbutton.button == 0)
                    {
                        /* Check for Joystick 1 ONLY */
                        if (event.jbutton.which == 0)
                        {
                            handleFwd();
                        }
                    }
                    
                    /* "B" button */
                    if (event.jbutton.button == 1)
                    {
                        /* Check for Joystick 1 ONLY */
                        if (event.jbutton.which == 0)
                        {
                            handleBack();
                        }
                    }
                    
                    /* "X" button */
                    if (event.jbutton.button == 2)
                    {
                        /* Check for Joystick 1 ONLY */
                        if (event.jbutton.which == 0)
                        {
                            toggleMusic();
                        }
                    }
                    
                    /* "Select + Start" to quit */
                    /*if (event.jbutton.button == 6)
                    {
                        if (event.jbutton.button == 7)
                        {
                            Check for Joystick 1 ONLY
                            if (event.jbutton.which == 0)
                            {
                                done = 1;
                                break;
                            }
                        }
                    }*/
                    break;

                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_UP:
                            handleUp();
                            break;

                        case SDLK_DOWN:
                            handleDown();
                            break;

                        case SDLK_LEFT:
                            handleLeft();
                            break;

                        case SDLK_RIGHT:
                            handleRight();
                            break;

                        /* "A" button */
                        case SDLK_x:
                            handleFwd();
                            break;
                        
                        /* "B" button */
                        case SDLK_z:
                            handleBack();
                            break;
                        
                        /* "X" button */
                        case SDLK_s:
                            toggleMusic();
                            break;
			
                        case SDLK_q:
                            handleExit();
                            break;
                        
                        case SDLK_ESCAPE:
                            handleExit();
                            break;
                        
                        default:
                            break;
                    }
                    break;

                case SDL_QUIT:
                    handleExit();
                    break;
            }
        }
    }
}

/*******************************************************************************
*                                                                              *
*  handleUp()                                                                  *
*                                                                              *
*    Moves the cursor up, and scrolls up if necessary.                         *
*                                                                              *
*******************************************************************************/

void handleUp()
{
    if (currentPosition > minCursor)
    {
        currentPosition--;
        if (onMenu == 1)
        {
            menuCounter--;
        }
        else
        {
            gameCounter--;
        }
    }
    else if (currentPosition == minCursor)
    {
        if (onMenu == 1)
        {
            if (menuCounter > 0)
            {
                menuCounter--;  /* Need this first. */
                scrollUp();
            }
        }
        else
        {
            if (gameCounter > minGames)
            {
                gameCounter--;  /* Need this first. */
                scrollUp();
            }
        }
    }
			    
    render();
}

/*******************************************************************************
*                                                                              *
*  handleDown()                                                                *
*                                                                              *
*    Moves the cursor down, and scrolls down if necessary.                     *
*                                                                              *
*******************************************************************************/

void handleDown()
{
    if (currentPosition < maxCursor)
    {
        currentPosition++;
        if (onMenu == 1)
        {
            menuCounter++;
        }
        else
        {
            gameCounter++;
        }
    }
    else if (currentPosition == maxCursor)
    {
        if (onMenu == 1)
        {
            if (menuCounter < ini_num_sections - 1)
            {
                menuCounter++;  /* Need this first. */
                scrollDown();
            }
        }
        else
        {
            if (gameCounter < maxGames)
            {
                gameCounter++;  /* Need this first. */
                scrollDown();
            }
        }
    }
			    
    render();
}

/*******************************************************************************
*                                                                              *
*  handleLeft()                                                                *
*                                                                              *
*    Moves the cursor to the top, and pages up if necessary.                   *
*                                                                              *
*******************************************************************************/

void handleLeft()
{
    if (currentPosition > minCursor)
    {
        if (onMenu == 1)
        {
            menuCounter = menuCounter - currentPosition;
        }
        else
        {
            gameCounter = gameCounter - currentPosition;
        }

        currentPosition = minCursor;
    }
    else if (currentPosition == minCursor)
    {
        if (onMenu == 1)
        {
            if (menuCounter > minGames)
            {
                menuCounter = menuCounter - maxCursor - 1;
                if (menuCounter < minGames)
                {
                    menuCounter = minGames;
                }
                pageUp();
            }
        }
        else
        {
            if (gameCounter > minGames)
            {
                gameCounter = gameCounter - maxCursor - 1;
                if (gameCounter < minGames)
                {
                    gameCounter = minGames;
                }
                pageUp();
            }
        }
    }

    render();
}

/*******************************************************************************
*                                                                              *
*  handleRight()                                                               *
*                                                                              *
*    Moves the cursor to the bottom, and pages down if necessary.              *
*                                                                              *
*******************************************************************************/

void handleRight()
{
    if (currentPosition < maxCursor)
    {
        if (onMenu == 1)
        {
            menuCounter = menuCounter + maxCursor - currentPosition;
        }
        else
        {
            gameCounter = gameCounter + maxCursor - currentPosition;
        }
        
        currentPosition = maxCursor;
    }
    else if (currentPosition == maxCursor)
    {
        if (onMenu == 1)
        {
            if (menuCounter < ini_num_sections - 1)
            {
                menuCounter = menuCounter + maxCursor + 1;
                if (menuCounter > ini_num_sections - 1)
                {
                    menuCounter = ini_num_sections - 1;
                }
                pageDown();
            }
        }
        else
        {
            if (gameCounter < maxGames)
            {
                gameCounter = gameCounter + maxCursor + 1;
                if (gameCounter > maxGames)
                {
                    gameCounter = maxGames;
                }
                pageDown();
            }
        }
    }

    render();
}

/*******************************************************************************
*                                                                              *
*  handleFwd()                                                                 *
*                                                                              *
*    Either goes to the system menu, or starts up an emulator / ROM.           *
*                                                                              *
*******************************************************************************/

void handleFwd()
{
    if (onMenu == 1)
    {
        tempPosition = currentPosition;
        systemNumber = menuCounter;
        loadGames();
    }
    else
    {
        playGame();
    }
}

/*******************************************************************************
*                                                                              *
*  handleBack()                                                                *
*                                                                              *
*    Goes back to the main menu.                                               *
*                                                                              *
*******************************************************************************/

void handleBack()
{
    if (onMenu == 0)
    {
        currentPosition = tempPosition;
        loadConsoles();
    }
}

/*******************************************************************************
*                                                                              *
*  scrollDown()                                                                *
*                                                                              *
*    Moves the game list down 1 game at a time.                                *
*                                                                              *
*******************************************************************************/

void scrollDown()
{
    int maxCount;
    int counter;
    int x_pos = 80;
    int y_pos = 100;
    int font_renderstyle;
    int greaterBool = 0;

    if (onMenu == 1)
        counter = menuCounter;
    else
        counter = gameCounter;

    maxCount = counter;
	counter = counter - maxCursor;

    if (onMenu == 1)
    {
        if (maxCount <= ini_num_sections)
        {
            greaterBool = 1;
        }
    }
    else
    {
        if (maxCount <= maxGames)
        {
            greaterBool = 1;
        }
    }

    if (greaterBool == 1)
    {
        clearApp();
            
        while (counter <= maxCount)
        {
            /* Print next batch of games to screen. */
            font_renderstyle = TTF_STYLE_NORMAL;
            TTF_SetFontStyle(font, font_renderstyle);
            if (onMenu == 1)
            {
                console_games = TTF_RenderText_Solid(
                                               font,
                                               ini_system_name_str[counter],
                                               *forecol);
            }
            else
            {
                console_games = TTF_RenderText_Solid(font,
                                                     gamenames[counter],
                                                     *forecol);
            }
            if (console_games == NULL)
            {
                fprintf(stderr, "Couldn't render text: %s\n",
                                SDL_GetError());
                TTF_CloseFont(font);
            }
            else
            {
                console_games_drect.x = x_pos;
                console_games_drect.y = y_pos;
                console_games_drect.w = console_games->w;
                console_games_drect.h = console_games->h;

                if (SDL_BlitSurface(console_games, NULL, mainScreen,
                                    &console_games_drect) < 0)
                {
                    fprintf(stderr, "Couldn't blit text to display: %s\n",
                                    SDL_GetError());
                    TTF_CloseFont(font);
                }
                SDL_FreeSurface(console_games);
            }
            y_pos = y_pos + distance;

            counter++;
        }
    }
}

/*******************************************************************************
*                                                                              *
*  scrollUp()                                                                  *
*                                                                              *
*    Moves the game list up 1 game at a time.                                  *
*                                                                              *
*******************************************************************************/

void scrollUp()
{
    int maxCount;
    int counter;
    int x_pos = 80;
    int y_pos = 100;
    int font_renderstyle;
    int greaterBool = 0;

    if (onMenu == 1)
        counter = menuCounter;
    else
        counter = gameCounter;

    maxCount = counter + maxCursor;

     if (onMenu == 1)
     {
         if (maxCount <= ini_num_sections)
         {
             greaterBool = 1;
         }
     }
     else
     {
         if (maxCount <= maxGames)
         {
             greaterBool = 1;
         }
     }

    if (greaterBool == 1)
    {
        clearApp();
            
        while (counter <= maxCount)
        {
            /* Print next batch of games to screen. */
            font_renderstyle = TTF_STYLE_NORMAL;
            TTF_SetFontStyle(font, font_renderstyle);
            if (onMenu == 1)
            {
                console_games = TTF_RenderText_Solid(
                                               font,
                                               ini_system_name_str[counter],
                                               *forecol);
            }
            else
            {
                console_games = TTF_RenderText_Solid(font,
                                                     gamenames[counter],
                                                     *forecol);
            }

            if (console_games == NULL)
            {
                fprintf(stderr, "Couldn't render text: %s\n",
                                SDL_GetError());
                TTF_CloseFont(font);
            }
            else
            {
                console_games_drect.x = x_pos;
                console_games_drect.y = y_pos;
                console_games_drect.w = console_games->w;
                console_games_drect.h = console_games->h;

                if (SDL_BlitSurface(console_games, NULL, mainScreen,
                                    &console_games_drect) < 0)
                {
                    fprintf(stderr, "Couldn't blit text to display: %s\n",
                                    SDL_GetError());
                    TTF_CloseFont(font);
                }
                SDL_FreeSurface(console_games);
            }
            y_pos = y_pos + distance;

            counter++;
        }
    }
}

/*******************************************************************************
*                                                                              *
*  pageDown()                                                                  *
*                                                                              *
*    Moves the game list down 1 screen at a time.                              *
*                                                                              *
*******************************************************************************/

void pageDown()
{
    int maxCount;
    int counter;
    int x_pos = 80;
    int y_pos = 100;
    int font_renderstyle;
    int greaterBool = 0;

    if (onMenu == 1)
        counter = menuCounter;
    else
        counter = gameCounter;

    maxCount = counter;
    counter = counter - maxCursor;

    if (onMenu == 1)
    {
        if (maxCount <= ini_num_sections)
        {
            greaterBool = 1;
        }
    }
    else
    {
        if (maxCount <= maxGames)
        {
            greaterBool = 1;
        }
    }

    if (greaterBool == 1)
    {
        clearApp();
         
        while (counter <= maxCount)
        {
            /* Print next batch of games to screen. */
            font_renderstyle = TTF_STYLE_NORMAL;
            TTF_SetFontStyle(font, font_renderstyle);
            if (onMenu == 1)
            {
                console_games = TTF_RenderText_Solid(
                                               font,
                                               ini_system_name_str[counter],
                                               *forecol);
            }
            else
            {
                console_games = TTF_RenderText_Solid(font,
                                                     gamenames[counter],
                                                     *forecol);
            }

            if (console_games == NULL)
            {
                fprintf(stderr, "Couldn't render text: %s\n",
                                SDL_GetError());
                TTF_CloseFont(font);
            }
            else
            {
                console_games_drect.x = x_pos;
                console_games_drect.y = y_pos;
                console_games_drect.w = console_games->w;
                console_games_drect.h = console_games->h;

                if (SDL_BlitSurface(console_games, NULL, mainScreen,
                                    &console_games_drect) < 0)
                {
                    fprintf(stderr, "Couldn't blit text to display: %s\n",
                                    SDL_GetError());
                    TTF_CloseFont(font);
                }
                SDL_FreeSurface(console_games);
            }
            y_pos = y_pos + distance;

            counter++;
        }
    }
}

/*******************************************************************************
*                                                                              *
*  pageUp()                                                                    *
*                                                                              *
*    Moves the game list up 1 screen at a time.                                *
*                                                                              *
*******************************************************************************/

void pageUp()
{
    int maxCount;
    int counter;
    int x_pos = 80;
    int y_pos = 100;
    int font_renderstyle;
    int greaterBool = 0;

    if (onMenu == 1)
        counter = menuCounter;
    else
        counter = gameCounter;

    maxCount = counter + maxCursor;

    if (onMenu == 1)
    {
        if (maxCount <= ini_num_sections)
        {
            greaterBool = 1;
        }
    }
    else
    {
        if (maxCount <= maxGames)
        {
            greaterBool = 1;
        }
    }
    
    if (greaterBool == 1)
    {
        clearApp();
            
        while (counter <= maxCount)
        {
            /* Print next batch of games to screen. */
            font_renderstyle = TTF_STYLE_NORMAL;
            TTF_SetFontStyle(font, font_renderstyle);
            if (onMenu == 1)
            {
                console_games = TTF_RenderText_Solid(
                                               font,
                                               ini_system_name_str[counter],
                                               *forecol);
            }
            else
            {
                console_games = TTF_RenderText_Solid(font,
                                                     gamenames[counter],
                                                     *forecol);
            }

            if (console_games == NULL)
            {
                fprintf(stderr, "Couldn't render text: %s\n",
                                SDL_GetError());
                TTF_CloseFont(font);
            }
            else
            {
                console_games_drect.x = x_pos;
                console_games_drect.y = y_pos;
                console_games_drect.w = console_games->w;
                console_games_drect.h = console_games->h;
 
                if (SDL_BlitSurface(console_games, NULL, mainScreen,
                                    &console_games_drect) < 0)
                {
                    fprintf(stderr, "Couldn't blit text to display: %s\n",
                                    SDL_GetError());
                    TTF_CloseFont(font);
                }
                SDL_FreeSurface(console_games);
            }
            y_pos = y_pos + distance;

            counter++;
        }

        /*currentPosition = maxCursor;*/
    }
}

/*******************************************************************************
*                                                                              *
*  handleExit()                                                                *
*                                                                              *
*    Stops the mainLoop() from running.                                        *
*                                                                              *
*******************************************************************************/

void handleExit()
{
    done = 1;
}

/*******************************************************************************
*                                                                              *
*  quit()                                                                      *
*                                                                              *
*    Shuts down all SDL subsystems.                                            *
*                                                                              *
*******************************************************************************/

int quit()
{
    printf("\nQuitting SDL.. ");

    SDL_ShowCursor(SDL_ENABLE);

    if (SDL_JoystickOpened(0))
      SDL_JoystickClose(joystick1);
    if (SDL_JoystickOpened(1))
      SDL_JoystickClose(joystick2);
    
    Mix_CloseAudio();
    
    TTF_Quit();

    SDL_Quit();

    printf("done.\n");
    
    /* We'll need to make a call to "sudo shutdown -h now" here, eventually */
    printf("Now shutting down system.. \n\n");

    exit(0);
}

/*******************************************************************************
*                                                                              *
*  main()                                                                      *
*                                                                              *
*    Calls init_frontend(), mainLoop(), and quit(), in that order.             *
*                                                                              *
*******************************************************************************/

int main()
{
    parseTuxCfg();

    init_frontend();
    
    mainLoop();

    quit();

    return 0;
}
