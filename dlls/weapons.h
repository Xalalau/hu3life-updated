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

#pragma once

#include "effects.h"
#include "weaponinfo.h"

class CBasePlayer;
class CBasePlayerWeapon;

void DeactivateSatchels(CBasePlayer* pOwner);

// Contact Grenade / Timed grenade / Satchel Charge
class CGrenade : public CBaseMonster
{
public:
	void Spawn() override;

	typedef enum
	{
		SATCHEL_DETONATE = 0,
		SATCHEL_RELEASE
	} SATCHELCODE;

	static CGrenade* ShootTimed(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, float time);
	static CGrenade* ShootContact(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity);
	static CGrenade* ShootSatchelCharge(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity);
	static void UseSatchelCharges(entvars_t* pevOwner, SATCHELCODE code);

	void Explode(Vector vecSrc, Vector vecAim);
	void Explode(TraceResult* pTrace, int bitsDamageType);
	void EXPORT Smoke();

	void EXPORT BounceTouch(CBaseEntity* pOther);
	void EXPORT SlideTouch(CBaseEntity* pOther);
	void EXPORT ExplodeTouch(CBaseEntity* pOther);
	void EXPORT DangerSoundThink();
	void EXPORT PreDetonate();
	void EXPORT Detonate();
	void EXPORT DetonateUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT TumbleThink();

	virtual void BounceSound();
	int BloodColor() override { return DONT_BLEED; }
	void Killed(entvars_t* pevAttacker, int iGib) override;

	bool m_fRegisteredSound; // whether or not this grenade has issued its DANGER sound to the world sound list yet.
};


// constant items
#define ITEM_HEALTHKIT 1
#define ITEM_ANTIDOTE 2
#define ITEM_SECURITY 3
#define ITEM_BATTERY 4

// ############ hu3lifezado ############ //
// O novo maximo e 666
#define MAX_NORMAL_BATTERY	666
// ############ //


// weapon weight factors (for auto-switching)   (-1 = noswitch)
#define CROWBAR_WEIGHT 0
#define GLOCK_WEIGHT 10
#define PYTHON_WEIGHT 15
#define MP5_WEIGHT 15
#define SHOTGUN_WEIGHT 15
#define CROSSBOW_WEIGHT 10
#define RPG_WEIGHT 20
#define GAUSS_WEIGHT 20
#define EGON_WEIGHT 20
#define HORNETGUN_WEIGHT 15
#define HANDGRENADE_WEIGHT 5
#define SNARK_WEIGHT 5
#define SATCHEL_WEIGHT -10
#define TRIPMINE_WEIGHT -10
// ############ hulifezado ############ //
// Novas armas
#define DEAGLE_WEIGHT 15
#define PIPEWRENCH_WEIGHT 2
// ############ //

// weapon clip/carry ammo capacities
#define URANIUM_MAX_CARRY 100
#define _9MM_MAX_CARRY 250
#define _357_MAX_CARRY 36
#define BUCKSHOT_MAX_CARRY 125
#define BOLT_MAX_CARRY 50
#define ROCKET_MAX_CARRY 5
#define HANDGRENADE_MAX_CARRY 10
#define SATCHEL_MAX_CARRY 5
#define TRIPMINE_MAX_CARRY 5
#define SNARK_MAX_CARRY 15
#define HORNET_MAX_CARRY 8
#define M203_GRENADE_MAX_CARRY 10

// the maximum amount of ammo each weapon's clip can hold
#define WEAPON_NOCLIP -1

//#define CROWBAR_MAX_CLIP		WEAPON_NOCLIP
#define GLOCK_MAX_CLIP 17
#define PYTHON_MAX_CLIP 6
#define MP5_MAX_CLIP 50
#define MP5_DEFAULT_AMMO 25
#define SHOTGUN_MAX_CLIP 8
#define CROSSBOW_MAX_CLIP 5
#define RPG_MAX_CLIP 1
#define GAUSS_MAX_CLIP WEAPON_NOCLIP
#define EGON_MAX_CLIP WEAPON_NOCLIP
#define HORNETGUN_MAX_CLIP WEAPON_NOCLIP
#define HANDGRENADE_MAX_CLIP WEAPON_NOCLIP
#define SATCHEL_MAX_CLIP WEAPON_NOCLIP
#define TRIPMINE_MAX_CLIP WEAPON_NOCLIP
#define SNARK_MAX_CLIP WEAPON_NOCLIP
// ############ hulifezado ############ //
// Novas armas
#define DEAGLE_MAX_CLIP 17
// ############ //

// the default amount of ammo that comes with each gun when it spawns
#define GLOCK_DEFAULT_GIVE 17
#define PYTHON_DEFAULT_GIVE 6
#define MP5_DEFAULT_GIVE 25
#define MP5_DEFAULT_AMMO 25
#define MP5_M203_DEFAULT_GIVE 0
#define SHOTGUN_DEFAULT_GIVE 12
#define CROSSBOW_DEFAULT_GIVE 5
#define RPG_DEFAULT_GIVE 1
#define GAUSS_DEFAULT_GIVE 20
#define EGON_DEFAULT_GIVE 20
#define HANDGRENADE_DEFAULT_GIVE 5
#define SATCHEL_DEFAULT_GIVE 1
#define TRIPMINE_DEFAULT_GIVE 1
#define SNARK_DEFAULT_GIVE 5
#define HIVEHAND_DEFAULT_GIVE 8
// ############ hulifezado ############ //
// Novas armas
#define DESERT_EAGLE_DEFAULT_GIVE 17
// ############ //

