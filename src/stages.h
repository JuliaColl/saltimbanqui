#ifndef STAGES_H
#define STAGES_H

#include "camera.h"
#include "entity.h"
#include "world.h"


enum STAGE_ID {
	INTRO = 0,
	TUTORIAL = 1,
	PLAY = 2,
	END = 3
};

struct sLevel {
	Vector3 tileSize;
	std::string mapFileName;
	std::string itemFileName;
	std::string enemiesFileName;
	Vector2 spawnPlayer;
};

//enum eCollisionType {
//	OTHER,
//	FLOOR,
//	WALL,
//	ITEM
//};

enum eTutorialStage {
	TUTORIAL_MOVE,
	TUTORIAL_JUMP,
	TUTORIAL_BOUNCE,
	TUTORIAL_COIN,
	TUTORIAL_CANNON,
	TUTORIAL_HEART,
	TUTORIAL_SKELETON,
	TUTORIAL_END,
	TUTORIAL_FALL
};

enum eButtonsGui {
	NEW_GAME,
	LOAD_GAME,
	RESTART_GAME,
	SAVE_GAME,
	TUTOIAL,
	SETTINGS,
	CONTINUE,
	EXIT,

};

class Stage
{
public:
	// attributes
	bool static hasFinished;
	Texture* buttons; 
	static Texture* panel; 
	static STAGE_ID currentStage;
	static EntityMesh* ground;
	bool endLevel = false;
	static bool exit;

	//gui
	bool isHelp = false;
	bool isFlagNotActivated = false; 

	//levels
	static std::vector<sLevel*> levels;
	static int currentLevel;
	static std::string tutorialLevelFileName;


	// constructor
	Stage();
	Stage(const char* textureName);

	// virtual fucntions
	virtual STAGE_ID GetId() = 0;
	virtual void Render(Camera* camera, float time) = 0;
	virtual void Update(Camera* camera, float seconds_elapsed, bool mouse_locked, float mouse_speed, float angle) = 0;
	// GUI
	virtual void RenderAllGui() = 0;

	// other functions
	void setButtonsTexture(const char* textureName);
	void static loadMeshEntity(EntityMesh* entity, const char* meshName, const char* textureName);
	void AddEntityInFront(Camera* cam, const char* pathMesh, const char* pathTexture);
	void restartGame();
	void spawnPlayer();
	void playerVelGround();
	Vector3 getSpawnPos();
	void RenderSky(Camera* camera);
	GameMap* getCurrentMap();


	// initilize
	void InitEntities();
	void InitMap();
	void InitLevel();
	void InitLevelTutorial(const char* filename);

	//levels
	sLevel* GetLevel(int id);
	sLevel* GetCurrentLevel();
	void loadLevels(const char* filename);
	void changeLevel(int level = currentLevel);

	// player
	void updatePlayerPos(float elapsed_time);
	bool checkRespawnPlayer();

	//collisions
	bool checkCollisionWithCell(EntityMesh* cell, Vector3 character_center, int& countFloor, bool& isFloor, Vector3& coll, Vector3& collnorm);
	bool checkCollisionWithItem(EntityMesh* item, Vector3 character_center, int& itemType, eColissionType& collType, Vector3& coll, Vector3& collnorm);
	Vector3 pushAway(Vector3 prevPos, Vector3 coll, Vector3 character_center, float elapsed_time);
	Vector3 checkColision(float elapsed_time, Vector3 nextPos, Vector3 prevPos);
	Vector3 checkColisionWithEnemies(float elapsed_time, Vector3 nextPos, Vector3 prevPos);

	// items
	void updateCoins(sCellPos cell);
	void updateHearts(sCellPos cell);
	void bouncerTouched();
	void endFlag();
	Vector3 spikesTouched();
};

class IntroStage : public Stage {
public:
	//gui
	bool isSettings = false;
	EntityWolf* skeleton;
	EntityCannon* cannon;
	EntityMesh* cube;
	EntityMesh* plane;

	// constructor
	IntroStage() {};
	IntroStage(const char* textName) : Stage(textName) {};

	STAGE_ID GetId();
	void Render(Camera* camera, float time);
	void Update(Camera* camera, float seconds_elapsed, bool mouse_locked, float mouse_speed, float angle);

	// GUI
	void RenderAllGui(); 

	//load
	void loadGame();
	void eraseItems(std::vector<Vector2> posToErase, int type);
	void settings();
};

class PlayStage : public Stage {
public:

	// constructor
	PlayStage() {};
	PlayStage(const char* textName) : Stage(textName) {};

	STAGE_ID GetId();
	
	//render
	void Render(Camera* camera, float time);
	
	//update
	void Update(Camera* camera, float seconds_elapsed, bool mouse_locked, float mouse_speed, float angle);

	//other functions
	int getCurrentHeight(Vector3 pos, GameMap* map);
	
	// GUI
	void RenderAllGui();
	void RenderHelpGui();
	
	// Pathfinding
	static void init(Vector3 spawnPos);
	static void path(Vector3 spawnPos);

	//save
	void saveGame();

};

class TutorialStage : public Stage {
public:
	eTutorialStage tutorialStage = eTutorialStage::TUTORIAL_MOVE;

	// constructor
	TutorialStage() {};
	TutorialStage(const char* textName) : Stage(textName) {};

	STAGE_ID GetId();
	void Render(Camera* camera, float time);
	void Update(Camera* camera, float seconds_elapsed, bool mouse_locked, float mouse_speed, float angle);
	void updateTutorialStage(); 
	// GUI
	void RenderAllGui();
	void RenderHelpGui();
	void RenderGuiMove();
	void RenderGuiJump();
	void RenderGuiBounce();
	void RenderGuiCoin();
	void RenderGuiCannon();
	void RenderGuiHeart();
	void RenderGuiSkeleton();
	void RenderGuiEnd();
	
};


class EndStage : public Stage {
public:
	
	// constructor
	EndStage(){};
	EndStage(const char* textName) : Stage(textName) {};
	
	STAGE_ID GetId();
	void Render(Camera* camera, float time);
	void Update(Camera* camera, float seconds_elapsed, bool mouse_locked, float mouse_speed, float angle);
	// GUI
	void RenderAllGui();
	void RenderExitGui();
	void RenderLoseGui();
	void RenderWinGui();
};

#endif