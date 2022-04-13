#include <hal_init.h>
#include <hal_delay.h>
#include <hal_gpio.h>
#include <hal_usart_async.h>
#include <peripheral_clk_config.h>
#include <hpl_gclk_base.h>
#include <hpl_pm_base.h>

static void _on_usart_rx_cplt_cb(const struct usart_async_descriptor *const iODescriptor);
static void _on_usart_tx_cplt_cb(const struct usart_async_descriptor *const iODescriptor);
static void _on_usart_error_cb(const struct usart_async_descriptor *const iODescriptor);

#define BUTTON            GPIO(GPIO_PORTA, 15)
#define LED_YELLOW        GPIO(GPIO_PORTB, 30)
#define VPCOM_USART_TX    GPIO(GPIO_PORTA, 22)
#define VPCOM_USART_RX    GPIO(GPIO_PORTA, 23)
#define USART_BUFFER_SIZE 32

static uint8_t usartRxByte = 0;
static uint8_t usartRxIndex = 0;
static uint8_t usartRxBuffer[USART_BUFFER_SIZE] = {0};
static struct usart_async_descriptor usartDescriptor = {0};

int main(void)
{
  /* Initialize the hardware abstraction layer */
  init_mcu();
  /* Configure LED GPIO */
  gpio_set_pin_level(LED_YELLOW, false);
  gpio_set_pin_direction(LED_YELLOW, GPIO_DIRECTION_OUT);
  gpio_set_pin_function(LED_YELLOW, GPIO_PIN_FUNCTION_OFF);
  /* Configure USART */
  _pm_enable_bus_clock(PM_BUS_APBC, SERCOM3);
  _gclk_enable_channel(SERCOM3_GCLK_ID_CORE, CONF_GCLK_SERCOM3_CORE_SRC);
  usart_async_init(&usartDescriptor, SERCOM3, &usartRxByte, USART_BUFFER_SIZE, (void *)NULL);
  gpio_set_pin_function(VPCOM_USART_TX, PINMUX_PA22C_SERCOM3_PAD0);
  gpio_set_pin_function(VPCOM_USART_RX, PINMUX_PA23C_SERCOM3_PAD1);
  usart_async_register_callback(&usartDescriptor, USART_ASYNC_TXC_CB, _on_usart_tx_cplt_cb);
  usart_async_register_callback(&usartDescriptor, USART_ASYNC_RXC_CB, _on_usart_rx_cplt_cb);
  usart_async_register_callback(&usartDescriptor, USART_ASYNC_ERROR_CB, _on_usart_error_cb);
  usart_async_enable(&usartDescriptor);

  /* Periodically send "Hello world" string and blink yellow LED */
  while(1)
  {
    io_write(&usartDescriptor.io, (const uint8_t *)"Hello world!\r\n", sizeof("Hello world!\r\n")-1);
    gpio_toggle_pin_level(LED_YELLOW);
    delay_ms(1000);
  }
}

static void _on_usart_rx_cplt_cb(const struct usart_async_descriptor *const iODescriptor)
{
  /* Reception complete */
  if(SERCOM3 == iODescriptor->device.hw)
  {
    /* Circular buffer mechanism */
    if(sizeof(usartRxBuffer) == usartRxIndex)
    {
      usartRxIndex = 0;
    }
    usartRxBuffer[usartRxIndex++] = usartRxByte;
  }
}

static void _on_usart_tx_cplt_cb(const struct usart_async_descriptor *const iODescriptor)
{
  /* Transmission complete */
}

static void _on_usart_error_cb(const struct usart_async_descriptor *const iODescriptor)
{
  /* Error occurred */
}
