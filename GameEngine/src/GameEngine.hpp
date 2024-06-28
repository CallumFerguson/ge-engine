#pragma once

// For use by GameEngine applications

#include "GameEngine/Assets/AssetManager.hpp"

#include "GameEngine/Core/App.hpp"
#include "GameEngine/Core/Input.hpp"
#include "GameEngine/Core/KeyCode.hpp"
#include "GameEngine/Core/Time.hpp"
#include "GameEngine/Core/Window.hpp"
#include "GameEngine/Core/Exit.hpp"

#include "GameEngine/Scene/Components.hpp"
#include "GameEngine/Scene/Entity.hpp"
#include "GameEngine/Scene/Scene.hpp"
#include "GameEngine/Scene/ScriptableEntity.hpp"
#include "GameEngine/Scene/prefab.hpp"

#include "GameEngine/Rendering/Backends/WebGPU/WebGPURenderer.hpp"
#include "GameEngine/Rendering/Backends/WebGPU/WebGPUShader.hpp"
#include "GameEngine/Rendering/Backends/WebGPU/generateMipmapWebGPU.hpp"
#include "GameEngine/Rendering/Backends/WebGPU/textureUtils.hpp"
#include "GameEngine/Rendering/Mesh.hpp"
#include "GameEngine/Rendering/Material.hpp"
#include "GameEngine/Rendering/CubeMap.hpp"
#include "GameEngine/Rendering/EnvironmentMap.hpp"

#include "GameEngine/Utility/emscriptenUtility.hpp"
#include "GameEngine/Utility/RollingAverage.hpp"
#include "GameEngine/Utility/utility.hpp"
#include "GameEngine/Utility/Random.hpp"
#include "GameEngine/Utility/TimingHelper.hpp"

#include "GameEngine/Utility/Stream/StreamWriter.hpp"
#include "GameEngine/Utility/Stream/StreamReader.hpp"
#include "GameEngine/Utility/Stream/FileStreamWriter.hpp"
#include "GameEngine/Utility/Stream/FileStreamReader.hpp"
