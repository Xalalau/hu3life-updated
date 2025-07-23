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
#include "soundent.h"
#include "shake.h"
#include "gamerules.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(weapon_gauss, CGauss);

float CGauss::GetFullChargeTime()
{
#ifdef CLIENT_DLL
	if (bIsMultiplayer())
#else
	if (g_pGameRules->IsMultiplayer())
#endif
	{
		return 1.5;
	}

	return 4;
}

#ifdef CLIENT_DLL
extern bool g_irunninggausspred;
#endif

void CGauss::Spawn()
{
	Precache();
	m_iId = WEAPON_GAUSS;
	SET_MODEL(ENT(pev), "models/w_gauss.mdl");

	m_iDefaultAmmo = GAUSS_DEFAULT_GIVE;

	FallInit(); // get ready to fall down.

	// ############ hu3lifezado ############ //
	// Anel verde supremo comeca com tamanho 0
	m_iSpriteTextureRange = 0;
	// ############ //
}


void CGauss::Precache()
{
	PRECACHE_MODEL("models/w_gauss.mdl");
	PRECACHE_MODEL("models/v_gauss.mdl");
	PRECACHE_MODEL("models/p_gauss.mdl");

	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/gauss2.wav");
	PRECACHE_SOUND("weapons/electro4.wav");
	PRECACHE_SOUND("weapons/electro5.wav");
	PRECACHE_SOUND("weapons/electro6.wav");
	PRECACHE_SOUND("ambience/pulsemachine.wav");

	m_iGlow = PRECACHE_MODEL("sprites/hotglow.spr");
	m_iBalls = PRECACHE_MODEL("sprites/hotglow.spr");
	m_iBeam = PRECACHE_MODEL("sprites/smoke.spr");

	m_usGaussFire = PRECACHE_EVENT(1, "events/gauss.sc");
	m_usGaussSpin = PRECACHE_EVENT(1, "events/gaussspin.sc");

	// ############ hu3lifezado ############ //
	// Novos sons
	PRECACHE_SOUND("weapons/deploy_cajado.wav");
	PRECACHE_SOUND("weapons/gausscharging.wav");
	PRECACHE_SOUND("weapons/gauss_brasil.wav");
	// Textura dos aneis
	m_iSpriteTexture = PRECACHE_MODEL("sprites/shockwave.spr");
	// ############ //
}

bool CGauss::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "uranium";
	p->iMaxAmmo1 = URANIUM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_GAUSS;
	p->iFlags = 0;
	p->iWeight = GAUSS_WEIGHT;

	return true;
}

bool CGauss::Deploy()
{
	// ############ hu3lifezado ############ //
	// Um belo novo som para quando puxamos a arma.
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/deploy_cajado.wav", RANDOM_FLOAT(0.7, 0.8), ATTN_NORM);
	// ############ //

	m_pPlayer->m_flPlayAftershock = 0.0;
	return DefaultDeploy("models/v_gauss.mdl", "models/p_gauss.mdl", GAUSS_DRAW, "gauss");
}

// ############ hu3lifezado ############ //
// [Terceira Pessoa]
// Chamada do ponto de mira da terceira pessoa
void CGauss::ItemPreFrame()
{
#ifndef CLIENT_DLL
	if (m_pPlayer->hu3_cam_crosshair == 0)
	{
		if (m_pLaser)
		{
			m_pLaser->RemoveSpot(m_pLaser);
			m_pLaser = nullptr;
		}
	}
	else
	{
		if (!m_pLaser)
			m_pLaser = CHu3XSpot::CreateSpot();

		m_pLaser->UpdateSpot(m_pPlayer);
	}
#endif	
}
// ############ //

void CGauss::Holster()
{

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Remocao da mira em terceira pessoa
#ifndef CLIENT_DLL
	if (m_pLaser)
	{
		m_pLaser->RemoveSpot(m_pLaser);
		m_pLaser = nullptr;
	}
#endif
	// ############ //

	SendStopEvent(true);

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim(GAUSS_HOLSTER);
	m_fInAttack = 0;
}


