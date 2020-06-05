#pragma once
#ifndef COMMANDS_HPP
#define COMMANDS_HPP
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
// Result rc;
extern Handle debughandle;
extern u64 buttonClickSleepTime;
extern bool bControllerIsInitialised;
extern HiddbgHdlsDeviceInfo controllerDevice;
extern HiddbgHdlsState controllerState;
extern Handle applicationDebug;
extern u64 applicationProcessId;
extern u64 controllerHandle;
extern u64 pid;

// void poke(u64 offset, u64 size, u8* val);
// u8* peek(u64 offset, u64 size);

void click(HidControllerKeys btn);
void press(HidControllerKeys btn);
void release(HidControllerKeys btn);
void setStickState(int side, int dxVal, int dyVal);

u64 getMainNsoBase();
#endif