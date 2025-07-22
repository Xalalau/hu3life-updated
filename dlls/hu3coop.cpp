// ##############################
// HU3-LIFE VARIAVEIS DO COOP
// ##############################
// Colocamos as variaveis do modo cooperativo em um arquivo separado
// para deixa-las acessiveis em outras partes do codigo mais facilmente

#include "mathlib.h"
#include "hu3coop.h"
#include "cdll_dll.h"

// Guardar as infos dos jogadores
struct playerCoopSaveRestore CoopPlyData[MAX_PLAYERS];

// Nome do landmark no changelevel ativado
char hu3LandmarkName[32] = "";
// Nome do proximo mapa
char hu3NextMap[32] = "";

// Liga o processamento de trigger_changelevel no think do modo coop
bool hu3ChangelevelCheck = false;

// Tempo mínimo de criação entre cada item do jogador durante respawn, em segundos
const float respawnItemInterval = 0.03;

// Nomes avacalhados do coop
char GENERIC_COOP_NAMES[121][MAX_PLAYER_NAME_LENGTH] = {
	"DeLadinho", // 0

	"Jamelao", "Jararaca", "Wallita", "Pereba", "Gandalf",
	"Hipoglos", "Gonorreia", "Gordon", "UNO", "Jabolane",  // 10

	"Galinacio", "Estupido", "Jirafa", "Gayetty", "Ceboso",
	"Cajado", "Galudo", "Bebum", "Anarcolixo", "Comulixo", // 20

	"Lalau", "Mesoclise", "Fuscazul", "FiDiKenga", "Birinbau",
	"Lampiao", "Danoninho", "SuaMae", "Gugu", "Torando", // 30

	"Amenopausa", "PutaRampeira", "Eressao", "Televisao", "Amendoim",
	"Ovohcito", "Polichop", "Gulosa", "Suvacao", "Bilau", // 40

	"Mindingu", "Gatuno", "Loirinha18", "Bostolhozo", "Fezaldo",
	"Trivaga", "Insignificante", "QueroQuero", "Leitinho", "Noçasenhoar", // 50

	"Arrombado", "Piperodaptiloh", "Pistolao", "Papaco", "Teocu",
	"Teopai", "Tuamae", "Cagapau", "Bizonho", "Pipistrelo", // 60

	"CeguetaParkson", "Metiolate", "NoobLixo", "XXXXPorn", "Ratazana",
	"Xoraum", "Isk8tero", "Acreman", "Sr.Peitos", "Sr.Pau", // 70

	"VerrugaNoOlho", "MercedesBens", "Otahrio", "GastriteAnal", "MC_Menor",
	"Aszideia", "Mohrbido", "Bozo", "CretinoLindo", "Desempregado12", // 80

	"CuDeFoca(o)", "Cotoco1cm", "Alargada", "VigarioEsquina", "1000otwo",
	"Gargarejo", "Kitinet", "Ser.Veja", "Dollynetx", "CafetaoGTA", // 90

	"Entalado", "BaianoSuave", "Bundao", "Hostil", "Nyes",
	"Quentinho","QuerAlho", "Castigador", "CHUTEME", "Bucetaldx", // 100

	"Monosselha", "Zarolha", "TonsDeCinza", "Sr.dos_anais", "HomemLaranja",
	"InutilMais1", "VirjaoMeAjuda", "SuasLagrimas", "ParideiraInterior", "Motumbo", // 110

	"Xalolau", "NickNBR", "Janks", "M4n0Crize", "Pipeu",
	"R4t0o", "DratVerder", "Zuzuo", "Pavambo", "Gaben" // 120
};