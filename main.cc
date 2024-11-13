#include "editor.h"
#include "shell.h"
#include <csignal>
#include <err.h>
#include <histedit.h>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <unistd.h>

using namespace std::string_literals;

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
  // History *hist = history_init();
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

  history(hist.get(), &hv, H_SETSIZE, 100);
  history(hist.get(), &hv, H_LAST);
  el_set(el.get(), EL_HIST, history, hist.get());
  el_set(el.get(), EL_PROMPT, set_prompt);
  el_set(el.get(), EL_EDITOR, "emacs");
  el_set(el.get(), EL_SIGNAL, 1);

  while (true) {
    // Run the program here
    std::optional<std::string> line = get_line(el.get());
    if (line.has_value()) {
      std::string l = line.value();
      add_to_history(hist.get(), &hv, l);
      if (l == "q") {
        break;
      }
      if (l.starts_with("!"s)) {
        l.erase(0, 1);
        run_command(l);
      }
    }
    editor->display_error();
  }
  std::string cmd = "ls -l fien";
  std::string out = get_command_output(cmd).value();
}
