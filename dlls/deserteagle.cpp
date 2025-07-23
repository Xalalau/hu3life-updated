// ##############################
// HU3-LIFE port da desert eagle
// Virou Cornus
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
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"

// Arma Touros quebrada voadora e tiro secundario, codigos adaptados de:
// http://web.archive.org/web/20020717063241/http://lambda.bubblemod.org/tuts/crowbar/
// Para balancar a tela:
#include "shake.h"
// Imprimir mensagens
#ifdef CLIENT_DLL
#include "hud.h"
#endif

LINK_ENTITY_TO_CLASS(weapon_eagle, CDesertEagle);

void CDesertEagle::Spawn()
{
	pev->classname = MAKE_STRING("weapon_eagle"); // hack to allow for old names
	Precache();
	m_iId = WEAPON_DESERT_EAGLE;
	SET_MODEL(ENT(pev), "models/w_desert_eagle.mdl");

	m_iDefaultAmmo = DESERT_EAGLE_DEFAULT_GIVE;

	// Tempo ate processar a nova chance da arma atirar sozinha
	m_nextbadshootchance = gpGlobals->time + 6;
	// Nos nao podemos imprimir mensagens assim que o jogo comeca
	m_waitforthegametobeready = gpGlobals->time + 12;
	// Arma travada
	m_jammedweapon = false;
	// Qualidade da arma
	m_quality = 0;
	// Porcentagem extra de problemas
	m_qualitypercentageeffect = 0;
	// Diz se estamos aguardando um tiro
	m_hasDelayedShot = false;
	// Indica se precisamos informar a qualidade da arma
	m_tellQuality = true;
#ifdef CLIENT_DLL
	// Server -> client: Comando para copiarmos valores de qualidade inicial
	if (gEngfuncs.pfnGetCvarFloat("hu3_touros_qualidade_inicial") == 0)
		hu3_touros_qualidade_inicial = gEngfuncs.pfnRegisterVariable("hu3_touros_qualidade_inicial", "0", 0);
#endif

	FallInit();
}

void CDesertEagle::Precache()
{
	PRECACHE_MODEL("models/v_desert_eagle.mdl");
	PRECACHE_MODEL("models/w_desert_eagle.mdl");
	PRECACHE_MODEL("models/p_desert_eagle.mdl");
	m_iShell = PRECACHE_MODEL("models/shell.mdl");
	PRECACHE_SOUND("weapons/desert_eagle_fire.wav");
	PRECACHE_SOUND("weapons/desert_eagle_reload.wav");
	PRECACHE_SOUND("weapons/desert_eagle_sight.wav");
	PRECACHE_SOUND("weapons/desert_eagle_sight2.wav");
	m_usFireEagle = PRECACHE_EVENT(1, "events/eagle.sc");
	// Arma Touros quebrada voadora, codigo adaptado de:
	// http://web.archive.org/web/20020717063241/http://lambda.bubblemod.org/tuts/crowbar/
	UTIL_PrecacheOther("flying_touros");
	PRECACHE_SOUND("weapons/desert_eagle_fuck.wav");
}

bool CDesertEagle::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = DEAGLE_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iId = WEAPON_DESERT_EAGLE;
	p->iFlags = 0;
	p->iWeight = DEAGLE_WEIGHT;

	return true;
}

bool CDesertEagle::Deploy()
{
	return DefaultDeploy(
		"models/v_desert_eagle.mdl", "models/p_desert_eagle.mdl",
		DEAGLE_DRAW,
		"onehanded");
}

void CDesertEagle::Holster()
{
	SendWeaponAnim(DEAGLE_HOLSTER);
}

