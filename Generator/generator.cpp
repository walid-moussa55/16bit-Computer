#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>

void Generate_Instruction_table(const std::string& filename, const std::vector<std::string>& lines){
    int numInst = lines.size();
    std::cout<<numInst<<std::endl;

    std::ifstream ofile_read(filename);
    // if (!ofile_read) {std::cerr << "This file can't be opened for reading" << std::endl;exit(1);}
    if (!ofile_read.good()){
        std::ofstream ofile_write(filename);
        std::cout<<"Creating file "<<filename<<"..."<<std::endl;
        if (!ofile_write) {std::cerr << "This file can't be opened for writing" << std::endl;exit(1);}
        ofile_write << "// "<<numInst;
        ofile_write << "\nstruct Unicode_t{unsigned int unicode[8];};\nUnicode_t getUnicode(const unsigned char& code){\n";
        int n = 0;
        std::stringstream newInstructions;
        for(auto& line : lines){
            std::stringstream sstext;
            sstext << "\tif(code == 0x"<<std::setw(2)<<std::setfill('0')<<std::hex<<n<<") return {{MI|PCO,RO|II|PCE,0					,0				,0			,0			,0			,0			}}; // "<<lines[n++]<<"\n";
            std::cout<<"Writting..."<<std::endl;
            newInstructions << sstext.str();
        }
        newInstructions<<"\telse             return {{0		,0		  ,0					,0				,0			,0			,0			,0			}};\n}";
        newInstructions<<"\nUnicode_t getUnicode2(const unsigned char& code){\n";
        n = 0;
        for(auto& line : lines){
            std::stringstream sstext;
            sstext << "\tif(code == 0x"<<std::setw(2)<<std::setfill('0')<<std::hex<<n<<") return {{0,0,0			,0			,0			,0			,0			,0			}}; // "<<lines[n++]<<"\n";
            newInstructions << sstext.str();
        }
        newInstructions<<"\telse             return {{0,0,0			,0			,0			,0			,0			,0			}};\n}";
        ofile_write << newInstructions.str();
        ofile_write.close();
    }else{
        std::stringstream existingContent;
        {std::string nullSTR = "";ofile_read >> nullSTR;}
        int lastpos;
        ofile_read >> lastpos;
        std::cout<<"old posotion is : "<<lastpos<<std::endl;
        existingContent << ofile_read.rdbuf();  // Read the entire file content
        ofile_read.close();

        std::string content = existingContent.str();

        // Prepare to insert the new instructions before the "else" statement
        int elsePos1 = content.find("else");
        int elsePos2 = content.find("else",elsePos1+1);
        if (elsePos1 == std::string::npos || elsePos2 == std::string::npos) {std::cerr << "'else' not found in the file" << std::endl;exit(1);}
        std::stringstream newInstructions;
        for(int i = lastpos;i<numInst;i++){
            std::stringstream sstext;
            sstext << "\tif(code == 0x"<<std::setw(2)<<std::setfill('0')<<std::hex<<i<<") return {{MI|PCO,RO|II|PCE,0					,0				,0			,0			,0			,0			}}; // "<<lines[i]<<"\n";
            std::string text = sstext.str();
            std::cout<<"Writting..."<<std::endl;
            newInstructions << sstext.str();
        }
        std::stringstream newInstructions2;
        for(int i = lastpos;i<numInst;i++){
            std::stringstream sstext;
            sstext << "\tif(code == 0x"<<std::setw(2)<<std::setfill('0')<<std::hex<<i<<") return {{0,0,0			,0			,0			,0			,0			,0			}}; // "<<lines[i]<<"\n";
            std::string text = sstext.str();
            std::cout<<"Writting..."<<std::endl;
            newInstructions2 << sstext.str();
        }
        // Insert the new instructions before the "else"
        content.insert(elsePos1-1, newInstructions.str());
        content.insert(elsePos2-1+newInstructions.str().size(), newInstructions2.str());

        std::ofstream ofile_write(filename);
        std::cout<<"Updating file "<<filename<<"..."<<std::endl;
        if (!ofile_write) {std::cerr << "This file can't be opened for writing" << std::endl;exit(1);}
        ofile_write << "// "<<numInst;
        ofile_write << content;  // Write the entire modified content back to the file
        ofile_write.close();
    }
}

void Generate_Instruction_dictionary(const std::string& filename, const std::vector<std::string>& lines){
    int numInst = lines.size();
    std::cout<<numInst<<std::endl;

    std::ifstream ofile_read(filename);
    // if (!ofile_read) {std::cerr << "This file can't be opened for reading" << std::endl;exit(1);}
    if (!ofile_read.good()){
        std::ofstream ofile_write(filename);
        std::cout<<"Creating file "<<filename<<"..."<<std::endl;
        if (!ofile_write) {std::cerr << "This file can't be opened for writing" << std::endl;exit(1);}
        ofile_write << "// "<<numInst;
        ofile_write << "\n#include <unordered_map>\nstd::unordered_map<std::string,unsigned int> Instructions_Dict = {\n";  // Write the entire modified content back to the file
        int n = 0;
        for(auto& line : lines){
            std::stringstream sstext;
            sstext << "\t{ \""<<lines[n]<<"\" , 0x"<<std::setw(2)<<std::setfill('0')<<std::hex<<n++<<"},"<<"\n";
            ofile_write << sstext.str();
            std::cout<<"Writting..."<<std::endl;
        }
        ofile_write<<"\t// Add other instructions here\n};";
        ofile_write.close();
    }else{
        std::stringstream existingContent;
        {std::string nullSTR = "";ofile_read >> nullSTR;}
        int lastpos;
        ofile_read >> lastpos;
        std::cout<<"old posotion is : "<<lastpos<<std::endl;
        existingContent << ofile_read.rdbuf();  // Read the entire file content
        ofile_read.close();

        std::string content = existingContent.str();

        // Prepare to insert the new instructions before the "else" statement
        int endPos = content.find("// Add other instructions here");
        if (endPos == std::string::npos) {std::cerr << "'else' not found in the file" << std::endl;exit(1);}
        std::stringstream newInstructions;
        for(int i = lastpos;i<numInst;i++){
            std::stringstream sstext;
            sstext << "\t{ \""<<lines[i]<<"\" , 0x"<<std::setw(2)<<std::setfill('0')<<std::hex<<i<<"},"<<"\n";
            std::string text = sstext.str();
            std::cout<<"Writting..."<<std::endl;
            newInstructions << sstext.str();
        }
        // Insert the new instructions before the "else"
        content.insert(endPos-1, newInstructions.str());
        
        std::ofstream ofile_write(filename);
        std::cout<<"Updating file "<<filename<<"..."<<std::endl;
        if (!ofile_write) {std::cerr << "This file can't be opened for writing" << std::endl;exit(1);}
        ofile_write << "// "<<numInst;
        ofile_write << content;  // Write the entire modified content back to the file
        ofile_write.close();
    }

}

int main(){
    std::ifstream ifile("Instructions.inst");
    if(!ifile){std::cerr<<"This file is not exist"<<std::endl;exit(1);}
    std::vector<std::string> lines;
    std::string line;

    while(std::getline(ifile,line)){
        lines.push_back(line);
    }
    ifile.close();

    Generate_Instruction_table("./Unicode_CU/instructions_tab.h", lines);
    Generate_Instruction_dictionary("./Assembler/instructions_dict.h",lines);
}
