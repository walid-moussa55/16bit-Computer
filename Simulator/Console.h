
/*
Console: it prints the output of the simulation to the console inside a string, it contain a register that store a two caraters in 16bits, and ptrint them one by one.
    - inputs: D, IE, CLK, SP, Push, OE, CCL, R
    - output: Q
*/

class Console {
public:
    Console(): reg(0), IE(false), CLK(false), SP(false), Push(false), OE(false), CCL(false), D(0), Q(0) {
        output = "";
    }
    ~Console() {}
    void setInputs(bool IE, bool SP, bool Push, bool OE, bool CCL) {
        this->IE = IE;
        this->SP = SP;
        this->Push = Push;
        this->OE = OE;
        this->CCL = CCL;
    }
    void setD(short D) {
        this->D = D;
    }
    void setCLK(bool CLK) {
        this->CLK = CLK;
    }
    void Reset() {
        reg = 0;
        output = "";
    }
    std::string getOutput() const{
        return output;
    }
    short getQ() const{
        return Q;
    }
    short getReg() const{
        return reg;
    }
    void step() {
        if (CLK && IE){
            reg = D; // store the input data in the register
        }
        char character = reg & 0xFF; // variable to store the character to print
        if (SP) character = (reg >> 8) & 0xFF; // get the upper 8 bits of the register
        if (CLK && Push) {
            output += character; // append the character to the output string
        }
        if (OE) {
            Q = character; // output the value of the register
        }
        if(CCL) {
            reg = 0; // clear the register
            output = ""; // clear the output string after printing
        }
    }
    friend std::ostream& operator<<(std::ostream& os, const Console& console) {
        os << std::hex << std::setw(4) << std::setfill('0') << console.getReg();
        return os;
    }
    void printOutput() {
        if (std::empty(output)) return;
        std::cout << "Output:-----------------------------------------" << std::endl
        << output << std::endl
        <<            "-----------------------------------------------" << std::endl; // print the output string to the console
    }
    
private:
    short reg; // register to store two characters in 16 bits
    bool IE, CLK, SP, Push, OE, CCL; // input signals
    unsigned char D, Q; // input data and output data
    std::string output; // output string to print to console
};