// The amount of ammo given to a player by an ammo item.
#define AMMO_URANIUMBOX_GIVE 20
#define AMMO_GLOCKCLIP_GIVE GLOCK_MAX_CLIP
#define AMMO_357BOX_GIVE PYTHON_MAX_CLIP
#define AMMO_MP5CLIP_GIVE MP5_MAX_CLIP
#define AMMO_CHAINBOX_GIVE 200
#define AMMO_M203BOX_GIVE 2
#define AMMO_BUCKSHOTBOX_GIVE 12
#define AMMO_CROSSBOWCLIP_GIVE CROSSBOW_MAX_CLIP
#define AMMO_RPGCLIP_GIVE RPG_MAX_CLIP
#define AMMO_SNARKBOX_GIVE 5

// bullet types
typedef enum
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM,		// glock
	BULLET_PLAYER_MP5,		// mp5
	BULLET_PLAYER_357,		// python
	BULLET_PLAYER_BUCKSHOT, // shotgun
	BULLET_PLAYER_CROWBAR,	// crowbar swipe

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,

	// ############ hulifezado ############ //
	// Novas armas
	BULLET_PLAYER_DEAGLE,
	// ############ //
} Bullet;


#define ITEM_FLAG_SELECTONEMPTY 1
#define ITEM_FLAG_NOAUTORELOAD 2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY 4
#define ITEM_FLAG_LIMITINWORLD 8
#define ITEM_FLAG_EXHAUSTIBLE 16 // A player can totally exhaust their ammo supply and lose this weapon
#define ITEM_FLAG_NOAUTOSWITCHTO 32

#define WEAPON_IS_ONTARGET 0x40

typedef struct
{
	int iSlot;
	int iPosition;
	const char* pszAmmo1; // ammo 1 type
	int iMaxAmmo1;		  // max ammo 1
	const char* pszAmmo2; // ammo 2 type
	int iMaxAmmo2;		  // max ammo 2
	const char* pszName;
	int iMaxClip;
	int iId;
	int iFlags;
	int iWeight; // this value used to determine this weapon's importance in autoselection.
} ItemInfo;

struct AmmoInfo
{
	const char* pszName;
	int iId;

	/**
	*	@brief For exhaustible weapons. If provided, and the player does not have this weapon in their inventory yet it will be given to them.
	*/
	const char* WeaponName = nullptr;
};

inline int giAmmoIndex = 0;

void AddAmmoNameToAmmoRegistry(const char* szAmmoname, const char* weaponName);

// Items that the player has in their inventory that they can use
class CBasePlayerItem : public CBaseAnimating
{
public:
	void SetObjectCollisionBox() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	virtual bool CanAddToPlayer(CBasePlayer* player) { return true; } // return true if the item you want the item added to the player inventory

	virtual void AddToPlayer(CBasePlayer* pPlayer);
	virtual bool AddDuplicate(CBasePlayerItem* pItem) { return false; } // return true if you want your duplicate removed from world
	void EXPORT DestroyItem();
	void EXPORT DefaultTouch(CBaseEntity* pOther); // default weapon touch
	void EXPORT FallThink();					   // when an item is first spawned, this think is run to determine when the object has hit the ground.
	void EXPORT Materialize();					   // make a weapon visible and tangible
	void EXPORT AttemptToMaterialize();			   // the weapon desires to become visible and tangible, if the game rules allow for it
	CBaseEntity* Respawn() override;			   // copy a weapon
	void FallInit();
	void CheckRespawn();
	virtual bool GetItemInfo(ItemInfo* p) { return false; } // returns false if struct not filled out
	virtual bool CanDeploy() { return true; }
	virtual bool Deploy() // returns is deploy was successful
	{
		return true;
	}

	virtual bool CanHolster() { return true; } // can this weapon be put away right now?
	virtual void Holster();
	virtual void UpdateItemInfo() {}

	virtual void ItemPreFrame() {}	// called each frame by the player PreThink
	virtual void ItemPostFrame() {} // called each frame by the player PostThink

	virtual void Drop();
	virtual void Kill();
	virtual void AttachToPlayer(CBasePlayer* pPlayer);

	virtual int PrimaryAmmoIndex() { return -1; }
	virtual int SecondaryAmmoIndex() { return -1; }

	virtual bool UpdateClientData(CBasePlayer* pPlayer) { return false; }

	virtual CBasePlayerWeapon* GetWeaponPtr() { return NULL; }

	virtual void GetWeaponData(weapon_data_t& data) {}

	virtual void SetWeaponData(const weapon_data_t& data) {}

	virtual void DecrementTimers() {}

	static inline ItemInfo ItemInfoArray[MAX_WEAPONS];
	static inline AmmoInfo AmmoInfoArray[MAX_AMMO_SLOTS];

	CBasePlayer* m_pPlayer;
	CBasePlayerItem* m_pNext;
	int m_iId; // WEAPON_???

	virtual int iItemSlot() { return 0; } // return 0 to MAX_ITEMS_SLOTS, used in hud

	int iItemPosition() { return ItemInfoArray[m_iId].iPosition; }
	const char* pszAmmo1() { return ItemInfoArray[m_iId].pszAmmo1; }
	int iMaxAmmo1() { return ItemInfoArray[m_iId].iMaxAmmo1; }
	const char* pszAmmo2() { return ItemInfoArray[m_iId].pszAmmo2; }
	int iMaxAmmo2() { return ItemInfoArray[m_iId].iMaxAmmo2; }
	const char* pszName() { return ItemInfoArray[m_iId].pszName; }
	int iMaxClip() { return ItemInfoArray[m_iId].iMaxClip; }
	int iWeight() { return ItemInfoArray[m_iId].iWeight; }
	int iFlags() { return ItemInfoArray[m_iId].iFlags; }

