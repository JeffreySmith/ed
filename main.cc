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
#include "shell.h"
#include <algorithm>
#include <cctype>
#include <csignal>
#include <cstdlib>
#include <err.h>
#include <histedit.h>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unistd.h>

static std::string g_prompt = "";

static void usage(const std::string &name) {
  std::cerr << "Usage: " << name << " [-v] [-p string] [file]\n";
}
static const char *set_prompt(EditLine *el) { return g_prompt.c_str(); }

int main(int argc, char **argv) {
#ifdef HAVE_PLEDGE
  if (pledge("stdio rpath wpath cpath exec tty proc", NULL)) {
    err(1, "pledge");
  }
#endif
  std::unique_ptr<EditLine, decltype(&el_end)> el(
      el_init("ed++", stdin, stdout, stderr), &el_end);
  if (el == NULL) {
    std::cerr << "Error initializing editline\n";
    return 1;
  }
  std::unique_ptr<History, decltype(&history_end)> hist(history_init(),
                                                        &history_end);
  if (hist == NULL) {
    std::cerr << "Error initializing history\n";
    return 1;
  }
  HistEvent hv;
  int ch;
  bool verbose = false;
  std::string filename = "";
  std::string editline_editor = "emacs";
  std::unique_ptr<Editor> editor;
  while ((ch = getopt(argc, argv, "vp:")) != -1) {
    switch (ch) {
    case 'v':
      verbose = true;
      break;
    case 'p':
      g_prompt = optarg;
      break;
    case '?':
      usage(argv[0]);
      return 1;
    }
  }
  if (optind < argc) {
    if (std::string(argv[optind]) == "-") {
      std::cerr << "Using '-' with " << argv[0]
                << " for scripting is not supported\n";
      usage(argv[0]);
      return 1;
    }
    filename = argv[optind];
  }

  if (filename == "") {
    editor = std::make_unique<Editor>(verbose);
  } else {
    editor = std::make_unique<Editor>(filename, verbose);
  }
  signal(SIGINT, editor->handle_sigint);

  if (const char *env_editor = std::getenv("EDITOR")) {
    if (strstr(env_editor, "vi") != NULL) {
      editline_editor = "vi";
    }
  }

  history(hist.get(), &hv, H_SETSIZE, 100);
  history(hist.get(), &hv, H_LAST);
  el_set(el.get(), EL_HIST, history, hist.get());
  el_set(el.get(), EL_PROMPT, set_prompt);
  el_set(el.get(), EL_EDITOR, editline_editor.c_str());
  el_set(el.get(), EL_SIGNAL, 1);

  while (true) {
    // Run the program here
    std::optional<std::string> line;
    if (editor->state == command) {
      line = get_line(el.get());
    } else if (editor->state == insert) {
      std::string local_line = get_raw_line();
      line = local_line;
    }
    if (line.has_value()) {
      std::string l = line.value();
      if (editor->state == command) {
        add_to_history(hist.get(), &hv, l);
        if (l == "q") {
          if (editor->check_quit()) {
            break;
          }

        } else if (l != "" && std::all_of(l.begin(), l.end(), ::isdigit)) {
          uint64_t n = std::stol(l);
          editor->goto_line(n);
        } else if (l == "w") {
          editor->write();
        } else if (l == "p") {
          editor->display_current_line(false);
        } else if (l == "P") {
          editor->display_all_lines(true);
        } else if (l == "n") {
          editor->display_current_line(true);
        } else if (l == "N") {
          editor->display_all_lines(true);
        } else if ((l.front() == '-' || l.front() == '+') &&
                   std::all_of(l.begin() + 1, l.end(), ::isdigit)) {
          int64_t n = std::stol(l);
          editor->rel_move(n);
        } else if (l == "h") {
          editor->display_error_once();
        } else if (l == "H") {
          editor->toggle_verbose();
        } else if (l.front() == '!') {
          l.erase(0, 1);
          run_command(l);
        } else if (l == "a") {
          editor->approach = append;
          editor->state = insert;
        } else if (l == "i") {
          editor->approach = prepend;
          editor->state = insert;
        } else {
          editor->unknown_command();
        }
      } else {
        if (l == ".") {
          editor->state = command;
        } else {
          editor->insert_line(l);
        }
      }
    }
    editor->display_error();
  }
  std::string cmd = "ls -l fien";
  std::string out = get_command_output(cmd).value();
  std::cout << out;
}
