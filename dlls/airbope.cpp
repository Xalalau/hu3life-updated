// ############ hu3lifezado ############ //
// HU3-LIFE BOPE AEREO

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "soundent.h"
#include "effects.h"
#include "customentity.h"
#include "osprey.h"

class CAirBope : public COsprey
{
public:
	void Spawn(void);
	void Precache(void);
};

LINK_ENTITY_TO_CLASS(bope_aereo, CAirBope);

void CAirBope::Spawn(void)
{
	Precache();
	// motor
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/bope_aereo.mdl");
	UTIL_SetSize(pev, Vector(-400, -400, -100), Vector(400, 400, 32));
	UTIL_SetOrigin(pev, pev->origin);

	pev->flags |= FL_MONSTER;
	pev->takedamage = DAMAGE_YES;
	m_flRightHealth = 200;
	m_flLeftHealth = 200;
	pev->health = 400;

	m_flFieldOfView = 0; // 180 degrees

	pev->sequence = 0;
	ResetSequenceInfo();
	pev->frame = RANDOM_LONG(0, 0xFF);

	InitBoneControllers();

	SetThink(&COsprey::FindAllThink);
	SetUse(&COsprey::CommandUse);

	if (!(pev->spawnflags & SF_WAITFORTRIGGER))
	{
		pev->nextthink = gpGlobals->time + 1.0;
	}

	m_pos2 = pev->origin;
	m_ang2 = pev->angles;
	m_vel2 = pev->velocity;
}

void CAirBope::Precache(void)
{
	UTIL_PrecacheOther("monster_human_grunt");

	PRECACHE_MODEL("models/bope_aereo.mdl");
	PRECACHE_MODEL("models/HVR.mdl");

	PRECACHE_SOUND("apache/ap_rotor4.wav");
	PRECACHE_SOUND("weapons/mortarhit.wav");

	m_iSpriteTexture = PRECACHE_MODEL("sprites/rope.spr");

	m_iExplode = PRECACHE_MODEL("sprites/fexplo.spr");
	m_iTailGibs = PRECACHE_MODEL("models/osprey_tailgibs.mdl");
	m_iBodyGibs = PRECACHE_MODEL("models/osprey_bodygibs.mdl");
	m_iEngineGibs = PRECACHE_MODEL("models/osprey_enginegibs.mdl");
}