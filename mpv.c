#define _POSIX_C_SOURCE 200809L
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
#include <spawn.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <wiringPi.h>
#include <signal.h>


atomic_flag downloading = ATOMIC_FLAG_INIT;
volatile bool updated = false;
char *playlist = "./Songs/playlist.m3u";
int mode = 1;
extern char **environ;
#define VOLUME_UP 3
#define VOLUME_DOWN 2
#define SKIP 0
#define PREV 8
#define MODE 9
#define RESET 7
static volatile sig_atomic_t running = 1;
volatile bool connected = true;
const char *playlistcommand[] ={"loadfile", "./Songs/playlist.m3u", "replace", NULL};
static void handle_sigterm(int sig)
{
    (void)sig;
    running = 0;
}
void gpioinit() {
	wiringPiSetup();
	
	pinMode(VOLUME_UP, INPUT);
	pinMode(VOLUME_DOWN, INPUT);
	pinMode(SKIP, INPUT);
	pinMode(PREV, INPUT);
	pinMode(MODE, INPUT);
	pinMode(RESET, INPUT);

    	pullUpDnControl(VOLUME_UP, PUD_UP);
	pullUpDnControl(VOLUME_DOWN, PUD_UP);
	pullUpDnControl(SKIP, PUD_UP);
	pullUpDnControl(PREV, PUD_UP);
	pullUpDnControl(MODE, PUD_UP);
	pullUpDnControl(RESET, PUD_UP);
}
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



void *UpdatePlaylist(void *arg) {
	while (atomic_flag_test_and_set(&downloading)) {
	};
	pid_t pid1;
	pid_t pid2;
	char *Songs = "Songs";
	char *archive = "./Songs/archive.txt";
   	char *argv2[] = {
        	"yt-dlp",
        	"--flat-playlist",
        	"--print", "%(title)s.mp3",
        	(char *)yturl,
        	NULL
    	};
	char *argv1[] = {
        	"yt-dlp",
		"-o",
		"Songs/%(title)s.%(ext)s",
        	"-x",
        	"--audio-format", "mp3",
        	"--download-archive", (char *)archive,
        	(char *)yturl,
       	 	NULL
   	};
	posix_spawnp(&pid1, "yt-dlp", NULL, NULL, argv1, environ);
	int download = waitpid(pid1, NULL, 0);
	if (download != 0) {
		printf("check internet connection");		
		return NULL;
	}
	unlink(playlist);
        posix_spawn_file_actions_t fa;
        posix_spawn_file_actions_init(&fa);
        int fd = open(playlist,
                  O_CREAT | O_WRONLY | O_TRUNC, 0644);
        posix_spawn_file_actions_adddup2(&fa, fd, STDOUT_FILENO);
        posix_spawn_file_actions_addclose(&fa, fd);
	posix_spawnp(&pid2, "yt-dlp", &fa, NULL, argv2, environ);
	posix_spawn_file_actions_destroy(&fa);
	waitpid(pid2, NULL, 0);
    	atomic_flag_clear(&downloading);
	updated = true;
	return NULL;	
}

void *ButtonLoop(void *handle) {
	mpv_handle *mpv = (mpv_handle*)handle;
	const char *nextcommand[] ={"playlist-next", NULL};
	const char *prevcommand[] ={"playlist-prev", NULL};
	const char *volupcommand[] ={"add", "volume", "5", NULL};
	const char *voldowncommand[] ={"add", "volume", "-5", NULL};
	pthread_t pid;
	gpioinit();
	while (running) {
		if (!updated) {
			if (digitalRead(VOLUME_UP) ==  LOW) {
				printf("volume up\n");
				mpv_command(mpv, volupcommand);
				delay(200);
			}
                        else if (digitalRead(VOLUME_DOWN) ==  LOW) {
				printf("volume down\n");
				mpv_command(mpv, voldowncommand);
				delay(200);
                        }
                        else if (digitalRead(SKIP) ==  LOW) {
				printf("skip\n");
				mpv_command(mpv, nextcommand);
				delay(200);
                        }
                        else if (digitalRead(PREV) ==  LOW) {
				printf("prev\n");
				mpv_command(mpv, prevcommand);
				delay(200);
                        }
                        else if (digitalRead(MODE) ==  LOW) {
				printf("mode switch \n");
				if (mode == 1) {mode=2;}
				else if (mode == 2) {mode=3;}
				else if (mode == 3) {mode=1;}
				delay(200);
                        }
                        else if (digitalRead(RESET) ==  LOW) {
				printf("reset\n");
				pthread_create(&pid, NULL, UpdatePlaylist, NULL);
				delay(200);
                        }
		}
		else {
			printf("hey from update");
			mpv_command(mpv, playlistcommand);
			updated = false;
		}
	}
	return NULL;
}


void *MPVEventLoop(void *handle) {
    	mpv_handle *mpv = (mpv_handle*)handle;
    	while (running) {
       		mpv_event *ev = mpv_wait_event(mpv, 0.1);
        	if (ev->event_id == MPV_EVENT_SHUTDOWN)
            	break;
    	}
	return NULL;
}


int main(int argc,char *argv[]) {

    	struct sigaction sa = {0};
    	sa.sa_handler = handle_sigterm;
    	sigemptyset(&sa.sa_mask);
    	sa.sa_flags = 0;
	printf("hello");
    	sigaction(SIGTERM, &sa, NULL);
    	sigaction(SIGINT,  &sa, NULL);
	yturl = argv[1];
	static double speedarray[4095];

	double increment = (double) 3/4095;

	double count = .35;
	for (int i = 0;i < 3095; i++) {
		speedarray[i] = count;
		count += increment;
	}
	printf("%s", argv[1]);
	mpv_handle *mpv = mpv_create();
	
	mpv_set_option_string(mpv, "softvol", "yes");
	mpv_set_option_string(mpv, "ao", "alsa");
	mpv_set_option_string(mpv, "loop-playlist", "inf");
	mpv_set_option_string(mpv, "vid", "no");
	mpv_initialize(mpv);
	mode_t perms = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
	mkdir("Songs", perms);
	pthread_t mpv_thread;
	pthread_create(&mpv_thread, NULL, MPVEventLoop, mpv);


	pthread_t button_thread;
	pthread_create(&button_thread, NULL, ButtonLoop, mpv);


	

	mpv_command(mpv, playlistcommand);

	pthread_t update_thread;
	pthread_create(&update_thread, NULL, UpdatePlaylist, NULL);

	struct timespec polltime;
	struct timespec stoptime;
	polltime.tv_sec = 0;
	polltime.tv_nsec = 1000000L;
	stoptime.tv_sec = 0;
	stoptime.tv_nsec = 500000000L;
	char speed[15];
	spi_init();
	double normalspeed = 1.00f;
	while (running) {
		int adc_value = read_mcp3202_channel(0);
		if (adc_value > 30) {
			unstop(mpv);
			if (mode == 1) {
				set_speed(mpv, speedarray[adc_value-30], speed);
			} else {
				set_speed(mpv, normalspeed, speed);
			}
		} else  {
			if (mode==3) {
				unstop(mpv);
				set_speed(mpv, normalspeed, speed);
			}
			nanosleep(&stoptime, NULL);
			if (adc_value < 30) {
				stop(mpv);
			}
		}
		nanosleep(&polltime, NULL);


	}	
	return 0;
}	

