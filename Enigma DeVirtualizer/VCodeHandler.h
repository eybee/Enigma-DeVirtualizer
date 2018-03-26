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

#include <vector>
#include "StreamParser.h"
#include "xbyak/xbyak.h"
#include "global.h"

enum EReg
{
	vAL = 1, vCL, vDL, vBL, vAX = 0x15, vCX, vDX, vBX, vSP, vBP, vSI, vDI, vEAX = 0x25, vECX ,vEDX ,vEBX ,vESP ,vEBP ,vESI ,vEDI 
};

class VirtualCodeHandler : public Xbyak::CodeGenerator
{
public:
	StreamParser myParse;
	StreamParser::Node myNode[3];
	StreamParser::PtrNode myPtr[7];
	DWORD vDllBase;

	virtual bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf) = 0;

	Xbyak::Address AssemblePtr(StreamParser::PtrNode* &myPtr, bool isRVA, lModInfo mInf, StreamParser::ESize pSize = StreamParser::NotDefined)
	{
		DWORD vDllBase = mInf.lModVA;
		int regbase = vEAX;
		int reg1, reg2, index, base;

		StreamParser myParse;

		reg1 = reg2 = index = base = 0;


		if (myPtr[0].OpType == StreamParser::IsOperator)
		{
			for (int i = 0; i <= 5; i++)
			{
				myPtr[i] = myPtr[i+1];
			}
		}

		if (pSize == StreamParser::NotDefined)
		{
			if (myParse.ExistNodePtr(myPtr[6]))
			{
				reg1 = myPtr[0].PtrValue - regbase;
				reg2 = myPtr[2].PtrValue - regbase;
				index = myPtr[4].PtrValue;
				base = myPtr[6].PtrValue;

				if (isRVA) base += vDllBase;

				
				return ptr [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2) * index + base];				
			}

			if (!myParse.ExistNodePtr(myPtr[5]) && myParse.ExistNodePtr(myPtr[4]))
			{
				reg1 = myPtr[0].PtrValue - regbase;
				reg2 = myPtr[2].PtrValue - regbase;

				if (myPtr[3].Operation == StreamParser::Mul)
				{
					index = myPtr[4].PtrValue;
					return ptr [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2) * index];
				}

				if (myPtr[1].Operation == StreamParser::Mul)
				{
					reg2 = myPtr[0].PtrValue - regbase;
					index = myPtr[2].PtrValue;
					base = myPtr[4].PtrValue;
					if (isRVA) base += vDllBase;

					return ptr [Xbyak::Reg32(reg2) * index + base];
				}


				if (myPtr[3].Operation == StreamParser::Plus)
				{
					base = myPtr[4].PtrValue;
					if (isRVA) base += vDllBase;
					return ptr [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2) + base];;
				}

			}

			if (!myParse.ExistNodePtr(myPtr[3]) && myParse.ExistNodePtr(myPtr[2]))
			{
				reg1 = myPtr[0].PtrValue - regbase;

				if (myPtr[1].Operation == StreamParser::Mul)
				{
					index = myPtr[2].PtrValue;
					return ptr [Xbyak::Reg32(reg1) * index];
				}

				if (myPtr[1].Operation == StreamParser::Plus)
				{
					if (myPtr[2].OpType == StreamParser::IsReg)
					{
						reg2 = myPtr[2].PtrValue - regbase;
						return ptr [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2)];
					}
					else
					{
						base = myPtr[2].PtrValue;
						if (isRVA) base += vDllBase;
						return ptr [Xbyak::Reg32(reg1) + base];
					}
				}
			}
		
			if (!myParse.ExistNodePtr(myPtr[1]) && myParse.ExistNodePtr(myPtr[0]))
			{
				if (myPtr[0].OpType == StreamParser::IsReg)
				{
					reg1 = myPtr[0].PtrValue - regbase;
					return ptr [Xbyak::Reg32(reg1)];
				}
				else
				{
					base = myPtr[0].PtrValue;
					if (isRVA) base += vDllBase;
					return ptr [(void*)base];
				}
			
			}
		}

		if (pSize == StreamParser::DWord)
		{
			if (myParse.ExistNodePtr(myPtr[6]))
			{
				reg1 = myPtr[0].PtrValue - regbase;
				reg2 = myPtr[2].PtrValue - regbase;
				index = myPtr[4].PtrValue;
				base = myPtr[6].PtrValue;

				if (isRVA) base += vDllBase;

				return dword [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2) * index + base];
			}

			if (!myParse.ExistNodePtr(myPtr[5]) && myParse.ExistNodePtr(myPtr[4]))
			{
				reg1 = myPtr[0].PtrValue - regbase;
				reg2 = myPtr[2].PtrValue - regbase;

				if (myPtr[3].Operation == StreamParser::Mul)
				{
					index = myPtr[4].PtrValue;
					return dword [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2) * index];
				}

				if (myPtr[1].Operation == StreamParser::Mul)
				{
					reg2 = myPtr[0].PtrValue - regbase;
					index = myPtr[2].PtrValue;
					base = myPtr[4].PtrValue;
					if (isRVA) base += vDllBase;

					return dword [Xbyak::Reg32(reg2) * index + base];
				}

				if (myPtr[3].Operation == StreamParser::Plus)
				{
					base = myPtr[4].PtrValue;
					if (isRVA) base += vDllBase;
					return dword [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2) + base];;
				}

			}

			if (!myParse.ExistNodePtr(myPtr[3]) && myParse.ExistNodePtr(myPtr[2]))
			{
				reg1 = myPtr[0].PtrValue - regbase;

				if (myPtr[1].Operation == StreamParser::Mul)
				{
					index = myPtr[2].PtrValue;
					return dword [Xbyak::Reg32(reg1) * index];
				}

				if (myPtr[1].Operation == StreamParser::Plus)
				{
					if (myPtr[2].OpType == StreamParser::IsReg)
					{
						reg2 = myPtr[2].PtrValue - regbase;
						return dword [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2)];
					}
					else
					{
						base = myPtr[2].PtrValue;
						if (isRVA) base += vDllBase;
						return dword [Xbyak::Reg32(reg1) + base];
					}
				}
			}

			if (!myParse.ExistNodePtr(myPtr[1]) && myParse.ExistNodePtr(myPtr[0]))
			{
				if (myPtr[0].OpType == StreamParser::IsReg)
				{
					reg1 = myPtr[0].PtrValue - regbase;
					return dword [Xbyak::Reg32(reg1)];
				}
				else
				{
					base = myPtr[0].PtrValue;
					if (isRVA) base += vDllBase;
					return dword [(void*)base];
				}

			}
		}

		if (pSize == StreamParser::Word)
		{
			if (myParse.ExistNodePtr(myPtr[6]))
			{
				reg1 = myPtr[0].PtrValue - regbase;
				reg2 = myPtr[2].PtrValue - regbase;
				index = myPtr[4].PtrValue;
				base = myPtr[6].PtrValue;

				if (isRVA) base += vDllBase;

				return word [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2) * index + base];
			}

			if (!myParse.ExistNodePtr(myPtr[5]) && myParse.ExistNodePtr(myPtr[4]))
			{
				reg1 = myPtr[0].PtrValue - regbase;
				reg2 = myPtr[2].PtrValue - regbase;

				if (myPtr[3].Operation == StreamParser::Mul)
				{
					index = myPtr[4].PtrValue;
					return word [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2) * index];
				}

				if (myPtr[1].Operation == StreamParser::Mul)
				{
					reg2 = myPtr[0].PtrValue - regbase;
					index = myPtr[2].PtrValue;
					base = myPtr[4].PtrValue;
					if (isRVA) base += vDllBase;

					return word [Xbyak::Reg32(reg2) * index + base];
				}

				if (myPtr[3].Operation == StreamParser::Plus)
				{
					base = myPtr[4].PtrValue;
					if (isRVA) base += vDllBase;
					return word [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2) + base];;
				}

			}

			if (!myParse.ExistNodePtr(myPtr[3]) && myParse.ExistNodePtr(myPtr[2]))
			{
				reg1 = myPtr[0].PtrValue - regbase;

				if (myPtr[1].Operation == StreamParser::Mul)
				{
					index = myPtr[2].PtrValue;
					return word [Xbyak::Reg32(reg1) * index];
				}

				if (myPtr[1].Operation == StreamParser::Plus)
				{
					if (myPtr[2].OpType == StreamParser::IsReg)
					{
						reg2 = myPtr[2].PtrValue - regbase;
						return word [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2)];
					}
					else
					{
						base = myPtr[2].PtrValue;
						if (isRVA) base += vDllBase;
						return word [Xbyak::Reg32(reg1) + base];
					}
				}
			}

			if (!myParse.ExistNodePtr(myPtr[1]) && myParse.ExistNodePtr(myPtr[0]))
			{
				if (myPtr[0].OpType == StreamParser::IsReg)
				{
					reg1 = myPtr[0].PtrValue - regbase;
					return word [Xbyak::Reg32(reg1)];
				}
				else
				{
					base = myPtr[0].PtrValue;
					if (isRVA) base += vDllBase;
					return word [(void*)base];
				}

			}
		}

		if (pSize == StreamParser::Byte)
		{
			if (myParse.ExistNodePtr(myPtr[6]))
			{
				reg1 = myPtr[0].PtrValue - regbase;
				reg2 = myPtr[2].PtrValue - regbase;
				index = myPtr[4].PtrValue;
				base = myPtr[6].PtrValue;

				if (isRVA) base += vDllBase;

				return byte [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2) * index + base];
			}

			if (!myParse.ExistNodePtr(myPtr[5]) && myParse.ExistNodePtr(myPtr[4]))
			{
				reg1 = myPtr[0].PtrValue - regbase;
				reg2 = myPtr[2].PtrValue - regbase;

				if (myPtr[3].Operation == StreamParser::Mul)
				{
					index = myPtr[4].PtrValue;
					return byte [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2) * index];
				}

				if (myPtr[1].Operation == StreamParser::Mul)
				{
					reg2 = myPtr[0].PtrValue - regbase;
					index = myPtr[2].PtrValue;
					base = myPtr[4].PtrValue;
					if (isRVA) base += vDllBase;

					return byte [Xbyak::Reg32(reg2) * index + base];
				}

				if (myPtr[3].Operation == StreamParser::Plus)
				{
					base = myPtr[4].PtrValue;
					if (isRVA) base += vDllBase;
					return byte [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2) + base];;
				}

			}

			if (!myParse.ExistNodePtr(myPtr[3]) && myParse.ExistNodePtr(myPtr[2]))
			{
				reg1 = myPtr[0].PtrValue - regbase;

				if (myPtr[1].Operation == StreamParser::Mul)
				{
					index = myPtr[2].PtrValue;
					return byte [Xbyak::Reg32(reg1) * index];
				}

				if (myPtr[1].Operation == StreamParser::Plus)
				{
					if (myPtr[2].OpType == StreamParser::IsReg)
					{
						reg2 = myPtr[2].PtrValue - regbase;
						return byte [Xbyak::Reg32(reg1) + Xbyak::Reg32(reg2)];
					}
					else
					{
						base = myPtr[2].PtrValue;
						if (isRVA) base += vDllBase;
						return byte [Xbyak::Reg32(reg1) + base];
					}
				}
			}

			if (!myParse.ExistNodePtr(myPtr[1]) && myParse.ExistNodePtr(myPtr[0]))
			{
				if (myPtr[0].OpType == StreamParser::IsReg)
				{
					reg1 = myPtr[0].PtrValue - regbase;
					return byte [Xbyak::Reg32(reg1)];
				}
				else
				{
					base = myPtr[0].PtrValue;
					if (isRVA) base += vDllBase;
					return byte [(void*)base];
				}

			}
		}
	};
		
