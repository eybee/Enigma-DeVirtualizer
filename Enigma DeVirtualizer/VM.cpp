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


#include "VM.h"

VM::VM(){
	handler.push_back((VirtualCodeHandler *)new Nop());
	handler.push_back((VirtualCodeHandler *)new Idiv());
	handler.push_back((VirtualCodeHandler *)new Div());
	handler.push_back((VirtualCodeHandler *)new Imul());
	handler.push_back((VirtualCodeHandler *)new Neg());
	handler.push_back((VirtualCodeHandler *)new Not());
	handler.push_back((VirtualCodeHandler *)new Movs());
	handler.push_back((VirtualCodeHandler *)new Stos());
	handler.push_back((VirtualCodeHandler *)new Lods());
	handler.push_back((VirtualCodeHandler *)new Cmps());
	handler.push_back((VirtualCodeHandler *)new Scas());
	handler.push_back((VirtualCodeHandler *)new Stc());
	handler.push_back((VirtualCodeHandler *)new Clc());
	handler.push_back((VirtualCodeHandler *)new Std());
	handler.push_back((VirtualCodeHandler *)new Cld());
	handler.push_back((VirtualCodeHandler *)new Cdq());
	handler.push_back((VirtualCodeHandler *)new Cmc());
	handler.push_back((VirtualCodeHandler *)new Rcr());
	handler.push_back((VirtualCodeHandler *)new Rcl());
	handler.push_back((VirtualCodeHandler *)new Shl());
	handler.push_back((VirtualCodeHandler *)new Sar());
	handler.push_back((VirtualCodeHandler *)new Rol());
	handler.push_back((VirtualCodeHandler *)new Ror());
	handler.push_back((VirtualCodeHandler *)new ShlSal());
	handler.push_back((VirtualCodeHandler *)new Shr());
	handler.push_back((VirtualCodeHandler *)new Loop());
	handler.push_back((VirtualCodeHandler *)new Jmp());
	handler.push_back((VirtualCodeHandler *)new Je());
	handler.push_back((VirtualCodeHandler *)new Jnz());
	handler.push_back((VirtualCodeHandler *)new Js());
	handler.push_back((VirtualCodeHandler *)new Jns());
	handler.push_back((VirtualCodeHandler *)new Jp());
	handler.push_back((VirtualCodeHandler *)new Jnp());
	handler.push_back((VirtualCodeHandler *)new Jo());
	handler.push_back((VirtualCodeHandler *)new Jno());
	handler.push_back((VirtualCodeHandler *)new Jl());
	handler.push_back((VirtualCodeHandler *)new Jge());
	handler.push_back((VirtualCodeHandler *)new Jle());
	handler.push_back((VirtualCodeHandler *)new Jg());
	handler.push_back((VirtualCodeHandler *)new Jb());
	handler.push_back((VirtualCodeHandler *)new Jae());
	handler.push_back((VirtualCodeHandler *)new Jbe());
	handler.push_back((VirtualCodeHandler *)new Ja());
	handler.push_back((VirtualCodeHandler *)new Sete());
	handler.push_back((VirtualCodeHandler *)new Setne());
	handler.push_back((VirtualCodeHandler *)new Sets());
	handler.push_back((VirtualCodeHandler *)new Setns());
	handler.push_back((VirtualCodeHandler *)new Setp());
	handler.push_back((VirtualCodeHandler *)new Setnp());
	handler.push_back((VirtualCodeHandler *)new Seto());
	handler.push_back((VirtualCodeHandler *)new Setno());
	handler.push_back((VirtualCodeHandler *)new Setl());
	handler.push_back((VirtualCodeHandler *)new Setge());
	handler.push_back((VirtualCodeHandler *)new Setle());
	handler.push_back((VirtualCodeHandler *)new Setg());
	handler.push_back((VirtualCodeHandler *)new Setb());
	handler.push_back((VirtualCodeHandler *)new Setae());
	handler.push_back((VirtualCodeHandler *)new Setna());
	handler.push_back((VirtualCodeHandler *)new Seta());
	handler.push_back((VirtualCodeHandler *)new Adc());
	handler.push_back((VirtualCodeHandler *)new Add());
	handler.push_back((VirtualCodeHandler *)new Sbb());
	handler.push_back((VirtualCodeHandler *)new Sub());
	handler.push_back((VirtualCodeHandler *)new Cmp());
	handler.push_back((VirtualCodeHandler *)new Lea());
	handler.push_back((VirtualCodeHandler *)new SetNewSEH());
	handler.push_back((VirtualCodeHandler *)new Movzx());
	handler.push_back((VirtualCodeHandler *)new Movsx());
	handler.push_back((VirtualCodeHandler *)new Xchg());
	handler.push_back((VirtualCodeHandler *)new Xor());
	handler.push_back((VirtualCodeHandler *)new And());
	handler.push_back((VirtualCodeHandler *)new Test());
	handler.push_back((VirtualCodeHandler *)new Or());
	handler.push_back((VirtualCodeHandler *)new Push());
	handler.push_back((VirtualCodeHandler *)new Pop());
	handler.push_back((VirtualCodeHandler *)new Pushf());
	handler.push_back((VirtualCodeHandler *)new Popf());
	handler.push_back((VirtualCodeHandler *)new Pushad());
	handler.push_back((VirtualCodeHandler *)new Popad());
	handler.push_back((VirtualCodeHandler *)new Call());
	handler.push_back((VirtualCodeHandler *)new Exit());
	handler.push_back((VirtualCodeHandler *)new Jcxz());
	handler.push_back((VirtualCodeHandler *)new Jecxz());
	handler.push_back((VirtualCodeHandler *)new MUL());
}



