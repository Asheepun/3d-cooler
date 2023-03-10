#include "engine/engine.h"
#include "engine/geometry.h"
#include "engine/3d.h"
#include "engine/shaders.h"
#include "engine/renderer2d.h"
#include "engine/igui.h"
#include "engine/strings.h"
#include "engine/files.h"

#include "game.h"

#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "time.h"
#include <cstring>
#include <vector>

Game game;

Renderer2D_Renderer renderer2D;

int TERRAIN_WIDTH = 50;

float terrainScale = 100.0;
float terrainTextureScale = 10.0;

unsigned int modelShader;
unsigned int shadowMapShader;
unsigned int particleShader;

unsigned int shadowMapFBO;
unsigned int shadowMapStaticFBO;
Texture shadowMapDepthTexture;
Texture shadowMapDepthTextureStatic;
unsigned int transparentShadowMapFBO;
unsigned int transparentShadowMapStaticFBO;
Texture transparentShadowMapDepthTexture;
Texture transparentShadowMapColorTexture;
Texture transparentShadowMapDepthTextureStatic;
Texture transparentShadowMapColorTextureStatic;
int SHADOW_MAP_WIDTH = 2000;
int SHADOW_MAP_HEIGHT = 2000;
float shadowMapScale = 10.0;

float fov = M_PI / 4;

Vec3f lightPos = { -10.0, 20.0, -10.0 };
Vec3f lightDirection = { 0.5, -1.0, 0.5 };

int viewMode = 0;

