#ifndef CONTROLLER_OP_H
#define CONTROLLER_OP_H

#include <map>

#include <rsclocal_com.hpp>
#include "rscui.hpp"

namespace rscutil {

  class PCList;

}  // rscutil

namespace rscui {

  class ControllerOperation
  {
    static std::map<rsclocalcom::Message::Ack, std::string> _err_msg;

    RscUi * _ui;

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
  
    int _getlist(rscutil::PCList& list, const std::string& file_name);
    
  public:
    ControllerOperation(RscUi * ui)
      : _ui(ui) {}

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

    /**
     *\brief Start the core if it is paused
     *\return 1 if there was an error. 0 otherwise
     *\todo Start a new process if core is not launched ?
     */
  
    int start();

    /**
     *\brief Stop the core.
     *\return 1 if there was an error. 0 otherwise
     */
  
    int stop();

    /**
     *\brief Pause the core if it is running. 
     *\return 1 if there was an error. 0 otherwise
     */
  
    int pause();
  };


}  // rscui

#endif /* CONTROLLER_OP_H */
