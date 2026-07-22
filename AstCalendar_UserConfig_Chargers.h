/*
 * AstCalendar_UserConfig_Chargers.h
 *
 * Project-specific AstCalendar configuration for charger projects.
 */

#ifndef ASTCALENDAR_USER_CONFIG_CHARGERS_H
#define ASTCALENDAR_USER_CONFIG_CHARGERS_H

#include <stdint.h>

static constexpr uint32_t APP_ASTCALENDAR_NVS_ID[] = {
	CONFIG_ASTCALENDAR_NVS_ID
};

static constexpr uint8_t APP_ASTCALENDAR_NVS_ID_SIZE =
	(uint8_t)(sizeof(APP_ASTCALENDAR_NVS_ID) / sizeof(APP_ASTCALENDAR_NVS_ID[0]));

static constexpr uint32_t APP_ASTCALENDAR_NVS_KEYS[] = {
	CONFIG_ASTCALENDAR_NVS_KEYS
};

#endif
