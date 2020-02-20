#ifndef COMBO_H
#define COMBO_H

#include <list>
#include <tuple>
#include <functional>

#include <ptr.hpp>

struct CursorInfo;

namespace rscutil {

  class Combo
  {
  public:
    enum class Way { NONE, LEFT, RIGHT };
      
  protected:
    Way                         _way;
    std::function<void(Combo*)> _action;
  
  public:
    using ptr = Ptr<Combo>::ptr;

    static const std::string TYPE;
  
    explicit Combo(Way way = Way::NONE)
      : _way(way),_action(nullptr) {}
    
    virtual ~Combo() = default;

    Way get_way() { return _way; }

    /**
     *\brief Set an action to call when the combo is finished
     *\param c An action to call (a function, a lambda, ...)
     */
    
    template<typename Callable>
    void set_action(Callable&& c) { _action = c; }

    /**
     *\brief Update the combo with a new entry
     *\param code The key code
     *\param value The corresponding value
     *\param x The cursor position on the x-axis
     *\param u The cursor position on the y-axis
     *\return true if the combo is a success. false otherwise.
     */
    
    virtual bool update(int code, int value, int x, int y) = 0;
    virtual const std::string& get_type() const { return TYPE; }
  };

  class ComboShortcut : public Combo, public Ptr<ComboShortcut>
  {
  public:
    using shortcut_t = std::tuple<int,int,int>;

  private:
    std::list<shortcut_t>           _shortcut_list;
    std::list<shortcut_t>::iterator _current;

    std::string _name;
    std::string _description;

    enum TupleID : unsigned { CODE, VALUE, TIME }; // The indice for std::get
  
  public:
    using ComboShortcutList = std::list<ComboShortcut>;
    using Ptr<ComboShortcut>::ptr;

    static constexpr int ANY = -1;                    // Wildcard for any key
    static constexpr int INFINITE = -1;               // No timeout on a key
    static constexpr int DEFAULT_TIMEOUT = INFINITE;

    static const std::string TYPE; 
  
    explicit ComboShortcut(const std::string& name,
			   const std::string& description,
			   Way way = Way::RIGHT)
      : Combo(way),
	_current(_shortcut_list.end()),
	_name(name),
	_description(description) {}

    /**
     *\brief Add a new shortcut to the list.
     *\param code The key code.
     *\param value The value (release, pressed, ...)
     *\param timeout_ms The timeout until the combo is considered a failure (no timeout as default)
     */
    
    void add_shortcut(int code, int value, int timeout_ms = DEFAULT_TIMEOUT);

    void for_each(const std::function<void(shortcut_t&)>& f);

    /**
     *\brief Add a release key for every key in the list at the end. This way, we wait until the user release all the key before changing screen. 
     */
  
    void release_for_all();
  
    ~ComboShortcut() = default;
  
    bool update(int code, int value, int x = 0, int y = 0) override;
    std::string to_string() const;

    /**
     *\brief Get the name of the shortcut
     *\return The name as a std::string
     */
    
    const std::string& get_name() const { return _name; }

    /**
     *\brief Get the description of the shortcut
     *\return The description as a std::string
     */
    
    const std::string& get_description() const { return _description; }

    /**
     *\brief Deserialize a comboshortcut from a file
     *\warning The file must be opened.
     *\param ifs The file stream
     */
    
    void load(std::ifstream& ifs);

    /**
     *\brief Serialize a comboshortcut in a file
     *\warning The file must be opened.
     *\param ofs The file stream
     */
    
    void save(std::ofstream& ofs) const;

    /**
     *\brief Deserialize a list of comboshortcut.
     *\param list The list which will be filled with saved shortcuts
     *\return true if success, false otherwise (if the file does not exist)
     */
    
    static bool load(ComboShortcutList& list);

     /**
     *\brief Serialize a list of comboshortcut.
     *\param list The list which will be saved in a file.
     */
    
    static void save(const ComboShortcutList& list);

    static void make_shortcut(ComboShortcut& combo);

    const std::string& get_type() const override { return TYPE; }

    ComboShortcut(const ComboShortcut& other);
    ComboShortcut(ComboShortcut&& other) noexcept;
    
    ComboShortcut& operator=(const ComboShortcut& other);
    ComboShortcut& operator=(ComboShortcut&& other) noexcept;
  };

#ifndef NO_CURSOR

  class ComboMouse : public Combo, public Ptr<ComboMouse>
  {
    size_t       _width, _height;

  public:
    using Ptr<ComboMouse>::ptr;

    static const std::string TYPE;
  
    explicit ComboMouse(size_t width, size_t height);
    
    ~ComboMouse() = default;
  
    bool update(int code, int value, int x, int y) override;
    const std::string& get_type() const override { return TYPE; }
  };

#endif
}

#endif /* COMBO_H */
