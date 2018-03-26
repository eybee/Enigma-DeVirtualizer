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
Improved By Raham



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


#include "StreamParser.h"

bool StreamParser::ParseStream(BYTE* Stream,struct Node* myNode, struct PtrNode* myPtr, lModInfo mInf)
{
	BYTE currParam;
	DWORD result;
	DWORD currentPtr;
	int arrindex = 0;
	int counter;

	for (int count = 0; count < 3; count++)
	{
		currentPtr = (count*5)*4;

		switch (count)
		{
		case 0:
			currParam = Stream[0x19];
			break;
		case 1:
			currParam = Stream[0x2D];
			break;
		case 2:
			currParam = Stream[0x41];
			break;
		}

		myNode[count].IsRVA = false;
		myNode[count].PtrRead = false;
		myNode[count].NodePtr = 0;
		myNode[count].Value = 0;

		switch (Stream[currentPtr+0xC])
		{
		case 0x8C:
			myNode[count].Type = IsReg;
			myNode[count].Value = Stream[currentPtr+0x10];
			break;

		case 0x90:
			myNode[count].Type = IsImm;
			myNode[count].Value = *((DWORD*)(Stream + currentPtr + 0x1C));
			break;

		case 0x8D:
			myPtr[0].PtrValue = 0;
			myPtr[1].PtrValue = 0;
			myPtr[2].PtrValue = 0;
			myPtr[3].PtrValue = 0;
			myPtr[4].PtrValue = 0;
			myPtr[5].PtrValue = 0;
			myPtr[6].PtrValue = 0;

			myNode[count].NodePtr = myPtr;
			myNode[count].Type = IsPtr;
			if (Stream[currentPtr+0x1B] <= 0)
			{
				if (Stream[currentPtr+0x10]!=0)
				{
					myPtr[arrindex].OpType = IsReg;
					myPtr[arrindex].PtrValue = Stream[currentPtr+0x10];

					arrindex++;
				}
				if (Stream[currentPtr+0x14]!=0)
				{
					myPtr[arrindex].OpType = IsOperator;
					myPtr[arrindex].Operation = Plus;

					arrindex++;

					myPtr[arrindex].OpType = IsReg;
					myPtr[arrindex].PtrValue = Stream[currentPtr+0x14];

					arrindex++;
				}
				if (*((long*)(Stream + currentPtr + 0x1C))!=0)
				{
					myPtr[arrindex].OpType = IsOperator;
					myPtr[arrindex].Operation = Plus;

					arrindex++;

					myPtr[arrindex].OpType = IsImm;
					myPtr[arrindex].PtrValue = *((DWORD*)(Stream + currentPtr + 0x1C));

					arrindex++;

				}

			}
			else
			{
				if (Stream[currentPtr+0x10]!=0)
				{
					myPtr[arrindex].OpType = IsReg;
					myPtr[arrindex].PtrValue = Stream[currentPtr+0x10];

					arrindex++;
				}
				if (Stream[currentPtr+0x14]!=0)
				{
					myPtr[arrindex].OpType = IsOperator;
					myPtr[arrindex].Operation = Plus;

					arrindex++;

					myPtr[arrindex].OpType = IsReg;
					myPtr[arrindex].PtrValue = Stream[currentPtr+0x14];

					arrindex++;
				}
				if (Stream[currentPtr+0x1B]!=0)
				{
					myPtr[arrindex].OpType = IsOperator;
					myPtr[arrindex].Operation = Mul;

					arrindex++;

					myPtr[arrindex].OpType = IsImm;
					myPtr[arrindex].PtrValue = Stream[currentPtr+0x1B];

					arrindex++;
				}
				if (*((DWORD*)(Stream + currentPtr + 0x1C))!=0)
				{
					myPtr[arrindex].OpType = IsOperator;
					myPtr[arrindex].Operation = Plus;

					arrindex++;

					myPtr[arrindex].OpType = IsImm;
					myPtr[arrindex].PtrValue = *((DWORD*)(Stream + currentPtr + 0x1C));

					arrindex++;
				}

			}
			if (Stream[currentPtr+0x18])
			{
				myNode[count].IsRVA = true;
			}

			if (Stream[0]!=0x51 || !count)
			{
				myNode[count].PtrRead = true;
				switch (currParam)
				{
				case 0x8:
					myNode[count].Size = Byte;
					break;
				case 0x10:
					myNode[count].Size = Word;
					break;
				default:
					myNode[count].Size = DWord;
				}
			}			
			break;

		case 0x8F:
			myNode[count].Type = IsImm;
			myNode[count].Value = *((DWORD*)(Stream + currentPtr + 0x1C));
			if (Stream[currentPtr+0x18])
			{
				myNode[count].IsRVA = true;
				myNode[count].Value += mInf.lModVA;
			}
			break;
			
		case 0x91:
			myNode[count].Type = IsImm;
			myNode[count].Value = *((DWORD*)(Stream + currentPtr + 0x1C));
			if (Stream[currentPtr+0x18])
			{
				myNode[count].IsRVA = true;
				myNode[count].Value += mInf.lModVA;
			}
			break;
		}
	}
	return true;
}


bool StreamParser::ExistNodePtr(PtrNode NodePtr)
{
	if (!NodePtr.PtrValue)
	{
		return false;
	}

	return true;
}

bool StreamParser::ExistNode(Node myNode)
{
	if (!myNode.Value && !myNode.NodePtr)
	{
		return false;
	}

	return true;
}