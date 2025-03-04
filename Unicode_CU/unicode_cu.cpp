#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <iomanip>
// ROM 1 00000000
#define AD      0b00000000000000000000000000000000
#define SU      0b00000000000000000000000000000001
#define MU      0b00000000000000000000000000000010
#define DIV     0b00000000000000000000000000000011
#define LGN     0b00000000000000000000000000000000
#define LG0     0b00000000000000000000000000000100
#define LG1     0b00000000000000000000000000001000
#define NO      0b00000000000000000000000000001100
#define AND     0b00000000000000000000000000010000
#define OR      0b00000000000000000000000000010100
#define XOR     0b00000000000000000000000000011000
#define CMP     0b00000000000000000000000000011100
#define SHN     0b00000000000000000000000000000000
#define SH0     0b00000000000000000000000000100000
#define SHR     0b00000000000000000000000001000000
#define SHL     0b00000000000000000000000001100000
#define FI      0b00000000000000000000000010000000
// ROM 2
#define EO      0b00000000000000000000000100000000
#define AI      0b00000000000000000000001000000000
#define AO      0b00000000000000000000010000000000
#define BI      0b00000000000000000000100000000000
#define BO      0b00000000000000000001000000000000
#define AB      0b00000000000000000000000000000000
#define AA      0b00000000000000000010000000000000
#define BB      0b00000000000000000100000000000000
#define BA      0b00000000000000000110000000000000
#define HLT     0b00000000000000001000000000000000
// ROM 3
#define PCJ     0b00000000000000010000000000000000
#define PCD     0b00000000000000100000000000000000
#define PCE     0b00000000000001000000000000000000
#define PCO     0b00000000000010000000000000000000
#define SPJ     0b00000000000100000000000000000000
#define SPD     0b00000000001000000000000000000000
#define SPE     0b00000000010000000000000000000000
#define SPO     0b00000000100000000000000000000000
// ROM 4
#define MI      0b00000001000000000000000000000000
#define RI      0b00000010000000000000000000000000
#define RO      0b00000100000000000000000000000000
#define II      0b00001000000000000000000000000000
#define OI      0b00010000000000000000000000000000
#define IE      0b00100000000000000000000000000000
#define SP      0b01000000000000000000000000000000
#define CI      0b10000000000000000000000000000000
// ROM 5
#define CC      0b00000000000000000000000000000001
#define DI      0b00000000000000000000000000000010
#define DO      0b00000000000000000000000000000100
#define CO      0b00000000000000000000000000001000

#include "instructions_tab.h"

#define JZ  0x27
#define JAC 0x28
#define JSC 0x29
#define JE  0x2a
#define JG  0x2b
#define JL  0x2c
#define JN  0x2d

void UnicodeToROM(const std::string& filename){
    std::ofstream ofs(filename);
    ofs<<"v3.0 hex words addressed";
    int l=0;
    for(int i=0;i<128;i++){
        for(long address=0;address<8192;address++){
            int instruction = (address&0b00000000011111111000)>>3;
            int step =        (address&0b00000000000000000111);
            int sel =         (address&0b00000001100000000000)>>11;
            unsigned char db;
            if     (instruction==0x27&&step==2) db=((PCO|MI) * ((i >> 0) & 1))>>(sel*8);
            else if(instruction==0x27&&step==3) db=((RO|PCJ) * ((i >> 0) & 1))>>(sel*8);
            else if(instruction==0x27&&step==4) db=(((i >> 0) & 1) ? 0 : PCE)>> (sel*8);
            else if(instruction==0x28&&step==2) db=((PCO|MI) * ((i >> 1) & 1))>>(sel*8);
            else if(instruction==0x28&&step==3) db=((RO|PCJ) * ((i >> 1) & 1))>>(sel*8);
            else if(instruction==0x28&&step==4) db=(((i >> 1) & 1) ? 0 : PCE)>> (sel*8);
            else if(instruction==0x29&&step==2) db=((PCO|MI) * ((i >> 2) & 1))>>(sel*8);
            else if(instruction==0x29&&step==3) db=((RO|PCJ) * ((i >> 2) & 1))>>(sel*8);
            else if(instruction==0x29&&step==4) db=(((i >> 2) & 1) ? 0 : PCE)>> (sel*8);
            else if(instruction==0x2a&&step==2) db=((PCO|MI) * ((i >> 3) & 1))>>(sel*8);
            else if(instruction==0x2a&&step==3) db=((RO|PCJ) * ((i >> 3) & 1))>>(sel*8);
            else if(instruction==0x2a&&step==4) db=(((i >> 3) & 1) ? 0 : PCE)>> (sel*8);
            else if(instruction==0x2b&&step==2) db=((PCO|MI) * ((i >> 4) & 1))>>(sel*8);
            else if(instruction==0x2b&&step==3) db=((RO|PCJ) * ((i >> 4) & 1))>>(sel*8);
            else if(instruction==0x2b&&step==4) db=(((i >> 4) & 1) ? 0 : PCE)>> (sel*8);
            else if(instruction==0x2c&&step==2) db=((PCO|MI) * ((i >> 5) & 1))>>(sel*8);
            else if(instruction==0x2c&&step==3) db=((RO|PCJ) * ((i >> 5) & 1))>>(sel*8);
            else if(instruction==0x2c&&step==4) db=(((i >> 5) & 1) ? 0 : PCE)>> (sel*8);
            else if(instruction==0x2d&&step==2) db=((PCO|MI) * ((i >> 6) & 1))>>(sel*8);
            else if(instruction==0x2d&&step==3) db=((RO|PCJ) * ((i >> 6) & 1))>>(sel*8);
            else if(instruction==0x2d&&step==4) db=(((i >> 6) & 1) ? 0 : PCE)>> (sel*8);
            else db = getUnicode(instruction).unicode[step]>>(sel*8);
            std::stringstream buff;
            if(l%16==0) ofs<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<l<<":";
            buff<<std::hex<<std::setw(2)<<std::setfill('0')<<(0b0000000011111111&db);
            ofs<<" "<<buff.str();
            l++;
        }
    }
}

