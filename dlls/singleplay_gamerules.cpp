/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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
//
// teamplay_gamerules.cpp
//
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "skill.h"
#include "items.h"
#include "UserMessages.h"

//=========================================================
//=========================================================
CHalfLifeRules::CHalfLifeRules()
{
	SERVER_COMMAND( "exec spserver.cfg\n" );

	RefreshSkillData();
}

//=========================================================
//=========================================================
void CHalfLifeRules::Think()
{
}

//=========================================================
//=========================================================
bool CHalfLifeRules::IsMultiplayer()
{
	return false;
}

//=========================================================
//=========================================================
bool CHalfLifeRules::IsDeathmatch()
{
	return false;
}

//=========================================================
//=========================================================
bool CHalfLifeRules::IsCoOp()
{
	return false;
}


//=========================================================
//=========================================================
bool CHalfLifeRules::FShouldSwitchWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pWeapon)
{
	if (!pPlayer->m_pActiveItem)
	{
		// player doesn't have an active item!
		return true;
	}

	if (!pPlayer->m_pActiveItem->CanHolster())
	{
		return false;
	}

	return true;
}

//=========================================================
//=========================================================
bool CHalfLifeRules::GetNextBestWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pCurrentWeapon, bool alwaysSearch)
{
	//If this is an exhaustible weapon and it's out of ammo, always try to switch even in singleplayer.
	if (alwaysSearch || ((pCurrentWeapon->iFlags() & ITEM_FLAG_EXHAUSTIBLE) != 0 && pCurrentWeapon->PrimaryAmmoIndex() != -1 && pPlayer->m_rgAmmo[pCurrentWeapon->PrimaryAmmoIndex()] == 0))
	{
		return CGameRules::GetNextBestWeapon(pPlayer, pCurrentWeapon);
	}

	return false;
}

//=========================================================
//=========================================================
bool CHalfLifeRules::ClientConnected(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128])
{
	return true;
}

void CHalfLifeRules::InitHUD(CBasePlayer* pl)
{
}

//=========================================================
//=========================================================
void CHalfLifeRules::ClientDisconnected(edict_t* pClient)
{
}

//=========================================================
//=========================================================
float CHalfLifeRules::FlPlayerFallDamage(CBasePlayer* pPlayer)
{
	// subtract off the speed at which a player is allowed to fall without being hurt,
	// so damage will be based on speed beyond that, not the entire fall
	pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
	return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
}

//=========================================================
//=========================================================
void CHalfLifeRules::PlayerSpawn(CBasePlayer* pPlayer)
{
	// ############ hu3lifezado ############ //
	// Delay para executar checagem de comandos preconfigurados
	checkCommandsDelay = gpGlobals->time + 0.1;
	// ############ //
}

//=========================================================
//=========================================================
bool CHalfLifeRules::AllowAutoTargetCrosshair()
{
	return (g_iSkillLevel == SKILL_EASY);
}

//=========================================================
//=========================================================
void CHalfLifeRules::PlayerThink(CBasePlayer* pPlayer)
{
	// ############ hu3lifezado ############ //
	// Executo estados especiais nas entidades
	// NOTA: Isso era feito em PlayerSpawn() e funcionava quando usavamos o comando
	//       "map algum_mapa" para testarmos os niveis, mas em loads entre fases
	//       a funcao infelizmente rodava em um momento anterior ao que deveria e
	//       portanto nao tinha efeito. Fui obrigado a vir para o Think().
	if (gpGlobals->time > checkCommandsDelay && checkCommandsDelay != -1)
	{
		edict_t		*pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
		CBaseEntity *pEntity;
		char * sp_remove = (char*)CVAR_GET_STRING("sp_remove");
		int j, count_sp_remove = 0;

		// Contar a quantidade de entidades a remover
		char* tok = strtok(sp_remove, ";");
		while (tok != NULL) {
			count_sp_remove++;
			tok = strtok(NULL, ";");
		}

		if (count_sp_remove > 0)
		{
			for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
			{
				if (!pEdict)
					break;

				pEntity = CBaseEntity::Instance(pEdict);
				if (!pEntity)
					continue; // Essa verificacao em Util.cpp dentro de UTIL_MonstersInSphere() usa continue ao inves de break

				// Remover a entidade se ela estiver marcada como nao apropriada para o sp
				tok = sp_remove;
				for (j = 0; j < count_sp_remove; ++j) {
					if (strcmp(STRING(VARS(pEdict)->targetname), tok) == 0)
					{
						pEntity->SUB_Remove();

						break;
					}
					tok += strlen(tok) + 1;
					tok += strspn(tok, ";");
				}
			}
		}

		checkCommandsDelay = -1;
	}
	// ############ //
}


