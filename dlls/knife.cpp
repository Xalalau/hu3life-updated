// ##############################
// HU3-LIFE port da faca
// ##############################

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
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "skill.h"

#include "gamerules.h"

// ############ hu3lifezado ############ //
// Acesso ao networking
#include "UserMessages.h"
// ############ //

#define KNIFE_BODYHIT_VOLUME 128
#define KNIFE_WALLHIT_VOLUME 512

LINK_ENTITY_TO_CLASS(weapon_knife, CKnife);


// ############ hu3lifezado ############ //
// Pixação
const char *CKnife::pSelectionSounds[] =
{
	"weapons/spray_selection1.wav",
	"weapons/spray_selection2.wav",
	"weapons/spray_selection3.wav",
	"weapons/spray_selection4.wav",
};
// ############ //

void CKnife::Precache()
{
	PRECACHE_MODEL("models/v_knife.mdl");
	PRECACHE_MODEL("models/w_knife.mdl");
	PRECACHE_MODEL("models/p_knife.mdl");

	// ############ hu3lifezado ############ //
	// Alguns audios foram removidos e os seguintes renomeados
	PRECACHE_SOUND("weapons/spray_hit_flesh1.wav");
	PRECACHE_SOUND("weapons/spray_hit_flesh2.wav");
	PRECACHE_SOUND("weapons/spray_hit_wall1.wav");
	PRECACHE_SOUND("weapons/spray_hit_wall2.wav");
	PRECACHE_SOUND("weapons/spray_eupichavasim.wav");
	PRECACHE_SOUND_ARRAY(pSelectionSounds);
	// ############ //

	m_usKnife = PRECACHE_EVENT(1, "events/knife.sc");
}

void CKnife::Spawn()
{
	Precache();

	m_iId = WEAPON_KNIFE; 

	SET_MODEL(edict(), "models/w_knife.mdl");

	m_iClip = WEAPON_NOCLIP;


	// ############ hu3lifezado ############ //
	// Tempo para controlar mudanca de cor no botao secundario do mouse
	m_nextcolorchange = 0;
	// Tempo que quando ultrapassado forca a rechecagem do primeiro acerto
	m_nextfirsthit = 0;
	// Tempo para o proximo som de spray aplicado em parede
	m_nextsprayonwallsound = 0;
	// ############ hu3lifezado ############ //
	// Inicializo a selecao de cores na primeira cor
#ifndef CLIENT_DLL
	for (int i = 0; i <= 64; i++)
		hu3_spray_color[i] = 1;
#else
	hu3_spray_color[1] = 1;
#endif
	// [COOP] Resetar a selecao no HUD
	reset_hud = true;
	// ############ //

	FallInit();
}

bool CKnife::Deploy()
{
	// ############ hu3lifezado ############ //
	// Tempo entre chamadas HandleAnimationAndSound()
#ifndef CLIENT_DLL
	m_nextthink = gpGlobals->time + 0.1;
#endif
	return DefaultDeploy("models/v_knife.mdl", "models/p_knife.mdl", KNIFE_DRAW, "crowbar");
	// ############ //
}

void CKnife::Holster()
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim(KNIFE_HOLSTER);
}

// ############ hu3lifezado ############ //
// Pichar!
// Funcionamentos
// - Tenta colocar decals 40 vezes por segundo;
// - Tenta aplicar danos, animacoes e sons em intervalos minimos de 1 segundo;
// - Impede um novo ataque por 0.35s caso o jogador erre o alvo.
void CKnife::PrimaryAttack()
{
	// Trace generico (obs: o codigo da arma todo pode acessar o resultado desse trace pela variavel "m_trHit")
	bool bDidHit = TraceSomeShit();

	// Cagar cores no mapa
	if (bDidHit)
		PlaceColor();

	// Processamento de dano, animacoes e sons
	if (m_nextthink < pev->nextthink)
	{
		// Aplicamos as animacoes e, se necessario, sons
		HandleAnimationAndSound();

		// Dano
		ApplyDamage();

		if (bDidHit)
			// Se acertou algo os proximos ataques dentro do intervalo de tempo a seguir nao podem ser considerados o primeiro de uma sequencia deles
			m_nextfirsthit = gpGlobals->time + 0.35;
		else
			// Caso contrario eh necessario esperar um tempo ate poder dar o proximo ataque
			m_flNextPrimaryAttack = GetNextAttackDelay(0.35);

		// Tempo entre chamadas do HandleAnimationAndSound()
		m_nextthink = gpGlobals->time + 0.1;
	}

	// Delay caso o player esteja utilizando os decals de fundo ou de assinatura
	int index;
#ifndef CLIENT_DLL
	CBaseEntity *hu3Player = (CBaseEntity *)m_pPlayer;
	index = hu3Player->entindex();
#else
	index = 1;
#endif
	if (hu3_spray_color[index] > 10)
		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);

	// Tempo minimo entre chamadas do PrimaryAttack()
	pev->nextthink = gpGlobals->time + 0.0025; // Tempo para o resto dos casos (rapido!)
}