private:
};

class Nop : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x03 || Index == 0x62 || Index == 0x63 || Index == 0x64)
		{
			size_ = 0;
			nop();

			return true;
		}

		return false;
	}
};


class Idiv : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0x07)
		{
			Idiv::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
				{
					base = vAL;
					reg[0] = myNode[0].Value - base;

					idiv(Xbyak::Reg8(reg[0]));

				}

				if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
				{
					base = vAX;
					reg[0] = myNode[0].Value - base;

					idiv(Xbyak::Reg16(reg[0]));

				}

				if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
				{
					base = vEAX;
					reg[0] = myNode[0].Value - base;

					idiv(Xbyak::Reg32(reg[0]));
				}
				break;

			case StreamParser::IsPtr:
				idiv(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size));
				break;
			}

			return true;
		}

		return false;
	}
};

class Div : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0x08)
		{
			Div::size_ = 0;
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
				{
					base = vAL;
					reg[0] = myNode[0].Value - base;

					div(Xbyak::Reg8(reg[0]));

				}

				if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
				{
					base = vAX;
					reg[0] = myNode[0].Value - base;

					div(Xbyak::Reg16(reg[0]));

				}

				if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
				{
					base = vEAX;
					reg[0] = myNode[0].Value - base;

					div(Xbyak::Reg32(reg[0]));
				}
				break;

			case StreamParser::IsPtr:
				div(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size));
				break;
			}

			return true;
		}

		return false;
	}
};


class Imul : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0x09)
		{
			Imul::size_ = 0;
			
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			if (!myParse.ExistNode(myNode[1]) && !myParse.ExistNode(myNode[2]))		//1 operand
			{
				switch (myNode[0].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;
					
						imul(Xbyak::Reg8(reg[0]));
						

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						imul(Xbyak::Reg16(reg[0]));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						imul(Xbyak::Reg32(reg[0]));
					}
					break;

				case StreamParser::IsPtr:
					imul(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size));
					break;
				}
			}
				if (!myParse.ExistNode(myNode[2]))		//2 operands
				{
					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						switch (myNode[1].Type)
						{
						case StreamParser::IsReg:
							reg[1] = myNode[1].Value - base;
							imul(Xbyak::Reg16(reg[0]),Xbyak::Reg16(reg[1]));
							break;
						case StreamParser::IsPtr:
							imul(Xbyak::Reg16(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf, StreamParser::Word));
							break;
						}

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						switch (myNode[1].Type)
						{
						case StreamParser::IsReg:
							reg[1] = myNode[1].Value - base;
							imul(Xbyak::Reg32(reg[0]),Xbyak::Reg32(reg[1]));
							break;
						case StreamParser::IsPtr:
							imul(Xbyak::Reg32(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf, StreamParser::DWord));
							break;
						}

					}
				}

				if (myParse.ExistNode(myNode[2]))		//3 operands
				{
					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						switch (myNode[1].Type)
						{
						case StreamParser::IsReg:
							reg[1] = myNode[1].Value - base;

							imul(Xbyak::Reg16(reg[0]),Xbyak::Reg16(reg[1]),(long)myNode[2].Value);
							break;
						case StreamParser::IsPtr:
							imul(Xbyak::Reg16(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf, StreamParser::Word),(long)myNode[2].Value);
							break;
						}

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						switch (myNode[1].Type)
						{
						case StreamParser::IsReg:
							reg[1] = myNode[1].Value - base;
							imul(Xbyak::Reg32(reg[0]),Xbyak::Reg32(reg[1]),(long)myNode[2].Value);
							break;
						case StreamParser::IsPtr:
							imul(Xbyak::Reg32(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf, StreamParser::DWord),(long)myNode[2].Value);
							break;
						}

					}

				}
			

			return true;
		}

		return false;
	}
};

