//#define SET_BIT(number, bit, loc) (number) ^= (-(unsigned long)(bit) ^ (number)) & (1UL << (loc))

#include "args.hpp"
#include "commands.hpp"
#include "util.hpp"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <unistd.h>

#define TITLE_ID long 0x0100000000102323

extern "C"
{
	// Sysmodules should not use applet*.
	u32 __nx_applet_type = AppletType_None;

// Adjust size as needed.#
#define INNER_HEAP_SIZE 0x90000
#define HEAP_SIZE 0x000288000
	size_t nx_inner_heap_size = HEAP_SIZE;
	char fake_heap[HEAP_SIZE];

	void __libnx_init_time(void);
	void __libnx_initheap(void);
	void __appInit(void);
	void __appExit(void);
}

Result rc;
u64 mainAddr = {0};
Handle debughandle = {0};
Handle applicationDebug = {0};
u64 applicationProcessId = {0};
u64 pid = {0};
Event vsync_event;
ViDisplay disp;

// we override libnx internals to do a minimal init
void __libnx_initheap(void)
{
	extern char *fake_heap_start;
	extern char *fake_heap_end;

	// setup newlib fake heap
	fake_heap_start = fake_heap;
	fake_heap_end = fake_heap + HEAP_SIZE;
}
void __appInit(void)
{
	Result rc;
	svcSleepThread(10000000000); //10 seconds
	rc = smInitialize();
	if (R_FAILED(rc))
		fatalThrow(0x5122);
	rc = ldrDmntInitialize();
	if (R_FAILED(rc))
		fatalThrow(0x5123);
	else if (hosversionGet() == 0)
	{
		rc = setsysInitialize();
		if (R_FAILED(rc))
			fatalThrow(0x5124);
		else if (R_SUCCEEDED(rc))
		{
			SetSysFirmwareVersion fw;
			rc = setsysGetFirmwareVersion(&fw);
			if (R_FAILED(rc))
				fatalThrow(0x5125);
			else if (R_SUCCEEDED(rc))
				hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
			setsysExit();
		}
	}
	rc = fsInitialize();
	if (R_FAILED(rc))
		fatalThrow(0x5126);
	//rc = fsdevMountSdmc();
	//if (R_FAILED(rc))
	//	fatalThrow(rc);
	rc = timeInitialize();
	if (R_FAILED(rc))
		fatalThrow(0x5127);
	rc = socketInitializeDefault();
	if (R_FAILED(rc))
		fatalThrow(0x5128);
	rc = pmdmntInitialize();
	if (R_FAILED(rc))
		fatalThrow(0x5129);
	rc = usbCommsInitialize();
	if (R_FAILED(rc))
		fatalThrow(0x512a);
	rc = viInitialize(ViServiceType_System);
	if (R_FAILED(rc))
		fatalThrow(0x512b);
}
void __appExit(void)
{
	fsdevUnmountAll();
	fsExit();
	smExit();
	audoutExit();
	timeExit();
	socketExit();
	pmdmntExit();
	ldrDmntExit();
	usbCommsExit();
	timeExit();
	socketExit();
	viExit();
}
u64 pidpls(u64 pid)
{
	if (debughandle != 0)
		svcCloseHandle(debughandle);
	u64 pids[300];
	s32 numProc;
	rc = svcGetProcessList(&numProc, pids, 300);
	if (R_FAILED(0x0BA3A1A3))
	{
	}
	pid = pids[numProc - 1];
	return pid;
}
u64 mainLoopSleepTime = 1;
bool debugResultCodes = false;
bool echoCommands = false;

