#include "TimeServer.h"
#include "lib_debug/Debug.h"

#include <camkes.h>

static const if_OS_Timer_t timer =
    IF_OS_TIMER_ASSIGN(timeServer_rpc, timeServer_notify);

void util_sleep(uint32_t us) {
  OS_Error_t err;

  if ((err = TimeServer_sleep(&timer, TimeServer_PRECISION_USEC, us)) !=
      OS_SUCCESS) {
    Debug_LOG_ERROR("TimeServer_sleep() failed with %d", err);
  }
}