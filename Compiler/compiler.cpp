#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <optional>
#include <unordered_set>
#include <cstring>
#include <filesystem>
#include <cassert>


std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    size_t last = str.find_last_not_of(" \t");
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

enum TokenType{
    t_IntLiter,t_Let,t_Ident,t_Assign,t_Semi,t_Func,t_Operator,t_LeftPar,t_RightPar,t_Comma,t_LeftBrack,t_RightBrack,t_Return,t_If,t_Else,t_While,t_Percent,t_CustomInst,t_StrLiter,t_Const, t_LeftSqrBrack, t_RightSqrBrack
};
const std::string TokenName[] = {"IntLiter","Let","Ident","Assign","Semi","Func","Operator","LeftPar","RigthPar","Comma","LeftBrack","RightBrack","Return","If","Else","While","Percent","CustomInst","StrLiter","Const", "LeftSqrBrack", "RightSqrBrack"};
struct Token_t{
    TokenType type;
    std::string value;
};

// Function to read file content
std::string readFile(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) throw std::runtime_error("PreProcessing Error: Cannot open file: " + fileName);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void preprocessFile(const std::string& filename, std::ofstream& tempFile, std::unordered_set<std::string>& includedFiles) {
    std::ifstream file(filename);
    if (!file.is_open()) throw std::runtime_error("Error opening file: " + filename);
    std::string line;
    while (std::getline(file, line)) {
        std::string trimmed = trim(line);
        if (trimmed.rfind("import", 0) == 0) {
            size_t start = trimmed.find("\"") + 1;
            size_t end = trimmed.rfind("\"");
            if (start != std::string::npos && end != std::string::npos && end > start) {
                std::string importedFile = trimmed.substr(start, end - start);
                if (includedFiles.find(importedFile) == includedFiles.end()) {
                    includedFiles.insert(importedFile);
                    // std::string fullpath = std::filesystem::current_path().string() + "\\" + importedFile;
                    std::filesystem::path fullpath = std::filesystem::current_path() / importedFile;
                    tempFile << "// Importing \"" << importedFile << "\"\n";
                    preprocessFile(fullpath.string(), tempFile, includedFiles);
                }
            }
        } else {
            tempFile << line << std::endl;
        }
    }
}

void preprocess(const std::string& inputFile, const std::string& outputFile) {
    std::unordered_set<std::string> includedFiles;
    std::ofstream tempFile(outputFile);
    preprocessFile(inputFile, tempFile, includedFiles);
    std::cout << "Preprocessing completed. Output written to " << outputFile << std::endl;
}

#include "parser.h"
#include "semantic.h"
#include "generator.h"

