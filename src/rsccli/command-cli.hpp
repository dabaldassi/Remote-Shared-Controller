#ifndef COMMAND_H
#define COMMAND_H

#include <memory>
#include <list>
#include <map>
#include <functional>

namespace rscui {

  class ControllerOperation;

  /**
   *\class Command
   *\brief Build and execute an rsccli command.
   */

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

    /**
     *\brief Execute the command
     *\param ops The command line interface object to call the corresponding command
     *\return The value returned by the command
     *\todo Check for every subclasses if the ops object implement the method 
     *      with SFINAE
     */
  
    virtual int execute(ControllerOperation & ops) = 0;

    /**
     *\brief Add an option to the command.
     * An option starts with the delimiter '-'
     *\param opt The option to add
     */
  
    virtual void add_opt(const std::string& opt) = 0;

    /**
     *\brief Add an argument to the command
     * Argument do not have delimiter
     *\param arg The argument to add. 
     */
  
    virtual void add_arg(const std::string& arg) = 0;

    /**
     *\brief Print the syntax on how to use the command
     */
  
    virtual void print_usage() const = 0;

    /**
     *\brief Return a new instance of a command corresponding to its name.
     *\param name The name of the command
     *\return A pointer on the command
     */
  
    static Command::Ptr get_command(const std::string& name);

    /**
     *\brief Get the name of the command
     *\return The name of the command
     */
  
    static std::string get_name() { return _NAME; }

    /**
     *\brief Print the default usage for the command
     */
  
    static void print_default_usage();

    virtual ~Command() noexcept = default;
  };

  // Help ///////////////////////////////////////////////////////////////////////

  /**
   *\class HelpCommand
   *\brief Display help of rsccli
   */

  class HelpCommand : public Command
  {
    static constexpr char _NAME[] = "help";
  
  public:
    HelpCommand() = default;

    void print_usage() const override;
    void add_arg(const std::string& arg) override;
    void add_opt(const std::string& opt) override;
    int  execute(ControllerOperation & ops) override;

    static void print_help();
    static std::string get_name() { return _NAME; }
  };

  // List ///////////////////////////////////////////////////////////////////////

  /**
   *\class ListCommand
   *\brief List the different PC according to the option
   */

  class ListCommand : public Command
  {
    static constexpr char _NAME[] = "list";
    static std::map<char, std::function<int(ListCommand*,ControllerOperation&)>> _on_opt;

    static constexpr char ALL = 'a';
    static constexpr char REFRESH = 'r';
    static constexpr char CURRENT = 'c';
  
  public:
    ListCommand()
      : Command(0,1) {}
  
    void print_usage() const override;
    void add_arg(const std::string& arg) override;
    void add_opt(const std::string& opt) override;
    int  execute(ControllerOperation & ops) override;

    int listall(ControllerOperation&);
    int listrefresh(ControllerOperation&);
    int listcurrent(ControllerOperation&);

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
    int  execute(ControllerOperation & ops) override;

    static void print_help();
    static std::string get_name() { return _NAME; }
  };

  // Remove ///////////////////////////////////////////////////////////////////////

  class RemoveCommand : public Command
  {
    static constexpr char _NAME[] = "remove";
  
  public:
    RemoveCommand()
      : Command(1,0) {}

    void print_usage() const override;
    void add_arg(const std::string& arg) override;
    void add_opt(const std::string& opt) override;
    int  execute(ControllerOperation & ops) override;

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
    int  execute(ControllerOperation & ops) override;

    static void print_help();
    static std::string get_name() { return _NAME; }
  };

  // If ///////////////////////////////////////////////////////////////////////

  class IfCommand : public Command
  {
    static constexpr char _NAME[] = "if";
    static std::map<char,
		    std::function<int(IfCommand*,ControllerOperation&)>> _on_opt;

    static constexpr char SET = 's';
    static constexpr char LIST = 'l';
  
  public:
    IfCommand()
      : Command(1,1) {}

    void print_usage() const override;
    void add_arg(const std::string& arg) override;
    void add_opt(const std::string& opt) override;
    int  execute(ControllerOperation & ops) override;

    int set(ControllerOperation&);
    int list(ControllerOperation&);

    static void print_help();
    static std::string get_name() { return _NAME; }
  };


  // Start ///////////////////////////////////////////////////////////////////////

  class StartCommand : public Command
  {
    static constexpr char _NAME[] = "start";
  
  public:
    StartCommand() = default;

    void print_usage() const override;
    void add_arg(const std::string& arg) override;
    void add_opt(const std::string& opt) override;
    int  execute(ControllerOperation & ops) override;

    static void print_help();
    static std::string get_name() { return _NAME; }
  };

  // Stop ///////////////////////////////////////////////////////////////////////

  class StopCommand : public Command
  {
    static constexpr char _NAME[] = "stop";
  
  public:
    StopCommand() = default;

    void print_usage() const override;
    void add_arg(const std::string& arg) override;
    void add_opt(const std::string& opt) override;
    int  execute(ControllerOperation & ops) override;

    static void print_help();
    static std::string get_name() { return _NAME; }
  };

  // Pause ///////////////////////////////////////////////////////////////////////

  class PauseCommand : public Command
  {
    static constexpr char _NAME[] = "pause";
  
  public:
    PauseCommand() = default;

    void print_usage() const override;
    void add_arg(const std::string& arg) override;
    void add_opt(const std::string& opt) override;
    int  execute(ControllerOperation & ops) override;

    static void print_help();
    static std::string get_name() { return _NAME; }
  };

}  // rscui


#endif /* COMMAND_H */
