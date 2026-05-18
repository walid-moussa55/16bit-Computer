

/*
CCU: Command Control Unit, represents a 5 ROMs 1MbX8, that get its data from a files that represente like this:
``
v3.0 hex words addressed
00000: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00010: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00020: 00 00 00 00 00 00 00 00 00 00 80 00 00 00 00 00
00030: 00 00 80 00 00 00 00 00 00 00 80 00 00 00 00 00
``
it have to read this file and store the data in two vectors of 8bits.
    - inputs: T(step), I(instruction), F(flags)
    - address = F(7bits) + sel(2bits) + I(8bits) + T(3bits) = 20bits
    - outputs: O(control signals) = 8bits
*/

struct Output{ // 7 + 25 + 8 = 40bits
    unsigned char parts[5]; // 5 * 8bits
};

class CCU{
public:
    CCU(): T(0), I(0), F(0) {
        rom1_0 = loadROMFromFile("./Computer/ROMs/cu_unicode");
        rom1_1 = loadROMFromFile("./Computer/ROMs/cu_unicode");
        rom1_2 = loadROMFromFile("./Computer/ROMs/cu_unicode");
        rom1_3 = loadROMFromFile("./Computer/ROMs/cu_unicode");
        rom2_0 = loadROMFromFile("./Computer/ROMs/cu_unicode2");
    }
    ~CCU(){}
    void setInputs(char T, unsigned char I, char F){
        this->T = T;
        this->I = I;
        this->F = F;
    }
    std::vector<unsigned char> loadROMFromFile(const std::string& filename){
        std::vector<unsigned char> rom;
        std::ifstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            exit(1);
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == 'v' || line[0] == ' ') {
                continue;
            }

            std::istringstream iss(line);
            std::string address;
            iss >> address; // skip "00000:"

            std::string byteStr;  // ✅ MUST be string
            while (iss >> byteStr) {
                unsigned int value = std::stoul(byteStr, nullptr, 16);
                rom.push_back(static_cast<unsigned char>(value));
            }
        }

        return rom;
    }
    Output getOutput(){
        Output output;
        unsigned int address = ((F & 0x7F) << 13) | (0 << 11) | (I << 3) | (T & 0x07); // Construct the 20-bit address
        output.parts[0] = rom1_0[address];
        address = ((F & 0x7F) << 13) | (1 << 11) | (I << 3) | (T & 0x07); // Construct the 20-bit address
        output.parts[1] = rom1_1[address];
        address = ((F & 0x7F) << 13) | (2 << 11) | (I << 3) | (T & 0x07); // Construct the 20-bit address
        output.parts[2] = rom1_2[address];
        address = ((F & 0x7F) << 13) | (3 << 11) | (I << 3) | (T & 0x07); // Construct the 20-bit address
        output.parts[3] = rom1_3[address];
        address = ((F & 0x7F) << 13) | (0 << 11) | (I << 3) | (T & 0x07); // Construct the 20-bit address
        output.parts[4] = rom2_0[address];
        return output; // Return the output.
    }

private:
    std::vector<unsigned char> rom1_0; // 20bits
    std::vector<unsigned char> rom1_1; // 20bits
    std::vector<unsigned char> rom1_2; // 20bits
    std::vector<unsigned char> rom1_3; // 20bits
    std::vector<unsigned char> rom2_0; // 20bits
    char T; // 3bits
    unsigned char I; // 8bits
    char F; // 7bits

};