	// int		m_iIdPrimary;										// Unique Id for primary ammo
	// int		m_iIdSecondary;										// Unique Id for secondary ammo

	//Hack so deploy animations work when weapon prediction is enabled.
	bool m_ForceSendAnimations = false;
};


// inventory items that
class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	// generic weapon versions of CBasePlayerItem calls
	void AddToPlayer(CBasePlayer* pPlayer) override;
	bool AddDuplicate(CBasePlayerItem* pItem) override;

	virtual bool ExtractAmmo(CBasePlayerWeapon* pWeapon);	  //{ return true; }			// Return true if you can add ammo to yourself when picked up
	virtual bool ExtractClipAmmo(CBasePlayerWeapon* pWeapon); // { return true; }			// Return true if you can add ammo to yourself when picked up

	// generic "shared" ammo handlers
	bool AddPrimaryAmmo(CBasePlayerWeapon* origin, int iCount, char* szName, int iMaxClip, int iMaxCarry);
	bool AddSecondaryAmmo(int iCount, char* szName, int iMaxCarry);

	void UpdateItemInfo() override {} // updates HUD state

	bool m_iPlayEmptySound;
	bool m_fFireOnEmpty; // True when the gun is empty and the player is still holding down the
						 // attack key(s)
	virtual bool PlayEmptySound();
	virtual void ResetEmptySound();

	virtual void SendWeaponAnim(int iAnim, int body = 0);

	bool CanDeploy() override;
	virtual bool IsUseable();
	bool DefaultDeploy(const char* szViewModel, const char* szWeaponModel, int iAnim, const char* szAnimExt, int body = 0);
	bool DefaultReload(int iClipSize, int iAnim, float fDelay, int body = 0);

	void ItemPostFrame() override; // called each frame by the player PostThink
	// called by CBasePlayerWeapons ItemPostFrame()
	virtual void PrimaryAttack() {}						  // do "+ATTACK"
	virtual void SecondaryAttack() {}					  // do "+ATTACK2"
	virtual void Reload() {}							  // do "+RELOAD"
	virtual void WeaponIdle() {}						  // called when no buttons pressed
	bool UpdateClientData(CBasePlayer* pPlayer) override; // sends hud info to client dll, if things have changed
	void RetireWeapon();

	// Can't use virtual functions as think functions so this wrapper is needed.
	void EXPORT CallDoRetireWeapon()
	{
		DoRetireWeapon();
	}

	virtual void DoRetireWeapon();
	virtual bool ShouldWeaponIdle() { return false; }
	void Holster() override;
	virtual bool UseDecrement() { return false; }

	int PrimaryAmmoIndex() override;
	int SecondaryAmmoIndex() override;

	void PrintState();

	CBasePlayerWeapon* GetWeaponPtr() override { return this; }
	float GetNextAttackDelay(float delay);

	float m_flPumpTime;
	int m_fInSpecialReload;		   // Are we in the middle of a reload for the shotguns
	float m_flNextPrimaryAttack;   // soonest time ItemPostFrame will call PrimaryAttack
	float m_flNextSecondaryAttack; // soonest time ItemPostFrame will call SecondaryAttack
	float m_flTimeWeaponIdle;	   // soonest time ItemPostFrame will call WeaponIdle
	int m_iPrimaryAmmoType;		   // "primary" ammo index into players m_rgAmmo[]
	int m_iSecondaryAmmoType;	   // "secondary" ammo index into players m_rgAmmo[]
	int m_iClip;				   // number of shots left in the primary weapon clip, -1 it not used
	int m_iClientClip;			   // the last version of m_iClip sent to hud dll
	int m_iClientWeaponState;	   // the last version of the weapon state sent to hud dll (is current weapon, is on target)
	bool m_fInReload;			   // Are we in the middle of a reload;

	int m_iDefaultAmmo; // how much ammo you get when you pick up this weapon as placed by a level designer.

	// hle time creep vars
	float m_flPrevPrimaryAttack;
	float m_flLastFireTime;
};


class CBasePlayerAmmo : public CBaseEntity
{
public:
	void Spawn() override;
	void EXPORT DefaultTouch(CBaseEntity* pOther); // default weapon touch
	virtual bool AddAmmo(CBaseEntity* pOther) { return true; }

	CBaseEntity* Respawn() override;
	void EXPORT Materialize();
};


inline DLL_GLOBAL short g_sModelIndexLaser; // holds the index for the laser beam
constexpr DLL_GLOBAL const char* g_pModelNameLaser = "sprites/laserbeam.spr";

inline DLL_GLOBAL short g_sModelIndexLaserDot;	 // holds the index for the laser beam dot
inline DLL_GLOBAL short g_sModelIndexFireball;	 // holds the index for the fireball
inline DLL_GLOBAL short g_sModelIndexSmoke;		 // holds the index for the smoke cloud
inline DLL_GLOBAL short g_sModelIndexWExplosion; // holds the index for the underwater explosion
inline DLL_GLOBAL short g_sModelIndexBubbles;	 // holds the index for the bubbles model
inline DLL_GLOBAL short g_sModelIndexBloodDrop;	 // holds the sprite index for blood drops
inline DLL_GLOBAL short g_sModelIndexBloodSpray; // holds the sprite index for blood spray (bigger)

extern void ClearMultiDamage();
extern void ApplyMultiDamage(entvars_t* pevInflictor, entvars_t* pevAttacker);
extern void AddMultiDamage(entvars_t* pevInflictor, CBaseEntity* pEntity, float flDamage, int bitsDamageType);

