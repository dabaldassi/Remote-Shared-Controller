#include <Windows.h>

#include "fifo.hpp"

Fifo::Fifo(Contact c)
{
	if (c == Contact::CORE) {
		// CreateNamedPipe();
	}
}

int Fifo::open()
{

}

int Fifo::send_to(Contact c, const std::string& msg)
{

}

int Fifo::read_from(Contact c, std::string& answer)
{

}

int Fifo::send(const std::string& msg)
{

}

int Fifo::read(std::string& answer)
{

}

void Fifo::close()
{

}

Fifo::~Fifo()
{

}