/*
The Enigma Protector v2.x - v3.x 32bit Devirtualizer
This tool is designed to translate the virtual opcodes of the Enigma
VM back to normal x86 Code.

Author: DizzY_D & Raham
Version: 1.4

Credits:
Ap0x for TitanEngine
herumi for XBYAK
f0Gx, n0p0x90, BlackBerry for all their great support

Usage:
Load the DLL into the target's process space.
For the first (Enigma internal) VM use the "InnerDevirtualize" function
exported by this DLL.
For the second (SDK) VM use the "OuterDevirtualize" function exported by
this DLL and pass a pointer to the bytecode as a parameter.
The VM code should now automatically be fixed and run completely
independent from the Enigma VM.

Notes:
This was one of my first c++ projects so please don't expect super
perfect code. I also didn't implement exception handling and such, but 
now it's open source, so why not implementing it yourself?
It would be nice to see if someone would take up this project and prepare
it for future versions of Enigma.



Copyright (C) 2012 DizzY_D
Improved By Raham.


This program is free software; you can redistribute it and/or modify it 
under the terms of the GNU General Public License as published by the 
Free Software Foundation; either version 3 of the License, or (at your 
option) any later version.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General 
Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program; if not, see <http://www.gnu.org/licenses/>.
*/


#include "Relocater.h"


bool Relocater::Relocate(std::map<DWORD,DWORD> &RelocMapKey, std::map<DWORD,DWORD> &RelocMapAddr, DWORD FileMapVA)
{
	std::map<DWORD,DWORD> RelocMap;
	DWORD val = 0;
	int i = 0;

	for(std::map<DWORD,DWORD>::iterator n = RelocMapAddr.begin(); n != RelocMapAddr.end(); n++)
	{
		RelocMap[n->first] = n->second;
	}
	for(std::map<DWORD,DWORD>::iterator n = RelocMapKey.begin(); n != RelocMapKey.end(); n++)
	{
		RelocMap[n->first] = n->second;
	}
	for(std::map<DWORD,DWORD>::iterator n = RelocMap.begin(); n != RelocMap.end(); n++)
	{
			val = n->second - n->first - 4;
			memcpy((void *)n->first, &val, 4);
	}

	//return RelocateKey(RelocMapKey, typesPtr) && RelocateAddr(RelocMapAddr,FileMapVA, typesPtr);
	return true;
}


bool Relocater::RelocateAddr(std::map<DWORD,DWORD> &RelocMapAddr, DWORD FileMapVA, DWORD typesPtr)
{
	DWORD val = 0;

	for(std::map<DWORD,DWORD>::iterator n = RelocMapAddr.begin(); n != RelocMapAddr.end(); n++)
	{
		
		//val = ConvertFileOffsetToVA(FileMapVA, n->first, true);
		val = n->second - n->first - 4;
		
		memcpy((void *)n->first, &val, 4);
	}


	return true;
}

bool Relocater::RelocateKey(std::map<DWORD,DWORD> &RelocMap, DWORD typesPtr)
{
	DWORD val = 0;

	for(std::map<DWORD,DWORD>::iterator n = RelocMap.begin(); n != RelocMap.end(); n++)
	{
		val = n->second - n->first  - 4;
		memcpy((void *)n->first, &val, 4);
	}

	return true;
}


bool Relocater::ConvertKeyRefs(std::map<DWORD,DWORD> &RelocMap, std::map<int, DWORD> &KeyMap)
{
	for(std::map<DWORD,DWORD>::iterator n = RelocMap.begin(); n != RelocMap.end(); n++)
	{
		RelocMap[n->first] = KeyMap[RelocMap[n->first]];
	}
	return true;
}

bool Relocater::FixEntries(std::map<DWORD,refs> &log_list, std::map<DWORD,DWORD> &RelocMapAddr, std::map<int,DWORD> &KeyMap, DWORD FileMapVA)
{
	BYTE nopop[] = {0x90, 0x90, 0x90, 0x90, 0x90};
	DWORD valx, val, val2;
	

	for(std::map<DWORD,refs>::iterator n = log_list.begin(); n != log_list.end(); n++)
	{
		valx = n->second.key;
		val = KeyMap[valx];
		//val2 = ConvertFileOffsetToVA(FileMapVA, val, true);
		RelocMapAddr[n->first+6] = val;
		memcpy((void *)n->first, &nopop, 5);
	}
	return true;
}