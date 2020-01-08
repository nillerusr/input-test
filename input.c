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
#include <sys/time.h>
#include <string.h>

const char *getBootMode( void )
{
	FILE *f = fopen("/proc/cmdline","r");
	const char *token = "androidboot.mode=";
	int c, i, k;
	static char buf[32];
	k = i = 0;
	c = fgetc_unlocked(f);

	while( c != EOF )
	{
		if( c == token[i] )
		{
			if( c == '=' )
				k = 1;
			i++;
		}
		else
			i=0;

		c = fgetc_unlocked(f);
		if( k && c )
		{
			if( c == '\n' || c == ' ' )
				break;
			buf[k-1] = c;
			k++;
		}
	}

	buf[k] = '\0';
	return buf;
}

void bootsystem(const char *system)
{
	FILE *f;
	char filename[64];
	sprintf(filename,"/.%s",system);
	printf("\nBooting %s...\n", system);
	f=fopen(filename,"w");
	fclose(f);

}

int main()
{
	int timeout = 5;
	int savedtime;
	char devname[] = "/dev/input/event0";
	struct pollfd device;
	device.fd = open(devname, O_RDONLY|O_NONBLOCK);
	struct input_event ev;
	struct timeval time;

	FILE *f;

	if( strcmp(getBootMode(), "charger") == 0 )
	{
		bootsystem("android");
		return 0;
	}

	gettimeofday(&time, 0);
	savedtime = time.tv_sec;
	printf("Android( Volume up )\nGentoo( Volume down )\n");

	while(1)
	{
		read(device.fd, &ev, sizeof(ev));
		gettimeofday(&time, 0);

		if(ev.type == 1)
		{
			if( ev.code == 115 && ev.value == 0 )
			{
				bootsystem("android");
				break;
			}
			else if( ev.code == 114 && ev.value == 0 )
			{
				bootsystem("gentoo");
				break;
			}
		}

		if( time.tv_sec - savedtime >= timeout )
		{
			bootsystem("android");
			break;
		}
	}

	return 0;
}
