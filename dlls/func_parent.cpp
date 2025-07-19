// ##############################
// HU3-LIFE ENTIDADE ARRASTADORA
// ##############################
// Arrasta outras entidades consigo ao se mover.

#include "extdll.h"
#include "util.h"
#include "cbase.h"

#ifndef CLIENT_DLL
TYPEDESCRIPTION CFuncParent::m_SaveData[] =
{
	DEFINE_FIELD(CFuncParent, speed, FIELD_FLOAT),
	DEFINE_FIELD(CFuncParent, wait, FIELD_FLOAT),

	DEFINE_FIELD(CFuncParent, parent01_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent01, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent02_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent02, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent03_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent03, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent04_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent04, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent05_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent05, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent06_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent06, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent07_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent07, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent08_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent08, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent09_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent09, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent10_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent10, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent11_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent11, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent12_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent12, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent13_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent13, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent14_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent14, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent15_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent15, FIELD_EVARS),
	DEFINE_FIELD(CFuncParent, parent16_name, FIELD_STRING),
	DEFINE_FIELD(CFuncParent, parent16, FIELD_EVARS),
};

IMPLEMENT_SAVERESTORE(CFuncParent, CFuncParent::BaseClass);
#endif

LINK_ENTITY_TO_CLASS(func_parent, CFuncParent);

