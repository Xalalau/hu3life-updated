// ##############################
// HU3-LIFE PAVAO EXPLENDIDO
// ##############################

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "game.h"
#include "peacock.h"

Task_t	tlPVRangeFly[] =
{
	{ TASK_STOP_MOVING,			(float)0 },
	{ TASK_FACE_IDEAL,			(float)0 },
	{ TASK_RANGE_ATTACK1,		(float)0 },
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_IDEAL,			(float)0 },
	{ TASK_WAIT_RANDOM,			(float)0.5 },
};

Schedule_t	slPVRangeFly[] =
{
	{
		tlPVRangeFly,
		ARRAYSIZE(tlPVRangeFly),
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_NO_AMMO_LOADED,
	0,
	"HCRangeAttack1"
	},
};

Task_t	tlPVRangeFlyFast[] =
{
	{ TASK_STOP_MOVING,			(float)0 },
	{ TASK_FACE_IDEAL,			(float)0 },
	{ TASK_RANGE_ATTACK1,		(float)0 },
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
};

Schedule_t	slPVRangeFlyFast[] =
{
	{
		tlPVRangeFlyFast,
		ARRAYSIZE(tlPVRangeFlyFast),
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_NO_AMMO_LOADED,
	0,
	"HCRAFast"
	},
};

DEFINE_CUSTOM_SCHEDULES(CPeacock){
	slPVRangeFly,
	slPVRangeFlyFast,
};

IMPLEMENT_CUSTOM_SCHEDULES(CPeacock, CBaseMonster);


const char *CPeacock::pIdleSounds[] =
{
	"pavao/idle1.wav",
	"pavao/idle2.wav",
};

const char *CPeacock::pAlertSounds[] =
{
	"pavao/alert1.wav",
	"pavao/alert2.wav",
	"pavao/alert3.wav",
};

const char *CPeacock::pPainSounds[] =
{
	"pavao/pain1.wav",
	"pavao/pain2.wav",
	"pavao/pain3.wav",
};

const char *CPeacock::pFlySounds[] =
{
	"pavao/flying1.wav",
	"pavao/flying2.wav",
	"pavao/flying3.wav",
};

const char *CPeacock::pDeathSounds[] =
{
	"pavao/die1.wav",
	"pavao/die2.wav",
	"pavao/die3.wav",
};

LINK_ENTITY_TO_CLASS(monster_peacock, CPeacock);

int CPeacock::Classify()
{
	// Alguns pavoes sao assassinos
	switch(RANDOM_LONG(1, 6))
	{
	case 1:
		return CLASS_ALIEN_MONSTER;
	case 2:
		return CLASS_ALIEN_PREDATOR;
	default:
		return CLASS_ALIEN_PASSIVE;
	}
}

//=========================================================
// Center - returns the real center of the headcrab.  The 
// bounding box is much larger than the actual creature so 
// this is needed for targeting
//=========================================================
Vector CPeacock::Center()
{
	return Vector(pev->origin.x, pev->origin.y, pev->origin.z + 6);
}


Vector CPeacock::BodyTarget(const Vector &posSrc)
{
	return Center();
}

// ======================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CPeacock ::UpdateYawSpeed()
{
	int ys;

	switch(m_Activity)
	{
	case ACT_IDLE:			
		ys = 30;
		break;
	case ACT_RUN:			
	case ACT_WALK:			
		ys = 20;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 60;
		break;
	case ACT_RANGE_ATTACK1:	
		ys = 30;
		break;
	default:
		ys = 30;
		break;
	}

	pev->yaw_speed = ys;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CPeacock::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch(pEvent->event)
	{
		case PV_AE_JUMPATTACK:
		{
			ClearBits(pev->flags, FL_ONGROUND);

			pev->origin = pev->origin + Vector(0 , 0 , 1);// take him off ground so engine doesn't instantly reset onground 
			UTIL_MakeVectors(pev->angles);

			Vector vecJumpDir;
			if(m_hEnemy != NULL)
			{
				float gravity = g_psv_gravity->value;
				if(gravity <= 1)
					gravity = 1;

				// How fast does the headcrab need to travel to reach that height given gravity?
				float height =(m_hEnemy->pev->origin.z + m_hEnemy->pev->view_ofs.z - pev->origin.z);
				if(height < 16)
					height = 16;
				float speed = sqrt(2 * gravity * height) / 3;
				float time =(speed / gravity) * 3;

				// Scale the sideways velocity to get there at the right time
				vecJumpDir =(m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs - pev->origin);
				vecJumpDir = vecJumpDir *(1.0 / time);

				// Speed to offset gravity at the desired height
				vecJumpDir.z = speed * 6;

				// Don't jump too far/fast
				float distance = vecJumpDir.Length() * 2;
				
				if(distance > 1500)
				{
					vecJumpDir = vecJumpDir *(1500 / distance);
				}
			}
			else
			{
				// jump hop, don't care where
				vecJumpDir = Vector(gpGlobals->v_forward.x, gpGlobals->v_forward.y, gpGlobals->v_up.z) * 350;
			}

			int iSound = RANDOM_LONG(0,2);
			if(iSound != 0)
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pFlySounds[iSound], GetSoundVolume(), ATTN_IDLE, 0, GetVoicePitch());

			pev->velocity = vecJumpDir;
			m_flNextAttack = gpGlobals->time + 2;
		}
		break;

		default:
			CBaseMonster::HandleAnimEvent(pEvent);
			break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CPeacock::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/pavao.mdl");
	UTIL_SetSize(pev, Vector(-12, -12, 0), Vector(12, 12, 24));

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->effects		= 0;
	pev->health			= gSkillData.headcrabHealth;
	pev->view_ofs		= Vector(0, 0, 20);// position of the eyes relative to monster's origin.
	pev->yaw_speed		= 5;//!!! should we put this in the monster's changeanim function since turn rates may vary with state/anim?
	m_flFieldOfView		= 0.3;// indicates the width of this monster's forward view cone(as a dotproduct result)
	m_MonsterState		= MONSTERSTATE_NONE;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CPeacock::Precache()
{
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pFlySounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);

	PRECACHE_MODEL("models/pavao.mdl");
}	