DWORD VM::DevirtualizeExternal(DWORD IntDest)
{
	lModInfo keyJumpSec;
	lModInfo vDll;
	lModInfo cSec;
	MEMORY_BASIC_INFORMATION ByteCodeRegion;
	DWORD initPattern;
	DWORD areaStart;
	DWORD byteCode;
	std::map<DWORD,DWORD> RelocMapKey;
	std::map<DWORD,DWORD> RelocMapAddr;
	std::map<int, DWORD> KeyMap;
	Relocater reloc;
	MEMORY_BASIC_INFORMATION mbInf;

	cSec.lModVA = (DWORD)GetModuleHandleA(0);
	VirtualQuery((LPCVOID)cSec.lModVA, &mbInf, sizeof(MEMORY_BASIC_INFORMATION));
	cSec.lModSize = mbInf.RegionSize;

	keyJumpSec = GetSectionBase(-1);
	vDll = GetSectionBase(-2);
	FindVM(vDll);

	pBYTECODE=FindByteCode(GetVMAddrFromDispatcher(dispatcher[0]));

	if (pBYTECODE==0)
		return false;

	if (dispatcher[0]< 1000)
		return false;


	VirtualQuery((LPCVOID)pBYTECODE, &ByteCodeRegion, sizeof(MEMORY_BASIC_INFORMATION));
	//------------Handle Decompiled Code Dest------
	if (IntDest==0)
	{
		pDecompiledCode=pBYTECODE;
	
	}
	else if (IntDest==1)
	{
		pDecompiledCode=0;
		while (pDecompiledCode<(DWORD)mbInf.BaseAddress)
			pDecompiledCode=(DWORD)VirtualAlloc(0,ByteCodeRegion.RegionSize,0x3000,0x40);
	}
	else
	{
		pDecompiledCode=IntDest;
	}
	//-------------------

	std::map<DWORD,refs> entries = LogVParts(pBYTECODE,keyJumpSec,0);		//false: External
	ConvertByteCode(entries, RelocMapKey, RelocMapAddr, KeyMap, cSec);

	//---------
	
	
		//val2 = ConvertFileOffsetToVA(FileMapVA, val, true);

	//--------------------------
	reloc.FixEntries(entries, RelocMapAddr, KeyMap, modInfo.lModVA);
	reloc.ConvertKeyRefs(RelocMapKey, KeyMap);
	reloc.Relocate(RelocMapKey, RelocMapAddr, keyJumpSec.lModVA);

	return true;
}

