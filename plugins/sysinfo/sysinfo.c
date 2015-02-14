/*
 * SysInfo - sysinfo plugin for HexChat
 * Copyright (c) 2012 Berke Viktor.
 *
 * xsys.c - main functions for X-Sys 2
 * by mikeshoup
 * Copyright (C) 2003, 2004, 2005 Michael Shoup
 * Copyright (C) 2005, 2006, 2007 Tony Vroon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "hexchat-plugin.h"
#include "sysinfo-backend.h"
#include "sysinfo.h"

#define DEFAULT_ANNOUNCE TRUE

static hexchat_plugin *ph;

static char name[] = "SysInfo";
static char desc[] = "Display info about your hardware and OS";
static char version[] = "3.0";
static char sysinfo_help[] = "SysInfo Usage:\n  /SYSINFO [-e|-o] [OS|DISTRO|CPU|RAM|DISK|VGA|SOUND|ETHERNET|UPTIME], print various details about your system or print a summary without arguments\n  /SYSINFO LIST, print current settings\n  /SYSINFO SET <variable>, update given setting\n  /SYSINFO RESET, reset settings to defaults\n  /NETDATA <iface>, show transmitted data on given interface\n  /NETSTREAM <iface>, show current bandwidth on given interface\n";

typedef struct
{
	const char *name;
	char *(*callback) (void);
} hwinfo;

static hwinfo hwinfos[] = {
	{"OS", sysinfo_backend_get_os},
	{"CPU", sysinfo_backend_get_cpu},
	{"MEMORY", sysinfo_backend_get_memory},
	{"DISK", sysinfo_backend_get_disk},
	{"GPU", sysinfo_backend_get_gpu},
	{"SOUND", sysinfo_backend_get_sound},
	{"ETHERNET", sysinfo_backend_get_network},
	{"UPTIME", sysinfo_backend_get_uptime},
	{NULL, NULL},
};

static gboolean
should_show_info (const char *name)
{
	return TRUE;
}

static void
print_summary (gboolean announce)
{
	char **strings = g_new0 (char*, G_N_ELEMENTS(hwinfos));
	int i, x;
	char *output;

	for (i = 0, x = 0; hwinfos[i].name != NULL; i++)
	{
		if (should_show_info (hwinfos[i].name))
		{
			char *str = hwinfos[i].callback();
			if (str)
			{
				strings[x++] = g_strdup_printf ("\002%s\002: %s", hwinfos[i].name, str);
				g_free (str);
			}
		}
	}

	output = g_strjoinv (" \002\342\200\242\002 ", strings);
	hexchat_commandf (ph, "%s %s", announce ? "SAY" : "ECHO", output);

	g_strfreev (strings);
	g_free (output);
}

static void
print_info (char *info, gboolean announce)
{
	int i;

	for (i = 0; hwinfos[i].name != NULL; i++)
	{
		if (!strcmp (info, hwinfos[i].name))
		{
			char *str = hwinfos[i].callback();
			hexchat_commandf (ph, "%s %s", announce ? "SAY" : "ECHO", str);
			g_free (str);
			return;
		}
	}

	hexchat_printf (ph, "No info by that name");
}

/*
 * Simple wrapper for backend specific options.
 * Ensure dest >= 512.
 */
int
sysinfo_get_pref (const char *pref, char *dest)
{
	return hexchat_pluginpref_get_str (ph, pref, dest);
}

static gboolean
sysinfo_get_announce (void)
{
	int announce;

	if ((announce = hexchat_pluginpref_get_int (ph, "announce") != -1))
		return announce;

	return DEFAULT_ANNOUNCE;
}

