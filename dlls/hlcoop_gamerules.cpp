// ##############################
// HU3-LIFE REGRAS DO COOP
// ##############################
/*
NOTA: eu escrevi esse código mas portei ele entre duas codebases antes de conseguir lançar o Hu3, então
espere por comentários estranhos sobre classes inexistentes etc. Outra coisa... Olhando agora, 1 década
depois, o código é uma bela porcaria, cheio de chamada insegura, lógica mal feita etc. - porém funcional.
Estão avisados.

Para entender melhor o funcionamento deste modo, veja a nossa documentação no GitHub - parte de Wiki.
*/

#include "extdll.h"
#include "cdll_dll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "triggers.h"
#include "gamerules.h"
#include "hu3coop.h"
#include "hlcoop_gamerules.h"

#include "skill.h"
#include "game.h"
#include "items.h"
#include "voice_gamemgr.h"
#include "hltv.h"
#include "trains.h"
#include "UserMessages.h"

extern DLL_GLOBAL CGameRules *g_pGameRules;

extern cvar_t mp_chattime;

CVoiceGameMgr g_VoiceGameMgr2;

// HACK HACK HACK
// Eu não consigo de jeito nenhum atualizar alguns ponteiros com nomes
// de armas, entã fiz esse gato vindos de pWeapon->GetItemInfo(&wepInfo);
// Esse mapa me livra de contar com o string wepInfo.pszName.
static const char* WeaponIdToName[] = {
    "weapon_none",        // 0
    "weapon_crowbar",     // 1
    "weapon_9mmhandgun",  // 2 (GLOCK)
    "weapon_357",         // 3 (PYTHON)
    "weapon_9mmAR",       // 4 (MP5)
    "weapon_chaingun",    // 5
    "weapon_crossbow",    // 6
    "weapon_shotgun",     // 7
    "weapon_rpg",         // 8
    "weapon_gauss",       // 9
    "weapon_egon",        // 10
    "weapon_hornetgun",   // 11
    "weapon_handgrenade", // 12
    "weapon_tripmine",    // 13
    "weapon_satchel",     // 14
    "weapon_snark",       // 15
    "weapon_eagle",       // 16 (DESERT_EAGLE)
    "weapon_knife",       // 17
    "weapon_pipewrench",  // 18
    NULL,                 // 19..30 (if unused)
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 
    "item_suit"           // 31
};


class CMultiplayGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool CanPlayerHearPlayer(CBasePlayer *pListener, CBasePlayer *pTalker)
	{
		if (g_teamplay)
		{
			if (g_pGameRules->PlayerRelationship(pListener, pTalker) != GR_TEAMMATE)
			{
				return false;
			}
		}

		return true;
	}
};
static CMultiplayGameMgrHelper g_GameMgrHelper;

//*********************************************************
// Rules for the half-life multiplayer game.
//*********************************************************

CBaseHalfLifeCoop::CBaseHalfLifeCoop()
{
	g_VoiceGameMgr2.Init(&g_GameMgrHelper, gpGlobals->maxClients);

	RefreshSkillData();

	// 11/8/98
	// Modified by YWB:  Server .cfg file is now a cvar, so that
	//  server ops can run multiple game servers, with different server .cfg files,
	//  from a single installed directory.
	// Mapcyclefile is already a cvar.

	// 3/31/99
	// Added lservercfg file cvar, since listen and dedicated servers should not
	// share a single config file. (sjb)
	if (IS_DEDICATED_SERVER())
	{
		// this code has been moved into engine, to only run server.cfg once
	}
	else
	{
		// listen server
		char* lservercfgfile = (char*)CVAR_GET_STRING("lservercfgfile");

		if (lservercfgfile && '\0' != lservercfgfile[0])
		{
			char szCommand[256];

			ALERT(at_console, "Executing listen server config file\n");
			sprintf(szCommand, "exec %s\n", lservercfgfile);
			SERVER_COMMAND(szCommand);
		}
	}
}

bool CBaseHalfLifeCoop::ClientCommand(CBasePlayer *pPlayer, const char *pcmd)
{
	if (g_VoiceGameMgr2.ClientCommand(pPlayer, pcmd))
		return true;

	return CGameRules::ClientCommand(pPlayer, pcmd);
}

void CBaseHalfLifeCoop::ClientUserInfoChanged(CBasePlayer* pPlayer, char* infobuffer)
{
	pPlayer->SetPrefsFromUserinfo(infobuffer);
}

//=========================================================
//=========================================================
void CBaseHalfLifeCoop::RefreshSkillData()
{
	// load all default values
	CGameRules::RefreshSkillData();

	// Alterar dano de armas etc aqui, se for necessário.
}


//=========================================================
//=========================================================
void CBaseHalfLifeCoop::Think(void)
{
	g_VoiceGameMgr2.Update(gpGlobals->frametime);
}

//=========================================================
//=========================================================
bool CBaseHalfLifeCoop::IsMultiplayer()
{
	return true;
}

//=========================================================
//=========================================================
bool CBaseHalfLifeCoop::IsDeathmatch()
{
	return false;
}

//=========================================================
//=========================================================
bool CBaseHalfLifeCoop::IsCoOp()
{
	return true;
}

//=========================================================
//=========================================================
bool CBaseHalfLifeCoop::FShouldSwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	if (!pWeapon->CanDeploy())
	{
		// that weapon can't deploy anyway.
		return false;
	}

	if (!pPlayer->m_pActiveItem)
	{
		// player doesn't have an active item!
		return true;
	}

	if (!pPlayer->m_pActiveItem->CanHolster())
	{
		// can't put away the active item.
		return false;
	}

	//Never switch
	if (pPlayer->m_iAutoWepSwitch == 0)
	{
		return false;
	}

	//Only switch if not attacking
	if (pPlayer->m_iAutoWepSwitch == 2 && (pPlayer->m_afButtonLast & (IN_ATTACK | IN_ATTACK2)) != 0)
	{
		return false;
	}

	if (pWeapon->iWeight() > pPlayer->m_pActiveItem->iWeight())
	{
		return true;
	}

	return false;
}

//=========================================================
//=========================================================
bool CBaseHalfLifeCoop::ClientConnected(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128])
{
	g_VoiceGameMgr2.ClientConnected(pEntity);
	return true;
}

