/*
 * Wolfenstein: Enemy Territory GPL Source Code
 * Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.
 *
 * ET: Legacy
 * Copyright (C) 2012 Jan Simek <mail@etlegacy.com>
 *
 * This file is part of ET: Legacy - http://www.etlegacy.com
 *
 * ET: Legacy is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ET: Legacy is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ET: Legacy. If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, Wolfenstein: Enemy Territory GPL Source Code is also
 * subject to certain additional terms. You should have received a copy
 * of these additional terms immediately following the terms and conditions
 * of the GNU General Public License which accompanied the source code.
 * If not, please request a copy in writing from id Software at the address below.
 *
 * id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
 */
/**
 * @file g_config.c
 * @brief Config/Mode settings
 */

#include "g_local.h"

/*
=============
G_ConfigParse
=============
*/
static qboolean G_ConfigError(int handle, char *format, ...)
{
	int         line = 0;
	char        filename[MAX_QPATH];
	va_list     argptr;
	static char string[4096];

	va_start(argptr, format);
	Q_vsnprintf(string, sizeof(string), format, argptr);
	va_end(argptr);

	filename[0] = '\0';

	trap_PC_SourceFileAndLine(handle, filename, &line);

	Com_Printf(S_COLOR_RED "ERROR: %s, line %d: %s\n", filename, line, string);

	trap_PC_FreeSource(handle);

	return qfalse;
}

void G_PrintConfigs(gentity_t *ent)
{
	char configNames[8192];
	char filename[MAX_QPATH];
	int  numconfigs = 0, i = 0, namelen = 0;
	char *configPointer;
	G_Printf("Starting to read configs\n");
	numconfigs    = trap_FS_GetFileList("configs", ".config", configNames, sizeof(configNames));
	configPointer = configNames;
	for (i = 0; i < numconfigs; i++, configPointer += namelen + 1)
	{
		namelen = strlen(configPointer);
		strcpy(filename, Q_StrReplace(configPointer, ".config", ""));
		if (!Q_stricmp(filename, g_customConfig.string))
		{
			G_refPrintf(ent, "^7Config: ^3%s ^2- in use", filename);
		}
		else
		{
			G_refPrintf(ent, "^7Config: ^3%s", filename);
		}
	}
	G_Printf("Config list done\n");
}

qboolean G_ParseSettings(int handle, qboolean setvars, config_t *config)
{
	pc_token_t token;
	char       text[256];
	char       value[256];

	if (!trap_PC_ReadToken(handle, &token) || Q_stricmp(token.string, "{"))
	{
		G_Printf("Malformed config\n");
	}

	while (1)
	{
		if (!trap_PC_ReadToken(handle, &token))
		{
			break;
		}

		if (token.string[0] == '}')
		{
			break;
		}

		//If we want to skip the settings
		if (!setvars)
		{
			continue;
		}

		if (!Q_stricmp(token.string, "set"))
		{
			if (!PC_String_ParseNoAlloc(handle, text, sizeof(text)))
			{
				return G_ConfigError(handle, "expected cvar to set");
			}
			else
			{
				if (!PC_String_ParseNoAlloc(handle, value, sizeof(value)))
				{
					return G_ConfigError(handle, "expected cvar value");
				}

				if (value[0] == '-')
				{
					if (!PC_String_ParseNoAlloc(handle, text, sizeof(text)))
					{
						return G_ConfigError(handle, "expected value after '-'");
					}

					Q_strncpyz(value, va("-%s", text), sizeof(value));
				}

				trap_Cvar_Set(text, value);
				G_Printf("set %s %s\n", text, value);
			}
		}
		else if (!Q_stricmp(token.string, "setl"))
		{
			int      i         = 0;
			qboolean overwrite = qfalse;

			if (!PC_String_ParseNoAlloc(handle, text, sizeof(text)))
			{
				return G_ConfigError(handle, "expected cvar to set");
			}
			else
			{
				if (!PC_String_ParseNoAlloc(handle, value, sizeof(value)))
				{
					return G_ConfigError(handle, "expected cvar value");
				}

				if (value[0] == '-')
				{
					if (!PC_String_ParseNoAlloc(handle, text, sizeof(text)))
					{
						return G_ConfigError(handle, "expected value after '-'");
					}

					Q_strncpyz(value, va("-%s", text), sizeof(value));
				}
			}

			for (; i < config->numSetl; i++)
			{
				if (!Q_stricmp(config->setl[i].name, text))
				{
					overwrite = qtrue;
					break;
				}
			}

			if (overwrite)
			{
				Q_strncpyz(config->setl[i].name, text, sizeof(config->setl[0].name));
				Q_strncpyz(config->setl[i].value, value, sizeof(config->setl[0].name));
			}
			else
			{
				Q_strncpyz(config->setl[config->numSetl].name, text, sizeof(config->setl[0].name));
				Q_strncpyz(config->setl[config->numSetl].value, value, sizeof(config->setl[0].name));
				i = config->numSetl;
				config->numSetl++;
			}

			trap_Cvar_Set(config->setl[i].name, config->setl[i].value);
			G_Printf("setl %s %s\n", config->setl[i].name, config->setl[i].value);
		}
		else if (!Q_stricmp(token.string, "command"))
		{
			if (!trap_PC_ReadToken(handle, &token))
			{
				return G_ConfigError(handle, "excepted a command value");
			}
			trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", token.string));
		}
		else if (!Q_stricmp(token.string, "mapscripthash"))
		{
			if (!PC_String_ParseNoAlloc(handle, config->mapscripthash, sizeof(config->mapscripthash)))
			{
				return G_ConfigError(handle, "expected mapscript hash value");
			}
		}
		else
		{
			return G_ConfigError(handle, "unknown token: %s", token.string);
		}
	}

	return qtrue;
}

