#include "world.h"
#include "utils.h"
#include "stages.h"

#define MAX_LINE_LEN 1000

GameMap::GameMap()
{
	width = height = 0;
	data = NULL;
	itemsFileName = NULL;
	
	initNavegationPath();
	initCellModels();
	initItemModels();

	spawnPlayerPos = Vector2(0, 0);
}

GameMap::GameMap(int w, int h)
{
	width = w;
	height = h;
	data = new sCell[w * h];
	itemsFileName = NULL;

	initNavegationPath();
	initCellModels();
	initItemModels();
	initEnemies();
	
	spawnPlayerPos = Vector2(0, 0);
}

void GameMap::initNavegationPath()
{
	navPath = new uint8[width * height];

	for (size_t i = 0; i < width * height; i++) {
		navPath[i] = 1;
	}
}

void GameMap::initCellModels()
{
	for (int i = 0; i < eCellType::COUNT + 1; i++) {
		std::vector<Matrix44> models;
		cellModels.push_back(models);
	}
}

void GameMap::initItemModels()
{
	for (int i = 0; i < eItemType::NUMBER_ITEMS + 1; i++) {
		std::vector<Matrix44> models;
		itemModels.push_back(models);
	}
}

void GameMap::initEnemies()
{
	for (int i = 0; i < ENEMY::NUMBER_ENEMIES; i++) {
		std::vector<EntityEnemy*> enemy;
		enemies.push_back(enemy);
	}
}

sCell& GameMap::getCell(int x, int y)
{
	return data[x + y * width];
}

void GameMap::setCell(int x, int y, eCellType type) {
	data[x + y * width].type = type;
}


Vector2 GameMap::WorldToCell(Vector3 worldPos)
{
	int x = clamp(round(worldPos.x / tileWidth), 0, width);
	int y = clamp(round(worldPos.z / tileDepth), 0, height);
	return Vector2(x, y);
}

Vector3 GameMap::CellToWorld(Vector2 cellPos){
	int height = getHeight(cellPos); 
	return Vector3(cellPos.x * tileWidth, height * tileHeight, cellPos.y * tileDepth);
}

std::vector<Vector2> GameMap::getObjPos(int indexItem) {
	std::vector<Vector2> itemsPos;
	int objLeft = itemModels[indexItem].size();
	itemsPos.reserve(objLeft);

	for (auto& i : itemModels[indexItem]) {
		Matrix44 temp = i; 
		Vector2 posS = Vector2(temp.getTranslation().x, temp.getTranslation().z);
		itemsPos.push_back(posS);
	}

	return itemsPos;
}

bool GameMap::eraseItem(Vector2 pos, int indexItem)
{
	Vector2 posToErase = pos;
	posToErase.x = round(posToErase.x);
	posToErase.y = round(posToErase.y);
	auto i = itemModels[indexItem].begin();
	while (i != itemModels[indexItem].end()) {
		Matrix44 m = *i;
		Vector2 cellPosM = WorldToCell(m.getTranslation());
		if (posToErase.x == cellPosM.x && posToErase.y == cellPosM.y) {
			i = itemModels[indexItem].erase(i);
			int posX = posToErase.x;
			int posY = posToErase.y;
			getCell(posX, posY).item = eItemType::NOTHING;
			navPath[posX + posY * width] = 1;
			return true;
		}
		else {
			i++;
		}
	}
	return false;
}

void GameMap::restartMap()
{
	itemModels.clear();
	initItemModels();
	loadItems(itemsFileName); 

	enemies.clear();
	initEnemies();
	loadEnemies(enemiesFileName);
}

void GameMap::spinItem(float seconds_elapsed, std::vector<Matrix44>& models)
{
	float rotSpeed = 90.0f * seconds_elapsed;

	for (int i = 0; i < models.size(); i++) {
		models[i].rotate(rotSpeed * DEG2RAD, Vector3(0,1,0));
	}
}

void GameMap::spinAll(float seconds_elapsed)
{
	spinItem(seconds_elapsed, itemModels[eItemType::COIN]);
	spinItem(seconds_elapsed, itemModels[eItemType::HEART]);
}

//std::vector<GameMap*> GameMap::loadLevels(const char* filename)
//{
//
//}

