//
// Created by Calxf on 5/7/2024.
//

#include "tiny_gltf_http_fs.hpp"

bool ReadWholeFile(std::vector<unsigned char> *out, std::string *err,
                   const std::string &filepath, void *) {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS | EMSCRIPTEN_FETCH_REPLACE;
    emscripten_fetch_t *fetch = emscripten_fetch(&attr, filepath.c_str());
    if (fetch->status == 200) {
        out->assign(fetch->data, fetch->data + fetch->numBytes);
        emscripten_fetch_close(fetch);
        return true;
    } else {
        if (err) {
            (*err) += "Downloading " + std::string(fetch->url) + " failed, HTTP failure status code: " +
                      std::to_string(fetch->status) + ".\n";
        }
        emscripten_fetch_close(fetch);
        return false;
    }
}
