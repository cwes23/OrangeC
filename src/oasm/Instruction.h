/* Software License Agreement
 * 
 *     Copyright(C) 1994-2019 David Lindauer, (LADSoft)
 * 
 *     This file is part of the Orange C Compiler package.
 * 
 *     The Orange C Compiler package is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     The Orange C Compiler package is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU General Public License
 *     along with Orange C.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *     contact information:
 *         email: TouchStone222@runbox.com <David Lindauer>
 * 
 */

#ifndef Instruction_h
#define Instruction_h

#include "Label.h"
#include "AsmExpr.h"
#include <vector>
#include <string>

class Fixup;
class AsmFile;

typedef std::vector<Fixup*> FixupContainer;
class Instruction
{
  public:
    enum iType
    {
        LABEL,
        DATA,
        CODE,
        ALIGN,
        RESERVE,
        ALT
    };
    Instruction(Label* lbl) : data(nullptr), label(lbl), type(LABEL), altdata(nullptr), pos(0), fpos(0), size(0), offs(0), repeat(1), fill(0)
    {
    }
    Instruction(void* Data, int Size, bool isData = false) :
        type(isData ? DATA : CODE),
        label(nullptr),
        pos(0),
        fpos(0),
        repeat(1),
        size(Size),
        offs(0),
        lost(false),
        fill(0)
    {
        data = LoadData(!isData, (unsigned char*)Data, Size);
    }
    Instruction(int aln) :
        data(nullptr),
        type(ALIGN),
        label(nullptr),
        altdata(nullptr),
        fill(0),
        pos(0),
        fpos(0),
        size(aln),
        offs(0),
        repeat(1)
    {
    }
    Instruction(int Repeat, int Size) :
        type(RESERVE),
        label(nullptr),
        altdata(nullptr),
        fill(0),
        pos(0),
        fpos(0),
        size(Size),
        repeat(Repeat),
        offs(0)
    {
        data = new unsigned char[size];
        memset(data, 0, size);
    }
    Instruction(void* data) : data(nullptr), type(ALT), label(nullptr), altdata(data), pos(0), fpos(0), size(0), offs(0), repeat(1), fill(0)
    {
    }
    virtual ~Instruction();

    void RepRemoveCancellations(AsmExprNode *exp, bool commit, int &count, Section *sect[], bool sign[], bool plus);
    void* GetAltData() const { return altdata; }
    Label* GetLabel() const { return label; }
    bool IsLabel() { return type == LABEL; }
    int GetType() const { return type; }
    void Optimize(Section* sect, int pc, bool doErrors);
    void SetOffset(int Offs) { offs = Offs; }
    int GetOffset() const { return offs; }
    void SetFill(int fv) { fill = fv; }
    int GetFill() const { return fill; }
    int const GetSize()
    {
        if (type == ALIGN)
        {
            int n = size - offs % size;
            if (n == size)
                return 0;
            return n;
        }
        else if (type == RESERVE)
            return size * repeat;
        return size;
    }
    unsigned char fill;
    unsigned char* GetData() const { return data; }
    int GetRepeat() const { return repeat; }
    int GetNext(Fixup& fixup, unsigned char* buf);
    void Rewind() { pos = fpos = 0; }
    static bool ParseSectionAttrib(AsmFile* file);
    void Add(Fixup* fixup) { fixups.push_back(fixup); }
    unsigned char* GetBytes() const { return data; }
    FixupContainer* GetFixups() { return &fixups; }
    static void SetBigEndian(bool be) { bigEndian = be; }
    unsigned char* LoadData(bool isCode, unsigned char* data, size_t size);
    bool Lost() const { return lost; }

  private:
    enum iType type;
    unsigned char* data;
    Label* label;
    int size;
    int offs;
    int pos;
    int fpos;
    int repeat, xrepeat;
    void* altdata;
    bool lost;

    FixupContainer fixups;
    static bool bigEndian;
};
#endif