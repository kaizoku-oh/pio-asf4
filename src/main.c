#include <hal_init.h>
#include <hal_delay.h>
#include <hal_gpio.h>
#include <hal_ext_irq.h>
#include <hal_usart_async.h>
#include <peripheral_clk_config.h>
#include <hpl_gclk_base.h>
#include <hpl_pm_base.h>

#include <unistd.h>
#include <stdio.h>

static void _on_button_pressed(void);
static void _enable_systick_timer(void);
static void _on_uart_rx_cplt_cb(const struct usart_async_descriptor *const iODescriptor);
static void _on_uart_tx_cplt_cb(const struct usart_async_descriptor *const iODescriptor);
static void _on_uart_error_cb(const struct usart_async_descriptor *const iODescriptor);

#define USART_BUFFER_SIZE 32
#define BUTTON GPIO(GPIO_PORTA, 15)
#define LED_YELLOW GPIO(GPIO_PORTB, 30)
#define VPCOM_UART_TX GPIO(GPIO_PORTA, 22)
#define VPCOM_UART_RX GPIO(GPIO_PORTA, 23)

static volatile uint32_t ticks = 0;
static uint8_t usartRxByte = 0;
static volatile uint8_t usartRxIndex = 0;
static struct usart_async_descriptor usartDescriptor = {0};
static uint8_t usartRxBuffer[USART_BUFFER_SIZE] = {0};

int main(void)
{
  /* Initialize the hardware abstraction layer */
  init_mcu();
  /* Enable the SysTick timer */
  _enable_systick_timer();
  /* Configure LED GPIO */
  gpio_set_pin_level(LED_YELLOW, false);
  gpio_set_pin_direction(LED_YELLOW, GPIO_DIRECTION_OUT);
  gpio_set_pin_function(LED_YELLOW, GPIO_PIN_FUNCTION_OFF);
  /* Configure button interrupt */
  _gclk_enable_channel(EIC_GCLK_ID, CONF_GCLK_EIC_SRC);
  gpio_set_pin_direction(BUTTON, GPIO_DIRECTION_IN);
  gpio_set_pin_pull_mode(BUTTON, GPIO_PULL_UP);
  gpio_set_pin_function(BUTTON, PINMUX_PA15A_EIC_EXTINT15);
  ext_irq_init();
  ext_irq_register(BUTTON, _on_button_pressed);
  /* Configure UART */
  _pm_enable_bus_clock(PM_BUS_APBC, SERCOM3);
  _gclk_enable_channel(SERCOM3_GCLK_ID_CORE, CONF_GCLK_SERCOM3_CORE_SRC);
  usart_async_init(&usartDescriptor, SERCOM3, &usartRxByte, USART_BUFFER_SIZE, (void *)NULL);
  gpio_set_pin_function(VPCOM_UART_TX, PINMUX_PA22C_SERCOM3_PAD0);
  gpio_set_pin_function(VPCOM_UART_RX, PINMUX_PA23C_SERCOM3_PAD1);
  usart_async_register_callback(&usartDescriptor, USART_ASYNC_TXC_CB, _on_uart_tx_cplt_cb);
  usart_async_register_callback(&usartDescriptor, USART_ASYNC_RXC_CB, _on_uart_rx_cplt_cb);
  usart_async_register_callback(&usartDescriptor, USART_ASYNC_ERROR_CB, _on_uart_error_cb);
  usart_async_enable(&usartDescriptor);

  while(1)
  {
    printf("ticks = %lu\r\n", ticks);
    delay_ms(1000);
  }
}

static void _enable_systick_timer(void)
{
  /* 1. Load the SysTick Counter Value */
  SysTick->VAL = 0;
  /* 2. Set the preload register, the value should be your clock frequency in kHz minus 1 */
  SysTick->LOAD = (CONF_CPU_FREQUENCY / 1000) - 1;
  /* 3. Enable SysTick exception, SysTick system timer and use processor clock source */
  SysTick->CTRL = (SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk) | SysTick_CTRL_CLKSOURCE_Msk;
}

static void _on_button_pressed(void)
{
  gpio_toggle_pin_level(LED_YELLOW);
}

static void _on_uart_rx_cplt_cb(const struct usart_async_descriptor *const iODescriptor)
{
  if(SERCOM3 == iODescriptor->device.hw)
  {
    if(sizeof(usartRxBuffer) == usartRxIndex)
    {
      usartRxIndex = 0;
    }
    usartRxBuffer[usartRxIndex++] = usartRxByte;
  }
}

static void _on_uart_tx_cplt_cb(const struct usart_async_descriptor *const iODescriptor)
{
}

static void _on_uart_error_cb(const struct usart_async_descriptor *const iODescriptor)
{
}

void SysTick_Handler(void)
{
  ticks++;
}

void HardFault_Handler(void)
{
}

int _write(int fileDesc, char *string, int length)
{
  int retVal;

  if((STDOUT_FILENO == fileDesc) || (STDERR_FILENO == fileDesc))
  {
    retVal = io_write(&usartDescriptor.io, (uint8_t *)string, length);
  }
  else
  {
    retVal = -1;
  }
  return retVal;
}
