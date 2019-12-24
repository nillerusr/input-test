#include <poll.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/signalfd.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BACKLIGHT "/sys/class/backlight/sprd_backlight/brightness"

typedef struct buttons_s
{
	unsigned long time;
} buttons_t;

buttons_t buttons[512];


static void skeleton_daemon()
{
	pid_t pid;
	pid = fork();

	if (pid < 0)
		exit(EXIT_FAILURE);

	if (pid > 0)
		exit(EXIT_SUCCESS);

	if (setsid() < 0)
		exit(EXIT_FAILURE);

	signal(SIGCHLD, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	pid = fork();

	if (pid < 0)
		exit(EXIT_FAILURE);

	if (pid > 0)
		exit(EXIT_SUCCESS);

	umask(0);

	chdir("/");

	int x;
	for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
	{
		close (x);
	}
}


void SetPowerState(const char *state)
{
	FILE *f = fopen("/sys/power/state","w");
	fprintf(f,"%s",state);
	fclose(f);
}

void EnableFramebuffer()
{
	FILE *f = fopen("/sys/class/graphics/fb0/pan","w");
	fprintf(f,"0,854\n");
	fclose(f);
}

int KeyPressTime( int code )
{
	int i;
	struct timeval time;
	gettimeofday( &time, NULL );
	return time.tv_sec-buttons[code].time;
}

int GetBrightness()
{
	int b;
	FILE *f = fopen(BACKLIGHT,"r");
	fscanf(f,"%d",&b);
	fclose(f);
	return b;
}

void SetBrightness(int b)
{
	FILE *f = fopen(BACKLIGHT,"w");
	fprintf(f,"%d",b);
	fclose(f);
}

int main()
{
	char devname[] = "/dev/input/event0";
	struct pollfd device;
	struct input_event ev;
	FILE *f;

	skeleton_daemon();

	device.fd = open(devname, O_RDONLY|O_NONBLOCK);


	while(1)
	{
		read(device.fd, &ev, sizeof(ev));

		if( KeyPressTime(116) == 3)
		{
			system("reboot\n");
			buttons[116].time = 0;
		}

		if(ev.type == 1)
		{
			printf("ev.code: %d, ev.value: %d\n", ev.code, ev.value);
			if( ev.value == 1 )
				buttons[ev.code].time = ev.time.tv_sec;
			else
				buttons[ev.code].time = 0;

			if( ev.code == 116 && ev.value == 0 )
			{
				if( GetBrightness() > 0 )
				{
					SetBrightness(0);
					SetPowerState("standby");
				}
				else
				{
					SetPowerState("on");
					usleep(600000);
					SetBrightness(100);
					EnableFramebuffer();
				}
			}
		}
	}

	return 0;
}
