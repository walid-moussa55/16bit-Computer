#include <unordered_map>

struct SymboleInfo_t{
    std::string type;
    std::variant<std::string, std::vector<std::string>> info;
};

class SymboleTable {
private:
    std::unordered_map<std::string,SymboleInfo_t> m_table;
public:
    SymboleTable* m_parent;
    SymboleTable()
        // : m_parent(nullptr)
    {
        m_parent = nullptr;
    }
    SymboleTable(SymboleTable* parentScope)
        // :   m_parent(parentScope)
    {
        m_parent = parentScope;
    }
    void define(const std::string& name, const SymboleInfo_t& info) {
        // Check if the variable is already defined in the current scope
        if (m_table.find(name) != m_table.end()) {
            throw std::runtime_error("Semantic Error: '" + name + "' is already defined in this scope.");
        }

        // Check in the parent scopes
        SymboleTable* currentScope = this;
        while (currentScope) {
            if (currentScope->m_table.find(name) != currentScope->m_table.end()) {
                throw std::runtime_error("Semantic Error: '" + name + "' is already defined in a parent scope.");
            }
            currentScope = currentScope->m_parent;
        }

        // If not defined anywhere, add it to the current scope
        m_table[name] = info;
    }

    const SymboleInfo_t* resolve(const std::string& name) const {
        if(m_table.find(name) != m_table.end()){
            return &m_table.at(name);
        }else if(m_parent){
            return m_parent->resolve(name);
        }else{
            throw std::runtime_error("Semantic Error: '" + name + "' is not defined.");
        }
    }
};

std::variant<Node_Number,Node_Bin*> constantFolding(Node_Bin* node){
    if(node->op == "/" && std::holds_alternative<Node_Number>(node->right->value) && std::get<Node_Number>(node->right->value).value == "0"){
        throw std::runtime_error("Semantic Error: Division by zero.");
    }
    if(std::holds_alternative<Node_Number>(node->left->value) && std::holds_alternative<Node_Number>(node->right->value)){
        int leftValue = std::atoi(std::get<Node_Number>(node->left->value).value.c_str());
        int rightValue = std::atoi(std::get<Node_Number>(node->right->value).value.c_str());
        std::stringstream result;
        if(node->op == "+"){int res = leftValue + rightValue;result<<res;return Node_Number{result.str()};}
        if(node->op == "-"){int res = leftValue - rightValue;result<<res;return Node_Number{result.str()};}
        if(node->op == "*"){int res = leftValue * rightValue;result<<res;return Node_Number{result.str()};}
        if(node->op == "/"){int res = leftValue / rightValue;result<<res;return Node_Number{result.str()};}
        if(node->op == "=="){int res = leftValue == rightValue;result<<res;return Node_Number{result.str()};}
        if(node->op == "!="){int res = leftValue != rightValue;result<<res;return Node_Number{result.str()};}
        if(node->op == "<"){int res = leftValue < rightValue;result<<res;return Node_Number{result.str()};}
        if(node->op == ">"){int res = leftValue > rightValue;result<<res;return Node_Number{result.str()};}
        if(node->op == "<="){int res = leftValue <= rightValue;result<<res;return Node_Number{result.str()};}
        if(node->op == ">="){int res = leftValue >= rightValue;result<<res;return Node_Number{result.str()};}
    }
    return node;
}

Node_Stamt* visitNode(Node_Stamt* node,SymboleTable& scope);

Node_Expression* visitNode(Node_Expression* node,const SymboleTable& scope){
    std::visit([scope,&node](auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, Node_Number>) {
            
        }
        else if constexpr (std::is_same_v<T, Node_String>){

        }
        else if constexpr (std::is_same_v<T, Node_Ident>){
            const SymboleInfo_t* resolvedSymbol = scope.resolve(value.name);  // Resolve the symbol

            // Example check: if the symbol is a variable
            if (resolvedSymbol->type != "variable" && resolvedSymbol->type != "constant") {
                throw std::runtime_error("Semantic Error: Expected type 'variable' for symbol '" + value.name + "', but found '" + resolvedSymbol->type + "'.");
            }

        }
        else if constexpr (std::is_same_v<T, Node_CallFunc*>){
            const SymboleInfo_t* func = scope.resolve(value->name);
            if(func->type != "function"){
                throw std::runtime_error("Semantic Error: '" + value->name + "' is not a function.");
            }
            auto expectedArgs = std::get<std::vector<std::string>>(func->info);
            if(expectedArgs.size() != value->args.size()){
                throw std::runtime_error("Semantic Error: Function '" + value->name + "' expects " + std::to_string(expectedArgs.size()) +" arguments but got " + std::to_string(value->args.size()) + ".");
            }
            for(auto*& arg : value->args){
                arg = visitNode(arg, scope);
            }
        }
        else if constexpr (std::is_same_v<T, Node_Bin*>){
            value->left = visitNode(value->left, scope);
            value->right = visitNode(value->right, scope);
            auto folded = constantFolding(value);
            if (std::holds_alternative<Node_Number>(folded)) {
                // Replace binary node's value with the folded constant
                node = new Node_Expression{std::get<Node_Number>(folded)};
            }
        }
        
    }, node->value);
    return node;
}

