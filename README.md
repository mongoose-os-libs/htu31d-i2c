# HTU31D I2C Driver

A Mongoose library for Measurement Specialities HTU31D(F) integrated circuit.
Adaptation of the [HTU21D library](https://github.com/mongoose-os-libs/htu21df-i2c)
with timing parameters pulled from the 
[Adafruit HTU31D library](https://github.com/adafruit/Adafruit_HTU31D)


## Example application

An example program using a timer to read data from the sensor every 5 seconds:

```
#include "mgos.h"
#include "mgos_i2c.h"
#include "mgos_htu31d.h"

static struct mgos_htu31d *s_htu31d;

static void timer_cb(void *user_data) {
  float temperature, humidity;

  temperature=mgos_htu31d_getTemperature(s_htu31d);
  humidity=mgos_htu31d_getHumidity(s_htu31d);

  LOG(LL_INFO, ("htu31d temperature=%.2f humidity=%.2f", temperature, humidity));

  (void) user_data;
}

enum mgos_app_init_result mgos_app_init(void) {
  struct mgos_i2c *i2c;

  i2c=mgos_i2c_get_global();
  if (!i2c) {
    LOG(LL_ERROR, ("I2C bus missing, set i2c.enable=true in mos.yml"));
  } else {
    s_htu31d=mgos_htu31d_create(i2c, 0x40); // Default I2C address
    if (s_htu31d) {
      mgos_set_timer(5000, true, timer_cb, NULL);
    } else {
      LOG(LL_ERROR, ("Could not initialize sensor"));
    }
  }
  return MGOS_APP_INIT_SUCCESS;
}
```
