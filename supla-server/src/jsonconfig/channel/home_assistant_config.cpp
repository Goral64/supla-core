/*
 Copyright (C) AC SOFTWARE SP. Z O.O.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "home_assistant_config.h"

// static
const char home_assistant_config::root_key[] = "homeAssistant";

// static
const char home_assistant_config::disabled_key[] = "homeAssistantDisabled";

home_assistant_config::home_assistant_config(void) : supla_json_config() {}

home_assistant_config::home_assistant_config(supla_json_config *root)
    : supla_json_config(root) {}

bool home_assistant_config::is_discovery_disabled(bool default_value) {
  cJSON *root = get_user_root();
  if (!root) {
    return default_value;
  }

  root = cJSON_GetObjectItem(root, root_key);
  if (root && cJSON_IsObject(root)) {
    cJSON *value = cJSON_GetObjectItem(root, disabled_key);
    if (value && cJSON_IsBool(value)) {
      return cJSON_IsTrue(value);
    }
  }

  return default_value;
}