Node_BlockStamts* visitNode(Node_BlockStamts* node, SymboleTable& scope){
    for(auto*& stmt : node->body){
        stmt = visitNode(stmt, scope);
    }
    return node;
}

Node_Stamt* visitNode(Node_Stamt* node,SymboleTable& scope){
    std::visit([&scope](auto* value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, Node_DeclareVar*>){
            scope.define(value->name, {"variable"});
            value->expr = visitNode(value->expr, scope);
        }
        else if constexpr (std::is_same_v<T, Node_DeclareConst*>){
            scope.define(value->name, {"constant"});
            value->expr = visitNode(value->expr, scope);
        }
        else if constexpr (std::is_same_v<T, Node_AssignVar*>){
            const SymboleInfo_t* variable = scope.resolve(value->name);
            if(variable->type == "constant"){
                throw std::runtime_error("Semantic Error: '"+ value->name +"' is a constant cannot be modified.");
            }
            else if(variable->type != "variable" && variable->type != "constant"){
                throw std::runtime_error("Semantic Error: '"+ value->name +"' is not a variable.");
            }
            value->expr = visitNode(value->expr, scope);
        }
        else if constexpr (std::is_same_v<T, Node_DeclareFunc*>){
            scope.define(value->name, {"function", value->params});
            SymboleTable functionscope(&const_cast<SymboleTable&>(scope));
            for(const auto& param : value->params){
                functionscope.define(param, {"variable"});
            }
            for(auto*& stmt : value->body){
                stmt = visitNode(stmt, functionscope);
            }
        }
        else if constexpr (std::is_same_v<T, Node_BlockStamts*>){
            SymboleTable blockScope(&const_cast<SymboleTable&>(scope));
            for(auto*& stmt : value->body){
                stmt = visitNode(stmt, blockScope);
            }
        }
        else if constexpr (std::is_same_v<T, Node_CallFunc*>){
            const SymboleInfo_t* func = scope.resolve(value->name);
            if(func->type != "function"){
                throw std::runtime_error("Semantic Error: '" + value->name + "' is not a function.");
            }
            auto expectedArgs = std::get<std::vector<std::string>>(func->info);
            if(expectedArgs.size() != value->args.size()){
                throw std::runtime_error("Semantic Error: Function '" + value->name + "' expects " + std::to_string(expectedArgs.size()) +" arguments but got " + std::to_string(value->args.size()) + ".");
            }
            for(auto*& arg : value->args){
                arg = visitNode(arg, scope);
            }
        }
        else if constexpr (std::is_same_v<T, Node_Return*>){
            value->value = visitNode(value->value, scope);
        }
        else if constexpr (std::is_same_v<T, Node_IfStamt*>){
            value->test = visitNode(value->test, scope);
            SymboleTable ifScope(&const_cast<SymboleTable&>(scope));
            value->then = visitNode(value->then, ifScope);
            SymboleTable elseScope(&const_cast<SymboleTable&>(scope));
            if (value->alter) value->alter = visitNode(value->alter, elseScope);
        }
        else if constexpr (std::is_same_v<T, Node_WhileStamt*>){
            value->test = visitNode(value->test, scope);
            SymboleTable whileScope(&const_cast<SymboleTable&>(scope));
            value->then = visitNode(value->then, whileScope);
        }
        
    }, node->value);
    return node;
}

Node_Prog semanticAnalyze(Node_Prog* ast){
    SymboleTable globalScope;

    for (auto*& stmt : ast->stmts){
        stmt = visitNode(stmt, globalScope);
    }

    return *ast;
}