void CDesertEagle::WeaponIdle()
{
	ResetEmptySound();

	// Gato para dar um tiro atrasado
	DelayedShot();

	// Entre 4 e 7 segundos tem entre 3% e 13% de chance da arma atirar sozinha (leva 12 segundos para comecar a rodar inicialmente)
	// OBS: esse codigo nao funciona 100%!! A sincronia de balas nao eh fiel quando eu chamo PrimaryAttack(); por aqui.
	// Mas como no final tudo se acerto no jogo, eu preferi deixar esse efeito, porque ele é muito interessante. So reduzi a chance dele.
	if (m_nextbadshootchance <= gpGlobals->time && m_waitforthegametobeready <= gpGlobals->time)
	{
		m_nextbadshootchance = gpGlobals->time + UTIL_SharedRandomLong(m_pPlayer->random_seed, 4, 7);

		if (UTIL_SharedRandomLong(m_pPlayer->random_seed, 0, 99) >= (97 - 10 * m_qualitypercentageeffect) && !m_jammedweapon && m_iClip != 0)
		{
#ifndef CLIENT_DLL
			UTIL_Sparks(m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 22 + gpGlobals->v_right * 10);
			UTIL_SayText("Sua arma Cornus atirou sozinha ou balas sairam voando!|r", m_pPlayer); //Nota: "balas sairam voando" nao deveria acontecer.

			// 30% de chance de levar dano de 1 a 5 por estilhacos
			ShrapnelDamage(30, 1, 5);
#endif
			InstantShot(true);

			return;
		}
	}

	if (m_flTimeWeaponIdle <= UTIL_WeaponTimeBase() && m_iClip)
	{
		const float flNextIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0, 1.0);

		int iAnim;

		if (flNextIdle <= 0.3)
		{
			iAnim = DEAGLE_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flNextIdle * 5 + 3.2;
		}
		else if (flNextIdle > 0.6)
		{
			iAnim = DEAGLE_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flNextIdle * 5 + 3.2;
		}
		else
		{
			iAnim = DEAGLE_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flNextIdle * 5 + 5.6;
		}

		SendWeaponAnim(iAnim);
	}
}

// De 3% a 17% de chance da arma travar e nao atirar mais
bool CDesertEagle::RandomlyJammed()
{
	bool print = false;

	if (m_jammedweapon)
		print = true;
	else if (UTIL_SharedRandomLong(m_pPlayer->random_seed, 0, 99) >= (97 - 14 * m_qualitypercentageeffect))
	{
		m_jammedweapon = true;
		print = true;
	}

	if (print)
	{
#ifndef CLIENT_DLL
		UTIL_SayText("Sua arma Cornus travou! Recarregue-a!|r", m_pPlayer);
#endif
		return true;
	}

	return false;
}

// De 1% a 9% de chance da arma quebrar e sair voando
bool CDesertEagle::RandomlyBreak()
{
	if (UTIL_SharedRandomLong(m_pPlayer->random_seed, 0, 99) >= (99 - 8 * m_qualitypercentageeffect))
	{
#ifndef CLIENT_DLL
		UTIL_SayText("Sua arma Cornus simplesmente quebrou!|r", m_pPlayer);
#endif
		ThrowWeapon(false);

		return true;
	}

	return false;
}

// De 1% a 8% de chance da arma perder toda a municao
bool CDesertEagle::RandomlyLostAllAmmo()
{
	if (UTIL_SharedRandomLong(m_pPlayer->random_seed, 1, 100) <= 1 + 7 * m_qualitypercentageeffect)
	{
		int i;

		// Mensagem
#ifndef CLIENT_DLL
		UTIL_SayText("Infelizmente a sua arma Cornus estragou toda a municao dela...|r", m_pPlayer);
#endif
		// Tomar dano de estilhacos
		ShrapnelDamage(100, 1, 5);

#ifndef CLIENT_DLL
		// Um bando de faisca
		for (i = 0; i <= 5; i++)
			UTIL_Sparks(m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 22 + gpGlobals->v_right * 10);
#endif

		// Tchau municao
		m_iClip = 0;
		m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] = 0;

		return true;
	}
	return false;
}

// Dano por estilhaco
void CDesertEagle::ShrapnelDamage(int chance, int min_damage, int max_damage)
{
#ifndef CLIENT_DLL
	if (RANDOM_LONG(1, 100) <= chance)
	{
		UTIL_SayText("Voce foi ferido por estilhacos de bala da sua arma Cornus...|r", m_pPlayer);

		float damage = RANDOM_LONG(min_damage, max_damage);
		TraceResult tr = UTIL_GetGlobalTrace();
        ClearMultiDamage();
		m_pPlayer->TraceAttack(pev, damage, pev->velocity.Normalize(), &tr, DMG_BLAST);
		ApplyMultiDamage(pev, m_pPlayer->pev);

		UTIL_ScreenFade(m_pPlayer, Vector(255, 0, 0), 0.2, 0.1, 128, FFADE_IN);
	}
#endif
}

