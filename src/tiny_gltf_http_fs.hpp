//
// Created by Calxf on 5/7/2024.
//

#ifndef GAMEENGINE_TINY_GLTF_HTTP_FS_HPP
#define GAMEENGINE_TINY_GLTF_HTTP_FS_HPP

#include <vector>
#include <string>
#include <fstream>
#include <limits>

#ifdef __EMSCRIPTEN__

#include <emscripten/fetch.h>

#endif

bool ReadWholeFile(std::vector<unsigned char> *out, std::string *err,
                   const std::string &filepath, void *);

#endif //GAMEENGINE_TINY_GLTF_HTTP_FS_HPP
