///////////////////////////////////////////////////////////////////////
//	SpeedTreeRT runtime configuration #defines
//
//	(c) 2003 IDV, Inc.
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization and may
//	not be copied or disclosed except in accordance with the terms of
//	that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//		IDV, Inc.
//		1233 Washington St. Suite 610
//		Columbia, SC 29201
//		Voice: (803) 799-1699
//		Fax:   (803) 931-0320
//		Web:   http://www.idvinc.com

#pragma once

const int c_nNumWindMatrices = 40;
const float c_fNearLodFactor = 1000000000.0f;
const float c_fFarLodFactor = 50000000000.0f;

// vertex shader constant locations
const int c_nVertexShader_LeafLightingAdjustment = 70;
const int c_nVertexShader_Light = 71;
const int c_nVertexShader_Material = 74;
const int c_nVertexShader_TreePos = 52;
const int c_nVertexShader_CompoundMatrix = 0;
const int c_nVertexShader_WindMatrices = 54;
const int c_nVertexShader_LeafTables = 4;
const int c_nVertexShader_Fog = 85;

// setup lighting (enable ONE of the two below)
#define WRAPPER_USE_STATIC_LIGHTING
//#define WRAPPER_USE_DYNAMIC_LIGHTING

#if defined WRAPPER_USE_STATIC_LIGHTING && defined WRAPPER_USE_DYNAMIC_LIGHTING
#	error Please define exactly one lighting mode
#endif

// setup wind (enable ONE of the three below)
//#define WRAPPER_USE_GPU_WIND
//#define WRAPPER_USE_CPU_WIND
#define WRAPPER_USE_NO_WIND

#if defined WRAPPER_USE_GPU_WIND && defined WRAPPER_USE_CPU_WIND
#	error Please define exactly one lighting mode
#elif defined WRAPPER_USE_GPU_WIND && defined WRAPPER_USE_NO_WIND
#	error Please define exactly one lighting mode
#elif defined WRAPPER_USE_CPU_WIND && defined WRAPPER_USE_NO_WIND
#	error Please define exactly one lighting mode
#endif

// leaf placement algorithm (enable ONE of the two below)
//#define WRAPPER_USE_GPU_LEAF_PLACEMENT
#define WRAPPER_USE_CPU_LEAF_PLACEMENT

#if defined WRAPPER_USE_GPU_LEAF_PLACEMENT && defined WRAPPER_USE_CPU_LEAF_PLACEMENT
#	error Please define exactly one leaf placement algorithm
#endif

// texture coordinates (enable this define for DirectX-based engines)
#define WRAPPER_FLIP_T_TEXCOORD

// up vector
//#define WRAPPER_UP_POS_Y



// loading from STF or clones/instances? (enable ONE of the two below)
#define WRAPPER_FOREST_FROM_STF

#if defined WRAPPER_FOREST_FROM_STF && defined WRAPPER_FOREST_FROM_INSTANCES
#	error Please define exactly one loading mechanism
#endif

// billboard modes
#define WRAPPER_BILLBOARD_MODE
//#define WRAPPER_RENDER_HORIZONTAL_BILLBOARD

// use fog
#define WRAPPER_USE_FOG

// derived constants
#ifdef WRAPPER_USE_GPU_WIND
#	define BRANCHES_USE_SHADERS
#	define FRONDS_USE_SHADERS
#	define LEAVES_USE_SHADERS
#endif

#ifdef WRAPPER_USE_GPU_LEAF_PLACEMENT
#	define LEAVES_USE_SHADERS
#endif
