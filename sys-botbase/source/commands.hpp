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
extern HiddbgHdlsHandle controllerHandle;
extern u64 pid;
extern u64 mainAddr;
extern Event vsync_event;
extern HiddbgHdlsSessionId SessionId;
typedef struct
{
    u64 size;
    void *data;
} USBResponse;

void sendUsbResponse(USBResponse x);

// void poke(u64 offset, u64 size, u8* val);
// u8* peek(u64 offset, u64 size);

void click(HidNpadButton btn);
void press(HidNpadButton btn);
void release(HidNpadButton btn);
void setStickState(int side, int dxVal, int dyVal);

u64 getMainNsoBase();
void pointerMode();
void BuildID(u8 joe[0x20]);
bool issame(u8 one[0x20], u8 two[0x20]);
#endif