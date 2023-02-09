#pragma once
#ifndef WORLD_H
#define WORLD_H

#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "entity.h"

enum eCellType : uint8 {
	NADA,
	H1_CORNER_T_L,
	H1_CORNER_T_R,
	H1_CORNER_B_R,
	H1_CORNER_B_L,
	H1_CENTER,
	H1_SIDE_T,
	H1_SIDE_L,
	H1_SIDE_B,
	H1_SIDE_R,
	H1_WATER,
	H1_CUBE,
	COUNT
};

enum eItemType : uint8 {
	NOTHING,
	FENCE,
	COIN,
	HEART,
	ARROW_RIGHT,
	ARROW_LEFT,
	ARROW_UP,
	BOUNCER,
	SPIKES,
	TREE,
	FLAG,
	HOUSE,
	NUMBER_ITEMS
};

//enum eEnemiesType : uint8 {
//	NO_ENEMY,
//	WOLF,
//	CANNON,
//	NUMBER_ENEMIES
//};

enum eColissionType {
	OTHER,
	COLL_COIN,
	COLL_HEART,
	COLL_BOUNCER,
	COLL_FLAG,
	COLL_SPIKES
};

struct sCell {
	eCellType type;
	int height; 
	int item;
};

struct sCellPos {
	sCell cell;
	Vector2 pos;
};

//struct sProp {
//	Texture* texture;
//	Mesh* mesh;
//	Shader* shader;
//	//Matrix44 model;
//};

struct sMapHeader {
	int w; //width of map
	int h; //height of map
	unsigned char bytes; //num bytes per cell
	unsigned char extra[7]; //filling bytes, not used
};



//struct sPropViewData {
//	Mesh* mesh;
//	Texture* texture;
//};

//struct sItem {
//	eItemType type;
//	Vector2 pos;
//	int height;
//	Vector3 rot;
//};

class GameMap
{
public:

	int width;
	int height;
	sCell* data;
	uint8* navPath;

	float tileWidth = 1.0f;
	float tileHeight = 1.0f;
	float tileDepth = 1.0f;

	Vector2 spawnPlayerPos; 

	const char* itemsFileName; 
	const char* enemiesFileName;

	std::vector<std::vector<Matrix44>> cellModels;
	std::vector<std::vector<Matrix44>> itemModels;
	std::vector<std::vector<EntityEnemy*>> enemies;

	//functions
	GameMap();
	GameMap(int w, int h);

	// init
	void initNavegationPath();
	void initCellModels();
	void initItemModels();
	void initEnemies();
	
	// setters
	void setCell(int x, int y, eCellType type);

	//getters
	sCell& getCell(int x, int y);
	int getHeight(Vector3 worldPos);
	int getHeight(Vector2 cellPos);
	std::vector<sCellPos> getNeighbourCells(Vector3 worldPos);
	Vector3 getSpawnPlayerPos(); 

	// items
	bool eraseItem(Vector2 pos, int item);
	void spinItem(float seconds_elapsed, std::vector<Matrix44>& models);
	void spinAll(float seconds_elapsed);

	// loads
	static std::vector<GameMap*> loadLevels(const char* filename);
	static GameMap* loadGameMap(const char* filename, float tileWidth, float tileHeight, float tileDepth);
	void loadItems(const char* filename);
	void loadEnemies(const char* filename);

	//save
	std::vector<Vector2> getObjPos(int indexItem);

	//render
	void RenderMap(std::vector<EntityMesh*> viewDatas, std::vector<EntityMesh*>  itemEntities);
	void RenderEnemies(float time);

	//other
	void restartMap(); 
	Vector2 WorldToCell(Vector3 worldPos);
	Vector3 CellToWorld(Vector2 cellPos); 
	std::vector<Vector3> s(int indexItem);
	int coins;

};

#endif