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

#include "HomeAssistantConfigTest.h"

#include "TestHelper.h"
#include "jsonconfig/channel/home_assistant_config.h"
#include "proto.h"

namespace testing {

HomeAssistantConfigTest::HomeAssistantConfigTest(void) {}

HomeAssistantConfigTest::~HomeAssistantConfigTest(void) {}

TEST_F(HomeAssistantConfigTest, defaultValueWhenNotSet) {
  home_assistant_config config;

  EXPECT_FALSE(config.is_discovery_disabled(false));
  EXPECT_TRUE(config.is_discovery_disabled(true));

  config.set_user_config("{\"alexa\":{\"alexaDisabled\":true}}");

  EXPECT_FALSE(config.is_discovery_disabled(false));
  EXPECT_TRUE(config.is_discovery_disabled(true));
}

TEST_F(HomeAssistantConfigTest, userSettingOverridesDefault) {
  home_assistant_config config;

  config.set_user_config(
      "{\"homeAssistant\":{\"homeAssistantDisabled\":true}}");
  EXPECT_TRUE(config.is_discovery_disabled(false));
  EXPECT_TRUE(config.is_discovery_disabled(true));

  config.set_user_config(
      "{\"homeAssistant\":{\"homeAssistantDisabled\":false}}");
  EXPECT_FALSE(config.is_discovery_disabled(false));
  EXPECT_FALSE(config.is_discovery_disabled(true));
}

TEST_F(HomeAssistantConfigTest, invalidValueIsIgnored) {
  home_assistant_config config;

  config.set_user_config("{\"homeAssistant\":{\"homeAssistantDisabled\":1}}");
  EXPECT_FALSE(config.is_discovery_disabled(false));
  EXPECT_TRUE(config.is_discovery_disabled(true));

  config.set_user_config("{\"homeAssistant\":true}");
  EXPECT_TRUE(config.is_discovery_disabled(true));
}

} /* namespace testing */
