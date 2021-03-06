// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 头文件
#include <Windows.h>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
#pragma comment(linker, "/EXPORT:GetFileVersionInfoA=_AheadLib_GetFileVersionInfoA,@1")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoByHandle=_AheadLib_GetFileVersionInfoByHandle,@2")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoExA=_AheadLib_GetFileVersionInfoExA,@3")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoExW=_AheadLib_GetFileVersionInfoExW,@4")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeA=_AheadLib_GetFileVersionInfoSizeA,@5")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeExA=_AheadLib_GetFileVersionInfoSizeExA,@6")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeExW=_AheadLib_GetFileVersionInfoSizeExW,@7")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoSizeW=_AheadLib_GetFileVersionInfoSizeW,@8")
#pragma comment(linker, "/EXPORT:GetFileVersionInfoW=_AheadLib_GetFileVersionInfoW,@9")
#pragma comment(linker, "/EXPORT:VerFindFileA=_AheadLib_VerFindFileA,@10")
#pragma comment(linker, "/EXPORT:VerFindFileW=_AheadLib_VerFindFileW,@11")
#pragma comment(linker, "/EXPORT:VerInstallFileA=_AheadLib_VerInstallFileA,@12")
#pragma comment(linker, "/EXPORT:VerInstallFileW=_AheadLib_VerInstallFileW,@13")
#pragma comment(linker, "/EXPORT:VerLanguageNameA=_AheadLib_VerLanguageNameA,@14")
#pragma comment(linker, "/EXPORT:VerLanguageNameW=_AheadLib_VerLanguageNameW,@15")
#pragma comment(linker, "/EXPORT:VerQueryValueA=_AheadLib_VerQueryValueA,@16")
#pragma comment(linker, "/EXPORT:VerQueryValueW=_AheadLib_VerQueryValueW,@17")
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 宏定义
#define EXTERNC extern "C"
#define NAKED __declspec(naked)
#define EXPORT __declspec(dllexport)

#define ALCPP EXPORT NAKED
#define ALSTD EXTERNC EXPORT NAKED void __stdcall
#define ALCFAST EXTERNC EXPORT NAKED void __fastcall
#define ALCDECL EXTERNC NAKED void __cdecl
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AheadLib 命名空间
namespace AheadLib
{
	HMODULE m_hModule = NULL;	// 原始模块句柄
	DWORD m_dwReturn[17] = { 0 };	// 原始函数返回地址


	// 加载原始模块
	inline BOOL WINAPI Load()
	{
		TCHAR tzPath[MAX_PATH];
		TCHAR tzTemp[MAX_PATH * 2];

		GetSystemDirectory(tzPath, MAX_PATH);
		lstrcat(tzPath, TEXT("\\version.dll"));
		m_hModule = LoadLibrary(tzPath);
		if (m_hModule == NULL)
		{
			wsprintf(tzTemp, TEXT("无法加载 %s，程序无法正常运行。"), tzPath);
			MessageBox(NULL, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
		}

		return (m_hModule != NULL);
	}

	// 释放原始模块
	inline VOID WINAPI Free()
	{
		if (m_hModule)
		{
			FreeLibrary(m_hModule);
		}
	}

	// 获取原始函数地址
	FARPROC WINAPI GetAddress(PCSTR pszProcName)
	{
		FARPROC fpAddress;
		CHAR szProcName[16];
		TCHAR tzTemp[MAX_PATH];

		fpAddress = GetProcAddress(m_hModule, pszProcName);
		if (fpAddress == NULL)
		{
			if (HIWORD(pszProcName) == 0)
			{
				wsprintf(szProcName, "%d", pszProcName);
				pszProcName = szProcName;
			}

			wsprintf(tzTemp, TEXT("无法找到函数 %hs，程序无法正常运行。"), pszProcName);
			MessageBox(NULL, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
			ExitProcess(-2);
		}

		return fpAddress;
	}
}
using namespace AheadLib;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Psapi.h>
#pragma comment(lib,"Psapi.lib")

BOOL EnableDebugPriv()
{
	HANDLE   hToken;
	LUID   sedebugnameValue;
	TOKEN_PRIVILEGES   tkp;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		return   FALSE;
	}

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue))
	{
		CloseHandle(hToken);
		return   FALSE;
	}
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		return   FALSE;
	}
	CloseHandle(hToken);
	return TRUE;

}
#include "MemoryManager.h"