std::optional<Token_t> getKeyword(const std::string& text){
    if(text == "let"){ return Token_t{TokenType::t_Let,text}; }
    else if(text == "function"){ return Token_t{TokenType::t_Func,text}; }
    else if(text == "return"){ return Token_t{TokenType::t_Return,text}; }
    else if(text == "if"){ return Token_t{TokenType::t_If,text}; }
    else if(text == "else"){ return Token_t{TokenType::t_Else,text}; }
    else if(text == "while"){ return Token_t{TokenType::t_While,text}; }
    else if(text == "const"){ return Token_t{TokenType::t_Const,text}; }
    else{ return std::nullopt; }
}
std::vector<std::optional<Token_t>> tokenize(const std::string& filename){
    std::ifstream ifile(filename);
    if(!ifile){throw std::runtime_error("This file '"+ filename +"' not found");}
    std::stringstream content_str;
    content_str << ifile.rdbuf();
    std::string content = content_str.str();
    ifile.close();
    char c;
    std::vector<std::optional<Token_t>> tokens;
    int i=0;
    while(i<content.length()){
        c = content[i];
        if(c == ' ' || c == '\n') {i++;continue;}
        else if(c == '/' && i+1 < content.length() && content[i+1] == '/'){
            i += 2;
            while(i<content.length()){
                if(content[i] == '\n') break;
                i++;
            }
        }else if(c == '/' && i+1 < content.length() && content[i+1] == '*'){
            i += 2;
            while(i<content.length()){
                if(content[i] == '*' && i+1 < content.length() && content[i+1] == '/'){ i += 2; break; }
                i++;
            }
        }
        else if(c == '+' || c == '-'|| c == '*'|| c == '/'){std::string ss(1, c);tokens.push_back(Token_t{TokenType::t_Operator,ss});}
        else if(c == '<' || c == '>'|| c == '!'){
            std::string ss(1, c);
            c = content[i+1];
            if(c == '='){
                ss.push_back(c);
                tokens.push_back(Token_t{TokenType::t_Operator,ss});
                i++;
            }else{
                tokens.push_back(Token_t{TokenType::t_Operator,ss});
            }
        }
        else if(c == ';'){std::string ss(1, c);tokens.push_back(Token_t{TokenType::t_Semi,ss});}
        else if(c == '='){
            std::string ss(1, c);
            c = content[i+1];
            if(c == '='){
                ss.push_back(c);
                tokens.push_back(Token_t{TokenType::t_Operator,ss});
                i++;
            }else{
                tokens.push_back(Token_t{TokenType::t_Assign,ss});
            }
        }
        else if(c == '('){std::string ss(1, c);tokens.push_back(Token_t{TokenType::t_LeftPar,ss});}
        else if(c == ')'){std::string ss(1, c);tokens.push_back(Token_t{TokenType::t_RightPar,ss});}
        else if(c == ','){std::string ss(1, c);tokens.push_back(Token_t{TokenType::t_Comma,ss});}
        else if(c == '{'){std::string ss(1, c);tokens.push_back(Token_t{TokenType::t_LeftBrack,ss});}
        else if(c == '}'){std::string ss(1, c);tokens.push_back(Token_t{TokenType::t_RightBrack,ss});}
        else if(c == '['){std::string ss(1, c);tokens.push_back(Token_t{TokenType::t_LeftSqrBrack,ss});}
        else if(c == ']'){std::string ss(1, c);tokens.push_back(Token_t{TokenType::t_RightSqrBrack,ss});}
        else if(c == '%'){
            std::string ss(1, c);tokens.push_back(Token_t{TokenType::t_Percent,ss});
            std::string value;
            c = content[++i];
            while(c && c != '%'){
                value.push_back(c);
                c = content[++i];
            }
            tokens.push_back(Token_t{TokenType::t_CustomInst,value});
            if(c) tokens.push_back(Token_t{TokenType::t_Percent,ss});
        }
        else if(c == '"'){
            std::string value;
            c = content[++i];
            while(c && c != '"'){
                value.push_back(c);
                c = content[++i];
            }
            tokens.push_back(Token_t{TokenType::t_StrLiter,value});
        }
        else if(isdigit(c)){
            std::string ss;
            ss.push_back(c);
            c = content[++i];
            while(isdigit(c)){ss.push_back(c);c = content[++i];}
            tokens.push_back(Token_t{TokenType::t_IntLiter, ss});
            continue;
        }
        else if(isalpha(c) || c == '_'){
            std::string ss;
            ss.push_back(c);
            c = content[++i];
            while(isalnum(c) || c == '_'){ss.push_back(c);c = content[++i];}
            std::optional<Token_t> token = getKeyword(ss);
            if(token.has_value()){tokens.push_back(token);}
            else{tokens.push_back(Token_t{TokenType::t_Ident,ss});}
            continue;
        }
        else {std::string ss(1, c);throw std::runtime_error("Tokenize Error: Undefined character '"+ ss +"'");}
        i++;
    }
    return tokens;
}


int main(int argc, char* argv[]){
    std::string fileName = "program.tom";
    std::string outFileName = "program.ass";
    bool isdebug = false;
    if(argc == 2){
        if (!strcmp(argv[1] , "--debug")) isdebug = true;
        else fileName = argv[argc - 1];
    }
    if(argc == 3){
        if (!strcmp(argv[1] , "--debug")) isdebug = true;
        else if (!strcmp(argv[2] , "--debug")){
            isdebug = true;
            fileName = argv[argc-1];
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
    std::vector<std::optional<Token_t>> tokens;
    std::string tempFile = "temp.tom";
    try{
        preprocess(fileName, tempFile);
        tokens = tokenize(tempFile);
        if(isdebug) {for(const auto& token : tokens){std::cout<<"Token type : "<<TokenName[static_cast<int>(token.value().type)]<<" , Value : "<<token.value().value<<std::endl;}}
        Parser parser(tokens);
        Node_Prog prog = parser.parseProgram();
        if(isdebug) printNodeProg(prog);
        std::cout << "Allocator Used memory: " << parser.getUsedMem() << std::endl;
        
        prog = semanticAnalyze(&prog);
        if(isdebug) printNodeProg(prog);
        std::cout << "Semantic analysis completed successfully." << std::endl;

        Generator generator(prog);
        std::string opcode = generator.generateOpcodes();
        std::cout<<"Generating Assembly Opcode"<<std::endl;
        if(isdebug) std::cout<<opcode<<std::endl;

        std::ofstream ofile(outFileName);
        ofile << opcode << std::endl;
        ofile.close();

    } catch (const std::exception& e) {std::cerr << "An error occurred: " << e.what() << std::endl;remove(tempFile.c_str());return 1;}
    remove(tempFile.c_str());
}