/*
 * SysInfo - sysinfo plugin for HexChat
 * Copyright (c) 2015 Patrick Griffis.
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

#include <glib.h>
#include "parse.h"
#include "match.h"
#include "sysinfo.h"

char *sysinfo_backend_get_os(void)
{
	char name[bsize];

	if (xs_parse_distro (name) != 0)
	{
		return NULL;
	}

	return g_strdup(name);
}

char *sysinfo_backend_get_disk(void)
{
	char string[bsize] = {0,};

	if (xs_parse_df (NULL, string))
	{
		return NULL;
	}

	return g_strdup (string);
}

char *sysinfo_backend_get_memory(void)
{
	unsigned long long mem_total;
	unsigned long long mem_free;
	unsigned long long swap_total;
	unsigned long long swap_free;
	char string[bsize];

	if (xs_parse_meminfo (&mem_total, &mem_free, 0) == 1)
	{
		return NULL;
	}
	if (xs_parse_meminfo (&swap_total, &swap_free, 1) == 1)
	{
		return NULL;
	}

	g_snprintf (string, bsize, "%s - %s", pretty_freespace ("Physical", &mem_free, &mem_total), pretty_freespace ("Swap", &swap_free, &swap_total));

	return g_strdup (string);
}

char *sysinfo_backend_get_cpu(void)
{
	char model[bsize];
	char vendor[bsize];
	char cache[bsize];
	char buffer[bsize];
	unsigned int count;
	double freq;
	int giga = 0;

	if (xs_parse_cpu (model, vendor, &freq, cache, &count) != 0)
	{
		return NULL;
	}

	if (freq > 1000)
	{
		freq /= 1000;
		giga = 1;
	}

	if (giga)
	{
		g_snprintf (buffer, bsize, "%u x %s @ %.2fGHz w/ %s L2 Cache", count, model, freq, cache);
	}
	else
	{
		g_snprintf (buffer, bsize, "%u x %s @ %.0fMHz w/ %s L2 Cache", count, model, freq, cache);
	}
	
	return g_strdup (buffer);
}

char *sysinfo_backend_get_gpu(void)
{
	char vid_card[bsize];
	char agp_bridge[bsize];
	char buffer[bsize];
	int ret;

	if ((ret = xs_parse_video (vid_card)) != 0)
	{
		return NULL;
	}

	if (xs_parse_agpbridge (agp_bridge) != 0)
	{
		g_snprintf (buffer, bsize, "%s", vid_card);
	}
	else
	{
		g_snprintf (buffer, bsize, "%s @ %s", vid_card, agp_bridge);
	}

	return g_strdup (buffer);
}

char *sysinfo_backend_get_sound(void)
{
	char sound[bsize];

	if (xs_parse_sound (sound) != 0)
	{
		return NULL;
	}
	return g_strdup (sound);
}

char *sysinfo_backend_get_uptime(void)
{
	char buffer[bsize];
	int weeks;
	int days;
	int hours;
	int minutes;
	int seconds;

	if (xs_parse_uptime (&weeks, &days, &hours, &minutes, &seconds))
	{
		return NULL;
	}

	if (minutes != 0 || hours != 0 || days != 0 || weeks != 0)
	{
		if (hours != 0 || days != 0 || weeks != 0)
		{
			if (days  !=0 || weeks != 0)
			{
				if (weeks != 0)
				{
					g_snprintf (buffer, bsize, "%dw %dd %dh %dm %ds", weeks, days, hours, minutes, seconds);
				}
				else
				{
					g_snprintf (buffer, bsize, "%dd %dh %dm %ds", days, hours, minutes, seconds);
				}
			}
			else
			{
				g_snprintf (buffer, bsize, "%dh %dm %ds", hours, minutes, seconds);
			}
		}
		else
		{
			g_snprintf (buffer, bsize, "%dm %ds", minutes, seconds);
		}
	}

	return g_strdup (buffer);
}

char *sysinfo_backend_get_network(void)
{
	char ethernet_card[bsize];

	if (xs_parse_ether (ethernet_card))
	{
		g_strlcpy (ethernet_card, "None found", bsize);
	}
	
	return g_strdup (ethernet_card);
}
