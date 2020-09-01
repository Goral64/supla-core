/*
 * channel.h
 *
 *  Created on: 18 lut 2020
 *      Author: beku
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_

#include <chrono>
#include <cstring>
#include <iomanip>
#include <string>
#include <vector>

#include "notification_loop.h"
#include "supla-client-lib/proto.h"
#include "supla-client-lib/safearray.h"

class channel {
 private:
  int channel_id;
  int channel_function;
  bool online;
  unsigned _supla_int_t flags;
  TDSC_ChannelState* state;

  std::string caption;
  char value[SUPLA_CHANNELVALUE_SIZE];
  char sub_value[SUPLA_CHANNELVALUE_SIZE];

  std::vector<void*> notification_list;
  std::vector<void*> connection_change_list;
  bool value_changed(char first[SUPLA_CHANNELVALUE_SIZE],
                     char second[SUPLA_CHANNELVALUE_SIZE]);

 public:
  channel(int channel_id, int channel_function, std::string caption,
          char value[SUPLA_CHANNELVALUE_SIZE],
          char sub_value[SUPLA_CHANNELVALUE_SIZE], bool online,
          unsigned _supla_int_t flags);
  virtual ~channel();

  void setValue(char value[SUPLA_CHANNELVALUE_SIZE]);
  void setSubValue(char value[SUPLA_CHANNELVALUE_SIZE]);
  void setCaption(std::string value);
  void setFunction(int value);
  void notify(void);
  void setState(TDSC_ChannelState* state);
  TDSC_ChannelState* getState(void);

  void getValue(char value[SUPLA_CHANNELVALUE_SIZE]);
  void getSubValue(char value[SUPLA_CHANNELVALUE_SIZE]);
  std::string getCaption(void);
  int getFunction(void);
  int getChannelId(void);
  std::string getStringValue(int index);
  unsigned _supla_int_t getFlags(void);

  void add_notification_on_change(void* value);
  void add_notification_on_connection(void* value);

  bool getOnline(void);
  void setOnline(bool value);
};

class channels {
 private:
  void* arr;

 public:
  channels();
  virtual ~channels();

  channel* add_channel(int channel_id, int channel_function,
                       std::string caption, char value[SUPLA_CHANNELVALUE_SIZE],
                       char sub_value[SUPLA_CHANNELVALUE_SIZE], bool online,
                       unsigned _supla_int_t flags);
  channel* find_channel(int channel_id);
  void* get_channels(void);
};

extern channels* chnls;

#endif /* CHANNEL_H_ */