extern void DecalGunshot(TraceResult* pTrace, int iBulletType);
extern void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage);
extern int DamageDecal(CBaseEntity* pEntity, int bitsDamageType);
extern void RadiusDamage(Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType);

typedef struct
{
	CBaseEntity* pEntity;
	float amount;
	int type;
} MULTIDAMAGE;

inline MULTIDAMAGE gMultiDamage;

void FindHullIntersection(const Vector& vecSrc, TraceResult& tr, const Vector& mins, const Vector& maxs, edict_t* pEntity);

#define LOUD_GUN_VOLUME 1000
#define NORMAL_GUN_VOLUME 600
#define QUIET_GUN_VOLUME 200

#define BRIGHT_GUN_FLASH 512
#define NORMAL_GUN_FLASH 256
#define DIM_GUN_FLASH 128

#define BIG_EXPLOSION_VOLUME 2048
#define NORMAL_EXPLOSION_VOLUME 1024
#define SMALL_EXPLOSION_VOLUME 512

#define WEAPON_ACTIVITY_VOLUME 64

#define VECTOR_CONE_1DEGREES Vector(0.00873, 0.00873, 0.00873)
#define VECTOR_CONE_2DEGREES Vector(0.01745, 0.01745, 0.01745)
#define VECTOR_CONE_3DEGREES Vector(0.02618, 0.02618, 0.02618)
#define VECTOR_CONE_4DEGREES Vector(0.03490, 0.03490, 0.03490)
#define VECTOR_CONE_5DEGREES Vector(0.04362, 0.04362, 0.04362)
#define VECTOR_CONE_6DEGREES Vector(0.05234, 0.05234, 0.05234)
#define VECTOR_CONE_7DEGREES Vector(0.06105, 0.06105, 0.06105)
#define VECTOR_CONE_8DEGREES Vector(0.06976, 0.06976, 0.06976)
#define VECTOR_CONE_9DEGREES Vector(0.07846, 0.07846, 0.07846)
#define VECTOR_CONE_10DEGREES Vector(0.08716, 0.08716, 0.08716)
#define VECTOR_CONE_15DEGREES Vector(0.13053, 0.13053, 0.13053)
#define VECTOR_CONE_20DEGREES Vector(0.17365, 0.17365, 0.17365)

//=========================================================
// CWeaponBox - a single entity that can store weapons
// and ammo.
//=========================================================
class CWeaponBox : public CBaseEntity
{
	void Precache() override;
	void Spawn() override;
	void Touch(CBaseEntity* pOther) override;
	bool KeyValue(KeyValueData* pkvd) override;
	bool IsEmpty();
	int GiveAmmo(int iCount, const char* szName, int iMax, int* pIndex = NULL);
	void SetObjectCollisionBox() override;

public:
	void EXPORT Kill();
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	bool HasWeapon(CBasePlayerItem* pCheckItem);
	bool PackWeapon(CBasePlayerItem* pWeapon);
	bool PackAmmo(int iszName, int iCount);

	CBasePlayerItem* m_rgpPlayerItems[MAX_ITEM_TYPES]; // one slot for each

	int m_rgiszAmmo[MAX_AMMO_SLOTS]; // ammo names
	int m_rgAmmo[MAX_AMMO_SLOTS];	 // ammo quantities

	int m_cAmmoTypes; // how many ammo types packed into this box (if packed by a level designer)
};

#ifdef CLIENT_DLL
bool bIsMultiplayer();
void LoadVModel(const char* szViewModel, CBasePlayer* m_pPlayer);
#endif

enum glock_e
{
	GLOCK_IDLE1 = 0,
	GLOCK_IDLE2,
	GLOCK_IDLE3,
	GLOCK_SHOOT,
	GLOCK_SHOOT_EMPTY,
	GLOCK_RELOAD,
	GLOCK_RELOAD_NOT_EMPTY,
	GLOCK_DRAW,
	GLOCK_HOLSTER,
	GLOCK_ADD_SILENCER
};

#ifndef CLIENT_DLL
// ############ hu3lifezado ############ //
// [Terceira Pessoa]
class CHu3XSpot : public CBaseEntity
{
	void Spawn() override;
	void Precache() override;
	
	int ObjectCaps() override { return FCAP_DONT_SAVE; }
	
public:
	void UpdateSpot(CBasePlayer* m_pPlayer);
	void RemoveSpot(CHu3XSpot* m_pLaser);

	static CHu3XSpot* CreateSpot();
};
// ############ //
#endif

class CGlock : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 2; }
	bool GetItemInfo(ItemInfo* p) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void GlockFire(float flSpread, float flCycleTime, bool fUseAutoAim);
	bool Deploy() override;
	void Reload() override;
	void WeaponIdle() override;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Funcoes para renderizar a mira em terceira pessoa
	void ItemPreFrame() override;
	void Holster() override;
	// ############ //

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

private:
	int m_iShell;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Mira em terceira pessoa
#ifndef CLIENT_DLL
	CHu3XSpot* m_pLaser;
#endif
	// ############ //

	unsigned short m_usFireGlock1;
	unsigned short m_usFireGlock2;
};

enum crowbar_e
{
	CROWBAR_IDLE = 0,
	CROWBAR_DRAW,
	CROWBAR_HOLSTER,
	CROWBAR_ATTACK1HIT,
	CROWBAR_ATTACK1MISS,
	CROWBAR_ATTACK2MISS,
	CROWBAR_ATTACK2HIT,
	CROWBAR_ATTACK3MISS,
	CROWBAR_ATTACK3HIT
};

