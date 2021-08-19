#include <Windows.h>
#include <iostream>
#include <vector>
#include <fstream>

DWORD game_base = (DWORD)GetModuleHandle("game.dll");
std::vector<std::string> blacklist;

inline bool patch(DWORD address, DWORD data, size_t size)
{
	if (!address || !size)
		return false;

	DWORD dwOldProtect;
	if (VirtualProtect((LPVOID)address, size, PAGE_EXECUTE_READWRITE, &dwOldProtect))
	{
		memcpy((LPVOID)address, (LPVOID)&data, size);
		VirtualProtect((LPVOID)address, size, dwOldProtect, nullptr);

		return true;
	}

	return false;
}

template <typename A>
inline bool call(DWORD address, A proc)
{
	return patch(address, 0xe8, 1) && patch(address + 1, (DWORD)proc - (address + 5), 4);
}

bool __fastcall DrawHeroIcon(DWORD offset);

//------------------------------------------------------------

BOOL APIENTRY DllMain(HMODULE hModule, UINT ul_reason_for_call, LPVOID lpReserve)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH && game_base)
	{
		// 1.26
		call(game_base + 0x370b8f, DrawHeroIcon);

		std::ifstream file("blacklist.txt");

		if (file)
		{
			while (!file.eof())
			{
				char buffer[MAX_PATH];
				ZeroMemory(buffer, sizeof(buffer));

				file.getline(buffer, sizeof(buffer));

				blacklist.push_back(buffer);
			}

			file.close();
		}
		
		// 1.27a
		// call(game_base + 0x3a9904, DrawHeroIcon);
	}
	

	return TRUE;
}

//------------------------------------------------------------

// 1.26
HANDLE __declspec(naked) Player(int index)
{
	_asm
	{
		mov eax, game_base
		add eax, 0x3bbb30
		jmp eax
	}
}

bool __declspec(naked) IsPlayerEnemy(HANDLE player_first, HANDLE player_second)
{
	_asm
	{
		mov eax, game_base
		add eax, 0x3c9580
		jmp eax
	}
}

bool Exists(std::string id)
{
	for (const auto& e : blacklist)
		if (!e.compare(id))
			return true;

	return false;
}

bool __fastcall DrawHeroIcon(DWORD offset)
{
	HANDLE player = Player (*(WORD*)(*(DWORD*)(game_base + 0xab65f4) + 0x28));
	HANDLE unit_owner = Player(reinterpret_cast<int(__fastcall*)(DWORD)>(*(DWORD*)(*(DWORD*)offset + 0xec))(offset));
	
	char id[5] = { 0 };
	CopyMemory(id, (LPVOID)(offset + 0x30), 4);
	char temp = id[0];
	id[0] = id[3];
	id[3] = temp;
	temp = id[1];
	id[1] = id[2];
	id[2] = temp;

	return player == unit_owner || (IsPlayerEnemy(player, unit_owner) && !Exists(id)) ? true : false;
}

// 1.27a
// HANDLE __declspec(naked) Player(int index)
// {
// 	_asm
// 	{
// 		mov eax, game_base
// 		add eax, 0x1f1e70 // File offset 0x1f1270
// 		jmp eax
// 	}
// }
// 
// bool __declspec(naked) IsPlayerEnemy(HANDLE player_first, HANDLE player_second)
// {
// 	_asm
// 	{
// 		mov eax, game_base
// 		add eax, 0x1e8090 // File offset 0x1e7490
// 		jmp eax
// 	}
// }
// 
// bool __fastcall DrawHeroIcon(DWORD offset)
// {
// 	HANDLE player = Player (*(WORD*)(*(DWORD*)(game_base + 0xbe4238) + 0x28));
// 	HANDLE unit_owner = Player(reinterpret_cast<int(__fastcall*)(DWORD)>(*(DWORD*)(*(DWORD*)offset + 0xec))(offset));
// 
//	char id[5] = { 0 };
//	CopyMemory(id, (LPVOID)(offset + 0x30), 4);
//	char temp = id[0];
//	id[0] = id[3];
//	id[3] = temp;
//	temp = id[1];
//	id[1] = id[2];
//	id[2] = temp;
//
// 	return player == unit_owner || (IsPlayerEnemy(player, unit_owner) && !Exists(id)) ? true : false;
// }
