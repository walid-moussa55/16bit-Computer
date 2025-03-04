
class ScopeVariable{
public:
    int m_scope;
    std::unordered_map<std::string, size_t> m_table;
    ScopeVariable(size_t scope)
        :   m_scope(scope){}
};

class Generator{
private:
    Node_Prog m_ast;
    std::vector<std::string> m_opcodes;
    std::unordered_map<std::string, std::string> m_globalSymbolTable;
    std::unordered_map<std::string, std::string> m_string_Label;
    size_t m_labelCounter;
    std::unordered_map<std::string, std::vector<std::string>> m_funcPars;
    std::vector<ScopeVariable> m_scopeStack;
    size_t m_spOffset;
    size_t m_offsetforwaitting;
public:
    Generator(const Node_Prog& ast)
    :   m_ast(ast), m_opcodes({"jmp _start"}), m_labelCounter(0), m_spOffset(0), m_scopeStack({}), m_offsetforwaitting(0)
    {}
    std::string generateOpcodes(){
        pushScope();
        for(const auto* stmt : m_ast.stmts){
            if(std::holds_alternative<Node_DeclareVar*>(stmt->value)){
                std::string initialValue = "0";
                if(std::get<Node_DeclareVar*>(stmt->value)->expr && std::holds_alternative<Node_Number>(std::get<Node_DeclareVar*>(stmt->value)->expr->value)){
                    initialValue = std::get<Node_Number>(std::get<Node_DeclareVar*>(stmt->value)->expr->value).value;
                }else if(std::get<Node_DeclareVar*>(stmt->value)->expr && std::holds_alternative<Node_String>(std::get<Node_DeclareVar*>(stmt->value)->expr->value)){
                    std::string stringLabel = generateLabel("string");
                    std::string text = std::get<Node_String>(std::get<Node_DeclareVar*>(stmt->value)->expr->value).value;
                    std::cout<<"text : "<<text<<std::endl;
                    m_opcodes.push_back(stringLabel +":");
                    m_opcodes.push_back("\""+text+"$\"");
                    m_string_Label[text] = stringLabel;
                }
                declareGlobVar(std::get<Node_DeclareVar*>(stmt->value)->name, initialValue);
            }else if(std::holds_alternative<Node_DeclareConst*>(stmt->value)){
                std::string initialValue = "0";
                if(std::get<Node_DeclareConst*>(stmt->value)->expr && std::holds_alternative<Node_Number>(std::get<Node_DeclareConst*>(stmt->value)->expr->value)){
                    initialValue = std::get<Node_Number>(std::get<Node_DeclareConst*>(stmt->value)->expr->value).value;
                }else if(std::get<Node_DeclareConst*>(stmt->value)->expr && std::holds_alternative<Node_String>(std::get<Node_DeclareConst*>(stmt->value)->expr->value)){
                    std::string stringLabel = generateLabel("string");
                    std::string text = std::get<Node_String>(std::get<Node_DeclareConst*>(stmt->value)->expr->value).value;
                    std::cout<<"text : "<<text<<std::endl;
                    m_opcodes.push_back(stringLabel +":");
                    m_opcodes.push_back("\""+text+"$\"");
                    m_string_Label[text] = stringLabel;
                }
                declareGlobVar(std::get<Node_DeclareConst*>(stmt->value)->name, initialValue);
            }else if(std::holds_alternative<Node_DeclareFunc*>(stmt->value)){
                declareFunction(std::get<Node_DeclareFunc*>(stmt->value));
            }else if(std::holds_alternative<Node_CustomBlock*>(stmt->value)){
                m_opcodes.push_back("; custom");
                m_opcodes.push_back(std::get<Node_CustomBlock*>(stmt->value)->value);
            }
        }
        m_opcodes.push_back("_start:");
        for(const auto* stmt : m_ast.stmts){
            if(!std::holds_alternative<Node_DeclareFunc*>(stmt->value) && !std::holds_alternative<Node_CustomBlock*>(stmt->value)){
                visitNode(stmt);
            }
        }
        popScope();
        m_opcodes.push_back("hlt");
        int index = 0;
        std::stringstream fullOpcode;
        for(const auto& opcode : m_opcodes){
            if(index++) fullOpcode<<std::endl;
            fullOpcode<<opcode;
        }
        return fullOpcode.str();
    }
private:
    std::string generateLabel(const std::string& prefix) {
        std::stringstream label;
        label << prefix << "_" << m_scopeStack.size() << "_" << m_labelCounter++;
        return label.str();
    }
    void pushScope(){
        ScopeVariable newScope(m_scopeStack.size());
        m_scopeStack.push_back(newScope);
    }
    void popScope(){
        ScopeVariable currentScope = m_scopeStack.back();
        m_scopeStack.pop_back();
        size_t varsToPop = currentScope.m_table.size();
        for(int i = 0; i < varsToPop; i++){
            m_opcodes.push_back("pop");
        }
        m_spOffset -= varsToPop;
    }
    void declareVarInScope(const std::string& name, const std::string& initialValue = "0"){
        if(m_scopeStack.size() <= 1) throw std::runtime_error("Generator Error: No active scope to declare variable");
        ScopeVariable& currentScope = m_scopeStack.back();
        currentScope.m_table[name] = m_spOffset++;
        m_opcodes.push_back("ldva "+ initialValue);
        m_opcodes.push_back("pusha");
    }
    void accessVariable(const std::string& name){
        for(int i = m_scopeStack.size()-1; i >= 0; i--){
            ScopeVariable& scope = m_scopeStack.at(i);
            if(scope.m_table.find(name) != scope.m_table.end()){
                size_t offset = scope.m_table[name];
                m_opcodes.push_back("mspb");
                m_opcodes.push_back("ldva "+std::to_string(offset+m_offsetforwaitting));
                m_opcodes.push_back("addba");
                return;
            }
        }
        // If not found in scope stack, check global table
        if (m_globalSymbolTable.find(name) != m_globalSymbolTable.end()) {
            accessGlobVar(name);
        } else {
            std::string errorMsg = "Generator Error: Variable '" + name + "' is not declared in any active scope.";
            errorMsg += " Available variables in the current scope: ";
            for (const auto& var : m_scopeStack.back().m_table) {
                errorMsg += var.first + " ";
            }
            throw std::runtime_error(errorMsg);
        }
            // std::runtime_error("Generator Error: Variable '"+ name +"' not declared in any active scope.");
    }
    std::string declareGlobVar(const std::string& name, const std::string& initialValue = "0"){
        if (m_scopeStack.size() > 1) {
            throw std::runtime_error("Generator Error: Attempt to declare a global variable '" + name + "' in a non-global scope.");
        }
        std::string label = generateLabel(name);
        m_globalSymbolTable[name] = label;
        m_opcodes.push_back(label+":\n"+initialValue);
        return label;
    }
    void accessGlobVar(const std::string& name){
        if (m_globalSymbolTable.find(name) == m_globalSymbolTable.end()) {
            throw std::runtime_error("Generator Error: Global variable '" + name + "' not found.");
        }
        std::string label = m_globalSymbolTable.at(name);
        // if(label.empty()) throw std::runtime_error("Generator Error: Global variable '"+ name +"' not found.");
        m_opcodes.push_back("lda "+label);
    }
    void declareFunction(const Node_DeclareFunc* node){
        std::string funcLabel = generateLabel(node->name);
        m_globalSymbolTable[node->name] = funcLabel;

        std::vector<std::string> pars;
        for(const auto& param : node->params){
            pars.push_back(declareGlobVar(param));
        }
        m_funcPars[node->name] = pars;
        m_opcodes.push_back(funcLabel+":");
        pushScope();
        for(const auto* stmt : node->body){
            visitNode(stmt);
        }
        popScope();
        m_opcodes.push_back("ret");
    }
    void visitOp(const std::string& op){
        m_opcodes.push_back("cmpba");
        std::string yesL = generateLabel("yes");
        std::string endTest = generateLabel("endTest");
        if(op == "==") m_opcodes.push_back("je "+yesL);
        if(op == "!=") m_opcodes.push_back("jn "+yesL);
        if(op == "<") m_opcodes.push_back("jl "+yesL);
        if(op == ">") m_opcodes.push_back("jg "+yesL);
        if(op == "<=") {m_opcodes.push_back("jl "+yesL);m_opcodes.push_back("je "+yesL);}
        if(op == ">=") {m_opcodes.push_back("je "+yesL);m_opcodes.push_back("je "+yesL);}
        m_opcodes.push_back("ldva 0\njmp "+endTest+"\n"+yesL+":\nldva 1\n"+endTest+":");
    }
    void visitNode(const Node_Expression* node){
        std::visit([this](const auto& value){
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, Node_Number>){
                m_opcodes.push_back("; number");
                m_opcodes.push_back("ldva "+value.value);
            }
            else if constexpr (std::is_same_v<T, Node_String>){
                m_opcodes.push_back("; string");
                if(m_string_Label.find(value.value) == m_string_Label.end()){
                    throw std::runtime_error("error: can't found a label to this :"+ value.value);
                }
                std::string label = m_string_Label.at(value.value);
                m_opcodes.push_back("ldva "+ label);
            }
            else if constexpr (std::is_same_v<T, Node_Ident>){
                m_opcodes.push_back("; ident");
                if(m_globalSymbolTable.find(value.name) != m_globalSymbolTable.end()){
                    accessGlobVar(value.name);
                }else if(m_scopeStack.size() > 1){
                    accessVariable(value.name);
                    m_opcodes.push_back("ldba");
                }
            }
            else if constexpr (std::is_same_v<T, Node_Bin*>){
                m_opcodes.push_back("; binexp");
                m_offsetforwaitting++;
                visitNode(value->left);
                m_opcodes.push_back("pusha");
                visitNode(value->right);
                m_opcodes.push_back("popb");
                m_offsetforwaitting--;
                if(value->op == "+"){
                    m_opcodes.push_back("addba\nmvba");
                }else if(value->op == "-"){
                    m_opcodes.push_back("subba\nmvba");
                }else if(value->op == "*"){
                    m_opcodes.push_back("mulba\nmvba");
                }else if(value->op == "/"){
                    m_opcodes.push_back("divba\nmvba");
                }else if( value->op == "<" || value->op == ">" || value->op == "==" || value->op == "!=" || value->op == "<=" || value->op == ">=" ){
                    visitOp(value->op);
                }else throw std::runtime_error("Generator Error: Unsupported operator: "+ value->op);
            }
            else if constexpr (std::is_same_v<T, Node_CallFunc*>){
                m_opcodes.push_back("; funcall");
                if (m_funcPars.find(value->name) == m_funcPars.end()) {
                    throw std::runtime_error("Generator Error: Function '" + value->name + "' not found.");
                }
                auto paramLabels = m_funcPars.at(value->name);
                if(!paramLabels.size()) throw std::runtime_error("Generator Error: function '"+ value->name +"' is not defined or parameters are missing.");
                if(value->args.size() > paramLabels.size()){
                    throw std::runtime_error("Generator Error: Too many arguments provided for function "+value->name);
                }
                int index = 0;
                for(const auto* arg : value->args){
                    std::string paramLabel = paramLabels[index++];
                    if(std::holds_alternative<Node_Number>(arg->value)){
                        visitNode(arg);
                    }else{
                        visitNode(arg);
                    }
                    m_opcodes.push_back("sta "+ paramLabel);
                }
                if (m_globalSymbolTable.find(value->name) == m_globalSymbolTable.end()) {
                    throw std::runtime_error("Generator Error: Function '" + value->name + "' not declared.");
                }
                m_opcodes.push_back("call "+ m_globalSymbolTable.at(value->name));
            }else throw std::runtime_error("Generator Error: Unknown Expression node ");
        }, node->value);
    }
    void visitNode(const Node_BlockStamts* node){
        pushScope();
        for(const auto& stmt : node->body){
            visitNode(stmt);
        }
        popScope();
    }
    void visitNode(const Node_Stamt* node){
        std::visit([this](const auto& value){
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, Node_BlockStamts*>){
                pushScope();
                for(const auto* stmt : value->body){
                    visitNode(stmt);
                }
                popScope();
            }else if constexpr (std::is_same_v<T, Node_DeclareVar*>){
                m_opcodes.push_back("; vardecl");
                if(m_scopeStack.size() > 1){
                    if(value->expr && std::holds_alternative<Node_Number>(value->expr->value)){
                        std::string initialValue = std::get<Node_Number>(value->expr->value).value;
                        declareVarInScope(value->name, initialValue);
                    }else{
                        declareVarInScope(value->name);
                        Node_AssignVar* assNode = new Node_AssignVar{value->name, value->expr};
                        Node_Stamt* assignNode = new Node_Stamt{assNode};
                        visitNode(assignNode);
                        delete assignNode;
                        delete assNode;
                    }
                }else if(value->expr && !std::holds_alternative<Node_Number>(value->expr->value)){
                    Node_AssignVar* assNode = new Node_AssignVar{value->name, value->expr};
                    Node_Stamt* assignNode = new Node_Stamt{assNode};
                    visitNode(assignNode);
                    delete assignNode;
                    delete assNode;
                }
            }else if constexpr (std::is_same_v<T, Node_DeclareConst*>){
                m_opcodes.push_back("; constdecl");
                if(m_scopeStack.size() > 1){
                    if(value->expr && std::holds_alternative<Node_Number>(value->expr->value)){
                        std::string initialValue = std::get<Node_Number>(value->expr->value).value;
                        declareVarInScope(value->name, initialValue);
                    }else{
                        declareVarInScope(value->name);
                        Node_AssignVar* assNode = new Node_AssignVar{value->name, value->expr};
                        Node_Stamt* assignNode = new Node_Stamt{assNode};
                        visitNode(assignNode);
                        delete assignNode;
                        delete assNode;
                    }
                }else if(value->expr && !std::holds_alternative<Node_Number>(value->expr->value)){
                    Node_AssignVar* assNode = new Node_AssignVar{value->name, value->expr};
                    Node_Stamt* assignNode = new Node_Stamt{assNode};
                    visitNode(assignNode);
                    delete assignNode;
                    delete assNode;
                }
            }else if constexpr (std::is_same_v<T, Node_AssignVar*>){
                m_opcodes.push_back("; varassign");
                if(m_scopeStack.size() > 1 && m_globalSymbolTable.find(value->name) == m_globalSymbolTable.end()){
                    accessVariable(value->name);
                    m_opcodes.push_back("pushb");
                    visitNode(value->expr);
                    m_opcodes.push_back("popb\nstab");
                }else{
                    visitNode(value->expr);
                    m_opcodes.push_back("sta "+ m_globalSymbolTable.at(value->name));
                }
            }else if constexpr (std::is_same_v<T, Node_CallFunc*>){
                m_opcodes.push_back("; callfunc");
                auto paramLabels = m_funcPars.at(value->name);
                if(!paramLabels.size()) throw std::runtime_error("Generator Error: function '"+ value->name +"' is not defined or parameters are missing.");
                if(value->args.size() > paramLabels.size()){
                    throw std::runtime_error("Generator Error: Too many arguments provided for function "+value->name);
                }
                int index = 0;
                for(const auto* arg : value->args){
                    std::string paramLabel = paramLabels[index++];
                    if(std::holds_alternative<Node_Number>(arg->value)){
                        visitNode(arg);
                    }else{
                        visitNode(arg);
                    }
                    m_opcodes.push_back("sta "+ paramLabel);
                }
                m_opcodes.push_back("call "+ m_globalSymbolTable.at(value->name));
            }else if constexpr (std::is_same_v<T, Node_Return*>){
                m_opcodes.push_back("; return");
                visitNode(value->value);
            }else if constexpr (std::is_same_v<T, Node_CustomBlock*>){
                m_opcodes.push_back("; custom");
                m_opcodes.push_back(value->value);
            }else if constexpr (std::is_same_v<T, Node_IfStamt*>){
                m_opcodes.push_back("; if");
                visitNode(value->test);
                m_opcodes.push_back("testa");
                std::string elseLabel = generateLabel("else");
                std::string endLabel = generateLabel("endif");
                m_opcodes.push_back("jz "+ elseLabel);
                visitNode(value->then);
                m_opcodes.push_back("jmp "+ endLabel);
                m_opcodes.push_back(elseLabel+ ":");
                if(value->alter) visitNode(value->alter);
                m_opcodes.push_back(endLabel+ ":");
            }else if constexpr (std::is_same_v<T, Node_WhileStamt*>){
                m_opcodes.push_back("; While");
                std::string loop = generateLabel("loop");
                std::string endLoop = generateLabel("endloop");
                m_opcodes.push_back(loop+ ":");
                visitNode(value->test);
                m_opcodes.push_back("testa");
                m_opcodes.push_back("jz "+ endLoop);
                visitNode(value->then);
                m_opcodes.push_back("jmp "+ loop);
                m_opcodes.push_back(endLoop+ ":");
            }else throw std::runtime_error("Generator Error: Unknown Statment node ");
        },node->value);
    }

};