class MUL : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0xA)//<----------------------------------------------0xA is Not Correct!
		{
			MUL::size_ = 0;
			
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			if (!myParse.ExistNode(myNode[1]) && !myParse.ExistNode(myNode[2]))		//1 operand
			{
				switch (myNode[0].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						mul(Xbyak::Reg8(reg[0]));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						mul(Xbyak::Reg16(reg[0]));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						mul(Xbyak::Reg32(reg[0]));
					}
					break;

				case StreamParser::IsPtr:
					mul(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size));
					break;
				}
			}



			

			return true;
		}

		return false;
	}
};

class Neg : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0xB)
		{
			Neg::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
				{
					base = vAL;
					reg[0] = myNode[0].Value - base;

					neg(Xbyak::Reg8(reg[0]));

				}

				if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
				{
					base = vAX;
					reg[0] = myNode[0].Value - base;

					neg(Xbyak::Reg16(reg[0]));

				}

				if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
				{
					base = vEAX;
					reg[0] = myNode[0].Value - base;

					neg(Xbyak::Reg32(reg[0]));
				}
				break;

			case StreamParser::IsPtr:
				switch (myNode[0].Size)
				{
				case StreamParser::DWord:
					neg(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size));
					break;
				case StreamParser::Word:
					neg(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size));
					break;
				case StreamParser::Byte:
					neg(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size));
					break;
				}

				break;

			}

			return true;
		}

		return false;
	}
};



class Not : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0xC)
		{
			Not::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
				{
					base = vAL;
					reg[0] = myNode[0].Value - base;

					not(Xbyak::Reg8(reg[0]));

				}

				if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
				{
					base = vAX;
					reg[0] = myNode[0].Value - base;

					not(Xbyak::Reg16(reg[0]));

				}

				if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
				{
					base = vEAX;
					reg[0] = myNode[0].Value - base;

					not(Xbyak::Reg32(reg[0]));
				}
				break;

			case StreamParser::IsPtr:
				switch (myNode[0].Size)
				{
				case StreamParser::DWord:
					not(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size));
					break;
				case StreamParser::Word:
					not(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size));
					break;
				case StreamParser::Byte:
					not(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size));
					break;
				}

				break;

			}

			return true;
		}

		return false;
	}
};

class Movs : VirtualCodeHandler		//USED
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		Movs::size_ = 0;


		if (Stream[0x8]==0xF3)
			db(0xF3);
		if (Stream[0x8]==0xF2)
			db(0xF2);

		if(Index == 0x0D)
		{
			db(0xA4);
			return true;
		}

		if(Index == 0x0E)
		{
			db(0x66);
			db(0xA5);
			return true;
		}

		if(Index == 0x0F)
		{
			db(0xA5);
			return true;
		}


		return false;
	}
};

class Stos : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		Stos::size_ = 0;

		if (Stream[0x8]==0xF3)
			db(0xF3);
		if (Stream[0x8]==0xF2)
			db(0xF2);

		if(Index == 0x10)
		{
			db(0xAA);
			return true;
		}
		

		if(Index == 0x11)
		{
			db(0x66);
			db(0xAB);
			return true;
		}

		if(Index == 0x12)
		{
			db(0xAB);
			return true;
		}

		return false;
	}
};

class Lods : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		Lods::size_ = 0;
		if(Index == 0x13)
		{
			db(0xAC);
			return true;
		}
		if(Index == 0x14)
		{
			db(0x66);
			db(0xAD);
			return true;
		}
		if(Index == 0x15)
		{
			db(0xAD);
			return true;
		}

		return false;
	}
};

class Cmps : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		Cmps::size_ = 0;

		if (Stream[0x8]==0xF3)
			db(0xF3);
		if (Stream[0x8]==0xF2)
			db(0xF2);


		if(Index == 0x16)
		{
			db(0xA6);
			return true;
		}
		if(Index == 0x17)
		{
			db(0x66);
			db(0xA7);
			return true;
		}
		if(Index == 0x18)
		{
			db(0xA7);
			return true;
		}

		return false;
	}
};

class Scas : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		Scas::size_ = 0;


		if (Stream[0x8]==0xF3)
			db(0xF3);
		if (Stream[0x8]==0xF2)
			db(0xF2);

		if(Index == 0x19)
		{
			db(0xAE);
			return true;
		}
		if(Index == 0x1A)
		{
			db(0x66);
			db(0xAF);
			return true;
		}
		if(Index == 0x1B)
		{
			db(0xAF);
			return true;
		}


		return false;
	}
};


class Stc : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x1C)
		{
			Stc::size_ = 0;
			stc();
			return true;
		}

		return false;
	}
};


class Clc : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x1D)
		{
			Clc::size_ = 0;
			clc();
			return true;
		}

		return false;
	}
};


class Std : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x1E)
		{
			Std::size_ = 0;
			std();
			return true;
		}

		return false;
	}
};


class Cld : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x1F)
		{
			Cld::size_ = 0;
			cld();
			return true;
		}

		return false;
	}
};

class Cdq : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x20)
		{
			Cdq::size_ = 0;
			cdq();
			return true;
		}

		return false;
	}
};


class Cmc : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x21)
		{
			Cmc::size_ = 0;
			cmc();
			return true;
		}

		return false;
	}
};

class Rcr : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		int reg[2] = { -1, -1 }; // -1 means no register
		if(Index == 0x22)
		{
			Rcr::size_ = 0;
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						rcr(Xbyak::Reg8(reg[0]),cl);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						rcr(Xbyak::Reg16(reg[0]),cl);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						rcr(Xbyak::Reg32(reg[0]),cl);
					}
					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						rcr(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						rcr(Xbyak::Reg16(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						rcr(Xbyak::Reg32(reg[0]),(char)myNode[1].Value);
					}
					break;
					//IsPtr
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					rcr(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),cl);
					break;
				case StreamParser::IsImm:
					rcr(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
					break;
				}

				break;

			}
			return true;
		}

		return false;
	}
};


class Rcl : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		int reg[2] = { -1, -1 }; // -1 means no register
		if(Index == 0x23)
		{
			Rcl::size_ = 0;
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						rcl(Xbyak::Reg8(reg[0]),cl);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						rcl(Xbyak::Reg16(reg[0]),cl);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						rcl(Xbyak::Reg32(reg[0]),cl);
					}
					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						rcl(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						rcl(Xbyak::Reg16(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						rcl(Xbyak::Reg32(reg[0]),(char)myNode[1].Value);
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					rcl(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),cl);
					break;
				case StreamParser::IsImm:
					rcl(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
					break;
				}

				break;

			}
			return true;
		}

		return false;
	}
};

class Shl : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		int reg[2] = { -1, -1 }; // -1 means no register
		if(Index == 0x24)
		{
			Shl::size_ = 0;
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						shl(Xbyak::Reg8(reg[0]),cl);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						shl(Xbyak::Reg16(reg[0]),cl);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						shl(Xbyak::Reg32(reg[0]),cl);
					}
					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						shl(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						shl(Xbyak::Reg16(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						shl(Xbyak::Reg32(reg[0]),(char)myNode[1].Value);
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					shl(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),cl);
					break;
				case StreamParser::IsImm:
					shl(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
					break;
				}

				break;

			}
			return true;
		}

		return false;
	}
};


