#ifndef MCP3202_HEADER
#define MCP3202_HEADER



#include <stdint.h>


#define SPI_DEVICE "/dev/spidev0.0"
// The voltage connected to Vdd pin
#define VREF_VOLTAGE 3.3  
// 12-bit ADC has 2^12 = 4096 steps (0 to 4095)
#define ADC_RESOLUTION 4095.0 



extern int spi_fd;

void spi_init();

int read_mcp3202_channel(uint8_t channel);


#endif //MCP_3202_HEADER
