#include <variant>

#include "allocator.h"

struct Node_Number{
    std::string value;
};

struct Node_String{
    std::string value;
};

struct Node_Ident{
    std::string name;
};

struct Node_Stamt;
struct Node_Expression;

struct Node_CustomBlock{
    std::string value;
};

struct Node_BlockStamts{
    std::vector<Node_Stamt*> body;
};

struct Node_WhileStamt{
    Node_Expression* test;
    Node_BlockStamts* then;
};

struct Node_IfStamt{
    Node_Expression* test;
    Node_BlockStamts* then;
    Node_Stamt* alter;
};

struct Node_Return{
    Node_Expression* value;
};

struct Node_DeclareFunc{
    std::string name;
    std::vector<std::string> params;
    std::vector<Node_Stamt*> body;
};

struct Node_CallFunc{
    std::string name;
    std::vector<Node_Expression*> args;
};

struct Node_Bin{
    Node_Expression* left;
    std::string op;
    Node_Expression* right;
};

struct Node_Expression{
    std::variant<Node_Number, Node_String, Node_Ident,Node_Bin*, Node_CallFunc*> value;
};

struct Node_DeclareVar{
    std::string name;
    Node_Expression* expr;
};

struct Node_DeclareConst{
    std::string name;
    Node_Expression* expr;
};

struct Node_AssignVar{
    std::string name;
    Node_Expression* expr;
};

struct Node_Stamt{
    std::variant<Node_DeclareVar*,Node_AssignVar*,Node_CallFunc*,Node_DeclareFunc*,Node_Return*,Node_BlockStamts*,Node_IfStamt*,Node_WhileStamt*,Node_CustomBlock*,Node_DeclareConst*> value;
};

struct Node_Prog{
    std::vector<Node_Stamt*> stmts;
};

class Parser{
    private:
    std::vector<std::optional<Token_t>> m_tokens;
    int current;
    Allocator allocator;
    public:
    Parser(const std::vector<std::optional<Token_t>> tokens)
        : m_tokens(tokens), current(0), allocator(2 * 1024 * 1024)
    {}
    ~Parser(){}
    Node_Prog parseProgram(){
        std::vector<Node_Stamt*> body;
        while(current < m_tokens.size()){
            body.push_back(parseStatement());
        }
        return Node_Prog{
            body
        };
    }
    size_t getUsedMem(){
        return allocator.usedSize();
    }
private:
    std::optional<Token_t> peek(int offset = 0){
        // std::cout << "Vector size: " << m_tokens.size() << ", Index: " << current+offset << std::endl;
        if (current + offset < m_tokens.size()) {
            return m_tokens[current + offset];
        }
        return std::nullopt; // No more tokens
    }
    Token_t consume(const TokenType& type){
        std::optional<Token_t> token = peek();
        if(!token.has_value() || token.value().type != type){throw std::runtime_error("Parser Error: Expected token of type "+ TokenName[static_cast<int>(type)] +", but found "+ (token.has_value() ? TokenName[static_cast<int>(token.value().type)] : "EOF")); }
        current++;
        return token.value();
    }
    int getPrecedence(const std::string& op){
        if(op == "/" || op == "*") return 7;
        if(op == "-" || op == "+") return 6;
        if(op == "<") return 5;
        if(op == ">") return 4;
        if(op == "<=") return 3;
        if(op == ">=") return 2;
        if(op == "!=" || op == "==") return 1;
        return 0;
    }