class Sar : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		int reg[2] = { -1, -1 }; // -1 means no register
		if(Index == 0x25)
		{
			Sar::size_ = 0;
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						sar(Xbyak::Reg8(reg[0]),cl);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						sar(Xbyak::Reg16(reg[0]),cl);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						sar(Xbyak::Reg32(reg[0]),cl);
					}
					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						sar(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						sar(Xbyak::Reg16(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						sar(Xbyak::Reg32(reg[0]),(char)myNode[1].Value);
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					sar(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),cl);
					break;
				case StreamParser::IsImm:
					sar(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
					break;
				}

				break;

			}
			return true;
		}

		return false;
	}
};


class Rol : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		int reg[2] = { -1, -1 }; // -1 means no register

		if(Index == 0x26)
		{
			Rol::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						rol(Xbyak::Reg8(reg[0]),cl);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						rol(Xbyak::Reg16(reg[0]),cl);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						rol(Xbyak::Reg32(reg[0]),cl);
					}
					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						rol(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						rol(Xbyak::Reg16(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						rol(Xbyak::Reg32(reg[0]),(char)myNode[1].Value);
					}
					break;
		break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					rol(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),cl);
					break;
				case StreamParser::IsImm:
					rol(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
					break;
				}
			}
			return true;
		}

		return false;
	}
};


class Ror : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		int reg[2] = { -1, -1 }; // -1 means no register

		if(Index == 0x27)
		{
			Ror::size_ = 0;
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						ror(Xbyak::Reg8(reg[0]),cl);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						ror(Xbyak::Reg16(reg[0]),cl);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						ror(Xbyak::Reg32(reg[0]),cl);
					}
					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						ror(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						ror(Xbyak::Reg16(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						ror(Xbyak::Reg32(reg[0]),(char)myNode[1].Value);
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					ror(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),cl);
					break;
				case StreamParser::IsImm:
					ror(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
					break;
				}

				break;

			}

			return true;
		}

		return false;
	}
};


class ShlSal : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		int reg[2] = { -1, -1 }; // -1 means no register
		if(Index == 0x28)
		{
			ShlSal::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						shl(Xbyak::Reg8(reg[0]),cl);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						shl(Xbyak::Reg16(reg[0]),cl);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						shl(Xbyak::Reg32(reg[0]),cl);
					}
					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						shl(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						shl(Xbyak::Reg16(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						shl(Xbyak::Reg32(reg[0]),(char)myNode[1].Value);
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					shl(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),cl);
					break;
				case StreamParser::IsImm:
					shl(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
					break;
				}

				break;

			}
			return true;
		}

		return false;
	}
};


class Shr : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		int reg[2] = { -1, -1 }; // -1 means no register

		if(Index == 0x29)
		{
			Shr::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);
			
			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						shr(Xbyak::Reg8(reg[0]),cl);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						shr(Xbyak::Reg16(reg[0]),cl);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						shr(Xbyak::Reg32(reg[0]),cl);
					}
					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						shr(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						shr(Xbyak::Reg16(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						shr(Xbyak::Reg32(reg[0]),(char)myNode[1].Value);
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					shr(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),cl);
					break;
				case StreamParser::IsImm:
					shr(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
					break;
				}

			break;

				}

			return true;
		}

		return false;
	}
};


class Loop : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x2A)
		{
			size_ = 0;

			relocOffset = 3;
			relocKey = *((DWORD*)(Stream + 0x1C));

			dec(ecx);

			db(0x0f);
			db(0x85);
			dd(0);

			return true;
		}

		return false;
	}
};


class Jmp : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		int reg, base;
		if(Index == 0x2B)
		{
			size_ = 0;
			if(Stream[0xC] == 0x90)					//Key jmp
			{
				relocOffset = 1;
				relocKey = *((DWORD*)(Stream + 0x1C));

				db(0xe9);
				dd(0);
				return true;
			}

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				base = vEAX;
				reg = myNode[0].Value - base;

				jmp(Xbyak::Reg32(reg));
				break;
			case StreamParser::IsImm:

				myNode[0].Value += vDllBase;

				relocOffset = 1;
				relocAddr = myNode[0].Value;

				jmp((void*)myNode[0].Value);

				break;
			case StreamParser::IsPtr:
				jmp(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size));

			}

			return true;
		}

		return false;
	}
};


class Je : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		
		if(Index == 0x2C)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));
			
			db(0x0f);
			db(0x84);
			dd(0);
			return true;
		}

		return false;
	}
};


class Jnz : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		
		if(Index == 0x2D)
		{
			size_ = 0;
			
			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x0f);
			db(0x85);
			dd(0);
			return true;
		}

		return false;
	}
};


class Js : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x2E)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));
			
			db(0x0f);
			db(0x88);
			dd(0);
			return true;
		}

		return false;
	}
};


class Jns : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x2F)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x0f);
			db(0x89);
			dd(0);
			return true;
		}

		return false;
	}
};


class Jp : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x30)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x0f);
			db(0x8A);
			dd(0);
			return true;
		}

		return false;
	}
};


class Jnp : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x31)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x0f);
			db(0x8B);
			dd(0);			return true;
		}

		return false;
	}
};


class Jo : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x32)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x0f);
			db(0x80);
			dd(0);			return true;
		}

		return false;
	}
};


class Jno : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x33)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x0f);
			db(0x81);
			dd(0);			return true;
		}

		return false;
	}
};


class Jl : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x34)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x0f);
			db(0x8c);
			dd(0);
			return true;
		}

		return false;
	}
};


class Jge : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x35)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x0f);
			db(0x8D);
			dd(0);			return true;
		}

		return false;
	}
};


class Jle : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x36)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x0f);
			db(0x8E);
			dd(0);			return true;
		}

		return false;
	}
};


class Jg : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x37)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x0f);
			db(0x8F);
			dd(0);
			return true;
		}

		return false;
	}
};


class Jb : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x38)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x0f);
			db(0x82);
			dd(0);
			return true;
		}

		return false;
	}
};


class Jae : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x39)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x0f);
			db(0x83);
			dd(0);			return true;
		}

		return false;
	}
};


class Jbe : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x3A)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x0f);
			db(0x86);
			dd(0);			return true;
		}

		return false;
	}
};


class Ja : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x3B)
		{
			size_ = 0;

			relocOffset = 2;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x0f);
			db(0x87);
			dd(0);			
			return true;
		}

		return false;
	}
};


class Sete : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x3C)
		{
			Sete::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				sete(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				sete(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}
			return true;
		}

		return false;
	}
};


class Setne : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x3D)
		{
			Setne::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				setne(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				setne(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}
			return true;
		}

		return false;
	}
};


class Sets : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x3E)
		{
			Sets::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				sets(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				sets(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}

			return true;
		}

		return false;
	}
};


class Setns : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x3F)
		{
			Setns::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				setns(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				setns(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}
			return true;
		}

		return false;
	}
};


class Setp : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x40)
		{
			Setp::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				setp(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				setp(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}
			return true;
		}

		return false;
	}
};


class Setnp : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x41)
		{
			Setnp::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				setnp(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				setnp(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}

			return true;
		}
		return false;
	}
};


class Seto : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x42)
		{
			Seto::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				seto(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				seto(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}
			return true;
		}
		return false;
	}
};


class Setno : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x43)
		{
			Setno::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				setno(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				setno(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}

			return true;
		}
		return false;
	}
};


