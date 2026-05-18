
/*
RAM: Memory module for the simulator. It provides an interface to read and write data(16bits) to the memory with 16bits address, as well as to load memory contents from files like this:
v3.0 hex words addressed
0000: 0026 01e2 0026 01e2 0000 0001 0004 003f 0021 003a 0003 0004 0022 0042 0027 002b
0010: 0020 0002 0004 0040 0033 0022 003c 0020 0022 0042 0027 002b 0020 0002 0004 0040
0020: 0035 0022 003c 0020 0002 0004 003b 0004 0004 0026 000c 0025 0000 0001 002c 003f
    - inputs: MI, RI, CLK, RO, D(16bits)
    - outputs: Q(16bits)
*/

class RAM {
public:
    RAM(const std::string& filename): MI(false), RI(false), CLK(false), RO(false), D(0), Q(0), AddressReg(0), pointer(0) {
        std::fill(std::begin(memory), std::end(memory), 0);
        loadFromFile(filename.c_str());
    }
    ~RAM(){}
    void setInputs(bool MI, bool RI, bool RO){
        this->MI = MI;
        this->RI = RI;
        this->RO = RO;
    }
    void setD(unsigned short D){
        this->D = D;
    }
    void setCLK(bool CLK){
        this->CLK = CLK;
    }
    void Reset(){
        AddressReg = 0;
    }
    unsigned short getQ() const{
        return Q;
    }
    unsigned short getPointer() const{
        return pointer;
    }
    unsigned short getAddressReg() const{
        return AddressReg;
    }
    void loadFromFile(const std::string& filename){
        std::ifstream infile(filename);
        if (!infile.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            return;
        }
        std::string line;
        while (std::getline(infile, line)) {
            if (line.empty() || line[0] == 'v' || line[0] == ' ') {
                continue; // Skip empty lines and header
            }
            std::istringstream iss(line);
            std::string addressToken;
            if (!(iss >> addressToken)) { continue; }
            if (addressToken.back() != ':' || addressToken.size() < 2) {
                continue;
            }
            std::string addressHex = addressToken.substr(0, addressToken.size() - 1);
            unsigned long addrLong;
            try {
                addrLong = std::stoul(addressHex, nullptr, 16);
            } catch (const std::invalid_argument&) {
                continue;
            } catch (const std::out_of_range&) {
                std::cerr << "Address out of range: " << addressHex << std::endl;
                continue;
            }
            unsigned short addr = static_cast<unsigned short>(addrLong);
            unsigned short data;
            while (iss >> std::hex >> data) {
                if (addr < 65536) {
                    memory[addr] = data;
                    addr++;
                } else {
                    std::cerr << "Address out of range: " << addr << std::endl;
                    break;
                }
            }
        }
        infile.close();
    }
    void step(){
        if (CLK) {
            if (MI) {
                AddressReg = D;
            }
            if (RI) {
                memory[AddressReg] = D;
            }
        }
        pointer = memory[AddressReg];
        if (RO) {
            Q = memory[AddressReg];
        }
    }
    friend std::ostream& operator<<(std::ostream& os, const RAM& ram) {
        os << std::hex << std::setw(4) << std::setfill('0') << ram.getAddressReg() << ":" << std::hex << std::setw(4) << std::setfill('0') << ram.getPointer();
        return os;
    }
private:
    bool MI, RI, CLK, RO;
    unsigned short D;
    unsigned short Q;
    unsigned short AddressReg;
    unsigned short pointer;
    unsigned short memory[65536];
};