// Definir a qualidade da arma
void CDesertEagle::SetQuality()
{
	// Se nao houver a qualidade definida, precisamos de uma
	if (m_tellQuality)
	{
		m_tellQuality = false;

#ifdef CLIENT_DLL
		// Vejo se ha um valor de qualidade a ser sincronizado
		int cvar_quality = gEngfuncs.pfnGetCvarFloat("hu3_touros_qualidade_inicial");

		if (cvar_quality != 0 && cvar_quality != m_quality)
		{
			m_quality = gEngfuncs.pfnGetCvarFloat("hu3_touros_qualidade_inicial");
			gEngfuncs.pfnClientCmd("hu3_touros_qualidade_inicial 0");
		}
		else
		{
			m_quality = UTIL_SharedRandomLong(m_pPlayer->random_seed, 1, 9);
		}
#endif

#ifndef CLIENT_DLL
		if (m_quality == 0)
			m_quality = UTIL_SharedRandomLong(m_pPlayer->random_seed, 1, 9);

		// Mostro a qualidade da arma para o jogador
		char mensagem_i[70] = "Qualidade da sua Cornus: ";
		char mensagem_m[15];
		snprintf(mensagem_m, 9, "%d/10", m_quality);
		char mensagem_f[5] = "|g";
		strcat(mensagem_i, strcat(mensagem_m, mensagem_f));
		UTIL_SayText(mensagem_i, m_pPlayer);
#endif
	}

	// Cada defeito da arma tem um bonus que e adicionado de 0 até 100% dependendo dessa qualidade 9 ate 1;
	if (m_qualitypercentageeffect == 0)
		m_qualitypercentageeffect = 1.0 - (m_quality - 1 + (m_quality - 1) * 1 / 8) * 1.0 / 9;
}

void CDesertEagle::PrimaryAttack()
{
	// Descubro a qualidade da arma tentando usá-la
	// Eu roubo esse espaço porque é chamado no cliete e no servidor.
	SetQuality();

	/*
	// A melhor parte dessa arma é que o UTIL_SharedRandomFloat não produz valores confiáveis.
#ifndef CLIENT_DLL
	ALERT(at_console, "SERVER\n");
#else
	ALERT(at_console, "CLIENT\n");
#endif
	ALERT(at_console, "------------- QUALIDADE %d\n", m_quality);
	ALERT(at_console, "------------- BALAS: %d\n", m_iClip);
	*/

	// Chance de quebrar
	if (!m_jammedweapon)
		if (RandomlyBreak())
			return;

	if (m_pPlayer->pev->waterlevel == 3) // Head
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
		return;
	}

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0;
		}
		else
		{
			Reload();
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	// Entre 2% e 10% de chance de levar dano de 1 a 5 por estilhacos (caso a arma nao esteja travada)
	if (!m_jammedweapon)
		ShrapnelDamage(2 + 8 * m_qualitypercentageeffect, 1, 5);
	// Nao fazer nada caso a arma esteja travada
	else
		return;

	// Chance de travar a arma
	if (RandomlyJammed())
	{
		if (m_iClip > 0)
			--m_iClip;

		SendWeaponAnim(DEAGLE_SHOOT);
	}
	// Chance de quebrar a arma
	else if (RandomlyLostAllAmmo())
	{}
	// Evento para atirar atrasado
	else
	{
		SendWeaponAnim(DEAGLE_SWINGING);
		m_hasDelayedShot = true;
		pev->nextthink = gpGlobals->time + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.2, 1.5);
	}

	// Novos tempos
	//m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + (m_bLaserActive ? 0.5 : 0.22);
	//m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10.0, 15.0);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 4.0, 8.0);
}

// Atirar na hora certa
// Nota: essa função deve rodar no server e client, porém SetThink(&CDesertEagle::DelayedShot)
//       só chama o sv. Fiz um gato em WeaponIdle para resolver o meu problema, lá é show.
void CDesertEagle::DelayedShot()
{
	// Se não estamos esperando um tiro, não fazemos nada
	if (!m_hasDelayedShot)
		return;

	if (gpGlobals->time > pev->nextthink) {
		m_hasDelayedShot = false;
		InstantShot(false);
	}
}