void CBaseHalfLifeCoop::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(0); // game mode none
	MESSAGE_END();

	// Executo estados especiais nas entidades
	edict_t		*pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);
	CBaseEntity *pEntity;
	char * coop_remove = (char*)CVAR_GET_STRING("coop_remove");
	char * coop_nophysics = (char*)CVAR_GET_STRING("coop_nophysics");
	char * coop_teleport_plys = (char*)CVAR_GET_STRING("coop_teleport_plys");
	int j, count_coop_remove = 0, count_coop_nophysics = 0, count_coop_teleport_plys = 0;

	// Contar a quantidade de entidades a desativar a fisica
	char* tok1 = strtok(coop_nophysics, "|");
	while (tok1 != NULL)
	{
		count_coop_nophysics++;
		tok1 = strtok(NULL, "|");
	}

	// Contar a quantidade de entidades de teletransporte generalizado
	char* tok2 = strtok(coop_teleport_plys, "|");
	while (tok2 != NULL)
	{
		count_coop_teleport_plys++;
		tok2 = strtok(NULL, "|");
	}

	// Contar a quantidade de entidades a remover
	char* tok3 = strtok(coop_remove, "|");
	while (tok3 != NULL)
	{
		count_coop_remove++;
		tok3 = strtok(NULL, "|");
	}

	if (count_coop_remove > 0 || count_coop_nophysics > 0 || count_coop_teleport_plys > 0)
	{
		for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
		{
			if (!pEdict)
				break;

			pEntity = CBaseEntity::Instance(pEdict);
			if (!pEntity)
				continue; // Essa verificacao em Util.cpp dentro de UTIL_MonstersInSphere() usa continue ao inves de break

			bool next = false;

			//  Deixar a entidade com transparencia e sem efeitos fisicos (sendo que ela continua a funcionar)
			tok1 = coop_nophysics;
			for (j = 0; j < count_coop_nophysics; ++j)
			{
				if (pEntity->pev && FStrEq(STRING(pEntity->pev->targetname), tok1))
				{
					DisablePhysics(pEntity);
					next = true;

					break;
				}
				tok1 += strlen(tok1) + 1;
				tok1 += strspn(tok1, "|");
			}

			if (next)
				continue;

			// Marcar a entidade de teletransporte par uso generalizado em todos os players
			if (FClassnameIs(pEntity->pev, "trigger_teleport"))
			{
				CBaseTrigger *pEntity2 = (CBaseTrigger *)pEntity;

				tok2 = coop_teleport_plys;
				for (j = 0; j < count_coop_teleport_plys; ++j)
				{
					if (FStrEq(STRING(pEntity2->pev->targetname), tok2))
					{
						pEntity2->teleport_all_coop = true;
						next = true;

						break;
					}
					tok2 += strlen(tok2) + 1;
					tok2 += strspn(tok2, "|");
				}
			}

			if (next)
				continue;

			// Remover a entidade se ela estiver marcada como nao apropriada para o coop
			tok3 = coop_remove;
			for (j = 0; j < count_coop_remove; ++j)
			{
				if (FStrEq(STRING(pEntity->pev->targetname), tok3))
				{
					pEntity->SUB_Remove();

					break;
				}
				tok3 += strlen(tok3) + 1;
				tok3 += strspn(tok3, "|");
			}

		}
	}

	// Adiciono um sprite em cada landmark ou info_target de mesmo targetname de um landmark

	CBaseEntity* pLandmark = nullptr;
	CBaseEntity* pTarget = nullptr;

	while ((pLandmark = UTIL_FindEntityByClassname(pLandmark, "info_landmark")) != nullptr)
	{
		bool pTargetFound = false;

		// Busco por um info_target de mesmo targetname do landmark
		while ((pTarget = UTIL_FindEntityByClassname(pTarget, "info_target")) != nullptr)
		{
			if (FStrEq(STRING(pTarget->pev->targetname), STRING(pLandmark->pev->targetname)))
			{
				CSprite* pSprite1 = CSprite::SpriteCreate("sprites/changelevel.spr", pTarget->pev->origin, false);
				pSprite1->SetTransparency(kRenderTransAdd, 255, 255, 255, 150, kRenderFxFadeFast);

				pTargetFound = true;

				pTarget = nullptr;

				break;
			}
		}

		// Se eu nao encontrar o info_target acima, ponto o sprite no landmark
		if (!pTargetFound)
		{
			CSprite* pSprite1 = CSprite::SpriteCreate("sprites/changelevel.spr", pLandmark->pev->origin, false);
			pSprite1->SetTransparency(kRenderTransAdd, 255, 255, 255, 150, kRenderFxFadeFast);
		}
	}
}

void CBaseHalfLifeCoop::InitHUD(CBasePlayer *pl)
{
	// notify other clients of player joining the game
	UTIL_ClientPrintAll(HUD_PRINTNOTIFY, UTIL_VarArgs("%s se juntou a esta bosta\n",
		(!FStringNull(pl->pev->netname) && STRING(pl->pev->netname)[0] != 0) ? STRING(pl->pev->netname) : "desconectado"));

	// team match?
	if (g_teamplay)
	{
		UTIL_LogPrintf("\"%s<%i><%s><%s>\" entrou na zona\n",
			STRING(pl->pev->netname),
			GETPLAYERUSERID(pl->edict()),
			GETPLAYERAUTHID(pl->edict()),
			g_engfuncs.pfnInfoKeyValue(g_engfuncs.pfnGetInfoKeyBuffer(pl->edict()), "model"));
	}
	else
	{
		UTIL_LogPrintf("\"%s<%i><%s><%i>\" entrou socando\n",
			STRING(pl->pev->netname),
			GETPLAYERUSERID(pl->edict()),
			GETPLAYERAUTHID(pl->edict()),
			GETPLAYERUSERID(pl->edict()));
	}

	UpdateGameMode(pl);

	// sending just one score makes the hud scoreboard active;  otherwise
	// it is just disabled for single play
	MESSAGE_BEGIN(MSG_ONE, gmsgScoreInfo, NULL, pl->edict());
	WRITE_BYTE(ENTINDEX(pl->edict()));
	WRITE_SHORT(0);
	WRITE_SHORT(0);
	WRITE_SHORT(0);
	WRITE_SHORT(0);
	MESSAGE_END();

	SendMOTDToClient(pl);

	// loop through all active players and send their score info to the new client
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		// FIXME:  Probably don't need to cast this just to read m_iDeaths
		CBasePlayer* plr = (CBasePlayer*)UTIL_PlayerByIndex(i);

		if (plr)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgScoreInfo, NULL, pl->edict());
			WRITE_BYTE(i); // client number
			WRITE_SHORT(plr->pev->frags);
			WRITE_SHORT(plr->m_iDeaths);
			WRITE_SHORT(0);
			WRITE_SHORT(GetTeamIndex(plr->m_szTeamName) + 1);
			MESSAGE_END();
		}
	}
}

//=========================================================
//=========================================================
void CBaseHalfLifeCoop::ClientDisconnected(edict_t *pClient)
{
	if (pClient)
	{
		CBasePlayer* pPlayer = (CBasePlayer*)CBaseEntity::Instance(pClient);

		if (pPlayer)
		{
			FireTargets("game_playerleave", pPlayer, pPlayer, USE_TOGGLE, 0);

			// "Limpar" tabela do coop
			int CoopPlyIndex = pPlayer->entindex() - 1;
			if (!CoopPlyData[CoopPlyIndex].waitingforchangelevel)
				strcpy(CoopPlyData[CoopPlyIndex].pName, "");

			// team match?
			if (g_teamplay)
			{
				UTIL_LogPrintf("\"%s<%i><%s><%s>\" morreu de vez\n",
					STRING(pPlayer->pev->netname),
					GETPLAYERUSERID(pPlayer->edict()),
					GETPLAYERAUTHID(pPlayer->edict()),
					g_engfuncs.pfnInfoKeyValue(g_engfuncs.pfnGetInfoKeyBuffer(pPlayer->edict()), "model"));
			}
			else
			{
				UTIL_LogPrintf("\"%s<%i><%s><%i>\" morreu mamando\n",
					STRING(pPlayer->pev->netname),
					GETPLAYERUSERID(pPlayer->edict()),
					GETPLAYERAUTHID(pPlayer->edict()),
					GETPLAYERUSERID(pPlayer->edict()));
			}

			pPlayer->RemoveAllItems(true); // destroy all of the players weapons and items
		}
	}
}

//=========================================================
//=========================================================
float CBaseHalfLifeCoop::FlPlayerFallDamage(CBasePlayer *pPlayer)
{
	int iFallDamage = (int)falldamage.value;

	switch (iFallDamage)
	{
	case 1://progressive
		pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
		return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
		break;
	default:
	case 0:// fixed
		return 10;
		break;
	}
}

//=========================================================
//=========================================================
bool CBaseHalfLifeCoop::FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity* pAttacker)
{
	// Nao deixo matarem jogadores que estao aguardando o changelevel
	if (CoopPlyData[pPlayer->entindex() - 1].waitingforchangelevel)
		return false;

	// Protejo jogadores no spawn, principalmente porque um pode nascer dentro do outro
	if (CoopPlyData[pPlayer->entindex() - 1].notSolidWait > gpGlobals->time)
	{
		char* hu3Train = (char*)CVAR_GET_STRING("coop_train_spawnpoint");
		
		// Em mapas com func_tracktrain configurados, ativo a defesa contra dano generico automaticamente
		if (!FStrEq(hu3Train, "0"))
		{
			return false;
		}
		else
		{
			// Vejo se tem spawnpoints por perto
			CBaseEntity *ent = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(0));

			while ((ent = UTIL_FindEntityInSphere(ent, pPlayer->pev->origin, 128)) != nullptr)
			{
				if ((FClassnameIs(ent->pev, "info_player_start")) || (FClassnameIs(ent->pev, "info_player_coop")))
				{
					DisablePhysics(pPlayer);

					return false;
				}
			}
		}
	}

	return true;
}

