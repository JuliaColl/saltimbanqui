#include "stages.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "game.h"
#include "gui.h"
#include "pathfinders.h"

#include <iostream>
#include <fstream> 
#include <bass.h>

//VARIABLES
bool cameraLocked = true;  //no bloquear la camera 
int respawnHeight = -15; 

//Entities
EntityMesh* sky = NULL;
EntityPlayer* player = NULL;
std::vector<EntityMesh*> entities;
std::vector<EntityMesh*> static_entities;

//MAPA
GameMap* tutorial_gamemap;
GameMap* gamemap;
std::vector<EntityMesh*> baseMap;
std::vector<EntityMesh*> itemMap;

// gui
Texture* help;
Texture* title;

// levels
std::vector<sLevel*> Stage::levels;
int Stage::currentLevel;
std::string Stage::tutorialLevelFileName;




Stage::Stage()
{
	buttons = NULL;
}

Stage::Stage(const char* textureName)
{
	setButtonsTexture(textureName);
}

void Stage::setButtonsTexture(const char* textureName)
{
	Texture* tex = Texture::Get(textureName);
	buttons = tex;
}

void Stage::loadMeshEntity(EntityMesh* entity, const char* meshName, const char* textureName)
{
	entity->mesh = Mesh::Get(meshName);
	entity->texture = Texture::Get(textureName);
	entity->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
}

void Stage::AddEntityInFront(Camera* cam, const char* pathMesh, const char* pathTexture)
{
	Vector2 mouse = Input::mouse_position;
	Game* g = Game::instance;
	Vector3 dir = cam->getRayDirection(mouse.x, mouse.y, g->window_width, g->window_height);
	Vector3 rayOrigin = cam->eye;

	Vector3 spawnPos = RayPlaneCollision(Vector3(), Vector3(0, 1, 0), rayOrigin, dir);

	EntityMesh* entity = new EntityMesh();
	loadMeshEntity(entity, pathMesh, pathTexture);
	entity->model.translate(spawnPos.x, spawnPos.y, spawnPos.z);

	entities.push_back(entity);
}

void Stage::restartGame()
{
	player->restart();
	spawnPlayer();

	changeLevel();
	hasFinished = false;
}

void Stage::playerVelGround(){
	player->vel = Vector3();
	player->isGround = true;
}

void Stage::spawnPlayer()
{
	isFlagNotActivated = false;
	Vector3 spawnPos = getSpawnPos();
	player->setPos(spawnPos);
	playerVelGround();
}

Vector3 Stage::getSpawnPos()
{
	GameMap* currentMap = getCurrentMap();
	Vector3 spawnPos = currentMap->getSpawnPlayerPos();
	spawnPos.y = spawnPos.y + player->mesh->radius;
	return spawnPos;
}

void Stage::RenderSky(Camera* camera)
{
	Matrix44 skymodel;
	skymodel.translate(camera->eye.x, camera->eye.y - 40.0f, camera->eye.z);
	sky->model = skymodel;

	glDisable(GL_DEPTH_TEST);
	sky->render();
	glEnable(GL_DEPTH_TEST);

	/*skymodel.translate(camera->eye.x, camera->eye.y + 40.0f, camera->eye.z);
	skymodel.scale(1, -1, 1);
	sky->model = skymodel;

	glDisable(GL_DEPTH_TEST);
	sky->render();
	glEnable(GL_DEPTH_TEST);*/
}

// levels
sLevel* Stage::GetLevel(int id) {
	return levels[id];
}

sLevel* Stage::GetCurrentLevel() {
	return GetLevel(currentLevel);
}

GameMap* Stage::getCurrentMap()
{
	GameMap* currentMap;
	if (currentStage == STAGE_ID::TUTORIAL) {
		currentMap = tutorial_gamemap;
	}
	else {
		currentMap = gamemap;
	}
	return currentMap;
}

