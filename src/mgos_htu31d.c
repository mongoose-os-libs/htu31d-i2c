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

#include "mgos.h"
#include "mgos_htu31d_internal.h"
#include "mgos_i2c.h"

// Private functions follow
static bool mgos_htu31d_cmd(struct mgos_htu31d *sensor, uint8_t cmd) {
  if (!sensor || !sensor->i2c) {
    return false;
  }

  if (!mgos_i2c_write(sensor->i2c, sensor->i2caddr, &cmd, 1, true)) {
    LOG(LL_ERROR,
        ("I2C=0x%02x cmd=%u (0x%02x) write error", sensor->i2caddr, cmd, cmd));
    return false;
  }
  LOG(LL_DEBUG,
      ("I2C=0x%02x cmd=%u (0x%02x) write success", sensor->i2caddr, cmd, cmd));
  return true;
}

static uint8_t crc8(const uint8_t *data, int len) {
  const uint8_t poly = 0x31;
  uint8_t crc = 0x00;

  for (int j = len; j; --j) {
    crc ^= *data++;
    for (int i = 8; i; --i) {
      crc = (crc & 0x80) ? (crc << 1) ^ poly : (crc << 1);
    }
  }

  LOG(LL_DEBUG, ("CRC=0x%02x", crc));

  return crc;
}

// Private functions end

// Public functions follow
struct mgos_htu31d *mgos_htu31d_create(struct mgos_i2c *i2c, uint8_t i2caddr) {
  struct mgos_htu31d *sensor;
  uint32_t version;

  if (!i2c) {
    return NULL;
  }

  sensor = calloc(1, sizeof(struct mgos_htu31d));
  if (!sensor) {
    return NULL;
  }

  memset(sensor, 0, sizeof(struct mgos_htu31d));
  sensor->i2caddr = i2caddr;
  sensor->i2c = i2c;

  mgos_htu31d_cmd(sensor, MGOS_HTU31D_RESET);
  mgos_usleep(15000);

  mgos_htu31d_cmd(sensor, MGOS_HTU31D_READREG);
  if (!mgos_i2c_read(sensor->i2c, sensor->i2caddr, &version, 4, true)) {
    LOG(LL_ERROR, ("Could not read command"));
    free(sensor);
    return NULL;
  }

  if (version != 0) {
    LOG(LL_DEBUG, ("HTU31D serial number %lu created at I2C 0x%02x",
                   (unsigned long) version, sensor->i2caddr));
    return sensor;
  }

  LOG(LL_ERROR, ("Failed to create HTU31D at I2C 0x%02x", i2caddr));
  free(sensor);
  return NULL;
}

void mgos_htu31d_destroy(struct mgos_htu31d **sensor) {
  if (!*sensor) {
    return;
  }

  free(*sensor);
  *sensor = NULL;
  return;
}

bool mgos_htu31d_read(struct mgos_htu31d *sensor) {
  double start = mg_time();

  if (!sensor || !sensor->i2c) {
    return false;
  }

  sensor->stats.read++;

  if (start - sensor->stats.last_read_time < MGOS_HTU31D_READ_DELAY) {
    sensor->stats.read_success_cached++;
    return true;
  }

  // Trigger the conversion
  mgos_htu31d_cmd(sensor, MGOS_HTU31D_CONVERSION);
  mgos_usleep(20000);

  // Read out sensor data here
  uint8_t data[6];
  uint8_t tmp[3];
  uint8_t hum[3];

  mgos_htu31d_cmd(sensor, MGOS_HTU31D_READTEMPHUM);
  mgos_usleep(20000);
  if (!mgos_i2c_read(sensor->i2c, sensor->i2caddr, data, 6, true)) {
    LOG(LL_ERROR, ("Could not read command"));
    return false;
  }

  tmp[0] = data[0];
  tmp[1] = data[1];
  tmp[2] = data[2];
  hum[0] = data[3];
  hum[1] = data[4];
  hum[2] = data[5];

  if (tmp[2] != crc8(data, 2)) {
    LOG(LL_ERROR, ("CRC error on temperature data"));
    return false;
  }

  uint16_t temp = (tmp[0] << 8) + tmp[1];
  float temperature = temp;

  temperature /= 65535.0;
  temperature *= 165;
  temperature -= 40;
  sensor->temperature = temperature;

  if (hum[2] != crc8(hum, 2)) {
    LOG(LL_ERROR, ("CRC error on temperature data"));
    return false;
  }

  uint16_t hmdty = (hum[0] << 8) + hum[1];
  float humidity = hmdty;
  humidity /= 65535.0;
  humidity *= 100;

  sensor->humidity = humidity;

  LOG(LL_DEBUG, ("temperature=%.2fC humidity=%.1f%%", sensor->temperature,
                 sensor->humidity));
  sensor->stats.read_success++;
  sensor->stats.read_success_usecs += 1000000 * (mg_time() - start);
  sensor->stats.last_read_time = start;
  return true;
}

float mgos_htu31d_getTemperature(struct mgos_htu31d *sensor) {
  if (!mgos_htu31d_read(sensor)) {
    return NAN;
  }

  return sensor->temperature;
}

float mgos_htu31d_getHumidity(struct mgos_htu31d *sensor) {
  if (!mgos_htu31d_read(sensor)) {
    return NAN;
  }

  return sensor->humidity;
}

bool mgos_htu31d_setHeater(struct mgos_htu31d *sensor, bool state) {
  uint8_t cmd;
  if (state) {
    cmd = MGOS_HTU31D_HEATERON;
  } else {
    cmd = MGOS_HTU31D_HEATEROFF;
  }

  return mgos_htu31d_cmd(sensor, cmd);
}

bool mgos_htu31d_getStats(struct mgos_htu31d *sensor,
                          struct mgos_htu31d_stats *stats) {
  if (!sensor || !stats) {
    return false;
  }

  memcpy((void *) stats, (const void *) &sensor->stats,
         sizeof(struct mgos_htu31d_stats));
  return true;
}

bool mgos_htu31d_i2c_init(void) {
  return true;
}

// Public functions end