qboolean G_ParseMapSettings(int handle, config_t *config)
{
	pc_token_t token;
	char       serverinfo[MAX_INFO_STRING];
	char       *mapname;

	trap_GetServerinfo(serverinfo, sizeof(serverinfo));
	mapname = Info_ValueForKey(serverinfo, "mapname");

	trap_PC_ReadToken(handle, &token);

	G_Printf("Map settings for: %s\n", token.string);
	G_Printf("Current map: %s\n", mapname);

	if (!Q_stricmp(token.string, "default"))
	{
		G_Printf("Setting rules for map: %s\n", token.string);
		return G_ParseSettings(handle, qtrue, config);
	}
	else if (!Q_stricmp(token.string, mapname))
	{
		fileHandle_t f;
		char         *code, *signature;
		qboolean     res = qfalse;

		G_Printf("Setting rules for map: %s\n", token.string);
		res = G_ParseSettings(handle, qtrue, config);
		if (res && strlen(config->mapscripthash))
		{
			char sdir[MAX_QPATH];
			int  flen = 0;

			trap_Cvar_VariableStringBuffer("g_mapScriptDirectory", sdir, sizeof(sdir));

			flen = trap_FS_FOpenFile(va("%s/%s.script", sdir, mapname), &f, FS_READ);
			if (flen < 0)
			{
				return G_ConfigError(handle, "Cannot open mapscript file for hash verification: %s/%s.script", sdir, mapname);
			}

			code = malloc(flen + 1);
			trap_FS_Read(code, flen, f);
			*(code + flen) = '\0';
			trap_FS_FCloseFile(f);
			signature = G_SHA1(code);

			free(code);

			if (Q_stricmp(config->mapscripthash, signature))
			{
				return G_ConfigError(handle, "Invalid mapscript hash for map: %s hash given in config: \"%s\" scripts actual hash \"%s\"", mapname, config->mapscripthash, signature);
			}

			G_Printf("Hash is valid for map: %s\n", mapname);
		}

		return res;
	}
	else
	{
		G_Printf("Ignoring rules for map: %s\n", token.string);
		return G_ParseSettings(handle, qfalse, config);
	}
}

