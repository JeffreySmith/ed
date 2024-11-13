#ifndef H_EDITOR
#define H_EDITOR
#include <csignal>
#include <histedit.h>
#include <list>
#include <optional>
#include <string>
enum State {
  insert,
  command,
};
class Editor {
  inline static std::string error_msg = "";
  inline static bool error = false;
  int file_bytes;
  std::string filename;
  State state;
  std::list<std::string> lines;
  bool verbose;
  uint64_t line_num;
  std::list<std::string>::iterator current_address;

  std::optional<std::list<std::string>> load_file(std::string filename);

public:
  explicit Editor(bool);
  Editor(const std::string &, bool);
  static void handle_sigint(int);
  void display_error();
};

std::optional<std::string> get_line(EditLine *);
std::string get_raw_line();
void add_to_history(History *, HistEvent *, std::string &);

#endif