void CGauss::PrimaryAttack()
{
	// ############ hu3lifezado ############ //
	// Anel e liberado
	m_iSpriteTextureRange = 130;
	SonicAttack(2);
	m_iSpriteTextureRange = 0;

	// Funciona embaixo d'agua sim, amiguinho!
	/*
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextSecondaryAttack = m_flNextPrimaryAttack = GetNextAttackDelay(0.15);
		return;
	}
	*/

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Remocao da mira em terceira pessoa
#ifndef CLIENT_DLL
	if (m_pLaser)
	{
		m_pLaser->RemoveSpot(m_pLaser);
		m_pLaser = nullptr;
	}
#endif
	// ############ //

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 2)
	{
		PlayEmptySound();
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
		return;
	}

	m_pPlayer->m_iWeaponVolume = GAUSS_PRIMARY_FIRE_VOLUME;
	m_fPrimaryFire = true;

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 2;

	StartFire();
	m_fInAttack = 0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
	// ############ hu3lifezado ############ //
	// Delay aumentado (original: 0.2)
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.8;
	// ############ //
}

void CGauss::SecondaryAttack()
{
	// JoshA: Sanitize this so it's not total garbage on level transition
	// and we end up ear blasting the player!
	m_pPlayer->m_flStartCharge = V_min(m_pPlayer->m_flStartCharge, gpGlobals->time);

	// ############ hu3lifezado ############ //
	// Funciona embaixo d'agua sim, amiguinho!
		/*
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		if (m_fInAttack != 0)
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro4.wav", 1.0, ATTN_NORM, 0, 80 + RANDOM_LONG(0, 0x3f));
			//Have to send to the host as well because the client will predict the frame with m_fInAttack == 0
			SendStopEvent(true);
			SendWeaponAnim(GAUSS_IDLE);
			m_fInAttack = 0;
		}
		else
		{
			PlayEmptySound();
		}

		m_flNextSecondaryAttack = m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		return;
	}
	*/
	// ############ //

	if (m_fInAttack == 0)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		{
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/357_cock1.wav", 0.8, ATTN_NORM);
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
			return;
		}

		m_fPrimaryFire = false;

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--; // take one ammo just to start the spin
		m_pPlayer->m_flNextAmmoBurn = UTIL_WeaponTimeBase();

		// spin up
		m_pPlayer->m_iWeaponVolume = GAUSS_PRIMARY_CHARGE_VOLUME;

		SendWeaponAnim(GAUSS_SPINUP);
		m_fInAttack = 1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		m_pPlayer->m_flStartCharge = gpGlobals->time;
		m_pPlayer->m_flAmmoStartCharge = UTIL_WeaponTimeBase() + GetFullChargeTime();

		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usGaussSpin, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, 110, 0, 0, 0);

		m_iSoundState = SND_CHANGE_PITCH;
	}
	else if (m_fInAttack == 1)
	{
		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			SendWeaponAnim(GAUSS_SPIN);
			m_fInAttack = 2;
		}
	}
	else
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
		{
			// during the charging process, eat one bit of ammo every once in a while
			if (UTIL_WeaponTimeBase() >= m_pPlayer->m_flNextAmmoBurn && m_pPlayer->m_flNextAmmoBurn != 1000)
			{
#ifdef CLIENT_DLL
				if (bIsMultiplayer())
#else
				if (g_pGameRules->IsMultiplayer())
#endif
				{
					m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
					// ############ hu3lifezado ############ //
					// Os aneis aumentam de 25 em 25 unidades
					m_iSpriteTextureRange += 25;
					// ############ //
					m_pPlayer->m_flNextAmmoBurn = UTIL_WeaponTimeBase() + 0.1;
				}
				else
				{
					m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
					// ############ hu3lifezado ############ //
					// Os aneis aumentam de 25 em 25 unidades
					m_iSpriteTextureRange += 25;
					// ############ //
					m_pPlayer->m_flNextAmmoBurn = UTIL_WeaponTimeBase() + 0.3;
				}
			}
		}

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		{
			// out of ammo! force the gun to fire
			// ############ hu3lifezado ############ //
			// Desativar isso aqui
			// StartFire();
			// ############ //
			m_fInAttack = 0;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1;
			// ############ hu3lifezado ############ //
			// Quando acaba a municao, solta aneis roxos
			SonicAttack(2);
			// Reseto o tamanho dos aneis para 0
			m_iSpriteTextureRange = 0;
			// Nova finalizacao que desbuga o som de sobrecarga
			Holster();
			SendWeaponAnim( GAUSS_DRAW ); // Original: GAUSS_IDLE
			// ############ //
			return;
		}

		if (UTIL_WeaponTimeBase() >= m_pPlayer->m_flAmmoStartCharge)
		{
			// ############ hu3lifezado ############ //
			// A arma deveria parar de gastar municao, mas ela ja esta OP demais
			// don't eat any more ammo after gun is fully charged.
			//m_pPlayer->m_flNextAmmoBurn = 1000;
			// Aneis supremos de poder
			if (gpGlobals->time - m_pPlayer->m_flStartCharge <= GetFullChargeTime() + 0.1)
			{
				m_iSpriteTextureRange = 20;
			}
			else
			{
				if (m_iSpriteTextureRange <= 200)
					SonicAttack(1);
				else
					SonicAttack(2);
			}
			// ############ //
		}

		int pitch = (gpGlobals->time - m_pPlayer->m_flStartCharge) * (150 / GetFullChargeTime()) + 100;
		if (pitch > 250)
			pitch = 250;

		// ALERT( at_console, "%d %d %d\n", m_fInAttack, m_iSoundState, pitch );

		if (m_iSoundState == 0)
			ALERT(at_console, "sound state %d\n", m_iSoundState);

		// ############ hu3lifezado ############ //
		// Son de charging modificado
		//PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usGaussSpin, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, pitch, 0, (m_iSoundState == SND_CHANGE_PITCH) ? 1 : 0, 0);
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/gausscharging.wav", 1.0, ATTN_NORM, 0, pitch);
		// ############ //
		
		m_iSoundState = SND_CHANGE_PITCH; // hack for going through level transitions

		m_pPlayer->m_iWeaponVolume = GAUSS_PRIMARY_CHARGE_VOLUME;

		// m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1;
		// ############ hu3lifezado ############ //
		// Toco o som de dor aqui (meio que um hack...)
		if (m_pPlayer->m_flStartCharge < gpGlobals->time - 9.75)
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_VOICE, "weapons/gauss_brasil.wav", 1.0, ATTN_NORM, 0, 80 + RANDOM_LONG(0, 0x3f));
		// ############ //
		if (m_pPlayer->m_flStartCharge < gpGlobals->time - 10)
		{
			// Player charged up too long. Zap him.
			// ############ hu3lifezado ############ //
			// Sons de overcharge modificados
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro4.wav", 1.0, ATTN_NORM, 0, 80 + RANDOM_LONG(0, 0x3f));
			//EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/electro6.wav", 1.0, ATTN_NORM, 0, 75 + RANDOM_LONG(0, 0x3f));
			// ############ //

			m_fInAttack = 0;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;

			SendStopEvent(false);

#ifndef CLIENT_DLL
			m_pPlayer->TakeDamage(CWorld::World->pev, CWorld::World->pev, 50, DMG_SHOCK);
			// ############ hu3lifezado ############ //
			// Cor de dor por dano de excesso de carga alterada (original: 255,128,0)
			UTIL_ScreenFade(m_pPlayer, Vector(255, 0, 0), 2, 0.5, 128, FFADE_IN);
			// ############ //
#endif
			// ############ hu3lifezado ############ //
			// Nova finalizacao que desbuga o som de sobrecarga
			Holster();
			SendWeaponAnim(GAUSS_DRAW); // Original: GAUSS_IDLE
			// ############ //

			// Player may have been killed and this weapon dropped, don't execute any more code after this!
			return;
		}
	}
}

