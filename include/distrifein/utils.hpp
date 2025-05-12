#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>
#include <regex>
#include <optional>
#include <map>

extern std::map<int, int> id_to_port;
std::vector<int> split_peers_string(const char *str, char delim);
void generate_message_id(char out[41]);
std::optional<std::string> extract_ppm_filename(const std::string& line);
bool contains_ppm(const std::string &line);

#endif