#ifndef ENTITY_H
#define ENTITY_H

#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "audio.h"
#include "animation.h"


#define NUM_BULLETS 2
#define TIME_SHOOT 2.0f
#define TTL_BULLET 5.0f
#define BULLET_SPEED 5.0f


// class entity, for our tree of entities

// some enemies for different levels
enum ENEMY {
    WOLF,
    CANNON,
    NUMBER_ENEMIES
}; 

enum ENEMY_BEHAVIOUR {
    COMBAT,
    SEARCH, 
    IDLE
};

float const gravityC = 1.0f;

using namespace std;

struct sCannonBullet {
    Matrix44 model;
    Vector3 lastPos;
    Vector3 vel;
    float speed;
    float ttl;

    bool isActive() {
        return ttl >= 0.0f;
    }
};

enum playerState {
    STAND,
    WALK,
    JUMP,
    BOUNCE,
};

// parent 
class Entity {
public:
    Entity() {
        Matrix44 m;
        model = m;
    };

    //virtual ~Entity() = 0; //destructor

    //some attributes 
    //std::string name;
    Matrix44 model;

    //methods overwritten by derived classes 
    virtual void render(float time = NULL,bool bounding = false, float tiling = 1.0f) = 0;

    //some useful methods...
    //Vector3 getPosition();

};

// first child

class EntityMesh : public Entity
{
public:
    //Attributes of this class 
    Mesh* mesh;
    Texture* texture;
    Shader* shader;
    Vector4 color;
    Vector3 pos;
    float yaw;

    //Constructor
    EntityMesh() {
        mesh = NULL;
        texture = NULL;
        shader = NULL;
        yaw = 0;
    };

    EntityMesh(Mesh* mesh, Texture* tex, Shader* shader);
    EntityMesh(const char* mesh, const char* tex);
    ~EntityMesh() {}
    
    //static methods
    void static renderVectorOfModels(std::vector<Matrix44> models, Mesh* mesh, Texture* tex, Shader* shader);

    //methods overwritten 
    void render(float time = NULL, bool bounding = false, float tiling = 1.0f);
   
    //setters
    void setPos(Vector3 nextPos);
    void setRotation(float nextYaw);
    void setPosRot(Vector3 nextPos, float nextYaw);
    void updateModel(); 
};

// second child

class EntityPlayer : public EntityMesh
{
public:
    // To move the player
    
    Vector3 vel;
    playerState currentState = playerState::STAND; 

    // speeds
    float const jumpSpeed = 0.15f;
    float const airSpeed = 5.0f;
    float const walkSpeed = 10.0f;
    float const bounceSpeed = 0.22f;

    
    //counters
    int coins = 0;
    int lives = 3;
    
    // booleans
    bool isGround = true;
    bool isBouncing = false; 
    //bool isJumping = false;

    //Constructor
    EntityPlayer() : EntityMesh() {
        vel = Vector3(); 
        isGround = true;
        coins = 0;
    }

    EntityPlayer(const char* mesh, const char* tex) : EntityMesh(mesh, tex) {
        vel = Vector3();
        isGround = true;
        coins = 0;
    };

    EntityPlayer(Mesh* mesh, Texture* tex, Shader* shader) : EntityMesh(mesh, tex, shader) {
        vel = Vector3();
        isGround = true;
        coins = 0;
    }

    // Destructor
    ~EntityPlayer() {
    }

    bool isAlive();

    // move player
    Vector3 nextPos(float elapsed_time);
    void getNextVelStand(float elapsed_time);
    void getNextVelWalk(float elapsed_time);
    void getNextVelJump(float elapsed_time);
    void getNextVelBounce(float elapsed_time);
    bool isMoving();
    void move(float elapsed_time, bool isAir = false);
    void rotate(float elapsed_time);
   
    void jump(float elapsed_time);
    void bounce(float elapsed_time);