bool VM::DevirtualizeInternal(void)
{
	DWORD byteCode;
	std::map<DWORD,DWORD> RelocMapKey;
	std::map<DWORD,DWORD> RelocMapAddr;
	std::map<int, DWORD> KeyMap;
	Relocater reloc;


	modInfo = GetSectionBase(-2);
	if (!FindVM(modInfo)) return false;
	DWORD dispatcherr = FindDispatcher(dispatcher[1]);
	int NumberOfHandler = DetectVMVersion(GetVMAddrFromDispatcher(dispatcher[1]));


	if (NumberOfHandler==0)
		return false;
	byteCode = FindByteCode(dispatcherr);
	pBYTECODE=byteCode;
	//FindXORValue(dispatcherr);
	pDecompiledCode=pBYTECODE;

	std::map<DWORD,refs> entries = LogVParts(byteCode,modInfo, 1);		//true: Internal
	
	int esize = entries.size();
	
	ConvertByteCode(entries, RelocMapKey, RelocMapAddr, KeyMap, modInfo);
	reloc.FixEntries(entries, RelocMapAddr, KeyMap, modInfo.lModVA);
	reloc.ConvertKeyRefs(RelocMapKey, KeyMap);
	reloc.Relocate(RelocMapKey, RelocMapAddr, modInfo.lModVA);

	return true;
}

BOOL VM::FindVM(lModInfo modInfo){
	BYTE dispatch[] = {0x60, 0x9C, 0xB2, 0x01, 0xBE, 0x00, 0x00, 0x00, 0x00, 0x8D, 0xBE, 0xcc, 0xcc, 0xcc, 0xcc, 0x8D, 0x3F, 0xB9, 0x01, 0x00, 0x00, 0x00, 0x31, 0xC0, 0xF0, 0x0F, 0xB1, 0x0F, 0x74, 0x0F};
	BYTE wildcard = 0xcc;

	dispatcher[0] = Find((LPVOID)(modInfo.lModVA),modInfo.lModSize,&dispatch,sizeof(dispatch),&wildcard);
	dispatcher[1] = Find((LPVOID)(dispatcher[0] + 1),modInfo.lModSize,&dispatch,sizeof(dispatch),&wildcard);

	if (!dispatcher[0] || !dispatcher[1]) return false;
	else return true; 

}

DWORD VM::FindDispatcher(DWORD vm){
	BYTE dispatcherPattern[] = {0x8B, 0x67, 0x04, 0x57, 0x8D, 0x4F, 0x10, 0x8B, 0x49, 0x10, 0xFF, 0x71, 0xFC, 0xE8};

	return GetJumpDestination(GetCurrentProcess(), Find((LPVOID)vm, 0x100, &dispatcherPattern, sizeof(dispatcherPattern), 0)+ 13);
}