//=========================================================
// StartFire- since all of this code has to run and then
// call Fire(), it was easier at this point to rip it out
// of weaponidle() and make its own function then to try to
// merge this into Fire(), which has some identical variable names
//=========================================================
void CGauss::StartFire()
{
	float flDamage;

	// JoshA: Sanitize this so it's not total garbage on level transition
	// and we end up ear blasting the player!
	m_pPlayer->m_flStartCharge = V_min(m_pPlayer->m_flStartCharge, gpGlobals->time);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecSrc = m_pPlayer->GetGunPosition(); // + gpGlobals->v_up * -8 + gpGlobals->v_right * 8;

	if (gpGlobals->time - m_pPlayer->m_flStartCharge > GetFullChargeTime())
	{
		flDamage = 200;
	}
	else
	{
		flDamage = 200 * ((gpGlobals->time - m_pPlayer->m_flStartCharge) / GetFullChargeTime());
	}

	if (m_fPrimaryFire)
	{
		// fixed damage on primary attack
#ifdef CLIENT_DLL
		// ############ hu3lifezado ############ //
		// Dano aumentado (20)
		flDamage = 120;
		// ############ //
#else
		flDamage = gSkillData.plrDmgGauss;
#endif
	}

	if (m_fInAttack != 3)
	{
		//ALERT ( at_console, "Time:%f Damage:%f\n", gpGlobals->time - m_pPlayer->m_flStartCharge, flDamage );

#ifndef CLIENT_DLL
		float flZVel = m_pPlayer->pev->velocity.z;

		if (!m_fPrimaryFire)
		{
			m_pPlayer->pev->velocity = m_pPlayer->pev->velocity - gpGlobals->v_forward * flDamage * 5;
		}

		if (!g_pGameRules->IsMultiplayer())

		{
			// in deathmatch, gauss can pop you up into the air. Not in single play.
			m_pPlayer->pev->velocity.z = flZVel;
		}
#endif
		// player "shoot" animation
		// ############ hu3lifezado ############ //
		// Ajeito a animacao de ataque
		//m_pPlayer->SetAnimation( PLAYER_ATTACK1 ); // Original
		switch (RANDOM_LONG(0, 1))
		{
		case 0:	SendWeaponAnim(GAUSS_FIRE_HU3_FIX); break;
		case 1: SendWeaponAnim(GAUSS_FIRE); break;
		}
		// ############ //
	}

	// time until aftershock 'static discharge' sound
	m_pPlayer->m_flPlayAftershock = gpGlobals->time + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.3, 0.8);

	Fire(vecSrc, vecAiming, flDamage);
}

