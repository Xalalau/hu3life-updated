// ##############################
// HU3-LIFE REGRAS DO COOP
// ##############################

#define SPAWNPROTECTIONTIME 3;

/**
*	Minimum horizontal velocity that a player must be moving at before step sounds are played.
*   	Definido na base antiga
*/
#define PLAYER_STEP_SOUND_SPEED 220

class CBaseHalfLifeCoop : public CGameRules
{
public:
	CBaseHalfLifeCoop();

	void Think() override;
	void RefreshSkillData() override;
	bool IsAllowedToSpawn(CBaseEntity *pEntity) override;
	bool FAllowFlashlight() override;

	bool FShouldSwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) override;

// Functions to verify the single/multiplayer status of a game
	bool IsMultiplayer() override;
	bool IsDeathmatch() override;
	bool IsCoOp() override;

// Client connection/disconnection
	bool ClientConnected(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128]) override;
	void InitHUD(CBasePlayer *pl) override;
	void ClientDisconnected(edict_t *pClient) override;
	void UpdateGameMode(CBasePlayer *pPlayer) override; 

// Client damage rules
	float FlPlayerFallDamage(CBasePlayer *pPlayer) override;
	bool  FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity* pAttacker) override;

// Client spawn/respawn control
	void FixPlayerCrouchStuck(edict_t *pPlayer);
	bool StartCoopPlayer(CBaseEntity *pPlayer);
	bool LoadPlayerItems(CBasePlayer *pPlayer);
	void ChangeLevelCoopToogle();
	void ChangeLevelCoop();
	void SavePlayerItems(CBasePlayer *pPlayer);
	void SavePlayerItem(CBasePlayer *pPlayer, CBasePlayerWeapon *wepInfo, int j);

	void PlayerSpawn(CBasePlayer *pPlayer) override;
	void PlayerThink(CBasePlayer *pPlayer) override;
	bool FPlayerCanRespawn(CBasePlayer *pPlayer) override;
	float FlPlayerSpawnTime(CBasePlayer *pPlayer) override;
	edict_t* GetPlayerSpawnSpot(CBasePlayer* pPlayer) override;

	bool AllowAutoTargetCrosshair() override;
	bool ClientCommand(CBasePlayer *pPlayer, const char *pcmd) override;
	void ClientUserInfoChanged(CBasePlayer* pPlayer, char* infobuffer) override;

// Client kills/scoring
	int IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled) override;
	void PlayerKilled(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pInflictor) override;
	void DeathNotice(CBasePlayer* pVictim, entvars_t* pKiller, entvars_t* pInflictor) override;

// Weapon retrieval
	void PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) override;
	bool CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) override;

// Weapon spawn/respawn control
	int WeaponShouldRespawn(CBasePlayerItem *pWeapon) override;
	float FlWeaponRespawnTime(CBasePlayerItem *pWeapon) override;
	float FlWeaponTryRespawn(CBasePlayerItem *pWeapon) override;
	Vector VecWeaponRespawnSpot(CBasePlayerItem *pWeapon) override;
	
// Item retrieval
	bool CanHaveItem(CBasePlayer *pPlayer, CItem *pItem) override;
	void PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem) override;

// Item spawn/respawn control
	int ItemShouldRespawn(CItem *pItem) override;
	float FlItemRespawnTime(CItem *pItem) override;
	Vector VecItemRespawnSpot(CItem *pItem) override;

// Ammo retrieval
	void PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount) override;

// Ammo spawn/respawn control
	int AmmoShouldRespawn(CBasePlayerAmmo *pAmmo) override;
	float FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo) override;
	Vector VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo) override;

// Healthcharger respawn control
	float FlHealthChargerRechargeTime() override;
	float FlHEVChargerRechargeTime() override;

// What happens to a dead player's weapons
	int DeadPlayerWeapons(CBasePlayer *pPlayer) override;

// What happens to a dead player's ammo	
	int DeadPlayerAmmo(CBasePlayer *pPlayer) override;

// Teamplay stuff	
	const char *GetTeamID(CBaseEntity *pEntity) override { return ""; }
	int PlayerRelationship(CBaseEntity *pPlayer, CBaseEntity *pTarget) override;

	bool PlayTextureSounds() override { return false; }
	bool PlayFootstepSounds(CBasePlayer *pl, float fvol) override;

// Monsters
	bool FAllowMonsters() override;

protected:
	int ChangeLevelVolume();
	Vector GetPlySpawnPos(int i);
	void DisablePhysics(CBaseEntity *pEntity);
	void EnablePhysics(CBaseEntity *pEntity);
	float m_flIntermissionEndTime2;
	bool m_bEndIntermissionButtonHit2 = false;
	void SendMOTDToClient(CBasePlayer* pPlayer);
};
