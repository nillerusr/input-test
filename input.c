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

int main()
{
	int timeout_ms = 100;
	char devname[] = "/dev/input/event0";
	struct pollfd device;
	device.fd = open(devname, O_RDONLY|O_NONBLOCK);
	struct input_event ev;
	FILE *f;

	if( strcmp(getBootMode(), "charger") == 0 )
	{
		f=fopen("/.android","w");
		fclose(f);
		return 0;
	}

	printf("Android( Volume up )\nGentoo( Volume down )\n");
	while(1)
	{
		read(device.fd, &ev, sizeof(ev));

		if(ev.type == 1)
		{
			if( ev.code == 115 && ev.value == 0 )
			{
				printf("\nBooting android...\n");
				f=fopen("/.android","w");
				fclose(f);
				break;
			}
			else if( ev.code == 114 && ev.value == 0 )
			{
				printf("\nBooting gentoo...\n");
				f=fopen("/.gentoo","w");
				fclose(f);
				break;
			}
		}
	}

	return 0;
}
