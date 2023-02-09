#include "entity.h"
#include "camera.h"
#include "utils.h"
#include "input.h"
#include "audio.h"
#include "pathfinders.h"

#define TARGET_DISTANCE_WOLF 10
#define TARGET_DISTANCE_CANNON 10

float noRenderD = 100.0f;
float radius = 10.0f;

Animation* EntityWolf::animIdle;
Animation* EntityWolf::animCombat;

// constructor of entity Mesh
EntityMesh::EntityMesh(Mesh* mesh, Texture* tex, Shader* shader) {
	this->mesh = mesh;
	this->texture = tex;
	this->shader = shader;
}

EntityMesh::EntityMesh(const char* mesh, const char* tex)
{
	this->mesh = Mesh::Get(mesh);
	this->texture = Texture::Get(tex);
	this->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
}

// overall render function
void EntityMesh::render(float time, bool bounding, float tiling) {
	assert(mesh != NULL, "mesh in renderMesh was null");

	if (!shader) return;

	Camera* camera = Camera::current;
	//Matrix44 new_model = this->model;

	Vector3 pos = model.getTranslation();
	
	if (!camera->testSphereInFrustum(pos, mesh->radius)) return;

	Vector3 camPos = camera->eye;
	float dist = pos.distance(camPos);

	if (dist > noRenderD) return;

	//enable shader
	shader->enable();

	//upload uniforms
	shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_texture", texture, 0);
	shader->setUniform("u_time", time);
	//shader->setUniform("u_tex_tilling", tiling);
	shader->setUniform("u_model", model);

	//do the draw call
	mesh->render(GL_TRIANGLES);

	//disable shader
	shader->disable();

	if (bounding) {
		mesh->renderBounding(model);
	}
}


void EntityMesh::renderVectorOfModels(std::vector<Matrix44> models, Mesh* mesh, Texture* tex, Shader* shader)
{
	assert(mesh != NULL, "mesh in renderMesh was null");

	if (!shader) return;

	Camera* camera = Camera::current;
	//Matrix44 new_model = this->model;

	//enable shader
	shader->enable();

	//upload uniforms
	shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_texture", tex, 0);
	shader->setUniform("u_time", time);
	//shader->setUniform("u_tex_tilling", tiling);
	for (size_t i = 0; i < models.size(); i++) {
		Matrix44 model = models[i];

		Vector3 pos = model.getTranslation();
		//if not in frustum continue
		if (!camera->testSphereInFrustum(pos, mesh->radius)) continue;

		Vector3 camPos = camera->eye;
		float dist = pos.distance(camPos);
		//if very far continue
		if (dist > noRenderD) continue;

		shader->setUniform("u_model", model);
		//do the draw e
		mesh->render(GL_TRIANGLES);
	}
	
	//disable shader
	shader->disable();

	//mesh->renderBounding(models[0]); 
}

void EntityMesh::updateModel()
{
	Matrix44 newModel;
	newModel.translate(pos.x, pos.y, pos.z);  // podemos hacer una funcion translate que acepte directamente un Vector3
	newModel.rotate(yaw * DEG2RAD, Vector3(0, 1, 0));
	model = newModel;
}

void EntityMesh::setPos(Vector3 nextPos)
{
	pos = nextPos;
	updateModel();
}

void EntityMesh::setRotation(float nextYaw)
{
	yaw = nextYaw;
	updateModel();
}

void EntityMesh::setPosRot(Vector3 nextPos, float nextYaw)
{
	pos = nextPos;
	yaw = nextYaw;
	updateModel();
}

// ------------------------------------------- PLAYER ---------------------------------------------------------

bool EntityPlayer::isAlive()
{
	return (lives > 0);
}

Vector3 EntityPlayer::nextPos(float elapsed_time)
{
	rotate(elapsed_time);

	switch (currentState)
	{
		
		case playerState::STAND: {

			getNextVelStand(elapsed_time);
			break;
		}
		case playerState::WALK: {
			getNextVelWalk(elapsed_time);
			break;
		}
		case playerState::JUMP: {
			getNextVelJump(elapsed_time);
			break;
		}
		case playerState::BOUNCE: {
			getNextVelBounce(elapsed_time);
			break;
		}
	}
	// add gravity
	float gravity = gravityC * elapsed_time;
	vel.y = vel.y - gravity;
	vel.y = clamp(vel.y, -0.5f, vel.y);
	return (vel + pos);

	//vel = velocity;
}

