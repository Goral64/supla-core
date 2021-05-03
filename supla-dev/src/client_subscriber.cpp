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

#include "client_subscriber.h"

#include <vector>

bool value_exists(jsoncons::json payload, std::string path) {
  try {
    return jsoncons::jsonpointer::contains(payload, path);
  } catch (jsoncons::json_exception& je) {
    return false;
  }
}

bool handle_subscribed_message(client_device_channel* channel,
                               std::string topic, std::string message,
                               _func_channelio_valuechanged  cb,
                               void* user_data) {
  if (message.length() == 0) return false;

  while (!cb) {
    supla_log(LOG_DEBUG, "waiting for registration result...");
    usleep(10);
  }

  supla_log(LOG_DEBUG, "handling message %s", message.c_str());

  char value[SUPLA_CHANNELVALUE_SIZE];

  channel->getValue(value);
  int channelNumber = channel->getNumber();

  jsoncons::json payload;
  std::string template_value = message;

  try {
    payload = jsoncons::json::parse(message);

    TSC_ImpulseCounter_ExtendedValue ic_ev; 
    TSC_SuplaChannelExtendedValue channel_extendedvalue;

    double total = payload["total_m3"].as<double>(); //jsoncons::jsonpointer::get(payload, "total_m3").as<double>();

    _supla_int64_t tt = total * 1000;

    
     char newValue[SUPLA_CHANNELVALUE_SIZE];

     memset(newValue, 0, SUPLA_CHANNELVALUE_SIZE);

     memcpy(newValue, &tt, sizeof(value));

     channel->setValue(newValue);

     if (cb) {
       cb(channelNumber, newValue, user_data);
     }

     return true;
  

  } catch (jsoncons::ser_error& ser) {
     supla_log(LOG_PERROR, "test1");
     supla_log(LOG_PERROR, ser.what());
  } catch (jsoncons::jsonpointer::jsonpointer_error& error) {
     supla_log(LOG_PERROR, error.what());
  }
  return false;
}
