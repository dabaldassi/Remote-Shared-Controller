#include <stdlib.h>

#include "interface.h"

IF * get_interfaces()
{
  return if_nameindex();
}

void free_interfaces(IF * interfaces)
{
  if_freenameindex(interfaces);
}

int interface_exists(unsigned int if_index)
{
  IF * interfaces = get_interfaces();
  while (interfaces != NULL && interfaces->if_index != if_index) {
    ++interfaces;
  }
  int return_value = interfaces != NULL;
  free_interfaces(interfaces);
  return return_value;
}
