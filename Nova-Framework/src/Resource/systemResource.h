#pragma once

enum SystemResourceID : unsigned long long {
// =============================== Shader =============================== 
	DEFAULT_PBR_SHADER_ID		= 1,	// don't change this value of 1!!
	DEFAULT_COLOR_SHADER_ID		= 2,	// don't change this value of 2!!

// =============================== Models =============================== 
	CUBE_MODEL_ID,
	SPHERE_MODEL_ID,
	CAPSULE_MODEL_ID,

// =============================== Material =============================== 
	DEFAULT_PBR_MATERIAL_ID,	// References DEFAULT_PBR_SHADER_ID with constant the constant 1! Don't change DEFAULT_PBR_SHADER_ID's value.
	DEFAULT_COLOR_MATERIAL_ID,	// References DEFAULT_COLOR_SHADER_ID with constant the constant 2! Don't change DEFAULT_COLOR_SHADER_ID's value.

// =============================== Texture =============================== 
	NONE_TEXTURE_ID,			// None texture id.. is not really none ;) but rather a 1x1 white texture.
	INVALID_TEXTURE_ID,			// the invalid minecraft texture
};