// Trocar as cores
void CKnife::SecondaryAttack()
{
	// Verifica o tempo para sabermos se eh possivel mudar a cor
	if (m_nextcolorchange > gpGlobals->time)
		return;

	// Pego o index que guarda a selecao de cores
	int index;
#ifndef CLIENT_DLL
	CBaseEntity *hu3Player = (CBaseEntity *)m_pPlayer;
	index = hu3Player->entindex();
#else
	index = 1;
#endif

	// 10 cores + 2 fundos + 1 Carlos Adao no Decals.cpp
	if (hu3_spray_color[index] < 13)
		hu3_spray_color[index] = hu3_spray_color[index] + 1;
	else
		hu3_spray_color[index] = 1;

	// Atualizo a selecao no HUD
#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_ONE, gmsgHu3PicheColors, NULL, m_pPlayer->pev);
	WRITE_BYTE(hu3_spray_color[index]);
	MESSAGE_END();
#endif

	// Som de balancar a latinha
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, RANDOM_SOUND_ARRAY(pSelectionSounds), 1, ATTN_IDLE, 0, PITCH_NORM);

	// Animacao e seu tempo:
	SendWeaponAnim(KNIFE_SELECTION);

	// Faz a mudanca de cor ficar desativada durante tempo da animacao
	m_nextcolorchange = gpGlobals->time + 0.35;
	m_flNextPrimaryAttack = GetNextAttackDelay(0.35);

	// Idle tambem so volta depois desse tempo
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.35;

	return;
}
// ############ //


void CKnife::ApplyDamage()
{
#ifndef CLIENT_DLL
	CBaseEntity *pEntity = CBaseEntity::Instance(m_trHit.pHit);

	if (pEntity)
	{
		ClearMultiDamage();
		if((m_flNextPrimaryAttack + 1 < UTIL_WeaponTimeBase()) || g_pGameRules->IsMultiplayer())
		{
			// first swing does full damage
			pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgKnife, gpGlobals->v_forward, &m_trHit, DMG_CLUB);
		}
		else
		{
			// subsequent swings do half
			pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgKnife / 2, gpGlobals->v_forward, &m_trHit, DMG_CLUB);
		}

		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
	}
#endif
}

void CKnife::HandleAnimationAndSound()
{
	// ############ hu3lifezado ############ //
	// !!! Parte de tracing foi movida para a funcao TraceSomeShit()

	// Caso o ataque em questao seja o primeiro de uma sequencia, devemos rodar a funcao de evento
	if (m_nextfirsthit < gpGlobals->time)
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usKnife, 0.0, g_vecZero, g_vecZero, 0, 0, 0, 0.0, 0, 0.0);

	// Animacoes simplificadas:
	switch (((m_iSwing++) % 2) + 1)
	{
		case 0:
			SendWeaponAnim(KNIFE_ATTACK1); break;
		case 1:
			SendWeaponAnim(KNIFE_ATTACK2HIT); break;
		case 2:
			SendWeaponAnim(KNIFE_ATTACK3HIT); break;
	}

	// Animacao "de tiro" do jogador
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifndef CLIENT_DLL
	// Sons mais abaixo...
	// ############ //
	if (m_trHit.flFraction < 1.0)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance(m_trHit.pHit);

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		bool bHitWorld = true;

		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				// ############ hu3lifezado ############ //
				// Novos nomes de audios
				// play thwack or smack sound
				switch (RANDOM_LONG(0, 1))
				{
				case 0:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/spray_hit_flesh1.wav", 1, ATTN_NORM);
					break;
				case 1:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/spray_hit_flesh2.wav", 1, ATTN_NORM);
					break;
				}
				// ############ //

				m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
				if (!pEntity->IsAlive())
					// ############ hu3lifezado ############ //
					// Nao retorna mais nada
					return;
					// ############ //
				else
					flVol = 0.1;

				bHitWorld = false;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		// ############ hu3lifezado ############ //
		// Novos nomes de audios e adicionado tempo de espera
		if (m_nextsprayonwallsound < gpGlobals->time)
		{
			if (bHitWorld)
			{
				// override the volume here, cause we don't play texture sounds in multiplayer,
				// and fvolbar is going to be 0 from the above call.
				float fvolbar = 1;

				// Novos nomes de audios
				// also play crowbar strike
				switch (RANDOM_LONG(0, 1))
				{
				case 0:
					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/spray_hit_wall1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
					break;
				case 1:
					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/spray_hit_wall2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
					break;
				}

				// Nao tem mais utilidade aqui, comentado...
				// delay the decal a bit
				//m_trHit = tr;
			}

			// Delay ate a proxima reproducao de som. Deve ser o tempo do som menos o atraso de entrada
			// em HandleAnimationAndSound() feito em PrimaryAttack()
			m_nextsprayonwallsound = gpGlobals->time + 0.3;
		}

		m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;

		// Codigo de controle de frames tirado daqui e refeito no PrimaryAttack()
		// ############ //
	}