class CCrowbar : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 1; }
	void EXPORT SwingAgain();
	void EXPORT Smack();
	bool GetItemInfo(ItemInfo* p) override;

	void PrimaryAttack() override;
	bool Swing(bool fFirst);
	bool Deploy() override;
	void Holster() override;
	int m_iSwing;
	TraceResult m_trHit;

	// ############ hu3lifezado ############ //
	// Tiro secundario, adaptado de: 
	// http://web.archive.org/web/20020717063241/http://lambda.bubblemod.org/tuts/crowbar/
	void SecondaryAttack();
	// ############ //

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usCrowbar;
};

class CFlyingCrowbar : public CBasePlayerWeapon
{
public:
	void Spawn();
	void Precache();
	void BubbleThink();
	void SpinTouch(CBaseEntity *pOther);
	CBasePlayer *m_pPlayer;

private:
	EHANDLE m_hOwner;        // Original owner is stored here so we can
							 // allow the crowbar to hit the user.

	unsigned short m_usCrowbar;
};

class CFlyingTouros : public CBasePlayerWeapon
{
public:
	void Spawn();
	void Precache();
	void BubbleThink();
	void SpinTouch(CBaseEntity *pOther);	
	void SaveQuality(int m_quality);
	void SetMode(int m_mode);
	CBasePlayer *m_pPlayer;

private:
	EHANDLE m_hOwner;        // Original owner is stored here so we can
							 // allow the crowbar to hit the user.

	unsigned short m_usCrowbar;

	int quality;
	int mode;
};

enum python_e
{
	PYTHON_IDLE1 = 0,
	PYTHON_FIDGET,
	PYTHON_FIRE1,
	PYTHON_RELOAD,
	PYTHON_HOLSTER,
	PYTHON_DRAW,
	PYTHON_IDLE2,
	PYTHON_IDLE3
};

class CPython : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 2; }
	bool GetItemInfo(ItemInfo* p) override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Holster() override;
	void Reload() override;
	void WeaponIdle() override;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Funcoes para renderizar a mira em terceira pessoa
	void ItemPreFrame() override;
	// ############ //

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usFirePython;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Mira em terceira pessoa
#ifndef CLIENT_DLL
	CHu3XSpot* m_pLaser;
#endif
	// ############ //
};

enum mp5_e
{
	MP5_LONGIDLE = 0,
	MP5_IDLE1,
	MP5_LAUNCH,
	MP5_RELOAD,
	MP5_DEPLOY,
	MP5_FIRE1,
	MP5_FIRE2,
	MP5_FIRE3,
};

class CMP5 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 3; }
	bool GetItemInfo(ItemInfo* p) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Reload() override;
	void WeaponIdle() override;
	float m_flNextAnimTime;
	int m_iShell;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Funcoes para renderizar a mira em terceira pessoa
	void ItemPreFrame() override;
	void Holster() override;
	// ############ //

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usMP5;
	unsigned short m_usMP52;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Mira em terceira pessoa
#ifndef CLIENT_DLL
	CHu3XSpot* m_pLaser;
#endif
	// ############ //
};

enum crossbow_e
{
	CROSSBOW_IDLE1 = 0, // full
	CROSSBOW_IDLE2,		// empty
	CROSSBOW_FIDGET1,	// full
	CROSSBOW_FIDGET2,	// empty
	CROSSBOW_FIRE1,		// full
	CROSSBOW_FIRE2,		// reload
	CROSSBOW_FIRE3,		// empty
	CROSSBOW_RELOAD,	// from empty
	CROSSBOW_DRAW1,		// full
	CROSSBOW_DRAW2,		// empty
	CROSSBOW_HOLSTER1,	// full
	CROSSBOW_HOLSTER2,	// empty
};

class CCrossbow : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 3; }
	bool GetItemInfo(ItemInfo* p) override;

	void FireBolt();
	void FireSniperBolt();
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Holster() override;
	void Reload() override;
	void WeaponIdle() override;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Funcoes para renderizar a mira em terceira pessoa
	void ItemPreFrame() override;
	// ############ //

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usCrossbow;
	unsigned short m_usCrossbow2;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Mira em terceira pessoa
#ifndef CLIENT_DLL
	CHu3XSpot* m_pLaser;
#endif
	// ############ //
};

enum shotgun_e
{
	SHOTGUN_IDLE = 0,
	SHOTGUN_FIRE,
	SHOTGUN_FIRE2,
	SHOTGUN_RELOAD,
	SHOTGUN_PUMP,
	SHOTGUN_START_RELOAD,
	SHOTGUN_DRAW,
	SHOTGUN_HOLSTER,
	SHOTGUN_IDLE4,
	SHOTGUN_IDLE_DEEP
};

class CShotgun : public CBasePlayerWeapon
{
public:
#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
#endif


	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 3; }
	bool GetItemInfo(ItemInfo* p) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Reload() override;
	void WeaponIdle() override;
	void ItemPostFrame() override;
	int m_fInReload; //TODO: not used, remove
	float m_flNextReload;
	int m_iShell;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Funcoes para renderizar a mira em terceira pessoa
	void ItemPreFrame() override;
	void Holster() override;
	// ############ //

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usDoubleFire;
	unsigned short m_usSingleFire;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Mira em terceira pessoa
#ifndef CLIENT_DLL
	CHu3XSpot* m_pLaser;
#endif
	// ############ //
};

class CLaserSpot : public CBaseEntity
{
	void Spawn() override;
	void Precache() override;