    Node_Expression* parsePrimary(){
        std::optional<Token_t> token = peek();
        if(token.has_value() && token.value().type == TokenType::t_IntLiter){
            consume(TokenType::t_IntLiter);
            Node_Expression* node = allocator.alloc<Node_Expression>();
            new (node) Node_Expression{Node_Number{token.value().value}};
            return node;
        }
        if(token.has_value() && token.value().type == TokenType::t_StrLiter){
            consume(TokenType::t_StrLiter);
            Node_Expression* node = allocator.alloc<Node_Expression>();
            new (node) Node_Expression{Node_String{token.value().value}};
            return node;
        }
        if(token.has_value() && token.value().type == TokenType::t_Ident){
            Node_Expression* node = allocator.alloc<Node_Expression>();
            if(peek(1).has_value() && peek(1).value().type == TokenType::t_LeftPar){
                new (node) Node_Expression {parseCallFunc()};
            }else{
                consume(TokenType::t_Ident);
                new (node) Node_Expression {Node_Ident{token.value().value}};
            }
            return node;
        }
        if(token.has_value() && token.value().type == TokenType::t_LeftPar){
            consume(TokenType::t_LeftPar);
            Node_Expression* node = parseExpression();
            consume(TokenType::t_RightPar);
            return node;
        }
        
        throw std::runtime_error("Parser Error: Undefined token: '"+ TokenName[static_cast<int>(token.value().type)] +"'");
    }
    Node_Expression* parseExpression(int precedence = 0){
        std::optional<Token_t> token = peek();
        Node_Expression* left = parsePrimary();

        while(true){
            std::optional<Token_t> token = peek();
            if(!token.has_value() || token.value().type != TokenType::t_Operator) break;
            int currentPrecedence = getPrecedence(token.value().value);
            if (currentPrecedence <= precedence) break;
            std::string op = consume(TokenType::t_Operator).value;
            Node_Expression* right = parseExpression(currentPrecedence);
            Node_Bin* node = allocator.alloc<Node_Bin>();
            new (node) Node_Bin{
                left,
                op,
                right
            };
            left = allocator.alloc<Node_Expression>(); // Allocate a new Node_Expression for left
            new (left) Node_Expression{node};
        }
        return left;
    }
    Node_DeclareVar* parseVariableDec(){
        consume(TokenType::t_Let);
        std::string identifier = consume(TokenType::t_Ident).value;
        Node_Expression* value = NULL;
        if(peek().has_value() && peek().value().type != TokenType::t_Semi){
            consume(TokenType::t_Assign);
            value = parseExpression();
        }
        consume(TokenType::t_Semi);

        Node_DeclareVar* node = allocator.alloc<Node_DeclareVar>();
        new (node) Node_DeclareVar{
            identifier,
            value
        };

        return node;
    }
    Node_AssignVar* parseVariableAssign(){
        Token_t token = consume(TokenType::t_Ident);
        consume(TokenType::t_Assign);
        Node_Expression* value = parseExpression();
        consume(TokenType::t_Semi);

        Node_AssignVar* node = allocator.alloc<Node_AssignVar>();
        new (node) Node_AssignVar{token.value,value};
        return node;
    }
    Node_DeclareFunc* parseDeclareFunc(){
        consume(TokenType::t_Func);
        Token_t token = consume(TokenType::t_Ident);
        consume(TokenType::t_LeftPar);
        std::vector<std::string> params;
        if(peek().has_value() && peek().value().type != TokenType::t_RightPar){
            while(true){
                params.push_back(consume(TokenType::t_Ident).value);
                if(peek().has_value() && peek().value().type == TokenType::t_Comma) consume(TokenType::t_Comma);
                else break;
            }
        }
        consume(TokenType::t_RightPar);
        consume(TokenType::t_LeftBrack);
        std::vector<Node_Stamt*> body;
        while(peek().has_value() && peek().value().type != TokenType::t_RightBrack){
            body.push_back(parseStatement());
        }
        consume(TokenType::t_RightBrack);
        Node_DeclareFunc* node = allocator.alloc<Node_DeclareFunc>();
        new (node) Node_DeclareFunc{token.value,params,body};
        return node;
    }
    Node_CallFunc* parseCallFunc(){
        Token_t token = consume(TokenType::t_Ident);
        consume(TokenType::t_LeftPar);
        std::vector<Node_Expression*> args;
        if(peek().has_value() && peek().value().type != TokenType::t_RightPar){
            while(true){
                args.push_back(parseExpression());
                if(peek().has_value() && peek().value().type == TokenType::t_Comma) consume(TokenType::t_Comma);
                else break;
            }
        }
        consume(TokenType::t_RightPar);
        Node_CallFunc* node = allocator.alloc<Node_CallFunc>();
        new (node) Node_CallFunc{token.value,args};
        return node;
    }
    Node_BlockStamts* parseBlockStamts(){
        consume(TokenType::t_LeftBrack);
        std::vector<Node_Stamt*> body;
        while(peek().has_value() && peek().value().type != TokenType::t_RightBrack){
            body.push_back(parseStatement());
        }
        consume(TokenType::t_RightBrack);
        Node_BlockStamts* node = allocator.alloc<Node_BlockStamts>();
        new (node) Node_BlockStamts{body};
        return node;
    }
    Node_Return* parseReturn(){
        consume(TokenType::t_Return);
        Node_Expression* value = NULL;
        if(peek().has_value() && peek().value().type != TokenType::t_Semi){
            value = parseExpression();
        }
        consume(TokenType::t_Semi);
        Node_Return* node = allocator.alloc<Node_Return>();
        new (node) Node_Return{value};
        return node;
    }
    Node_IfStamt* parseIfStamt(){
        consume(TokenType::t_If);
        consume(TokenType::t_LeftPar);
        Node_Expression* test = parseExpression();
        consume(TokenType::t_RightPar);
        Node_BlockStamts* then = parseBlockStamts();
        Node_Stamt* alter = NULL;
        if(peek().has_value() && peek().value().type == TokenType::t_Else){
            consume(TokenType::t_Else);
            alter = parseStatement();
        }
        Node_IfStamt* node = allocator.alloc<Node_IfStamt>();
        new (node) Node_IfStamt{test,then,alter};
        return node;
    }
    Node_WhileStamt* parseWhileStamt(){
        consume(TokenType::t_While);
        consume(TokenType::t_LeftPar);
        Node_Expression* test = parseExpression();
        consume(TokenType::t_RightPar);
        Node_BlockStamts* body = parseBlockStamts();
        Node_WhileStamt* node = allocator.alloc<Node_WhileStamt>();
        new (node) Node_WhileStamt{test,body};
        return node;
    }
    Node_DeclareConst* parseConstantDec(){
        consume(TokenType::t_Const);
        std::string identifier = consume(TokenType::t_Ident).value;
        consume(TokenType::t_Assign);
        Node_Expression* value = parseExpression();
        consume(TokenType::t_Semi);

        Node_DeclareConst* node = allocator.alloc<Node_DeclareConst>();
        new (node) Node_DeclareConst{
            identifier,
            value
        };

        return node;
    }
    Node_CustomBlock* parseCustomBlock(){
        consume(TokenType::t_Percent);
        std::string inst = consume(TokenType::t_CustomInst).value;
        consume(TokenType::t_Percent);
    
        std::istringstream stream(inst);
        std::string line;
        std::vector<std::string> lines;

        while (std::getline(stream, line)) {
            line = trim(line); // Trim each line
            if (!line.empty()) lines.push_back(line); // Filter out empty lines
        }
        std::ostringstream result;
        for (size_t i = 0; i < lines.size(); ++i) {
            if (i > 0) result << "\n";
            result << lines[i];
        }
        Node_CustomBlock* node = allocator.alloc<Node_CustomBlock>();
        new (node) Node_CustomBlock{result.str()};
        return node;
    }
    Node_Stamt* parseStatement(){
        std::optional<Token_t> token = peek();
        Node_Stamt* stmt = allocator.alloc<Node_Stamt>();

        if(token.has_value() && token.value().type == TokenType::t_Let){
            new (stmt) Node_Stamt{parseVariableDec()};
            return stmt;
        }
        
        if(token.has_value() && token.value().type == TokenType::t_Ident){
            if(peek(1).has_value() && peek(1).value().type == TokenType::t_LeftPar){
                new (stmt) Node_Stamt{parseCallFunc()};
                consume(TokenType::t_Semi);
                return stmt;
            }
            new (stmt) Node_Stamt{parseVariableAssign()};
            return stmt;
        }
        
        if(token.has_value() && token.value().type == TokenType::t_Func){
            new (stmt) Node_Stamt{parseDeclareFunc()};
            return stmt;
        }

        if(token.has_value() && token.value().type == TokenType::t_Return){
            new (stmt) Node_Stamt{parseReturn()};
            return stmt;
        }

        if(token.has_value() && token.value().type == TokenType::t_LeftBrack){
            new (stmt) Node_Stamt{parseBlockStamts()};
            return stmt;
        }

        if(token.has_value() && token.value().type == TokenType::t_If){
            new (stmt) Node_Stamt{parseIfStamt()};
            return stmt;
        }

        if(token.has_value() && token.value().type == TokenType::t_While){
            new (stmt) Node_Stamt{parseWhileStamt()};
            return stmt;
        }

        if(token.has_value() && token.value().type == TokenType::t_Percent){
            new (stmt) Node_Stamt{parseCustomBlock()};
            return stmt;
        }

        if(token.has_value() && token.value().type == TokenType::t_Const){
            new (stmt) Node_Stamt{parseConstantDec()};
            return stmt;
        }

        throw std::runtime_error("Parser Error: Unexpected statement: '"+ TokenName[static_cast<int>(token.value().type)] +"'");
    }
    
};