void CGauss::Fire(Vector vecOrigSrc, Vector vecDir, float flDamage)
{
	m_pPlayer->m_iWeaponVolume = GAUSS_PRIMARY_FIRE_VOLUME;

	Vector vecSrc = vecOrigSrc;
	Vector vecDest = vecSrc + vecDir * 8192;
	edict_t* pentIgnore;
	TraceResult tr, beam_tr;
	float flMaxFrac = 1.0;
	int nTotal = 0;
	bool fHasPunched = false;
	bool fFirstBeam = true;
	// ############ hu3lifezado ############ //
	// limite de rebatidas aumentado de 10 para 100
	int	nMaxHits = 100;
	// ############ //

	pentIgnore = m_pPlayer->edict();

#ifdef CLIENT_DLL
	if (m_fPrimaryFire == false)
		g_irunninggausspred = true;
#endif

	// The main firing event is sent unreliably so it won't be delayed.
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usGaussFire, 0.0, m_pPlayer->pev->origin, m_pPlayer->pev->angles, flDamage, 0.0, 0, 0, m_fPrimaryFire ? 1 : 0, 0);

	SendStopEvent(false);


	/*ALERT( at_console, "%f %f %f\n%f %f %f\n", 
		vecSrc.x, vecSrc.y, vecSrc.z, 
		vecDest.x, vecDest.y, vecDest.z );*/


	//	ALERT( at_console, "%f %f\n", tr.flFraction, flMaxFrac );

