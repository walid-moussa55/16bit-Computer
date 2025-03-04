#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstring>

#include "instructions_dict.h"

struct Instruction_t {
    std::string command;
    std::string value;
};

struct Table {
    std::vector<unsigned short> items;
};

void Preprocessing(const std::string& filename){
    std::ifstream ifs(filename);
    if(!ifs){std::cerr<<"Error : This file \""<<filename<<"\" not exist"<<std::endl;exit(1);}
    std::ofstream tempf("tempf.ass");
    std::string line;
    tempf<<"jmp _start\n";
    while(getline(ifs,line)){
        if(line.find(".lib")!=std::string::npos){
            size_t start = line.find('"');
            size_t end = line.rfind('"');
            std::string name;
            if (start != std::string::npos && end != std::string::npos) {
                if (start != end) { // Correct condition
                    name = line.substr(start + 1, end - start - 1);
                } else {std::cerr << "Error: Undefined file inside \" \"" << std::endl;exit(1);}
            } else {std::cerr << "Error: Expected \"" << std::endl;exit(1);}
            std::ifstream other(name);
            if(!other) {std::cerr<<"this file \""<<name<<"\" not exist!"<<std::endl;exit(1);}
            std::cout<<"Importing '"<< name <<"' ..."<< std::endl;
            std::stringstream ss;
            ss<<other.rdbuf();
            ss<<"\n";
            line = ss.str();
            tempf<<line<<"\n";
        }else{
            tempf<<line<<"\n";
        }
    }
    ifs.close();
    tempf.close();
}


int countWordsInText(const std::string& text) {
    int valid_characters = 0;
    // Iterate through the text, excluding '"' and '$'
    for (int i = 0; i < text.length(); ++i) {
        if (text[i] != '\"' && text[i] != '$') valid_characters++;
    }
    return valid_characters;
}

std::string trim(const std::string& text) {
    size_t start = text.find_first_not_of(" \t");
    size_t end = text.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : text.substr(start, end - start + 1);
}

std::string splitInstruction(const std::string& line) {
    bool insideQuotes = false;
    size_t commentPos = std::string::npos;
    for (size_t i = 0; i < line.length(); ++i) {
        if (line[i] == '\"') insideQuotes = !insideQuotes;  // Toggle quote status
        else if (!insideQuotes && line[i] == ';') {
            commentPos = i;
            break;  // Found the semicolon outside quotes, stop searching
        }
    }
    if (commentPos != std::string::npos) {
        std::string instruction = line.substr(0, commentPos);
        return trim(instruction);
    }
    return trim(line); // No comment
}


void ReadFile(const std::string& filename, std::vector<Instruction_t>& lines, std::unordered_map<std::string, unsigned int>& labels){
    std::ifstream ifile(filename);
    if(!ifile){std::cerr<<"This file \""<<filename<<"\" is not exist"<<std::endl;exit(1);}
    unsigned int n = 0;
    std::string line;
    while(std::getline(ifile,line)){
        line = splitInstruction(line);
        if(line.empty()) continue;
        std::string first;
        std::string second;
        if(line[line.length()-1] == ':'){
            std::replace(line.begin(),line.end(),':',' ');
            std::stringstream ss(line);
            std::string name;
            ss >> name;
            labels[name] = n;
            continue;
        }else{
            if(line.find("\"")!= std::string::npos){
                std::cout<<line<<std::endl;
                int count = (countWordsInText(line)+1)/2;
                std::cout<<count<<std::endl;
                n+= count+1;
                lines.push_back({line,second});
            }else{
                std::stringstream ss(line);
                ss>>first;
                ss>>second;
                lines.push_back({first,second});
                n = (second.empty() || second == "")? n+1 : n+2;
            }
        }
    }
    ifile.close();
}

Table StringToCode(const std::string& text) {
    Table table;
    int n = text.length();
    // Ensure the string has at least two characters (ignoring the quote)
    if (n <= 1) return table;
    for (int i = 1; i < n - 1 && text[i] != '$'; i += 2) { // Start at 1 to skip leading quote, end before last quote
        unsigned short buff;
        // Process the first character
        char c1 = (text[i] == '$') ? '\0' : text[i];
        // Process the second character (if present)
        char c2 = (i + 1 >= n - 1) ? '\0' : (text[i + 1] == '$' ? '\0' : text[i + 1]);
        unsigned short s1 = static_cast<unsigned short>(c1);
        unsigned short s2 = static_cast<unsigned short>(c2);
        // Combine the two characters into a 16-bit value
        buff = (s1 & 0x00FF) | ((s2 & 0x00FF) << 8);
        // Add the combined value to the table
        table.items.push_back(buff);
    }
    return table;
}

