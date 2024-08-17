#include <Windows.h>
#include <iostream>
#include <vector>
#include <inttypes.h>

#include <Zydis/Zydis.h>


using namespace std;


std::vector<ZyanU8> __jmp(ZyanI64& Target)
{
    std::vector<ZyanU8> encoded_instruction;

    // mov rdx, Target
    {
        ZydisEncoderRequest req;
        memset(&req, 0, sizeof(req));

        req.mnemonic = ZYDIS_MNEMONIC_MOV;
        req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
        req.operand_count = 2;
        req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
        req.operands[0].reg.value = ZYDIS_REGISTER_RDX;
        req.operands[1].type = ZYDIS_OPERAND_TYPE_IMMEDIATE;
        req.operands[1].imm.u = Target;

        ZyanU8 encoded_instr[ZYDIS_MAX_INSTRUCTION_LENGTH];
        ZyanUSize encoded_length = sizeof(encoded_instr);

        if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, encoded_instr, &encoded_length)))
            throw std::runtime_error("Failed to encode MOV instruction");

        encoded_instruction.insert(encoded_instruction.end(), encoded_instr, encoded_instr + encoded_length);
    }

    // jmp rdx
    {
        ZydisEncoderRequest req;
        memset(&req, 0, sizeof(req));

        req.mnemonic = ZYDIS_MNEMONIC_JMP;
        req.machine_mode = ZYDIS_MACHINE_MODE_LONG_64;
        req.operand_count = 1;
        req.operands[0].type = ZYDIS_OPERAND_TYPE_REGISTER;
        req.operands[0].reg.value = ZYDIS_REGISTER_RDX;

        ZyanU8 encoded_instr[ZYDIS_MAX_INSTRUCTION_LENGTH];
        ZyanUSize encoded_length = sizeof(encoded_instr);

        if (ZYAN_FAILED(ZydisEncoderEncodeInstruction(&req, encoded_instr, &encoded_length)))
            throw std::runtime_error("Failed to encode JMP instruction");
        
        encoded_instruction.insert(encoded_instruction.end(), encoded_instr, encoded_instr + encoded_length);
    }

    return encoded_instruction;
}


void Disassem(std::vector<ZyanU8> &buffer, ZyanU64&lpAddress)
{
    ZyanUSize offset = 0;
    ZydisDisassembledInstruction instruction;

    while (ZYAN_SUCCESS(ZydisDisassembleIntel(ZYDIS_MACHINE_MODE_LONG_64,lpAddress,buffer.data() + offset,buffer.size() - offset, &instruction)))
    {
        printf("%016" PRIX64 "  %s\n", lpAddress, instruction.text);
        offset += instruction.info.length;
        lpAddress += instruction.info.length;
    }
}

int main()
{
    void* lpAddress = VirtualAlloc(nullptr, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (lpAddress != nullptr)
    {
        try
        {
            std::vector<ZyanU8> r = __jmp(reinterpret_cast<ZyanI64&>(lpAddress));
            
            ZyanU64 runtime_address = (ZyanU64)GetModuleHandleA(0);
            Disassem(r, runtime_address);
        }
        catch (const std::runtime_error &e)
        {
            printf("%s\n", e.what());
        }
    }

    return 0;
}