//=========================================================
//=========================================================
void CBaseHalfLifeCoop::DisablePhysics(CBaseEntity * pEntity)
{
	if (pEntity->IsPlayer())
		CoopPlyData[pEntity->entindex() - 1].notSolidWait = gpGlobals->time + SPAWNPROTECTIONTIME;

	// Em mapas com func_tracktrain configurados, eu não posso ficar 100% intocável. O trem precisa interagir.
	char* hu3Train = (char*)CVAR_GET_STRING("coop_train_spawnpoint");
	if (!FStrEq(hu3Train, "0"))
		pEntity->pev->solid = SOLID_BBOX;
	else
		pEntity->pev->solid = SOLID_NOT;

	pEntity->pev->takedamage = DAMAGE_NO;
	pEntity->pev->rendermode = kRenderTransAdd;
	pEntity->pev->renderamt = 230;
	const Vector vecColor = { 0.6f, 0.8f, 1.0f };
	pEntity->pev->rendercolor = vecColor;
	pEntity->pev->renderfx = kRenderFxFadeFast;
}

//=========================================================
//=========================================================
void CBaseHalfLifeCoop::EnablePhysics(CBaseEntity * pEntity)
{
	pEntity->pev->solid = SOLID_SLIDEBOX;
	pEntity->pev->takedamage = DAMAGE_YES;
	pEntity->pev->rendermode = kRenderNormal;
}

//=========================================================
//=========================================================
void CBaseHalfLifeCoop::PlayerThink(CBasePlayer *pPlayer)
{
	int CoopPlyIndex = pPlayer->entindex() - 1;

	// Processo a area de changelevel se for necessario
	if (hu3ChangelevelCheck)
	{
		// Sempre no primeiro jogador (para evitar rodar o codigo desnecessariamente)
		if (pPlayer->entindex() == 1)
		{
			int i = ChangeLevelVolume();
			if (i == 1)
				ChangeLevelCoop();
			if (i == 3)
				hu3ChangelevelCheck = false;
		}
	}

	// Restauro itens e munições iniciais do jogador
	if (!CoopPlyData[CoopPlyIndex].newplayer &&
		(CoopPlyData[CoopPlyIndex].changinglevel ||
		CoopPlyData[CoopPlyIndex].respawning))
	{
		// Carregar itens e munições em intervalos de tempo
		if (gpGlobals->time > CoopPlyData[CoopPlyIndex].respawnItemWait)
		{
			if (LoadPlayerItems(pPlayer))
			{
				// Atualizo o estado de changing level
				CoopPlyData[CoopPlyIndex].changinglevel = false;
				CoopPlyData[CoopPlyIndex].respawning = false;

				// Reativo sons de captura de itens
				pPlayer->enable_item_pickup_sound = true;
			}
			else 
			{
				CoopPlyData[CoopPlyIndex].respawnItemWait = gpGlobals->time + respawnItemInterval;
			}
		}
	}

	// Apos respawn e de certo tempo, restaurar solidez de pessoas que nao estejam em area de changelevel
	if (!CoopPlyData[CoopPlyIndex].waitingforchangelevel && pPlayer->pev->solid == SOLID_NOT)
	{
		if (CoopPlyData[CoopPlyIndex].notSolidWait < gpGlobals->time)
		{
			CBaseEntity *ent = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(0));

			while ((ent = UTIL_FindEntityInSphere(ent, pPlayer->pev->origin, 64)) != nullptr)
			{
				//if (ent->IsPlayer()) // Sempre retorna falso
				if ((FClassnameIs(ent->pev, "player")) && (ent != pPlayer))
				{
					CoopPlyData[CoopPlyIndex].notSolidWait = gpGlobals->time + SPAWNPROTECTIONTIME;
				}
			}

			EnablePhysics(pPlayer);
		}
	}

	// Rodo um comando de troca de nome caso seja necessario
	if (CoopPlyData[CoopPlyIndex].pSetNewName)
	{
		char comando[35];
		snprintf(comando, sizeof(comando), "name %s\n", CoopPlyData[CoopPlyIndex].pName);
		CoopPlyData[CoopPlyIndex].pSetNewName = false;
		CLIENT_COMMAND(pPlayer->edict(), comando);
	}
}

//=========================================================
//=========================================================
// "glorious hack" copiado do arquivo de players
void CBaseHalfLifeCoop::FixPlayerCrouchStuck(edict_t *pPlayer)
{
	TraceResult trace;

	// Mover ate 18 pixels para cima se o jogador estiver preso no chao
	for (int i = 0; i < 18; i++)
	{
		UTIL_TraceHull(pPlayer->v.origin, pPlayer->v.origin, dont_ignore_monsters, head_hull, pPlayer, &trace);
		if (trace.fStartSolid)
			pPlayer->v.origin.z++;
		else
			break;
	}
}

//=========================================================
//=========================================================
void CBaseHalfLifeCoop::PlayerSpawn(CBasePlayer *pPlayer)
{
	// Omitir sons de captura de itens durante o spawn
	pPlayer->enable_item_pickup_sound = false;

	// Pego o indice do jogador
	int CoopPlyIndex = pPlayer->entindex() - 1;

	// Configuramos o nome do jogador e descobrimos se ele é novo no server
	if (!StartCoopPlayer(pPlayer))
		CoopPlyData[CoopPlyIndex].newplayer = true;

	// Deixar o jogador passavel e com efeitos durante algum tempo
	DisablePhysics(pPlayer);

	// Posicionar jogador no mundo
	pPlayer->pev->origin = GetPlySpawnPos(CoopPlyIndex);

	// Configuramos o jogador no caso dele NAO ser novo no server (ja ter passado por changelevel)
	if (!CoopPlyData[CoopPlyIndex].newplayer)
	{
		// Propriedades de posicionamento
		// Nota: angles, v_angle, punchangle e FIXANGLE_NO juntos restauram a visao do jogador. Isso foi bem testado
		pPlayer->pev->angles = CoopPlyData[CoopPlyIndex].angles;
		pPlayer->pev->v_angle = CoopPlyData[CoopPlyIndex].v_angle;
		pPlayer->pev->punchangle = CoopPlyData[CoopPlyIndex].punchangle;
		pPlayer->pev->fixangle = 0;
		pPlayer->pev->velocity = CoopPlyData[CoopPlyIndex].velocity;
		pPlayer->pev->flFallVelocity = CoopPlyData[CoopPlyIndex].flFallVelocity;
		if (CoopPlyData[CoopPlyIndex].bInDuck) /* Ajeita o agachamento. Eh meio bugado, mas funciona... */
		{
			pPlayer->pev->flags = FL_DUCKING;
			pPlayer->pev->button = IN_DUCK;
			FixPlayerCrouchStuck(pPlayer->edict());
			pPlayer->pev->view_ofs[2] = 3;
		}

		// Reparacao de estados gerais
		pPlayer->pev->health = CoopPlyData[CoopPlyIndex].health;
		pPlayer->pev->armorvalue = CoopPlyData[CoopPlyIndex].armorvalue;
		pPlayer->pev->team = CoopPlyData[CoopPlyIndex].team;
		pPlayer->pev->frags = CoopPlyData[CoopPlyIndex].frags;
		pPlayer->pev->weapons = CoopPlyData[CoopPlyIndex].weapons;
		pPlayer->hu3_cam_crosshair = CoopPlyData[CoopPlyIndex].hu3_cam_crosshair;
		if (CoopPlyData[CoopPlyIndex].flashlight)
			pPlayer->FlashlightTurnOn();
		if (CoopPlyData[CoopPlyIndex].hasSuit)
			pPlayer->SetHasSuit(true);

		// Restaurar godmode e notarget
		if (CoopPlyData[CoopPlyIndex].godmode)
			CoopPlyData[CoopPlyIndex].godmode ? pPlayer->pev->flags |= FL_GODMODE : pPlayer->pev->flags &= ~FL_GODMODE;
		if (CoopPlyData[CoopPlyIndex].notarget)
			CoopPlyData[CoopPlyIndex].notarget ? pPlayer->pev->flags |= FL_NOTARGET : pPlayer->pev->flags &= ~FL_NOTARGET;

		// Restaurar noclip
		if (CoopPlyData[CoopPlyIndex].noclip)
			pPlayer->pev->movetype = MOVETYPE_NOCLIP;

		// Zerar contador de respawn de itens
		CoopPlyData[CoopPlyIndex].respawnItemWait = gpGlobals->time;

		// Atualizo o HUD
		pPlayer->UpdateClientData();
	}

	// Liberar a checagem de changelevel 
	if (CoopPlyData[CoopPlyIndex].waitingforchangelevel)
		CoopPlyData[CoopPlyIndex].waitingforchangelevel = false;
}