std::vector<std::string> UnicodeForRAM(const std::vector<Instruction_t> lines, const std::unordered_map<std::string, unsigned int>& labels){
    std::vector<std::string> buffers;
    for(const Instruction_t& inst : lines){
        if(inst.command.empty()) continue;
        if(Instructions_Dict.find(inst.command)==Instructions_Dict.end()){
            if(inst.command.find("\"") != std::string::npos){
                Table table = StringToCode(inst.command);
                std::cout<<"Text count :"<<countWordsInText(inst.command)<<std::endl;
                std::cout<<"Table size :"<<table.items.size()<<std::endl;
                std::stringstream ss1;
                ss1<<std::setw(4)<<std::setfill('0')<<std::hex<<countWordsInText(inst.command);
                buffers.push_back(ss1.str());
                for(const unsigned short& item : table.items){
                    std::stringstream buffer;
                    buffer<<std::setw(4)<<std::setfill('0')<<std::hex<<item;
                    buffers.push_back(buffer.str());
                }continue;
            }else if(labels.find(inst.command)!=labels.end()){
                std::stringstream buffer;
                buffer<<std::setw(4)<<std::setfill('0')<<std::hex<<labels.at(inst.command);
                buffers.push_back(buffer.str());continue;
            }else if(std::all_of(inst.command.begin(), inst.command.end(), ::isdigit)){
                std::stringstream buffer;
                buffer<<std::setw(4)<<std::setfill('0')<<std::hex<<std::atoi(inst.command.c_str());
                buffers.push_back(buffer.str());continue;
            }else{
                std::cerr<<"Error : undefined command \""<<inst.command<<"\" "<<std::endl;exit(1);
            }
        }else{
            std::stringstream buffer1;
            buffer1<<std::setw(4)<<std::setfill('0')<<std::hex<<Instructions_Dict[inst.command];
            buffers.push_back(buffer1.str());
            if(!inst.value.empty()){
                std::stringstream buffer;
                if(labels.find(inst.value)!=labels.end()){
                    buffer<<std::setw(4)<<std::setfill('0')<<std::hex<<labels.at(inst.value);
                }else if(std::all_of(inst.value.begin(), inst.value.end(), ::isdigit)){
                    buffer<<std::setw(4)<<std::setfill('0')<<std::hex<<std::atoi(inst.value.c_str());
                }else{
                    std::cerr<<"Error : undefined label \""<<inst.value<<"\" "<<std::endl;exit(1);
                }
                buffers.push_back(buffer.str());
            }continue;
        }
    }
    return buffers;
}

void ParseToRAM(const std::string& filename, const std::vector<std::string>& buffers){
    std::ofstream ofile(filename);
    ofile<<"v3.0 hex words addressed";
    unsigned int c = 0;
    for(const std::string& buffer : buffers){
        if(c % 16 == 0){
            ofile<<"\n"<<std::setw(4)<<std::setfill('0')<<std::hex<<c<<":";
        }
        ofile<<" "<<buffer;
        c++;
    }
    std::cout<<"File '"<<filename<<"' with size : "<<c<<std::endl;
    ofile.close();
}

int main(int argc, char* argv[]){
    std::string fileName = "program.ass";
    bool isV2Logisim = false;
    if(argc == 2){
        fileName = argv[argc-1];
    }
    
    Preprocessing(fileName);
    std::vector<Instruction_t> lines;
    std::unordered_map<std::string, unsigned int> labels;
    ReadFile("tempf.ass",lines, labels);
    for(auto& lab : labels){
        std::cout<<lab.first<<"->"<<lab.second<<std::endl;
    }

    std::vector<std::string> buffers = UnicodeForRAM(lines, labels);
    ParseToRAM("ram_unicode",buffers);
    remove("tempf.ass");
}