#ifndef CLIENT_DLL
	while (flDamage > 10 && nMaxHits > 0)
	{
		nMaxHits--;

		// ALERT( at_console, "." );
		UTIL_TraceLine(vecSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr);

		if (0 != tr.fAllSolid)
			break;

		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		if (pEntity == NULL)
			break;

		if (fFirstBeam)
		{
			m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
			fFirstBeam = false;

			nTotal += 26;
		}

		if (0 != pEntity->pev->takedamage)
		{
			ClearMultiDamage();

			// ############ hu3lifezado ############ //
			// Somei 30 ao flDamage
			pEntity->TraceAttack(m_pPlayer->pev, flDamage + 30, vecDir, &tr, DMG_BULLET);
			// Explodo o bastardo que acertamos!
			MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pEntity->pev->origin );
				WRITE_BYTE( TE_EXPLOSION);
				WRITE_COORD( pEntity->pev->origin.x );
				WRITE_COORD( pEntity->pev->origin.y );
				WRITE_COORD( pEntity->pev->origin.z );
				WRITE_SHORT( g_sModelIndexFireball );
				WRITE_BYTE( 10 ); // scale * 10
				WRITE_BYTE( 15  ); // framerate
				WRITE_BYTE( TE_EXPLFLAG_NONE );
			MESSAGE_END();
			// ############ //

			// if you hurt yourself clear the headshot bit
			if (m_pPlayer->pev == pEntity->pev)
			{
				tr.iHitgroup = 0;
			}

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
		}

		if (pEntity->ReflectGauss())
		{
			float n;

			pentIgnore = NULL;

			n = -DotProduct(tr.vecPlaneNormal, vecDir);

			if (n < 0.5) // 60 degrees
			{
				// ALERT( at_console, "reflect %f\n", n );
				// reflect
				Vector r;

				r = 2.0 * tr.vecPlaneNormal * n + vecDir;
				flMaxFrac = flMaxFrac - tr.flFraction;
				vecDir = r;
				vecSrc = tr.vecEndPos + vecDir * 8;
				vecDest = vecSrc + vecDir * 8192;

				// explode a bit
				m_pPlayer->RadiusDamage(tr.vecEndPos, pev, m_pPlayer->pev, flDamage * n, CLASS_NONE, DMG_BLAST);

				nTotal += 34;

				// lose energy
				if (n == 0)
					n = 0.1;
				flDamage = flDamage * (1 - n);
			}
			else
			{
				nTotal += 13;

				// limit it to one hole punch
				if (fHasPunched)
					break;
				fHasPunched = true;

				// try punching through wall if secondary attack (primary is incapable of breaking through)
				if (!m_fPrimaryFire)
				{
					UTIL_TraceLine(tr.vecEndPos + vecDir * 8, vecDest, dont_ignore_monsters, pentIgnore, &beam_tr);
					if (0 == beam_tr.fAllSolid)
					{
						// trace backwards to find exit point
						UTIL_TraceLine(beam_tr.vecEndPos, tr.vecEndPos, dont_ignore_monsters, pentIgnore, &beam_tr);

						float n = (beam_tr.vecEndPos - tr.vecEndPos).Length();

						if (n < flDamage)
						{
							if (n == 0)
								n = 1;
							flDamage -= n;

							// ALERT( at_console, "punch %f\n", n );
							nTotal += 21;

							// exit blast damage
							//m_pPlayer->RadiusDamage( beam_tr.vecEndPos + vecDir * 8, pev, m_pPlayer->pev, flDamage, CLASS_NONE, DMG_BLAST );
							float damage_radius;


							if (g_pGameRules->IsMultiplayer())
							{
								damage_radius = flDamage * 1.75; // Old code == 2.5
							}
							else
							{
								damage_radius = flDamage * 2.5;
							}

							::RadiusDamage(beam_tr.vecEndPos + vecDir * 8, pev, m_pPlayer->pev, flDamage, damage_radius, CLASS_NONE, DMG_BLAST);

							CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0);

							nTotal += 53;

							vecSrc = beam_tr.vecEndPos + vecDir;
						}
					}
					else
					{
						//ALERT( at_console, "blocked %f\n", n );
						flDamage = 0;
					}
				}
				else
				{
					//ALERT( at_console, "blocked solid\n" );

					flDamage = 0;
				}
			}
		}
		else
		{
			vecSrc = tr.vecEndPos + vecDir;
			pentIgnore = ENT(pEntity->pev);
		}
	}
#endif
	// ALERT( at_console, "%d bytes\n", nTotal );
}