// initilize 
void Stage::InitEntities()
{
	//sky
	sky = new EntityMesh("data/cielo.ASE", "data/cielo.tga");
	//loadMeshEntity(sky, "data/cielo.ASE", "data/cielo.tga");

	//player
	player = new EntityPlayer("data/ball.obj", "data/texture.tga");
	player->yaw = 120.0f;
	player->pos = player->model.getTranslation();

	//map
	baseMap.reserve(eCellType::COUNT);
	baseMap.push_back(new EntityMesh());
	baseMap.push_back(new EntityMesh("data/cubes/grass/cube_grass_corner_topLeft.obj", "data/cubes/grass/cube_grass_corner.png"));
	baseMap.push_back(new EntityMesh("data/cubes/grass/cube_grass_corner_topRight.obj", "data/cubes/grass/cube_grass_corner.png"));
	baseMap.push_back(new EntityMesh("data/cubes/grass/cube_grass_corner_bottomRight.obj", "data/cubes/grass/cube_grass_corner.png"));
	baseMap.push_back(new EntityMesh("data/cubes/grass/cube_grass_corner_bottomLeft.obj", "data/cubes/grass/cube_grass_corner.png"));
	baseMap.push_back(new EntityMesh("data/cubes/grass/cube_grass_center.obj", "data/cubes/grass/cube_grass_center.png"));
	baseMap.push_back(new EntityMesh("data/cubes/grass/cube_grass_side_top.obj", "data/cubes/grass/cube_grass_side.png"));
	baseMap.push_back(new EntityMesh("data/cubes/grass/cube_grass_side_left.obj", "data/cubes/grass/cube_grass_side.png"));
	baseMap.push_back(new EntityMesh("data/cubes/grass/cube_grass_side_bottom.obj", "data/cubes/grass/cube_grass_side.png"));
	baseMap.push_back(new EntityMesh("data/cubes/grass/cube_grass_side_right.obj", "data/cubes/grass/cube_grass_side.png"));
	baseMap.push_back(new EntityMesh("data/water.obj", "data/water.png"));
	baseMap.push_back(new EntityMesh("data/cubes/grass/cube_grass.obj", "data/cubes/grass/cube_grass.png"));

	//items
	itemMap.reserve(eItemType::NUMBER_ITEMS);
	itemMap.push_back(new EntityMesh());
	itemMap.push_back(new EntityMesh("data/items/fence.obj", "data/items/fence.png"));
	itemMap.push_back(new EntityMesh("data/items/coin.obj", "data/items/coin.png"));
	itemMap.push_back(new EntityMesh("data/items/heart.obj", "data/items/heart.png"));
	itemMap.push_back(new EntityMesh("data/items/arrow_right.obj", "data/items/arrow_side.png"));
	itemMap.push_back(new EntityMesh("data/items/arrow_left.obj", "data/items/arrow_side.png"));
	itemMap.push_back(new EntityMesh("data/items/arrow_up.obj", "data/items/arrow_up.png"));
	itemMap.push_back(new EntityMesh("data/items/bouncer.obj", "data/items/bouncer.png"));
	itemMap.push_back(new EntityMesh("data/items/spikes.obj", "data/items/spikes.png"));
	itemMap.push_back(new EntityMesh("data/items/oak.obj", "data/items/oak.png"));
	itemMap.push_back(new EntityMesh("data/items/endFlag.obj", "data/items/endFlag.png"));
	itemMap.push_back(new EntityMesh());

	// gui 
	help = Texture::Get("data/gui/help_black.png");
	title = Texture::Get("data/gui/saltimbanqui.png");

	// skeleton animation
	EntityWolf::animCombat = Animation::Get("data/enemies/skeleton_running.skanim");
	EntityWolf::animIdle = Animation::Get("data/enemies/skeleton_idle.skanim");

	// ground
	Mesh* groundMesh = new Mesh();
	groundMesh->createPlane(1000);
	ground->mesh = groundMesh;
	ground->texture = Texture::Get("data/blue.png");
	ground->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	ground->model.setTranslation(0, -20, 0);
}

void Stage::InitMap()
{
	const char* fileMap = &(GetCurrentLevel()->mapFileName[0]);
	const char* fileItems = &(GetCurrentLevel()->itemFileName)[0];
	const char* fileEnemies = &(GetCurrentLevel()->enemiesFileName)[0];
	Vector3 tileSize = GetCurrentLevel()->tileSize;
	Vector2 spawnPos = GetCurrentLevel()->spawnPlayer;
	
	gamemap = GameMap::loadGameMap(fileMap, tileSize.x, tileSize.y, tileSize.y);
	gamemap->spawnPlayerPos = spawnPos;
	gamemap->loadItems(fileItems);
	gamemap->loadEnemies(fileEnemies);
	gamemap->coins = gamemap->itemModels[(int)eItemType::COIN].size();

}

void Stage::InitLevel()
{
	InitMap();
	spawnPlayer();
}

void Stage::InitLevelTutorial(const char* filename)
{

	std::string content = "";
	readFile(filename, content);
	std::stringstream ss(content);

	// file name
	ss >> Stage::tutorialLevelFileName;

	// tile size
	int widht, heigh, depth;
	ss >> widht >> heigh >> depth;


	// spawn postion
	int SpawnPos_x, SpawnPos_y;
	ss >> SpawnPos_x >> SpawnPos_y;

	//file names
	std::string map;
	std::string items;
	std::string enemies;

	ss >> map >> items >> enemies;

	const char* mapFileName = &map[0];
	const char* itemFileName = &items[0];
	const char* enemiesFileName = &enemies[0];

	// load map
	Vector2 spawnPos = Vector2(SpawnPos_x, SpawnPos_y);

	tutorial_gamemap = GameMap::loadGameMap(mapFileName, widht, heigh, depth);
	tutorial_gamemap->spawnPlayerPos = spawnPos;
	tutorial_gamemap->loadItems(itemFileName);
	tutorial_gamemap->loadEnemies(enemiesFileName);
	tutorial_gamemap->coins = tutorial_gamemap->itemModels[(int)eItemType::COIN].size();

	// restart player
	player->restart();
	Vector3 pos = tutorial_gamemap->getSpawnPlayerPos();
	pos.y = pos.y + player->mesh->radius;
	player->setPosRot(pos, 90);
}


void Stage::loadLevels(const char* filename) {

	std::string content = "";
	readFile(filename, content);
	std::stringstream ss(content);
	int numberOfLevels;
	
	ss >> numberOfLevels;
	levels.reserve(numberOfLevels);

	for (int i = 0; i < numberOfLevels; i++){
		levels.push_back(new sLevel);
		 
		// tile size
		int widht, heigh, depth;
		ss >> widht >> heigh >> depth;
		levels[i]->tileSize.x = widht;
		levels[i]->tileSize.y = heigh;
		levels[i]->tileSize.z = depth;

		// spawn postion
		int x, y;
		ss >> x >> y;
		levels[i]->spawnPlayer.x = x;
		levels[i]->spawnPlayer.y = y;
		 
		std::string map;
		std::string items;
		std::string enemies;

		ss >> map >> items >> enemies;

		levels[i]->mapFileName = map;
		levels[i]->itemFileName = items;
		levels[i]->enemiesFileName = enemies;
		
	}
}

