#include "engine/engine.h"
#include "engine/geometry.h"
#include "engine/strings.h"
#include "engine/engine.h"

#include "game.h"

#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "time.h"
#include <cstring>
#include <vector>

static size_t currentEntityID = 0;

void Entity_init(Entity *entity_p, Vec3f pos, Vec3f rotation, float scale, const char *modelName, const char *textureName, Vec4f color, enum EntityType type){

	entity_p->ID = currentEntityID;
	currentEntityID++;

	entity_p->pos = pos;
	entity_p->rotation = rotation;
	entity_p->scale = scale;
	entity_p->type = type;
	entity_p->color = color;
	String_set(entity_p->modelName, modelName, SMALL_STRING_SIZE);
	String_set(entity_p->textureName, textureName, SMALL_STRING_SIZE);

	String_set(entity_p->levelName, "", SMALL_STRING_SIZE);
	//entity_p->velocity = getVec3f(0.0, 0.0, 0.0);
	entity_p->velocityIndex = -1;

	entity_p->floating = false;

}

Entity *Game_getEntityByID(Game *game_p, size_t ID){

	for(int i = 0; i < game_p->entities.size(); i++){
		
		if(game_p->entities[i].ID == ID){
			return &game_p->entities[i];
		}

	}

	return NULL;

}

void Game_removeEntityByID(Game *game_p, size_t ID){

	for(int i = 0; i < game_p->entities.size(); i++){
		
		if(game_p->entities[i].ID == ID){
			game_p->entities.erase(game_p->entities.begin() + i, game_p->entities.begin() + i + 1);
			break;
		}

	}

}

void Game_addPlayer(Game *game_p, Vec3f pos){

	Entity entity;

	Entity_init(&entity, pos, getVec3f(0.0, 0.0, 0.0), 0.5, "cube", "player", PLAYER_COLOR, ENTITY_TYPE_PLAYER);

	game_p->entities.push_back(entity);

}

void Game_addObstacle(Game *game_p, Vec3f pos){

	Entity entity;

	Entity_init(&entity, pos, getVec3f(0.0, 0.0, 0.0), 0.5, "cube", "cube-borders", OBSTACLE_COLOR, ENTITY_TYPE_OBSTACLE);

	game_p->entities.push_back(entity);

}

void Game_addRock(Game *game_p, Vec3f pos){

	Entity entity;

	Entity_init(&entity, pos, getVec3f(0.0, 0.0, 0.0), 0.5, "cube", "cube-borders", ROCK_COLOR, ENTITY_TYPE_ROCK);

	game_p->entities.push_back(entity);

}

void Game_addStickyRock(Game *game_p, Vec3f pos){

	Entity entity;

	Entity_init(&entity, pos, getVec3f(0.0, 0.0, 0.0), 0.5, "cube", "cube-borders", STICKY_ROCK_COLOR, ENTITY_TYPE_STICKY_ROCK);

	game_p->entities.push_back(entity);

}

void Game_addGoal(Game *game_p, Vec3f pos){

	Entity entity;

	Entity_init(&entity, pos, getVec3f(0.0, 0.0, 0.0), 0.5, "cube", "cube-borders", GOAL_COLOR, ENTITY_TYPE_GOAL);

	game_p->entities.push_back(entity);

}

void Game_addLevelDoor(Game *game_p, Vec3f pos, const char *levelName){

	Entity entity;

	Entity_init(&entity, pos, getVec3f(0.0, 0.0, 0.0), 0.5, "cube", "cube-borders", LEVEL_DOOR_COLOR, ENTITY_TYPE_LEVEL_DOOR);

	String_set(entity.levelName, levelName, SMALL_STRING_SIZE);

	game_p->entities.push_back(entity);

}

void Game_addLevelCable(Game *game_p, Vec3f pos){

	Entity entity;

	Entity_init(&entity, pos, getVec3f(0.0, 0.0, 0.0), 0.5, "cable", "blank", LEVEL_CABLE_COLOR, ENTITY_TYPE_LEVEL_CABLE);

	game_p->entities.push_back(entity);

}

void Game_addRiser(Game *game_p, Vec3f pos){

	Entity entity;

	Entity_init(&entity, pos, getVec3f(0.0, 0.0, 0.0), 0.5, "cube", "pusher-up", RISER_COLOR, ENTITY_TYPE_RISER);

	entity.pusherDirection = DIRECTION_UP;

	game_p->entities.push_back(entity);

}

void Game_addCloner(Game *game_p, Vec3f pos){

	Entity entity;

	Entity_init(&entity, pos, getVec3f(0.0, 0.0, 0.0), 0.5, "cube", "blank", CLONER_COLOR, ENTITY_TYPE_CLONER);

	game_p->entities.push_back(entity);

}

void Particle_init(Particle *particle_p){

	particle_p->pos = getVec3f(0.0, 0.0, 0.0);
	particle_p->velocity = getVec3f(0.0, 0.0, 0.0);
	particle_p->acceleration = getVec3f(0.0, 0.0, 0.0);
	particle_p->resistance = getVec3f(1.0, 1.0, 1.0);
	particle_p->scale = 1.0;
	particle_p->color = getVec4f(1.0, 1.0, 1.0, 1.0);
	particle_p->counter = 0;

}

bool isStaticEntity(Entity entity){
	return entity.type == ENTITY_TYPE_OBSTACLE
		|| entity.type == ENTITY_TYPE_GOAL
		|| entity.type == ENTITY_TYPE_RISER
		|| entity.type == ENTITY_TYPE_CLONER
		|| entity.type == ENTITY_TYPE_LEVEL_DOOR
		|| entity.type == ENTITY_TYPE_LEVEL_CABLE;
}