/*DWORD FindByteCode2(lModInfo modInfo, int numberOfHandler)
{
	BYTE jmppush[] = {0x68, 0xcc, 0xcc, 0xcc, 0xcc, 0xe9};
	BYTE jmpwildcard = 0xcc;
	DWORD jmppushptr;
	DWORD lastgood;
	DWORD bytecode;
	long bytecodecalc;
	
	jmppushptr = modInfo.lModVA;

	do{
		jmppushptr = Find((LPVOID)(jmppushptr),modInfo.lModSize,&jmppush,sizeof(jmppush),&jmpwildcard);
		jmppushptr += 5;
		DWORD jmpdest = GetJumpDestination(GetCurrentProcess(),jmppushptr);
		if(jmpdest == dispatcher[1]){
			lastgood = jmppushptr;
		}
	}
	while(jmppushptr != 5);

	bytecode = lastgood + 5;
	bytecodecalc = *(unsigned long*)bytecode;
	bytecodecalc = bytecodecalc * 8 + 4;
	bytecode = bytecode + bytecodecalc;

	if (numberOfHandler > 0x61)			// Version 2.13 or higher
	{
		DWORD numberOfPointerToSkip = *(DWORD*)bytecode;
		bytecode += numberOfPointerToSkip * 4 + 4;
	}
	
	return bytecode;
}*/
DWORD VM::FindByteCode(DWORD dispatcher)
{

	BYTE findPattern[] = {0xA1,0xCC,0xCC,0xCC,0xCC,0x8B,0x04,0xF0,0x83,0xC0,0xFD,0x83,0xF8};
	BYTE wildcard = 0xcc;
	DWORD VMCall = Find((LPVOID)dispatcher,0x200,&findPattern,sizeof(findPattern),&wildcard ) + 1;
	
	if (VMCall<1000) return NULL; 

	DWORD **byteCode=(DWORD**)VMCall;

	

	return **byteCode;



}
std::map<DWORD, refs> VM::LogVParts(DWORD bytecode, lModInfo modInfo, int mode)		//mode = false: External, mode = true: Internal
{
	DWORD *tPTR;
	BYTE jmppush[] = {0x68, 0xcc, 0xcc, 0xcc, 0xcc, 0xe9};
	BYTE jmpwildcard = 0xcc;
	DWORD jmppushptr;
	int partcount = 0;
	DWORD pushval;
	DWORD pushvalptr;
	std::map<DWORD,refs> log_list;
	refs references;
	FindXORValue( dispatcher[mode]);
	jmppushptr = modInfo.lModVA - 1;

	do{
		jmppushptr = Find((LPVOID)(jmppushptr+1),modInfo.lModSize,&jmppush,sizeof(jmppush),&jmpwildcard);
		DWORD jmpdest = GetJumpDestination(GetCurrentProcess(),jmppushptr+5);
		if(jmpdest == dispatcher[mode])
		{
			pushvalptr = jmppushptr + 1;
			tPTR=(DWORD*)pushvalptr;
			*tPTR^=pushXOR;
			pushval = *(DWORD*)pushvalptr;
			//pushval^=pushXOR;
			references.addr =  CalcByteCodeVAfromPushVal(bytecode,pushval);
			references.key = pushval;
			log_list[jmppushptr] =  references;
		}
	}
	while(jmppushptr != 0);

	return log_list;
}

DWORD VM::CalcByteCodeVAfromPushVal(DWORD bytecode, DWORD valu){
	DWORD calcval;
	//valu^=pushXOR;
	calcval = valu * 9;
	calcval = bytecode + calcval * 8;
	return calcval;
}

void VM::ConvertByteCode(std::map<DWORD,struct refs> entrylist, std::map<DWORD,DWORD> &RelocMapKey, std::map<DWORD, DWORD> &RelocMapAddr, std::map<int, DWORD> &KeyMap, lModInfo mInf){
	Relocater reloc;
	DWORD bytecode;
	DWORD entry;
	DWORD assemblePtr;
	BYTE bla[0x10];
	BYTE *destbuff = bla;
	size_t cSize;
	unsigned int currentindex = 0;
	int key = 0;
	int typeCount = 0;
	bool CodeHandled=false;
	int relocOffset = 0;
	DWORD relocKey = 0;
	DWORD relocAddr = 0;
	//entrylist.begin()->second.addr=pBYTECODE;
	//assemblePtr = bytecode = entrylist.begin()->second.addr;
	 bytecode = pBYTECODE;
	// assemblePtr=pBYTECODE;
	 assemblePtr=pDecompiledCode;

		entry = entrylist.begin()->first;

		currentindex = *((unsigned int*)bytecode);
		
		while(currentindex >= 3 && currentindex <= 0x66)
		{
			CodeHandled=false;
			for(std::vector<VirtualCodeHandler*>::const_iterator n = handler.begin(); n != handler.end(); n++)
			{
				if((*n)->HandleBytecode((unsigned char *)bytecode, currentindex, (unsigned char *)destbuff, relocOffset, relocKey, relocAddr, mInf))
				{
					destbuff = (BYTE*)(*n)->getCode();
					cSize = (*n)->getSize();
					//-------Force Fix IMUL R16,32
					if (currentindex==0x9)
					{
						if ((destbuff[0]==0x66) && (destbuff[1]==0xF7) && (destbuff[2]>=0xE8) && (destbuff[2]<=0xEF))
							cSize=3;
						if ((destbuff[0]==0xF7) && (destbuff[1]>=0xE8) && (destbuff[1]<=0xEF) )
							cSize=2;

					}


					//------------------------------

					KeyMap[key] = assemblePtr;
					key++;
					CodeHandled=true;
					memcpy((void *)assemblePtr, destbuff, cSize);

					if (relocOffset && relocKey) RelocMapKey[assemblePtr + relocOffset] = (DWORD)relocKey;
					if (relocOffset && relocAddr) RelocMapAddr[assemblePtr + relocOffset] = (DWORD)relocAddr;

					relocOffset = 0;
					relocKey = 0;
					relocAddr = 0;

					assemblePtr += cSize;

					break;
				}
			}
			
			if (CodeHandled==false)
			{
				key++;
			}
			bytecode += 0x48;				// instruct size
			currentindex = *((unsigned int*)bytecode);
		}

		
}