void CGauss::WeaponIdle()
{
	ResetEmptySound();

	// play aftershock static discharge
	if (0 != m_pPlayer->m_flPlayAftershock && m_pPlayer->m_flPlayAftershock < gpGlobals->time)
	{
		switch (RANDOM_LONG(0, 3))
		{
		case 0:
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro4.wav", RANDOM_FLOAT(0.7, 0.8), ATTN_NORM);
			break;
		case 1:
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro5.wav", RANDOM_FLOAT(0.7, 0.8), ATTN_NORM);
			break;
		case 2:
			EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/electro6.wav", RANDOM_FLOAT(0.7, 0.8), ATTN_NORM);
			break;
		case 3:
			break; // no sound
		}
		m_pPlayer->m_flPlayAftershock = 0.0;
	}

	// ############ hu3lifezado ############ //
	// Troquei a verificacao para o anel funcionar durante as cargas tambem
	//if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
	//	return;
	if (m_iSpriteTextureRange == 0)
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;
	// ############ //

	// ############ hu3lifezado ############ //
	// Verifico se m_iSpriteTextureRange nao e 0 tambem
	if ((m_fInAttack != 0) || (m_iSpriteTextureRange != 0))
	{
		// Solta aneis roxos ou vermelhos dependendo da situacao
		if (gpGlobals->time - m_pPlayer->m_flStartCharge >= GetFullChargeTime())
		{
			SendWeaponAnim(GAUSS_FIRE2);
			if (m_pPlayer->m_flStartCharge < gpGlobals->time - 10)
			{
				m_iSpriteTextureRange = 800;
				SonicAttack(3);
				SonicAttack(3);
				SonicAttack(3);
				SonicAttack(3);
				SonicAttack(3);
				SonicAttack(3);
				SonicAttack(3);
				SonicAttack(3);
				SonicAttack(3);
				SonicAttack(3);
			}
			else
				m_iSpriteTextureRange = 450;
			SonicAttack(3);
		}
		else
			SonicAttack(2);
		m_iSpriteTextureRange = 0;
		// ############ //

		StartFire();
		m_fInAttack = 0;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
	}
	else
	{
		int iAnim;
		float flRand = RANDOM_FLOAT(0, 1);
		// ############ hu3lifezado ############ //
		// Maior chance de dar dedo do meio agora
		if (flRand <= 0.35) // Origial: 0.5
		{
			iAnim = GAUSS_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
		}
		else if (flRand <= 0.65) // Original 0.75
		{
			iAnim = GAUSS_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
		}
		else
		{
			iAnim = GAUSS_FIDGET;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;
		}
		// ############ //

		return;
		SendWeaponAnim(iAnim);
	}
}

void CGauss::SendStopEvent(bool sendToHost)
{
	// This reliable event is used to stop the spinning sound
	// It's delayed by a fraction of second to make sure it is delayed by 1 frame on the client
	// It's sent reliably anyway, which could lead to other delays

	int flags = FEV_RELIABLE | FEV_GLOBAL;

	if (!sendToHost)
	{
		flags |= FEV_NOTHOST;
	}

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usGaussFire, 0.01, m_pPlayer->pev->origin, m_pPlayer->pev->angles, 0.0, 0.0, 0, 0, 0, 1);
}