// Function prototypes for recursive printing
void printNodeDeclareConst(const Node_DeclareConst* node, int indent = 0);
void printNodeBlockStamts(const Node_BlockStamts* node, int indent = 0);
void printNodeIfStamt(const Node_IfStamt* node, int indent = 0);
void printNodeWhileStamt(const Node_WhileStamt* node, int indent = 0);
void printNodeReturn(const Node_Return* node, int indent = 0);
void printNodeDeclareFunc(const Node_DeclareFunc* node, int indent = 0);
void printNodeCallFunc(const Node_CallFunc* node, int indent = 0);
void printNodeExpression(const Node_Expression* node, int indent = 0);
void printNodeAssignVar(const Node_AssignVar* node, int indent = 0);
void printNodeDeclareVar(const Node_DeclareVar* node, int indent = 0);
void printNodeStamt(const Node_Stamt* node, int indent = 0);
void printNodeProg(const Node_Prog& node, int indent = 0);

// Utility function to print indentation
void printIndent(int indent) {
    for (int i = 0; i < indent-1; ++i) {
        std::cout << "| ";
    }
    if(indent) std::cout << "+-";
}

// Print a Node_Number
void printNodeNumber(const Node_Number& node, int indent = 0) {
    printIndent(indent);
    std::cout << "Node_Number: " << node.value << std::endl;
}

