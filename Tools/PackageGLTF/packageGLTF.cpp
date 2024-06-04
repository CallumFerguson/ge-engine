#include <iostream>
#include <fstream>
#include <string>
#include <tiny_gltf.h>
#include <nlohmann/json.hpp>
#include "GameEngine.hpp"

tinygltf::TinyGLTF loader;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file_path> <output_file_path>" << std::endl;
        return 1;
    }

    std::string inputFilePath = argv[1];
    std::string outputFilePath = argv[2];

    tinygltf::Model model;
    std::string err;
    std::string warn;

    bool result = loader.LoadBinaryFromFile(&model, &err, &warn, inputFilePath);
    if (!warn.empty()) {
        std::cout << "loadModel warning: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cerr << "loadModel error: " << err << std::endl;
    }
    if (!result) {
        std::cerr << "loadModel ailed to load GLTF model." << std::endl;
        return 1;
    }

    if (model.meshes.empty()) {
        std::cout << "Model does not contain any meshes." << std::endl;
        return 1;
    }

    auto &mesh = model.meshes[0];

    GameEngine::writeGLTFMeshToFile(model, mesh, outputFilePath);

    nlohmann::json j2 = {
            {"pi",      3.141},
            {"happy",   true},
            {"name",    "Niels"},
            {"nothing", nullptr},
            {"answer",  {
                                {"everything", 42}
                        }},
            {"list",    {       1, 0, 2}},
            {"object",  {
                                {"currency",   "USD"},
                                   {"value", 42.99}
                        }}
    };

//    std::ofstream outputFile(outputFilePath, std::ios::out);
//    outputFile << j2.dump(4);
}
