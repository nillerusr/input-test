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
	char value;
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

int KeyPressTime( int code )
{
	int i;
	struct timeval time;
	gettimeofday( &time, NULL );
	if( buttons[code].value != 1 )
		return -1;
	else
		return time.tv_sec-buttons[code].time;
}

int getBrightness()
{
	int b;
	FILE *f = fopen(BACKLIGHT,"r");
	fscanf(f,"%d",&b);
	fclose(f);
	return b;
}

void setBrightness(int b)
{
	FILE *f = fopen(BACKLIGHT,"w");
	fprintf(f,"%d",b);
	fclose(f);
}

int main()
{
	int timeout_ms = 100;
	char devname[] = "/dev/input/event0";
	struct pollfd device;
	device.fd = open(devname, O_RDONLY|O_NONBLOCK);
	struct input_event ev;
	FILE *f;

	printf("Android( Volume up )\nGentoo( Volume down )\n");
	while(1)
	{
		poll(&device, 1, timeout_ms);
		read(device.fd, &ev, sizeof(ev));
		/*if( buttons[116].value == 2 )
		{
			if( getBrightness() == 0 )
				setBrightness(100);
			else
				setBrightness(0);
			buttons[116].value = 0;
		}*/

		if( buttons[115].value == 2 )
		{
			printf("\nBooting android...\n");
			buttons[115].value = 0;
			f=fopen("/.android","w");
			fclose(f);
			break;
		}

		if( buttons[114].value == 2 )
		{
			printf("\nBooting gentoo...\n");
			buttons[114].value = 0;
			f=fopen("/.gentoo","w");
			fclose(f);
			break;
		}


/*		if( KeyPressTime(116) == 3)
		{
			//system("echo fucking power\n");
			buttons[116].value = 0;
		}*/
		if(ev.type == 1)
		{
//			printf("%d\n",ev.code);
			if( ev.value == 1 )
			{
//				printf("ev.code:  %d\n",ev.code);
				buttons[ev.code].time = ev.time.tv_sec;
			}

			if( buttons[ev.code].value == 1 && ev.value == 0 )
				buttons[ev.code].value = 2;
			else
				buttons[ev.code].value = ev.value;
		}
	}

	return 0;
}