// Print a Node_String
void printNodeString(const Node_String& node, int indent = 0) {
    printIndent(indent);
    std::cout << "Node_String: " << node.value << std::endl;
}

// Print a Node_Ident
void printNodeIdent(const Node_Ident& node, int indent = 0) {
    printIndent(indent);
    std::cout << "Node_Ident: " << node.name << std::endl;
}

// Print a Node_CustomBlock
void printNodeCustomBlock(const Node_CustomBlock* node, int indent = 0) {
    if(!node) return;

    printIndent(indent);
    std::cout << "Node_CustomBlock: " << std::endl;
    std::cout << "\"" << node->value << "\"" << std::endl;
}

// Print a Node_Bin
void printNodeBin(const Node_Bin* node, int indent = 0) {
    if (!node) return;

    printIndent(indent);
    std::cout << "Node_Bin:" << std::endl;
    printIndent(indent + 1);
    std::cout << "Operator: " << node->op << std::endl;

    printIndent(indent + 1);
    std::cout << "Left:" << std::endl;
    printNodeExpression(node->left, indent + 2);

    printIndent(indent + 1);
    std::cout << "Right:" << std::endl;
    printNodeExpression(node->right, indent + 2);
}

// Print a Node_Return
void printNodeReturn(const Node_Return* node, int indent){
    if(!node) return;

    printIndent(indent);
    std::cout << "Node_Return:" << std::endl;

    printIndent(indent + 1);
    std::cout << "Value:";
    if(!node->value) std::cout << " None";
    std::cout << std::endl;
    printNodeExpression(node->value,indent + 2);
}

