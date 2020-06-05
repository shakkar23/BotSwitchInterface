#include "commands.hpp"
#include "util.hpp"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
// Result rc;
Mutex actionLock;

// Controller:
bool bControllerIsInitialised = false;
u64 controllerHandle = 0;
HiddbgHdlsDeviceInfo controllerDevice = {0};
HiddbgHdlsState controllerState = {0};

u64 buttonClickSleepTime = 50;

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
	controllerDevice.npadInterfaceType = NpadInterfaceType_Bluetooth;
	// Set the controller colors. The grip colors are for Pro-Controller on [9.0.0+].
	controllerDevice.singleColorBody = RGBA8_MAXALPHA(255, 255, 255);
	controllerDevice.singleColorButtons = RGBA8_MAXALPHA(0, 0, 0);
	controllerDevice.colorLeftGrip = RGBA8_MAXALPHA(230, 255, 0);
	controllerDevice.colorRightGrip = RGBA8_MAXALPHA(0, 40, 20);

	// Setup example controller state.
	controllerState.batteryCharge = 4; // Set battery charge to full.
	controllerState.joysticks[JOYSTICK_LEFT].dx = 0x0;
	controllerState.joysticks[JOYSTICK_LEFT].dy = -0x0;
	controllerState.joysticks[JOYSTICK_RIGHT].dx = 0x0;
	controllerState.joysticks[JOYSTICK_RIGHT].dy = -0x0;
	rc = hiddbgAttachHdlsWorkBuffer();
	if (R_FAILED(rc) && debugResultCodes)
		fatalThrow(rc);
	;
	rc = hiddbgAttachHdlsVirtualDevice(&controllerHandle, &controllerDevice);
	if (R_FAILED(rc) && debugResultCodes)
		fatalThrow(rc);
	bControllerIsInitialised = true;
}

void click(HidControllerKeys btn)
{
	initController();
	press(btn);
	svcSleepThread(buttonClickSleepTime * 1e+6L);
	release(btn);
}
void press(HidControllerKeys btn)
{
	initController();
	controllerState.buttons |= btn;
	hiddbgSetHdlsState(controllerHandle, &controllerState);
}

void release(HidControllerKeys btn)
{
	initController();
	controllerState.buttons &= ~btn;
	hiddbgSetHdlsState(controllerHandle, &controllerState);
}

void setStickState(int side, int dxVal, int dyVal)
{
	initController();
	controllerState.joysticks[side].dx = dxVal;
	controllerState.joysticks[side].dy = dyVal;
	hiddbgSetHdlsState(controllerHandle, &controllerState);
}

u64 getMainNsoBase()
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
	rc = ldrDmntGetProcessModuleInfo(pid, mainpls, 2, &mainPaste);
	if (R_FAILED(rc))
	{
		fatalThrow(0x01200D);
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