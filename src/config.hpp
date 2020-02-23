#ifndef CONFIG_H
#define CONFIG_H

// General

#define RSC_NAME "Remote-Shared-Controller"
#define RSC_VERSION "0.1"

#if defined(__gnu_linux__)

#define RSC_BASE_PATH "/var/lib/rsc"
#define RSC_PID_FILE "/var/lib/rsc/pid"
#define RSC_SHORTCUT_SAVE "/var/lib/rsc/shortcut"

#else

#define RSC_BASE_PATH "."
#define RSC_PID_FILE "pid"
#define RSC_SHORTCUT_SAVE "shortcut"

#endif

// Local Communication

#if defined(__gnu_linux__)

#define CURRENT_PC_LIST "/var/lib/rsc/current_pc"
#define ALL_PC_LIST "/var/lib/rsc/all_pc"

#else

#define CURRENT_PC_LIST "current_pc"
#define ALL_PC_LIST "all_pc"

#endif

// rsccli

#define RSCCLI_NAME "rsccli"
#define RSCCLI_VERSION "0.1"

#endif /* CONFIG_H */
