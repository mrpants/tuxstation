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
*******************************************************************************/

#ifndef tuxconfig_h
#define tuxconfig_h

#include <stdlib.h>
#include <unistd.h>
#include "parsecfg.h"

int cfg_autoscan_bool;
int cfg_music_bool;
char *cfg_music_file_str;
char *cfg_border_file_str;
char *cfg_font_file_str;
int cfg_font_size_int;
Uint8 cfg_font_color_r_uint;
Uint8 cfg_font_color_g_uint;
Uint8 cfg_font_color_b_uint;
Uint8 cfg_background_color_r_uint;
Uint8 cfg_background_color_g_uint;
Uint8 cfg_background_color_b_uint;
int cfg_hres_int;
int cfg_vres_int;
int cfg_bpp_int;
int cfg_fullscreen_bool;

char **ini_system_name_str;
char **ini_system_emu_str;
char **ini_system_opts_before_str;
char **ini_system_opts_after_str;
char **ini_system_romdir_str;
char **ini_system_gamelist_str;
int *ini_system_autogen_int;

int ini_num_sections;

char *homedir;
char *config_file;
char *system_file;

/* Main settings */
cfgStruct cfgMain[] =
{
    /* parameter    type        address of variable */
    {"autoscan_roms", CFG_BOOL, &cfg_autoscan_bool},
    {"music",       CFG_BOOL,   &cfg_music_bool},
    {"music_file",  CFG_STRING, &cfg_music_file_str},
    {"border_file", CFG_STRING, &cfg_border_file_str},
    {"font_file",   CFG_STRING, &cfg_font_file_str},
    {"font_size",   CFG_INT,    &cfg_font_size_int},
    {"font_color_r", CFG_UINT, &cfg_font_color_r_uint},
    {"font_color_g", CFG_UINT, &cfg_font_color_g_uint},
    {"font_color_b", CFG_UINT, &cfg_font_color_b_uint},
    {"background_color_r", CFG_UINT, &cfg_background_color_r_uint},
    {"background_color_g", CFG_UINT, &cfg_background_color_g_uint},
    {"background_color_b", CFG_UINT, &cfg_background_color_b_uint},
    {"h_res", CFG_INT, &cfg_hres_int},
    {"v_res", CFG_INT, &cfg_vres_int},
    {"color_bpp", CFG_INT, &cfg_bpp_int},
    {"fullscreen",  CFG_BOOL,   &cfg_fullscreen_bool},
    {NULL, CFG_END, NULL}	/* no more parameters */
};

/* System specific settings */
cfgStruct cfgSystem[] =
{
    {"system_name", CFG_STRING, &ini_system_name_str},
    {"emulator",    CFG_STRING, &ini_system_emu_str},
    {"options_before", CFG_STRING, &ini_system_opts_before_str},
    {"options_after",  CFG_STRING, &ini_system_opts_after_str},
    {"rom_dir",     CFG_STRING, &ini_system_romdir_str},
    {"game_list",   CFG_STRING, &ini_system_gamelist_str},
    {"autogenerate", CFG_INT, &ini_system_autogen_int},

    {NULL, CFG_END, NULL}	/* no more parameters */
};

void checkHomeDir()
{
    char *tux_data_dir;
    
    /* First get the user's home directory. */
    homedir = getenv("HOME");
    tux_data_dir = (char *)malloc(strlen(homedir) + 20);
    strcpy(tux_data_dir, homedir);
    strcat(tux_data_dir, "/.tuxstation");

    config_file = (char *)malloc(strlen(tux_data_dir) + 20);
    strcpy(config_file, tux_data_dir);
    strcat(config_file, "/tuxstation.conf");
    
    system_file = (char *)malloc(strlen(tux_data_dir) + 25);
    strcpy(system_file, tux_data_dir);
    strcat(system_file, "/tuxstation_system.conf");

    /* If these files don't exist, it means that the user has not made their
       own configuration files.  So, we'll use the global files instead. */
    if (access(config_file, F_OK) != 0)
    {
        printf("Local file doesn't exist.  Using global file.\n");
        config_file = "tuxstation.conf";
        system_file = "tuxstation_system.conf";
        /*
        config_file = "/usr/local/share/tuxstation/tuxstation.conf";
        system_file = "/usr/local/share/tuxstation/tuxstation_system.conf";
        */
    }
}

void parseTuxCfg()
{
    /* Get the user's home directory first. */
    checkHomeDir();
    
    /* Parse tuxstation.conf */
    if (cfgParse(config_file, cfgMain, CFG_SIMPLE) == -1)
    {
        printf("An error was detected in tuxstation.conf.\n");
    }

    /* Parse tuxstation_system.conf */
    ini_num_sections = cfgParse(system_file, cfgSystem, CFG_INI);
    if (ini_num_sections == -1)
    {
        printf("An error was detected in tuxstation_system.conf.\n");
    }
}

#endif
