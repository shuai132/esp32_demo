#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*on_get_ip_t)(const char*);

void wifi_init_sta(on_get_ip_t handle);

#ifdef __cplusplus
}
#endif
