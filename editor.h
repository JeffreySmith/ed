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

#ifndef H_EDITOR
#define H_EDITOR
#include <csignal>
#include <histedit.h>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <vector>
enum State {
  insert,
  command,
};
enum Approach {
  prepend,
  append,
};

class Editor {
  inline static std::string error_msg = "";
  inline static bool error = false;
  int file_bytes = 0;
  std::string filename = "";
  std::list<std::string> lines;
  bool verbose = false;
  bool edited = false;
  uint64_t valid_to_quit = 0;
  uint64_t line_num = 1;
  uint64_t total_lines = 0;
  std::list<std::string>::iterator current_address;
  std::map<std::string, std::vector<std::string>> registers;

  std::optional<std::list<std::string>> load_file(std::string filename);
  void display_one_line(bool line_number);

public:
  State state = command;
  Approach approach = prepend;

  explicit Editor(bool);
  explicit Editor(const std::string &, bool);
  static void handle_sigint(int);
  void display_all_lines(bool display_line_number = false);
  void append_line(const std::string &input);
  void prepend_line(const std::string &input);
  void insert_line(const std::string &input);
  void display_error();
  void display_error_once();
  void unknown_command();
  void goto_line(uint64_t n);
  void rel_move(int64_t n);
  void display_current_line(bool display_line_number);
  void toggle_verbose();
  bool check_quit();
  std::optional<uint64_t> write();
};

std::optional<std::string> get_line(EditLine *);
std::string get_raw_line();
void add_to_history(History *, HistEvent *, std::string &);

#endif