class Setl : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
;
		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x44)
		{
			Setl::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				setl(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				setl(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}

			return true;
		}
		return false;
	}
};


class Setge : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x45)
		{
			Setge::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				setge(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				setge(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}

			return true;
		}

		return false;
	}
};


class Setle : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x46)
		{
			Setle::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				setle(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				setle(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}

			return true;
		}

		return false;
	}
};


class Setg : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x47)
		{
			Setg::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				setg(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				setg(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}

			return true;
		}

		return false;
	}
};


class Setb : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x48)
		{
			Setb::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				setb(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				setb(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}

			return true;
		}

		return false;
	}
};


class Setae : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x49)
		{
			Setae::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				setae(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				setae(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}

			return true;
		}
		return false;
	}
};


class Setna : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x4A)
		{
			Setna::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				setna(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				setna(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}

			return true;
		}

		return false;
	}
};


class Seta : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base = vAL;
		int reg[2] = { -1, -1 }; // -1 means no register


		if(Index == 0x4B)
		{
			Seta::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				reg[0] = myNode[0].Value - base;
				seta(Xbyak::Reg8(reg[0]));
				break;
			case StreamParser::IsPtr:
				seta(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, StreamParser::Byte));
				break;
			}
			
			return true;
		}

		return false;
	}
};

class Adc : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0x4C)
		{
			Adc::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						adc(Xbyak::Reg8(reg[0]),Xbyak::Reg8(reg[1]));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						adc(Xbyak::Reg16(reg[0]),Xbyak::Reg16(reg[1]));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						adc(Xbyak::Reg32(reg[0]),Xbyak::Reg32(reg[1]));
					}

					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						adc(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						adc(Xbyak::Reg16(reg[0]),(short)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						adc(Xbyak::Reg32(reg[0]),(long)myNode[1].Value);
					}
					break;
				case StreamParser::IsPtr:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						adc(Xbyak::Reg8(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						adc(Xbyak::Reg16(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						adc(Xbyak::Reg32(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[1].Value >= vAL && myNode[1].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[1].Value - base;

						adc(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg8(reg[0]));

					}

					if (myNode[1].Value >= vAX && myNode[1].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[1].Value - base;

						adc(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf), Xbyak::Reg16(reg[0]));

					}

					if (myNode[1].Value >= vEAX && myNode[1].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[1].Value - base;

						adc(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg32(reg[0]));
					}
					break;
				case StreamParser::IsImm:
					switch (myNode[0].Size)
					{
					case StreamParser::DWord:
						adc(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(long)myNode[1].Value);
						break;
					case StreamParser::Word:
						adc(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(short)myNode[1].Value);
						break;
					case StreamParser::Byte:
						adc(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
						break;
					}

					break;
				}
			}

			return true;
		}

		return false;

	}
};


class Add : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0x4D)
		{
			Add::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						add(Xbyak::Reg8(reg[0]),Xbyak::Reg8(reg[1]));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						add(Xbyak::Reg16(reg[0]),Xbyak::Reg16(reg[1]));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						add(Xbyak::Reg32(reg[0]),Xbyak::Reg32(reg[1]));
					}

					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						add(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						add(Xbyak::Reg16(reg[0]),(short)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						add(Xbyak::Reg32(reg[0]),(long)myNode[1].Value);
					}
					break;
				case StreamParser::IsPtr:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						add(Xbyak::Reg8(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						add(Xbyak::Reg16(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						add(Xbyak::Reg32(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[1].Value >= vAL && myNode[1].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[1].Value - base;

						add(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg8(reg[0]));

					}

					if (myNode[1].Value >= vAX && myNode[1].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[1].Value - base;

						add(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf), Xbyak::Reg16(reg[0]));

					}

					if (myNode[1].Value >= vEAX && myNode[1].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[1].Value - base;

						add(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg32(reg[0]));
					}
					break;
				case StreamParser::IsImm:
					switch (myNode[0].Size)
					{
					case StreamParser::DWord:
						add(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(long)myNode[1].Value);
						break;
					case StreamParser::Word:
						add(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(short)myNode[1].Value);
						break;
					case StreamParser::Byte:
						add(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
						break;
					}

					break;
				}
			}

			return true;
		}

		return false;

	}
};

class Sbb : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};


		if(Index == 0x4E)
		{
			Sbb::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						sbb(Xbyak::Reg8(reg[0]),Xbyak::Reg8(reg[1]));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						sbb(Xbyak::Reg16(reg[0]),Xbyak::Reg16(reg[1]));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						sbb(Xbyak::Reg32(reg[0]),Xbyak::Reg32(reg[1]));
					}

					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						sbb(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						sbb(Xbyak::Reg16(reg[0]),(short)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						sbb(Xbyak::Reg32(reg[0]),(long)myNode[1].Value);
					}
					break;
				case StreamParser::IsPtr:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						sbb(Xbyak::Reg8(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						sbb(Xbyak::Reg16(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						sbb(Xbyak::Reg32(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[1].Value >= vAL && myNode[1].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[1].Value - base;

						sbb(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg8(reg[0]));

					}

					if (myNode[1].Value >= vAX && myNode[1].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[1].Value - base;

						sbb(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf), Xbyak::Reg16(reg[0]));

					}

					if (myNode[1].Value >= vEAX && myNode[1].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[1].Value - base;

						sbb(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg32(reg[0]));
					}
					break;
				case StreamParser::IsImm:
					switch (myNode[0].Size)
					{
					case StreamParser::DWord:
						sbb(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(long)myNode[1].Value);
						break;
					case StreamParser::Word:
						sbb(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(short)myNode[1].Value);
						break;
					case StreamParser::Byte:
						sbb(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
						break;
					}

					break;
				}
			}

			return true;
		}

		return false;
	}
};


class Sub : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0x4F)
		{
			Sub::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						sub(Xbyak::Reg8(reg[0]),Xbyak::Reg8(reg[1]));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						sub(Xbyak::Reg16(reg[0]),Xbyak::Reg16(reg[1]));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						sub(Xbyak::Reg32(reg[0]),Xbyak::Reg32(reg[1]));
					}

					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						sub(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						sub(Xbyak::Reg16(reg[0]),(short)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						sub(Xbyak::Reg32(reg[0]),(long)myNode[1].Value);
					}
					break;
				case StreamParser::IsPtr:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						sub(Xbyak::Reg8(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						sub(Xbyak::Reg16(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						sub(Xbyak::Reg32(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[1].Value >= vAL && myNode[1].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[1].Value - base;

						sub(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg8(reg[0]));

					}

					if (myNode[1].Value >= vAX && myNode[1].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[1].Value - base;

						sub(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf), Xbyak::Reg16(reg[0]));

					}

					if (myNode[1].Value >= vEAX && myNode[1].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[1].Value - base;

						sub(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg32(reg[0]));
					}
					break;
				case StreamParser::IsImm:
					switch (myNode[0].Size)
					{
					case StreamParser::DWord:
						sub(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(long)myNode[1].Value);
						break;
					case StreamParser::Word:
						sub(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(short)myNode[1].Value);
						break;
					case StreamParser::Byte:
						sub(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
						break;
					}

					break;
				}
			}

			return true;
		}

		return false;
	}
};

class Cmp : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0x50)
		{
			Cmp::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						cmp(Xbyak::Reg8(reg[0]),Xbyak::Reg8(reg[1]));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						cmp(Xbyak::Reg16(reg[0]),Xbyak::Reg16(reg[1]));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						cmp(Xbyak::Reg32(reg[0]),Xbyak::Reg32(reg[1]));
					}

					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						cmp(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						cmp(Xbyak::Reg16(reg[0]),(short)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						cmp(Xbyak::Reg32(reg[0]),(long)myNode[1].Value);
					}
					break;
				case StreamParser::IsPtr:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						cmp(Xbyak::Reg8(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						cmp(Xbyak::Reg16(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						cmp(Xbyak::Reg32(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[1].Value >= vAL && myNode[1].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[1].Value - base;

						cmp(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg8(reg[0]));

					}

					if (myNode[1].Value >= vAX && myNode[1].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[1].Value - base;

						cmp(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf), Xbyak::Reg16(reg[0]));

					}

					if (myNode[1].Value >= vEAX && myNode[1].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[1].Value - base;

						cmp(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg32(reg[0]));
					}
					break;
				case StreamParser::IsImm:
					switch (myNode[0].Size)
					{
					case StreamParser::DWord:
						cmp(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(long)myNode[1].Value);
						break;
					case StreamParser::Word:
						cmp(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(short)myNode[1].Value);
						break;
					case StreamParser::Byte:
						cmp(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
						break;
					}

					break;
				}
			}
			return true;
		}

		return false;
	}
};


class Lea : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
	BYTE base;
	BYTE reg[3] = {-1,-1,-1};


		if(Index == 0x51)
		{
			Lea::size_ = 0;
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
			{
				base = vEAX;
				reg[0] = myNode[0].Value - base;

				lea(Xbyak::Reg32(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));
			}

			return true;
		}

		return false;
	}
};