void Stage::changeLevel(int level)
{
	currentLevel = level;
	delete(gamemap);
	InitLevel();
	spawnPlayer();
	player->coins = 0;
}

void Stage::updatePlayerPos(float elapsed_time)
{
	// compute player next position
	Vector3 nextPos = player->nextPos(elapsed_time);

	// check if the player collides with a cell or an item and return the next postion after the collition
	nextPos = checkColision(elapsed_time, nextPos, player->pos);
	
	// if hasFinished
	if (hasFinished) return; 

	// if it has end the current level
	GameMap* currentMap = getCurrentMap();
	if (endLevel) {
		if (currentStage != STAGE_ID::TUTORIAL) changeLevel();
		spawnPlayer();
		isFlagNotActivated = false;
		endLevel = false;
		return;
	}

	// check collisions with enemies and return the next position after the collition
	nextPos = checkColisionWithEnemies(elapsed_time, nextPos, player->pos);
	
	// if the player is in a cell of type nothing (no cell), it has to fall (to avoid getting stuck)
	Vector2 cellPos = currentMap->WorldToCell(nextPos);
	if (currentMap->getCell(cellPos.x, cellPos.y).type == eCellType::NADA) {
		nextPos.y -= elapsed_time;
	}

	// set next position of the player
	player->setPos(nextPos);

	// check if it has to respawn (when it falls)
	if (checkRespawnPlayer()) spawnPlayer();
}

bool Stage::checkCollisionWithCell(EntityMesh* cell, Vector3 character_center, int& countFloor, bool& isFloor, Vector3& coll, Vector3& collnorm) {
	
	GameMap* currentMap = getCurrentMap();
	
	if (cell->mesh->testSphereCollision(cell->model, character_center, 0.5, coll, collnorm) == false) return false;
	int height = currentMap->getHeight(character_center);
	int playerHeight = character_center.y - player->mesh->radius;
	//if (coll.y < height.y) isFloor = true;
	if (coll.y < playerHeight) isFloor = true;
	if (collnorm.x == 0 && collnorm.z == 0)	isFloor = true;
	if (isFloor) {
		player->vel.y = 0;
		countFloor++;
	}

	return true;
}

bool Stage::checkCollisionWithItem(EntityMesh* item, Vector3 character_center, int& itemType, eColissionType& collType, Vector3& coll, Vector3& collnorm) {

	if (item->mesh->testSphereCollision(item->model, character_center, 0.5, coll, collnorm) == false) return false;
	switch (itemType) {
	case eItemType::COIN: collType = eColissionType::COLL_COIN; break;
	case eItemType::HEART: collType = eColissionType::COLL_HEART; break;
	case eItemType::BOUNCER: collType = eColissionType::COLL_BOUNCER; break;
	case eItemType::FLAG: collType = eColissionType::COLL_FLAG; break;
	case eItemType::SPIKES: collType = eColissionType::COLL_SPIKES; break;
	}

	return true;
}


Vector3 Stage::pushAway(Vector3 prevPos, Vector3 coll, Vector3 character_center, float elapsed_time) {
	Vector3 push_away = normalize(coll - character_center) * elapsed_time;
	push_away.y = 0;
	return (prevPos - push_away); //move to previous pos but a little bit further
}

Vector3 Stage::checkColision(float elapsed_time, Vector3 nextPos, Vector3 prevPos)
{
	GameMap* currentMap = getCurrentMap();

	//calculamos el centro de la esfera de colisión del player elevandola hasta la cintura
	Vector3 character_center = nextPos + Vector3(0, player->mesh->radius, 0);

	std::vector<sCellPos> neighbourCells = currentMap->getNeighbourCells(prevPos);

	int countFloor = 0;

	//para las cells al rededor del player
	for (auto& c : neighbourCells) {
		sCellPos cell = c;
		int index = cell.cell.type;
		int height = cell.cell.height;
		int item = cell.cell.item;
		Vector2 cellPos = cell.pos;

		if (index == eCellType::NADA) continue;

		EntityMesh* currentCell = new EntityMesh(baseMap[index]->mesh, baseMap[index]->texture, baseMap[index]->shader);
		currentCell->model.translate(cellPos.x * currentMap->tileWidth, height * currentMap->tileHeight, cellPos.y * currentMap->tileDepth);

		// collisions with cells
		Vector3 collCell;
		Vector3 collnormCell;
		bool isFloor = false;
		bool cellColl = checkCollisionWithCell(currentCell, character_center, countFloor, isFloor, collCell, collnormCell);


		// collisions with items
		Vector3 collItem;
		Vector3 collnormItem;
		bool itemColl = false;
		eColissionType collType = eColissionType::OTHER;

		if (item != (int)eItemType::NOTHING) {
			EntityMesh* currentItem = new EntityMesh(itemMap[item]->mesh, itemMap[item]->texture, itemMap[item]->shader);
			currentItem->model.translate(cellPos.x * currentMap->tileWidth, height * currentMap->tileHeight, cellPos.y * currentMap->tileDepth);
			itemColl = checkCollisionWithItem(currentItem, character_center, item, collType, collItem, collnormItem);
			delete(currentItem);
		}

		// si no hay colision con nada
		if (cellColl == false && itemColl == false) continue;

		// si hay colision solo con un item (que no sea una moneda/bouncer/heart)
		if (cellColl == false && itemColl == true) {
			if (collType == eColissionType::OTHER) nextPos = pushAway(prevPos, collItem, character_center, elapsed_time);
		}

		// si hay colision solo con la cell
		else if (cellColl == true && itemColl == false) {
			if (isFloor) nextPos.y = prevPos.y;  // si choca contra el suelo solo volvemos a la posicion anterior en el eje vertical
			else nextPos = pushAway(prevPos, collCell, character_center, elapsed_time); // si la esfera está colisionando la movemos a su posicion anterior alejandola del objeto
		}

		// si hay colision con la cell y un item (que no sea una moneda/bouncer/heart)
		else if (cellColl == true && itemColl == true) {
			if (collType == eColissionType::OTHER) nextPos = pushAway(prevPos, collItem, character_center, elapsed_time);
			if (isFloor) nextPos.y = prevPos.y;
		}

		
		// if we collided with a coin/bounce/heart/spike/flag
		switch (collType) {
		case eColissionType::COLL_COIN: updateCoins(cell); break;
		case eColissionType::COLL_HEART: updateHearts(cell); break;
		case eColissionType::COLL_BOUNCER: bouncerTouched(); break;
		case eColissionType::COLL_FLAG: endFlag(); return nextPos; break;
		case eColissionType::COLL_SPIKES: {
			nextPos = spikesTouched();
			break;
		}

		}
		delete(currentCell);
	}
	player->isGround = (countFloor != 0);


	return nextPos;
}