//=========================================================
//=========================================================
Vector CBaseHalfLifeCoop::GetPlySpawnPos(int CoopPlyIndex)
{
	// A posicao correta do jogador no mundo
	Vector center(0, 0, 0);
	Vector absPos = center;

	// O ponto de spawn
	CBaseEntity* spawnPoint = nullptr;

	// Tenta pegar o ponto de spawn em relacao a algum trem com spawn consertado para coop
	char* hu3Train = (char*)CVAR_GET_STRING("coop_train_spawnpoint");
	if (!FStrEq(hu3Train, "0"))
	{
		spawnPoint = UTIL_FindEntityByString(NULL, "targetname", hu3Train);

		if (spawnPoint)
			// Garante que o jogador nao prenda no chao do trem do inicio do HL
			absPos = Vector(spawnPoint->pev->origin.x, spawnPoint->pev->origin.y, spawnPoint->pev->origin.z + 75);
	}

	// Configuramos o jogador no caso dele NAO ser novo no server (ja ter passado por changelevel)
	if (!CoopPlyData[CoopPlyIndex].newplayer)
	{
		// Tentar encontrar o local de spawn em relacao a um landmark valido
		if (absPos == center)
		{
			while ((spawnPoint = UTIL_FindEntityByTargetname(spawnPoint, hu3LandmarkName)) != nullptr)
			{
				if (FClassnameIs(spawnPoint->pev, "info_landmark"))
				{
					if (CoopPlyData[CoopPlyIndex].relPos != center)
						absPos = spawnPoint->pev->origin + CoopPlyData[CoopPlyIndex].relPos;

					break;
				}
			}
		}
	}

	// Tenta pegar o ponto de spawn em relacao a algum info_player_coop disponivel
	if (absPos == center)
	{
		while ((spawnPoint = UTIL_FindEntityByClassname(spawnPoint, "info_player_coop")) != nullptr)
		{
			absPos = spawnPoint->pev->origin + Vector(CoopPlyData[CoopPlyIndex].relPos.x, CoopPlyData[CoopPlyIndex].relPos.y, 0);

			break;
		}
	}

	// No ultimo dos casos, pega o ponto de spawn em relacao ao info_player_start
	if (absPos == center)
	{
		while ((spawnPoint = UTIL_FindEntityByClassname(spawnPoint, "info_player_start")) != nullptr)
		{
			absPos = spawnPoint->pev->origin;

			break;
		}
	}

	return absPos;
}

//=========================================================
//=========================================================
bool CBaseHalfLifeCoop::StartCoopPlayer(CBaseEntity *pPlayer)
{
	// Retorno False quando o jogador é novo ou quando ele se chama "Player".
	// Sempre seto o nome do jogador na tabela, e se ele for "Player" eu automaticamente renomeio para outra coisa
	// e despacho o comando de alteração.
	// Além disso, coloquei uma chance de 7% de avacalhar o nome das pessoas.

	char newName[MAX_PLAYER_NAME_LENGTH] = "";

	int CoopPlyIndex = pPlayer->entindex() - 1;
	string_t originalName = pPlayer->pev->netname;

	// O nome do jogador nao pode ser "Player"
	bool needsNewName = FStrEq(STRING(originalName), "Player");

	// Atualiza estado do nome
	CoopPlyData[CoopPlyIndex].pSetNewName = needsNewName;

	// Se o nome for válido...
	if (!needsNewName)
	{
		// Jogador novo = falta fazer o registro inicial
		// Jogador antigo = tudo registrado
		int plyIterator = 0;
		bool playerExists = false;
		while (!FStrEq(CoopPlyData[plyIterator].pName, ""))
		{
			if (FStrEq(STRING(originalName), CoopPlyData[plyIterator].pName))
			{
				playerExists = true;
				break;
			}
			plyIterator++;
		}

		// Registro o nome do jogador na tabela coop
		strncpy(CoopPlyData[CoopPlyIndex].pName, STRING(originalName), sizeof(CoopPlyData[CoopPlyIndex].pName) - 1);
		CoopPlyData[CoopPlyIndex].pName[sizeof(CoopPlyData[CoopPlyIndex].pName) - 1] = '\0';

		// Chance de 7% de mudar o nome do jogador novo porque sim
		if (playerExists)
			return true;
		else if (RANDOM_LONG(0, 99) > 7)
			return false;
	}

	// Começo um loop ate que o novo nome esteja aprovado
	int plyIterator;
	bool nameIsUsed;
	while (true)
	{
		plyIterator = 0;
		nameIsUsed = false;

		strncpy(newName, GENERIC_COOP_NAMES[RANDOM_LONG(0, ARRAYSIZE(GENERIC_COOP_NAMES) - 1)], MAX_PLAYER_NAME_LENGTH);

		while (!FStrEq(CoopPlyData[plyIterator].pName, ""))
		{
			// Verifico se o nome em questao eh igual ao nome que queremos validar
			if (FStrEq(newName, CoopPlyData[plyIterator].pName))
			{
				nameIsUsed = true;
				break;
			}

			plyIterator++;
		}

		if (!nameIsUsed)
			break;
	}

	// Marco o nome para aplicação
	CoopPlyData[CoopPlyIndex].pSetNewName = true;

	// Registro o nome do jogador na tabela coop
	strncpy(CoopPlyData[CoopPlyIndex].pName, newName, sizeof(CoopPlyData[CoopPlyIndex].pName) - 1);
	CoopPlyData[CoopPlyIndex].pName[sizeof(CoopPlyData[CoopPlyIndex].pName) - 1] = '\0';

	// Falta fazer o registro inicial do jogador
	return false;
}

//=========================================================
//=========================================================
bool CBaseHalfLifeCoop::LoadPlayerItems(CBasePlayer *pPlayer)
{
	// Pego o indice do jogador
	int CoopPlyIndex = pPlayer->entindex() - 1;
	playerCoopSaveRestore *CurCoopPlyData = &CoopPlyData[CoopPlyIndex];

	// Configuramos o jogador no caso dele NAO ser novo no server (ja ter passado por changelevel)
	int weaponCount = 0;

	// Os itens salvos serao verificados um a um
	while (!FStrEq(CurCoopPlyData->keepweapons[weaponCount].name, "-1"))
	{
		if (CurCoopPlyData->keepweapons[weaponCount].spawned) {
			weaponCount++;
			continue;
		}

		CurCoopPlyData->keepweapons[weaponCount].spawned = true;

		// Corrijo as munições (para a arma carregar corretamente)
		for (int ammoIdx = 0; ammoIdx < MAX_AMMO_SLOTS; ++ammoIdx)
		{
			pPlayer->m_rgAmmo[ammoIdx] = CurCoopPlyData->m_rgAmmo[CoopPlyIndex][ammoIdx];
		}

		// Restauro a arma
		CBaseEntity* pItem = pPlayer->GiveNamedItem(CurCoopPlyData->keepweapons[weaponCount].name);

		// Restauro a munição atual no clip
		CBasePlayerWeapon *pWeapon = (CBasePlayerWeapon*)pItem;
		pWeapon->m_iClip = CurCoopPlyData->keepweapons[weaponCount].clip;

		// Atualizo o ponteiro da info da arma
		CurCoopPlyData->keepweapons[weaponCount].weapon = pWeapon;

		// Corrijo as munições (para não termos munição extra)
		for (int ammoIdx = 0; ammoIdx < MAX_AMMO_SLOTS; ++ammoIdx)
		{
			pPlayer->m_rgAmmo[ammoIdx] = CurCoopPlyData->m_rgAmmo[CoopPlyIndex][ammoIdx];
		}

		// Atualizo o HUD
		pWeapon->UpdateClientData(pPlayer);

		return false;
	}

	// Corrigo a arma ativa
	if (weaponCount > 0)
	{
		CBasePlayerWeapon *pWeapon = CurCoopPlyData->keepweapons[weaponCount - 1].weapon;
		if (pWeapon)
		{
			pPlayer->SwitchWeapon(pWeapon);
		}
	}

	return true;
}