void EntityPlayer::getNextVelStand(float elapsed_time)
{
	
	vel = Vector3();
		
	//if is not on the ground (and not bouncing), it can no longer stand --> jump state
	if (!isGround && !isBouncing) { 
		vel.y = 0;
		currentState = playerState::JUMP;
		return;
	}

	//if is bouncing --> bounce state
	if (isBouncing) {
		vel.y = 0;
		bounce(elapsed_time);
		return;
	}

	// if is moving and is on the ground --> walk state
	if (isMoving()) {
		currentState = playerState::WALK;
		return;
	}
	// is space is press --> jump;
	else if (Input::wasKeyPressed(SDL_SCANCODE_SPACE)) {
		vel.y = 0;
		jump(elapsed_time);
		return;
	}

	return;
}

void EntityPlayer::getNextVelWalk(float elapsed_time)
{

	//if is not moving return to stand state
	if (!isMoving()) {
		currentState = playerState::STAND;
		return;
	}

	//move
	move(elapsed_time);

	// is space is press --> jump;
	if (Input::wasKeyPressed(SDL_SCANCODE_SPACE)) {
		vel.y = 0;
		jump(elapsed_time);
		return;
	}

	//if is not on the ground (and not bouncing)--> jump state
	if (!isGround && !isBouncing) {
		currentState = playerState::JUMP;
		return;
	}

	//if is bouncing --> bounce state
	if (isBouncing) {
		bounce(elapsed_time);
		return;
	}

	return;
}

void EntityPlayer::getNextVelJump(float elapsed_time) {
	
	//// add gravity
	//float gravity = gravityC * elapsed_time;
	//vel = vel - Vector3(0.0f, gravity, 0.0f);

	// move in the air also
	move(elapsed_time, true);

	//if is in ground
	if (isGround) {
		currentState = playerState::STAND;
	}

}

void EntityPlayer::getNextVelBounce(float elapsed_time)
{
	
	// move in the air also
	move(elapsed_time, true);

	//if is in ground
	if (isGround) {
		currentState = playerState::WALK;
		isBouncing = false;
	}
}

//void EntityPlayer::checkGround()
//{
//	if (isGround) {
//		isJumping = false;
//		isBouncing = false;
//	}
//}

void EntityPlayer::jump(float elapsed_time)
{
	float playerSpeed = jumpSpeed;
	vel.y = playerSpeed;
	currentState = playerState::JUMP;
	isGround = false;
	Audio::Play("data/sound/Jump.wav");
}

void EntityPlayer::bounce(float elapsed_time)
{
	float playerSpeed = bounceSpeed;
	vel.y = playerSpeed;
	currentState = playerState::BOUNCE;
	isGround = false;
	Audio::Play("data/sound/Jump.wav");
}

void EntityPlayer::restartLives(int numLives)
{
	lives = numLives;
}

void EntityPlayer::restart(int numLives)
{
	restartLives(numLives);
	coins = 0; 
	vel = Vector3();
	isGround = true;
}

bool EntityPlayer::isMoving()
{
	return (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_D));
}

void EntityPlayer::move(float elapsed_time, bool isAir)
{
	Matrix44 playerRotation;
	playerRotation.rotate(yaw * DEG2RAD, Vector3(0, 1, 0));

	float currSpeed = isAir ? airSpeed : walkSpeed;
	float playerSpeed = currSpeed * elapsed_time;


	Vector3 forward = playerRotation.rotateVector(Vector3(0, 0, -1));
	Vector3 right = playerRotation.rotateVector(Vector3(1, 0, 0));  // video colisions2 min 1:05:00

	if (Input::isKeyPressed(SDL_SCANCODE_W)) {
		vel.x = (forward.x * playerSpeed);
		vel.z = (forward.z * playerSpeed);
	}
	if (Input::isKeyPressed(SDL_SCANCODE_S)) {
		vel.x = -(forward.x * playerSpeed);
		vel.z = -(forward.z * playerSpeed);
	}if (Input::isKeyPressed(SDL_SCANCODE_A)) {
		vel.x = -(right.x * playerSpeed);
		vel.z = -(right.z * playerSpeed);
	}if (Input::isKeyPressed(SDL_SCANCODE_D)) {
		vel.x = (right.x * playerSpeed);
		vel.z = (right.z * playerSpeed);
	}
	
}

void EntityPlayer::rotate(float elapsed_time)
{
	float rotSpeed = 120.0f * elapsed_time;

	if (Input::isKeyPressed(SDL_SCANCODE_Q)) yaw += rotSpeed;
	if (Input::isKeyPressed(SDL_SCANCODE_E)) yaw -= rotSpeed;
}

// ------------------------------------------------------------- ENEMIES -------------------------------------------------------------------

float sign(float value) {
	return value >= 0.0f ? 1.0f : -1.0f;
}

