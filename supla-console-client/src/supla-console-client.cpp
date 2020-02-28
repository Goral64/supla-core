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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ccronexpr.h"
#include "client_loop.h"
#include "clientcfg.h"
#include "globals.h"
#include "notification_loop.h"
#include "supla-client-lib/log.h"
#include "supla-client-lib/sthread.h"
#include "supla-client-lib/supla-client.h"
#include "supla-client-lib/tools.h"
#include "time.h"

int getch() {
  int r;
  unsigned char c;
  if ((r = read(0, &c, sizeof(c))) < 0) return r;

  return c;
}

int kbhit() {
  struct timeval tv = {0L, 0L};
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(0, &fds);
  return select(1, &fds, NULL, NULL, &tv);
}

void crontest(char *expression) {
  cron_expr expr;
  const char *err = NULL;
  memset(&expr, 0, sizeof(expr));
  cron_parse_expr(expression, &expr, &err);

  if (err) {
    std::cout << "error parsing crontab value " << err << std::endl;
    return;
  }

  time_t next = std::time(NULL);

  std::cout << "next 20 executions: " << std::endl;

  for (int i = 0; i < 20; i++) {
    next = cron_next(&expr, next);
    std::cout << ctime(&next);
  }
}

int main(int argc, char *argv[]) {
  void *client_loop_t = NULL;
  void *notification_loop_t = NULL;

  printf("SUPLA-PUSHOVER v1.1.2\n");

  for (int i = 0; i < argc; i++) {
    if (strcmp("-v", argv[i]) == 0) {
      return 0;
    } else if ((strcmp("-ct", argv[i]) == 0)) {
      crontest(argv[i + 1]);
      return 0;
    }
  }

  if (clientcfg_init(argc, argv) == 0) {
    clientcfg_free();
    return EXIT_FAILURE;
  }

  struct timeval runtime;
  gettimeofday(&runtime, NULL);

#if defined(__DEBUG) && defined(__SSOCKET_WRITE_TO_FILE)
  unlink("ssocket_write.raw");
#endif

  st_mainloop_init();
  st_hook_signals();

  // CLIENT LOOP
  void *sclient = NULL;
  client_loop_t = sthread_simple_run(client_loop, (void *)&sclient, 0);
  notification_loop_t = sthread_simple_run(notification_loop, NULL, 0);

  // MAIN LOOP

  while (st_app_terminate == 0) {
    st_mainloop_wait(5000);
  }

  // RELEASE BLOCK
  sthread_twf(client_loop_t);
  sthread_twf(notification_loop_t);

  delete ntfns;
  delete chnls;

  st_mainloop_free();
  clientcfg_free();

  return EXIT_SUCCESS;
}
