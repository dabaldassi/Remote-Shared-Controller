#ifndef RSCCLI_H
#define RSCCLI_H

#include <iosfwd>
#include <rsclocal_com.hpp>

class PCList;

class RSCCli
{
  rsclocalcom::RSCLocalCom _com;
  
  int _send_cmd(const rsclocalcom::Message& msg);
  void _getlist(PCList& list, const std::string& file_name);
  
public:
  RSCCli() = default;

  int run(int argc, char * argv[]);

  int listall();
  int listcurrent();
  int listrefresh();

  int add(const std::string& id);
  int add(const std::string& id1, const std::string& id2);

  int remove(const std::string& id);

  int version();

  int help();

  int setif(const std::string& id);
  int listif();
};


#endif /* RSCCLI_H */
