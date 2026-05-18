#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <windows.h>

#include "Register.h"
#include "Counter.h"
#include "Stepper.h"
#include "RAM.h"
#include "CCU.h"
#include "ALU.h"
#include "SevenSegDriver.h"
#include "Console.h"

struct CommandSplited{
    bool bits[8]; // 8 bits for parameters
};
struct CommandSplited splitCommand(unsigned char command){
    struct CommandSplited result;
    for(int i = 0; i < 8; i++){
        result.bits[i] = (command >> (i)) & 0x01; // Get the remaining bits for parameters
    }
    return result;
}

class Simulator{
public:
    Simulator(const std::string& programFilename, const bool& debugMode = false){
        // Initialize components
        regA = new Register();
        regB = new Register();
        regInstr = new Register();
        regTmp = new Register();
        PC = new Counter();
        SP = new Counter();
        stepper = new Stepper();
        ram = new RAM(programFilename);
        ccu = new CCU();
        alu = new ALU();
        sevenSegDriver = new SevenSegDriver();
        console = new Console();
        CLK = false; // Clock signal
        HLT = false; // Halt signal
        Bus = 0; // Data bus
        _debugMode = debugMode;
    }
    ~Simulator(){
        // Clean up resources
        delete regA;
        delete regB;
        delete regInstr;
        delete regTmp;
        delete PC;
        delete SP;
        delete stepper;
        delete ram;
        delete ccu;
        delete alu;
        delete sevenSegDriver;
        delete console;
    }
    void Reset(){
        // Reset all components to their initial state
        regA->Reset();
        regB->Reset();
        regInstr->Reset();
        regTmp->Reset();
        PC->Reset();
        SP->Reset();
        stepper->Reset();
        ram->Reset();
        alu->Reset();
        sevenSegDriver->Reset();
        console->Reset();
        CLK = false; // Reset clock signal
        HLT = false; // Reset halt signal
    }
    void start_simulation(){
        // Main simulation loop
        while(!HLT){ // Run until halt signal is received
            this->stepper->setCLK(CLK); // Update the stepper with the current clock signal
            this->regInstr->setCLK(CLK); // Update the instruction register with the current clock signal
            this->PC->setCLK(CLK); // Update the program counter with the current clock signal
            this->SP->setCLK(CLK); // Update the stack pointer with the current clock
            this->regA->setCLK(CLK); // Update register A with the current clock signal
            this->regB->setCLK(CLK); // Update register B with the current clock signal
            this->regTmp->setCLK(CLK); // Update temporary register with the current clock signal
            this->ram->setCLK(CLK); // Update RAM with the current clock signal
            this->alu->setCLK(CLK); // Update ALU with the current clock signal
            this->sevenSegDriver->setCLK(CLK); // Update seven segment driver with the current clock signal
            this->console->setCLK(CLK); // Update console with the current clock signal

            char t = this->stepper->getQ(); // Get the current step from the stepper
            unsigned char i = this->regInstr->getValue(); // Get the current instruction from the instruction register
            char f = this->alu->getFlags(); // Get the current flags from the ALU
            this->ccu->setInputs(t, i, f); // Set the inputs for the control unit

            // Get the control signals from the control unit
            struct Output output = this->ccu->getOutput(); // Get the control signals from the control unit
            
            char alup = output.parts[0] & 0x7F; // Get the first 7 bits ALU control signals from the output
            bool fi = (output.parts[0] >> 7) & 0x01; // Get the 8th bit flag input signal from the output
            struct CommandSplited part1_splited = splitCommand(output.parts[1]); // Split the first part of the output into parameters
            bool eo = part1_splited.bits[0]; // Get the enable output signal from the first part of the output
            bool ai = part1_splited.bits[1]; // Get the register A input signal from the first part of the output
            bool ao = part1_splited.bits[2]; // Get the register A output signal from the first part of the output
            bool bi = part1_splited.bits[3]; // Get the register B input signal from the first part of the output
            bool bo = part1_splited.bits[4]; // Get the register B output signal from the first part of the output
            char ab = (part1_splited.bits[6] << 1) | part1_splited.bits[5]; // Get the AB selection signal from the first part of the output
            HLT = part1_splited.bits[7]; // Get the halt signal from the first part of the output

            struct CommandSplited part2_splited = splitCommand(output.parts[2]); // Split the second part of the output into parameters
            bool pc_j = part2_splited.bits[0]; // Get the program counter jump signal from the second part of the output
            bool pc_in = part2_splited.bits[1]; // Get the program
            bool pc_e = part2_splited.bits[2]; // Get the program counter enable signal from the second part of the output
            bool pc_o = part2_splited.bits[3]; // Get the program counter output signal from the second part of the output
            bool sp_j = part2_splited.bits[4]; // Get the stack pointer jump signal from the second part of the output
            bool sp_in = part2_splited.bits[5]; // Get the stack pointer input signal from the second part of the output
            bool sp_e = part2_splited.bits[6]; // Get the stack pointer enable signal from the second part of the output
            bool sp_o = part2_splited.bits[7]; // Get the stack pointer output signal from the second part of the output

            struct CommandSplited part3_splited = splitCommand(output.parts[3]); // Split the third part of the output into parameters
            bool mi = part3_splited.bits[0]; // Get the memory input signal from the third part of the output
            bool ri = part3_splited.bits[1]; // Get the RAM input signal from the third part of the output
            bool ro = part3_splited.bits[2]; // Get the RAM output signal from the third part of the output
            bool ii = part3_splited.bits[3]; // Get the instruction register input signal from the third part of the output
            bool oi = part3_splited.bits[4]; // Get the SevenSegDrive input signal from the third part of the output
            bool ie = part3_splited.bits[5]; // Get the input enable signal for Console from the third part of the output
            bool sp = part3_splited.bits[6]; // Get the shiflt8bits signal from the third part of the output
            bool ci = part3_splited.bits[7]; // Get the push console signal from the third part of the output

            struct CommandSplited part4_splited = splitCommand(output.parts[4]); // Split the fourth part of the output into parameters
            bool ccl = part4_splited.bits[0]; // Get the clear console signal from the fourth part of the output
            bool di = part4_splited.bits[1]; // Get the tmp register input signal from the fourth part of the output
            bool do_ = part4_splited.bits[2]; // Get the tmp register output signal from the fourth part of the output
            bool co = part4_splited.bits[3]; // Get the console output signal from the fourth part of the output

            // set parameters come from CUU to each compenents
            this->alu->setParams(ab, alup, eo, fi); // Set the parameters for the ALU
            this->regA->setInputs(ai, ao); // Set the input and output for register A
            this->regB->setInputs(bi, bo); // Set the input and output for register B
            this->PC->setJInEO(pc_j, pc_in, pc_e, pc_o); // Set the inputs for the program counter
            this->SP->setJInEO(sp_j, sp_in, sp_e, sp_o); // Set the inputs for the stack pointer
            this->ram->setInputs(mi, ri, ro); // Set the inputs for the RAM
            this->regInstr->setInputs(ii, oi); // Set the input and output for the instruction register
            this->regTmp->setInputs(di, do_); // Set the input and output for the temporary register
            this->sevenSegDriver->setOI(oi); // Set the input for the seven segment driver
            this->console->setInputs(ie, sp, ci, co, ccl); // Set the inputs for the console

            this->stepper->step(); // Step the stepper
            this->regA->step(); // Step register A
            this->regB->step(); // Step register B
            this->PC->step(); // Step the program counter
            this->SP->step(); // Step the stack pointer
            this->ram->step(); // Step the RAM
            this->regInstr->step(); // Step the instruction register
            this->regTmp->step(); // Step the temporary register
            this->alu->execute(); // Execute the ALU operation
            this->sevenSegDriver->step(); // Step the seven segment driver
            this->console->step(); // Step the console

            if (this->_debugMode) printStatenCLK(output);

            _togglreClock(); // Toggle the clock signal

            this->stepper->setCLK(CLK); // Update the stepper with the current clock signal
            this->regInstr->setCLK(CLK); // Update the instruction register with the current clock signal
            this->PC->setCLK(CLK); // Update the program counter with the current clock signal
            this->SP->setCLK(CLK); // Update the stack pointer with the current clock
            this->regA->setCLK(CLK); // Update register A with the current clock signal
            this->regB->setCLK(CLK); // Update register B with the current clock signal
            this->regTmp->setCLK(CLK); // Update temporary register with the current clock signal
            this->ram->setCLK(CLK); // Update RAM with the current clock signal
            this->alu->setCLK(CLK); // Update ALU with the current clock signal
            this->sevenSegDriver->setCLK(CLK); // Update seven segment driver with the current clock signal
            this->console->setCLK(CLK); // Update console with the current clock signal

            // Set each component outputs to the bus
            if (ao) Bus =  this->regA->getQ(); // Get the output of register A and put it on the bus
            else if (bo) Bus =  this->regB->getQ(); // Get the output of register B and put it on the bus
            else if (eo) Bus =  this->alu->getQ(); // Get the output of the ALU and put it on the bus
            else if (pc_o) Bus =  this->PC->getQ(); // Get the output of the program counter and put it on the bus
            else if (sp_o) Bus =  this->SP->getQ(); // Get the output of the stack pointer and put it on the bus
            else if (ro) Bus =  this->ram->getQ(); // Get the output of the RAM and put it on the bus
            else if (do_) Bus =  this->regTmp->getQ(); // Get the output of the temporary register and put it on the bus
            else if (co) Bus =  this->console->getQ(); // Get the output of the console and put it on the bus
            // Set the bus value to the input of each component
            this->regA->setD(Bus); // Set the input of register A to the bus
            this->regB->setD(Bus); // Set the input of register B to the bus
            this->PC->setD(Bus); // Set the input of the program counter to the bus
            this->SP->setD(Bus); // Set the input of the stack pointer to the bus
            this->ram->setD(Bus); // Set the input of the RAM to the bus
            this->regInstr->setD(Bus); // Set the input of the instruction register to the bus
            this->regTmp->setD(Bus); // Set the input of the temporary register to the bus
            this->console->setD(Bus); // Set the input of the console to the bus
            this->sevenSegDriver->setD(Bus); // Set the input of the seven segment driver to the bus
            this->alu->setA_B(this->regA->getValue(), this->regB->getValue()); // Set the inputs of the ALU to the values of register A and register B
            // Step all components to update their state based on the current inputs
            this->regA->step(); // Step register A
            this->regB->step(); // Step register B
            this->PC->step(); // Step the program counter
            this->SP->step(); // Step the stack pointer
            this->ram->step(); // Step the RAM
            this->regInstr->step(); // Step the instruction register
            this->regTmp->step(); // Step the temporary register
            this->alu->execute(); // Execute the ALU operation
            this->sevenSegDriver->step(); // Step the seven segment driver
            this->console->step(); // Step the console
            if (this->_debugMode) printStateCLK();

            _togglreClock(); // Toggle the clock signal

            if (this->_debugMode) Sleep(1000);
        }
        std::cout << "Program completed!" << std::endl;
    }
private:
    Register* regA;
    Register* regB;
    Register* regInstr;
    Register* regTmp;
    Counter* PC;
    Counter* SP;
    Stepper* stepper;
    RAM* ram;
    CCU* ccu;
    ALU* alu;
    SevenSegDriver* sevenSegDriver;
    Console* console;
    bool CLK;
    bool HLT; // Halt signal
    bool _debugMode;
    unsigned short Bus; // Data bus
    void _togglreClock(){
        CLK = !CLK; // Toggle the clock signal
    }
    void printStatenCLK(const Output& output){
        std::ostringstream table;
        table<<"___________________________________________________________________________________________________________________"<<std::endl;
        table<<"|CLK|S|       ALUP         |     |RegA |RegB |        |     PC    |    SP     | RAM    |In|7s|  Console     |RegTm|"<<"Instr : "<<*this->regInstr<<std::setfill(' ')<<std::endl;
        table<<"-------------------------------------------------------------------------------------------------------------------"<<std::endl;
        table<<"|0|1|S| SHF |  LOGIC | OPR |FI|EO|AI|AO|BI|BO| A-B |HL| J| I| E| O| J| I| E| O|MI|RI|RO|II|OI|IE|SP|CI|CC|CO|DI|DO|"<<std::endl;
        table<<"-------------------------------------------------------------------------------------------------------------------"<<std::endl;
        table<<"|"<<(!CLK?"x":" ")<<"|"<<(CLK?"x":" ")<<"|"<<this->stepper->getQ()<<"|";
        for (int i = 0; i < 5; i++){
            struct CommandSplited part_splited = splitCommand(output.parts[i]);
            for (int j = 0; j<8;j++){
                if (i==4&&j>3) break;
                table<<std::setw(2)<<part_splited.bits[j]<<"|";
            }
        }
        if(!CLK) std::cout<<table.str();
    }
    void printStateCLK(){
        std::ostringstream table;
        table<<std::endl<<"-------------------------------------------------------------------------------------------------------------------"<<std::endl;
        table<<"|"<<(!CLK?"x":" ")<<"|"<<(CLK?"x":" ")<<"|"<<*this->stepper<<"|";
        table<<*this->alu<<" |     |"<<*this->regA<<" |"<<*this->regB<<" |        |    "<<*this->PC<<"   |    "<<*this->SP<<"  |"<<*this->ram<<"|"<<*this->regInstr;
        table<<"|"<<*this->sevenSegDriver<<"|  "<<*this->console<<"    |"<<*this->regTmp<<"|Bus:"<<std::hex << std::setw(2) << std::setfill('0') <<this->Bus<<"|";
        table<<std::endl<<"-------------------------------------------------------------------------------------------------------------------"<<std::endl;
        if(CLK) std::cout<<table.str();
        this->console->printOutput();
    }
};


int main(int argc, char* argv[]){
    std::string programFilename = "ram_unicode"; // Default program filename
    bool debugMode = false; // Debug mode flag
    // Parse command line arguments
    // syntaxe : > simulator.exe programFile --debug
    if(argc == 2){
        if (!strcmp(argv[1] , "--debug")) debugMode = true;
        else programFilename = argv[argc - 1];
    }
    if(argc == 3){
        if (!strcmp(argv[1] , "--debug")) debugMode = true;
        else if (!strcmp(argv[2] , "--debug")){
            debugMode = true;
            programFilename = argv[argc-2];
        }else{
            programFilename = argv[argc - 2];
        }
    }
    Simulator simulator(programFilename, debugMode); // Create a simulator instance with the specified program filename
    simulator.Reset(); // Reset the simulator to initialize all components
    simulator.start_simulation(); // Start the simulation
    return 0;
}