class SetNewSEH : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};
		int arrindex = 0;

		if(Index == 0x52)
		{
			SetNewSEH::size_ = 0;

			if (Stream[9])
			{
				myPtr[0].PtrValue = 0;
				myPtr[1].PtrValue = 0;
				myPtr[2].PtrValue = 0;
				myPtr[3].PtrValue = 0;
				myPtr[4].PtrValue = 0;
				myPtr[5].PtrValue = 0;
				myPtr[6].PtrValue = 0;

				db(0x64);

				if (Stream[0xC] == 0x8C && Stream[0x20] == 0x8D)
				{
					myNode[1].Type = StreamParser::IsPtr;
					myNode[1].NodePtr = myPtr;

					myNode[0].Type = StreamParser::IsReg;
					myNode[0].Value = Stream[0x10];

					if (Stream[0x24]!=0)
					{
						myPtr[arrindex].OpType = StreamParser::IsReg;
						myPtr[arrindex].PtrValue = Stream[0x24];

						arrindex++;
					}
					if (Stream[0x28]!=0)
					{
						myPtr[arrindex].OpType = StreamParser::IsOperator;
						myPtr[arrindex].Operation = StreamParser::Plus;

						arrindex++;

						myPtr[arrindex].OpType = StreamParser::IsReg;
						myPtr[arrindex].PtrValue = Stream[0x28];

						arrindex++;
					}
					if (Stream[0x2F]!=0)
					{
						myPtr[arrindex].OpType = StreamParser::IsOperator;
						myPtr[arrindex].Operation = StreamParser::Mul;

						arrindex++;

						myPtr[arrindex].OpType = StreamParser::IsImm;
						myPtr[arrindex].PtrValue = Stream[0x2F];

						arrindex++;
					}
					//if (*((DWORD*)(Stream+0x30))!=0)
					//{
						myPtr[arrindex].OpType = StreamParser::IsOperator;
						myPtr[arrindex].Operation = StreamParser::Plus;

						arrindex++;

						myPtr[arrindex].OpType = StreamParser::IsImm;
						myPtr[arrindex].PtrValue = *((DWORD*)(Stream+0x30));

						arrindex++;
					//}

					mov(Xbyak::Reg32(myNode[0].Value - vEAX),dword[0] );
				}

				else
				{
					if(Stream[0xC] == 0x8D)
					{
						if (Stream[0x20] == 0x8C)
						{
							/*db(0x89);
							db(0x25);
							dd(0);*/
							//db(0x64);
							myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

							Xbyak::Address s(32,true,myNode[0].Value,true,false);
							
							base = vEAX;
							reg[0] = myNode[1].Value - base;

							if(reg[0]==0)
							{
								db(0xA3);
								dd(myNode[0].Value);
							}
							else
							{
							db(0x89);
							db(0x0D + reg[0]*8 - 8);
							dd(myNode[0].Value);
							}
							//mov(s,Xbyak::Reg32(reg[0]));
						}
					}

				}

				return true;
			}
				myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);
				
				switch (myNode[0].Type)
				{
				case StreamParser::IsReg:
					switch (myNode[1].Type)
					{
					case StreamParser::IsReg:
						if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
						{
							base = vAL;
							reg[0] = myNode[0].Value - base;
							reg[1] = myNode[1].Value - base;

							mov(Xbyak::Reg8(reg[0]),Xbyak::Reg8(reg[1]));

						}

						if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
						{
							base = vAX;
							reg[0] = myNode[0].Value - base;
							reg[1] = myNode[1].Value - base;

							mov(Xbyak::Reg16(reg[0]),Xbyak::Reg16(reg[1]));

						}

						if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
						{
							base = vEAX;
							reg[0] = myNode[0].Value - base;
							reg[1] = myNode[1].Value - base;

							mov(Xbyak::Reg32(reg[0]),Xbyak::Reg32(reg[1]));
						}
						break;
					case StreamParser::IsImm:
						if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
						{
							base = vAL;
							reg[0] = myNode[0].Value - base;


							mov(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

						}

						if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
						{
							base = vAX;
							reg[0] = myNode[0].Value - base;


							mov(Xbyak::Reg16(reg[0]),(short)myNode[1].Value);

						}

						if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
						{
							base = vEAX;
							reg[0] = myNode[0].Value - base;


							mov(Xbyak::Reg32(reg[0]),(long)myNode[1].Value);
						}
						break;
					case StreamParser::IsPtr:
						if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
						{
							base = vAL;
							reg[0] = myNode[0].Value - base;

							mov(Xbyak::Reg8(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

						}

						if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
						{
							base = vAX;
							reg[0] = myNode[0].Value - base;

							mov(Xbyak::Reg16(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

						}

						if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
						{
							base = vEAX;
							reg[0] = myNode[0].Value - base;

							mov(Xbyak::Reg32(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));
						}
						break;
					}
					break;
				case StreamParser::IsPtr:
					switch (myNode[1].Type)
					{
					case StreamParser::IsReg:
						if (myNode[1].Value >= vAL && myNode[1].Value <= vBL)			//8bit
						{
							base = vAL;
							reg[0] = myNode[1].Value - base;

							mov(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg8(reg[0]));

						}

						if (myNode[1].Value >= vAX && myNode[1].Value <= vDI)			//16bit
						{
							base = vAX;
							reg[0] = myNode[1].Value - base;

							mov(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf), Xbyak::Reg16(reg[0]));

						}

						if (myNode[1].Value >= vEAX && myNode[1].Value <= vEDI)			//32bit
						{
							base = vEAX;
							reg[0] = myNode[1].Value - base;

							mov(AssemblePtr(myNode[0].NodePtr,myNode[0].IsRVA, mInf),Xbyak::Reg32(reg[0]));
						}
						break;
					case StreamParser::IsImm:
						switch (myNode[0].Size)
						{
						case StreamParser::DWord:
							mov(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(long)myNode[1].Value);
							break;
						case StreamParser::Word:
							mov(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(short)myNode[1].Value);
							break;
						case StreamParser::Byte:
							mov(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
							break;
						}

						break;

					}
				}
			
			return true;
		}

		return false;
	}
};

class Movzx : VirtualCodeHandler		//USED
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0x53)
		{
			size_ = 0;
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)
			{
				reg[0] = myNode[0].Value - vAX;
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					reg[1] = myNode[1].Value - vAL;

					movzx(Xbyak::Reg16(reg[0]), Xbyak::Reg8(reg[1]));

					break;
				case StreamParser::IsPtr:
					movzx(Xbyak::Reg16(reg[0]), AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf, StreamParser::Byte));
					break;
				}
			}

			if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
			{
				reg[0] = myNode[0].Value - vEAX;
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[1].Value >= vAL && myNode[1].Value <= vBL)
					{
						reg[1] = myNode[1].Value - vAL;
						movzx(Xbyak::Reg32(reg[0]), Xbyak::Reg8(reg[1]));
					}
					if (myNode[1].Value >= vAX && myNode[1].Value <= vDI)
					{
						reg[1] = myNode[1].Value - vAX;
						movzx(Xbyak::Reg32(reg[0]), Xbyak::Reg16(reg[1]));
					}
					break;

				case StreamParser::IsPtr:
					if (myNode[1].Size == StreamParser::Byte)
					{
						movzx(Xbyak::Reg32(reg[0]), AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf, StreamParser::Byte));
					}

					if (myNode[1].Size == StreamParser::Word)
					{
						movzx(Xbyak::Reg32(reg[0]), AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf, StreamParser::Word));
					}
					break;
				}

			}
			return true;
		}

		return false;
	}
};