#endif
}

// ############ hu3lifezado ############ //
// Movi a parte de tracing para ca para ter mais controle e disponibilidade
bool CKnife::TraceSomeShit()
{
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 120;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &m_trHit);

#ifndef CLIENT_DLL
	if (m_trHit.flFraction >= 1.0)
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &m_trHit);
#endif

	if (m_trHit.pHit)
		return true;

	return false;
}

// Posicionar ponto de cor
void CKnife::PlaceColor()
{
	TraceResult * pTrace = &m_trHit;

	if (!pTrace->pHit)
		return;

#ifndef CLIENT_DLL
	// Pega a entidade, a superficie acertada
	CBaseEntity* pHit = CBaseEntity::Instance(pTrace->pHit);

	// A entidade eh valida?
	if (!UTIL_IsValidEntity(pHit->edict()))
		return;

	// A entidade eh mapa ou objeto puxavel?
	if (pHit->pev->solid == SOLID_BSP || pHit->pev->movetype == MOVETYPE_PUSHSTEP)
	{
		// Desenhar decal sobre essa entidade
		// Os decals estao dentro do arquivo decals.wad e sao listados em Decals.cpp
		CBaseEntity *hu3Player = (CBaseEntity *)m_pPlayer;
		UTIL_DecalTrace(pTrace, 41 + hu3_spray_color[hu3Player->entindex()]);
	}
#endif
}

// Animacoes e sons de idle
void CKnife::WeaponIdle()
{
	int iAnim;
	float flRand = RANDOM_FLOAT(0, 1);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	// [COOP] Consertar o icone de cor no HUD depois de um changelevel
	// Isso eh necessario porque eu nao salvo a selecao nesse caso
#ifndef CLIENT_DLL
	if (g_pGameRules->IsCoOp() && reset_hud)
	{
		CBaseEntity *hu3Player = (CBaseEntity *)m_pPlayer;

		MESSAGE_BEGIN(MSG_ONE, gmsgHu3PicheColors, NULL, hu3Player->pev);
			WRITE_BYTE(1);
		MESSAGE_END();
	
		reset_hud = false;
	}
#endif

	// 3% de chance de tocar EU PICHAVA SIM E CURTIA MUITO!
	if (RANDOM_LONG(0, 99) >= 97)
	{
		iAnim = KNIFE_PICHAVASIM;
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/spray_eupichavasim.wav", RANDOM_FLOAT(0.7, 0.8), ATTN_NORM);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 8;

		// Impedir a arma de funcionar por pouco tempo para evitar que o jogador corte bruscamente a animacao
		m_nextcolorchange = gpGlobals->time + 4;
		m_flNextPrimaryAttack = GetNextAttackDelay(2.0);
	}
	// 97% de chance de executar essa parte
	else
	{
		// 15% de chance de olhar o rotulo caso o jogador nao tenha acabado de trocar a cor da arma
		if (flRand <= 0.15 && m_nextcolorchange + 0.35 <= gpGlobals->time)
		{
			iAnim = KNIFE_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
		}
		// 50% de chance para segurar normalmente mas com algum movimento - Tipo 1
		else if (flRand <= 0.50)
		{
			iAnim = KNIFE_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
		}
		// 35% de chance para segurar normalmente mas com algum movimento - Tipo 2
		else
		{
			iAnim = KNIFE_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;
		}
	}

	SendWeaponAnim(iAnim);
}
// ############ //

bool CKnife::GetItemInfo(ItemInfo* p)
{
	p->pszAmmo1 = nullptr;
	p->iMaxAmmo1 = WEAPON_NOCLIP;
	p->pszName = STRING(pev->classname);
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = WEAPON_NOCLIP;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_KNIFE;
	p->iWeight = 0;
	return true;
}