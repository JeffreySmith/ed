#include "editor.h"
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <histedit.h>
#include <ios>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

void Editor::handle_sigint(int n) {
  error_msg = "Interupt";
  error = true;
}
Editor::Editor(bool verbose) {
  this->state = command;
  this->file_bytes = 0;
  this->filename = "";
  this->verbose = verbose;
  this->state = command;
  this->line_num = 1;
}
Editor::Editor(const std::string &filename, bool verbose) {
  this->file_bytes = 0;
  this->state = command;
  this->verbose = verbose;
  this->filename = filename;
  this->line_num = 1;
  std::optional<std::list<std::string>> temp = this->load_file(filename);
  if (temp.has_value()) {
    this->lines = temp.value();
    this->current_address = this->lines.begin();
    std::advance(this->current_address, 1);
    std::cout << this->file_bytes << "\n";
    std::cout << *this->current_address << "\n";
  }
}
std::optional<std::list<std::string>> Editor::load_file(std::string filename) {
  struct stat file_info;
  stat(filename.c_str(), &file_info);
  if (stat(filename.c_str(), &file_info) != 0) {
    perror(filename.c_str());
    return std::nullopt;
  }
  std::fstream FILE;
  std::list<std::string> new_list;
  // if (access(filename.c_str(), R_OK) == 0) {
  if (file_info.st_mode & S_IRUSR || file_info.st_mode & S_IRGRP ||
      file_info.st_mode & S_IROTH) {
    FILE.open(filename, std::ios::in);
    if (!FILE.is_open()) {
      return std::nullopt;
    }
    std::string input;
    while (std::getline(FILE, input)) {
      new_list.push_back(input);
    }
    this->file_bytes = file_info.st_size;
    FILE.close();
  } else {
    std::cerr << filename << ": Permission denied\n";
    return std::nullopt;
  }
  return new_list;
}
void Editor::display_error() {
  std::string prefix = "";
  if (!this->error) {
    return;
  }
  if (this->error_msg == "Interupt") {
    prefix = "\n";
  }
  if (this->verbose) {
    std::cout << prefix << error_msg << "\n";
  } else {
    std::cout << prefix << "?\n";
  }
  this->error = false;
}
std::optional<std::string> get_line(EditLine *el) {
  int bytes;
  const char *input = el_gets(el, &bytes);
  if (input == NULL) {
    return std::nullopt;
  }
  if (bytes == -1) {
    perror("Error: ");
    return std::nullopt;
  }
  std::string stripped = std::string(input);
  if (!stripped.empty() && stripped.back() == '\n') {
    stripped.pop_back();
  }
  // Because windows just has to be special
  if (!stripped.empty() && stripped.back() == '\r') {
    stripped.pop_back();
  }
  return std::string(stripped);
}
std::string get_raw_line() {
  std::string input;
  std::getline(std::cin, input);
  return input;
}
void add_to_history(History *hist, HistEvent *hv, std::string &input) {
  history(hist, hv, H_ENTER, input.c_str());
}
