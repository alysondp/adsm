#include "loader.h"

#include <windows.h>
#include <set>

#define PtrFromRva(base, rva) (((PBYTE)base) + rva)

namespace gmac { namespace loader {

static const char *ImportSection_ = ".idata";
static std::set<HMODULE> Modules_;

PVOID PatchModuleSymbol(HMODULE module, DWORD symbol, const char *name)
{
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)module;
	PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)PtrFromRva(dosHeader, dosHeader->e_lfanew);
	if(ntHeader->Signature != IMAGE_NT_SIGNATURE) return NULL;
	PIMAGE_IMPORT_DESCRIPTOR importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)
		PtrFromRva(dosHeader, 
		ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	VOID *ret = NULL;

	// This loop checks the imports required by the module (i.e., EXE or DLL)
	// We need to check the names imported by the module to check if the target
	// symbol is being used. This loop goes through all the DLLs the current module
	// use
	for(int i = 0; importDescriptor[i].Characteristics != 0; i++) {
		// Make sure that the import data for the descriptor is valid
		if(importDescriptor[i].FirstThunk == 0) continue;
		if(importDescriptor[i].OriginalFirstThunk == 0) continue;

		// Get the memory address of the names being imported
		PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA) PtrFromRva(dosHeader,
			importDescriptor[i].FirstThunk);
		PIMAGE_THUNK_DATA original = (PIMAGE_THUNK_DATA) PtrFromRva(dosHeader,
			importDescriptor[i].OriginalFirstThunk);

		// Check all the functions of the module being imported
		// This is quite overkilling, but the DLL the patched symbol is being imported
		// from is unknown, so we need to check all DLLs
		for(; original->u1.Function != NULL; original++, thunk++) {
			// Make sure this is a named import
			if(original->u1.Ordinal & IMAGE_ORDINAL_FLAG) continue; 
			PIMAGE_IMPORT_BY_NAME import = (PIMAGE_IMPORT_BY_NAME) PtrFromRva(dosHeader,
				original->u1.AddressOfData);
			if(strncmp((const char *)import, name, strlen(name)) != 0)
				continue; // No luck, check next name
			// So... finally, this module is importing the symbol from some DLL. Go and patch
			MEMORY_BASIC_INFORMATION memoryInfo;
			if(VirtualQuery(thunk, &memoryInfo, sizeof(MEMORY_BASIC_INFORMATION)) == 0)
				continue;
			if(VirtualProtect(memoryInfo.BaseAddress, memoryInfo.RegionSize,
				PAGE_READWRITE, &memoryInfo.Protect) == 0) continue;
			ret = (PVOID)(DWORD_PTR)thunk->u1.Function;
#ifdef _WIN32
			thunk->u1.Function = (DWORD)(DWORD_PTR) symbol;
#else
			thunk->u1.Function = (ULONGLONG)(DWORD_PTR) symbol;			
#endif
			DWORD dummy;
			VirtualProtect(memoryInfo.BaseAddress, memoryInfo.RegionSize,
				memoryInfo.Protect, &dummy);
			// We are done -- there shouldn't be any more symbols named like that in this module
			return ret;
		}
	}
	return ret;
}

PVOID PatchModule(HMODULE module, DWORD symbol, const char *name)
{
	if(module == NULL) {
		module = GetModuleHandle(NULL);
		if(module == NULL) return NULL;
	}

	PVOID ret = NULL;
	ret = PatchModuleSymbol(module, symbol, name);

	// Now check the DLLs within this module

	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)module;
	PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)PtrFromRva(dosHeader, dosHeader->e_lfanew);
	if(ntHeader->Signature != IMAGE_NT_SIGNATURE) return false;
	if(ntHeader->FileHeader.SizeOfOptionalHeader != sizeof(ntHeader->OptionalHeader)) return false;
	PIMAGE_SECTION_HEADER sections = (PIMAGE_SECTION_HEADER)PtrFromRva(ntHeader, sizeof(IMAGE_NT_HEADERS));

	PIMAGE_IMPORT_DESCRIPTOR dlls = NULL;
	for(int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++) {
		if(strncmp(ImportSection_, (const char *)sections[i].Name, strlen(ImportSection_)) != 0)
			continue;
		dlls = (PIMAGE_IMPORT_DESCRIPTOR)PtrFromRva(dosHeader, sections[i].VirtualAddress);
		break;
	}

	if(dlls == NULL) return false;
	for(int i = 0; dlls[i].Name != NULL; i++) {
		const char *dllName = (const char *)PtrFromRva(dosHeader, dlls[i].Name);
		HMODULE module = GetModuleHandle(dllName);
		if(module == NULL) continue;
		if(Modules_.insert(module).second == false) continue;
		PVOID second = PatchModule(module, symbol, name);
		if(ret != NULL && second != ret)
			gmac::util::Logger::Fatal("Duplicated symbol %s", name);
	}
	return ret;
}

void LoadSymbol(PVOID *symbol, DWORD hook, const char *name)
{
	// Patch the symbol in all IATs
	
	*symbol = (PVOID) PatchModule(NULL, hook, name);	
}
} }
