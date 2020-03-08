#ifndef COMMAND_H
#define COMMAND_H

#include <memory>
#include <list>
#include <map>
#include <functional>

namespace rscui {

  class ControllerOperation;
  using ctrl_op_t = ControllerOperation;
  
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
  
    virtual int execute(ctrl_op_t & ops) = 0;

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
    int  execute(ctrl_op_t & ops) override;

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
    static std::map<char, std::function<int(ListCommand*,ctrl_op_t&)>> _on_opt;

    static constexpr char ALL = 'a';
    static constexpr char REFRESH = 'r';
    static constexpr char CURRENT = 'c';

    bool _all;
  
  public:
    ListCommand()
      : Command(0,2), _all{false} {}
  
    void print_usage() const override;
    void add_arg(const std::string& arg) override;
    void add_opt(const std::string& opt) override;
    int  execute(ctrl_op_t & ops) override;

    int listcurrent(ctrl_op_t& ops);
    int listrefresh(ctrl_op_t& ops);

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
    int  execute(ctrl_op_t & ops) override;

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
    int  execute(ctrl_op_t & ops) override;

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
    int  execute(ctrl_op_t & ops) override;

    static void print_help();
    static std::string get_name() { return _NAME; }
  };

  // If ///////////////////////////////////////////////////////////////////////

  class IfCommand : public Command
  {
    static constexpr char _NAME[] = "if";
    static std::map<char,
		    std::function<int(IfCommand*,ctrl_op_t&)>> _on_opt;

    static constexpr char SET = 's';
    static constexpr char LIST = 'l';
    static constexpr char GET = 'g';
  
  public:
    IfCommand()
      : Command(1,1) {}

    void print_usage() const override;
    void add_arg(const std::string& arg) override;
    void add_opt(const std::string& opt) override;
    int  execute(ctrl_op_t & ops) override;

    int set(ctrl_op_t&);
    
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
    int  execute(ctrl_op_t & ops) override;

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
    int  execute(ctrl_op_t & ops) override;

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
    int  execute(ctrl_op_t & ops) override;

    static void print_help();
    static std::string get_name() { return _NAME; }
  };

  // ShortcutCommand //////////////////////////////////////////////////////////

  class ShortcutCommand : public Command
  {
    static constexpr char _NAME[] = "shortcut";

    static constexpr char SET = 's';
    static constexpr char LIST = 'l';
    static constexpr char RESET = 'r';

    static std::map<char, std::function<int(ShortcutCommand*,ctrl_op_t&)>> _on_opt;
    
  public:
    ShortcutCommand()
      : Command(1,1) {}

    void print_usage() const override;
    void add_arg(const std::string& arg) override;
    void add_opt(const std::string& opt) override;
    int  execute(ctrl_op_t & ops) override;

    int list(ctrl_op_t & ops);
    int set(ctrl_op_t & ops);
    
    static void print_help();
    static std::string get_name() { return _NAME; }
  };

  // OptionCommand ////////////////////////////////////////////////////////////

  class OptionCommand : public Command
  {
    static constexpr char _NAME[] = "set";

    static constexpr char ENABLED[] = "enabled";
    static constexpr char DISABLED[] = "disabled";

    static std::map<std::string, unsigned> _option_id;

    unsigned _option_to_set;
    bool     _state_to_set;
    
  public:
    OptionCommand()
      : Command(2,0) {}

    void print_usage() const override;
    void add_arg(const std::string& arg) override;
    void add_opt(const std::string& opt) override;
    int  execute(ctrl_op_t & ops) override;
    
    static void print_help();
    static std::string get_name() { return _NAME; }
  };

  // SwapCommand //////////////////////////////////////////////////////////////
  
  class SwapCommand : public Command
  {
    static constexpr char _NAME[] = "swap";
    
  public:
    SwapCommand()
      : Command(2,0) {}

    void print_usage() const override;
    void add_arg(const std::string& arg) override;
    void add_opt(const std::string& opt) override;
    int  execute(ctrl_op_t & ops) override;
    
    static void print_help();
    static std::string get_name() { return _NAME; }
  };

  // KeyCommand ///////////////////////////////////////////////////////////////

  class KeyCommand : public Command
  {
    static constexpr char _NAME[] = "key";
    
  public:
    KeyCommand()
      : Command(1,0) {}

    void print_usage() const override;
    void add_arg(const std::string& arg) override;
    void add_opt(const std::string& opt) override;
    int  execute(ctrl_op_t & ops) override;
    
    static void print_help();
    static std::string get_name() { return _NAME; }
  };
  
}  // rscui


#endif /* COMMAND_H */