Vector3 Stage::checkColisionWithEnemies(float elapsed_time, Vector3 nextPos, Vector3 prevPos) {

	GameMap* currentMap = getCurrentMap();

	//calculamos el centro de la esfera de colisión del player elevandola hasta la cintura
	Vector3 character_center = nextPos + Vector3(0, player->mesh->radius, 0);

	for (int i = 0; i < ENEMY::NUMBER_ENEMIES; i++)
	{
		if (currentMap->enemies[i].size() <= 0) continue;
		for (auto& enemy : currentMap->enemies[i]) {

			//si el cañon esta disparando, checkear collision con las balas
			if (enemy->type == ENEMY::CANNON) {
				if (((EntityCannon*)enemy)->isShooting) {
					if (((EntityCannon*)enemy)->checkCollisionWithBullets(character_center)) {
						player->subtractLives();
						spawnPlayer();
						return getSpawnPos();
					}
				}
			}

			Vector3 coll;
			Vector3 collnorm;
			//comprobamos si colisiona el objeto con la esfera (radio 3)
			if (enemy->mesh->testSphereCollision(enemy->model, character_center, 0.5, coll, collnorm) == false)
				continue; //si no colisiona, pasamos al siguiente objeto

			// si colisiona con un wolf pierde una vida
			if (enemy->type == ENEMY::WOLF) {

				player->subtractLives();
				spawnPlayer();
				return getSpawnPos();
			}

			//si la esfera está colisionando muevela a su posicion anterior alejandola del objeto
			nextPos = pushAway(prevPos, coll, character_center, elapsed_time); //move to previous pos but a little bit further

			//cuidado con la Y, si nuestro juego es 2D la ponemos a 0
			nextPos.y = prevPos.y;
		}
	}

	return nextPos;
}


void Stage::updateCoins(sCellPos cell)
{
	GameMap* currentMap = getCurrentMap();

	if (currentMap->eraseItem(cell.pos, (int)eItemType::COIN)) player->updatePoints();
}

void Stage::updateHearts(sCellPos cell)
{
	GameMap* currentMap = getCurrentMap();

	if (currentMap->eraseItem(cell.pos, (int)eItemType::HEART)) player->addLives();
}


void Stage::bouncerTouched()
{
	player->isBouncing = true;
}

void Stage::endFlag()
{
	
	
	if (currentStage == STAGE_ID::TUTORIAL) {
		currentStage = STAGE_ID::INTRO;
		delete(tutorial_gamemap);
		const char* filename = &tutorialLevelFileName[0];
		InitLevelTutorial(filename);
		endLevel = true;
		return;
	}

	if (player->coins != gamemap->coins) {
		isFlagNotActivated = true;
		return;
	}

	currentLevel++;
	if (currentLevel >= levels.size()) {
		currentStage = STAGE_ID::END;
		hasFinished = true;
		endLevel = true;
	}
	else {
		endLevel = true;
	}

}

Vector3 Stage::spikesTouched() {
	player->subtractLives();
	return getSpawnPos();
}

bool Stage::checkRespawnPlayer()
{
	if (player->pos.y < respawnHeight) {
		player->subtractLives();
		return true;
	}
	return false;
}


// ----------------------------------Intro stage-------------------------------------
STAGE_ID IntroStage::GetId() { return STAGE_ID::INTRO; }


void IntroStage::Render(Camera* camera, float time)
{
	Vector3 eye = Vector3(0.0f, 1.0f, 1.0f);
	Vector3 center = Vector3(0.0f, 0.0f, -2.0f);
	Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
	camera->lookAt(eye, center, up);


	RenderSky(camera);
	plane->render();
	skeleton->setPosRot(Vector3(-3,0,-4), 150 );
	skeleton->render(time); 

	cannon->setPosRot(Vector3(2.5,0,-3), 85);
	cannon->render();

	RenderAllGui();
}


void IntroStage::Update(Camera* camera, float seconds_elapsed, bool mouse_locked, float mouse_speed, float angle){

}