//=========================================================
//=========================================================
void CBaseHalfLifeCoop::SavePlayerItems(CBasePlayer *pPlayer)
{
	// Salvar armas e municao antes de um changelevel
	// Funcao auxiliar

	int CoopPlyIndex = pPlayer->entindex() - 1;
	playerCoopSaveRestore *CurCoopPlyData = &CoopPlyData[CoopPlyIndex];
	int weaponCount = 0; 

	// Salva a munição
	for (int ammoIdx = 0; ammoIdx < MAX_AMMO_SLOTS; ++ammoIdx)
	{
		(*CurCoopPlyData).m_rgAmmo[CoopPlyIndex][ammoIdx] = pPlayer->m_rgAmmo[ammoIdx];
	}

	// Salvar as armas
	for (int slotCount = 0; slotCount <= MAX_WEAPON_SLOTS; slotCount++)
	{
		// Tenta pegar a primeira arma do slot
		CBasePlayerItem *pItem = pPlayer->m_rgpPlayerItems[slotCount];

		// Se existir a primeira arma, vamos salva-la e seguir fazendo o mesmo para as proximas caso estas existam
		while (pItem)
		{
			CBasePlayerWeapon* pWeapon = pItem->GetWeaponPtr();
			if (pWeapon && pWeapon->UseDecrement() && pWeapon->m_iId != WEAPON_NONE)
			{
				// Se a arma em questao estiver ativa nos devemos pula-la (ela precisa entrar por ultimo)
				if (pPlayer->m_pActiveItem == pItem)
				{
					pItem = pItem->m_pNext;
					continue;
				}

				// Salva as infos da arma
				SavePlayerItem(pPlayer, pWeapon, weaponCount);

				weaponCount++;
			}

			// pItem eh apontado para a proxima arma
			pItem = pItem->m_pNext;
		}
	}

	if (pPlayer->m_pActiveItem)
	{
		// Adicionamos a arma ativa no ultimo slot da lista (vai fazer ela carregar corretamente apos o changelevel)
		SavePlayerItem(pPlayer, (CBasePlayerWeapon*)pPlayer->m_pActiveItem, weaponCount);
		weaponCount++;
	}

	// Lista termina no "-1"
 	strcpy((*CurCoopPlyData).keepweapons[weaponCount].name, "-1");
}

//=========================================================
//=========================================================
// Salva as infos da arma
void CBaseHalfLifeCoop::SavePlayerItem(CBasePlayer *pPlayer, CBasePlayerWeapon *pWeapon, int weaponCount)
{
	int CoopPlyIndex = pPlayer->entindex() - 1;
	playerCoopSaveRestore *CurCoopPlyData = &CoopPlyData[CoopPlyIndex];

	// Nome da arma (só funciona no primeiro changelevel, depois vem ponteiros velhos no wepInfo e não consigo ajeitar)
	// ItemInfo wepInfo = g_wepInfo[weaponCount];
	// memset(&wepInfo, 0, sizeof(wepInfo));
	// pWeapon->GetItemInfo(&wepInfo);
	// strncpy((*CurCoopPlyData).keepweapons[weaponCount].name, wepInfo.pszName, sizeof((*CurCoopPlyData).keepweapons[weaponCount].name) - 1);
	// (*CurCoopPlyData).keepweapons[weaponCount].name[sizeof((*CurCoopPlyData).keepweapons[weaponCount].name) - 1] = '\0';

	// Nome da arma (versão gato cagada)
	int wepId = pWeapon->m_iId;
	const char* weaponName = (wepId >= 0 && wepId < (int)(sizeof(WeaponIdToName)/sizeof(*WeaponIdToName)))
		? WeaponIdToName[wepId]
		: "weapon_crowbar"; // qlqr coisa

	strncpy(CoopPlyData->keepweapons[weaponCount].name, weaponName, sizeof(CoopPlyData->keepweapons[weaponCount].name) - 1);
	CoopPlyData->keepweapons[weaponCount].name[sizeof(CoopPlyData->keepweapons[weaponCount].name) - 1] = '\0';

	// Quantidade da municao 1 em uso
	(*CurCoopPlyData).keepweapons[weaponCount].clip = pWeapon->m_iClip;

	// Estado de respawn
	CurCoopPlyData->keepweapons[weaponCount].spawned = false;
}

//=========================================================
//=========================================================
void CBaseHalfLifeCoop::ChangeLevelCoopToogle()
{
	int state = ChangeLevelVolume();

	// Verifica se todos os jogadores estao dentro de um mesmo changelevel e segue adiante
	if (state == 1)
		ChangeLevelCoop();
		// Se nao estiverem mas existir pelo menos um, ligo a verificacao de changelevel no think
	else if (state == 2)
		hu3ChangelevelCheck = true;
}

//=========================================================
//=========================================================
void CBaseHalfLifeCoop::ChangeLevelCoop()
{
	int i = 0;

	// Tento pegar um Landmark valido
	CBaseEntity* pLandmark = nullptr;
	if (!FStrEq(hu3LandmarkName, ""))
	{
		CBaseEntity* pLandmarkAux = nullptr;

		while ((pLandmarkAux = UTIL_FindEntityByTargetname(pLandmarkAux, hu3LandmarkName)) != nullptr)
		{
			if (FClassnameIs(pLandmarkAux->pev, "info_landmark"))
			{
				pLandmark = pLandmarkAux;
				break;
			}
		}
	}

	// Reseto o comando mp_hu3_trainspawnpoint
	char* hTarget = (char*)CVAR_GET_STRING("coop_train_spawnpoint");
	if (!FStrEq(hTarget, "0"))
		CVAR_SET_STRING("coop_train_spawnpoint", "0");

	// Preencho a tabela de infos dos players novamente
	for (i = 0; i < gpGlobals->maxClients; i++)
	{
		CBasePlayer *pPlayer = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i + 1));

		if (pPlayer && pPlayer->IsPlayer())
		{
			// Calculo a distancia do player ate o landmark
			Vector absPos = Vector(0, 0, 0);
			Vector plyPos = Vector(0, 0, 0);
			Vector relPos = Vector(0, 0, 0);

			if (pLandmark)
			{
				absPos = pLandmark->pev->origin;
				plyPos = pPlayer->pev->origin;
				relPos = plyPos - absPos;
			}

			// Vejo se o jogador esta abaixado
			bool inDuck = false;
			if (pPlayer->pev->flags & FL_DUCKING)
				inDuck = true;

			// Vejo se a lanterna esta ligada
			bool flashlightState = false;
			if (pPlayer->FlashlightIsOn())
				flashlightState = true;

			// Verifico o godmode
			bool godmodeState = false;
			if (pPlayer->pev->flags & FL_GODMODE)
				godmodeState = true;

			// Verifico o notarget
			bool notargetState = false;
			if (pPlayer->pev->flags & FL_NOTARGET)
				notargetState = true;

			// Verifico o noclip
			bool noclipState = false;
			if (pPlayer->pev->movetype == MOVETYPE_NOCLIP)
				noclipState = true;

			// Salvo as infos gerais
			CoopPlyData[i].relPos = relPos;
			CoopPlyData[i].v_angle = pPlayer->pev->v_angle;
			CoopPlyData[i].punchangle = pPlayer->pev->punchangle;
			CoopPlyData[i].angles = pPlayer->pev->angles;
			CoopPlyData[i].velocity = pPlayer->pev->velocity;
			CoopPlyData[i].flFallVelocity = pPlayer->pev->flFallVelocity;
			CoopPlyData[i].bInDuck = inDuck;
			CoopPlyData[i].flashlight = flashlightState;
			CoopPlyData[i].team = pPlayer->pev->team;
			CoopPlyData[i].frags = pPlayer->pev->frags;
			CoopPlyData[i].health = pPlayer->pev->health;
			CoopPlyData[i].armorvalue = pPlayer->pev->armorvalue;
			CoopPlyData[i].weapons = pPlayer->pev->weapons;
			CoopPlyData[i].newplayer = false;
			CoopPlyData[i].changinglevel = true;
			CoopPlyData[i].godmode = godmodeState;
			CoopPlyData[i].notarget = notargetState;
			CoopPlyData[i].noclip = noclipState;
			CoopPlyData[i].respawncommands = true;
			CoopPlyData[i].waitingforchangelevel = true;
			CoopPlyData[i].hu3_cam_crosshair = pPlayer->hu3_cam_crosshair;
			CoopPlyData[i].hasSuit = pPlayer->HasSuit();

			// Salvo as infos de municao e armas
			// Nota: fundamental pra ajeitar a munição
			SavePlayerItems(pPlayer);

			// Limpo o jogador
			pPlayer->RemoveAllItems(true);
			// clear attack/use commands from player
			pPlayer->m_afButtonPressed = 0;
			pPlayer->pev->button = 0;
			pPlayer->m_afButtonReleased = 0;
		}
	}

	// Agora comeco o processo de troca de mapa
	char comando[30] = "changelevel ";
	strcat(strcat(comando, hu3NextMap), ";");
	SERVER_COMMAND(comando);
}

