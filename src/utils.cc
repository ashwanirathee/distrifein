#include <charconv>
#include <distrifein/utils.h>
#include <uuid/uuid.h>

using namespace std;

std::map<int, int> id_to_port = {
    {0, 8000},
    {1, 8001},
    {2, 8002},
    {3, 8003},
    {4, 8004},
    {5, 8005},
    {6, 8006},
    {7, 8007},
};

std::map<int, int> port_to_id = {
    {8000, 0},
    {8001, 1},
    {8002, 2},
    {8003, 3},
    {8004, 4},
    {8005, 5},
    {8006, 6},
    {8007, 7},
};

std::vector<int> split_peers_string(const char *str, char delim)
{
    std::vector<int> tokens;
    const char *start = str;
    const char *p = str;
    while (*p)
    {
        if (*p == delim)
        {
            int val;
            std::from_chars(start, p, val);
            tokens.push_back(val);
            start = p + 1;
        }
        p++;
    }
    int val;
    std::from_chars(start, p, val);
    tokens.push_back(val);
    return tokens;
}

void generate_message_id(char out[41]) {
    uuid_t uuid;
    char uuid_str[37]; // 36 chars + null terminator
    uuid_generate_random(uuid); // random uuid
    uuid_unparse_lower(uuid, uuid_str); // convert to str in char

    // put msg- in out and finally copy uuid_str to out
    std::strcpy(out, "msg-");
    std::strcat(out, uuid_str);
}

std::optional<std::string> extract_ppm_filename(const std::string& line) {
    std::regex pattern(R"(([\w\-.]+\.ppm))", std::regex::icase);
    std::smatch match;
    if (std::regex_search(line, match, pattern)) {
        return match[1].str();  // The matched filename
    }
    return std::nullopt;  // No .ppm found
}

bool contains_ppm(const std::string &line)
{
    return line.find(".ppm") != std::string::npos;
}