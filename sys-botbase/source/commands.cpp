
#include "commands.hpp"
#include "util.hpp"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
// Result rc;

HiddbgHdlsSessionId SessionId = { 0 };
Mutex actionLock;

// Controller:
bool bControllerIsInitialised = false;
HiddbgHdlsHandle controllerHandle = {0};
HiddbgHdlsDeviceInfo controllerDevice = {0};
HiddbgHdlsState controllerState = {0};

u64 buttonClickSleepTime = 50;

void sendUsbResponse(USBResponse x)
{
	usbCommsWrite((void *)&x.size, 4); // send size of response
	if (x.size > 0)
	{
		usbCommsWrite(x.data, x.size); // send actual response
	}
}

void initController()
{
	if (bControllerIsInitialised)
		return;
	// taken from switchexamples github
	Result rc = hiddbgInitialize();
	if (R_FAILED(rc) && debugResultCodes)
		fatalThrow(rc);
	;
	// Set the controller type to Pro-Controller, and set the npadInterfaceType.
	controllerDevice.deviceType = HidDeviceType_FullKey3;
	// Make controller bluetooth
	controllerDevice.npadInterfaceType = HidNpadInterfaceType_Bluetooth;
	// Set the controller colors. The grip colors are for Pro-Controller on [9.0.0+].
	controllerDevice.singleColorBody = RGBA8_MAXALPHA(255, 255, 255);
	controllerDevice.singleColorButtons = RGBA8_MAXALPHA(0, 0, 0);
	controllerDevice.colorLeftGrip = RGBA8_MAXALPHA(230, 255, 0);
	controllerDevice.colorRightGrip = RGBA8_MAXALPHA(0, 40, 20);

	// Setup example controller state.
	// Setup example controller state.
	controllerState.battery_level = 4; // Set battery charge to full.
	controllerState.analog_stick_l = {0x0, 0x0};
	// controllerState.joysticks[JOYSTICK_LEFT].dy = -0x0;
	controllerState.analog_stick_r = {0x0, 0x0};
	//controllerState.joysticks[JOYSTICK_RIGHT].dy = -0x0;
	rc = hiddbgAttachHdlsWorkBuffer(&SessionId);
	if (R_FAILED(rc) && debugResultCodes)
		fatalThrow(rc);
	;
	rc = hiddbgAttachHdlsVirtualDevice(&controllerHandle, &controllerDevice);
	if (R_FAILED(rc) && debugResultCodes)
		fatalThrow(rc);
	bControllerIsInitialised = true;
}

void click(HidNpadButton btn)
{
	initController();
	press(btn);
	svcSleepThread(buttonClickSleepTime * 1e+6L);
	release(btn);
}
void press(HidNpadButton btn)
{
	initController();
	controllerState.buttons |= btn;
	hiddbgSetHdlsState(controllerHandle, &controllerState);
}

void release(HidNpadButton btn)
{
	initController();
	controllerState.buttons &= ~btn;
	hiddbgSetHdlsState(controllerHandle, &controllerState);
}

void setStickState(int side, int dxVal, int dyVal)
{
	initController();
	switch (side)
	{
	case JOYSTICK_MIN:
		controllerState.analog_stick_l = {dxVal, dyVal};
		hiddbgSetHdlsState(controllerHandle, &controllerState);
	case JOYSTICK_MAX:
		controllerState.analog_stick_r = {dxVal, dyVal};
		//controllerState.joysticks[side].dy = dyVal;
		hiddbgSetHdlsState(controllerHandle, &controllerState);
	}
}

u64 getMainNsoBase()
{
	Result rc;

	if (R_FAILED(rc))
		fatalThrow(0x123);
	u64 pid = 0;
	LoaderModuleInfo mainpls[2]; // proc_modules
	s32 mainPaste = 0;			 // numModules

	pmdmntGetApplicationProcessId(&pid);
	rc = ldrDmntGetProcessModuleInfo(pid, mainpls, 2, &mainPaste);
	if (R_FAILED(rc))
	{
		fatalThrow(0x1200D);
	}
	// ldrDmntExit();
	LoaderModuleInfo *mainPaste2 = 0;
	if (mainPaste == 2)
	{
		mainPaste2 = &mainpls[1];
	}
	else
	{
		mainPaste2 = &mainpls[0];
	}
	return mainPaste2->base_address;
}