//=========================================================
//=========================================================
// Verificar se jogador esta dentro da area do trigger_changelevel
int CBaseHalfLifeCoop::ChangeLevelVolume()
{
	// Suporte para 2 changelevels por mapa
	// Retorno:
	// 1 = Todos os jogadores estao no mesmo changelevel
	// 2 = Existem jogadores em changelevel, mas nao no mesmo
	// 3 = Nao ha nenhum jogador em changelevel

	CBaseEntity* pEntity = nullptr;
	CBaseEntity* pChangelevel1 = nullptr;
	CBaseEntity* pChangelevel2 = nullptr;
	CBaseEntity* temp = nullptr;
	int trigger0 = 0, trigger1 = 0, i, plyCount = 0;

	// Conta o total de jogadores
	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		pEntity = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(i));

		if (pEntity && pEntity->IsPlayer())
			plyCount++;
		else
			break;
	}

	// Pega os trigger_changelevel
	while ((temp = UTIL_FindEntityByClassname(temp, "trigger_changelevel")) != nullptr)
	{
		if (!pChangelevel1)
			pChangelevel1 = temp;
		else
			pChangelevel2 = temp;
	}

	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		edict_t *pPlayer = g_engfuncs.pfnPEntityOfEntIndex(i);
		pEntity = CBaseEntity::Instance(pPlayer);

		if (pEntity && pEntity->IsPlayer())
		{
			bool plyInTrigger = false;

			// Conta os jogadores em cada trigger_changelevel
			if (pChangelevel1 && pChangelevel1->Intersects(pEntity))
			{
				trigger0++;
				plyInTrigger = true;
			}
			else if (pChangelevel2 && pChangelevel2->Intersects(pEntity))
			{
				trigger1++;
				plyInTrigger = true;
			}

			// Aplica ou remove efeitos nos jogadores e avisa sobre o estado do changelevel
			if (plyInTrigger)
			{
				if (!CoopPlyData[pEntity->entindex()].waitingforchangelevel)
				{
					DisablePhysics(pEntity);
					CLIENT_COMMAND(pPlayer, "say Changelevel|G;");
					CoopPlyData[pEntity->entindex()].waitingforchangelevel = true;
				}
			}
			else
			{
				if (CoopPlyData[pEntity->entindex()].waitingforchangelevel)
				{
					EnablePhysics(pEntity);
					CLIENT_COMMAND(pPlayer, "say Changelevel|R;");
					CoopPlyData[pEntity->entindex()].waitingforchangelevel = false;
				}
			}
		}
	}

	// Forcar o changelevel se o mapa estiver configurado para isso
	bool shouldForceChangelevel = FStrEq(CVAR_GET_STRING("coop_force_changelevel"), "1");

	// Todos os jogadores estao no mesmo changelevel
	if (plyCount == trigger0 || trigger0 > 0 && shouldForceChangelevel)
	{
		CChangeLevel* activatedChangelevel = (CChangeLevel*) pChangelevel1;

		strncpy(hu3LandmarkName, activatedChangelevel->m_szLandmarkName, sizeof(hu3LandmarkName) - 1);
		hu3LandmarkName[sizeof(hu3LandmarkName) - 1] = '\0';
		strncpy(hu3NextMap, activatedChangelevel->m_szMapName, sizeof(hu3NextMap) - 1);
		hu3NextMap[sizeof(hu3NextMap) - 1] = '\0';

		return 1;
	} 
	else if (plyCount == trigger1 || trigger1 > 0 && shouldForceChangelevel)
	{
		CChangeLevel* activatedChangelevel = (CChangeLevel*)pChangelevel2;

		strncpy(hu3LandmarkName, activatedChangelevel->m_szLandmarkName, sizeof(hu3LandmarkName) - 1);
		hu3LandmarkName[sizeof(hu3LandmarkName) - 1] = '\0';
		strncpy(hu3NextMap, activatedChangelevel->m_szMapName, sizeof(hu3NextMap) - 1);
		hu3NextMap[sizeof(hu3NextMap) - 1] = '\0';

		return 1;
	}
	// Existem jogadores em changelevel, mas nao todos no mesmo
	else if (trigger0 > 0 || trigger1 > 0)
		return 2;
	// Nao ha nenhum jogador em changelevel
	return 3;
}

//=========================================================
//=========================================================
bool CBaseHalfLifeCoop::FPlayerCanRespawn(CBasePlayer *pPlayer)
{
	return true;
}

//=========================================================
//=========================================================
float CBaseHalfLifeCoop::FlPlayerSpawnTime(CBasePlayer *pPlayer)
{
	return gpGlobals->time; // Imediatamente
}

bool CBaseHalfLifeCoop::AllowAutoTargetCrosshair()
{
	return (aimcrosshair.value != 0);
}

//=========================================================
// IPointsForKill - how many points awarded to anyone
// that kills this player?
//=========================================================
int CBaseHalfLifeCoop::IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled)
{
	return 1;
}

//=========================================================
// PlayerKilled - someone/something killed this player
//=========================================================
void CBaseHalfLifeCoop::PlayerKilled(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pInflictor)
{
	int victimIndex = pVictim->entindex();
	int CoopPlyIndex = pVictim->entindex() - 1;

	// Stado de respawn
	CoopPlyData[CoopPlyIndex].respawning = true;

	// Reseto o estado das armas
	playerCoopSaveRestore *CurCoopPlyData = &CoopPlyData[CoopPlyIndex];
	int weaponCount = 0;
	while (!FStrEq(CurCoopPlyData->keepweapons[weaponCount].name, "-1"))
	{
		CurCoopPlyData->keepweapons[weaponCount].spawned = false;
		weaponCount++;
	}

	// Coisas de aviso de morte
	DeathNotice(pVictim, pKiller, pInflictor);

	CBaseEntity* pKillerEnt = CBaseEntity::Instance(pKiller);
	CBasePlayer* pKillerPly = nullptr;
	if (pKillerEnt->IsPlayer())
		pKillerPly = (CBasePlayer*)pKillerEnt;

	pVictim->m_iDeaths += 1;

	FireTargets("game_playerdie", pVictim, pVictim, USE_TOGGLE, 0);

	if (pVictim == pKillerPly)
	{
		// killed self
		pVictim->pev->frags = pVictim->pev->frags - 1;
	}
	else if (pKillerPly)
	{
		// if a player dies in a deathmatch game and the killer is a client, award the killer some points
		pKillerPly->pev->frags = pKillerPly->pev->frags + IPointsForKill(pKillerPly, pVictim);

		FireTargets("game_playerkill", pKillerPly, pKillerPly, USE_TOGGLE, 0);
	}
	else
	{
		// killed by the world
		pKillerEnt->pev->frags = pKillerEnt->pev->frags - 1;
	}

	// update the scores
	// killed scores
	MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
	WRITE_BYTE(pVictim->entindex());
	WRITE_SHORT(pVictim->pev->frags);
	WRITE_SHORT(pVictim->m_iDeaths);
	WRITE_SHORT(0);
	WRITE_SHORT(GetTeamIndex(pVictim->m_szTeamName) + 1);
	MESSAGE_END();

	// killers score, if it's a player
	if (pKillerPly)
	{
		MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
		WRITE_BYTE(pKillerPly->entindex());
		WRITE_SHORT(pKillerPly->pev->frags);
		WRITE_SHORT(pKillerPly->m_iDeaths);
		WRITE_SHORT(0);
		WRITE_SHORT(GetTeamIndex(pKillerPly->m_szTeamName) + 1);
		MESSAGE_END();

		// let the killer paint another decal as soon as he'd like.
		pKillerPly->m_flNextDecalTime = gpGlobals->time;
	}

	if (pVictim->HasNamedPlayerItem("weapon_satchel"))
	{
		DeactivateSatchels(pVictim);
	}
}

