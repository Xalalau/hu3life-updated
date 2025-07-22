#include "cdll_dll.h"

class CBaseEntity;
class CSprite;
class CBasePlayerWeapon;

using string_t = unsigned int;

#define MAX_PLAYER_NAME_LENGTH 32 // hud.h
#define MAX_WEAPON_NAME_LENGTH 32 // Inventei

//Guardar as informacoes dos players
struct playerCoopWeapons {
	CBasePlayerWeapon* weapon;
	char name[MAX_WEAPON_NAME_LENGTH];
	bool spawned;
	int clip;
};

struct playerCoopSaveRestore {
	Vector relPos;
	Vector v_angle;
	Vector punchangle;
	Vector velocity;
	Vector angles;
	bool newplayer; // Usada para diferenciar o primeiro spawn
	bool changinglevel; // Estado de mudança de nível terminada ou não
	bool waitingforchangelevel; // Serve para prender os jogadores numa posicao valida de changelevel
	bool respawning; // Estado de respawn do jogador após sua morte
	int weapons; // HUD
	int team;
	float armorvalue;
	bool bInDuck;
	bool flashlight;
	float health;
	float frags;
	float flFallVelocity;
	char pName[MAX_PLAYER_NAME_LENGTH];
	bool pSetNewName;
	bool godmode;
	bool notarget;
	bool noclip;
	bool hasSuit;
	bool respawncommands; // Uso isso para finalizar changelevels ou mortes do jogador
	float notSolidWait; // Tempo minimo que o jogador precisa levar no estado nao solido assim que spawna
	float respawnItemWait; // Tempo mínimo de criação entre cada item do jogador durante respawn, em segundos, somado a gpGlobals->time
	int hu3_cam_crosshair;
	struct playerCoopWeapons keepweapons[MAX_PLAYERS];
	int m_rgAmmo[MAX_PLAYERS][MAX_AMMO_SLOTS]; // Salva as municoes de cada jogador
};

// Guardar as infos dos jogadores
extern struct playerCoopSaveRestore CoopPlyData[MAX_PLAYERS];

// Nome do landmark no changelevel ativado
extern char hu3LandmarkName[32];
// Nome do proximo mapa
extern char hu3NextMap[32];

// Liga o processamento de trigger_changelevel no think do modo coop
extern bool hu3ChangelevelCheck;

// Tempo mínimo de criação entre cada item do jogador durante respawn, em segundos
extern const float respawnItemInterval;

// Nomes avacalhados do coop
extern char GENERIC_COOP_NAMES[121][MAX_PLAYER_NAME_LENGTH];