DWORD WINAPI ThreadPatch(LPVOID p)
{
	char szSelf[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, szSelf, MAX_PATH);
	strlwr(szSelf);
	if (strstr(szSelf, "qq.exe"))
	{
		EnableDebugPriv();

		while (GetModuleHandleA("im.dll") == NULL)
		{
			Sleep(1000);
		}
		Sleep(1000);
		MODULEINFO mi;
		GetModuleInformation(GetCurrentProcess(), GetModuleHandleA("im.dll"), &mi, sizeof(mi));
		DWORD dwSartAddr = (DWORD)GetModuleHandleA("im.dll");
		DWORD dwEndAddr = dwSartAddr + mi.SizeOfImage;

		MemoryManager *pMM = new MemoryManager(GetCurrentProcess());

		BYTE bTag[] =
		{
			0x8B,0x75,0x14,0x8D,0x4D,0xF4,0x83,0xC4,0x20,0x33,0xFF,0x89,0x7D,0xF4,0x8B,0x06,0x51,0x68,'?','?','?','?',0x56,0xFF,0x50,0x78,0x85,0xC0,0x79,0x39
		};
		std::vector<DWORD> vAddr;
		pMM->MemSearch(bTag, sizeof(bTag), dwSartAddr, dwEndAddr, TRUE, 0, vAddr);

		if (vAddr.size() != 1)
		{
			MessageBoxA(NULL, "tips", "查找对话撤回特征错误", MB_ICONERROR);
			return 1;
		}

		//准备patch
		DWORD dwPatchAddr = vAddr[0] + 28;
		DWORD dwOld;
		VirtualProtect((LPVOID)dwPatchAddr, 2, PAGE_EXECUTE_READWRITE, &dwOld);
		BYTE bPatch[2] = { 0x90,0x90 };
		memcpy((LPVOID)dwPatchAddr, bPatch, 2);
		VirtualProtect((LPVOID)dwPatchAddr, 2, dwOld, &dwOld);


		//群消息防撤回特征
		BYTE bTag2[] =
		{
			0x8B,0x75,0x08,0x8D,0x45,0xF4,0x50,0xFF,0x75,0x10,0x33,0xDB,0x56,0x89,0x5D,0xF4
		};
		vAddr.clear();
		pMM->MemSearch(bTag2, sizeof(bTag2), dwSartAddr, dwEndAddr, TRUE, 1, vAddr);

		if (vAddr.size() != 1)
		{
			MessageBoxA(NULL, "tips", "查找群聊撤回特征错误", MB_ICONERROR);
			return 1;
		}
		//准备patch
		dwPatchAddr = vAddr[0] + 0x1A;
		VirtualProtect((LPVOID)dwPatchAddr, 2, PAGE_EXECUTE_READWRITE, &dwOld);
		memcpy((LPVOID)dwPatchAddr, bPatch, 2);
		VirtualProtect((LPVOID)dwPatchAddr, 2, dwOld, &dwOld);

	}
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 入口函数
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		for (INT i = 0; i < sizeof(m_dwReturn) / sizeof(DWORD); i++)
		{
			m_dwReturn[i] = TlsAlloc();
		}
		CloseHandle(CreateThread(0, 0, ThreadPatch, 0, 0, 0));
		return Load();
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		for (INT i = 0; i < sizeof(m_dwReturn) / sizeof(DWORD); i++)
		{
			TlsFree(m_dwReturn[i]);
		}

		Free();
	}

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_GetFileVersionInfoA(void)
{
	GetAddress("GetFileVersionInfoA");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_GetFileVersionInfoByHandle(void)
{
	GetAddress("GetFileVersionInfoByHandle");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_GetFileVersionInfoExA(void)
{
	GetAddress("GetFileVersionInfoExA");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_GetFileVersionInfoExW(void)
{
	GetAddress("GetFileVersionInfoExW");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_GetFileVersionInfoSizeA(void)
{
	GetAddress("GetFileVersionInfoSizeA");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_GetFileVersionInfoSizeExA(void)
{
	GetAddress("GetFileVersionInfoSizeExA");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_GetFileVersionInfoSizeExW(void)
{
	GetAddress("GetFileVersionInfoSizeExW");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_GetFileVersionInfoSizeW(void)
{
	GetAddress("GetFileVersionInfoSizeW");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_GetFileVersionInfoW(void)
{
	GetAddress("GetFileVersionInfoW");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_VerFindFileA(void)
{
	GetAddress("VerFindFileA");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_VerFindFileW(void)
{
	GetAddress("VerFindFileW");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_VerInstallFileA(void)
{
	GetAddress("VerInstallFileA");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_VerInstallFileW(void)
{
	GetAddress("VerInstallFileW");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_VerLanguageNameA(void)
{
	GetAddress("VerLanguageNameA");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_VerLanguageNameW(void)
{
	GetAddress("VerLanguageNameW");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_VerQueryValueA(void)
{
	GetAddress("VerQueryValueA");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_VerQueryValueW(void)
{
	GetAddress("VerQueryValueW");
	__asm JMP EAX;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
