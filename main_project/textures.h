#pragma once
#include <glad/glad.h>

// Upload raw RGB data to a GL_TEXTURE_2D (with mipmaps + repeat wrap)
unsigned int uploadTexture(unsigned char* data, int w, int h);

// Individual procedural texture creators
unsigned int createBrickTexture();
unsigned int createGrassTexture();
unsigned int createConcreteTexture();
unsigned int createMarbleTexture();
unsigned int createWoodTexture();
unsigned int createTileTexture();

// Create all textures and store into the globals texBrick … texTile
void createAllTextures();
