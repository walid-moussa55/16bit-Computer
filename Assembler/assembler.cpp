#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <unordered_set>

#include "instructions_dict.h"

struct Instruction_t {
    std::string command;
    std::string value;
};

struct Table {
    std::vector<unsigned short> items;
};

std::string trim(const std::string& text) {
    size_t start = text.find_first_not_of(" \t");
    size_t end = text.find_last_not_of(" \t");
    return (start == std::string::npos) ? "" : text.substr(start, end - start + 1);
}

std::unordered_set<std::string> includedFiles;

void PreprocessFile(const std::string& filename, std::ofstream& tempf) {
    std::ifstream ifs(filename);
    if (!ifs) { std::cerr << "Error: file \"" << filename << "\" not exist\n"; exit(1); }

    std::string line;
    while (std::getline(ifs, line)) {
        std::string trimmed = trim(line);
        if (trimmed.rfind(".lib", 0) == 0) {
            size_t start = trimmed.find('"');
            size_t end = trimmed.rfind('"');
            if (start == std::string::npos || end == std::string::npos || start == end) {
                std::cerr << "Error: invalid .lib syntax\n";
                exit(1);
            }
            std::string name = trimmed.substr(start + 1, end - start - 1);
            std::string fullpath = std::filesystem::current_path().string() + "\\" + name;
            if (includedFiles.find(name) == includedFiles.end()) {
                includedFiles.insert(name);
                std::cout<<"Importing '"<< name <<"'"<< std::endl;
                tempf << "; --- Importing '"<< name <<"'\n";
                PreprocessFile(fullpath, tempf);
            }
        } else {
            tempf << line << "\n";
        }
    }
}

void Preprocessing(const std::string& filename, const std::string& outputfilename = "tempf.ass") {
    std::ofstream tempf(outputfilename);
    tempf << "jmp _start\n";
    PreprocessFile(filename, tempf);
}

int countWordsInText(const std::string& text) {
    int valid_characters = 0;
    // Iterate through the text, excluding '"' and '$'
    for (int i = 0; i < text.length(); ++i) {
        if (text[i] != '\"' && text[i] != '$') valid_characters++;
    }
    return valid_characters;
}

int getReservedWords(const std::string& text) {
    std::string trimmed = trim(text);
    std::string lower = trimmed;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return std::tolower(c); });
    if (lower.rfind("resw(", 0) == 0) {
        size_t open = trimmed.find('(');
        size_t close = trimmed.rfind(')');
        if (open == std::string::npos || close == std::string::npos || close <= open + 1) {
            std::cerr << "Error: invalid resw syntax\n";
            exit(1);
        }
        std::string number = trim(trimmed.substr(open + 1, close - open - 1));
        if (number.empty() || !std::all_of(number.begin(), number.end(), ::isdigit)) {
            std::cerr << "Error: invalid resw count\n";
            exit(1);
        }
        return std::stoi(number);
    }
    return 0;
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
                int count = (countWordsInText(line)+1)/2;
                n+= count+2;
                lines.push_back({line,second});
            }else if(line.find("resw(")!= std::string::npos || line.find("RESW(")!= std::string::npos){
                int count = getReservedWords(line);
                n+= count+2;
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
                // std::cout<<"Text count :"<<countWordsInText(inst.command)<<std::endl;
                // std::cout<<"Table size :"<<table.items.size()<<std::endl;
                std::stringstream ss1;
                std::stringstream ss2;
                ss1<<std::setw(4)<<std::setfill('0')<<std::hex<<table.items.size();
                ss2<<std::setw(4)<<std::setfill('0')<<std::hex<<countWordsInText(inst.command);
                buffers.push_back(ss1.str());
                buffers.push_back(ss2.str());
                for(const unsigned short& item : table.items){
                    std::stringstream buffer;
                    buffer<<std::setw(4)<<std::setfill('0')<<std::hex<<item;
                    buffers.push_back(buffer.str());
                }continue;
            }else if(inst.command.find("resw(")!= std::string::npos || inst.command.find("RESW(")!= std::string::npos) {
                int count = getReservedWords(inst.command);
                std::stringstream ss1;
                std::stringstream ss2;
                ss1<<std::setw(4)<<std::setfill('0')<<std::hex<<count;
                ss2<<std::setw(4)<<std::setfill('0')<<std::hex<<0;
                buffers.push_back(ss1.str());
                buffers.push_back(ss2.str());
                for (int i=0;i<count;i++){
                    std::stringstream buffer;
                    buffer<<std::setw(4)<<std::setfill('0')<<std::hex<<0;
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
    std::string outFileName = "ram_unicode";
    bool isdebug = false;
    if(argc == 2){
        if (!strcmp(argv[1] , "--debug")) isdebug = true;
        else fileName = argv[argc - 1];
    }
    if(argc == 3){
        if (!strcmp(argv[1] , "--debug")) isdebug = true;
        else if (!strcmp(argv[2] , "--debug")){
            isdebug = true;
            fileName = argv[argc-2];
        }else{
            fileName = argv[argc-2];
            outFileName = argv[argc-1];
        }
    }
    if(argc == 4){
        if(!strcmp(argv[3] , "--debug")){
            isdebug = true;
            fileName = argv[argc-3];
            outFileName = argv[argc-2];
        }else{
            fileName = argv[argc-3];
            outFileName = argv[argc-2];
        }
    }    
    Preprocessing(fileName);
    std::vector<Instruction_t> lines;
    std::unordered_map<std::string, unsigned int> labels;
    ReadFile("tempf.ass",lines, labels);
    if(isdebug){
        for(auto& lab : labels){
            std::cout<<lab.first<<" -> "<<lab.second<<" (0x"<<std::setw(4)<<std::setfill('0')<<std::hex<<lab.second<<")"<<std::dec<<std::endl;
        }
    }

    std::vector<std::string> buffers = UnicodeForRAM(lines, labels);
    ParseToRAM(outFileName,buffers);
    remove("tempf.ass");
}