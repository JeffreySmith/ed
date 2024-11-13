#ifndef H_SHELL
#define H_SHELL
#include <optional>
#include <string>
#include <vector>
void run_command(const std::string &);
std::optional<std::string> get_command_output(std::string &);
std::optional<std::vector<std::string>> split_string(const std::string &);
#endif
