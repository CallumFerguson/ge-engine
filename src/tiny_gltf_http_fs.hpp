#pragma  once

#include <vector>
#include <string>

bool FileExists(const std::string &abs_filename, void *);

std::string ExpandFilePath(const std::string &filepath, void *userdata);

bool ReadWholeFile(std::vector<unsigned char> *out, std::string *err, const std::string &filepath, void *);

bool WriteWholeFile(std::string *err, const std::string &filepath, const std::vector<unsigned char> &contents, void *);

bool GetFileSizeInBytes(size_t *filesize_out, std::string *err, const std::string &filepath, void *);
