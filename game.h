#ifndef GAME_H_
#define GAME_H_

#include "engine/geometry.h"
#include "engine/3d.h"
#include "engine/igui.h"
#include "engine/strings.h"

#include <vector>

#include "math.h"

enum GameState{
	GAME_STATE_LEVEL,
	GAME_STATE_EDITOR,
};

enum EntityType{
	ENTITY_TYPE_OBSTACLE,
	ENTITY_TYPE_ROCK,
	ENTITY_TYPE_STICKY_ROCK,
	ENTITY_TYPE_GOAL,
	ENTITY_TYPE_PLAYER,
	ENTITY_TYPE_LEVEL_DOOR,
	NUMBER_OF_ENTITY_TYPES,
};

typedef struct Entity{
	size_t ID;
	int playerID;
	enum EntityType type;
	Vec3f pos;
	Vec3f startPos;
	Vec3f endPos;
	Vec3f rotation;
	Vec3f velocity;
	float scale;
	char modelName[SMALL_STRING_SIZE];
	char textureName[SMALL_STRING_SIZE];
	char levelName[SMALL_STRING_SIZE];
	Vec4f color;
}Entity;

typedef struct Game{

	std::vector<Entity> entities;

	Vec3f playerLevelHubPos;

	int numberOfPlayers;

	VertexMesh cubeMesh;

	std::vector<Model> models;
	std::vector<Texture> textures;

	enum GameState currentGameState;
	bool mustInitGameState;

	Vec3f cameraPos;
	Vec3f lastCameraPos;
	Vec2f cameraRotation;
	Vec3f cameraDirection;

	size_t hoveredEntityID;
	//char currentLevel[STRING_SIZE];
	IGUI_TextInputData levelNameTextInputData;
	IGUI_TextInputData levelDoorNameTextInputData;

}Game;

//GLOBAL VARIABLES

static int WIDTH = 1920;
static int HEIGHT = 1080;

static float PLAYER_SPEED = 0.10;
static float PLAYER_LOOK_SPEED = 0.0015;

static Vec4f PLAYER_COLOR = { 0.1, 0.1, 0.9, 1.0 };
static Vec4f OBSTACLE_COLOR = { 0.8, 0.6, 0.3, 1.0 };
static Vec4f ROCK_COLOR = { 0.7, 0.7, 0.7, 1.0 };
static Vec4f STICKY_ROCK_COLOR = { 0.1, 1.0, 0.2, 1.0 };
static Vec4f GOAL_COLOR = { 0.1, 0.1, 0.9, 0.5 };
static Vec4f LEVEL_DOOR_COLOR = { 1.0, 1.0, 1.0, 0.5 };

//static Vec3f STANDARD_CAMERA_POS = { 0.0, 6.0, -6.0 };
//static Vec2f STANDARD_CAMERA_ROTATION = { M_PI / 2.0, -M_PI / 4.0 };
static Vec3f STANDARD_CAMERA_POS = { 0.0, 8.0, -8.0 };
static Vec2f STANDARD_CAMERA_ROTATION = { M_PI / 2.0, -M_PI / 4.0 };

static const char *ENTITY_TYPE_NAMES[] = {
	"Obstacle",
	"Rock",
	"Sticky Rock",
	"Goal",
	"Player",
	"Level Door",
};

//FILE: world.cpp

void Entity_init(Entity *, Vec3f, Vec3f, float, const char *, const char *, Vec4f, enum EntityType);
Entity *Game_getEntityByID(Game *, size_t);
void Game_removeEntityByID(Game *, size_t);

void Game_addPlayer(Game *, Vec3f);
void Game_addObstacle(Game *, Vec3f);
void Game_addRock(Game *, Vec3f);
void Game_addStickyRock(Game *, Vec3f);
void Game_addGoal(Game *, Vec3f);
void Game_addLevelDoor(Game *, Vec3f, const char *);

//FILE: levelState.cpp

void Game_initLevelState(Game *);

void Game_levelState(Game *);

//FILE: editorState.cpp

void Game_initEditorState(Game *);

void Game_editorState(Game *);

//FILE: levels.cpp

void Game_writeCurrentLevelStateToFile(Game *, const char *);

void Game_loadLevelFile(Game *, const char *);

void Game_loadLevelByName(Game *, const char *);

#endif