void IntroStage::RenderAllGui()
{
	//RenderAllGui
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// title
	Vector4 textureRange = Vector4(0, 0, 1, 1);
	Vector4 buttonRange = Vector4(400, 100, 400, 100);
	GUI::RenderGui(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, title, textureRange);

	// new game button
	textureRange = Vector4(0, 0, 0.5, 0.25);
	if (GUI::RenderButton(450, 240, 400, 100, buttons, textureRange)) {
		InitLevel();
		player->restart(); 
		spawnPlayer();
		currentStage = STAGE_ID::PLAY;
		currentLevel = 0; 
	}

	// load game button
	textureRange = Vector4(0, 0.25, 0.5, 0.25);
	if (GUI::RenderButton(450, 340, 400, 100, buttons, textureRange)) {
		loadGame();
	}
		
	//tutorial button
	textureRange = Vector4(0.5, 0.75, 0.5, 0.25);
	if (GUI::RenderButton(450, 440, 400, 100, buttons, textureRange)) {
		currentStage = STAGE_ID::TUTORIAL;
		spawnPlayer();
	}

	// EXIT button
	textureRange = Vector4(0.5, 0.5, 0.5, 0.25);
	if (GUI::RenderButton(450, 540, 400, 100, buttons, textureRange)) {
		exit = true;

	}

	Input::wasLeftMousePressed = false;
}

void IntroStage::loadGame() {
	FILE* f = fopen("data/levels/game.txt", "rb");
	if (f == NULL)
		return;
	int level, lives, coins;
	float posx, posy, posz;

	fscanf(f, "%d", &level);
	fscanf(f, "%f %f %f", &posx, &posy, &posz);
	fscanf(f, "%d %d", &lives, &coins);

	int numC;
	fscanf(f, "\nCoins: %d", &numC);
	std::vector<Vector2> posC;
	posC.reserve(numC);
	for (int i = 0; i < numC; i++) {
		Vector2 temp;
		fscanf(f, "%f %f", &temp.x, &temp.y);
		posC.push_back(temp);
	}

	int numH;
	fscanf(f, "\nLives: %d", &numH);
	std::vector<Vector2> posH;
	posH.reserve(numH);
	for (int i = 0; i < numH; i++) {
		Vector2 temp;
		fscanf(f, "%f %f", &temp.x, &temp.y);
		posH.push_back(temp);
	}
	fclose(f);

	currentLevel = level;

	InitMap();

	playerVelGround();

	player->coins = coins;
	player->lives = lives;

	Vector3 posWorld = Vector3(posx, posy, posz);
	player->setPos(posWorld);

	eraseItems(posC, (int)eItemType::COIN);
	eraseItems(posH, (int)eItemType::HEART);

	currentStage = STAGE_ID::PLAY;
}

void IntroStage::eraseItems(std::vector<Vector2> posNotToErase, int type) {
	std::vector<Vector2> posToErase;
	for (auto& i : gamemap->itemModels[type]) {
		Matrix44 m = i;
		Vector3 pos = m.getTranslation();
		bool inFile = false;

		for (auto& t : posNotToErase) {
			if (t.x == pos.x && t.y == pos.z) {
				inFile = true;
			}
		}

		if (inFile == false) {
			Vector2 erasePos = gamemap->WorldToCell(pos);
			posToErase.push_back(erasePos);
		}
	}
	for (auto& i : posToErase) {
		gamemap->eraseItem(i, type);
	}
}

// -------------------------------------Play Stage----------------------------------------------------------------
STAGE_ID PlayStage::GetId() { return STAGE_ID::PLAY; }


void PlayStage::Render(Camera* camera, float time)
{
	RenderSky(camera);
	ground->render();

	if (cameraLocked) {
		Vector3 eye = player->model * Vector3(0.0f, 2.0f, 2.0f);
		Vector3 center = player->model * Vector3(0.0f, 0.0f, -2.0f);
		Vector3 up = player->model.rotateVector(Vector3(0.0f, 1.0f, 0.0f)); //con las models puedes hacer muchas cosas
		camera->lookAt(eye, center, up);  //hacer lerps no look at (hacer una entity camara?) ir transformando la camara haciendo interpolaciones
	}

	//Render map
	gamemap->RenderMap(baseMap, itemMap);

	// player
	player->render(true);

	// enemies
	gamemap->RenderEnemies(time);
	
	RenderAllGui();
}

void PlayStage::Update(Camera* camera, float seconds_elapsed, bool mouse_locked, float mouse_speed, float angle)
{	
	if (isHelp) return; 

	float speed = seconds_elapsed * mouse_speed; //the speed is defined by the seconds_elapsed so it goes constant

	//mouse input to rotate the cam
	if ((Input::mouse_state & SDL_BUTTON_LEFT) || mouse_locked) //is left button pressed?
	{
		camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f, -1.0f, 0.0f));
		camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));
	}

	if (Input::wasKeyPressed(SDL_SCANCODE_TAB))
	{
		cameraLocked = !cameraLocked;
	}

	if (cameraLocked)
	{	
		// player
		updatePlayerPos(seconds_elapsed);
		if(!player->isAlive()) currentStage = STAGE_ID::END;


		// enemies
		for (auto& enemies : gamemap->enemies) {
			for (auto& e : enemies) {
				e->updateEnemy(player->pos, seconds_elapsed);
			}
		}

		// animations coins
		gamemap->spinAll(seconds_elapsed);
	}
	else {
		//async input to move the camera around
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 3; //move faster with left shift
		if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
	}

	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();
}

int PlayStage::getCurrentHeight(Vector3 pos, GameMap* map)
{
	return map->getHeight(pos);
}


