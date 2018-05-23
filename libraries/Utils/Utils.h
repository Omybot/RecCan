#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

class Utils {
	
	public :

		static unsigned int ReadInt16(byte* tab, int index);
		static unsigned int GetMSB(int value);
		static unsigned int GetLSB(int value);
	
};

#endif