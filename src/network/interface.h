#ifndef INTERFACE_H
#define INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <net/if.h>

  typedef struct if_nameindex IF;

  IF *  get_interfaces();

  void free_interfaces(IF * interfaces);

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_H */
