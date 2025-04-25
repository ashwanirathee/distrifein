#include <charconv>
#include <distrifein/utils.hpp>

using namespace std;

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