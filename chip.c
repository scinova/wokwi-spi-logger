#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  spi_dev_t spi;
  uint8_t spi_buffer[1];
  pin_t cs_pin;
  pin_t dc_pin;
  uint8_t dc_pin_state;
} chip_state_t;

static void chip_pin_change(void *user_data, pin_t pin, uint32_t value) {
  chip_state_t *chip = (chip_state_t*)user_data;
  chip->dc_pin_state = pin_read(chip->dc_pin);
  if (pin == chip->cs_pin) {
    if (value == LOW)
      spi_start(chip->spi, chip->spi_buffer, sizeof(chip->spi_buffer));
    else
      spi_stop(chip->spi);
  }
}

static void chip_spi_done(void *user_data, uint8_t *buffer, uint32_t count) {
  chip_state_t *chip = (chip_state_t*)user_data;
  printf (chip->dc_pin_state == LOW ? "cmd" : "data");
  printf (" %02X\n", chip->spi_buffer[0]);
  if (pin_read(chip->cs_pin) == LOW)
    spi_start(chip->spi, chip->spi_buffer, sizeof(chip->spi_buffer));
}

void chip_init(void) {
  chip_state_t *chip = malloc(sizeof(chip_state_t));
  chip->cs_pin = pin_init("CS", INPUT_PULLUP);
  chip->dc_pin = pin_init("DC", INPUT_PULLUP);
  const pin_watch_config_t watch_config = {
    .edge = BOTH,
    .pin_change = chip_pin_change,
    .user_data = chip,
  };
  pin_watch(chip->cs_pin, &watch_config);
  pin_watch(chip->dc_pin, &watch_config);
  const spi_config_t spi_config = {
    .sck = pin_init("CLK", INPUT),
    .mosi = pin_init("MOSI", INPUT),
    .miso = NO_PIN,
    .done = chip_spi_done,
    .user_data = chip,
  };
  chip->spi = spi_init(&spi_config);
}
