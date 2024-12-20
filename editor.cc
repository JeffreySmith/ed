/*
BSD 3-Clause License

Copyright (c) 2024, Jeffrey Smith

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "editor.h"
#include <cstdio>
#include <fcntl.h>
#include <fstream>
#include <histedit.h>
#include <ios>
#include <iostream>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

void Editor::handle_sigint(int n) {
  error_msg = "Interupt";
  error = true;
}
Editor::Editor(bool verbose) {
  this->verbose = verbose;
  this->current_address = this->lines.begin();
}
Editor::Editor(const std::string &filename, bool verbose) {
  this->verbose = verbose;
  this->filename = filename;
  std::optional<std::list<std::string>> temp = this->load_file(filename);
  if (temp.has_value()) {
    this->lines = temp.value();
    this->current_address = this->lines.begin();
    std::advance(this->current_address, 1);
    std::cout << this->file_bytes << "\n";
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
  if (file_info.st_mode & S_IRUSR || file_info.st_mode & S_IRGRP ||
      file_info.st_mode & S_IROTH) {
    FILE.open(filename, std::ios::in);
    if (!FILE.is_open()) {
      return std::nullopt;
    }
    std::string input;
    while (std::getline(FILE, input)) {
      new_list.push_back(input);
      this->total_lines += 1;
    }
    this->file_bytes = file_info.st_size;
    FILE.close();
  } else {
    std::cerr << filename << ": Permission denied\n";
    return std::nullopt;
  }
  this->filename = filename;
  return new_list;
}
void Editor::valid_to_read(const std::string &filename) {
  // int64_t bytes;
  // struct stat file_info;
  if (filename.empty()) {
    this->error = true;
    this->error_msg = "No current filename";
    return;
  }
  if (access(filename.c_str(), R_OK) != 0) {
    perror((filename + ":").c_str());
    this->error = true;
    this->error_msg = "Cannot open input file";
    return;
  }
  this->lines.erase(this->lines.begin(), this->lines.end());
  this->total_lines = 0;

  std::optional<std::list<std::string>> temp = this->load_file(filename);
  if (temp.has_value()) {
    this->lines = temp.value();
  } else {
    this->error = true;
    this->error_msg = "Cannot open input file";
    return;
  }
}
std::optional<uint64_t> Editor::write() {
  int64_t bytes;
  struct stat file_info;

  if (this->filename.empty()) {
    this->error = true;
    this->error_msg = "No current filename";
    return std::nullopt;
  }

  std::ofstream FILE(this->filename);
  if (FILE.is_open()) {
    if (access(this->filename.c_str(), W_OK) == -1) {
      FILE.close();
    } else {
      for (auto it = this->lines.begin(); it != this->lines.end(); it++) {
        FILE << *it << "\n";
      }
      FILE.close();
    }
  } else {
    if (access(this->filename.c_str(), W_OK) != 0) {
      this->error = true;
      this->error_msg = "Cannot open output file";
    }

    perror((this->filename + ": ").c_str());
    return std::nullopt;
  }
  if (stat(this->filename.c_str(), &file_info) == -1) {
    this->error = true;
    this->error_msg = "Cannot open output file";
    perror((this->filename + ": ").c_str());
    return std::nullopt;
  }
  bytes = file_info.st_size;

  if (bytes < 0) {
    return std::nullopt;
  } else {
    this->edited = false;
    this->valid_to_quit = 0;
    std::cout << bytes << "\n";
    return bytes;
  }
}
void Editor::unknown_command() {
  this->error = true;
  this->error_msg = "Unknown command";
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
void Editor::display_error_once() {
  if (this->error_msg != "") {
    std::cout << this->error_msg << "\n";
  }
}
void Editor::display_all_lines(bool display_line_num) {
  uint64_t n = 1;
  for (auto s : this->lines) {
    if (display_line_num) {
      std::cout << n << "\t";
      n++;
    }
    std::cout << s << "\n";
  }
}
void Editor::display_one_line(bool display_line_num) {
  if (display_line_num) {
    std::cout << this->line_num << "\t";
  }
  std::cout << *(this->current_address) << "\n";
}
void Editor::toggle_verbose() { this->verbose = !verbose; }
void Editor::insert_line(const std::string &input) {
  this->edited = true;
  if (this->approach == prepend) {
    prepend_line(input);
  } else if (this->approach == append) {
    append_line(input);
  }
  total_lines += 1;
}
void Editor::append_line(const std::string &input) {
  if (lines.empty()) {
    this->lines.push_front(input);
    this->current_address = this->lines.begin();
    this->line_num = 1;
  } else {
    this->line_num += 1;
    this->current_address = std::next(this->current_address);
    this->current_address = this->lines.insert(this->current_address, input);
  }
}
void Editor::prepend_line(const std::string &input) {
  if (lines.empty()) {
    this->line_num += 1;
    this->approach = append;
    this->append_line(input);
  } else {
    this->line_num -= 1;
    this->current_address = this->lines.insert(this->current_address, input);
    this->approach = append;
  }
  return;
}
void Editor::goto_line(uint64_t n) {
  n -= 1;
  if (n < this->lines.size()) {
    this->line_num = n;
    int pos = 0;
    for (auto it = this->lines.begin(); it != this->lines.end(); ++it) {
      if (pos == n) {
        this->current_address = it;
        return;
      }
      pos++;
    }
  } else {
    this->error = true;
    this->error_msg = "Invalid address";
  }
}
void Editor::rel_move(int64_t n) {
  if (n < 0 && int64_t(this->line_num) - n < 0) {
    this->error = true;
    this->error_msg = "Invalid address";
  } else if (n > 0 && this->line_num + n > this->total_lines) {
    this->error = true;
    this->error_msg = "Invalid address";
  }
  std::advance(this->current_address, n);
  this->line_num += n;
}
void Editor::display_current_line(bool display_line_number) {
  if (this->lines.size() > 0) {
    this->display_one_line(display_line_number);
  } else {
    this->error = true;
    this->error_msg = "Invalid address";
  }
}
// If the file has not been edited, quit.
// If we've already told the user the file has been edited, quit.
bool Editor::check_quit() {
  bool should_quit = false;
  this->valid_to_quit++;
  if (!this->edited) {
    should_quit = true;
  } else if (this->valid_to_quit > 1) {
    should_quit = true;
  } else {
    this->error = true;
    this->error_msg = "Warning: file modified";
  }
  return should_quit;
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
  if (input == "") {
    return;
  }
  history(hist, hv, H_ENTER, input.c_str());
}
