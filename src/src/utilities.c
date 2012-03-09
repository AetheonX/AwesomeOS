/*
 *@author: Valeriy Chibisov
 */

#ifdef DEBUG_0
#include <stdio.h>
#endif // DEBUG_0

int strcmp(char *c1, char *c2) {
	if (c1 == 0 || c2 == 0)
		return 0;

	while (*c1 != 0 && *c2 != 0) { // loop through each char in the string
		if (*c1 != *c2)
			return 0;

		c1++;
		c2++;
	}

	return 1;
}