#include "arduino.h"
#include "memory.h"
#include <iostream>
#include <string>

memory mem;
PModule bClient;
using namespace std;

//changing on updates
//current: 2022-08-16 22:03:17.146872300 UTC
//updated offsets: https://github.com/frk1/hazedumper/blob/master/csgo.hpp
#define dwLocalPlayer	0xDC04CC
#define dwEntityList	0x4DDC90C
#define m_iCrossHairID	0x11838
//changing on updates

//non-changing
#define m_iTeamNum		0xF4
#define m_iHealth		0x100
#define m_fFlags		0x104
#define FL_ONGROUND		257
//non-changing

int main()
{
	system("Color 4");

	printf(R"EOF( 
    __  __ _       __ ______ ____                ______ _____  ______ ____ 
   / / / /| |     / //_  __// __ )              / ____// ___/ / ____// __ \
  / /_/ / | | /| / /  / /  / __  |             / /     \__ \ / / __ / / / /
 / __  /  | |/ |/ /  / /  / /_/ /             / /___  ___/ // /_/ // /_/ / 
/_/ /_/   |__/|__/  /_/  /_____/    ______    \____/ /____/ \____/ \____/  
                                   /_____/                                 
)EOF");

	uintptr_t  TDelay;
	std::cout << "\n[!] Trigger Delay (ms) = ";
	std::cin >> TDelay;


	arduino duino("Arduino Leonardo");
	int delay = 0;
	int count = 0;
	while (true)
	{
		DWORD pLocal = mem.Read<DWORD>(bClient.dwBase + dwLocalPlayer);
		BYTE fFlags = mem.Read<DWORD>(pLocal + m_fFlags);
		if (GetAsyncKeyState(VK_XBUTTON1) & 0x8000 && (fFlags & FL_ONGROUND))
		{
			if ((fFlags & 1))
			{
				count++;
				//DEBUG: printf("[!] Jumps made: [%d]  \n", count);

				if (count > rand() % 5 + 3) //Hops before change of delay
				{
					delay = rand() % 14 + 10; //Random delays to make us mess up sometimes
					count = 0;
				}
				else
				{
					delay = 10;
				}

				Sleep(delay);
				char buffer[] = ".";
				duino.send_data(buffer, sizeof(buffer));
				Sleep(delay);
			}
			Sleep(1);
		}

		while (!mem.Attach("csgo.exe", PROCESS_ALL_ACCESS)) { Sleep(1000); }
		bClient = mem.GetModule("client.dll");

		cout << "[!] client.dll address found: " << bClient.dwBase << endl;
		std::cout.rdbuf(nullptr);

		{
			DWORD pLocal = mem.Read<DWORD>(bClient.dwBase + dwLocalPlayer);
			DWORD lHealth = mem.Read<DWORD>(pLocal + m_iHealth);

			if (!lHealth)
				continue;

			DWORD lTeamNum = mem.Read<DWORD>(pLocal + m_iTeamNum);


			DWORD lCrosshairId = mem.Read<DWORD>(pLocal + m_iCrossHairID);
			if (!lCrosshairId || lCrosshairId > 64)
				continue;

			DWORD entityBase = mem.Read<int>(bClient.dwBase + dwEntityList + (lCrosshairId - 1) * 0x10);
			if (!entityBase)
				continue;
			DWORD entityHp = mem.Read<DWORD>(entityBase + m_iHealth);
			DWORD entityTeam = mem.Read<DWORD>(entityBase + m_iTeamNum);

			if (!entityHp || entityTeam == lTeamNum)
				continue;

			if (GetAsyncKeyState(VK_XBUTTON2))
			{
				char buffers[] = "-";
				Sleep(TDelay);
				duino.send_data(buffers, sizeof(buffers));
			}
		}
	}
}