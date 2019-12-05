#include "interface.h"

IF * get_interfaces()
{
  return if_nameindex();
}

void free_interfaces(IF * interfaces)
{
  if_freenameindex(interfaces);
}
