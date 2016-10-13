/*
    Software License Agreement (BSD License)
    
    Copyright (c) 1997-2011, David Lindauer, (LADSoft).
    All rights reserved.
    
    Redistribution and use of this software in source and binary forms, 
    with or without modification, are permitted provided that the following 
    conditions are met:
    
    * Redistributions of source code must retain the above
      copyright notice, this list of conditions and the
      following disclaimer.
    
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the
      following disclaimer in the documentation and/or other
      materials provided with the distribution.
    
    * Neither the name of LADSoft nor the names of its
      contributors may be used to endorse or promote products
      derived from this software without specific prior
      written permission of LADSoft.
    
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
    OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
    OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    contact information:
        email: TouchStone222@runbox.com <David Lindauer>
*/
#include "DotNetPELib.h"
#include "PEFile.h"
namespace DotNetPELib
{
    bool Value::ILSrcDump(PELib &peLib)
    {
        // used for types
        type->ILSrcDump(peLib);
        return true;
    }
    size_t Value::Render(PELib &peLib, int opcode, int operandType, unsigned char *result)
    {
        return type->Render(peLib, result);
    }
    bool Local::ILSrcDump(PELib &peLib)
    {
        peLib.Out() << "'" << name << "/" << index << "'";
        return true;
    }
    size_t Local::Render(PELib &peLib, int opcode, int operandType, unsigned char *result)
    {
        int sz = 0;
        if (operandType == Instruction::o_index1)
        {
            *(unsigned char *)result = index;
            sz = 1;
        }
        else if (operandType == Instruction::o_index2)
        {
            *(unsigned short *)result = index;
            sz = 2;
        }
        return sz;
    }
    bool Param::ILSrcDump(PELib &peLib)
    {
        peLib.Out() << "'" << name << "'";
        return true;
    }
    size_t Param::Render(PELib &peLib, int opcode, int operandType, unsigned char *result)
    {
        int sz = 0;
        if (operandType == Instruction::o_index1)
        {
            *(unsigned char *)result = index;
            sz = 1;
        }
        else if (operandType == Instruction::o_index2)
        {
            *(unsigned short *)result = index;
            sz = 2;
        }
        return sz;
    }
    bool FieldName::ILSrcDump(PELib &peLib)
    {
        if (field->GetType()->GetBasicType() == Type::cls)
            if (field->GetFlags().flags & (Qualifiers::ValueType | Qualifiers::Value))
                peLib.Out() << "valuetype ";
            else
                peLib.Out() << "class ";
        field->GetType()->ILSrcDump(peLib);
        peLib.Out() << " ";
        if (field->GetFullName().size())
            peLib.Out() << field->GetFullName();
        else
            peLib.Out() << Qualifiers::GetName(field->GetName(), field->GetContainer());
        return true;
    }
    size_t FieldName::Render(PELib &peLib, int opcode, int operandType, unsigned char *result)
    {
        *(unsigned char *)result = field->GetPEIndex() | (tField << 24);
        return 4;
    }
    MethodName::MethodName(MethodSignature *M, std::string Name, std::string Path) : signature(M), Value("", NULL) 
    {
        if (Name.size())
            if (Path.size())
                signature->SetFullName(Path + "::'" + Name + "'");
            else
                signature->SetFullName(std::string("'") + Name + "'");
    }
    bool MethodName::ILSrcDump(PELib &peLib)
    {
        signature->ILSrcDump(peLib, false, false, false);
        return true;
    }
    size_t MethodName::Render(PELib &peLib, int opcode, int operandType, unsigned char *result)
    {
        if (opcode == Instruction:: i_calli)
        {
            if (signature->GetPEIndexType() == 0)
               signature->PEDump(peLib, true);
            *(unsigned char *)result = signature->GetPEIndexType() | (tStandaloneSig << 24);
        }
        else
        {
            if (signature->GetPEIndex() == 0 && signature->GetPEIndexCallSite() == 0)
               signature->PEDump(peLib, false);
            if (signature->GetPEIndex())
                *(unsigned char *)result = signature->GetPEIndex() | (tMethodDef << 24);
            else    
                *(unsigned char *)result = signature->GetPEIndexCallSite() | (tMemberRef << 24);
        }
        return 4;
    }
}