void UnicodeToROM2(const std::string& filename){
    std::ofstream ofs(filename);
    ofs<<"v3.0 hex words addressed";
    int l=0;
    for(int i=0;i<128;i++){
        for(long address=0;address<8192;address++){
            int instruction = (address&0b00000000011111111000)>>3;
            int step =        (address&0b00000000000000000111);
            int sel =         (address&0b00000001100000000000)>>11;
            unsigned char db;
            if     (instruction==0x27&&step==2) db=((0) * ((i >> 0) & 1))>>(sel*8);
            else if(instruction==0x28&&step==2) db=((0) * ((i >> 1) & 1))>>(sel*8);
            else if(instruction==0x29&&step==2) db=((0) * ((i >> 2) & 1))>>(sel*8);
            else if(instruction==0x2a&&step==2) db=((0) * ((i >> 3) & 1))>>(sel*8);
            else if(instruction==0x2b&&step==2) db=((0) * ((i >> 4) & 1))>>(sel*8);
            else if(instruction==0x2c&&step==2) db=((0) * ((i >> 5) & 1))>>(sel*8);
            else if(instruction==0x2d&&step==2) db=((0) * ((i >> 6) & 1))>>(sel*8);
            else db = getUnicode2(instruction).unicode[step]>>(sel*8);
            std::stringstream buff;
            if(l%16==0) ofs<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<l<<":";
            buff<<std::hex<<std::setw(2)<<std::setfill('0')<<(0b0000000011111111&db);
            ofs<<" "<<buff.str();
            l++;
        }
    }
}

short digits[] = { 0x7e, 0x12, 0xbc, 0xb6, 0xd2, 0xe6, 0xee, 0x32, 0xfe, 0xf6 };

void code7SEG(const std::string& filename){
    std::ofstream ofile(filename);
    int index = 0;
    ofile<<"v3.0 hex words addressed";
    for(long value = 0; value < 65536; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
        ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<(int)digits[ value % 10];
        index++;
    }
    for(long value = 0; value < 65536; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
        ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<(int)digits[ (value/10) % 10];
        index++;
    }
    for(long value = 0; value < 65536; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
        ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<(int)digits[ (value/100) % 10];
        index++;
    }
    for(long value = 0; value < 65536; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
       ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<(int)digits[ (value/1000) % 10];
       index++;
    }
    for(long value = 0; value < 65536; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
        ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<(int)digits[ (value/10000) % 10];
        index++;
    }
    for(long value = 0; value < 65536; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
        ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<0;
        index++;
    }
    for(long value = 0; value < 65536; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
        ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<0;
        index++;
    }
    for(long value = 0; value < 65536; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
        ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<0;
        index++;
    }

    for(long value = -32768; value < 32768; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
        ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<(int)digits[ abs(value) % 10];
        index++;
    }
    for(long value = -32768; value < 32768; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
       ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<(int)digits[ abs(value/10) % 10];
       index++;
    }
    for(long value = -32768; value < 32768; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
        ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<(int)digits[ abs(value/100) % 10];
        index++;
    }
    for(long value = -32768; value < 32768; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
        ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<(int)digits[ abs(value/1000) % 10];
        index++;
    }
    for(long value = -32768; value < 32768; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
        ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<(int)digits[ abs(value/10000) % 10];
        index++;
    }
    for(long value = -32768; value < 32768; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
        ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<0;
        index++;
    }
    for(long value = -32768; value < 32768; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
        ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<0;
        index++;
    }
    for(long value = -32768; value < 32768; value++){
        if(index%16==0) ofile<<"\n"<<std::hex<<std::setw(5)<<std::setfill('0')<<index<<":";
        if( value < 0 ) ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<0x40;
        else ofile<<" "<<std::setw(2)<<std::setfill('0')<<std::hex<<0;
        index++;
    }

    ofile.close();
}

int main(){
    UnicodeToROM("./Computer/ROMs/cu_unicode");
    UnicodeToROM2("./Computer/ROMs/cu_unicode2");
    code7SEG("./Computer/ROMs/7_seg");
}
