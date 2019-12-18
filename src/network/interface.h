#ifndef INTERFACE_H
#define INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <net/if.h>

  /**
   * @brief Structure that contain the name of an interface and its index.
   * Alias for struct if_nameindex in net/if.h
   */

  typedef struct if_nameindex IF;

  /**
   * @brief Returns the name and the index of all interfaces.
   * Alias for if_nameindex() in net/if.h
   * @return Array of name and index of interfaces.
   */

  IF * get_interfaces();

  /**
   * @brief Frees an IF array.
   * Alias for if_freenameindex() in net/if.h
   * @param interfaces IF array.
   */

  void free_interfaces(IF * interfaces);

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_H */
