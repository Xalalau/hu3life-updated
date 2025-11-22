// ##############################
// HU3-LIFE HOMEM DO GÁS
// ##############################

/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "explode.h"
#include "zombie.h"
#include "effects.h"
#include "butano.h"

int g_iButaneFireSprite = 0;

LINK_ENTITY_TO_CLASS(monster_butano, CButano);

// Sons butano
const char *CButano::pAttackHitSounds[] =
{
	"butano/gaas.wav",
};

const char *CButano::pAttackMissSounds[] =
{
	"butano/fluom.wav",
	"butano/fluim.wav",
};

const char *CButano::pAttackSounds[] =
{
	"butano/gaas.wav",
};

const char *CButano::pIdleSounds[] =
{
	"butano/plam-pluim8.wav",
	"butano/plam.wav",
	"butano/pluim.wav",
};

const char *CButano::pAlertSounds[] =
{
	"butano/plam-pluim-plam-plam.wav",
	"butano/plam-pluim-plam-plim.wav",
};

const char *CButano::pPainSounds[] =
{
	"butano/plam.wav",
	"butano/pluim.wav",
	"butano/gaiz.wav",
	"butano/o-gaiz.wav",
};

void CButano::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/butano.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->health = gSkillData.zombieHealth;
	pev->view_ofs = VEC_VIEW;
	m_flFieldOfView = -1.0; // Ver 360 graus
	m_MonsterState = MONSTERSTATE_HUNT;
	m_afCapability = bits_CAP_DOORS_GROUP;

	pev->nextthink = gpGlobals->time + 0.1;
	m_flTimeToExplode = gpGlobals->time + VAZAMENTO_BUTANO;

	SetNewSpawn();

	pev->body = RANDOM_LONG(0, 1); // 2 botijoes por escolher

	m_flLastMoveTime = gpGlobals->time;
	m_vecLastPos = pev->origin;

	MonsterInit();
}

void CButano::Precache()
{
	PRECACHE_MODEL("models/butano.mdl");

	//Sons butano
	PRECACHE_SOUND("butano/gaas.wav");
	PRECACHE_SOUND("butano/o-gaiz.wav");
	PRECACHE_SOUND("butano/gaiz.wav");

	PRECACHE_SOUND("butano/plam.wav");
	PRECACHE_SOUND("butano/pluim.wav");
	PRECACHE_SOUND("butano/fluom.wav");
	PRECACHE_SOUND("butano/fluim.wav");

	PRECACHE_SOUND("butano/plam-pluim-plam-plam.wav");
	PRECACHE_SOUND("butano/plam-pluim-plam-plim.wav");
	PRECACHE_SOUND("butano/plam-pluim8.wav");

	// Sprite de incêndio
	g_iButaneFireSprite = PRECACHE_MODEL("sprites/fire.spr");
}

// Busca pontos de spawn (info_targets) e escolhe um (aleatoriamente)
// Obs: essa funcao foi feita com o singleplayer e o modo coop em mente
void CButano::SetNewSpawn()
{
	CBaseEntity* pSpawnPoint = nullptr;
	CBaseEntity* hu3Player = nullptr;
	int plyQuant = 0;
	Vector butanoSpawnVec, dist, temp;

	// Numero de jogadores
	while ((hu3Player = UTIL_FindEntityByClassname(hu3Player, "player")) != nullptr)
		plyQuant++;

	// Escolher um jogador aleatoriamente
	hu3Player = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(RANDOM_LONG(1, plyQuant)));

	// Numero de info_targets validos (proximos ao player escolhido) e suas posicoes
	while ((pSpawnPoint = UTIL_FindEntityByClassname(pSpawnPoint, "info_target")) != nullptr)
	{
		// Vetor que vai do NPC ate o player (seu tamanho eh a distancia entre essas entidades)
		dist = hu3Player->pev->origin - pSpawnPoint->pev->origin;

		// Detectar pontos validos e pegar um aleatoriamente (entre 100 a 450 unidades de distancia)
		if (dist.Length() > 100 && dist.Length() < 450)
		{
			// Pego o primeiro ponto (garanto o spawn)
			if (!butanoSpawnVec)
				butanoSpawnVec = pSpawnPoint->pev->origin;
			// Tento selecionar outro ponto aleatoriamente (garanto um comportamento mais interessante)
			else if (RANDOM_LONG(0, 20) > 13) 
			{
				butanoSpawnVec = pSpawnPoint->pev->origin;
				break;
			}
		}
	}

	// Reposiciono a origem do NPC em local valido
	if (butanoSpawnVec)
	{
		pev->origin = butanoSpawnVec;

		// Faco o NPC olhar para o jogador escolhido
		temp = UTIL_VecToAngles(hu3Player->pev->origin - butanoSpawnVec);
		pev->angles.y = temp.y;
	}
	else
	{
        // Fallback: coloca em qualquer info_target
        CBaseEntity* pAnyTarget = NULL;

        pAnyTarget = UTIL_FindEntityByClassname(NULL, "info_target");
        if(pAnyTarget)
        {
            pev->origin = pAnyTarget->pev->origin;

            // Ainda faz ele olhar pro jogador escolhido
            temp = UTIL_VecToAngles(hu3Player->pev->origin - pev->origin);
            pev->angles.y = temp.y;
        }
		else
		{
			// Nao achou ponto de spawn, remove o NPC
			UTIL_Remove(this);
		}
	}
}