int argmain(int argc, char **argv)
{
	USBResponse x;
	if (argc == 0)
		return 0;
	// argc is probably the ammount of arguements, aka ammount of data that got sent over

	// peek <address in hex or dec> <amount of bytes in hex or dec>
	else if (!strcmp(argv[0], "peek"))
	{
		if (argc != 3)
		{
			char z[31] = "bruh momentum, peek didnt work";
			x.size = sizeof(z);
			x.data = z;
			sendUsbResponse(x);
		}
		if (debughandle != 0)
			svcCloseHandle(debughandle);
		u64 pids[300];
		s32 numProc;
		rc = svcGetProcessList(&numProc, pids, 300);
		if (R_FAILED(rc))
			fatalThrow(0x69);
		u64 pid = pids[numProc - 1];

		pmdmntGetApplicationProcessId(&pid);
		rc = svcDebugActiveProcess(&debughandle, pid);
		if (R_FAILED(rc))
			fatalThrow(0x1);
		u64 offset = parseStringToInt(argv[1]) + mainAddr;
		u64 size = parseStringToInt(argv[2]);
		u8 *data = new u8[size];
		rc = svcBreakDebugProcess(debughandle);
		if (R_FAILED(rc))
			fatalThrow(0x2);
		rc = svcReadDebugProcessMemory(data, debughandle, offset, size);
		if (R_FAILED(rc))
		{
			char z[26] = "Invalid offsetted address";
			x.size = sizeof(z);
			x.data = z;
			sendUsbResponse(x);
			return 0;
		}
		x.size = size;
		x.data = (void *)data;
		sendUsbResponse(x);
		delete[] data;
		rc = svcCloseHandle(debughandle);
		if (R_FAILED(rc))
			fatalThrow(0x798);
	}

	// else if (!strcmp(argv[0], "Configurable_Command"))
	// {
	// 	//just copy paste this to make another value looker atter command
	// 	if (debughandle != 0)
	// 		svcCloseHandle(debughandle);
	// 	u64 pids[300];
	// 	s32 numProc;
	// 	rc = svcGetProcessList(&numProc, pids, 300);
	// 	if (R_FAILED(rc))
	// 		fatalThrow(0x69);
	// 	u64 pid = pids[numProc - 1];
	// 	rc = svcDebugActiveProcess(&debughandle, pid);
	// 	if (R_FAILED(rc))
	// 		fatalThrow(0x1);
	// 	u64 offset1 = parseStringToInt(argv[1]);
	// 	u64 size = parseStringToInt(argv[2]);
	// 	//probably make variable here
	// 	rc = svcBreakDebugProcess(debughandle);
	// 	if (R_FAILED(rc))
	// 		fatalThrow(0x2);
	// 	//svcReadDebugProcessMemory(&data, debughandle, offset1, size);
	// 	//data is a variable you have to make to store whatever you are looking at
	// 	//debughandle is something that you dont change
	// 	//offset is the address you want to look at
	// 	//size is the size of the data you want to look at
	// 	//x.size = size;
	// 	//x.data = (void *)data;
	// 	//sendUsbResponse(x);
	// 	//rc = svcCloseHandle(debughandle);
	// 	//uncomment this code here^^
	// 	//pointers are easy to look at, just use this
	// 	//u64 offset2 = your second offset
	// 	//u64 offset3 = your third offset
	// 	//svcReadDebugProcessMemory(&offset2, debughandle, mainAddr + offset, size);
	// 	//svcReadDebugProcessMemory(&offset3, debughandle, offset2 + offset2, size);
	// 	// keep doing this and it should just work, add more u64's of offsets and set them to whatever your offsets are
	// }

	else if (!strcmp(argv[0], "PeekAbsolute"))
	{
		if (argc != 3)
		{ // if you get 3 as a response, either it worked and you got the value 3, or this if statement was triggered
			//or this if statement is telling you that it doesnt work
			char z[18] = "not enough args";
			x.size = sizeof(z);
			x.data = z;
			sendUsbResponse(x);
		}
		if (debughandle != 0)
			svcCloseHandle(debughandle);
		u64 pids[300];
		s32 numProc;
		rc = svcGetProcessList(&numProc, pids, 300);
		if (R_FAILED(rc))
			fatalThrow(0x69);
		u64 pid = pids[numProc - 1];

		pmdmntGetApplicationProcessId(&pid);
		rc = svcDebugActiveProcess(&debughandle, pid);
		if (R_FAILED(rc))
			fatalThrow(0x1);
		u64 offset = parseStringToInt(argv[1]);
		u64 size = parseStringToInt(argv[2]);
		u8 *data = new u8[size];
		rc = svcBreakDebugProcess(debughandle);
		if (R_FAILED(rc))
			fatalThrow(0x2);
		rc = svcReadDebugProcessMemory(data, debughandle, offset, size);
		if (R_FAILED(rc))
		{
			char z[25] = "Invalid Absolute address";
			x.size = sizeof(z);
			x.data = z;
			sendUsbResponse(x);
			rc = svcCloseHandle(debughandle);
			if (R_FAILED(rc))
				fatalThrow(0x795);
			return 0;
		}
		x.size = size;
		x.data = (void *)data;
		sendUsbResponse(x);
		delete[] data;
		rc = svcCloseHandle(debughandle);
		if (R_FAILED(rc))
			fatalThrow(0x798);
	}

	// click <buttontype>
	else if (!strcmp(argv[0], "click"))
	{
		if (argc != 2)
		{
			x.size = sizeof(int);
			x.data = (void *)4;
			sendUsbResponse(x);
			return 0;
		}
		HidNpadButton key = parseStringToButton(argv[1]);
		click(key);
	}

	// hold <buttontype>
	else if (!strcmp(argv[0], "press"))
	{
		if (argc != 2)
		{
			x.size = sizeof(int);
			x.data = (void *)5;
			sendUsbResponse(x);
			return 0;
		}
		HidNpadButton key = parseStringToButton(argv[1]);
		press(key);
	}

	else if (!strcmp(argv[0], "poke"))
	{
		if (argc != 4)
		{ // if you get 3 as a response, either it worked and you got the value 3, or this if statement was triggered
			//or this if statement is telling you that it doesnt work
			char z[18] = "not enough args";
			x.size = sizeof(z);
			x.data = z;
			sendUsbResponse(x);
		}
		if (debughandle != 0)
			svcCloseHandle(debughandle);
		u64 pids[300];
		s32 numProc;
		rc = svcGetProcessList(&numProc, pids, 300);
		if (R_FAILED(rc))
			fatalThrow(0x69);
		u64 pid = pids[numProc - 1];

		pmdmntGetApplicationProcessId(&pid);
		rc = svcDebugActiveProcess(&debughandle, pid);
		if (R_FAILED(rc))
			fatalThrow(0x67818);
		u64 offset = parseStringToInt(argv[1]);
		u64 size = parseStringToInt(argv[2]);
		u64 data = parseStringToInt(argv[3]);

		rc = svcBreakDebugProcess(debughandle);
		if (R_FAILED(rc))
			fatalThrow(0x2182);
		rc = svcWriteDebugProcessMemory(debughandle, &data, offset, size);

		rc = svcCloseHandle(debughandle);
		if (R_FAILED(rc))
			fatalThrow(0x794);
	}

	// release <buttontype>
	else if (!strcmp(argv[0], "release"))
	{
		if (argc != 2)
			x.size = sizeof(int);
		x.data = (void *)6;
		sendUsbResponse(x);
		return 0;
		HidNpadButton key = parseStringToButton(argv[1]);
		release(key);
	}

	// setStick <left or right stick> <x value> <y value>
	else if (!strcmp(argv[0], "setStick"))
	{
		if (argc != 4)
		{
			x.size = sizeof(int);
			x.data = (void *)7;
			sendUsbResponse(x);
			return 0;
		}

		int side = 0;
		if (!strcmp(argv[1], "LEFT"))
		{
			side = JOYSTICK_MIN;
		}
		else if (!strcmp(argv[1], "RIGHT"))
		{
			side = JOYSTICK_MAX;
		}
		else
		{
			return 0;
		}

		int dxVal = strtol(argv[2], NULL, 0);
		if (dxVal > JOYSTICK_MAX)
			dxVal = JOYSTICK_MAX; // 0x7FFF
		if (dxVal < JOYSTICK_MIN)
			dxVal = JOYSTICK_MIN; //-0x8000

		int dyVal = strtol(argv[3], NULL, 0);
		if (dxVal > JOYSTICK_MAX)
			dxVal = JOYSTICK_MAX;
		if (dxVal < JOYSTICK_MIN)
			dxVal = JOYSTICK_MIN;

		setStickState(side, dxVal, dyVal);
	}

	// detachController
	else if (!strcmp(argv[0], "detachController"))
	{
		Result rc = hiddbgDetachHdlsVirtualDevice(controllerHandle);
		if (R_FAILED(rc) && debugResultCodes)
			fatalThrow(rc);
		rc = hiddbgReleaseHdlsWorkBuffer();
		if (R_FAILED(rc) && debugResultCodes)
			fatalThrow(rc);
		hiddbgExit();
		bControllerIsInitialised = false;
	}

	// configure <mainLoopSleepTime or buttonClickSleepTime> <time in ms>
	else if (!strcmp(argv[0], "configure"))
	{
		if (argc != 3)
			return 0;

		else if (!strcmp(argv[1], "mainLoopSleepTime"))
		{
			u64 time = parseStringToInt(argv[2]);
			mainLoopSleepTime = time;
		}

		else if (!strcmp(argv[1], "buttonClickSleepTime"))
		{
			u64 time = parseStringToInt(argv[2]);
			buttonClickSleepTime = time;
		}

		else if (!strcmp(argv[1], "echoCommands"))
		{
			u64 shouldActivate = parseStringToInt(argv[2]);
			echoCommands = shouldActivate != 0;
		}

		else if (!strcmp(argv[1], "printDebugResultCodes"))
		{
			u64 shouldActivate = parseStringToInt(argv[2]);
			debugResultCodes = shouldActivate != 0;
		}
	}

	//never used this, i can read what language im using lol
	else if (!strcmp(argv[0], "getSystemLanguage"))
	{
		// thanks zaksa
		setInitialize();
		u64 languageCode = 0;
		SetLanguage language = SetLanguage_ENUS;
		setGetSystemLanguage(&languageCode);
		setMakeLanguage(languageCode, &language);
		x.size = sizeof(language);
		x.data = &language;
		sendUsbResponse(x);
	}

	//syntax for this command is "getMainNsoBase" lol
	else if (!strcmp(argv[0], "getMainNsoBase"))
	{
		// u64 mainAddr = getMainNsoBase();
		x.size = sizeof(mainAddr);
		x.data = (void *)&mainAddr;
		sendUsbResponse(x);
		//havent tested this but im pretty sure it works
	}

	else if (!strcmp(argv[0], "TitleID"))
	{
		u8 joe[0x20] = {0};
		BuildID(joe);
		//u8 mama[0x20] = {0x9D, 0x42, 0x5D, 0xBF, 0x1D, 0x9C, 0x54, 0x88, 0x53, 0xAE, 0xCB, 0xB1, 0xD4, 0xF2, 0x6C, 0xAB, 0x51, 0x35, 0x5C, 0xCC};

		x.size = sizeof(joe);
		x.data = joe;
		sendUsbResponse(x);
	}

	else if (!strcmp(argv[0], "enablePointerMode"))
	{
		pointerMode();
	}

	return 0;
}

int main()
{
	
	// Result rc;
	USBResponse x;
	mainAddr = getMainNsoBase();
	viOpenDefaultDisplay(&disp);
	viGetDisplayVsyncEvent(&disp, &vsync_event);

	while (appletMainLoop()) // it loops in here forever until you turn off the sysmod, when that happens it aborts on the instruction its on without calling destructors
	{
		int len;
		usbCommsRead(&len, sizeof(len));
		if (len == UINT32_MAX)
			continue;
		char linebuf[len + 1];

		for (int i = 0; i < len + 1; i++)
		{
			linebuf[i] = 0;
		}

		usbCommsRead(&linebuf, len);

		// Adds necessary escape characters for pasrser
		linebuf[len - 1] = '\n';
		linebuf[len - 2] = '\r';

		fflush(stdout);

		parseArgs(linebuf, &argmain);

		if (echoCommands)
		{
			x.size = sizeof(linebuf);
			x.data = &linebuf;
			sendUsbResponse(x);
		}
		// if you are getting preformance issues lower this, although for my purposes i dont need to
		svcSleepThread(mainLoopSleepTime * 100000000);
	}
	return 0;
}