void PlayStage::RenderAllGui()
{
	//RenderAllGui
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (isHelp) {
		RenderHelpGui();
		Input::wasLeftMousePressed = false;
		return;
	}

	// count coins
	Vector4 textureRange = Vector4(0, 0, 0.125, 0.25);
	Vector4 buttonRange = Vector4(20, 50, 20, 20);
	GUI::RenderGui(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange);

	// count lives
	int dist = 25;
	for (int i = 0; i < player->lives; i++) {
		textureRange = Vector4(0.125, 0, 0.125, 0.25);
		buttonRange = Vector4(20 + i * dist, 20, 25, 25);
		GUI::RenderGui(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange);
	}

	// help button
	textureRange = Vector4(0, 0, 1, 1);
	buttonRange = Vector4(Game::instance->window_width - 20, 20, 30, 30);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, help, textureRange)) {
		isHelp = true;
	}

	// hay que hacerlo al final --> no mover
	char numCoins[10];
	sprintf(numCoins, "%d", player->coins);
	drawText(40, 42, numCoins, Vector3(1, 1, 1), 2);

	if (isFlagNotActivated) {
		drawText(180, 120, "Collect all the coins", Vector3(1, 1, 1), 4);
	}

	Input::wasLeftMousePressed = false;
}

void PlayStage::RenderHelpGui()
{
	// panel 
	Vector4 textureRange = Vector4(0, 0, 1, 1);
	Vector4 buttonRange = Vector4(400, 300, 400, 500);
	GUI::RenderGui(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, panel, textureRange);

	//exit button
	textureRange = Vector4(0.5, 0.25, 0.125, 0.25);
	buttonRange = Vector4(560, 105, 50, 50);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange)) {
		isHelp = false;
	}

	//restart game button
	textureRange = Vector4(0, 0.5, 0.4, 0.19);
	buttonRange = Vector4(410, 150, 250, 55);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange)) {
		restartGame();
		currentStage = STAGE_ID::PLAY;
		isHelp = false;
	}

	//save game button
	textureRange = Vector4(0, 0.75, 0.4, 0.19);
	buttonRange = Vector4(410, 250, 250, 55);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange)) {
		saveGame();
		isHelp = false;
	}

	// tutorial button
	textureRange = Vector4(0.5, 0.75, 0.4, 0.19);
	buttonRange = Vector4(410, 350, 250, 55);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange)) {
		currentStage = STAGE_ID::TUTORIAL;
		spawnPlayer();
		player->restart();
		isHelp = false;
	}

	// exit button
	textureRange = Vector4(0.5, 0.5, 0.4, 0.19);
	buttonRange = Vector4(410, 450, 250, 55);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange)) {
		currentStage = STAGE_ID::END;
		isHelp = false;
	}
}


void PlayStage::saveGame() {
	std::ofstream gameFile;
	gameFile.open("data/levels/game.txt", std::ofstream::out | std::ofstream::trunc);

	int levelS = currentLevel;

	float posx = player->pos.x;
	float posy = player->pos.y;
	float posz = player->pos.z;

	int coinsS = player->coins;

	int livesS = player->lives;

	//player
	std::stringstream strLevel(levelS);

	//pos
	std::stringstream strPosX(posx);
	std::stringstream strPosY(posy);

	//lives
	std::stringstream strlivesS(livesS);

	//coins
	std::stringstream strCoinS(coinsS);
	
	gameFile << levelS << std::endl;
	gameFile << posx << " " << posy << " " << posz << std::endl;
	gameFile << livesS << " " << coinsS << std::endl;

	//non-static enemies
	gameFile << "Coins: ";
	//coins left
	int indexc = (int)eItemType::COIN;
	std::vector<Vector2> coinsL = gamemap->getObjPos(indexc);

	int size = coinsL.size();
	std::stringstream strSize(size);

	gameFile << size << std::endl;
	
	for (auto& i : coinsL) {
		float tempx = i.x;
		float tempy = i.y;

		std::stringstream strx(tempx);
		std::stringstream stry(tempy);
		gameFile << tempx << " " << tempy << std::endl;

	}

	//hearts left
	int indexh = (int)eItemType::HEART;
	std::vector<Vector2> heartsL = gamemap->getObjPos(indexh);

	int sizeh = heartsL.size();
	std::stringstream strSizeh(sizeh);

	gameFile << "Lives: ";
	gameFile << sizeh << std::endl;
	for (auto& i : heartsL) {
		float tempx = i.x;
		float tempy = i.y;

		std::stringstream strx(tempx);
		std::stringstream stry(tempy);

		gameFile << tempx << " " << tempy << std::endl;
	}

	gameFile.close();
}

// ----------------------------------------------------Tutorial Stage------------------------------------------------------------------------ 
STAGE_ID TutorialStage::GetId() { return STAGE_ID::TUTORIAL; }

void TutorialStage::Render(Camera* camera, float time)
{
	RenderSky(camera);
	ground->render();

	if (cameraLocked) {
		Vector3 eye = player->model * Vector3(0.0f, 2.0f, 2.0f);
		Vector3 center = player->model * Vector3(0.0f, 0.0f, -2.0f);
		Vector3 up = player->model.rotateVector(Vector3(0.0f, 1.0f, 0.0f)); //con las models puedes hacer muchas cosas
		camera->lookAt(eye, center, up);  //hacer lerps no look at (hacer una entity camara?) ir transformando la camara haciendo interpolaciones
	}

	//Render map
	tutorial_gamemap->RenderMap(baseMap, itemMap);

	// player
	player->render();

	// enemies
	tutorial_gamemap->RenderEnemies(time);

	RenderAllGui();
}