    // restart
    void restartLives(int numLives = 3);
    void restart(int numLives = 3);

    // counters
    void updatePoints() {
        coins += 1;
        Audio::Play("data/sound/coin.wav");
    }
    void subtractLives() {
        lives--;
        Audio::Play("data/sound/whack.wav");
    }
    void addLives() {
        lives++;
        Audio::Play("data/sound/coin.wav");  //busacr otro audio mejor
    }

};

// third child

class EntityEnemy : public EntityMesh
{
public:
    //Attributes of this class 
    ENEMY type;
    float speed = 3.0f;
    bool alive = true;
    ENEMY_BEHAVIOUR currentBehaviour;


    //vector of enemies
    static vector<EntityMesh*> enemies;

    //Constructor
    EntityEnemy() : EntityMesh(){};
    EntityEnemy(const char* mesh, const char* tex) : EntityMesh(mesh, tex) {};

    ~EntityEnemy() {
        alive = false;
    }

    void updateEnemy(Vector3 targetPos, float seconds_elapsed);

    // virtual methods 
    virtual void combat(Vector3 playerPos, float seconds_elapsed) = 0;
    virtual void search() = 0;
    virtual void idle(Vector3 playerPos) = 0;

    // movement 
    void rotateToTarget(Vector3 targetPos, float seconds_elapsed);
    void followTarget(Vector3 targetPos, float seconds_elapsed);
};

// children of enemy

class EntityWolf : public EntityEnemy 
{
public:

    static Animation* animIdle;
    static Animation* animCombat;

    Mesh* animMesh; 
    //attributs
    uint8* navPath;
    int pathWidht = 15;
    int pathHeight = 15;

    int tileWidth = 1.0f;
    int tileDepth = 1.0f;
    int mapWidth;

    //Constructor
    //EntityWolf(){}
    //Constructor
    EntityWolf() : EntityEnemy() {
        navPath = new uint8[pathWidht * pathHeight];
        shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");
    };
    EntityWolf(const char* mesh, const char* tex) : EntityEnemy(mesh, tex) {
        navPath = new uint8[pathWidht * pathHeight];
        shader = Shader::Get("data/shaders/skinning.vs", "data/shaders/texture.fs");
    };

    // overwritten
    void render(float time = NULL,  bool bounding = false, float tiling = 1.0f);

    void combat(Vector3 playerPos, float seconds_elapsed);
    void search(); 
    void idle(Vector3 playerPos);

    void followTargetNavPath(Vector3 targetPos, float seconds_elapsed);
};

class EntityCannon : public EntityEnemy
{
public:
    bool isShooting = false;
    float timeBetweenShoot = TIME_SHOOT;

    //bullet
    EntityMesh* bulletEntity;
    sCannonBullet bullets[NUM_BULLETS];
    int countBullet = 0; 


    //Constructor
    EntityCannon() : EntityEnemy() {
        bulletEntity = new EntityMesh("data/enemies/bullet.obj", "data/enemies/bullet.png");
        isShooting = false;
        editAllBullets(0.0f, model);
    }

    EntityCannon(const char* mesh, const char* tex) : EntityEnemy(mesh, tex) {
        bulletEntity = new EntityMesh("data/enemies/bullet.obj", "data/enemies/bullet.png");
        isShooting = false;
        editAllBullets(0.0f, model);
    }

    void combat(Vector3 playerPos, float seconds_elapsed);
    void search();
    void idle(Vector3 playerPos);

    //bullets
    void updateBullet(float seconds_elapsed, sCannonBullet& bullet);
    void shootBullets(float seconds_elapsed);
    void editAllBullets(float ttl, Matrix44 n_model = Matrix44(), Vector3 lastPos = Vector3(), float speed = BULLET_SPEED);
    void editAllBulletsVel(Vector3 velocity);
    void renderBullets(); 
    bool checkCollisionWithBullets(Vector3 character_center);
};


#endif
