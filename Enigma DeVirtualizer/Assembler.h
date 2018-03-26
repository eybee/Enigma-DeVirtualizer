#include "VCodeHandler.h"
#include "xbyak/xbyak.h"

class Assembler : public Xbyak::CodeGenerator
{
public:
	bool AssembleNode(StreamParser::Node* myNode,BYTE* Dest);
protected:
private:

};