void TutorialStage::Update(Camera* camera, float seconds_elapsed, bool mouse_locked, float mouse_speed, float angle)
{
	float speed = seconds_elapsed * mouse_speed; //the speed is defined by the seconds_elapsed so it goes constant

	
	if (Input::wasKeyPressed(SDL_SCANCODE_TAB))
	{
		cameraLocked = !cameraLocked;
	}

	if (cameraLocked)
	{
		// player
		updatePlayerPos(seconds_elapsed);
		if (!player->isAlive()) {
			currentStage = STAGE_ID::INTRO;
			delete(tutorial_gamemap);
			const char* tutorialLevelPathName = &Stage::tutorialLevelFileName[0];
			InitLevelTutorial(tutorialLevelPathName);
		}

		// enemies
		for (auto& enemies : tutorial_gamemap->enemies) {
			for (auto& e : enemies) {
				e->updateEnemy(player->pos, seconds_elapsed);
			}
		}

		// animations coins
		tutorial_gamemap->spinAll(seconds_elapsed);
		updateTutorialStage();
	}
	else {
		//mouse input to rotate the cam
		if ((Input::mouse_state & SDL_BUTTON_LEFT) || mouse_locked) //is left button pressed?
		{
			camera->rotate(Input::mouse_delta.x * 0.005f, Vector3(0.0f, -1.0f, 0.0f));
			camera->rotate(Input::mouse_delta.y * 0.005f, camera->getLocalVector(Vector3(-1.0f, 0.0f, 0.0f)));
		}

		//async input to move the camera around
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 3; //move faster with left shift
		if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
	}

	//to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();
}

void TutorialStage::updateTutorialStage()
{
	Vector2 cellPlayerPos = tutorial_gamemap->WorldToCell(player->pos);
	if (cellPlayerPos.x <= 14 && cellPlayerPos.y <= 18) {
		tutorialStage = eTutorialStage::TUTORIAL_MOVE; 
		return;
	}
	if (cellPlayerPos.x > 14 && cellPlayerPos.x <= 25 && cellPlayerPos.y >= 0 && cellPlayerPos.y <= 18) {
		tutorialStage = eTutorialStage::TUTORIAL_JUMP;
		return;
	}
	if (cellPlayerPos.x > 25 && cellPlayerPos.x <= 36 && cellPlayerPos.y >= 4 && cellPlayerPos.y <= 14) {
		tutorialStage = eTutorialStage::TUTORIAL_BOUNCE;
		return;
	}
	if (player->coins == 0) {
		tutorialStage = eTutorialStage::TUTORIAL_COIN;
		return;
	}
	if (cellPlayerPos.x >= 49 && cellPlayerPos.x <= 63 && cellPlayerPos.y >= 0 && cellPlayerPos.y <= 17) {
		tutorialStage = eTutorialStage::TUTORIAL_CANNON;
		return;
	}
	if (cellPlayerPos.x >= 53 && cellPlayerPos.x <= 59 && cellPlayerPos.y >= 17 && cellPlayerPos.y <= 25) {
		tutorialStage = eTutorialStage::TUTORIAL_HEART;
		return;
	}
	if (cellPlayerPos.x >= 35 && cellPlayerPos.x <= 63 && cellPlayerPos.y >= 24 && cellPlayerPos.y <= 31) {
		tutorialStage = eTutorialStage::TUTORIAL_SKELETON;
		return;
	}
	if (cellPlayerPos.x >= 23 && cellPlayerPos.x <= 35 && cellPlayerPos.y >= 24 && cellPlayerPos.y <= 31) {
		tutorialStage = eTutorialStage::TUTORIAL_END;
		return;
	}
	tutorialStage = eTutorialStage::TUTORIAL_MOVE;

}

void TutorialStage::RenderAllGui()
{
	//RenderAllGui
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (isHelp) {
		RenderHelpGui();
		Input::wasLeftMousePressed = false;
		return;
	}

	// count coins
	Vector4 textureRange = Vector4(0, 0, 0.125, 0.25);
	Vector4 buttonRange = Vector4(20, 50, 20, 20);
	GUI::RenderGui(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange);

	// count lives
	int dist = 25;
	for (int i = 0; i < player->lives; i++) {
		textureRange = Vector4(0.125, 0, 0.125, 0.25);
		buttonRange = Vector4(20 + i * dist, 20, 25, 25);
		GUI::RenderGui(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange);
	}

	// help button
	textureRange = Vector4(0, 0, 1, 1);
	buttonRange = Vector4(Game::instance->window_width - 20, 20, 30, 30);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, help, textureRange)) {
		isHelp = true;
	}

	char numCoins[10];
	sprintf(numCoins, "%d", player->coins);
	drawText(40, 42, numCoins, Vector3(1, 1, 1), 2);

	switch (tutorialStage) {
		case eTutorialStage::TUTORIAL_MOVE:  RenderGuiMove(); break;
		case eTutorialStage::TUTORIAL_JUMP:  RenderGuiJump(); break;
		case eTutorialStage::TUTORIAL_BOUNCE:  RenderGuiBounce(); break;
		case eTutorialStage::TUTORIAL_COIN:  RenderGuiCoin(); break;
		case eTutorialStage::TUTORIAL_CANNON:  RenderGuiCannon(); break; 
		case eTutorialStage::TUTORIAL_HEART:  RenderGuiHeart(); break;
		case eTutorialStage::TUTORIAL_SKELETON:  RenderGuiSkeleton(); break;
		case eTutorialStage::TUTORIAL_END:  RenderGuiEnd(); break;
	}

	Input::wasLeftMousePressed = false;
}