//=========================================================
// RunTask 
//=========================================================
void CPeacock::RunTask(Task_t* pTask)
{
	switch(pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
	case TASK_RANGE_ATTACK2:
		{
			if(m_fSequenceFinished)
			{
				TaskComplete();
				SetTouch(NULL);
				m_IdealActivity = ACT_IDLE;
			}
			break;
		}
	default:
		{
			CBaseMonster::RunTask(pTask);
		}
	}
}

//=========================================================
// LeapTouch - this is the headcrab's touch function when it
// is in the air
//=========================================================
void CPeacock::LeapTouch(CBaseEntity *pOther)
{
	if(!pOther->pev->takedamage)
	{
		return;
	}

	if(pOther->Classify() == Classify())
	{
		return;
	}

	// Don't hit if back on ground
	if(!FBitSet(pev->flags, FL_ONGROUND))
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pPainSounds), GetSoundVolume(), ATTN_IDLE, 0, GetVoicePitch());
		
		pOther->TakeDamage(pev, pev, GetDamageAmount(), DMG_SLASH);
	}

	SetTouch(NULL);
}

//=========================================================
// PrescheduleThink
//=========================================================
void CPeacock::PrescheduleThink()
{
	// make the crab coo a little bit in combat state
	if(m_MonsterState == MONSTERSTATE_COMBAT && RANDOM_FLOAT(0, 5) < 0.1)
	{
		IdleSound();
	}
}

void CPeacock::StartTask(Task_t* pTask)
{
	const Task_t* pTask2 = pTask;

	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch(pTask2->iTask)
	{
	case TASK_RANGE_ATTACK1:
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pFlySounds[0], GetSoundVolume(), ATTN_IDLE, 0, GetVoicePitch());
			m_IdealActivity = ACT_RANGE_ATTACK1;
			SetTouch(&CPeacock::LeapTouch);
			break;
		}
	default:
		{
			CBaseMonster::StartTask(pTask);
		}
	}
}

//=========================================================
// CheckRangeAttack1
//=========================================================
bool CPeacock::CheckRangeAttack1(float flDot, float flDist)
{
	if(FBitSet(pev->flags, FL_ONGROUND) && flDist <= 256 && flDot >= 0.65)
	{
		return true;
	}
	return false;
}

//=========================================================
// CheckRangeAttack2
//=========================================================
bool CPeacock::CheckRangeAttack2(float flDot, float flDist)
{
	return false;
	// BUGBUG: Why is this code here?  There is no ACT_RANGE_ATTACK2 animation.  I've disabled it for now.
#if 0
	if(FBitSet(pev->flags, FL_ONGROUND) && flDist > 64 && flDist <= 256 && flDot >= 0.5)
	{
		return true;
	}
	return false;
#endif
}

bool CPeacock::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	// Don't take any acid damage -- BigMomma's mortar is acid
	if(bitsDamageType & DMG_ACID)
		flDamage = 0;

	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

//=========================================================
// IdleSound
//=========================================================
#define CRAB_ATTN_IDLE(float)1.5
void CPeacock::IdleSound()
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), GetSoundVolume(), ATTN_IDLE, 0, GetVoicePitch());
}

//=========================================================
// AlertSound 
//=========================================================
void CPeacock::AlertSound()
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), GetSoundVolume(), ATTN_IDLE, 0, GetVoicePitch());
}

//=========================================================
// AlertSound 
//=========================================================
void CPeacock::PainSound()
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), GetSoundVolume(), ATTN_IDLE, 0, GetVoicePitch());
}

//=========================================================
// DeathSound 
//=========================================================
void CPeacock::DeathSound()
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), GetSoundVolume(), ATTN_IDLE, 0, GetVoicePitch());
}

Schedule_t* CPeacock::GetScheduleOfType(int Type)
{
	switch	(Type)
	{
		case SCHED_RANGE_ATTACK1:
		{
			return &slPVRangeFly[ 0 ];
		}
		break;
	}

	return CBaseMonster::GetScheduleOfType(Type);
}