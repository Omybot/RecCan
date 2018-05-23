#include <Utils.h>

unsigned int Utils::ReadInt16(byte* tab, int index)
{
	unsigned int val = 0;

	val += tab[index];
	val = val << 8;
	val += tab[index + 1];

	return val;
}

unsigned int Utils::GetMSB(int value)
{
	return value >> 8;
}

unsigned int Utils::GetLSB(int value)
{
	return value & 0xFF;
}