// Print a Node_DeclareFunc
void printNodeDeclareFunc(const Node_DeclareFunc* node, int indent){
    if(!node) return;

    printIndent(indent);
    std::cout << "Node_DeclareFunc:" << std::endl;

    printIndent(indent + 1);
    std::cout << "Name: " << node->name << std::endl;

    printIndent(indent + 1);
    std::cout << "Params:";
    if(!node->params.size()) std::cout << " None";
    std::cout << std::endl;
    for(const auto& param : node->params){
        printIndent(indent + 2);
        std::cout << "param: " << param << std::endl;
    }

    printIndent(indent + 1);
    std::cout << "Body:";
    if(!node->body.size()) std::cout << " None";
    std::cout << std::endl;
    for(const auto& stmt : node->body){
        printNodeStamt(stmt,indent + 2);
    }
}

// Print a Node_CallFunc
void printNodeCallFunc(const Node_CallFunc* node, int indent){
    if(!node) return;

    printIndent(indent);
    std::cout << "Node_CallFunc:" << std::endl;

    printIndent(indent + 1);
    std::cout << "Name: " << node->name << std::endl;

    printIndent(indent + 1);
    std::cout << "Agrs:";
    if(!node->args.size()) std::cout << " None";
    std::cout << std::endl;
    for(const auto* arg : node->args){
        printNodeExpression(arg, indent + 2);
    }
}

// Print a Node_Expression
void printNodeExpression(const Node_Expression* node, int indent) {
    if (!node) return;

    printIndent(indent);
    std::cout << "Node_Expression:" << std::endl;

    std::visit([indent](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, Node_Number>) {
            printNodeNumber(value, indent + 1);
        } else if constexpr (std::is_same_v<T, Node_String>) {
            printNodeString(value, indent + 1);
        } else if constexpr (std::is_same_v<T, Node_Ident>) {
            printNodeIdent(value, indent + 1);
        } else if constexpr (std::is_same_v<T, Node_Bin*>) {
            printNodeBin(value, indent + 1);
        } else if constexpr (std::is_same_v<T, Node_CallFunc*>) {
            printNodeCallFunc(value, indent + 1);
        }
    }, node->value);
}



// Print a Node_DeclareConst
void printNodeDeclareConst(const Node_DeclareConst* node, int indent) {
    if (!node) return;

    printIndent(indent);
    std::cout << "Node_DeclareConst:" << std::endl;

    printIndent(indent + 1);
    std::cout << "Name: " << node->name << std::endl;

    printIndent(indent + 1);
    std::cout << "Value:";
    if(!node->expr) std::cout << " None";
    std::cout << std::endl;
    printNodeExpression(node->expr, indent + 2);
}

// Print a Node_DeclareVar
void printNodeDeclareVar(const Node_DeclareVar* node, int indent) {
    if (!node) return;

    printIndent(indent);
    std::cout << "Node_DeclareVar:" << std::endl;

    printIndent(indent + 1);
    std::cout << "Name: " << node->name << std::endl;

    printIndent(indent + 1);
    std::cout << "Value:";
    if(!node->expr) std::cout << " None";
    std::cout << std::endl;
    printNodeExpression(node->expr, indent + 2);
}