class Movsx : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};
		if(Index == 0x54)
		{
			size_ = 0;
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)
			{
				reg[0] = myNode[0].Value - vAX;
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					reg[1] = myNode[1].Value - vAL;

					movsx(Xbyak::Reg16(reg[0]), Xbyak::Reg8(reg[1]));

					break;
				case StreamParser::IsPtr:
					movsx(Xbyak::Reg16(reg[0]), AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf, StreamParser::Byte));
					break;
				}
			}

			if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
			{
				reg[0] = myNode[0].Value - vEAX;
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[1].Value >= vAL && myNode[1].Value <= vBL)
					{
						reg[1] = myNode[1].Value - vAL;
						movsx(Xbyak::Reg32(reg[0]), Xbyak::Reg8(reg[1]));
					}
					if (myNode[1].Value >= vAX && myNode[1].Value <= vDI)
					{
						reg[1] = myNode[1].Value - vAX;
						movsx(Xbyak::Reg32(reg[0]), Xbyak::Reg16(reg[1]));
					}
					break;

				case StreamParser::IsPtr:
					if (myNode[1].Size == StreamParser::Byte)
					{
						movsx(Xbyak::Reg32(reg[0]), AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf, StreamParser::Byte));
					}

					if (myNode[1].Size == StreamParser::Word)
					{
						movsx(Xbyak::Reg32(reg[0]), AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf, StreamParser::Word));
					}
					break;
				}

			}
			return true;
		}

		return false;
	}
};


class Xchg : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0x55)
		{
			Xchg::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						xchg(Xbyak::Reg8(reg[0]),Xbyak::Reg8(reg[1]));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						xchg(Xbyak::Reg16(reg[0]),Xbyak::Reg16(reg[1]));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						xchg(Xbyak::Reg32(reg[0]),Xbyak::Reg32(reg[1]));
					}

					break;
				case StreamParser::IsPtr:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						xchg(Xbyak::Reg8(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						xchg(Xbyak::Reg16(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						xchg(Xbyak::Reg32(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[1].Value >= vAL && myNode[1].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[1].Value - base;

						xchg(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg8(reg[0]));

					}

					if (myNode[1].Value >= vAX && myNode[1].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[1].Value - base;

						xchg(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf), Xbyak::Reg16(reg[0]));

					}

					if (myNode[1].Value >= vEAX && myNode[1].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[1].Value - base;

						xchg(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg32(reg[0]));
					}
					break;
				}
			}
			return true;
		}

		return false;
	}
};


class Xor : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0x56)
		{
			Xor::size_ = 0;

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						xor(Xbyak::Reg8(reg[0]),Xbyak::Reg8(reg[1]));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						xor(Xbyak::Reg16(reg[0]),Xbyak::Reg16(reg[1]));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						xor(Xbyak::Reg32(reg[0]),Xbyak::Reg32(reg[1]));
					}

					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						xor(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						xor(Xbyak::Reg16(reg[0]),(short)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						xor(Xbyak::Reg32(reg[0]),(long)myNode[1].Value);
					}
					break;
				case StreamParser::IsPtr:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						xor(Xbyak::Reg8(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						xor(Xbyak::Reg16(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						xor(Xbyak::Reg32(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[1].Value >= vAL && myNode[1].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[1].Value - base;

						xor(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg8(reg[0]));

					}

					if (myNode[1].Value >= vAX && myNode[1].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[1].Value - base;

						xor(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf), Xbyak::Reg16(reg[0]));

					}

					if (myNode[1].Value >= vEAX && myNode[1].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[1].Value - base;

						xor(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg32(reg[0]));
					}
					break;
				case StreamParser::IsImm:
					switch (myNode[0].Size)
					{
					case StreamParser::DWord:
						xor(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(long)myNode[1].Value);
						break;
					case StreamParser::Word:
						xor(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(short)myNode[1].Value);
						break;
					case StreamParser::Byte:
						xor(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
						break;
					}

					break;
				}
			}
			return true;
		}

		return false;
	}
};


class And : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0x57)
		{
			And::size_ = 0;
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						and(Xbyak::Reg8(reg[0]),Xbyak::Reg8(reg[1]));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						and(Xbyak::Reg16(reg[0]),Xbyak::Reg16(reg[1]));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						and(Xbyak::Reg32(reg[0]),Xbyak::Reg32(reg[1]));
					}

					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						and(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						and(Xbyak::Reg16(reg[0]),(short)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						and(Xbyak::Reg32(reg[0]),(long)myNode[1].Value);
					}
					break;
				case StreamParser::IsPtr:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						and(Xbyak::Reg8(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						and(Xbyak::Reg16(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						and(Xbyak::Reg32(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[1].Value >= vAL && myNode[1].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[1].Value - base;

						and(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg8(reg[0]));

					}

					if (myNode[1].Value >= vAX && myNode[1].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[1].Value - base;

						and(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf), Xbyak::Reg16(reg[0]));

					}

					if (myNode[1].Value >= vEAX && myNode[1].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[1].Value - base;

						and(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg32(reg[0]));
					}
					break;
				case StreamParser::IsImm:
					switch (myNode[0].Size)
					{
					case StreamParser::DWord:
						and(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(long)myNode[1].Value);
						break;
					case StreamParser::Word:
						and(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(short)myNode[1].Value);
						break;
					case StreamParser::Byte:
						and(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
						break;
					}
					
					break;
				}
			}

			return true;
		}

		return false;
	}
};

class Test : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0x58)
		{
			Test::size_ = 0;
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						test(Xbyak::Reg8(reg[0]),Xbyak::Reg8(reg[1]));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						test(Xbyak::Reg16(reg[0]),Xbyak::Reg16(reg[1]));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						test(Xbyak::Reg32(reg[0]),Xbyak::Reg32(reg[1]));
					}

					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						test(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;


						test(Xbyak::Reg16(reg[0]),(short)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;


						test(Xbyak::Reg32(reg[0]),(long)myNode[1].Value);
					}
					break;
				}
				break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[1].Value >= vAL && myNode[1].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[1].Value - base;

						test(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg8(reg[0]));

					}

					if (myNode[1].Value >= vAX && myNode[1].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[1].Value - base;

						test(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf), Xbyak::Reg16(reg[0]));

					}

					if (myNode[1].Value >= vEAX && myNode[1].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[1].Value - base;

						test(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg32(reg[0]));
					}
					break;
				case StreamParser::IsImm:
					switch (myNode[0].Size)
					{
					case StreamParser::DWord:
						test(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(long)myNode[1].Value);
						break;
					case StreamParser::Word:
						test(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(short)myNode[1].Value);
						break;
					case StreamParser::Byte:
						test(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
						break;
					}

					break;
				}
			}
			return true;
		}

		return false;
	}
};



