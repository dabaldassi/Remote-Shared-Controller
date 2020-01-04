#ifndef RSCCLI_H
#define RSCCLI_H

#include <iosfwd>

class RSCCli
{
  
public:
  RSCCli() = default;

  int run(int argc, char * argv[]);

  int listall();
  int listcurrent();
  int listrefresh();

  int add(const std::string& id);
  int add(const std::string& id1, const std::string& id2);

  int version();

  int help();

  int setif(const std::string& id);
  int listif();
};


#endif /* RSCCLI_H */