//=========================================================
// Deathnotice. 
//=========================================================
void CBaseHalfLifeCoop::DeathNotice(CBasePlayer* pVictim, entvars_t* _pKiller, entvars_t* _pInflictor)
{
	CBaseEntity* pKillerEnt = CBaseEntity::Instance(_pKiller);
	CBaseEntity* pInflictorEnt = CBaseEntity::Instance(_pInflictor);

	// Work out what killed the player, and send a message to all clients about it
	const char *killer_weapon_name = "world";		// by default, the player is killed by the world
	int killer_index = 0;

	// Hack to fix name change
	const char* const tau = "tau_cannon";
	const char* const gluon = "gluon gun";

	if ((pKillerEnt->pev->flags & FL_CLIENT) != 0)
	{
		killer_index = pKillerEnt->entindex();

		if (pInflictorEnt)
		{
			if (pInflictorEnt == pKillerEnt)
			{
				// If the inflictor is the killer,  then it must be their current weapon doing the damage
				CBasePlayer *pPlayer = (CBasePlayer*)pKillerEnt;

				if (pPlayer->m_pActiveItem)
				{
					killer_weapon_name = pPlayer->m_pActiveItem->pszName();
				}
			}
			else
			{
				killer_weapon_name = STRING(pInflictorEnt->pev->classname);  // it's just that easy
			}
		}
	}
	else
	{
		killer_weapon_name = STRING(pInflictorEnt->pev->classname);
	}

	// strip the monster_* or weapon_* from the inflictor's classname
	if (strncmp(killer_weapon_name, "weapon_", 7) == 0)
		killer_weapon_name += 7;
	else if (strncmp(killer_weapon_name, "monster_", 8) == 0)
		killer_weapon_name += 8;
	else if (strncmp(killer_weapon_name, "func_", 5) == 0)
		killer_weapon_name += 5;

	MESSAGE_BEGIN(MSG_ALL, gmsgDeathMsg);
	WRITE_BYTE(killer_index);						// the killer
	WRITE_BYTE(pVictim->entindex());		// the victim
	WRITE_STRING(killer_weapon_name);		// what they were killed by (should this be a string?)
	MESSAGE_END();

	// replace the code names with the 'real' names
	if (FStrEq(killer_weapon_name, "egon"))
		killer_weapon_name = gluon;
	else if (FStrEq(killer_weapon_name, "gauss"))
		killer_weapon_name = tau;

	if (pVictim == pKillerEnt)
	{
		// killed self

		// team match?
		if (g_teamplay)
		{
			UTIL_LogPrintf("\"%s<%i><%s><%s>\" cometeu suicidio com um(a) \"%s\"\n",
				pVictim->pev->netname,
				GETPLAYERUSERID(pVictim->edict()),
				GETPLAYERAUTHID(pVictim->edict()),
				g_engfuncs.pfnInfoKeyValue(g_engfuncs.pfnGetInfoKeyBuffer(pVictim->edict()), "model"),
				killer_weapon_name);
		}
		else
		{
			UTIL_LogPrintf("\"%s<%i><%s><%i>\" cometeu suicidio com um(a) \"%s\"\n",
				pVictim->pev->netname,
				GETPLAYERUSERID(pVictim->edict()),
				GETPLAYERAUTHID(pVictim->edict()),
				GETPLAYERUSERID(pVictim->edict()),
				killer_weapon_name);
		}
	}
	else if ((pKillerEnt->pev->flags & FL_CLIENT) != 0)
	{
		// team match?
		if (g_teamplay)
		{
			UTIL_LogPrintf("\"%s<%i><%s><%s>\" matou \"%s<%i><%s><%s>\" com um(a) \"%s\"\n",
				pKillerEnt->pev->netname,
				GETPLAYERUSERID(pKillerEnt->edict()),
				GETPLAYERAUTHID(pKillerEnt->edict()),
				g_engfuncs.pfnInfoKeyValue(g_engfuncs.pfnGetInfoKeyBuffer(pKillerEnt->edict()), "model"),
				pVictim->pev->netname,
				GETPLAYERUSERID(pVictim->edict()),
				GETPLAYERAUTHID(pVictim->edict()),
				g_engfuncs.pfnInfoKeyValue(g_engfuncs.pfnGetInfoKeyBuffer(pVictim->edict()), "model"),
				killer_weapon_name);
		}
		else
		{
			UTIL_LogPrintf("\"%s<%i><%s><%i>\" matou \"%s<%i><%s><%i>\" com um(a) \"%s\"\n",
				pKillerEnt->pev->netname,
				GETPLAYERUSERID(pKillerEnt->edict()),
				GETPLAYERAUTHID(pKillerEnt->edict()),
				GETPLAYERUSERID(pKillerEnt->edict()),
				pVictim->pev->netname,
				GETPLAYERUSERID(pVictim->edict()),
				GETPLAYERAUTHID(pVictim->edict()),
				GETPLAYERUSERID(pVictim->edict()),
				killer_weapon_name);
		}
	}
	else
	{
		// killed by the world

		// team match?
		if (g_teamplay)
		{
			UTIL_LogPrintf("\"%s<%i><%s><%s>\" cometeu suicidio com um(a) \"%s\" (world)\n",
				pVictim->pev->netname,
				GETPLAYERUSERID(pVictim->edict()),
				GETPLAYERAUTHID(pVictim->edict()),
				g_engfuncs.pfnInfoKeyValue(g_engfuncs.pfnGetInfoKeyBuffer(pVictim->edict()), "model"),
				killer_weapon_name);
		}
		else
		{
			UTIL_LogPrintf("\"%s<%i><%s><%i>\" cometeu suicidio com um(a) \"%s\" (world)\n",
				pVictim->pev->netname,
				GETPLAYERUSERID(pVictim->edict()),
				GETPLAYERAUTHID(pVictim->edict()),
				GETPLAYERUSERID(pVictim->edict()),
				killer_weapon_name);
		}
	}

	MESSAGE_BEGIN(MSG_SPEC, SVC_DIRECTOR);
	WRITE_BYTE(9);	// command length in bytes
	WRITE_BYTE(DRC_CMD_EVENT);	// player killed
	WRITE_SHORT(pVictim->entindex());	// index number of primary entity
	if (pInflictorEnt)
		WRITE_SHORT(pInflictorEnt->entindex());	// index number of secondary entity
	else
		WRITE_SHORT(pKillerEnt->entindex());	// index number of secondary entity
	WRITE_LONG(7 | DRC_FLAG_DRAMATIC);   // eventflags (priority and flags)
	MESSAGE_END();

	char szBuffer[512];

	if ((pKillerEnt->pev->flags & FL_MONSTER) != 0)
	{
		// killed by a monster
		snprintf(szBuffer, sizeof(szBuffer), "%s foi morto por um monstro.\n", pVictim->pev->netname);
	}
	else if (pKillerEnt == pVictim)
	{
		snprintf(szBuffer, sizeof(szBuffer), "%s cometeu suicidio.\n", pVictim->pev->netname);
	}
	else if ((pKillerEnt->pev->flags & FL_CLIENT) != 0)
	{
		snprintf(szBuffer, sizeof(szBuffer), "%s : %s : %s\n", pKillerEnt->pev->netname, killer_weapon_name, pVictim->pev->netname);
	}
	else if (FClassnameIs(pKillerEnt->pev, "worldspawn"))
	{
		snprintf(szBuffer, sizeof(szBuffer), "%s caiu ou se afogou ou qualquer coisa.\n", pVictim->pev->netname);
	}
	else if (pKillerEnt->pev->solid == SOLID_BSP)
	{
		snprintf(szBuffer, sizeof(szBuffer), "%s foi assaltado.\n", pVictim->pev->netname);
	}
	else
	{
		snprintf(szBuffer, sizeof(szBuffer), "%s morreu misteriosamente.\n", pVictim->pev->netname);
	}

	//  Print a standard message
	UTIL_ClientPrintAll(HUD_PRINTNOTIFY, szBuffer);
}

//=========================================================
// PlayerGotWeapon - player has grabbed a weapon that was
// sitting in the world
//=========================================================
void CBaseHalfLifeCoop::PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
}

//=========================================================
// FlWeaponRespawnTime - what is the time in the future
// at which this weapon may spawn?
//=========================================================
float CBaseHalfLifeCoop::FlWeaponRespawnTime(CBasePlayerItem *pWeapon)
{
	return gpGlobals->time + 0.1; // O.1 para dar tempo de eu colocar flags anti spawn
}