GameMap* GameMap::loadGameMap(const char* filename, float tileWidth, float tileHeight, float tileDepth)
{
	eCellType type;
	int gridNum;
	int height;

	FILE* file = fopen(filename, "rb");
	if (file == NULL) //file not found
		return NULL;

	sMapHeader header; //read header and store it in the struct
	fread(&header, sizeof(sMapHeader), 1, file);
	assert(header.bytes == 1); //always control bad cases!!

	//allocate memory for the cells data and read it
	unsigned char* cells = new unsigned char[header.w * header.h];
	fread(cells, header.bytes, header.w * header.h, file);
	fclose(file); //always close open files
	//create the map where we will store it
	GameMap* map = new GameMap(header.w, header.h);
	map->tileWidth = tileWidth;
	map->tileHeight = tileHeight;
	map->tileDepth = tileDepth;

	for (int x = 0; x < map->width; x++)
		for (int y = 0; y < map->height; y++)
		{
			gridNum = (int)cells[x + y * map->width];
			type = (eCellType)(gridNum % 16);
			height = (int)(gridNum / 16);
			sCell& currCell = map->getCell(x, y);
			currCell.type = type;
			currCell.height = height;
			currCell.item = eItemType::NOTHING;

			Matrix44 m;
			m.translate(x * tileWidth, height * tileHeight, y * tileDepth);
			map->cellModels[type].push_back(m);

			//create prop
			/*if (type != eCellType::H1_CUBE && type != eCellType::H1_CENTER && type != eCellType::H1_WATER){
				EntityMesh* prop = new EntityMesh(viewDatas[type].mesh, viewDatas[type].texture, viewDatas[type].shader);
				prop->model.translate(x * tileWidth, height * tileHeight, y * tileDepth);
				entities.push_back(prop);
			}*/
		}

	delete[] cells; //always free any memory allocated!

	return map;
}

void GameMap::loadItems(const char* filename)
{
	//open file
	FILE* f = fopen(filename, "rb");
	if (f == NULL)
		return;
	itemsFileName = filename; 
	int type, posX, height, posY, deg;
	while (fscanf(f, "%d %d,%d,%d %d", &type, &posX, &height, &posY, &deg) == 5)
	{
		Matrix44 m;
		m.translate(posX* tileWidth, height * tileHeight, posY * tileDepth);
		m.rotate((deg * DEG2RAD), Vector3(0, 1, 0));
		//m.rotateVector(Vector3(rotX, rotY, rotZ));
		itemModels[type].push_back(m);
		getCell(posX, posY).item = (eItemType) type; 

		//Nav path
		navPath[posX + posY * width] = 0;
	}

	//close file at the end
	fclose(f);
}

void GameMap::loadEnemies(const char* filename)
{
	enemiesFileName = filename;

	std::string content = "";
	readFile(filename, content);
	std::stringstream ss(content);
	int mumberOfEnemies;
	ss >> mumberOfEnemies;

	for(int n = 0; n < mumberOfEnemies; n++){
		int type;
		std::string meshPath;
		std::string texPath;
		std::string animMeshPath;

		ss >> type >> meshPath >> texPath;

		const char* meshName = &meshPath[0];
		const char* texName = &texPath[0];
		const char* animMesName;
		if (type == ENEMY::WOLF) {
			ss >> animMeshPath;
			animMesName = &animMeshPath[0];
		}
		

		int numOfEntities;
		ss >> numOfEntities;

		for (int i = 0; i < numOfEntities; i++) {
			int posX, posY, posZ, yaw;
			char dsicard;
			ss >> posX >> dsicard >> posY >> dsicard >> posZ >> yaw;
			
			Vector3 pos = Vector3(posX*tileWidth, posY*tileHeight, posZ*tileDepth);
			
			EntityEnemy* enemy;
			switch (type) {
			case ENEMY::WOLF: {
				EntityWolf* wolf = new EntityWolf(meshName, texName);
				wolf->animMesh = Mesh::Get(animMesName);

				wolf->navPath = new uint8[width * height];
				int initPathX = posX - round(wolf->pathWidht/2);
				int initPathY = posZ - round(wolf->pathHeight/2);

				for (size_t i = 0; i < width * height; i++) {
					wolf->navPath[i] = 0;
				}

				for (size_t i = 0; i < wolf->pathWidht; i++) {
					for(size_t j = 0; j < wolf->pathHeight; j++){
						int x = clamp(initPathX + i, 0, width);
						int y = clamp(initPathY + j, 0, height);

						if (getHeight(Vector2(x, y)) == posY) {
							wolf->navPath[x + y * width] = navPath[x + y * width];;
						}
					}
				}

				wolf->mapWidth = width;
				wolf->tileDepth = tileDepth;
				wolf->tileWidth = tileWidth;
				enemy = wolf;
				break;
			}
					
			case ENEMY::CANNON: {
				EntityCannon* cannon = new EntityCannon(meshName, texName); 
				switch (yaw) {
				case 0: cannon->editAllBulletsVel(Vector3(1,0,0)); break;
				case 90: cannon->editAllBulletsVel(Vector3(0,0,1)); break;
				case 180: cannon->editAllBulletsVel(Vector3(-1, 0, 0)); break;
				case 270: cannon->editAllBulletsVel(Vector3(0, 0, -1)); break;
				}
				enemy = cannon;
				break;
			}
			}
			
			//Stage::loadMeshEntity(enemy, meshName, texName);
			enemy->type = (ENEMY)type; 
			enemy->currentBehaviour = ENEMY_BEHAVIOUR::IDLE;
			enemy->setPosRot(pos, yaw);
			enemies[type].push_back(enemy);
		}
	}
}

