/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#pragma once

#define FCVAR_ARCHIVE (1 << 0)			// set to cause it to be saved to vars.rc
#define FCVAR_USERINFO (1 << 1)			// changes the client's info string
#define FCVAR_SERVER (1 << 2)			// notifies players when changed
#define FCVAR_EXTDLL (1 << 3)			// defined by external DLL
#define FCVAR_CLIENTDLL (1 << 4)		// defined by the client dll
#define FCVAR_PROTECTED (1 << 5)		// It's a server cvar, but we don't send the data since it's a password, etc.  Sends 1 if it's not bland/zero, 0 otherwise as value
#define FCVAR_SPONLY (1 << 6)			// This cvar cannot be changed by clients connected to a multiplayer server.
#define FCVAR_PRINTABLEONLY (1 << 7)	// This cvar's string cannot contain unprintable characters ( e.g., used for player name etc ).
#define FCVAR_UNLOGGED (1 << 8)			// If this is a FCVAR_SERVER, don't log changes to the log file / console if we are creating a log
#define FCVAR_NOEXTRAWHITEPACE (1 << 9) // strip trailing/leading white space from this cvar
#define FCVAR_PRIVILEGED (1 << 10)		// Not queryable/settable by unprivileged sources
#define FCVAR_FILTERSTUFFTEXT (1 << 11) // Not queryable/settable if unprivileged and filterstufftext is enabled
#define FCVAR_FILTERCHARS (1 << 12)	// This cvar's string will be filtered for 'bad' characters (e.g. ';', '\n')
#define FCVAR_NOBADPATHS (1 << 13)		// This cvar's string cannot contain file paths that are above the current directory

// ############ hu3lifezado ############ //
// Agora e "char* string;" em vez de "const char* string;"
typedef struct cvar_s
{
	const char* name;
	//Technically this should be non-const but that only matters to engine code
	char* string;
	int flags;
	float value;
	struct cvar_s* next;
} cvar_t;
// ############ //