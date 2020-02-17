#include <stdlib.h>

#include "interface.h"

#ifdef __gnu_linux__

IF * get_interfaces()
{
  return if_nameindex();
}

void free_interfaces(IF * interfaces)
{
  if_freenameindex(interfaces);
}

#else

#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#pragma comment(lib, "IPHLPAPI.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
    
IF* get_interface()
{
    PIP_ADAPTER_INFO adapter_info;
    DWORD            result = 0;
    IF*              interfaces = NULL;
    ULONG            len_interface = sizeof(IP_ADAPTER_INFO);

    adapter_info = (IP_ADAPTER_INFO*)MALLOC(sizeof(IP_ADAPTER_INFO));
    if (adapter_info == NULL) {
        printf("Error allocating memory needed to call GetAdaptersinfo\n");
        return NULL;
    }
    // Make an initial call to GetAdaptersInfo to get
    // the necessary size into the len_interface variable
    if (GetAdaptersInfo(adapter_info, &len_interface) == ERROR_BUFFER_OVERFLOW) {
        FREE(adapter_info);
        adapter_info = (IP_ADAPTER_INFO*)MALLOC(len_interface);
        if (adapter_info == NULL) {
            printf("Error allocating memory needed to call GetAdaptersinfo\n");
            return NULL;
        }
    }

    if ((result = GetAdaptersInfo(adapter_info, &len_interface)) == NO_ERROR) {
        PIP_ADAPTER_INFO adapter = adapter_info;
        unsigned int     i = 0;

        interfaces = (IF*)malloc(sizeof(IF) * ((size_t)len_interface + 1));
        if (interfaces == NULL) return NULL;

        interfaces[len_interface].if_index = 0;
        interfaces[len_interface].if_name = NULL;

        while (adapter) {
            interfaces[i].if_index = adapter->Index;
            interfaces[i].if_name = _strdup(adapter->Description);
            interfaces[i].if_win_name = _strdup(adapter->AdapterName);
            memcpy(interfaces[i].if_addr, adapter->Address, 6);
           
            adapter = adapter->Next;
            ++i;
        }
    }
    else {
        printf("GetAdaptersInfo failed with error: %d\n", result);
        return NULL;
    }
    if (adapter_info) FREE(adapter_info);

    return interfaces;
}

void free_interfaces(IF* interfaces)
{
    if (interfaces) {
        IF* i = interfaces;

        while (i->if_index != 0 && i->if_name != NULL) {
            free(i->if_name);
            free(i->if_win_name);

            i->if_name = NULL;
            i->if_win_name = NULL;

            ++i;
        }

        free(interfaces);
    }
}

#endif

int interface_exists(unsigned int if_index)
{
    IF* interfaces = get_interfaces();
    IF* i = interfaces;
    while (i->if_index != 0 && i->if_name != NULL && i->if_index != if_index) {
        ++i;
    }
    int return_value = (i->if_index != 0 && i->if_name != NULL);
    free_interfaces(interfaces);
    return return_value;
}
