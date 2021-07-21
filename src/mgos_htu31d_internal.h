/*
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <math.h>
#include "mgos.h"
#include "mgos_htu31d.h"
#include "mgos_i2c.h"

#define MGOS_HTU31D_DEFAULT_I2CADDR (0x40)

#define MGOS_HTU31D_READTEMPHUM (0x00)
#define MGOS_HTU31D_CONVERSION (0x40)
#define MGOS_HTU31D_HEATERON (0x04)
#define MGOS_HTU31D_HEATEROFF (0x02)
#define MGOS_HTU31D_READREG (0x0A)
#define MGOS_HTU31D_RESET (0x1E)

#ifdef __cplusplus
extern "C" {
#endif

struct mgos_htu31d {
  struct mgos_i2c *i2c;
  uint8_t i2caddr;
  struct mgos_htu31d_stats stats;

  float humidity, temperature;
};

#ifdef __cplusplus
}
#endif