void pointerMode()
{
	bool loop = true;
	USBResponse x;
	u32 numberOfOffsets;
	u64 main = getMainNsoBase();

	while (loop)
	{
		if (debughandle != 0)
			svcCloseHandle(debughandle);

		eventWait(&vsync_event, UINT64_MAX);
		usbCommsRead((void *)&numberOfOffsets, sizeof(uint32_t)); // read the number of offets we will be sending over to the switch
		if (numberOfOffsets == (UINT32_MAX))
			return; //resetting the sysmod
		else if (numberOfOffsets == 0)
			continue; // there are no offets to look at, trash
		u32 offsets[numberOfOffsets];
		for (u32 i = 0; i < numberOfOffsets; i++)
		{
			usbCommsRead((void *)&offsets[i], sizeof(uint32_t)); // populate pointers maybve
		}

		u64 address = main;
		u64 pid;

		pmdmntGetApplicationProcessId(&pid);
		Result rc;

		rc = svcDebugActiveProcess(&debughandle, pid);
		if (R_FAILED(rc))
			fatalThrow(0xA05);

		rc = svcBreakDebugProcess(debughandle);
		if (R_FAILED(rc))
			fatalThrow(0xA06);

		for (u32 i = 0; i < numberOfOffsets - 1; i++)
		{
			address += offsets[i];
			rc = svcReadDebugProcessMemory((void *)&address, debughandle, address, sizeof(address));

			if (R_FAILED(rc))
			{
				char z[8] = "Invalid";
				x.size = sizeof(z);
				x.data = z;
				sendUsbResponse(x);
				svcCloseHandle(debughandle);
				break;
			}
		}
		if (R_FAILED(rc))
			continue;

		constexpr u64 size = 0x800;
		u8 *data = new u8[size];
		address += offsets[numberOfOffsets - 1];
		for (int i = 0; i < 3; i++)
		{

			rc = svcReadDebugProcessMemory(data, debughandle, address + (size * i), size);
			if (R_FAILED(rc))
			{
				char z[8] = "Invalid";
				x.size = sizeof(z);
				x.data = z;
				sendUsbResponse(x);
				svcCloseHandle(debughandle);
				delete[] data;
				break;
			}
			x.size = size;
			x.data = (void *)data;
			sendUsbResponse(x);
		}
		if (R_FAILED(rc))
			continue;
		delete[] data;

		u64 *addrdata = new u64;
		x.size = sizeof(addrdata);
		(*addrdata) = address;
		x.data = addrdata;
		sendUsbResponse(x);
		delete addrdata;

		rc = svcCloseHandle(debughandle);
		if (R_FAILED(rc))
			fatalThrow(0xA07);
		//eventWait(&vsync_event, UINT64_MAX);
	}
}

void BuildID(u8 joe[0x20])
{
	Result rc;
	u64 pids[300];
	s32 numProc;
	rc = svcGetProcessList(&numProc, pids, 300);
	if (R_FAILED(rc))
		fatalThrow(0x123);
	u64 pid = pids[numProc - 1];
	LoaderModuleInfo mainpls[2]; // proc_modules
	s32 mainPaste = 0;			 // numModules

	pmdmntGetApplicationProcessId(&pid);
	rc = ldrDmntGetProcessModuleInfo(pid, mainpls, 2, &mainPaste);
	if (R_FAILED(rc))
	{
		fatalThrow(0x1200D);
	}
	// ldrDmntExit();
	LoaderModuleInfo *mainPaste2 = 0;
	if (mainPaste == 2)
	{
		mainPaste2 = &mainpls[1];
	}
	else
	{
		mainPaste2 = &mainpls[0];
	}
	for (int i = 0; i < 0x20; i++)
		joe[i] = mainPaste2->build_id[i];
}

bool issame(u8 one[0x20], u8 two[0x20])
{
	for (int i = 0; i < 0x20; i++)
	{
		if (one[i] == two[i])
		{
		}
		else
		{
			return false;
		}
	}
	return true;
}
