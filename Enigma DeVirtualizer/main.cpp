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
Compile this source with MSVC++2010 to a native 32bit DLL.
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


#include <map>
#include "windows.h"
#include "global.h"
#include "VM.h"

#pragma comment(lib,"TitanEngine_x86.lib")

bool InnerDevirtualize(void);			// Devirtualize Enigma internal code
DWORD OuterDevirtualize(DWORD);			// Devirtualize Applications code

BOOL APIENTRY DllMain (HINSTANCE hInst, DWORD reason, LPVOID reserved){
	return true;
}

DWORD OuterDevirtualize(DWORD intDest)
{
	VM outerVM;

	if (outerVM.DevirtualizeExternal(intDest))
		return outerVM.pDecompiledCode;
	else
	return 0;

	
}

bool InnerDevirtualize(void)
{
	VM myVM;

	return myVM.DevirtualizeInternal();
	
}