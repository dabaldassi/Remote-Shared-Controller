#ifndef RSCCLI_H
#define RSCCLI_H

#include <iosfwd>
#include <map>
#include <functional>

#include <rsclocal_com.hpp>

class PCList;

class RSCCli
{
  static std::map<rsclocalcom::Message::Ack, std::function<void(void)>> _err_msg;
  
  rsclocalcom::RSCLocalCom _com;

  /**
   *\brief Send a message to the core and wait for the ack
   *\param msg The message to send
   *\return The result of the ack
   */
  
  int _send_cmd(const rsclocalcom::Message& msg);

  /**
   *\brief Load a list from a file name
   *\param list A reference to the list to build.
   *\param The file name where the list has been serialized
   */
  
  int _getlist(PCList& list, const std::string& file_name);
  
public:
  RSCCli() = default;

  /**
   *\brief Run the application
   *\param argc The number of arguments
   *\param argv The arguments
   */
  
  int run(int argc, char * argv[]);

  /**
   *\brief List all the charateristics of the current list of PC
   *\return 1 if there was an error. 0 otherwise
   */
  
  int listall();
  
  /**
   *\brief List all the current list of PC
   *\return 1 if there was an error. 0 otherwise
   */
  
  int listcurrent();
  
  /**
   *\brief List all the pc available on the network
   *\return 1 if there was an error. 0 otherwise
   */
  
  int listrefresh();

  /**
   *\brief Add a pc to the current list by its id
   *\param id The ID of the PC
   *\return 1 if there was an error. 0 otherwise
   */
  
  int add(const std::string& id);

  /**
   *\brief Add a pc to the current list by its id and before another id
   *\param id The ID of the PC to add
   *\param id2 The next PC id in the list
   *\return 1 if there was an error. 0 otherwise
   */  
  
  int add(const std::string& id1, const std::string& id2);

  /**
   *\brief Remove a pc from the current list by its id
   *\param id The ID of the PC to remove
   *\return 1 if there was an error. 0 otherwise
   */
  
  int remove(const std::string& id);
  
  /**
   *\brief Show the current version of rsccli
   *\return 1 if there was an error. 0 otherwise
   */
  
  int version();

  /**
   *\brief Show the help of rsccli
   *\return 1 if there was an error. 0 otherwise
   */
  
  int help();
  
  /**
   *\brief Set the current network interface by its id
   *\param id The id of the network interface as an std::string
   *\return 1 if there was an error. 0 otherwise
   */

  int setif(const std::string& id);

  /**
   *\brief List all the network interface of the PC
   *\return 1 if there was an error. 0 otherwise
   */
  
  int listif();
};


#endif /* RSCCLI_H */