	int ObjectCaps() override { return FCAP_DONT_SAVE; }

public:
	void Suspend(float flSuspendTime);
	void EXPORT Revive();

	static CLaserSpot* CreateSpot();
};

enum rpg_e
{
	RPG_IDLE = 0,
	RPG_FIDGET,
	RPG_RELOAD,	   // to reload
	RPG_FIRE2,	   // to empty
	RPG_HOLSTER1,  // loaded
	RPG_DRAW1,	   // loaded
	RPG_HOLSTER2,  // unloaded
	RPG_DRAW_UL,   // unloaded
	RPG_IDLE_UL,   // unloaded idle
	RPG_FIDGET_UL, // unloaded fidget
};

class CRpg : public CBasePlayerWeapon
{
public:
#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn() override;
	void Precache() override;
	void Reload() override;
	int iItemSlot() override { return 4; }
	bool GetItemInfo(ItemInfo* p) override;

	bool Deploy() override;
	bool CanHolster() override;
	void Holster() override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void WeaponIdle() override;

	void UpdateSpot();
	bool ShouldWeaponIdle() override { return true; }

	CLaserSpot* m_pSpot;
	bool m_fSpotActive;
	int m_cActiveRockets; // how many missiles in flight from this launcher right now?

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usRpg;
};

class CRpgRocket : public CGrenade
{
public:
	~CRpgRocket() override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
	void Spawn() override;
	void Precache() override;
	void EXPORT FollowThink();
	void EXPORT IgniteThink();
	void EXPORT RocketTouch(CBaseEntity* pOther);
	static CRpgRocket* CreateRpgRocket(Vector vecOrigin, Vector vecAngles, CBaseEntity* pOwner, CRpg* pLauncher);

	CRpg* GetLauncher();

	int m_iTrail;
	float m_flIgniteTime;
	EHANDLE m_hLauncher; // handle back to the launcher that fired me.
};

#define GAUSS_PRIMARY_CHARGE_VOLUME 256 // how loud gauss is while charging
#define GAUSS_PRIMARY_FIRE_VOLUME 450	// how loud gauss is when discharged

enum gauss_e
{
	GAUSS_IDLE = 0,
	GAUSS_IDLE2,
	GAUSS_FIDGET,
	GAUSS_SPINUP,
	GAUSS_SPIN,
	GAUSS_FIRE,
	GAUSS_FIRE2,
	GAUSS_HOLSTER,
	GAUSS_DRAW,
	// ############ hu3lifezado ############ //
	// Pequena gambiarra para consertar o saque da arma
	GAUSS_FIRE_HU3_FIX
	// ############ //
};

class CGauss : public CBasePlayerWeapon
{
public:
#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 4; }
	bool GetItemInfo(ItemInfo* p) override;

	bool Deploy() override;
	void Holster() override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void WeaponIdle() override;

	void StartFire();
	void Fire(Vector vecOrigSrc, Vector vecDirShooting, float flDamage);
	float GetFullChargeTime();
	int m_iBalls;
	int m_iGlow;
	int m_iBeam;
	int m_iSoundState; // don't save this

	// was this weapon just fired primary or secondary?
	// we need to know so we can pick the right set of effects.
	bool m_fPrimaryFire;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Funcoes para renderizar a mira em terceira pessoa
	void ItemPreFrame() override;
	// Codigo dos aneis de poder
	void SonicAttack(int force);
	void WriteBeamColor(int force);
	// Textura dos aneis
	int m_iSpriteTexture;
	// Tamanho dos aneis
	int m_iSpriteTextureRange;
	// ############ //

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

private:
	void SendStopEvent(bool sendToHost);

private:
	unsigned short m_usGaussFire;
	unsigned short m_usGaussSpin;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Mira em terceira pessoa
#ifndef CLIENT_DLL
	CHu3XSpot* m_pLaser;
#endif
	// ############ //
};

enum egon_e
{
	EGON_IDLE1 = 0,
	EGON_FIDGET1,
	EGON_ALTFIREON,
	EGON_ALTFIRECYCLE,
	EGON_ALTFIREOFF,
	EGON_FIRE1,
	EGON_FIRE2,
	EGON_FIRE3,
	EGON_FIRE4,
	EGON_DRAW,
	EGON_HOLSTER
};

enum EGON_FIRESTATE
{
	FIRE_OFF,
	FIRE_CHARGE
};

enum EGON_FIREMODE
{
	FIRE_NARROW,
	FIRE_WIDE
};

#define EGON_PRIMARY_VOLUME 450
#define EGON_BEAM_SPRITE "sprites/xbeam1.spr"
#define EGON_FLARE_SPRITE "sprites/XSpark1.spr"
#define EGON_SOUND_OFF "weapons/egon_off1.wav"
#define EGON_SOUND_RUN "weapons/egon_run3.wav"
#define EGON_SOUND_STARTUP "weapons/egon_windup2.wav"

class CEgon : public CBasePlayerWeapon
{
public:
#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 4; }
	bool GetItemInfo(ItemInfo* p) override;

	bool Deploy() override;
	void Holster() override;

	void UpdateEffect(const Vector& startPoint, const Vector& endPoint, float timeBlend);

	void CreateEffect();
	void DestroyEffect();

	void EndAttack();
	void Attack();
	void PrimaryAttack() override;
	bool ShouldWeaponIdle() override { return true; }
	void WeaponIdle() override;

	float m_flAmmoUseTime; // since we use < 1 point of ammo per update, we subtract ammo on a timer.

	float GetPulseInterval();
	float GetDischargeInterval();

	void Fire(const Vector& vecOrigSrc, const Vector& vecDir);

	bool HasAmmo();
	bool CanHolster();

