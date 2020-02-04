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
  IF * i = interfaces;
  while (i->if_index != 0 && i->if_name != NULL && i->if_index != if_index) {
    ++i;
  }
  int return_value = (i->if_index != 0 && i->if_name != NULL);
  free_interfaces(interfaces);
  return return_value;
}