class Or : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		BYTE base;
		BYTE reg[3] = {-1,-1,-1};

		if(Index == 0x59)
		{
			Or::size_ = 0;
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						or(Xbyak::Reg8(reg[0]),Xbyak::Reg8(reg[1]));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						or(Xbyak::Reg16(reg[0]),Xbyak::Reg16(reg[1]));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;
						reg[1] = myNode[1].Value - base;

						or(Xbyak::Reg32(reg[0]),Xbyak::Reg32(reg[1]));
					}

					break;
				case StreamParser::IsImm:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;


						or(Xbyak::Reg8(reg[0]),(char)myNode[1].Value);

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;
						

						or(Xbyak::Reg16(reg[0]),(short)myNode[1].Value);

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;
						

						or(Xbyak::Reg32(reg[0]),(long)myNode[1].Value);
					}
					break;
				case StreamParser::IsPtr:
					if (myNode[0].Value >= vAL && myNode[0].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[0].Value - base;

						or(Xbyak::Reg8(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[0].Value - base;

						or(Xbyak::Reg16(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));

					}

					if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[0].Value - base;

						or(Xbyak::Reg32(reg[0]),AssemblePtr(myNode[1].NodePtr, myNode[1].IsRVA, mInf));
					}
				break;
				}
			break;
			case StreamParser::IsPtr:
				switch (myNode[1].Type)
				{
				case StreamParser::IsReg:
					if (myNode[1].Value >= vAL && myNode[1].Value <= vBL)			//8bit
					{
						base = vAL;
						reg[0] = myNode[1].Value - base;

						or(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg8(reg[0]));

					}

					if (myNode[1].Value >= vAX && myNode[1].Value <= vDI)			//16bit
					{
						base = vAX;
						reg[0] = myNode[1].Value - base;

						or(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf), Xbyak::Reg16(reg[0]));

					}

					if (myNode[1].Value >= vEAX && myNode[1].Value <= vEDI)			//32bit
					{
						base = vEAX;
						reg[0] = myNode[1].Value - base;

						or(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf),Xbyak::Reg32(reg[0]));
					}
					break;
				case StreamParser::IsImm:
					switch (myNode[0].Size)
					{
					case StreamParser::DWord:
						or(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(long)myNode[1].Value);
						break;
					case StreamParser::Word:
						or(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(short)myNode[1].Value);
						break;
					case StreamParser::Byte:
						or(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size),(char)myNode[1].Value);
						break;
					}

					break;
				}
			}

			return true;
		}

		return false;
	}
};


class Push : VirtualCodeHandler
{
public:
	BYTE base;
	long imm;

	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		int reg[2] = { -1, -1 }; // -1 means no register
		if (Index == 0x5A)
		{
			Push::size_ = 0;

			if (Stream[0x9])
			{
				db(0x64);			//push dword ptr fs:[0]
				db(0xff);
				db(0x35);
				dd(0);

				return true;
			}

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//push 16bit
				{
					base = vAX;
					reg[0] = myNode[0].Value - base;

					push(Xbyak::Reg16(reg[0]));

				}

				if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//push 32bit
				{
					base = vEAX;
					reg[0] = myNode[0].Value - base;

					push(Xbyak::Reg32(reg[0]));
				}

				break;
			case StreamParser::IsImm:
				push(myNode[0].Value);
				break;
			case StreamParser::IsPtr:
				push(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size));
			
		}

		return true;
	}
		return false;
	}
};

class Pop : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		
		BYTE base;
		DWORD imm;
		int reg[2] = {-1,-1};


		if(Index == 0x5B)
		{
			Pop::size_ = 0;
			if (Stream[0x9])
			{
				db(0x64);			//pop dword ptr fs:[0]
				db(0x8F);
				db(0x05);
				dd(0);

				return true;
			}

			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
				if (myNode[0].Value >= vAX && myNode[0].Value <= vDI)			//push 16bit
				{
					base = vAX;
					reg[0] = myNode[0].Value - base;

					pop(Xbyak::Reg16(reg[0]));

				}

				if (myNode[0].Value >= vEAX && myNode[0].Value <= vEDI)			//push 32bit
				{
					base = vEAX;
					reg[0] = myNode[0].Value - base;

					pop(Xbyak::Reg32(reg[0]));
				}

				break;

			case StreamParser::IsPtr:
				pop(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size));
			}
			return true;
		}

		return false;
	}
};


class Pushf : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x5C)
		{
			Pushf::size_ = 0;
			pushf();
			return true;
		}

		return false;
	}
};


class Popf : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x5D)
		{
			Popf::size_ = 0;
			popf();
			return true;
		}

		return false;
	}
};


class Pushad : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x5E)
		{
			Pushad::size_ = 0;
			pushad();
			return true;
		}

		return false;
	}
};


class Popad : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x5F)
		{
			Popad::size_ = 0;
			popad();
			return true;
		}

		return false;
	}
};


class Call : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		BYTE base;
		BYTE opcode;
		int reg;

		if(Index == 0x60)
		{
			Call::size_ = 0;
			myParse.ParseStream(Stream,(StreamParser::Node *)myNode,(StreamParser::PtrNode *)myPtr, mInf);

			switch (myNode[0].Type)
			{
			case StreamParser::IsReg:
					base = vEAX;
					reg = myNode[0].Value - base;

					call(Xbyak::Reg32(reg));
				break;
			case StreamParser::IsImm:

				myNode[0].Value += mInf.lModVA;

				relocOffset = 1;
				relocAddr = myNode[0].Value;


				call((void*)myNode[0].Value);

				break;
			case StreamParser::IsPtr:
				call(AssemblePtr(myNode[0].NodePtr, myNode[0].IsRVA, mInf, myNode[0].Size));

			}

			return true;
		}

		return false;
	}
};

class Exit : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{
		//DWORD vDllBase = 0x45A000;
		if(Index == 0x61)
		{
			size_ = 0;

			relocOffset = 1;
			relocAddr = *((DWORD*)(Stream + 0x1C));

			relocAddr += mInf.lModVA;

			db(0xe9);
			dd(0);
			return true;
		}

		return false;
	}
};


class Jcxz : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x65)
		{
			size_ = 0;

			relocOffset = 6;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x66);
			db(0x83);
			db(0xF9);
			db(0x00);

			db(0x0f);
			db(0x84);
			dd(0);
			return true;
		}

		return false;
	}
};

class Jecxz : VirtualCodeHandler
{
public:
	bool HandleBytecode(unsigned char *Stream, unsigned int Index, unsigned char *Dest, int &relocOffset, DWORD &relocKey, DWORD &relocAddr, lModInfo mInf)
	{

		if(Index == 0x66)
		{
			size_ = 0;

			relocOffset = 5;
			relocKey = *((DWORD*)(Stream + 0x1C));

			db(0x83);
			db(0xF9);
			db(0x00);

			db(0x0f);
			db(0x84);
			dd(0);
			return true;
		}

		return false;
	}
};