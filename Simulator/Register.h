
/*
Register: 16-bit register with input enable, output enable, and clock signal.
    - inputs: IE, OE, CLK, D
    - putputs: Q
*/
class Register{
public:
    Register(): value(0), D(0), Q(0), CLK(false), IE(false), OE(false) {}
    ~Register(){}
    void setInputs(bool ie, bool oe){ // Set the input enable and output enable signals
        IE = ie;
        OE = oe;
    }
    void setCLK(bool clk){ // Set the clock signal
        CLK = clk;
    }
    void Reset(){
        value = 0;
    }
    void setD(unsigned short d){ // Data to input
        D = d;
    }
    unsigned short getQ() const{ // Output of the register
        return Q;
    }
    unsigned short getValue() const {
        return value;
    }
    void step(){
        if (IE && CLK) {
            value = D; // Load new data into the register
        }
        if (OE) {
            Q = value; // Output the current value of the register
        }
    }
    // print function that print as hexadecimal with width 4 filled with 0 with concatination with iostream using operator overloading
    friend std::ostream& operator<<(std::ostream& os, const Register& reg) {
        os << std::hex << std::setw(4) << std::setfill('0') << reg.getValue();
        return os;
    }
    unsigned short value;
    bool IE; // Input Enable
    bool OE; // Output Enable
    bool CLK; // Clock signal
    unsigned short D; // Data input
    unsigned short Q; // Data output
};