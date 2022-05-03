#include <hal_init.h>
#include <hal_delay.h>
#include <hal_gpio.h>

#define LED_YELLOW_PIN GPIO(GPIO_PORTB, 30)

int main(void)
{
  /* Initialize the hardware abstraction layer */
  init_mcu();
  /* Configure LED GPIO */
  gpio_set_pin_level(LED_YELLOW_PIN, false);
  gpio_set_pin_direction(LED_YELLOW_PIN, GPIO_DIRECTION_OUT);
  gpio_set_pin_function(LED_YELLOW_PIN, GPIO_PIN_FUNCTION_OFF);

  /* Periodically blink yellow LED */
  while(1)
  {
    gpio_toggle_pin_level(LED_YELLOW_PIN);
    delay_ms(1000);
  }
}

void HardFault_Handler(void)
{
}
