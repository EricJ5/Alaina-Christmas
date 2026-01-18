#include "mcp3202.h"

#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>


int spi_fd;

void spi_init() {
    uint8_t mode = 0; // SPI Mode 0 or 3
    uint8_t bits = 8;
    uint32_t speed = 1000000; // 1 MHz clock speed

    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("Error opening SPI device");
        exit(1);
    }

     // Configure SPI
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_RD_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
}

int read_mcp3202_channel(uint8_t channel) {
    uint8_t tx[] = {0x01, 0xA0, 0x00}; // Start bit, config byte (CH0 single-ended), dummy byte
    if (channel == 1) {
        tx[1] = 0xE0; // Config byte for CH1 single-ended
    }
    
    uint8_t rx[sizeof(tx)] = {0};
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = sizeof(tx),
        .speed_hz = 1000000,
        .delay_usecs = 0,
        .bits_per_word = 8,
    };

    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);

    // The result is 12 bits, spanning the second and third bytes
    // Mask the relevant bits: first 4 bits from rx[1] and all 8 bits from rx[2]
    int adc_value = ((rx[1] & 0x0F) << 8) | rx[2];
    
    return adc_value;
}
