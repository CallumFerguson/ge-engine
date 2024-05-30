#pragma once

// For use by GameEngine applications

#include "assets/gltfloader.hpp"

#include "engine/App.hpp"
#include "engine/Components.hpp"
#include "engine/Entity.hpp"
#include "engine/Input.hpp"
#include "engine/KeyCode.hpp"
#include "engine/Random.hpp"
#include "engine/Scene.hpp"
#include "engine/ScriptableEntity.hpp"
#include "engine/Time.hpp"
#include "engine/Window.hpp"

#include "rendering/backends/webgpu/WebGPURenderer.hpp"
#include "rendering/backends/webgpu/WebGPUShader.hpp"

#include "utility/emscriptenUtility.hpp"
#include "utility/RollingAverage.hpp"
#include "utility/utility.hpp"
