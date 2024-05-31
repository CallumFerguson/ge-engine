#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file_path> <output_file_path>" << std::endl;
        return 1;
    }

    std::string inputFilePath = argv[1];
    std::string outputFilePath = argv[2];

    std::ifstream inputFile(inputFilePath);
    if (!inputFile.is_open()) {
        std::cerr << "Could not open the input file: " << inputFilePath << std::endl;
        return 1;
    }

    std::ofstream outputFile(outputFilePath);
    if (!outputFile.is_open()) {
        std::cerr << "Could not open the output file: " << outputFilePath << std::endl;
        inputFile.close();
        return 1;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        outputFile << line << "test" << std::endl;
    }

    inputFile.close();
    outputFile.close();

    std::cout << "File processing complete. The modified content has been written to " << outputFilePath << std::endl;
    return 0;
}
