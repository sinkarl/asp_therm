#include "common.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

std::string trim_str(const std::string &str) {
  if (str.empty())
    return "";
  auto wsfront = std::find_if_not(str.begin(), str.end(),
      [](int c){return std::isspace(c);});
  return std::string(wsfront, std::find_if_not(str.rbegin(),
      std::string::const_reverse_iterator(wsfront),
      [](int c){return std::isspace(c);}).base());
}

bool is_exist(const std::string &path) {
  return std::filesystem::exists(path);
}

std::string dir_by_path(const std::string &path) {
  return std::filesystem::path(path).parent_path();
}

std::string hex2str(int hex) {
  std::stringstream hex_stream;
  hex_stream << hex;
  return hex_stream.str();
}
