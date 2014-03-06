// Based off of the pseudo code from http://en.wikipedia.org/wiki/Mersenne_twister
// Additional entropy added by me to ensure better randomness
// TODO: It only generates up to 10 digits.
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <windows.h> // Needed for GetTickCount

int MT[624]; // table to hold random numbers

// credits http://stackoverflow.com/a/275031
// Return clocks per second
__int64 rdtsc()
{
    __int32 lo, hi;
    __asm__ __volatile__ (
      "xorl %%eax, %%eax\n"
      "cpuid\n"
      "rdtsc\n"
      : "=a" (lo), "=d" (hi)
      :
      : "%ebx", "%ecx" );
    return (__int64)hi << 32 | lo;
}

// Return mouse position
__int64 ReturnMousePos()
{
	POINT point;
	ZeroMemory(&point, sizeof(&point));
	
	if (GetCursorPos(&point))
	{
		return point.x ^ point.y;
	}
	
	else
	{
		return 0;
	}
}

// Get number of clock cycles, and xor that with number of milliseconds the system has been up (GetTickCount)
__int64 gen_seed()
{
	__int64 t = rdtsc();
	if (t < 0)
	{
	  t = ~(t) + 1; // We don't want a negative number
	}

	return t ^ GetTickCount() ^ ReturnMousePos();
}

 // Initialize the generator from a seed
int *initialize_generator(int seed, int *ref) 
{
     int i, index = 0;
     MT[0] = seed;
     for (i = 1; i < 624; ++i)
     {
         MT[i] = 0x6c078965 * (0x6c078965 * (MT[i-1] ^ (MT[i-1] >> 30) + i) );
     }
	 
     memcpy(ref, MT, sizeof(MT)); // Copy MT into return buffer, and return the buffer.
     return ref;
 }

// Generate an array of 624 untempered numbers
void generate_numbers() 
{
	int i;
	for (i = 0; i < 624; ++i)
	{
		int y = (MT[i] & 0x80000000) + (MT[(i+1) % 624] & 0x7fffffff);   // First 31 bits of of MT[i]
		MT[i] = MT[(i + 397) % 624] ^ (y >> 1);
		
		if (y % 2 != 0) 
		{ 	 
			MT[i] = MT[i] ^ (0x9908b0df);
		}
	}
 }
 
// Extract a tempered pseudorandom number based on the index-th value,
// calling generate_numbers() every 624 numbers
int extract_number() 
{
	
     int temp[624], index = 0;
     int *gentable = initialize_generator(gen_seed(), temp);
	 
	 // Generate new table
     if (index == 0) 
     {
         generate_numbers();
     }
	
	 
     int y = gentable[index];
     y = y ^ (y >> 11);
     y = y ^ (y << 7 & (0x9d2c5680)); 
     y = y ^ (y << 15 & (0xefc60000));
     y = y ^ (y >> 18);
	 
     index = (index + 1) % 624;
     return y;
}

// This monstrosity is used to calculate the number to modulus the random number by
__int64 calc_power(__int64 input,int power)
{
	__int64 temp;
	if (power % 2 == 0)
	{
		temp = 0;
		
		power = power / 2;
		while (power-- > 0)
		{
			input = input * input;
		}
		return input;
	}
	
	else
	{
		temp = 1;
		while (power--)
		{
			temp *= input;
		}
		return temp;
	}
} 


// Little way of getting the length of an integer
int len(__int64 input)
{
	return log10(floor(input))+1;
}

/*
	Main
	Input: 
		n 	   = number of random numbers to generate
		l = length the random numbers should be
	Output:
		n amount of random numbers of l length
	Description:
		Read in n numbers of l length, and generate numbers based on the inputted criteria
*/
int main()
{
	int i, n, l;
	
	printf("Enter number of numbers to generate: ");
	scanf("%i", &n);
	printf("Enter length of numbers: ");
	scanf("%i", &l);
	
	__int64 mod_len = 9 * calc_power(10, l-1) + calc_power(10, l-1);
	
	for (i = 0; i < n; ++i)
	{
		int num = extract_number() % mod_len;
		if (len(num) < l) // Sometimes the number is less than the length specified, so keep trying until it meets the criteria
		{
			do 
			{
				num = extract_number() % mod_len;
			}
			while (len(num) < l);
		}
		printf("%I64d\n", num);
	}

	return 0;
}