// Print a Node_AssignVar
void printNodeAssignVar(const Node_AssignVar* node, int indent){
    if (!node) return;

    printIndent(indent);
    std::cout << "Node_AssignVar:" << std::endl;

    printIndent(indent + 1);
    std::cout << "Name: " << node->name << std::endl;

    printIndent(indent + 1);
    std::cout << "Value: " << std::endl;
    printNodeExpression(node->expr, indent + 2);
}

// Print a Node_BlockStamts
void printNodeBlockStamts(const Node_BlockStamts* node, int indent){
    if(!node) return;

    printIndent(indent);
    std::cout << "Node_BlockStamts:" << std::endl;

    printIndent(indent + 1);
    std::cout << "Body:";
    if(!node->body.size()) std::cout << " None";
    std::cout << std::endl;
    for(const auto* stmt : node->body){
        printNodeStamt(stmt, indent + 2);
    }
}

// Print a Node_IfStamt
void printNodeIfStamt(const Node_IfStamt* node, int indent){
    if(!node) return;

    printIndent(indent);
    std::cout << "Node_IfStamt:" << std::endl;

    printIndent(indent + 1);
    std::cout << "Test:" << std::endl;
    printNodeExpression(node->test, indent + 2);

    printIndent(indent + 1);
    std::cout << "Then:" << std::endl;
    printNodeBlockStamts(node->then, indent + 2);

    printIndent(indent + 1);
    std::cout << "Alter:";
    if(!node->alter) std::cout << " None";
    std::cout << std::endl;
    printNodeStamt(node->alter, indent + 2);
}

// Print a Node_WhileStamt
void printNodeWhileStamt(const Node_WhileStamt* node, int indent){
    if(!node) return;

    printIndent(indent);
    std::cout << "Node_WhileStamt:" << std::endl;

    printIndent(indent + 1);
    std::cout << "Test:" << std::endl;
    printNodeExpression(node->test, indent + 2);

    printIndent(indent + 1);
    std::cout << "Body:" << std::endl;
    printNodeBlockStamts(node->then, indent + 2);
}

// Print a Node_Stamt
void printNodeStamt(const Node_Stamt* node, int indent) {
    if (!node) return;

    printIndent(indent);
    std::cout << "Node_Stamt:" << std::endl;

    std::visit([indent](const auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, Node_DeclareVar*>){
            printNodeDeclareVar(value, indent + 1);
        }else if constexpr (std::is_same_v<T, Node_AssignVar*>){
            printNodeAssignVar(value, indent + 1);
        }else if constexpr (std::is_same_v<T, Node_CallFunc*>){
            printNodeCallFunc(value, indent + 1);
        }else if constexpr (std::is_same_v<T, Node_DeclareFunc*>){
            printNodeDeclareFunc(value, indent + 1);
        }else if constexpr (std::is_same_v<T, Node_Return*>){
            printNodeReturn(value, indent + 1);
        }else if constexpr (std::is_same_v<T, Node_BlockStamts*>){
            printNodeBlockStamts(value, indent + 1);
        }else if constexpr (std::is_same_v<T, Node_IfStamt*>){
            printNodeIfStamt(value, indent + 1);
        }else if constexpr (std::is_same_v<T, Node_WhileStamt*>){
            printNodeWhileStamt(value, indent + 1);
        }else if constexpr (std::is_same_v<T, Node_CustomBlock*>){
            printNodeCustomBlock(value, indent + 1);
        }else if constexpr (std::is_same_v<T, Node_DeclareConst*>){
            printNodeDeclareConst(value, indent + 1);
        }
    }, node->value);
}

// Print a Node_Prog
void printNodeProg(const Node_Prog& node, int indent) {
    printIndent(indent);
    std::cout << "Node_Prog:" << std::endl;

    for (const auto* stmt : node.stmts) {
        printNodeStamt(stmt, indent + 1);
    }
}