void Engine_start(){

	printf("Starting the engine\n");

	Engine_setWindowSize(WIDTH / 2, HEIGHT / 2);

	Engine_centerWindow();

	Engine_setFPSMode(true);

	Renderer2D_init(&renderer2D, WIDTH, HEIGHT);

	IGUI_init(WIDTH, HEIGHT);

	//init game
	{

		game.currentGameState = GAME_STATE_EDITOR;
		//game.currentGameState = GAME_STATE_LEVEL;
		game.mustInitGameState = true;

		game.cameraPos = STANDARD_CAMERA_POS;
		game.lastCameraPos = game.cameraPos;
		game.cameraRotation = STANDARD_CAMERA_ROTATION;
		game.cameraDirection = getVec3f(0.0, 0.0, 1.0);
		game.needToRenderStaticShadows = true;

		VertexMesh_initFromFile_mesh(&game.cubeMesh,  "assets/models/untitled.mesh");

		game.hoveredEntityID = -1;

		game.playerLevelHubPos = getVec3f(0.0, 0.0, 0.0);

		game.numberOfPlayers = 0;

		SmallString level1;
		String_set(level1, "level1", SMALL_STRING_SIZE);

		game.openLevels.push_back(level1);

	}
	
	{
		Model model;

		Model_initFromFile_mesh(&model, "assets/models/untitled.mesh");

		String_set(model.name, "cube", STRING_SIZE);

		game.models.push_back(model);
	}
	{
		Model model;

		Model_initFromFile_mesh(&model, "assets/models/cable.mesh");

		String_set(model.name, "cable", STRING_SIZE);

		game.models.push_back(model);
	}
	
	/*
	{
		Model model;

		Model_initFromFile_mesh(&model, "assets/models/teapot.mesh");

		String_set(model.name, "teapot", STRING_SIZE);

		game.models.push_back(model);
	}
	*/
	
	Game_addPlayer(&game, getVec3f(0.0, 0.0, 0.0));

	for(float x = -5.0; x < 6; x += 1.0){
		for(float z = -5.0; z < 6; z += 1.0){
			Game_addObstacle(&game, getVec3f(x, -1.0, z));
		}
	}

	Game_addObstacle(&game, getVec3f(2.0, 0.0, 2.0));
	Game_addRock(&game, getVec3f(2.0, 0.0, 1.0));
	Game_addStickyRock(&game, getVec3f(-2.0, 0.0, 1.0));

	{
		Texture texture;
		Texture_initFromFile(&texture, "assets/textures/wrapped-sheep.png", "sheep");
		game.textures.push_back(texture);
	}
	{
		Texture texture;
		Texture_initFromFile(&texture, "assets/textures/cube-borders.png", "cube-borders");
		game.textures.push_back(texture);
	}
	{
		Texture texture;
		Texture_initFromFile(&texture, "assets/textures/grass.jpg", "grass");
		game.textures.push_back(texture);
	}
	{
		Texture texture;
		Texture_initFromFile(&texture, "assets/textures/blank.png", "blank");
		game.textures.push_back(texture);
	}
	{
		Texture texture;
		Texture_initFromFile(&texture, "assets/textures/player.png", "player");
		game.textures.push_back(texture);
	}
	{
		Texture texture;
		Texture_initFromFile(&texture, "assets/textures/pusher-up.png", "pusher-up");
		game.textures.push_back(texture);
	}
	{
		Texture texture;
		Texture_initFromFile(&texture, "assets/textures/pusher-north.png", "pusher-north");
		game.textures.push_back(texture);
	}
	{
		Texture texture;
		Texture_initFromFile(&texture, "assets/textures/pusher-south.png", "pusher-south");
		game.textures.push_back(texture);
	}
	{
		Texture texture;
		Texture_initFromFile(&texture, "assets/textures/pusher-east.png", "pusher-east");
		game.textures.push_back(texture);
	}
	{
		Texture texture;
		Texture_initFromFile(&texture, "assets/textures/pusher-west.png", "pusher-west");
		game.textures.push_back(texture);
	}

	{
		const char *pathsAndNames[] = {
			"assets/textures/cube-borders.png", "cube-borders",
			"assets/textures/blank.png", "blank",
			"assets/textures/player.png", "player",
			"assets/textures/pusher-up.png", "pusher-up",
			"assets/textures/pusher-north.png", "pusher-north",
			"assets/textures/pusher-south.png", "pusher-south",
			"assets/textures/pusher-east.png", "pusher-east",
			"assets/textures/pusher-west.png", "pusher-west",
		};

		int numberOfPaths = sizeof(pathsAndNames) / sizeof(const char *);
		numberOfPaths /= 2;

		TextureAtlas_init(&game.textureAtlas, pathsAndNames, numberOfPaths);
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	//compile shaders
	{
		unsigned int vertexShader = getCompiledShader("shaders/vertex-shader.glsl", GL_VERTEX_SHADER);
		unsigned int fragmentShader = getCompiledShader("shaders/fragment-shader.glsl", GL_FRAGMENT_SHADER);

		unsigned int shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);

		modelShader = shaderProgram;
	}

	{
		unsigned int vertexShader = getCompiledShader("shaders/shadow-map-vertex-shader.glsl", GL_VERTEX_SHADER);
		unsigned int fragmentShader = getCompiledShader("shaders/shadow-map-fragment-shader.glsl", GL_FRAGMENT_SHADER);

		unsigned int shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);

		shadowMapShader = shaderProgram;
	}
	{
		unsigned int vertexShader = getCompiledShader("shaders/particle-vertex-shader.glsl", GL_VERTEX_SHADER);
		unsigned int fragmentShader = getCompiledShader("shaders/particle-fragment-shader.glsl", GL_FRAGMENT_SHADER);

		unsigned int shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);

		particleShader = shaderProgram;
	}

	//generate shadow map depth texture
	Texture_initAsDepthMap(&shadowMapDepthTexture, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

	//generate shadow map frame buffer
	glGenFramebuffers(1, &shadowMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapDepthTexture.ID, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	//generate static shadow map depth texture
	Texture_initAsDepthMap(&shadowMapDepthTextureStatic, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

	//generate static shadow map frame buffer
	glGenFramebuffers(1, &shadowMapStaticFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapStaticFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapDepthTextureStatic.ID, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	Texture_initAsDepthMap(&transparentShadowMapDepthTexture, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
	Texture_initAsColorMap(&transparentShadowMapColorTexture, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

	//generate transparent shadow map frame buffer
	glGenFramebuffers(1, &transparentShadowMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, transparentShadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, transparentShadowMapDepthTexture.ID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, transparentShadowMapColorTexture.ID, 0);

	Texture_initAsDepthMap(&transparentShadowMapDepthTextureStatic, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
	Texture_initAsColorMap(&transparentShadowMapColorTextureStatic, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);

	//generate transparent shadow map frame buffer
	glGenFramebuffers(1, &transparentShadowMapStaticFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, transparentShadowMapStaticFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, transparentShadowMapDepthTextureStatic.ID, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, transparentShadowMapColorTextureStatic.ID, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);  

}

int gameTime = 0;

void Engine_update(float deltaTime){

	IGUI_updatePointerScale();

	if(!game.levelNameTextInputData.focused
	&& !game.levelDoorNameTextInputData.focused){
		if(Engine_keys[ENGINE_KEY_Q].down){
			Engine_quit();
		}
		if(Engine_keys[ENGINE_KEY_F].downed){
			Engine_toggleFullscreen();
		}
		if(Engine_keys[ENGINE_KEY_V].downed){
			viewMode++;
		}
		if(viewMode > 1){
			viewMode = 0;
		}
	}

	if(game.mustInitGameState){

		if(game.currentGameState == GAME_STATE_LEVEL){
			Game_initLevelState(&game);
		}else if(game.currentGameState == GAME_STATE_EDITOR){
			Game_initEditorState(&game);
		}else if(game.currentGameState == GAME_STATE_MENU){
			Game_initMenuState(&game);
		}
	
		game.mustInitGameState = false;
	}
	if(game.currentGameState == GAME_STATE_LEVEL){
		Game_levelState(&game);
	}else if(game.currentGameState == GAME_STATE_EDITOR){
		Game_editorState(&game);
	}else if(game.currentGameState == GAME_STATE_MENU){
		Game_menuState(&game);
	}

	//set camera direction based on camera rotation
	game.cameraDirection.y = sin(game.cameraRotation.y);
	game.cameraDirection.x = cos(game.cameraRotation.x) * cos(game.cameraRotation.y);
	game.cameraDirection.z = sin(game.cameraRotation.x) * cos(game.cameraRotation.y);
	Vec3f_normalize(&game.cameraDirection);

}

clock_t startTicks = 0;
clock_t endTicks = 0;

void Engine_draw(){

	//setup texture buffers with model matrices and model rotation matrices
	struct TextureBufferContainer{
		std::vector<Mat4f> modelMatrices;
		TextureBuffer modelMatricesTextureBuffer;
		std::vector<Mat4f> modelRotationMatrices;
		TextureBuffer modelRotationMatricesTextureBuffer;
		std::vector<Vec4f> inputColors;
		TextureBuffer inputColorsTextureBuffer;
		std::vector<Vec4f> textureAtlasCoordinates;
		TextureBuffer textureAtlasCoordinatesTextureBuffer;
		int numberOfInstances;
	};

	TextureBufferContainer textureBufferContainers[2][game.models.size()];

	//init texture buffer containers
	for(int i = 0; i < 2; i++){
		for(int j = 0; j < game.models.size(); j++){
			textureBufferContainers[i][j].numberOfInstances = 0;
		}
	}

	//get texture buffer data from entities
	for(int i = 0; i < game.entities.size(); i++){

		Entity *entity_p = &game.entities[i];

		TextureBufferContainer *textureBufferContainer_p;

		int modelIndex = 0;
		for(int j = 0; j < game.models.size(); j++){
			if(strcmp(game.models[j].name, entity_p->modelName) == 0){
				modelIndex = j;
			}
		}

		if(entity_p->color.w < 1.0){
			textureBufferContainer_p = &textureBufferContainers[1][modelIndex];
		}else{
			textureBufferContainer_p = &textureBufferContainers[0][modelIndex];
		}

		Mat4f modelMat4f = getIdentityMat4f();

		Mat4f_mulByMat4f(&modelMat4f, getTranslationMat4f(entity_p->pos.x, entity_p->pos.y, entity_p->pos.z));

		Mat4f_mulByMat4f(&modelMat4f, getScalingMat4f(entity_p->scale));

		textureBufferContainer_p->modelMatrices.push_back(modelMat4f);

		Mat4f modelRotationMat4f = getIdentityMat4f();

		Mat4f_mulByMat4f(&modelRotationMat4f, getRotationMat4f(entity_p->rotation.x, entity_p->rotation.y, entity_p->rotation.z));

		textureBufferContainer_p->modelRotationMatrices.push_back(modelRotationMat4f);

		Vec4f inputColor = entity_p->color;

		if(entity_p->ID == game.hoveredEntityID){
			inputColor.x *= 0.5;
			inputColor.y *= 0.5;
			inputColor.z *= 0.5;
		}

		textureBufferContainer_p->inputColors.push_back(inputColor);

		Vec4f textureAtlasCoordinates = getVec4f(0.0, 0.0, 1.0, 1.0);
		for(int j = 0; j < game.textureAtlas.names.size(); j++){
			if(strcmp(entity_p->textureName, game.textureAtlas.names[j]) == 0){
				textureAtlasCoordinates = game.textureAtlas.textureCoordinates[j];
			}
		}
		
		textureBufferContainer_p->textureAtlasCoordinates.push_back(textureAtlasCoordinates);

		textureBufferContainer_p->numberOfInstances++;
	
	}

	//generate VBOs and texture buffers
	for(int i = 0; i < 2; i++){
		for(int j = 0; j < game.models.size(); j++){

			TextureBufferContainer *textureBufferContainer_p = &textureBufferContainers[i][j];

			TextureBuffer_init(
				&textureBufferContainer_p->modelMatricesTextureBuffer,
				&textureBufferContainer_p->modelMatrices[0],
				sizeof(Mat4f) * textureBufferContainer_p->modelMatrices.size()
			);

			TextureBuffer_init(
				&textureBufferContainer_p->modelRotationMatricesTextureBuffer,
				&textureBufferContainer_p->modelRotationMatrices[0],
				sizeof(Mat4f) * textureBufferContainer_p->modelRotationMatrices.size()
			);

			TextureBuffer_init(
				&textureBufferContainer_p->inputColorsTextureBuffer,
				&textureBufferContainer_p->inputColors[0],
				sizeof(Vec4f) * textureBufferContainer_p->inputColors.size()
			);

			TextureBuffer_init(
				&textureBufferContainer_p->textureAtlasCoordinatesTextureBuffer,
				&textureBufferContainer_p->textureAtlasCoordinates[0],
				sizeof(Vec4f) * textureBufferContainer_p->textureAtlasCoordinates.size()
			);

		}
	
	}

	glViewport(0, 0, Engine_clientWidth, Engine_clientHeight);

	//setup camera matrix
	Mat4f cameraMat4f = getLookAtMat4f(game.cameraPos, game.cameraDirection);

	//setup perspective matrix
	Mat4f perspectiveMat4f = getPerspectiveMat4f(fov, (float)Engine_clientWidth / (float)Engine_clientHeight);

	//setup shadow map camera matrix
	Vec3f_normalize(&lightDirection);
	Mat4f lightCameraMat4f = getLookAtMat4f(lightPos, lightDirection);

	//draw shadow map
	unsigned int currentShaderProgram = shadowMapShader;

	glUseProgram(currentShaderProgram);

	//set common uniforms
	GL3D_uniformMat4f(currentShaderProgram, "cameraMatrix", lightCameraMat4f);
	GL3D_uniformFloat(currentShaderProgram, "shadowMapScale", shadowMapScale);
	GL3D_uniformVec3f(currentShaderProgram, "lightDirection", lightDirection);
	
	//draw entities instanced to shadow map
	for(int i = 0; i < 2; i++){

		if(i == 0){
			glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);  
			glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
			glClearColor(1.0, 1.0, 1.0, 1.0);
		}
		if(i == 1){
			glBindFramebuffer(GL_FRAMEBUFFER, transparentShadowMapFBO);  
			glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
			glClearColor(0.0, 0.0, 0.0, 0.0);
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for(int j = 0; j < game.models.size(); j++){

			TextureBufferContainer *textureBufferContainer_p = &textureBufferContainers[i][j];

			Model *model_p = &game.models[j];

			glBindBuffer(GL_ARRAY_BUFFER, model_p->VBO);
			glBindVertexArray(model_p->VAO);

			GL3D_uniformTextureBuffer(currentShaderProgram, "modelMatrixTextureBuffer", 4, textureBufferContainer_p->modelMatricesTextureBuffer.TB);
			GL3D_uniformTextureBuffer(currentShaderProgram, "modelRotationMatrixTextureBuffer", 5, textureBufferContainer_p->modelRotationMatricesTextureBuffer.TB);
			GL3D_uniformTextureBuffer(currentShaderProgram, "inputColorTextureBuffer", 6, textureBufferContainer_p->inputColorsTextureBuffer.TB);

			glDrawArraysInstanced(GL_TRIANGLES, 0, model_p->numberOfTriangles * 3, textureBufferContainer_p->numberOfInstances);

		}
	}

	//draw world
	if(viewMode == 0){

		//bind and clear world buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);  
		glViewport(0, 0, Engine_clientWidth, Engine_clientHeight);

		glCullFace(GL_FRONT);

		startTicks = clock();

		glClearColor(0.5, 0.5, 0.9, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//set common uniforms
		unsigned int currentShaderProgram = modelShader;

		glUseProgram(currentShaderProgram);

		GL3D_uniformTexture(currentShaderProgram, "colorTexture", 0, game.textureAtlas.texture.ID);
		GL3D_uniformTexture(currentShaderProgram, "shadowMapDepthTexture", 1, shadowMapDepthTexture.ID);
		GL3D_uniformTexture(currentShaderProgram, "transparentShadowMapDepthTexture", 2, transparentShadowMapDepthTexture.ID);
		GL3D_uniformTexture(currentShaderProgram, "transparentShadowMapColorTexture", 3, transparentShadowMapColorTexture.ID);

		GL3D_uniformMat4f(currentShaderProgram, "modelMatrix", getIdentityMat4f());
		GL3D_uniformMat4f(currentShaderProgram, "modelRotationMatrix", getIdentityMat4f());
		GL3D_uniformMat4f(currentShaderProgram, "cameraMatrix", cameraMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "perspectiveMatrix", perspectiveMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "lightCameraMatrix", lightCameraMat4f);
		GL3D_uniformFloat(currentShaderProgram, "shadowMapScale", shadowMapScale);
		GL3D_uniformVec3f(currentShaderProgram, "lightDirection", lightDirection);

		//draw entities instanced to world
		for(int i = 0; i < 2; i++){
			for(int j = 0; j < game.models.size(); j++){

				TextureBufferContainer *textureBufferContainer_p = &textureBufferContainers[i][j];

				Model *model_p = &game.models[j];

				glBindBuffer(GL_ARRAY_BUFFER, model_p->VBO);
				glBindVertexArray(model_p->VAO);

				GL3D_uniformTextureBuffer(currentShaderProgram, "modelMatrixTextureBuffer", 4, textureBufferContainer_p->modelMatricesTextureBuffer.TB);
				GL3D_uniformTextureBuffer(currentShaderProgram, "modelRotationMatrixTextureBuffer", 5, textureBufferContainer_p->modelRotationMatricesTextureBuffer.TB);
				GL3D_uniformTextureBuffer(currentShaderProgram, "inputColorTextureBuffer", 6, textureBufferContainer_p->inputColorsTextureBuffer.TB);
				GL3D_uniformTextureBuffer(currentShaderProgram, "textureAtlasCoordinatesTextureBuffer", 7, textureBufferContainer_p->textureAtlasCoordinatesTextureBuffer.TB);

				glDrawArraysInstanced(GL_TRIANGLES, 0, model_p->numberOfTriangles * 3, textureBufferContainer_p->numberOfInstances);

			}
		}

	}

	//free texture buffers and vertex buffer objects
	for(int i = 0; i < 2; i++){
		for(int j = 0; j < game.models.size(); j++){

			TextureBufferContainer *textureBufferContainer_p = &textureBufferContainers[i][j];

			TextureBuffer_free(&textureBufferContainer_p->modelMatricesTextureBuffer);
			TextureBuffer_free(&textureBufferContainer_p->modelRotationMatricesTextureBuffer);
			TextureBuffer_free(&textureBufferContainer_p->inputColorsTextureBuffer);
			TextureBuffer_free(&textureBufferContainer_p->textureAtlasCoordinatesTextureBuffer);

		}
	}

	//draw particles
	{
		//setup texture particle buffers for instancing
		std::vector<Mat4f> modelMatrices;
		TextureBuffer modelMatricesTextureBuffer;

		for(int i = 0; i < game.particles.size(); i++){

			Particle *particle_p = &game.particles[i];

			Mat4f modelMat4f = getIdentityMat4f();

			Mat4f_mulByMat4f(&modelMat4f, getTranslationMat4f(particle_p->pos.x, particle_p->pos.y, particle_p->pos.z));

			Mat4f_mulByMat4f(&modelMat4f, getScalingMat4f(particle_p->scale));

			modelMatrices.push_back(modelMat4f);

		}

		TextureBuffer_init(&modelMatricesTextureBuffer, &modelMatrices[0], sizeof(Mat4f) * modelMatrices.size());

		//set shader program and common uniforms
		unsigned int currentShaderProgram = particleShader;

		glUseProgram(currentShaderProgram);

		Vec4f color = getVec4f(1.0, 1.0, 1.0, 1.0);

		Model *model_p;

		for(int k = 0; k < game.models.size(); k++){
			if(strcmp("cube", game.models[k].name) == 0){
				model_p = &game.models[k];
			}
		}


		glBindBuffer(GL_ARRAY_BUFFER, model_p->VBO);
		glBindVertexArray(model_p->VAO);

		Mat4f modelRotationMat4f = getIdentityMat4f();

		GL3D_uniformMat4f(currentShaderProgram, "modelRotationMatrix", modelRotationMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "cameraMatrix", cameraMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "perspectiveMatrix", perspectiveMat4f);
		GL3D_uniformMat4f(currentShaderProgram, "lightCameraMatrix", lightCameraMat4f);
		GL3D_uniformFloat(currentShaderProgram, "shadowMapScale", shadowMapScale);
		GL3D_uniformVec3f(currentShaderProgram, "lightDirection", lightDirection);
		GL3D_uniformVec4f(currentShaderProgram, "inputColor", color);

		GL3D_uniformTextureBuffer(currentShaderProgram, "modelMatrixTextureBuffer", 4, modelMatricesTextureBuffer.TB);

		glDrawArraysInstanced(GL_TRIANGLES, 0, model_p->numberOfTriangles * 3, game.particles.size());

	}

	//draw HUD
	glDisable(GL_DEPTH_TEST);

	Renderer2D_updateDrawSize(&renderer2D, Engine_clientWidth, Engine_clientHeight);

	if(game.currentGameState == GAME_STATE_EDITOR){
		Renderer2D_drawColoredRectangle(&renderer2D, WIDTH / 2 - 3, HEIGHT / 2 - 3, 6, 6, Renderer2D_getColor(0.7, 0.7, 0.7), 1.0);
	}

	//draw menu background
	if(game.currentGameState == GAME_STATE_MENU){
		Renderer2D_drawColoredRectangle(&renderer2D, 0, 0, WIDTH, HEIGHT, Renderer2D_getColor(0.0, 0.0, 0.0), 1.0);
	}

	IGUI_render(&renderer2D);

	glEnable(GL_DEPTH_TEST);

	endTicks = clock();

	clock_t worldTicks = endTicks - startTicks;

	//printf("\nRENDER TIMES\n");
	//printf("shadow map: %f ms\n", (float)(shadowMapTicks / (float)(CLOCKS_PER_SEC / 1000)));
	//printf("world:      %f ms\n", (float)(worldTicks / (float)(CLOCKS_PER_SEC / 1000)));

}

void Engine_finnish(){

	char text[STRING_SIZE];
	String_set(text, game.levelNameTextInputData.text, STRING_SIZE);
	String_append(text, "}");

	writeDataToFile("lastOpenedLevelName.txt", text, strlen(text));

}