bool CFuncParent::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		wait = atof(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent01"))
	{
		parent01_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent02"))
	{
		parent02_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent03"))
	{
		parent03_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent04"))
	{
		parent04_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent05"))
	{
		parent05_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent06"))
	{
		parent06_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent07"))
	{
		parent07_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent08"))
	{
		parent08_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent09"))
	{
		parent09_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent10"))
	{
		parent10_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent11"))
	{
		parent11_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent12"))
	{
		parent12_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent13"))
	{
		parent13_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent14"))
	{
		parent14_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent15"))
	{
		parent15_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "parent16"))
	{
		parent16_name = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else
		return CBaseToggle::KeyValue(pkvd);
}

void CFuncParent::Spawn()
{
	// Defino os angulos do movimento
	SetMovedir(pev);

	// Defino a velocidade
	if (pev->speed == 0)
		pev->speed = 100;
	speed = pev->speed;

	// Modifico as propriedades basicas da entidade
	pev->movetype = MOVETYPE_PUSH;
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	// Pego o vetor da posicao atual
	m_vecPosition1 = pev->origin;

	// Calculo o vetor deslocamento (creditos para a Valve):
	// Subtract 2 from size because the engine expands bboxes by 1 in all directions making the size too big
	m_vecPosition2 = m_vecPosition1 + (pev->movedir * (fabs(pev->movedir.x * (pev->size.x - 2)) + fabs(pev->movedir.y * (pev->size.y - 2)) + fabs(pev->movedir.z * (pev->size.z - 2)) - m_flLip));

	ASSERTSZ(m_vecPosition1 != m_vecPosition2, "door start/end positions are equal");

	// Deixo o interior de LinearMoveDone() acessivel
	blockThink = false;
}

void CFuncParent::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// Movimento do func_parent
	LinearMove(pev, m_vecPosition2, speed);
	// Movimento dos parents
	parent01 = ProcessMovement(parent01, parent01_name);
	parent02 = ProcessMovement(parent02, parent02_name);
	parent03 = ProcessMovement(parent03, parent03_name);
	parent04 = ProcessMovement(parent04, parent04_name);
	parent05 = ProcessMovement(parent05, parent05_name);
	parent06 = ProcessMovement(parent06, parent06_name);
	parent07 = ProcessMovement(parent07, parent07_name);
	parent08 = ProcessMovement(parent08, parent08_name);
	parent09 = ProcessMovement(parent09, parent09_name);
	parent10 = ProcessMovement(parent10, parent10_name);
	parent11 = ProcessMovement(parent11, parent11_name);
	parent12 = ProcessMovement(parent12, parent12_name);
	parent13 = ProcessMovement(parent13, parent13_name);
	parent14 = ProcessMovement(parent14, parent14_name);
	parent15 = ProcessMovement(parent15, parent15_name);
	parent16 = ProcessMovement(parent16, parent16_name);
}

// Coordena as configuracoes e chamadas de movimento (eh a principal)
entvars_t* CFuncParent::ProcessMovement(entvars_t *parent, string_t targetName)
{
	parent = SetEntVars_t(targetName);
	if (parent != NULL)
	{
		parent->movetype = MOVETYPE_PUSH;
		LinearMove(parent, m_vecPosition2, speed);

		return parent;
	}

	return nullptr;
}

// Encontra entidades no mapa
entvars_t * CFuncParent::SetEntVars_t(string_t targetName)
{
	edict_t* pentTarget = NULL;

	pentTarget = FIND_ENTITY_BY_STRING(pentTarget, "targetname", STRING(targetName));

	if (FNullEnt(pentTarget))
		return NULL;
	else
		return &pentTarget->v;
}

// Efetivamente move as entidades
void CFuncParent::LinearMove(entvars_t *entity, Vector vecDest, float flSpeed)
{
	// Ja chegou?
	ASSERTSZ(flSpeed != 0, "LinearMove:  no speed is defined!");
	if (vecDest == entity->origin)
		return;

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDest - pev->origin;

	// divide vector length by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to LinearMoveDone when dest is reached
	entity->nextthink = entity->ltime + flTravelTime;
	if (entity == pev)
	{
		parent_time = entity->ltime + flTravelTime + wait;
		SetThink(&CFuncParent::LinearMoveDone);
	}

	// scale the destdelta vector by the time spent traveling to get velocity
	entity->velocity = vecDestDelta / flTravelTime;
}

void CFuncParent::LinearMoveDone(void)
{
	if (!blockThink)
	{
		// Paro a entidade e todos os parents
		if (pev->velocity.Length() > 0)
			pev->velocity = pev->velocity * 0;
		if (parent01)
			if (parent01->velocity.Length() > 0)
				parent01->velocity = parent01->velocity * 0;
		if (parent02)
			if (parent02->velocity.Length() > 0)
				parent02->velocity = parent02->velocity * 0;
		if (parent03)
			if (parent03->velocity.Length() > 0)
				parent03->velocity = parent03->velocity * 0;
		if (parent04)
			if (parent04->velocity.Length() > 0)
				parent04->velocity = parent04->velocity * 0;
		if (parent05)
			if (parent05->velocity.Length() > 0)
				parent05->velocity = parent05->velocity * 0;
		if (parent06)
			if (parent06->velocity.Length() > 0)
				parent06->velocity = parent06->velocity * 0;
		if (parent07)
			if (parent07->velocity.Length() > 0)
				parent07->velocity = parent07->velocity * 0;
		if (parent08)
			if (parent08->velocity.Length() > 0)
				parent08->velocity = parent08->velocity * 0;
		if (parent09)
			if (parent09->velocity.Length() > 0)
				parent09->velocity = parent09->velocity * 0;
		if (parent10)
			if (parent10->velocity.Length() > 0)
				parent10->velocity = parent10->velocity * 0;
		if (parent11)
			if (parent11->velocity.Length() > 0)
				parent11->velocity = parent11->velocity * 0;
		if (parent12)
			if (parent12->velocity.Length() > 0)
				parent12->velocity = parent12->velocity * 0;
		if (parent13)
			if (parent13->velocity.Length() > 0)
				parent13->velocity = parent13->velocity * 0;
		if (parent14)
			if (parent14->velocity.Length() > 0)
				parent14->velocity = parent14->velocity * 0;
		if (parent15)
			if (parent15->velocity.Length() > 0)
				parent15->velocity = parent15->velocity * 0;
		if (parent16)
			if (parent16->velocity.Length() > 0)
				parent16->velocity = parent16->velocity * 0;

		// Verifico se ela ja pode iniciar a proxima entidade (target)
		if (pev->ltime > parent_time)
		{
			blockThink = true;

			if (strcmp(STRING(pev->target), "") != 0)
				FireTargets(STRING(pev->target), m_hActivator, this, USE_TOGGLE, 0);
		}
		else
			// Se ainda ouver delay para contabilizar, continuo rodando essa funcao...
			pev->nextthink = pev->ltime + 0.1;
	}
}