void EntityEnemy::updateEnemy(Vector3 targetPos, float seconds_elapsed)
{
	switch (currentBehaviour){
		case ENEMY_BEHAVIOUR::IDLE: idle(targetPos); break;
		case ENEMY_BEHAVIOUR::COMBAT: combat(targetPos, seconds_elapsed); break;
	}
}

void EntityEnemy::rotateToTarget(Vector3 targetPos, float seconds_elapsed)
{
	Vector3 side = model.rotateVector(Vector3(1, 0, 0)).normalize();

	Vector3 toTarget = targetPos - pos;
	float dist = toTarget.length();
	toTarget.normalize();

	float sideDot = side.dot(toTarget);

	yaw += 90.0f * sign(sideDot) * seconds_elapsed;
	updateModel(); 
}

void EntityEnemy::followTarget(Vector3 targetPos, float seconds_elapsed)
{
	Vector3 side = model.rotateVector(Vector3(1, 0, 0)).normalize();
	Vector3 forward = model.rotateVector(Vector3(0, 0, -1)).normalize();

	Vector3 toTarget = targetPos - pos;
	float dist = toTarget.length();
	toTarget.normalize();

	float sideDot = side.dot(toTarget);
	float forwardDot = forward.dot(toTarget);
	
	if (forwardDot < 0.98f) {
		yaw += 180.0f * sign(sideDot) * seconds_elapsed;
	}
	if (dist > 0.5f) {
		pos = pos + (forward * speed * seconds_elapsed);
	}

	updateModel();
}

// ------------------------------------------------------- wolf enemy ------------------------------------------------------

void EntityWolf::idle(Vector3 playerPos)
{
	Vector3 dif = playerPos - pos;
	double dist = dif.length();
	
	int x = round(playerPos.x / tileWidth);
	int y = round(playerPos.z / tileDepth);

	if (navPath[x + y * mapWidth] == 1) {
		currentBehaviour = ENEMY_BEHAVIOUR::COMBAT;
	}
}

void EntityWolf::render(float time, bool bounding, float tiling)
{
	assert(animMesh != NULL, "mesh in renderMesh was null");

	if (!shader) return;

	Camera* camera = Camera::current;
	//Matrix44 new_model = this->model;

	Vector3 pos = model.getTranslation();

	if (!camera->testSphereInFrustum(pos, mesh->radius)) return;

	Vector3 camPos = camera->eye;
	float dist = pos.distance(camPos);

	if (dist > noRenderD) return;

	Animation* anim;
	if (currentBehaviour == ENEMY_BEHAVIOUR::COMBAT) {
		anim = animCombat;
	}
	else {
		anim = animIdle;
	}

	
	anim->assignTime(time);
	
	Matrix44 animModel = model;
	animModel.scale(0.01, 0.01, 0.01);
	animModel.rotate(180 * DEG2RAD, Vector3(0, 1, 0));

	//enable shader
	shader->enable();

	//upload uniforms
	shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_texture", texture, 0);
	shader->setUniform("u_time", time);
	//shader->setUniform("u_tex_tilling", tiling);
	shader->setUniform("u_model", animModel);

	//do the draw call
	animMesh->renderAnimated(GL_TRIANGLES, &anim->skeleton);

	//disable shader
	shader->disable();

}


void EntityWolf::combat(Vector3 playerPos, float seconds_elapsed)
{
	int x = round(playerPos.x / tileWidth);
	int y = round(playerPos.z / tileDepth);

	if (navPath[x + y * mapWidth] != 1) {
		currentBehaviour = ENEMY_BEHAVIOUR::IDLE;
		return;
	}

	/*Vector3 dif = playerPos - pos;
	double dist = dif.length();

	if (dist > TARGET_DISTANCE_WOLF) {
		currentBehaviour = ENEMY_BEHAVIOUR::IDLE;
		return;
	}*/
	
	followTargetNavPath(playerPos, seconds_elapsed);
	
	//followTarget(playerPos, seconds_elapsed);
}

void EntityWolf::search()
{
}

void EntityWolf::followTargetNavPath(Vector3 targetPos, float seconds_elapsed)
{
	Vector3 side = model.rotateVector(Vector3(1, 0, 0)).normalize();
	Vector3 forward = model.rotateVector(Vector3(0, 0, -1)).normalize();

	Vector3 toTarget = targetPos - pos;
	float dist = toTarget.length();
	toTarget.normalize();

	float sideDot = side.dot(toTarget);
	float forwardDot = forward.dot(toTarget);

	if (forwardDot < 0.98f) {
		yaw += 180.0f * sign(sideDot) * seconds_elapsed;
	}
	if (dist > 0.5f) {
		Vector3 nextPos = pos + (forward * speed * seconds_elapsed);
		int x = round(nextPos.x / tileWidth);
		int y = round(nextPos.z / tileDepth);
	
		if (navPath[x + y * mapWidth] == 1) {
			pos = nextPos;
		}
		
	}

	updateModel();
}