	void UseAmmo(int count);

	CBeam* m_pBeam;
	CBeam* m_pNoise;
	CSprite* m_pSprite;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Funcoes para renderizar a mira em terceira pessoa
	void ItemPreFrame() override;
	// ############ //

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

	unsigned short m_usEgonStop;

private:
	float m_shootTime;
	EGON_FIREMODE m_fireMode;
	float m_shakeTime;
	bool m_deployed;

	unsigned short m_usEgonFire;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Mira em terceira pessoa
#ifndef CLIENT_DLL
	CHu3XSpot* m_pLaser;
#endif
	// ############ //
};

enum hgun_e
{
	HGUN_IDLE1 = 0,
	HGUN_FIDGETSWAY,
	HGUN_FIDGETSHAKE,
	HGUN_DOWN,
	HGUN_UP,
	HGUN_SHOOT
};

class CHgun : public CBasePlayerWeapon
{
public:
#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 4; }
	bool GetItemInfo(ItemInfo* p) override;
	void AddToPlayer(CBasePlayer* pPlayer) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	bool IsUseable() override;
	void Holster() override;
	void Reload() override;
	void WeaponIdle() override;
	float m_flNextAnimTime;

	float m_flRechargeTime;

	int m_iFirePhase; // don't save me.

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Funcoes para renderizar a mira em terceira pessoa
	void ItemPreFrame() override;
	// ############ //

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usHornetFire;

	// ############ hu3lifezado ############ //
	// [Terceira Pessoa]
	// Mira em terceira pessoa
#ifndef CLIENT_DLL
	CHu3XSpot* m_pLaser;
#endif
	// ############ //
};

enum handgrenade_e
{
	HANDGRENADE_IDLE = 0,
	HANDGRENADE_FIDGET,
	HANDGRENADE_PINPULL,
	HANDGRENADE_THROW1, // toss
	HANDGRENADE_THROW2, // medium
	HANDGRENADE_THROW3, // hard
	HANDGRENADE_HOLSTER,
	HANDGRENADE_DRAW
};

class CHandGrenade : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 5; }
	bool GetItemInfo(ItemInfo* p) override;

	void PrimaryAttack() override;
	bool Deploy() override;
	bool CanHolster() override;
	void Holster() override;
	void WeaponIdle() override;

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}
};

enum satchel_e
{
	SATCHEL_IDLE1 = 0,
	SATCHEL_FIDGET1,
	SATCHEL_DRAW,
	SATCHEL_DROP
};

enum satchel_radio_e
{
	SATCHEL_RADIO_IDLE1 = 0,
	SATCHEL_RADIO_FIDGET1,
	SATCHEL_RADIO_DRAW,
	SATCHEL_RADIO_FIRE,
	SATCHEL_RADIO_HOLSTER
};

class CSatchel : public CBasePlayerWeapon
{
public:
#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 5; }
	bool GetItemInfo(ItemInfo* p) override;
	void AddToPlayer(CBasePlayer* pPlayer) override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool AddDuplicate(CBasePlayerItem* pOriginal) override;
	bool CanDeploy() override;
	bool Deploy() override;
	bool IsUseable() override;

	void Holster() override;
	void WeaponIdle() override;
	void Throw();
	void Detonate();

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}
};

enum tripmine_e
{
	TRIPMINE_IDLE1 = 0,
	TRIPMINE_IDLE2,
	TRIPMINE_ARM1,
	TRIPMINE_ARM2,
	TRIPMINE_FIDGET,
	TRIPMINE_HOLSTER,
	TRIPMINE_DRAW,
	TRIPMINE_WORLD,
	TRIPMINE_GROUND,
};

class CTripmine : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 5; }
	bool GetItemInfo(ItemInfo* p) override;
	void SetObjectCollisionBox() override
	{
		//!!!BUGBUG - fix the model!
		pev->absmin = pev->origin + Vector(-16, -16, -5);
		pev->absmax = pev->origin + Vector(16, 16, 28);
	}

	void PrimaryAttack() override;
	bool Deploy() override;
	void Holster() override;
	void WeaponIdle() override;

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usTripFire;
};

enum squeak_e
{
	SQUEAK_IDLE1 = 0,
	SQUEAK_FIDGETFIT,
	SQUEAK_FIDGETNIP,
	SQUEAK_DOWN,
	SQUEAK_UP,
	SQUEAK_THROW
};

class CSqueak : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 5; }
	bool GetItemInfo(ItemInfo* p) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Holster() override;
	void WeaponIdle() override;
	bool m_fJustThrown;

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

private:
	unsigned short m_usSnarkFire;
};

// ############ hu3lifezado ############ //
// Novas armas

enum DesertEagleAnim
{
	DEAGLE_IDLE1 = 0,
	DEAGLE_IDLE2,
	DEAGLE_IDLE3,
	DEAGLE_IDLE4,
	DEAGLE_IDLE5,
	DEAGLE_SHOOT,
	DEAGLE_SHOOT_EMPTY,
	DEAGLE_RELOAD_NOSHOT,
	DEAGLE_RELOAD,
	DEAGLE_DRAW,
	DEAGLE_HOLSTER,
	DEAGLE_SWINGING
};