static int
sysinfo_cb (char *word[], char *word_eol[], void *userdata)
{
	gboolean announce = sysinfo_get_announce ();
	int offset = 0, channel_type;
	char *cmd;

	/* Allow overriding global announce setting */
	if (!strcmp ("-e", word[2]))
	{
		announce = FALSE;
		offset++;
	}
	else if (!strcmp ("-o", word[2]))
	{
		announce = TRUE;
		offset++;
	}

	/* Cannot send to server tab */
	channel_type = hexchat_list_int (ph, NULL, "type");
	if (channel_type != 2 /* SESS_CHANNEL */ || channel_type != 3 /* SESS_DIALOG */)
		announce = FALSE;

	cmd = g_ascii_strup (word[2+offset], -1);
	if (!strcmp ("LIST", cmd))
	{
		//list_settings ();
	}
	else if (!strcmp ("SET", cmd))
	{
		int buffer;

		if (!word[4+offset] || word[4+offset][0] == '\0')
		{
			hexchat_printf (ph, "%s\tEnter a value!\n", name);
		}
		else if (!g_ascii_strcasecmp ("format", word[3+offset]))
		{
			hexchat_pluginpref_set_str (ph, "format", word_eol[4+offset]);
			hexchat_printf (ph, "%s\tformat is set to: %s\n", name, word_eol[4+offset]);
		}
		else if (!g_ascii_strcasecmp ("percent", word[3+offset]))
		{
			buffer = atoi (word[4+offset]);	/* don't use word_eol, numbers must not contain spaces */

			if (buffer > 0 && buffer < INT_MAX)
			{
				hexchat_pluginpref_set_int (ph, "percent", buffer);
				hexchat_printf (ph, "%s\tpercent is set to: %d\n", name, buffer);
			}
			else
			{
				hexchat_printf (ph, "%s\tInvalid input!\n", name);
			}
		}
		else if (!g_ascii_strcasecmp ("announce", word[3+offset]))
		{
			buffer = atoi (word[4+offset]);	/* don't use word_eol, numbers must not contain spaces */

			if (buffer > 0)
			{
				hexchat_pluginpref_set_int (ph, "announce", 1);
				hexchat_printf (ph, "%s\tannounce is set to: On\n", name);
			}
			else
			{
				hexchat_pluginpref_set_int (ph, "announce", 0);
				hexchat_printf (ph, "%s\tannounce is set to: Off\n", name);
			}
		}
		else if (!g_ascii_strcasecmp ("pciids", word[3+offset]))
		{
			hexchat_pluginpref_set_str (ph, "pciids", word_eol[4+offset]);
			hexchat_printf (ph, "%s\tpciids is set to: %s\n", name, word_eol[4+offset]);
		}
		else
		{
			hexchat_printf (ph, "%s\tInvalid variable name! Use 'pciids', 'format' or 'percent'!\n", name);
		}
	}
	else if (!strcmp ("RESET", cmd))
	{
		//reset_settings ();
		hexchat_printf (ph, "%s\tSettings have been restored to defaults.\n", name);
	}
	else if (!cmd || !cmd[0])
	{
		print_summary (announce);
	}
	else
	{
		print_info (cmd, announce);
	}

	g_free (cmd);
	return HEXCHAT_EAT_ALL;
}

int
hexchat_plugin_init (hexchat_plugin *plugin_handle, char **plugin_name, char **plugin_desc, char **plugin_version, char *arg)
{
	ph = plugin_handle;
	*plugin_name = name;
	*plugin_desc = desc;
	*plugin_version = version;

	hexchat_hook_command (ph, "SYSINFO", HEXCHAT_PRI_NORM, sysinfo_cb, sysinfo_help, NULL);
	//hexchat_hook_command (ph, "NETDATA",	HEXCHAT_PRI_NORM,	netdata_cb,	NULL, NULL);
	//hexchat_hook_command (ph, "NETSTREAM",	HEXCHAT_PRI_NORM,	netstream_cb,	NULL, NULL);

	hexchat_command (ph, "MENU ADD \"Window/Send System Info\" \"SYSINFO\"");
	hexchat_printf (ph, "%s plugin loaded\n", name);
	return 1;
}

int
hexchat_plugin_deinit (void)
{
	hexchat_command (ph, "MENU DEL \"Window/Display System Info\"");
	hexchat_printf (ph, "%s plugin unloaded\n", name);
	return 1;
}
