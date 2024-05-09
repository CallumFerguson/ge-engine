#include "tiny_gltf_http_fs.hpp"

#include <emscripten/fetch.h>
#include <string>
#include <algorithm>
#include <cctype>

bool FileExists(const std::string &abs_filename, void *) {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "HEAD");
    attr.attributes = EMSCRIPTEN_FETCH_SYNCHRONOUS | EMSCRIPTEN_FETCH_REPLACE;
    emscripten_fetch_t *fetch = emscripten_fetch(&attr, abs_filename.c_str());
    bool fileExists = fetch->status == 200;
    emscripten_fetch_close(fetch);
    return fileExists;
}

std::string ExpandFilePath(const std::string &filepath, void *userdata) {
    return filepath;
}

bool ReadWholeFile(std::vector<unsigned char> *out, std::string *err, const std::string &filepath, void *) {
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

bool WriteWholeFile(std::string *err, const std::string &filepath, const std::vector<unsigned char> &contents, void *) {
    if (err) {
        (*err) += "Writing files is not supported with Emscripten\n";
    }
    return false;
}

bool GetFileSizeInBytes(size_t *filesize_out, std::string *err, const std::string &filepath, void *) {
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "HEAD");
    attr.attributes = EMSCRIPTEN_FETCH_SYNCHRONOUS | EMSCRIPTEN_FETCH_REPLACE;
    emscripten_fetch_t *fetch = emscripten_fetch(&attr, filepath.c_str());
    if (fetch->status == 200) {
        auto headersLength = emscripten_fetch_get_response_headers_length(fetch);

        std::string headersText(headersLength + 1, '\0');
        emscripten_fetch_get_response_headers(fetch, &headersText[0], headersLength + 1);

        char **headers = emscripten_fetch_unpack_response_headers(headersText.c_str());
        int i = 0;
        while (headers[i] != nullptr) {
            std::string key(headers[i]);

            std::transform(key.begin(), key.end(), key.begin(),
                           [](unsigned char c) { return std::tolower(c); });

            if (key == "content-length") {
                std::string value(headers[i + 1]);
                try {
                    size_t contentLength = std::stoi(value);
                    *filesize_out = contentLength;
                } catch (const std::exception &e) {
                    if (err) {
                        (*err) += "Found Content-Length in header but failed to parse value of:\n" + value +
                                  "\nwith error:\n" +
                                  std::string(e.what()) + "\n";
                    }
                    emscripten_fetch_free_unpacked_response_headers(headers);
                    emscripten_fetch_close(fetch);
                    return false;
                }
                break;
            }

            i += 2;
        }

        emscripten_fetch_free_unpacked_response_headers(headers);

        emscripten_fetch_close(fetch);
        return true;
    } else {
        emscripten_fetch_close(fetch);
        return false;
    }
}