void TutorialStage::RenderHelpGui()
{
	// panel 
	Vector4 textureRange = Vector4(0, 0, 1, 1);
	Vector4 buttonRange = Vector4(400, 300, 400, 200);
	GUI::RenderGui(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, panel, textureRange);

	//exit help button
	textureRange = Vector4(0.125, 0.25, 0.125, 0.25);
	buttonRange = Vector4(560, 250, 50, 50);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange)) {
		isHelp = false;
	}

	// exit button
	textureRange = Vector4(0.5, 0.5, 0.4, 0.19);
	buttonRange = Vector4(410, 300, 250, 55);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange)) {
		currentStage = STAGE_ID::INTRO;
		isHelp = false;
	}
}

void TutorialStage::RenderGuiMove()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	
	// move
	Vector4 textureRange = Vector4(0.25, 0, 0.25, 0.5);
	Vector4 buttonRange = Vector4(200, 200, 150, 150);
	GUI::RenderGui(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange);

	// rotate
	textureRange = Vector4(0.5, 0, 0.18, 0.2);
	buttonRange = Vector4(600, 200, 110, 60);
	GUI::RenderGui(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange);
	
	drawText(100, 120, "To move the player" , Vector3(1, 1, 1), 2);
	drawText(500, 120, "To rotate the player", Vector3(1, 1, 1), 2);
	

}

void TutorialStage::RenderGuiJump()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// move
	Vector4 textureRange = Vector4(0.25, 0, 0.25, 0.5);
	Vector4 buttonRange = Vector4(200, 200, 150, 150);
	GUI::RenderGui(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange);

	// rotate
	textureRange = Vector4(0.5, 0, 0.18, 0.2);
	buttonRange = Vector4(600, 200, 110, 60);
	GUI::RenderGui(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange);

	
	// jump
	drawText(120, 100, "Press SPACE BAR to jump", Vector3(1, 1, 1), 4);
	
}

void TutorialStage::RenderGuiCannon()
{
	drawText(180, 120, "Dodge the cannonballs", Vector3(1, 1, 1), 4);
}

void TutorialStage::RenderGuiHeart()
{
	drawText(100, 120, "Collect the hearts to obtain more lives", Vector3(1, 1, 1), 3);
}

void TutorialStage::RenderGuiSkeleton()
{
	drawText(180, 120, "Dodge the Skeleton", Vector3(1, 1, 1), 4);
}

void TutorialStage::RenderGuiEnd()
{
	drawText(180, 120, "Touch the end flag", Vector3(1, 1, 1), 4);
}

void TutorialStage::RenderGuiBounce()
{
	drawText(100, 120, "Touch the bouncer to power jump", Vector3(1, 1, 1), 3);
}

void TutorialStage::RenderGuiCoin()
{
	drawText(180, 120, "Collect all the coins", Vector3(1, 1, 1), 4);
}


// ----------------------------------------------------------End Stage-----------------------------------------------------------------------------
STAGE_ID EndStage::GetId() { return STAGE_ID::END; }

void EndStage::Render(Camera* camera, float time)
{
	RenderAllGui();
}

void EndStage::Update(Camera* camera, float seconds_elapsed, bool mouse_locked, float mouse_speed, float angle)
{
}

void EndStage::RenderAllGui()
{
	//RenderAllGui
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	if (!(player->isAlive()) && !hasFinished) {
		RenderLoseGui();
	}
	else if (hasFinished) {
		Audio::Play("data/sound/cheer.wav");
		RenderWinGui();
	}
	else RenderExitGui(); 

	Input::wasLeftMousePressed = false;
}

void EndStage::RenderExitGui()
{
	// panel 
	Vector4 textureRange = Vector4(0, 0, 1, 1);
	Vector4 buttonRange = Vector4(400, 300, 400, 300);
	GUI::RenderGui(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, panel, textureRange);

	//continue button
	textureRange = Vector4(0, 0, 1, 0.25);
	buttonRange = Vector4(450, 250, 350, 80);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange)) {
		currentStage = STAGE_ID::PLAY;
	}

	//exit button
	textureRange = Vector4(0, 0.25, 1, 0.25);
	buttonRange = Vector4(450, 370, 350, 80);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange)) {
		exit = true; 
	}
}

void EndStage::RenderLoseGui()
{
	// panel 
	Vector4 textureRange = Vector4(0, 0, 1, 1);
	Vector4 buttonRange = Vector4(400, 300, 400, 300);
	GUI::RenderGui(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, panel, textureRange);

	//play again button
	textureRange = Vector4(0, 0.5, 1, 0.25);
	buttonRange = Vector4(450, 250, 350, 80);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange)) {
		currentStage = STAGE_ID::PLAY;
		currentLevel = 0;
		restartGame();
	}

	//exit button
	textureRange = Vector4(0, 0.25, 1, 0.25);
	buttonRange = Vector4(450, 370, 350, 80);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange)) {
		exit = true;
	}
}

void EndStage::RenderWinGui()
{
	// panel 
	Vector4 textureRange = Vector4(0, 0, 1, 1);
	Vector4 buttonRange = Vector4(400, 300, 400, 300);
	GUI::RenderGui(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, panel, textureRange);

	//play again button
	textureRange = Vector4(0, 0.5, 1, 0.25);
	buttonRange = Vector4(450, 250, 350, 80);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange)) {
		currentLevel = 0;
		restartGame();
		currentStage = STAGE_ID::PLAY;
	}

	//exit button
	textureRange = Vector4(0, 0.25, 1, 0.25);
	buttonRange = Vector4(450, 370, 350, 80);
	if (GUI::RenderButton(buttonRange.x, buttonRange.y, buttonRange.z, buttonRange.w, buttons, textureRange)) {
		exit = true;
	}
}


