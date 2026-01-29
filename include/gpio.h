#ifndef GPIO_HEADER
#define GPIO_HEADER

#include <wiringPi.h>

#define VOLUME_UP 3
#define VOLUME_DOWN 2
#define SKIP 0
#define PREV 8
#define MODE 9
#define RESET 7

void gpio_init();

#endif //GPIO_HEADER