class CDesertEagle : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int iItemSlot() override { return 2; }
	bool GetItemInfo(ItemInfo* p) override;

	bool Deploy() override;
	void Holster() override;
	void WeaponIdle() override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Reload() override;

	// ############ hu3lifezado ############ //
	// Chance da arma quebrar e sair voando
	bool RandomlyBreak();
	// Chance da arma travar e nao atirar mais
	bool RandomlyJammed();
	// Definir a qualidade da arma
	void SetQuality();
	// Dano por estilhacos
	void ShrapnelDamage(int chance, int min_damage, int max_damage);
	// Perder toda a municao
	bool RandomlyLostAllAmmo();
	// Varejar a arma
	void ThrowWeapon(bool isReloading);
	// Tiros atrasados
	void DelayedShot();
	// Tiros instantaneos
	void InstantShot(bool forceShot);
	// ############ //

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

private:
	void UpdateLaser();

private:
	int m_iShell;
	unsigned short m_usFireEagle;

	// ############ hu3lifezado ############ //
	// Tempo ate processar a nova chance da arma atirar sozinha
	float m_nextbadshootchance;
	// Nos nao podemos imprimir mensagens assim que o jogo comeca
	float m_waitforthegametobeready;
	// Arma travada
	bool m_jammedweapon;
	// Cada defeito da arma tem um bonus que eh adicionado de 0 ate 100% dependendo dessa qualidade 9 ate 1;
	float m_qualitypercentageeffect;
	// Diz se estamos aguardando um tiro
	bool m_hasDelayedShot;
	// Indica se precisamos informar a qualidade da arma
	bool m_tellQuality;
#ifdef CLIENT_DLL
	// Sincronida da qualidade inicial
	cvar_t	*hu3_touros_qualidade_inicial;
#endif

public:
	// Qualidade da arma (de 1 ate 9, nunca 10)
	int m_quality;
	// ############ //
};

enum KnifeAnim
{
	KNIFE_IDLE1 = 0,
	KNIFE_DRAW,
	KNIFE_HOLSTER,
	KNIFE_ATTACK1,
	KNIFE_ATTACK1MISS,
	KNIFE_ATTACK2,
	KNIFE_ATTACK2HIT,
	KNIFE_ATTACK3,
	KNIFE_ATTACK3HIT,
	KNIFE_IDLE2,
	KNIFE_IDLE3,
	KNIFE_CHARGE,
	KNIFE_STAB,
	// ############ hu3lifezado ############ //
	// Adicionadas novas animacoes
	KNIFE_PICHAVASIM,
	KNIFE_SELECTION
	// ############ //
};

class CKnife : public CBasePlayerWeapon
{
public:
	using BaseClass = CBasePlayerWeapon;

	void Precache() override;

	void Spawn() override;

	bool Deploy() override;

	void Holster() override;

	void PrimaryAttack() override;

	int iItemSlot() override { return 1; }

	bool GetItemInfo(ItemInfo* p) override;

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

	// ############ hu3lifezado ############ //
	// Funcoes deletadas
	// bool Swing(const bool bFirst);
	// void SwingAgain();
	// void Smack();

	// Pixação
	void WeaponIdle() override;

	void SecondaryAttack() override;

	void HandleAnimationAndSound();

	void ApplyDamage();

	bool TraceSomeShit();

	void PlaceColor();

	static const char *pSelectionSounds[];
	// ############ //

private:
	unsigned short m_usKnife;

	int m_iSwing;
	TraceResult m_trHit;

	// ############ hu3lifezado ############ //
	// Tempo ate processar novamente dano, animacoes e sons
	float m_nextthink;
	// Tempo ate proxima mudanca de cor ser liberada
	float m_nextcolorchange;
	// Tempo que quando ultrapassado forca a rechecagem do primeiro acerto
	float m_nextfirsthit;
	// Tempo para o proximo som de spray aplicado em parede
	float m_nextsprayonwallsound;
	// Cor selecionada (64 clientes)
#ifndef CLIENT_DLL
	int hu3_spray_color[64];
#else
	int hu3_spray_color[2];
#endif
	// [COOP] Resetar a selecao no HUD
	bool reset_hud;
	// ############ //
};

enum pipewrench_e
{
	PIPEWRENCH_IDLE1 = 0,
	PIPEWRENCH_IDLE2,
	PIPEWRENCH_IDLE3,
	PIPEWRENCH_DRAW,
	PIPEWRENCH_HOLSTER,
	PIPEWRENCH_ATTACK1HIT,
	PIPEWRENCH_ATTACK1MISS,
	PIPEWRENCH_ATTACK2HIT,
	PIPEWRENCH_ATTACK2MISS,
	PIPEWRENCH_ATTACK3HIT,
	PIPEWRENCH_ATTACK3MISS,
	PIPEWRENCH_BIG_SWING_START,
	PIPEWRENCH_BIG_SWING_HIT,
	PIPEWRENCH_BIG_SWING_MISS,
	PIPEWRENCH_BIG_SWING_IDLE
};

class CPipewrench : public CBasePlayerWeapon
{
private:
	enum SwingMode
	{
		SWING_NONE = 0,
		SWING_START_BIG,
		SWING_DOING_BIG,
	};

public:
	using BaseClass = CBasePlayerWeapon;

#ifndef CLIENT_DLL
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn() override;
	void Precache() override;
	void EXPORT SwingAgain();
	void EXPORT Smack();

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Swing(const bool bFirst);
	void EXPORT BigSwing();
	bool Deploy() override;
	void Holster() override;
	void WeaponIdle() override;

	void GetWeaponData(weapon_data_t& data) override;

	void SetWeaponData(const weapon_data_t& data) override;

	int iItemSlot() override { return 1; }

	bool GetItemInfo(ItemInfo* p) override;

	bool UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

	float m_flBigSwingStart;
	int m_iSwingMode;
	int m_iSwing;
	TraceResult m_trHit;

private:
	unsigned short m_usPipewrench;
};

// ############