#ifndef COMMAND_H
#define COMMAND_H

#include <memory>
#include <list>
#include <map>
#include <functional>

class RSCCli;

class Command
{
  static constexpr char _NAME[] = "NA";

protected:
  size_t                 _nb_arg, _nb_opt;
  std::list<char>        _opts;
  std::list<std::string> _args;
  
public:
  using Ptr = std::unique_ptr<Command>;

  static const std::string OPT_DELIM;
  
  Command(size_t nb_args = 0, size_t nb_opt = 0)
    : _nb_arg(nb_args), _nb_opt(nb_opt) {}
  Command(const Command &other) = default;
  Command(Command &&other) noexcept = default;

  Command& operator=(const Command &other) = default;
  Command& operator=(Command &&other) = default;

  virtual int execute(RSCCli * cli) = 0;
  virtual void add_opt(const std::string& opt) = 0;
  virtual void add_arg(const std::string& arg) = 0;
  virtual void print_usage() const = 0;
  
  static Command::Ptr get_command(const std::string& name);
  static std::string get_name() { return _NAME; }
  static void print_default_usage();

  virtual ~Command() noexcept = default;
};

// Help ///////////////////////////////////////////////////////////////////////

class HelpCommand : public Command
{
  static constexpr char _NAME[] = "help";
  
public:
  HelpCommand() = default;

  void print_usage() const override;
  void add_arg(const std::string& arg) override;
  void add_opt(const std::string& opt) override;
  int  execute(RSCCli * cli) override;

  static void print_help();
  static std::string get_name() { return _NAME; }
};

// List ///////////////////////////////////////////////////////////////////////

class ListCommand : public Command
{
  static constexpr char _NAME[] = "list";
  static std::map<char, std::function<int(ListCommand*,RSCCli*)>> _on_opt;
  
public:
  ListCommand()
    : Command(0,1) {}
  
  void print_usage() const override;
  void add_arg(const std::string& arg) override;
  void add_opt(const std::string& opt) override;
  int  execute(RSCCli * cli) override;

  int listall(RSCCli*);
  int listrefresh(RSCCli*);
  int listcurrent(RSCCli*);

  static void print_help();
  static std::string get_name() { return _NAME; }
};

// Add ///////////////////////////////////////////////////////////////////////

class AddCommand : public Command
{
  static constexpr char _NAME[] = "add";
  
public:
  AddCommand()
    : Command(2,0) {}

  void print_usage() const override;
  void add_arg(const std::string& arg) override;
  void add_opt(const std::string& opt) override;
  int  execute(RSCCli * cli) override;

  static void print_help();
  static std::string get_name() { return _NAME; }
};

// Version ///////////////////////////////////////////////////////////////////////

class VersionCommand : public Command
{
  static constexpr char _NAME[] = "version";
  
public:
  VersionCommand() = default;

  void print_usage() const override;
  void add_arg(const std::string& arg) override;
  void add_opt(const std::string& opt) override;
  int  execute(RSCCli * cli) override;

  static void print_help();
  static std::string get_name() { return _NAME; }
};

// If ///////////////////////////////////////////////////////////////////////

class IfCommand : public Command
{
  static constexpr char _NAME[] = "if";
  static std::map<char, std::function<int(IfCommand*,RSCCli*)>> _on_opt;

  static constexpr char SET = 's';
  static constexpr char LIST = 'l';
  
public:
  IfCommand()
    : Command(1,1) {}

  void print_usage() const override;
  void add_arg(const std::string& arg) override;
  void add_opt(const std::string& opt) override;
  int  execute(RSCCli * cli) override;

  int set(RSCCli*);
  int list(RSCCli*);

  static void print_help();
  static std::string get_name() { return _NAME; }
};

#endif /* COMMAND_H */