// ------------------------------------------------------- cannon enemy ------------------------------------------------------


void EntityCannon::idle(Vector3 playerPos)
{
	Vector3 dif = playerPos - pos;
	double dist = dif.length();
	
	if (dist < TARGET_DISTANCE_CANNON) {
		currentBehaviour = ENEMY_BEHAVIOUR::COMBAT;
		// if (isShooting == true) return;
		editAllBullets(0.0f, model);
		timeBetweenShoot = TIME_SHOOT;
		bullets[countBullet].ttl = TTL_BULLET;
		isShooting = true;
		return;
	}
}

void EntityCannon::updateBullet(float seconds_elapsed, sCannonBullet& bullet) {
	
	Vector3 currentPos = bullet.model.getTranslation();
	Vector3 next = currentPos + (bullet.vel * bullet.speed * seconds_elapsed);

	//if next pos colide with player
	//		delete bullet (method)
	//		continue

	bullet.lastPos = currentPos;
	bullet.model.setTranslation(next.x, next.y, next.z);
	bullet.model.rotate(yaw * DEG2RAD, Vector3(0, 1, 0));
	bullet.ttl -= seconds_elapsed;

	if (!bullet.isActive()) {
		bullet.lastPos = model.getTranslation();
		bullet.model = model;
	};
}

void EntityCannon::shootBullets(float seconds_elapsed)
{
	
	for (int i = 0; i < NUM_BULLETS; i++) {
		if (!bullets[i].isActive()) continue;
		updateBullet(seconds_elapsed, bullets[i]);
	}

	timeBetweenShoot -= seconds_elapsed;
	if (timeBetweenShoot < 0.0f) {
		countBullet++; 
		if (countBullet >= NUM_BULLETS) countBullet = 0;
		timeBetweenShoot = TIME_SHOOT;
		bullets[countBullet].ttl = TTL_BULLET;
		bullets[countBullet].model = model;
	}
}

void EntityCannon::editAllBullets(float ttl, Matrix44 n_model, Vector3 lastPos, float speed)
{
	for (auto& b : bullets) {
		b.ttl = ttl;
		b.model = n_model;
		b.lastPos = lastPos;
		b.speed = speed;
	}
}

void EntityCannon::editAllBulletsVel(Vector3 velocity) {
	for (auto& b : bullets) {
		b.vel = velocity;
	}
}


void EntityCannon::combat(Vector3 playerPos, float seconds_elapsed)
{
	Vector3 dif = playerPos - pos;
	double dist = dif.length();

	if (dist > TARGET_DISTANCE_CANNON) {
		currentBehaviour = ENEMY_BEHAVIOUR::IDLE;
		isShooting = false;
		timeBetweenShoot = TIME_SHOOT;
		return;
	}

	shootBullets(seconds_elapsed);
}

void EntityCannon::search()
{
}

void EntityCannon::renderBullets()
{
	assert(bulletEntity->mesh != NULL, "mesh in renderMesh was null");

	if (!shader) return;

	Camera* camera = Camera::current;
	//Matrix44 new_model = this->model;

	//enable shader
	shader->enable();

	//upload uniforms
	shader->setUniform("u_color", Vector4(1, 1, 1, 1));
	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_texture", bulletEntity->texture, 0);
	shader->setUniform("u_time", time);
	//shader->setUniform("u_tex_tilling", tiling);
	for (int i = 0; i < NUM_BULLETS; i++) {
		if (bullets[i].isActive()) {

			Matrix44 model = bullets[i].model;

			Vector3 pos = model.getTranslation();
			//if not in frustum continue
			if (!camera->testSphereInFrustum(pos, bulletEntity->mesh->radius)) continue;

			Vector3 camPos = camera->eye;
			float dist = pos.distance(camPos);
			//if very far continue
			if (dist > noRenderD) continue;

			shader->setUniform("u_model", model);
			//do the draw e
			bulletEntity->mesh->render(GL_TRIANGLES);
		}
	}

	//disable shader
	shader->disable();
}

bool EntityCannon::checkCollisionWithBullets(Vector3 character_center)
{
	int touched = 0;
	for(auto& bullet : bullets){
		if (bulletEntity->mesh->testSphereCollision(bullet.model, character_center, 0.5, Vector3(), Vector3() ) == false)
			continue; //si no colisiona, pasamos al siguiente objeto
		touched++;
	}
	return touched > 0;
}




