#include "shell.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <unistd.h>
void run_command(const std::string &command) { system(command.c_str()); }

std::optional<std::string> get_command_output(std::string &command) {
  std::array<char, 2048> buffer;
  std::string output;
  command.append(" 2>&1");
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                pclose);
  if (!pipe) {
    return std::nullopt;
  }
  while (fgets(buffer.data(), 2048, pipe.get()) != NULL) {
    output += buffer.data();
  }
  return output;
}
