
/*
ALU: Arithmetic Logic Unit do aritmetic operations(add, sub, mul, div(result integer)) with shift operations(nop, zero, shr, shl) and logic operations(nop, zero, one, and, or, xor, cmp) with 16bits input and output, 2bits for selecting A or B or AB, 7bits for selecting operations, 1bit for output enable and 1bit for flag input. The ALU will output the result of the selected operation on the selected input(s) when the output enable is true. The flags will be set according to the result of the operation (Zero flag if the result is zero, AddCarry flag if there is a carry in addition, ShiftCarry flag if there is a carry in shift operations, and LogicFlags for logic operations). The ALU will also have a function to print the current value of the output in hexadecimal format.
    - input: A, B, AB(2bits), Sel(7bits: 2bits for operations, 3bits for logic, 2bits for shift), OE, FI, CLK, R
    - output: Q, FlagsReg(7bits: Zero, AddCarry, ShiftCarry, LogicsFlags(4bits))
*/

class ALU {
public:
    ALU() : A(0), B(0), AB(0), Sel(0), OE(false), Q(0), FI(false), FlagsReg(0), CLK(false) {}
    ~ALU() {}
    void setParams(char ab, char sel, bool oe, bool fi) {
        AB = ab;
        Sel = sel;
        OE = oe;
        FI = fi;
    }
    void setA_B(unsigned short a, unsigned short b) {
        A = a;
        B = b;
    }
    void setCLK(bool clk) {
        CLK = clk;
    }
    void Reset() {
        FlagsReg = 0;
    }
    unsigned short getQ() const{ return Q; }
    char getFlags() const{ return FlagsReg; }
    unsigned short getValue() const{ return value; }
    void execute(){
        selectAB();
        switch (Sel & 0b11) { // Select arithmetic operation
            case 0: value = A + B; if (value < A || value < B) if(CLK && FI) FlagsReg |= 0b00000010; break; // Add and set AddCarry flag if there is a carry
            case 1: value = A - B; break; // Sub
            case 2: value = A * B; break; // Mul
            case 3: value = (B != 0) ? A / B : 0; break; // Div
            default: std::cerr << "Invalid arithmetic operation selection" << std::endl; break;
        }
        switch ((Sel >> 2) & 0b111) { // Select logic operation
            case 0: value = B; break; // NOP
            case 1: value = 0; break; // ZERO
            case 2: value = 0x1; break; // ONE
            case 3: value = B & A; break; // AND
            case 4: value = B | A; break; // OR
            case 5: value = B ^ A; break; // XOR
            case 6: {
                value = B; 
                if(CLK && FI) FlagsReg |= ((B == A) ? 0b00001000 : 0); // EQ flag
                if(CLK && FI) FlagsReg |= ((value != A) ? 0b00010000 : 0); // NE flag
                if(CLK && FI) FlagsReg |= ((B < A) ?  0b00100000 : 0); // LT flag
                if(CLK && FI) FlagsReg |= ((B > A) ?  0b01000000 : 0); // GT flag
                break; // CMP
            }
            default: std::cerr << "Invalid logic operation selection" << std::endl; break;
        }
        switch ((Sel >> 5) & 0b11) { // Select shift operation
            case 0: value = A; if(CLK && FI) FlagsReg |= (A & 0x8000) ? 0b00000100 : 0; break; // NOP
            case 1: value = 0; if(CLK && FI) FlagsReg |= (A & 0x8000) ? 0b00000100 : 0; break; // ZERO
            case 2: value = A >> 1; if(CLK && FI) FlagsReg |= (A & 0x8000) ? 0b00000100 : 0; break; // SHR and set ShiftCarry flag if there is a carry
            case 3: value = A << 1; if(CLK && FI) FlagsReg |= (A & 0x8000) ? 0b00000100 : 0; break; // SHL and set ShiftCarry flag if there is a carry
            default: std::cerr << "Invalid shift operation selection" << std::endl; break;
        }
        if (value == 0) if(CLK && FI) FlagsReg |= 0b00000001; // Set Zero flag if the result is zero
        if (OE) Q = value; // Output the result if output enable is true
    }
    // print function that print as hexadecimal with width 4 filled with 0 with concatination with iostream using operator overloading
    friend std::ostream& operator<<(std::ostream& os, const ALU& alu) {
        os << " res:" << std::hex << std::setw(4) << std::setfill('0') << alu.getValue() << " - flags:" << std::hex << std::setw(2) << std::setfill('0') << alu.getFlags();
        return os;
    }
private:
    unsigned short A, B, Q, value;
    char AB, Sel, FlagsReg;
    bool OE, FI, CLK;
    void selectAB() {
        unsigned short tmp;
        switch (AB) {
            case 0: break;
            case 1: A = B; break;
            case 2: B = A; break;
            case 3: tmp = A; A = B; B = tmp; break;
            default: std::cerr << "Invalid AB selection" << std::endl; break;
        }
    }
};