void GameMap::RenderMap(std::vector<EntityMesh*> viewDatas, std::vector<EntityMesh*> itemsEntities)
{
	//for every cell
	/*for (int x = 0; x < width; ++x)
		for (int y = 0; y < height; ++y) {
			sCell& cell = getCell(x, y);
			int index = (int)cell.type;
			int height = cell.height;

			// Matrix44 model;
			if (index != 0) {

				EntityMesh* prop = new EntityMesh(viewDatas[index].mesh, viewDatas[index].texture, viewDatas[index].shader);
				prop->model.translate(x * tileWidth, height * tileHeight, y * tileDepth);
				//if(entities.size() < 8192) entities.push_back(prop);

				prop->render();
				delete(prop);

			}
		}*/
	for (int i = 1; i < eCellType::COUNT; i++) {
		if (cellModels[i].size() > 0)
			EntityMesh::renderVectorOfModels(cellModels[i], viewDatas[i]->mesh, viewDatas[i]->texture, viewDatas[i]->shader);
	}

	for (int i = 0; i < eItemType::NUMBER_ITEMS; i++) {
		if (itemModels[i].size() > 0)
			EntityMesh::renderVectorOfModels(itemModels[i], itemsEntities[i]->mesh, itemsEntities[i]->texture, itemsEntities[i]->shader);
	}

};

void GameMap::RenderEnemies(float time) {
	for (int i = 0; i < enemies.size(); i++) {
		if (enemies[i].size() <= 0) continue;
		for (auto& enemy : enemies[i]) {
			
			switch (enemy->type) {
				case ENEMY::CANNON:
				{
					enemy->render();
					EntityCannon* cannon = (EntityCannon*)enemy;
					if (cannon->isShooting) cannon->renderBullets();
					break;
				}
				case ENEMY::WOLF:
				{
					EntityWolf* skeleton = (EntityWolf*)enemy;
					skeleton->render(time);
				}
			}
			
		}

	}
}

std::vector<sCellPos> GameMap::getNeighbourCells(Vector3 worldPos)
{
	std::vector<sCellPos> neighbourCells;
	neighbourCells.reserve(9);
	
	Vector2 mapPos = WorldToCell(worldPos);
	
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			Vector2 pos = mapPos + Vector2(x, y);

			if (pos.x < 0 || (pos.x > width - 1) || pos.y < 0 || (pos.y > height-1)) continue;

			sCell& currentCell = getCell(pos.x, pos.y);
			sCellPos cellPos;
			cellPos.cell = currentCell;
			cellPos.pos = pos;
			neighbourCells.push_back(cellPos);
		}
	}

	return neighbourCells;
}

int GameMap::getHeight(Vector3 worldPos) {
	Vector2 GridPos = WorldToCell(worldPos);
	sCell& cell = getCell(GridPos.x, GridPos.y);
	return cell.height;
}

int GameMap::getHeight(Vector2 cellPos) {
	sCell& cell = getCell(cellPos.x, cellPos.y);
	return cell.height;
}

Vector3 GameMap::getSpawnPlayerPos() {
	return CellToWorld(spawnPlayerPos);
}