void G_configLoadAndSet(const char *name)
{
	pc_token_t token;
	int        handle;
	config_t   *config;
	qboolean   parseOK = qtrue;

	handle = trap_PC_LoadSource(va("configs/%s.config", name));

	if (!handle)
	{
		Com_Printf(S_COLOR_RED "ERROR: File not found: %s\n", name);
		return;
	}

	memset(&level.config, 0, sizeof(config_t));

	G_wipeCvars();

	config = &level.config;

	config->publicConfig = qfalse;

	while (1)
	{
		if (!trap_PC_ReadToken(handle, &token))
		{
			break;
		}

		if (!Q_stricmp(token.string, "configname"))
		{
			if (!PC_String_ParseNoAlloc(handle, config->name, sizeof(config->name)))
			{
				G_Printf("expected config name\n");
				parseOK = qfalse;
				break;
			}
			G_Printf("Config name is: %s\n", config->name);
		}
		else if (!Q_stricmp(token.string, "version"))
		{
			if (!PC_String_ParseNoAlloc(handle, config->version, sizeof(config->name)))
			{
				G_Printf("expected config version\n");
				parseOK = qfalse;
				break;
			}
		}
		else if (!Q_stricmp(token.string, "init"))
		{
			if (!G_ParseSettings(handle, qtrue, config))
			{
				G_Printf("Reading settings failed\n");
				parseOK = qfalse;
				break;
			}
		}
		else if (!Q_stricmp(token.string, "map"))
		{
			if (!G_ParseMapSettings(handle, config))
			{
				G_Printf("Reading map settings failed\n");
				parseOK = qfalse;
				break;
			}
		}
		else if (!Q_stricmp(token.string, "signature"))
		{
			if (!PC_String_ParseNoAlloc(handle, config->signature, sizeof(config->signature)))
			{
				G_Printf("expected config signature\n");
				parseOK = qfalse;
				break;
			}
		}
		else if (!Q_stricmp(token.string, "public"))
		{
			config->publicConfig = qtrue;
		}
		else
		{
			G_Printf("unknown token %s\n", token.string);
			parseOK = qfalse;
			break;
		}
	}
	trap_PC_FreeSource(handle);

	if (parseOK)
	{
		trap_SetConfigstring(CS_CONFIGNAME, config->name);

		if (level.config.version[0] && level.config.name[0])
		{
			trap_SendServerCommand(-1, va("cp \"^7Config '%s^7' version '%s'^7 loaded\"", level.config.name, level.config.version));
		}
		else if (level.config.name[0])
		{
			trap_SendServerCommand(-1, va("cp \"^7Config '%s^7' loaded\"", level.config.name));
		}
	}
	else
	{
		trap_SetConfigstring(CS_CONFIGNAME, "");
		trap_SendServerCommand(-1, va("cp \"^7Config '%s^7' ^1FAILED ^7to load\"", name));
	}

	G_UpdateCvars();
}

// Force settings to predefined state.
qboolean G_configSet(const char *configname)
{
	char filename[MAX_QPATH];

	if (configname[0])
	{
		Q_strncpyz(filename, configname, sizeof(filename));
	}
	else if (g_customConfig.string[0])
	{
		Q_strncpyz(filename, g_customConfig.string, sizeof(filename));
	}
	else
	{
		return qfalse;
	}

	G_Printf("Will try to load config: \"configs/%s.config\"\n", filename);
	if (!trap_FS_FOpenFile(va("configs/%s.config", filename), NULL, FS_READ))
	{
		G_Printf("^3Warning: No config with filename '%s' found\n", filename);
		return qfalse;
	}

	G_configLoadAndSet(filename);

	//TODO: handle the mapscript hash and run the map script
	// the current map script will be on 'level.scriptEntity'
	// will need to add sha1 to source (comes with auth files)

	G_Printf(">> %s settings loaded!\n", (level.config.publicConfig) ? "Public" : "Competition");

	trap_Cvar_Set("g_customConfig", filename);

	if (!level.config.publicConfig && g_gamestate.integer == GS_WARMUP_COUNTDOWN)
	{
		level.lastRestartTime = level.time;
		trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0 %i\n", GS_WARMUP));
	}
	else
	{
		trap_SendConsoleCommand(EXEC_APPEND, va("map_restart 0 %i\n", GS_WARMUP));
	}

	return qtrue;
}

void G_ConfigCheckLocked()
{
	int      i       = 0;
	config_t *config = &level.config;

	if (!config)
	{
		return;
	}

	for (; i < config->numSetl; i++)
	{
		char temp[256];

		if (!config->setl[i].name)
		{
			continue;
		}

		trap_Cvar_VariableStringBuffer(config->setl[i].name, temp, 256);

		if (Q_stricmp(config->setl[i].value, temp))
		{
			G_Printf("Config cvar \"%s\" value: %s does not match the currently set value %s\n", config->setl[i].name, config->setl[i].value, temp);
			trap_SetConfigstring(CS_CONFIGNAME, "");
			trap_SendServerCommand(-1, va("cp \"^7Config '%s^7' ^1WAS UNLOADED DUE TO EXTERNAL MANIPULATION\"", config->name));
			memset(&level.config, 0, sizeof(config_t));
			break;
		}
	}

	return;
}
