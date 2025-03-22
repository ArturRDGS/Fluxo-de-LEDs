#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2818b.pio.h"

// Definição de pinos e quantidade de LEDs
#define LED_COUNT 25
#define LED_PIN 7
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

// Estrutura para representar um pixel RGB
struct pixel_t {
  uint8_t G, R, B;
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t;

// Array para armazenar os LEDs
npLED_t leds[LED_COUNT];
PIO np_pio;
uint sm;

// Função para inicializar os LEDs
void npInit(uint pin) {
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;

  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0) {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true);
  }

  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);


  // Inicializa todos os LEDs com a cor branca (255, 255, 255)
  for (uint i = 0; i < LED_COUNT; ++i) {
    leds[i].R = 255;
    leds[i].G = 255;
    leds[i].B = 255;
  }
}

// Função para definir a cor de um LED específico
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
  leds[index].R = r;
  leds[index].G = g;
  leds[index].B = b;
}

// Função para atualizar os LEDs com as cores armazenadas no array
void npWrite() {
  for (uint i = 0; i < LED_COUNT; ++i) {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100);
}

int main() {
  stdio_init_all();

  npInit(LED_PIN);

  // Configuração dos botões como entrada com pull-up interno
  gpio_init(BUTTON_A_PIN);
  gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_A_PIN);

  gpio_init(BUTTON_B_PIN);
  gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
  gpio_pull_up(BUTTON_B_PIN);


  // Variável para armazenar o estado atual
  int estado = 0;

  while (true) {
    if (!gpio_get(BUTTON_A_PIN)) {
      estado = (estado + 1) % 4;
      sleep_ms(200);
    }

    if (!gpio_get(BUTTON_B_PIN)) {
      estado = (estado + 3) % 4;
      sleep_ms(200);
    }

    // Define a cor dos LEDs de acordo com o estado atual
    if (estado == 0) {
      for (int i = 0; i < LED_COUNT; i++) {
        npSetLED(i, 255, 255, 255);
      }
    } else if (estado == 1) {
      for (int i = 0; i < LED_COUNT; i++) {
        npSetLED(i, 0, 255, 0);
      }
    } else if (estado == 2) {
      for (int i = 0; i < LED_COUNT; i++) {
        npSetLED(i, 255, 0, 0);
      }
    } else if (estado == 3) {
      for (int i = 0; i < LED_COUNT; i++) {
        npSetLED(i, 0, 0, 255);
      }
    }

    // Atualiza os LEDs com as novas cores
    npWrite();
    // Pequeno atraso para evitar piscadas indesejadas
    sleep_ms(100);
  }
}