class CGaussAmmo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_gaussammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_gaussammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBaseEntity* pOther) override
	{
		if (pOther->GiveAmmo(AMMO_URANIUMBOX_GIVE, "uranium", URANIUM_MAX_CARRY) != -1)
		{
			// ############ hu3lifezado ############ //
			// Às vezes eu quero omitir esse som irritante
			if (pOther->enable_item_pickup_sound)
				EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			// ############ //

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(ammo_gaussclip, CGaussAmmo);

// ##############################
// HU3-LIFE Ataque Secudario
// ##############################
// Roubei e ajustei o ataque do houndeye

//=========================================================
// WriteBeamColor - writes a color vector to the network 
// based on the size of the group. 
//=========================================================
void CGauss::WriteBeamColor(int force)
{
	byte	bRed, bGreen, bBlue;

	switch (force)
	{
	case 1:
		bRed = 10;
		bGreen = 208;
		bBlue = 0;
		break;
	case 2:
		bRed = 60;
		bGreen = 33;
		bBlue = 64;
		break;
	case 3:
		bRed = 255;
		bGreen = 0;
		bBlue = 0;
		break;
	default:
		// Original: 188, 220, 255
		ALERT(at_aiconsole, "Unsupported Houndeye SquadSize!\n");
		bRed = 10;
		bGreen = 208;
		bBlue = 0;
		break;
	}

	WRITE_BYTE(bRed);
	WRITE_BYTE(bGreen);
	WRITE_BYTE(bBlue);
}

//=========================================================
// SonicAttack
//=========================================================
void CGauss::SonicAttack(int force)
{
#ifdef CLIENT_DLL
	return;
#endif

	edict_t *pClient;

	// manually find the single player. 
	pClient = g_engfuncs.pfnPEntityOfEntIndex(1);

	float		flAdjustedDamage;
	float		flDist;

	// blast circles
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE( TE_BEAMCYLINDER );
		WRITE_COORD( pev->origin.x);
		WRITE_COORD( pev->origin.y);
		WRITE_COORD( pev->origin.z);
		WRITE_COORD( pev->origin.x);
		WRITE_COORD( pev->origin.y);
		WRITE_COORD( pev->origin.z + 16 + m_iSpriteTextureRange / .2); // reach damage radius over .3 seconds
		WRITE_SHORT( m_iSpriteTexture );
		WRITE_BYTE( 0 ); // startframe
		WRITE_BYTE( 0 ); // framerate
		WRITE_BYTE( 2 ); // life
		WRITE_BYTE( 16 );  // width
		WRITE_BYTE( 0 );   // noise
		WriteBeamColor( force );
		WRITE_BYTE( 255 ); //brightness
		WRITE_BYTE( 0 );		// speed
	MESSAGE_END();
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_BEAMCYLINDER );
		WRITE_COORD( pev->origin.x);
		WRITE_COORD( pev->origin.y);
		WRITE_COORD( pev->origin.z);
		WRITE_COORD( pev->origin.x);
		WRITE_COORD( pev->origin.y);
		WRITE_COORD( pev->origin.z + 16 + ( m_iSpriteTextureRange / 2 ) / .2); // reach damage radius over .3 seconds
		WRITE_SHORT( m_iSpriteTexture );
		WRITE_BYTE( 0 ); // startframe
		WRITE_BYTE( 0 ); // framerate
		WRITE_BYTE( 2 ); // life
		WRITE_BYTE( 16 );  // width
		WRITE_BYTE( 0 );   // noise
		WriteBeamColor( force );
		
		WRITE_BYTE( 255 ); //brightness
		WRITE_BYTE( 0 );		// speed
	MESSAGE_END();


	CBaseEntity *pEntity = NULL;
	// iterate on all entities in the vicinity.

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, m_iSpriteTextureRange)) != NULL)
	{
		if (pEntity->pev->takedamage != DAMAGE_NO)
		{
			if (!FClassnameIs(pEntity->pev, STRING(pClient->v.classname)))
			{// houndeyes don't hurt other houndeyes with their attack

				// Para nao ser tao OP, dano durante o anel supremo tem que ser calculado bem mais leve do que o de um anel ?nico
				if ( (gpGlobals->time - m_pPlayer->m_flStartCharge >= GetFullChargeTime()) && (m_iSpriteTextureRange != 400))
					flAdjustedDamage =  m_iSpriteTextureRange * 3;
				else
					flAdjustedDamage =  m_iSpriteTextureRange * 10;

				flDist = (pEntity->Center() - pev->origin).Length();

				flAdjustedDamage -= ( flDist / m_iSpriteTextureRange ) * flAdjustedDamage;


				if (!FVisible(pEntity))
				{
					if (pEntity->IsPlayer())
					{
						// if this entity is a client, and is not in full view, inflict half damage. We do this so that players still 
						// take the residual damage if they don't totally leave the houndeye's effective radius. We restrict it to clients
						// so that monsters in other parts of the level don't take the damage and get pissed.
						flAdjustedDamage *= 0.5;
					}
					
					else if ((!FClassnameIs(pEntity->pev, "func_breakable")) && (!FClassnameIs(pEntity->pev, "func_pushable")))
					{
						// do not hurt nonclients through walls, but allow damage to be done to breakables
						flAdjustedDamage = 0;
					}
				}

				//ALERT ( at_aiconsole, "Damage: %f\n", flAdjustedDamage );

				if (flAdjustedDamage > 0)
				{
					pEntity->TakeDamage(pev, pev, flAdjustedDamage, DMG_SONIC | DMG_ALWAYSGIB);
				}
			}
		}
	}
}