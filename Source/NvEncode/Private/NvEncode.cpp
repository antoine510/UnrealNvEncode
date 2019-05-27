#include "NvEncodePCH.h"
#include "NvEncode.h"
#include "Core.h"
#include "ModuleManager.h"

bool NvEncodeModule::initialized = false;

namespace NvEncode {
	extern void LogMessage(const char* msg);
}

void NvEncodeModule::StartupModule() {
	initialized = true;
}

void NvEncodeModule::ShutdownModule() {
	initialized = false;
}

IMPLEMENT_MODULE(NvEncodeModule, NvEncode);
