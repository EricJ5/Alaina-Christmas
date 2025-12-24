#include <stdatomic.h>
#include <syscall.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <mpv/client.h>
#include <sys/types.h>


atomic_flag downloading = ATOMIC_FLAG_INIT;
volatile bool updated = false;
char *playlist = "./Songs/playlist.m3u";



void set_speed(mpv_handle *handle, double speedfloat, char *speed) {
        float speedf = speedfloat;
        if (speedf < 0) {
        speedf  = speedfloat*-1;
        }
        sprintf(speed, "%f", speedf);
        mpv_set_option_string(handle, "speed", speed);
}


double get_playback_left(mpv_handle *handle) {
        double time_remaining;
        mpv_get_property(handle, "time-remaining", MPV_FORMAT_DOUBLE, &time_remaining);
        // printf("%f\n", time_remaining);
        return time_remaining;
}


void seek_to_time(mpv_handle *handle, double seektime) {
        char time[20];
        sprintf(time, "%f", seektime);
        mpv_set_property_string(handle, "time-pos", time);
}


void stop(mpv_handle *handle) {
        mpv_set_property_string(handle, "pause", "yes");
}


void unstop(mpv_handle *handle) {
        mpv_set_property_string(handle, "pause", "no");
}


#define SPI_DEVICE "/dev/spidev0.0"
#define VREF_VOLTAGE 3.3  // The voltage connected to Vdd pin
#define ADC_RESOLUTION 4095.0 // 12-bit ADC has 2^12 = 4096 steps (0 to 4095)
char *yturl;
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



void *UpdatePlaylist(void *handle) {
	while (atomic_flag_test_and_set(&downloading)) {
	}
	mpv_handle *mpv = (mpv_handle *)handle;
	char *Songs = "./Songs";
	char *archive = "./Songs/archive.txt";
	char ytdlpcommand[242];
	printf("%s \n", yturl);
	snprintf(ytdlpcommand, 242, "yt-dlp -o \"./Songs/%%(title)s.mp3\" -x --print-to-file \"%%(title)s.mp3\" %s --audio-format mp3 --download-archive %s %s", playlist, archive, yturl);
	system(ytdlpcommand);
    	atomic_flag_clear(&downloading);
	updated = true;
	return NULL;
}

void *ButtonLoop(void *handle) {
	mpv_handle *mpv = (mpv_handle*)handle;
	const char *nextcommand[] ={"playlist-next", NULL};
	const char *prevcommand[] ={"playlist-prev", NULL};
	const char *playlistcommand[] ={"loadfile", playlist, "replace", NULL};
	pthread_t pid;
	for (;;) {
		if (!updated) {
			int key = getchar();
			switch (key) {
				case 's':
					mpv_command(mpv, nextcommand);
      					break;
				case 'p':
					mpv_command(mpv, prevcommand);
      					break;
				case 'r':
					pthread_create(&pid, NULL, UpdatePlaylist, (void *)mpv);
					break;
				
			}
		}
		else {
			printf("hey from update");
			mpv_command(mpv, playlistcommand);
			updated = false;
		}
	}
}



int main(int argc,char *argv[]) {

	yturl = argv[1];
	static double speedarray[3995];

	double increment = (double) 3/3996;

	printf("%f\n", increment);
	double count = .35;
	for (int i = 0;i < 3096; i++) {
		speedarray[i] = count;
		count += increment;
	}
	printf("%s", argv[1]);
	printf("hey-1\n");
	mpv_handle *mpv = mpv_create();
	
	mpv_set_option_string(mpv, "vid", "no");
    	mpv_set_option_string(mpv, "volume", "100");
	mpv_set_option_string(mpv, "ao", "alsa");
	mpv_set_option_string(mpv, "loop-playlist", "inf");
	mpv_initialize(mpv);
   	printf("hey \n", NULL);
	pthread_t id;	
	pthread_create(&id, NULL, ButtonLoop, (void*)mpv);
	printf("hey before initial playlist \n");
	UpdatePlaylist((void *)mpv);
	const char *playlistcommand[] ={"loadfile", playlist, "replace", NULL};
	mpv_command(mpv, playlistcommand);
	printf("hey after initial playlist \n");
		
	
	printf("hey2\n");
	char speed[15];
	spi_init();

	for (;;) {
		int adc_value = read_mcp3202_channel(0);
		if (adc_value > 30) {
			unstop(mpv);
			printf("%d\n", adc_value);
			printf("%f\n", speedarray[adc_value-100]);
			set_speed(mpv, speedarray[adc_value-100], speed);
		} else  {
			usleep(.5 * 1000000);
			if (adc_value < 30) {
				stop(mpv);
			}
		}
		usleep(100*0000);


	}	
	return 0;
}	