// Solta de 1 a 7 tiros por ataque primario e com mira pessima (variando com a qualidade da arma).
void CDesertEagle::InstantShot(bool forceShot)
{
	// Quantidade de balas atiradas de uma vez segundo a qualidade da arma
	int totalBullets = 0;

	if (m_iClip == 0)
	{
		PlayEmptySound();
		return;
	}

	if (forceShot)
	{
		totalBullets = m_iClip;		
	}
	else
	{
		if (m_qualitypercentageeffect <= 0.25)
		{
			totalBullets = 1;
		}
		else if (m_qualitypercentageeffect > 0.25 && m_qualitypercentageeffect <= 0.50)
		{
			totalBullets = UTIL_SharedRandomLong(m_pPlayer->random_seed, 1, 2);
		}
		else if (m_qualitypercentageeffect > 0.50 && m_qualitypercentageeffect <= 0.75)
		{
			totalBullets = UTIL_SharedRandomLong(m_pPlayer->random_seed, 1, 4);
		}
		else if (m_qualitypercentageeffect > 0.75 && m_qualitypercentageeffect <= 1.0)
		{
			totalBullets = UTIL_SharedRandomLong(m_pPlayer->random_seed, 2, 7);
		}

		if (m_iClip < totalBullets)
		{
			totalBullets = m_iClip;
		}
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip -= totalBullets;

	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	
	for (int i = 0; i < totalBullets; i++)
	{
		float flSpread = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.01, 0.91 + 0.49 * m_qualitypercentageeffect);

		Vector vecDir = m_pPlayer->FireBulletsPlayer(
			1, vecSrc, vecAiming, Vector(flSpread, flSpread, flSpread), 8192,
			BULLET_PLAYER_DEAGLE, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed + i);

		PLAYBACK_EVENT_FULL(
			flags, m_pPlayer->edict(), m_usFireEagle, 0.0, g_vecZero, g_vecZero,
			vecDir.x, vecDir.y, 0, 0, (m_iClip == 0) ? 1 : 0, 0);
	}

	if (0 == m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);
}

// Tiro secundario:
// Nota: por alguma razao eu nao consigo chamar essa funcao com o iClip cheio.
void CDesertEagle::SecondaryAttack()
{
	ThrowWeapon(true);
}

// Varejar a arma, adaptado de:
// http://web.archive.org/web/20020717063241/http://lambda.bubblemod.org/tuts/crowbar/
void CDesertEagle::ThrowWeapon(bool isCollectable)
{
	// Retirar a munição da arma
	m_pPlayer->GiveAmmo(m_iClip, "9mm", _9MM_MAX_CARRY);
	m_iClip = 0;

	// Only throw if we were able to detatch from player.
	if (m_pPlayer->RemovePlayerItem(this))
	{
		// Get the origin, direction, and fix the angle of the throw.
		Vector vecSrc = m_pPlayer->GetGunPosition();
		if (isCollectable)
			vecSrc = vecSrc + gpGlobals->v_right * 8 + gpGlobals->v_forward * 16;
		else
			vecSrc = vecSrc + gpGlobals->v_right * 16 + gpGlobals->v_forward * 3 - gpGlobals->v_up * 27;
		Vector vecDir = gpGlobals->v_forward + gpGlobals->v_right * 0.35;
		Vector vecAng = UTIL_VecToAngles(vecDir);
		vecAng.z = vecDir.z - 90;

#ifndef CLIENT_DLL
		// Create a flying Touros.
		CFlyingTouros *pFTouros = (CFlyingTouros *)Create("flying_touros", vecSrc, Vector(0, 0, 0), m_pPlayer->edict());

		// Give the Touros its velocity, angle, and spin. 
		// Lower the gravity a bit, so it flys. 
		pFTouros->pev->velocity = vecDir * 200 + m_pPlayer->pev->velocity;
		pFTouros->pev->angles = vecAng;
		pFTouros->pev->avelocity.x = -500;
		pFTouros->pev->gravity = .9;
		pFTouros->m_pPlayer = m_pPlayer;

		// Definir se o jogador vai poder pegar a arma do chao ou nao
		int mode = 0;
		if (!isCollectable)
			mode = 1;
		pFTouros->SetMode(mode);

		// Salvo a qualidade da arma e a quantidade de balas inicial
		if (isCollectable)
		{
			if (m_quality == 0) // Caso o jogador jogue a arma fora antes de dar o primeiro tiro
				m_quality = RANDOM_LONG(1, 9);

			pFTouros->SaveQuality(m_quality);
		}

		// Do player weapon anim and sound effect. 
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "weapons/cbar_miss1.wav", 1, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 0xF));
#endif

		if (isCollectable)
		{
			// Mensagem
#ifndef CLIENT_DLL
			UTIL_SayText("Voce jogou sua arma Cornus fora!|g", m_pPlayer);
#endif
			// Destroy this weapon
			DestroyItem();
		}
	}
}

void CDesertEagle::Reload()
{
	// Se a arma estiver travada, liberar
	if (m_jammedweapon)
		m_jammedweapon = false;

	if (m_iClip == SHOTGUN_MAX_CLIP)
		return;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase() || m_quality == 0)
		return;

	SendWeaponAnim(m_iClip > 0 ? DEAGLE_RELOAD_NOSHOT : DEAGLE_SHOOT_EMPTY);
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 3.7;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.7;
	m_flNextPrimaryAttack = GetNextAttackDelay(3.7);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.7;

	int j = V_min(DEAGLE_MAX_CLIP - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
	m_iClip += j;
	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
}