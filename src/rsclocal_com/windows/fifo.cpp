#include <iostream>
#include <Windows.h>

#include "fifo.hpp"

using rsclocalcom::Fifo;

Fifo::Fifo(Contact c) : _pipe(NULL), _contact(c), _connected(FALSE)
{
	
}

int Fifo::open()
{
    if (_contact == Contact::CORE) {
        _pipe = CreateNamedPipe(
            PIPE_NAME,             // pipe name 
            PIPE_ACCESS_DUPLEX |      // read/write access 
            WRITE_OWNER | WRITE_DAC,
            PIPE_TYPE_MESSAGE |       // message type pipe 
            PIPE_READMODE_MESSAGE |   // message-read mode 
            PIPE_WAIT,                // blocking mode 
            PIPE_UNLIMITED_INSTANCES, // max. instances  
            BUFSIZE,                  // output buffer size 
            BUFSIZE,                  // input buffer size 
            0,                        // client time-out 
            NULL);                    // default security
    }
    else {
        BOOL quit = FALSE;
        do {
            _pipe = CreateFile(
                PIPE_NAME,   // pipe name 
                GENERIC_READ |  // read and write access 
                GENERIC_WRITE,
                0,              // no sharing 
                NULL,           // default security attributes
                OPEN_EXISTING,  // opens existing pipe 
                0,              // default attributes 
                NULL);          // no template file

            if (GetLastError() == ERROR_PIPE_BUSY) {
                // All pipe instances are busy, so wait for 20 seconds. 
                if (!WaitNamedPipe(PIPE_NAME, 20000))
                {
                    std::cerr << "Could not open pipe: 20 second wait timed out.";
                    return 1;
                }
            }
            else {
                quit = TRUE;
            }
        } while (!quit);

        if (_pipe != INVALID_HANDLE_VALUE) {
            DWORD dwMode = PIPE_READMODE_MESSAGE;
            BOOL success = SetNamedPipeHandleState(
                _pipe,    // pipe handle 
                &dwMode,  // new pipe mode 
                NULL,     // don't set maximum bytes 
                NULL);    // don't set maximum time 

            if (!success) {
                std::cerr << "SetNamedPipeHandleState failed. GLE=" << GetLastError() << std::endl;
                return 1;
            }
        }
    }

    if (_pipe == INVALID_HANDLE_VALUE) {
        std::cerr << "Could not open pipe : " << GetLastError() << std::endl;
        return 1;
    }

    return 0;
}

int Fifo::send_to(Contact, const std::string& msg)
{
    return send(msg);
}

int Fifo::read_from(Contact, std::string& answer)
{
    return read(answer);
}

int Fifo::send(const std::string& msg)
{
    DWORD bytes_written;
    BOOL success = WriteFile(
        _pipe,        // handle to pipe 
        msg.c_str(),     // buffer to write from 
        (DWORD)msg.size(), // number of bytes to write 
        &bytes_written,   // number of bytes written 
        NULL);        // not overlapped I/O 

    if (!success) {
        std::cerr << "InstanceThread WriteFile failed, GLE=" << GetLastError() << std::endl;
        _connected = FALSE;
        return -1;
    }

    return bytes_written;
}

int Fifo::read(std::string& answer)
{
    char buf[BUFSIZE];
    DWORD bytes_read;

    if (_contact == Contact::CORE && !_connected) {
        do {
            _connected = ConnectNamedPipe(_pipe, NULL);
            if (GetLastError() == ERROR_PIPE_CONNECTED) _connected = TRUE;
        } while (!_connected);
    }

    BOOL success = ReadFile(_pipe, buf, BUFSIZE, &bytes_read, NULL);
    if (!success) {
        _connected = FALSE;
        
        if (_contact == Contact::CORE) {
            FlushFileBuffers(_pipe);
            DisconnectNamedPipe(_pipe);
        }

        return -1;
    }

    buf[min(BUFSIZE - 1, bytes_read)] = 0;

    answer = buf;
    return bytes_read;
}

void Fifo::close()
{
    CloseHandle(_pipe);
    _pipe = NULL;
}

Fifo::~Fifo()
{

}