void CreateButaneFire(const Vector& origin)
{
	// Hardcoded 6 segundos

	// Sprite

	CSprite* pSprite = CSprite::SpriteCreate("sprites/fire.spr", origin, true);
    if(!pSprite)
        return;

    pSprite->pev->rendermode = kRenderTransAdd;
    pSprite->pev->renderamt = 255;
    pSprite->pev->scale = 4.0f;

    pSprite->AnimateAndDie(2.5);

	// Iluminação

    float lifeTime = 6;
    int radius = 20;
    int life = (int)(lifeTime * 10.0f);

    MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, origin);
        WRITE_BYTE(TE_DLIGHT);
        WRITE_COORD(origin.x);
        WRITE_COORD(origin.y);
        WRITE_COORD(origin.z);
        WRITE_BYTE(radius);  // radius*10
        WRITE_BYTE(255);     // R
        WRITE_BYTE(180);     // G
        WRITE_BYTE(100);     // B
        WRITE_BYTE(life);    // life*0.1s
        WRITE_BYTE(0);       // decay
    MESSAGE_END();
}

// HACKZAO
// Evita que o som fique tocando apos a morte do NPC tocando outro som no mesmo canal 'CHAN_VOICE'
void CButano::Killed(entvars_t* pevAttacker, int iGib)
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_LONG(0, ARRAYSIZE(pPainSounds) - 1)], 1.0, ATTN_NORM, 0, 100);

	// Roda o codigo da classe herdada
	CZombie::Killed(pevAttacker, iGib);

    // Fogo que ilumina por alguns segundos
    CreateButaneFire(Center());
}

// BUG util! - Uma explosao de magnitude que nao mate o NPC faz ela repetir por algum motivo!!
void CButano::ButaneExplosion(int dmg, int magn)
{
	// Ssaporra cria um raio de dano que explode os gib tudo seloco cachuera
	RadiusDamage(pev->origin, pev, pev, dmg, CLASS_ALIEN_MONSTER, DMG_BLAST);

	// Cria a explosao, precisa importar o CEnvExplosion
	ExplosionCreate(Center(), pev->angles, ENT(pev), magn, true);

    // Fogo que ilumina por alguns segundos
    CreateButaneFire(Center());
}

void CButano::CabulosoAttack(void)
{
	// Cria um tracer para detectar se acertou ou nao o jogador
	// Esse tracer ja faz um dano, porem colocamos 0 para so detectar.
	CBaseEntity *pHurt = CheckTraceHullAttack(70, 0, DMG_BLAST);

	// Se o trace atingiu o jogador
	if (pHurt)
	{
		// Explode o NPC e tudo ao redor do raio.
		ButaneExplosion(200, 50);

		// Toca o som do Gas e Explode
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackHitSounds) - 1)], 1.0, ATTN_NORM, 0, 100);
	}
	else
	{
		// Senao, toca o som que errou
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0, 100);
	}
}

// Precisa dessa funcao para manusear os ataques do zombie
// Ela da override nos sons de ataque vindos na array pAttackSounds, pAttackHitSounds e pAttackMissSounds
void CButano::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	// Vazou Butano demais, ta na hora de Explodir!
	if (gpGlobals->time >= m_flTimeToExplode)
	{
		ButaneExplosion(50, 50);

		if (pev->health > 0) {
			pev->health = -1;
			Killed(pev, 0);
		}

		return;
	}

	// Todos os ataques levam a explosao
	switch (pEvent->event)
	{
		case ZOMBIE_AE_ATTACK_RIGHT:
			CabulosoAttack();
			break;

		case ZOMBIE_AE_ATTACK_LEFT:
			CabulosoAttack();
			break;

		case ZOMBIE_AE_ATTACK_BOTH:
			CabulosoAttack();
			break;

		default:
			CBaseMonster::HandleAnimEvent(pEvent);
			break;
	}
}

//Override nas funcoes de som sem usar o pitch porque usaremos a musica do gas.
void CButano::PainSound(void)
{
	// Emite o som de Dor sem efeito de pitch
	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pPainSounds[RANDOM_LONG(0, ARRAYSIZE(pPainSounds) - 1)], 1.0, ATTN_NORM, 0, 100);
}

void CButano::AlertSound(void)
{
	// Emite o som de Alerta sem efeito de pitch
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAlertSounds[RANDOM_LONG(0, ARRAYSIZE(pAlertSounds) - 1)], 0.6, ATTN_NORM, 0, 100);
}

void CButano::IdleSound(void)
{
	// Emite o som de Idle sem efeito de pitch
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pIdleSounds[RANDOM_LONG(0, ARRAYSIZE(pIdleSounds) - 1)], 1.0, ATTN_NORM, 0, 100);
}

void CButano::AttackSound(void)
{
	// Som de ataque sem efeito de pitch
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackSounds) - 1)], 1.0, ATTN_NORM, 0, 100);
}

void CButano::UpdateIdleNoMoveExplosion(void)
{
    // Do not do anything if already dead
    if(!IsAlive())
        return;

    float now = gpGlobals->time;

    // If origin changed since last check, consider it moving
    if(pev->origin.x != m_vecLastPos.x ||
       pev->origin.y != m_vecLastPos.y ||
       pev->origin.z != m_vecLastPos.z)
    {
        m_flLastMoveTime = now;
        m_vecLastPos = pev->origin;
        return;
    }

    // Time standing still at exactly the same origin
    float idleTime = now - m_flLastMoveTime;

    if(idleTime >= 3.0f)
    {
        // Prevent multiple explosions from this path
        m_flLastMoveTime = now + 9999.0f;

		// Tchau
        ButaneExplosion(25, 50);

		if (pev->health > 0) {
			pev->health = -1;
			Killed(pev, 0);
		}
    }
}

void CButano::PrescheduleThink(void)
{
    // Handle idle-explosion logic
    UpdateIdleNoMoveExplosion();

    // Keep default zombie behavior
    CZombie::PrescheduleThink();
}