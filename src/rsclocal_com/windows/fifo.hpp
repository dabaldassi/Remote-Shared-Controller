#ifndef WIN_FIFO_H
#define WIN_FIFO_H

#include <string>

typedef void* HANDLE;
typedef int BOOL;

namespace rsclocalcom {

    class Fifo
    {
    public:
        enum class Contact { CORE, CLIENT }; // Possible contact

    private:
        static constexpr char PIPE_NAME[] = "\\\\.\\pipe\\fifo_win";
        static constexpr size_t BUFSIZE = 1024;

        HANDLE  _pipe;
        Contact _contact;
        BOOL    _connected;

    public:

        explicit Fifo(Contact c);

        /**
         *\brief Open the right file for the communication
         *\return 1 if there was an error. 0 otherwise
         */

        int  open();

        /**
         *\brief Send a message to a contact.
         *\param c The contact
         *\param msg The message as std::string
         *\return Negative value if error. The number of bytes written otherwise.
         */

        int  send_to(Contact c, const std::string& msg);

        /**
         *\brief Read a message from a contact
         *\param c The contact
         *\param msg The buffer in which will be stored the message
         *\return Negative value if error. The number of bytes read otherwise.
         */

        int  read_from(Contact c, std::string& answer);

        /**
         *\brief Send a message to the other side of the fifo.
         *\param msg The message as std::string
         *\return Negative value if error. The number of bytes written otherwise.
         */

        int send(const std::string& msg);

        /**
         *\brief Read a message from the other side of the fifo
         *\param msg The buffer in which will be stored the message
         *\return Negative value if error. The number of bytes read otherwise.
         */

        int read(std::string& answer);

        /**
         *\brief Close the connection
         */

        void close();

        ~Fifo();
    };
}

#endif // !WIN_FIFO_H
