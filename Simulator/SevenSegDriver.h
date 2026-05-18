
/*
SevenSegDriver: 16-bit register that holds the value to be displayed, but don't display it.
    - inputs: D, OI, CLK.
*/

class SevenSegDriver {
public:
    SevenSegDriver() : value(0) {}
    ~SevenSegDriver() {}
    void setD(unsigned short d) { D = d; }
    void setOI(bool oi) { OI = oi; }
    void setCLK(bool clk) { CLK = clk; }
    void Reset() { value = 0; }
    void step(){
        if (CLK && OI) {
            value = D;
        }
    }
    unsigned short getValue() const { return value; }
    friend std::ostream& operator<<(std::ostream& os, const SevenSegDriver& driver) {
        os << std::hex << std::setw(4) << std::setfill('0') << driver.getValue();
        return os;
    }
private:
    unsigned short D, value; // 16-bit register to hold the value to be displayed
    bool OI; // Output enable
    bool CLK; // Clock signal
};