//=========================================================
// FlWeaponRespawnTime - Returns 0 if the weapon can respawn 
// now,  otherwise it returns the time at which it can try
// to spawn again.
//=========================================================

float CBaseHalfLifeCoop::FlWeaponTryRespawn(CBasePlayerItem* pWeapon)
{
	if (pWeapon && WEAPON_NONE != pWeapon->m_iId && (pWeapon->iFlags() & ITEM_FLAG_LIMITINWORLD) != 0)
	{
		if (NUMBER_OF_ENTITIES() < (gpGlobals->maxEntities - ENTITY_INTOLERANCE))
			return 0;

		// we're past the entity tolerance level,  so delay the respawn
		return FlWeaponRespawnTime(pWeapon);
	}

	return 0;
}

//=========================================================
// VecWeaponRespawnSpot - where should this weapon spawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CBaseHalfLifeCoop::VecWeaponRespawnSpot(CBasePlayerItem *pWeapon)
{
	return pWeapon->pev->origin;
}

//=========================================================
// WeaponShouldRespawn - any conditions inhibiting the
// respawning of this weapon?
//=========================================================
int CBaseHalfLifeCoop::WeaponShouldRespawn(CBasePlayerItem *pWeapon)
{
	if ((pWeapon->pev->spawnflags & SF_NORESPAWN) != 0)
	{
		return GR_WEAPON_RESPAWN_NO;
	}

	return GR_WEAPON_RESPAWN_YES;
}

//=========================================================
// CanHaveWeapon - returns false if the player is not allowed
// to pick up this weapon
//=========================================================
bool CBaseHalfLifeCoop::CanHavePlayerItem(CBasePlayer* pPlayer, CBasePlayerItem* pItem)
{
	if (weaponstay.value > 0)
	{
		if ((pItem->iFlags() & ITEM_FLAG_LIMITINWORLD) != 0)
			return CGameRules::CanHavePlayerItem(pPlayer, pItem);

		// check if the player already has this weapon
		for (int i = 0; i < MAX_ITEM_TYPES; i++)
		{
			CBasePlayerItem* it = pPlayer->m_rgpPlayerItems[i];

			while (it != NULL)
			{
				if (it->m_iId == pItem->m_iId)
				{
					return false;
				}

				it = it->m_pNext;
			}
		}
	}

	return CGameRules::CanHavePlayerItem(pPlayer, pItem);
}

//=========================================================
//=========================================================
bool CBaseHalfLifeCoop::CanHaveItem(CBasePlayer *pPlayer, CItem *pItem)
{
	return true;
}

//=========================================================
//=========================================================
void CBaseHalfLifeCoop::PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem)
{
}

//=========================================================
//=========================================================
int CBaseHalfLifeCoop::ItemShouldRespawn(CItem *pItem)
{
	if ((pItem->pev->spawnflags & SF_NORESPAWN) != 0)
	{
		return GR_ITEM_RESPAWN_NO;
	}

	return GR_ITEM_RESPAWN_YES;
}

//=========================================================
// At what time in the future may this Item respawn?
//=========================================================
float CBaseHalfLifeCoop::FlItemRespawnTime(CItem *pItem)
{
	CBaseEntity *pItem2 = (CBaseEntity *)pItem;

	if ((FClassnameIs(pItem2->pev, "item_healthkit")) || (FClassnameIs(pItem2->pev, "item_battery")))
	{
		return gpGlobals->time + 5; // Vida e escudo demoram 5 segundos
	}

	return gpGlobals->time + 0.1; // O.1 para dar tempo de eu colocar flags anti spawn
}

//=========================================================
// Where should this item respawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CBaseHalfLifeCoop::VecItemRespawnSpot(CItem *pItem)
{
	return pItem->pev->origin;
}

//=========================================================
//=========================================================
void CBaseHalfLifeCoop::PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount)
{
}

//=========================================================
//=========================================================
bool CBaseHalfLifeCoop::IsAllowedToSpawn(CBaseEntity *pEntity)
{
	return true;
}

//=========================================================
//=========================================================
int CBaseHalfLifeCoop::AmmoShouldRespawn(CBasePlayerAmmo *pAmmo)
{
	if ((pAmmo->pev->spawnflags & SF_NORESPAWN) != 0)
	{
		return GR_AMMO_RESPAWN_NO;
	}

	return GR_AMMO_RESPAWN_YES;
}

//=========================================================
//=========================================================
float CBaseHalfLifeCoop::FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo)
{
	return gpGlobals->time + 2; // 2 Segundos para a municao voltar
}

//=========================================================
//=========================================================
Vector CBaseHalfLifeCoop::VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo)
{
	return pAmmo->pev->origin;
}

//=========================================================
//=========================================================
float CBaseHalfLifeCoop::FlHealthChargerRechargeTime(void)
{
	return 60;
}

float CBaseHalfLifeCoop::FlHEVChargerRechargeTime(void)
{
	return 30;
}

//=========================================================
//=========================================================
int CBaseHalfLifeCoop::DeadPlayerWeapons(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_GUN_NO;
}

//=========================================================
//=========================================================
int CBaseHalfLifeCoop::DeadPlayerAmmo(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_AMMO_NO;
}

edict_t* CBaseHalfLifeCoop::GetPlayerSpawnSpot(CBasePlayer* pPlayer)
{
	// Já não me lembro mais, mas acho que eu deixo isso rolar e movo o jogador depois.
	edict_t* pentSpawnSpot = CGameRules::GetPlayerSpawnSpot(pPlayer);
	if (IsMultiplayer() && !FStringNull(pentSpawnSpot->v.target))
	{
		FireTargets(STRING(pentSpawnSpot->v.target), pPlayer, pPlayer, USE_TOGGLE, 0);
	}

	return pentSpawnSpot;
}


//=========================================================
//=========================================================
int CBaseHalfLifeCoop::PlayerRelationship(CBaseEntity *pPlayer, CBaseEntity *pTarget)
{
	// Inimigos para todos os lados
	return GR_NOTTEAMMATE;
}

bool CBaseHalfLifeCoop::PlayFootstepSounds(CBasePlayer *pl, float fvol)
{
	if (g_footsteps && g_footsteps->value == 0)
		return false;

	if (pl->IsOnLadder() || pl->pev->velocity.Length2D() > PLAYER_STEP_SOUND_SPEED)
		return true;  // only make step sounds in multiplayer if the player is moving fast enough

	return false;
}

bool CBaseHalfLifeCoop::FAllowFlashlight()
{
	return flashlight.value != 0;
}

//=========================================================
//=========================================================
bool CBaseHalfLifeCoop::FAllowMonsters()
{
	return (allowmonsters.value != 0);
}

void CBaseHalfLifeCoop::SendMOTDToClient(CBasePlayer* pPlayer)
{
	int CoopPlyIndex = pPlayer->entindex() - 1;
	if (CoopPlyData[CoopPlyIndex].newplayer)
	{
		// read from the MOTD.txt file
		int length, char_count = 0;
		char *pFileList;
		char *aFileList = pFileList = (char*)LOAD_FILE_FOR_ME((char *)CVAR_GET_STRING("motdfile"), &length);

		// send the server name
		MESSAGE_BEGIN(MSG_ONE, gmsgServerName, NULL, pPlayer->pev);
		WRITE_STRING(CVAR_GET_STRING("hostname"));
		MESSAGE_END();

		// Send the message of the day
		// read it chunk-by-chunk,  and send it in parts

		while (pFileList && *pFileList && char_count < MAX_MOTD_LENGTH)
		{
			char chunk[MAX_MOTD_CHUNK + 1];

			if (strlen(pFileList) < MAX_MOTD_CHUNK)
			{
				strcpy(chunk, pFileList);
			}
			else
			{
				strncpy(chunk, pFileList, MAX_MOTD_CHUNK);
				chunk[MAX_MOTD_CHUNK] = 0;		// strncpy doesn't always append the null terminator
			}

			char_count += strlen(chunk);
			if (char_count < MAX_MOTD_LENGTH)
				pFileList = aFileList + char_count;
			else
				*pFileList = 0;

			MESSAGE_BEGIN(MSG_ONE, gmsgMOTD, NULL, pPlayer->pev);
			WRITE_BYTE(*pFileList ? 0 : 1);	// 0 means there is still more message to come
			WRITE_STRING(chunk);
			MESSAGE_END();
		}

		FREE_FILE(aFileList);
	}
}
