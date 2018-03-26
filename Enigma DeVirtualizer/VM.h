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


#pragma once

#include "global.h"
#include "SDK.h"
#include <map>
#include "VCodeHandler.h"
#include "Relocater.h"

class VM
{
public:
	VM();																// Standard Constructor
	bool DevirtualizeInternal(void);									// Devirtualize Enigma VDLL
	DWORD DevirtualizeExternal(DWORD);							// Devirtualize Target Code
	//bool DevirtualizeByAddress();
	DWORD pDecompiledCode;
protected:
private:
	DWORD pushXOR;
	DWORD pBYTECODE;
	int DetectVMVersion(DWORD vMAddr);									// Detect Enigma Version by number of VM Handler
	DWORD GetVMAddrFromDispatcher(DWORD dispatcher);					// Get Handler Control Flow Function by VM Start Address
	lModInfo GetSectionBase(int secNum);								// Get Section Address from Section Number
	BOOL FindVM(lModInfo modInfo);										// Find internal and external VM and fill dispatcher array
	DWORD FindDispatcher(DWORD);										// Get VM dispatcher address
	//DWORD FindByteCode(lModInfo modInfo, int numberOfHandler);			// Get virtual bytecode start address
	DWORD VM::FindByteCode(DWORD dispatcher);
	std::map<DWORD,refs> LogVParts(DWORD, lModInfo modInfo, int);		// Log all VM entry PUSH/JMPs and fill key+address for relocater
	DWORD CalcByteCodeVAfromPushVal(DWORD,DWORD);						// Calculate byte code address from key value
	void ConvertByteCode(std::map<DWORD, refs>, std::map<DWORD,DWORD> &RelocMap, std::map<DWORD, DWORD> &RelocMapAddr,std::map<int, DWORD> &KeyMap, lModInfo mInf);		// Byte code converter routine
	lModInfo modInfo;
	DWORD dispatcher[2];
	DWORD FindXORValue(DWORD dispatcher);
	std::vector<VirtualCodeHandler*> handler;
};

