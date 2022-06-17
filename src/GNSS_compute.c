#include <stdlib.h>
#include <zephyr.h>
#include <math.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>


// Note! Following functions work only if coordinates have two digits before comma.
int32_t GNSS_Compute_lat(float latitude)
{
	char s1[10];
	char s2[7];
	char *ptr;

	sprintf(s1, "%f", latitude);
	// printk("%s", s1);
	memcpy(s2, &s1[3], 6);
	s2[6] = 0;
	// printk("  %s\n", s2);

	return strtol(s2, &ptr, 10);
}

int32_t GNSS_Compute_lon(float longitude)
{
	// Does not tolerate negative values
	char s1[10];
	char s2[7];
	char *ptr;

	sprintf(s1, "%f", longitude);
	// printk("%s", s1);
	if (longitude < 0)
	{
		memcpy(s2, &s1[4], 6);
	} else {
		memcpy(s2, &s1[3], 6);
	}
	s2[6] = 0;
	// printk("  %s\n", s2);

	return strtol(s2, &ptr, 10);
}
