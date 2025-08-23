#include "z80/z80.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"

uint8 memory[64 * 1024];
uint8 port[64 * 1024];

uint8 mapper_read(uint16 addr) {
    return memory[addr];
}

void mapper_write(uint16 addr, uint8 data) {
    memory[addr] = data;
}

uint8 port_read(uint16 addr) {
    addr %= 256;
    return port[addr];
}

void port_write(uint16 addr, uint8 data) {
    addr %= 256;
    port[addr] = data;
}

bool run_tests(z80& cpu, std::string path) {
    std::fstream file(path);
    nlohmann::json json = nlohmann::json::parse(file);
    file.close();

    // Run each test in the json file
    for(auto& test : json) {
        cpu.programCounter      = test["initial"]["pc"];
        cpu.stackPointer        = test["initial"]["sp"];
        cpu.memoryRefresh       = test["initial"]["r"];
        cpu.interruptVector     = test["initial"]["i"];
        cpu.reg[z80::A]         = test["initial"]["a"];
        cpu.reg[z80::B]         = test["initial"]["b"];
        cpu.reg[z80::C]         = test["initial"]["c"];
        cpu.reg[z80::D]         = test["initial"]["d"];
        cpu.reg[z80::E]         = test["initial"]["e"];
        cpu.reg[z80::F]         = test["initial"]["f"];
        cpu.reg[z80::H]         = test["initial"]["h"];
        cpu.reg[z80::L]         = test["initial"]["l"];
        cpu.indexRegisterX      = test["initial"]["ix"];
        cpu.indexRegisterY      = test["initial"]["iy"];
        cpu.interruptMode       = test["initial"]["im"];
        cpu.IFF1                = test["initial"]["iff1"];
        cpu.IFF2                = test["initial"]["iff2"];
        uint16 af_              = test["initial"]["af_"];
        uint16 bc_              = test["initial"]["bc_"];
        uint16 de_              = test["initial"]["de_"];
        uint16 hl_              = test["initial"]["hl_"];
        cpu.reg[z80::A_p]       = af_ >> 8;
        cpu.reg[z80::F_p]       = af_;
        cpu.reg[z80::B_p]       = bc_ >> 8;
        cpu.reg[z80::C_p]       = bc_;
        cpu.reg[z80::D_p]       = de_ >> 8;
        cpu.reg[z80::E_p]       = de_;
        cpu.reg[z80::H_p]       = hl_ >> 8;
        cpu.reg[z80::L_p]       = hl_;
    
        std::memset(memory, 0, 64*1024);
        for(auto& ramEntry : test["initial"]["ram"]) {
            uint16 addr = (int)ramEntry[0];
            uint8 data = (int)ramEntry[1];
            mapper_write(addr, data);
        }
    
        std::memset(port, 0, 64*1024);
        for(auto& portEntry : test["initial"]["ports"]) {
            uint16 addr = (int)portEntry[0];
            uint8 data = (int)portEntry[1];
            port_write(addr, data);
        }

        if(!cpu.cycle())
            continue; // Unimplemented instruction or illegal
    
        bool ok = true;
    
        if(cpu.programCounter != test["final"]["pc"]) {
            std::clog << "\t- Mismatch programCounter RESULT: " << (int)cpu.programCounter << ", ACTUAL: " << (int)test["final"]["pc"] << "\n";
            ok = false;
        }
    
        if(cpu.stackPointer != test["final"]["sp"]) {
            std::clog << "\t- Mismatch stackPointer RESULT: " << (int)cpu.stackPointer << ", ACTUAL: " << (int)test["final"]["sp"] << "\n";
            ok = false;
        }
    
        if(cpu.reg[z80::A] != test["final"]["a"]) {
            std::clog << "\t- Mismatch reg[A] RESULT: " << (int)cpu.reg[z80::A] << ", ACTUAL: " << (int)test["final"]["a"] << "\n";
            ok = false;
        }
    
        if(cpu.reg[z80::B] != test["final"]["b"]) {
            std::clog << "\t- Mismatch reg[B] RESULT: " << (int)cpu.reg[z80::B] << ", ACTUAL: " << (int)test["final"]["b"] << "\n";
            ok = false;
        }
    
        if(cpu.reg[z80::C] != test["final"]["c"]) {
            std::clog << "\t- Mismatch reg[C] RESULT: " << (int)cpu.reg[z80::C] << ", ACTUAL: " << (int)test["final"]["c"] << "\n";
            ok = false;
        }
    
        if(cpu.reg[z80::D] != test["final"]["d"]) {
            std::clog << "\t- Mismatch reg[D] RESULT: " << (int)cpu.reg[z80::D] << ", ACTUAL: " << (int)test["final"]["d"] << "\n";
            ok = false;
        }
    
        if(cpu.reg[z80::E] != test["final"]["e"]) {
            std::clog << "\t- Mismatch reg[E] RESULT: " << (int)cpu.reg[z80::E] << ", ACTUAL: " << (int)test["final"]["e"] << "\n";
            ok = false;
        }
    
        // Do not care about the undocumented flags
        cpu.reg[z80::F] &= 0b11010111;
        uint8 real_f = (int)test["final"]["f"] & 0b11010111;
        if(cpu.reg[z80::F] != real_f) {
            std::clog << "\t- Mismatch reg[F] RESULT: " << (int)cpu.reg[z80::F] << ", ACTUAL: " << (int)real_f << "\n";
            ok = false;
        }
    
        if(cpu.reg[z80::H] != test["final"]["h"]) {
            std::clog << "\t- Mismatch reg[H] RESULT: " << (int)cpu.reg[z80::H] << ", ACTUAL: " << (int)test["final"]["h"] << "\n";
            ok = false;
        }
    
        if(cpu.reg[z80::L] != test["final"]["l"]) {
            std::clog << "\t- Mismatch reg[L] RESULT: " << (int)cpu.reg[z80::L] << ", ACTUAL: " << (int)test["final"]["l"] << "\n";
            ok = false;
        }
    
        if(cpu.indexRegisterX != test["final"]["ix"]) {
            std::clog << "\t- Mismatch indexRegisterX RESULT: " << (int)cpu.indexRegisterX << ", ACTUAL: " << (int)test["final"]["ix"] << "\n";
            ok = false;
        }
    
        if(cpu.indexRegisterY != test["final"]["iy"]) {
            std::clog << "\t- Mismatch indexRegisterY RESULT: " << (int)cpu.indexRegisterY << ", ACTUAL: " << (int)test["final"]["iy"] << "\n";
            ok = false;
        }
    
        if(cpu.interruptMode != test["final"]["im"]) {
            std::clog << "\t- Mismatch interruptMode RESULT: " << (int)cpu.interruptMode << ", ACTUAL: " << (int)test["final"]["im"] << "\n";
            ok = false;
        }
    
        if(cpu.IFF1 != test["final"]["iff1"]) {
            std::clog << "\t- Mismatch IFF1 RESULT: " << (int)cpu.IFF1 << ", ACTUAL: " << (int)test["final"]["iff1"] << "\n";
            ok = false;
        }
    
        if(cpu.IFF2 != test["final"]["iff2"]) {
            std::clog << "\t- Mismatch IFF2 RESULT: " << (int)cpu.IFF2 << ", ACTUAL: " << (int)test["final"]["iff2"] << "\n";
            ok = false;
        }
    
        af_                  = test["final"]["af_"];
        bc_                  = test["final"]["bc_"];
        de_                  = test["final"]["de_"];
        hl_                  = test["final"]["hl_"];


        if(cpu.reg[z80::A_p] != (af_ >> 8)) {
            std::clog << "\t- Mismatch reg[A'] RESULT: " << (int)cpu.reg[z80::A_p] << ", ACTUAL: " << (int)(af_ >> 8) << "\n";
            ok = false;
        }
    
        // Do not care about the undocumented flags
        cpu.reg[z80::F_p] &= 0b11010111;
        uint8 real_fp = af_ & 0b11010111;
        if(cpu.reg[z80::F_p] != real_fp) {
            std::clog << "\t- Mismatch reg[F'] RESULT: " << (int)cpu.reg[z80::F_p] << ", ACTUAL: " << (int)real_fp << "\n";
            ok = false;
        }
    
        if(cpu.reg[z80::B_p] != bc_ >> 8) {
            std::clog << "\t- Mismatch reg[B'] RESULT: " << (int)cpu.reg[z80::B_p] << ", ACTUAL: " << (int)(bc_ >> 8) << "\n";
            ok = false;
        }
    
        if(cpu.reg[z80::C_p] != (uint8)bc_) {
            std::clog << "\t- Mismatch reg[C'] RESULT: " << (int)cpu.reg[z80::C_p] << ", ACTUAL: " << (int)(uint8)bc_ << "\n";
            ok = false;
        }
    
        if(cpu.reg[z80::D_p] != de_ >> 8) {
            std::clog << "\t- Mismatch reg[D'] RESULT: " << (int)cpu.reg[z80::D_p] << ", ACTUAL: " << (int)(de_ >> 8) << "\n";
            ok = false;
        }
    
        if(cpu.reg[z80::E_p] != (uint8)de_) {
            std::clog << "\t- Mismatch reg[E'] RESULT: " << (int)cpu.reg[z80::E_p] << ", ACTUAL: " << (int)(uint8)de_ << "\n";
            ok = false;
        }
    
        if(cpu.reg[z80::H_p] != hl_ >> 8) {
            std::clog << "\t- Mismatch reg[H'] RESULT: " << (int)cpu.reg[z80::H_p] << ", ACTUAL: " << (int)(uint8)hl_ << "\n";
            ok = false;
        }
    
        if(cpu.reg[z80::L_p] != (uint8)hl_) {
            std::clog << "\t- Mismatch reg[L'p] RESULT: " << (int)cpu.reg[z80::L_p] << ", ACTUAL: " << (int)(uint8)hl_ << "\n";
            ok = false;
        }
    
        for(auto& ramEntry : test["final"]["ram"]) {
    
            if(cpu.mapper_read(ramEntry[0]) != ramEntry[1]) {
                std::clog << "\t- Mismatch RAM[" << ramEntry[0] << "] RESULT: " << (int)cpu.mapper_read(ramEntry[0]) << ", ACTUAL: " << ramEntry[1] << "\n";
                ok = false;
            }
        }

        for(auto& portEntry : test["final"]["ports"]) {
    
            if(cpu.port_read(portEntry[0]) != portEntry[1]) {
                std::clog << "\t- Mismatch PORT[" << portEntry[0] << "] RESULT: " << (int)cpu.port_read(portEntry[0]) << ", ACTUAL: " << portEntry[1] << "\n";
                ok = false;
            }
        }

        if(ok) {
            std::clog << "\t" << test["name"] << " PASSED\n\n";
        }else {
            std::clog << "\t" << test["name"] << " FAILED\n\n";
        }
    }
    return true;
}

int main(int argc, char* argv[]) {

    // Redirect the log to a file
    std::fstream log ("log.txt", std::fstream::out | std::fstream::trunc);
    std::clog.rdbuf(log.rdbuf());

    // Create the emulator and allocate 64kb for tests
    z80 cpu;
    cpu.mapper_read = mapper_read;
    cpu.mapper_write = mapper_write;
    cpu.port_read = mapper_read;
    cpu.port_write = mapper_write;
    
    for(auto& entry : std::filesystem::directory_iterator("tests")) {
        run_tests(cpu, entry.path().string());
    }
    
    //run_tests(cpu, "tests/ED 45.json");

    log.close();

    return 0;
}