lModInfo VM::GetSectionBase(int secNum){
	HMODULE imagebase;
	WORD nrOfSec;
	HMODULE base;
	lModInfo modInfo;

	imagebase = GetModuleHandleA(0);
	
	
	
	if (secNum < 0)
	{
		nrOfSec = GetPE32DataFromMappedFile((ULONG_PTR)imagebase, NULL, UE_SECTIONNUMBER);
		secNum = nrOfSec + secNum;
	}

	modInfo.lModVA = (DWORD)GetPE32DataFromMappedFile((ULONG_PTR)imagebase, secNum, UE_SECTIONVIRTUALOFFSET);
	modInfo.lModVA += (DWORD)imagebase;
	modInfo.lModSize = (DWORD)GetPE32DataFromMappedFile((ULONG_PTR)imagebase, secNum , UE_SECTIONVIRTUALSIZE);
	return modInfo;
}

DWORD VM::GetVMAddrFromDispatcher(DWORD dispatcher)
{
	BYTE findPattern[] = {0x8D, 0x4F, 0x10, 0x8B, 0x49, 0x10, 0xFF, 0x71, 0xFC};

	DWORD VMCall = Find((LPVOID)dispatcher,0x200,&findPattern,sizeof(findPattern),0) + sizeof(findPattern);
	DWORD VMAddr = GetJumpDestination(GetCurrentProcess(), VMCall);

	return VMAddr;
}

int VM::DetectVMVersion(DWORD vMAddr)
{
	BYTE findPattern[] = {0x83, 0xC0, 0xFD};

	DWORD HandlerPtr = Find((LPVOID)vMAddr,0x100,&findPattern,sizeof(findPattern),0) + sizeof(findPattern) + 2;
	if (HandlerPtr<10)
		return 0;
	BYTE NumberOfHandler = *(BYTE*)HandlerPtr;

	return (int)NumberOfHandler + 3;
}


DWORD VM::FindXORValue(DWORD dispatcher)
{

	BYTE findPattern[] = {0x8B,0x5D,0x8,0x33,0x1D,0xCC,0xCC,0xCC,0xCC,0x33,0xD2}; //Internal Pattern
	BYTE findPattern2[] = {0xA1,0xCC,0xCC,0xCC,0xCC,0x8B,0x0,0x8B,0x98,0xF0,0x4,0x0,0x0,0x33,0x5D,0x8}; //External Pattern
	DWORD **iXOR;
	BYTE wildcard = 0xcc;
	DWORD VMCall = Find((LPVOID)GetVMAddrFromDispatcher(dispatcher),0x200,&findPattern,sizeof(findPattern),&wildcard ) + 5;
	if (VMCall!=5)
	{
	 iXOR=(DWORD**)VMCall;

	pushXOR=**iXOR;

	return **iXOR;

	}
	else
	{
		int tmp;
		VMCall = Find((LPVOID)GetVMAddrFromDispatcher(dispatcher),0x200,&findPattern2,sizeof(findPattern2),&wildcard ) + 1;
		if (VMCall!=1)
		{
			__asm
			{
				pushad
					mov eax,VMCall
					mov eax,[eax]
					mov eax,[eax]
					mov eax,[eax]
					mov eax,[eax+0x4F0]
					mov tmp,eax
				popad

			}
			pushXOR=tmp;
			return tmp;
		}
		else
		{
			pushXOR=0;
			return 0;

		}

	}

}