//=========================================================
//=========================================================
bool CHalfLifeRules::FPlayerCanRespawn(CBasePlayer* pPlayer)
{
	return true;
}

//=========================================================
//=========================================================
float CHalfLifeRules::FlPlayerSpawnTime(CBasePlayer* pPlayer)
{
	return gpGlobals->time; //now!
}

//=========================================================
// IPointsForKill - how many points awarded to anyone
// that kills this player?
//=========================================================
int CHalfLifeRules::IPointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled)
{
	return 1;
}

//=========================================================
// PlayerKilled - someone/something killed this player
//=========================================================
void CHalfLifeRules::PlayerKilled(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pInflictor)
{
}

//=========================================================
// Deathnotice
//=========================================================
void CHalfLifeRules::DeathNotice(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pInflictor)
{
}

//=========================================================
// PlayerGotWeapon - player has grabbed a weapon that was
// sitting in the world
//=========================================================
void CHalfLifeRules::PlayerGotWeapon(CBasePlayer* pPlayer, CBasePlayerItem* pWeapon)
{
}

//=========================================================
// FlWeaponRespawnTime - what is the time in the future
// at which this weapon may spawn?
//=========================================================
float CHalfLifeRules::FlWeaponRespawnTime(CBasePlayerItem* pWeapon)
{
	return -1;
}

//=========================================================
// FlWeaponRespawnTime - Returns 0 if the weapon can respawn
// now,  otherwise it returns the time at which it can try
// to spawn again.
//=========================================================
float CHalfLifeRules::FlWeaponTryRespawn(CBasePlayerItem* pWeapon)
{
	return 0;
}

//=========================================================
// VecWeaponRespawnSpot - where should this weapon spawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHalfLifeRules::VecWeaponRespawnSpot(CBasePlayerItem* pWeapon)
{
	return pWeapon->pev->origin;
}

//=========================================================
// WeaponShouldRespawn - any conditions inhibiting the
// respawning of this weapon?
//=========================================================
int CHalfLifeRules::WeaponShouldRespawn(CBasePlayerItem* pWeapon)
{
	return GR_WEAPON_RESPAWN_NO;
}

//=========================================================
//=========================================================
bool CHalfLifeRules::CanHaveItem(CBasePlayer* pPlayer, CItem* pItem)
{
	return true;
}

//=========================================================
//=========================================================
void CHalfLifeRules::PlayerGotItem(CBasePlayer* pPlayer, CItem* pItem)
{
}

//=========================================================
//=========================================================
int CHalfLifeRules::ItemShouldRespawn(CItem* pItem)
{
	return GR_ITEM_RESPAWN_NO;
}


//=========================================================
// At what time in the future may this Item respawn?
//=========================================================
float CHalfLifeRules::FlItemRespawnTime(CItem* pItem)
{
	return -1;
}

//=========================================================
// Where should this item respawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHalfLifeRules::VecItemRespawnSpot(CItem* pItem)
{
	return pItem->pev->origin;
}

//=========================================================
//=========================================================
bool CHalfLifeRules::IsAllowedToSpawn(CBaseEntity* pEntity)
{
	return true;
}

//=========================================================
//=========================================================
void CHalfLifeRules::PlayerGotAmmo(CBasePlayer* pPlayer, char* szName, int iCount)
{
}

//=========================================================
//=========================================================
int CHalfLifeRules::AmmoShouldRespawn(CBasePlayerAmmo* pAmmo)
{
	return GR_AMMO_RESPAWN_NO;
}

//=========================================================
//=========================================================
float CHalfLifeRules::FlAmmoRespawnTime(CBasePlayerAmmo* pAmmo)
{
	return -1;
}

//=========================================================
//=========================================================
Vector CHalfLifeRules::VecAmmoRespawnSpot(CBasePlayerAmmo* pAmmo)
{
	return pAmmo->pev->origin;
}

//=========================================================
//=========================================================
float CHalfLifeRules::FlHealthChargerRechargeTime()
{
	return 0; // don't recharge
}

//=========================================================
//=========================================================
int CHalfLifeRules::DeadPlayerWeapons(CBasePlayer* pPlayer)
{
	return GR_PLR_DROP_GUN_NO;
}

//=========================================================
//=========================================================
int CHalfLifeRules::DeadPlayerAmmo(CBasePlayer* pPlayer)
{
	return GR_PLR_DROP_AMMO_NO;
}

//=========================================================
//=========================================================
int CHalfLifeRules::PlayerRelationship(CBaseEntity* pPlayer, CBaseEntity* pTarget)
{
	// why would a single player in half life need this?
	return GR_NOTTEAMMATE;
}

//=========================================================
//=========================================================
bool CHalfLifeRules::FAllowMonsters()
{
	return true;
}
