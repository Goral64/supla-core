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

#ifndef HOME_ASSISTANT_CONFIG_H_
#define HOME_ASSISTANT_CONFIG_H_

#include "jsonconfig/json_config.h"

class home_assistant_config : public supla_json_config {
 private:
  static const char root_key[];
  static const char disabled_key[];

 public:
  explicit home_assistant_config(supla_json_config *root);
  home_assistant_config(void);

  // Returns the user's setting if present, otherwise the default value
  // (e.g. the one requested by the device).
  bool is_discovery_disabled(bool default_value);
};

#endif /* HOME_ASSISTANT_CONFIG_H_ */
