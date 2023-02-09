#include "game.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "entity.h"
#include "world.h"
#include "audio.h"
#include <bass.h>
#include "stages.h"

#include <cmath>

//some globals
float angle = 0;
float mouse_speed = 100.0f;

Game* Game::instance = NULL;

//Stages
std::vector<Stage*> stages;
STAGE_ID Stage::currentStage;
bool Stage::hasFinished;
bool Stage::exit;
EntityMesh* Stage::ground;



//gui 
Texture* Stage::panel;

Stage* GetStage(STAGE_ID id) {
	return stages[(int)id];
}

Stage* GetCurrentStage() {
	return GetStage(Stage::currentStage);
}

void SetStage(STAGE_ID id) {
	Stage::currentStage = id;
}

void InitStages() {
	stages.reserve(4);
	stages.push_back(new IntroStage("data/gui/buttons.png"));
	stages.push_back(new TutorialStage("data/gui/buttonsTutorial.png"));
	stages.push_back(new PlayStage("data/gui/buttonsPlay.png"));
	stages.push_back(new EndStage("data/gui/buttonsEnd.png"));

	((IntroStage*)GetStage(STAGE_ID::INTRO))->skeleton = new EntityWolf("data/enemies/skeleton.obj","data/enemies/skeleton.png");
	((IntroStage*)GetStage(STAGE_ID::INTRO))->skeleton->currentBehaviour = ENEMY_BEHAVIOUR::IDLE;
	((IntroStage*)GetStage(STAGE_ID::INTRO))->skeleton->animMesh = Mesh::Get("data/enemies/skeleton.mesh");
	((IntroStage*)GetStage(STAGE_ID::INTRO))->cannon = new EntityCannon("data/enemies/cannon.obj", "data/enemies/cannon.png");
	((IntroStage*)GetStage(STAGE_ID::INTRO))->cube = new EntityMesh("data/cube/grass/cube_grass.obj", "data/cube/grass/cube_grass.png");
	EntityMesh* ground = new EntityMesh(); 
	Mesh* groundMesh = new Mesh();
	groundMesh->createPlane(1000);
	ground->mesh = groundMesh;
	ground->texture = Texture::Get("data/cubes/ground.png");
	ground->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
	((IntroStage*)GetStage(STAGE_ID::INTRO))->plane = ground;

	Stage::ground = new EntityMesh(); 
	Stage::currentStage = STAGE_ID::INTRO;
	Stage::panel = Texture::Get("data/gui/grey_panel.png");
	Stage::hasFinished = false;
	Stage::currentLevel = 0;
	Stage::exit = false;
}


Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;

	InitStages();

	//OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	//create our camera
	camera = new Camera();
	camera->lookAt(Vector3(0.f,100.f, 100.f),Vector3(0.f,0.f,0.f), Vector3(0.f,1.f,0.f)); //position the camera and point to 0,0,0
	camera->setPerspective(70.f,window_width/(float)window_height,0.1f,10000.f); //set the projection, we want to be perspective
	//el ultimo numero deja ver mas lejos (far plane)
	
	// initilize stages
	GetCurrentStage()->InitEntities();
	GetCurrentStage()->loadLevels("data/levels/levels.txt");
	GetCurrentStage()->InitLevelTutorial("data/levels/tutorial_level.txt");

	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse

	//audio
	if (BASS_Init(-1, 44100, 0, 0, NULL) == false) //-1 significa usar el por defecto del sistema operativo
	{
		//error abriendo la tarjeta de sonido...
		
		std::cout << "ERROR initializing audio" << std::endl;
	}

	// init music
	Audio::Play("data/sound/background_music.wav", BASS_SAMPLE_LOOP);

}

//what to do when the image has to be draw
void Game::render(void)
{
	//set the clear color (the background color)
	glClearColor(0.0, 0.0, 0.0, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set flags
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	//set the camera as default
	camera->enable();
	

	// llamar a los renders aqui
	GetCurrentStage()->Render(camera, time);

	//render the FPS, Draw Calls, etc
	//drawText(2, 2, getGPUStats(), Vector3(1, 1, 1), 2);
			
	//swap between front buffer and back buffer
	SDL_GL_SwapWindow(this->window);

}


void Game::update(double seconds_elapsed)
{
	
	//Add here your update method
	GetCurrentStage()->Update(camera, seconds_elapsed, mouse_locked, mouse_speed, angle);
	if (Stage::exit) must_exit = true; 

}

//Keyboard event handler (sync input)
void Game::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
		case SDLK_F1: Shader::ReloadAll(); break; 		
	}
	
}

void Game::onKeyUp(SDL_KeyboardEvent event)
{
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Game::onMouseButtonDown( SDL_MouseButtonEvent event )
{	
	if (event.button == SDL_BUTTON_LEFT) {
		Input::wasLeftMousePressed = true;
	}

	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}
}

void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
	mouse_speed *= event.y > 0 ? 1.1 : 0.9;
}

void Game::onResize(int width, int height)
{
    
	std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	camera->aspect =  width / (float)height;
	window_width = width;
	window_height = height;
}

