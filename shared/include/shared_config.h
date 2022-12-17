#ifndef SHARED_CONFIG_H
#define SHARED_CONFIG_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>

#include "shared_utils.h"

bool config_has_all_properties(t_config* cfg, char** properties);

#endif