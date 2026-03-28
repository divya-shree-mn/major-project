#ifndef C_ANALYZER_OPERATIONLOG_HPP
#define C_ANALYZER_OPERATIONLOG_HPP

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <nlohmann/json.hpp>
#include <map>
#include <utility>
#include "clang/AST/Stmt.h"
#include "clang/AST/Expr.h"
#include "clang/Basic/SourceManager.h"

namespace clang {
    class Stmt;
    class Expr;
    class SourceManager;
    class BinaryOperator;
    class UnaryOperator;
    class IfStmt;
    class ArraySubscriptExpr;
}

struct IOperationLogEntry {
    virtual ~IOperationLogEntry() = default;
    [[nodiscard]] virtual nlohmann::json toJson() const = 0;
    [[nodiscard]] virtual std::string getTypeName() const = 0;
    [[nodiscard]] virtual const std::map<std::string, std::vector<std::string>>& getUserDefinedFunctionOperators() const = 0;
};

struct BasicOperation : public IOperationLogEntry {
    static constexpr char TYPE_NAME[] = "basic_operation";

    std::string opcode;
    std::string op_type_name;
    std::string operand_type;
    std::vector<std::string> operand_values;
    std::string type_result;
    
    [[nodiscard]] nlohmann::json toJson() const override;
    [[nodiscard]] std::string getTypeName() const override { return TYPE_NAME; }
    [[nodiscard]] const std::map<std::string, std::vector<std::string>>& getUserDefinedFunctionOperators() const override {
        static const std::map<std::string, std::vector<std::string>> empty_map;
        return empty_map;
    }
};

struct FunctionCall : public IOperationLogEntry {
    static constexpr char TYPE_NAME[] = "function_call";

    std::string function_name;
    std::string call_result_type;
    bool is_system_call = false;

    [[nodiscard]] nlohmann::json toJson() const override;
    [[nodiscard]] std::string getTypeName() const override { return TYPE_NAME; }
    [[nodiscard]] const std::map<std::string, std::vector<std::string>>& getUserDefinedFunctionOperators() const override {
        static const std::map<std::string, std::vector<std::string>> empty_map;
        return empty_map;
    }
};

struct LoopInfo : public IOperationLogEntry {
    static constexpr char TYPE_NAME[] = "loop_info";

    std::string loop_type;
    std::string initialization_text;
    std::string stop_condition_text;
    std::string increment_decrement_text;

    long long initial_value = 0;
    bool initial_value_resolved = false;
    long long stop_value = 0;
    bool stop_value_resolved = false;
    std::string comparison_operator = "";
    long long increment_value = 0;
    bool increment_value_resolved = false;
    long long iteration_count = -1;
    bool iteration_count_resolved = false;

    [[nodiscard]] nlohmann::json toJson() const override;
    [[nodiscard]] std::string getTypeName() const override { return TYPE_NAME; }
    [[nodiscard]] const std::map<std::string, std::vector<std::string>>& getUserDefinedFunctionOperators() const override {
        static const std::map<std::string, std::vector<std::string>> empty_map;
        return empty_map;
    }
};

struct IfInfo : public IOperationLogEntry {
    static constexpr char TYPE_NAME[] = "if_info";

    std::string condition;
    bool has_else_branch = false;

    [[nodiscard]] nlohmann::json toJson() const override;
    [[nodiscard]] std::string getTypeName() const override { return TYPE_NAME; }
    [[nodiscard]] const std::map<std::string, std::vector<std::string>>& getUserDefinedFunctionOperators() const override {
        static const std::map<std::string, std::vector<std::string>> empty_map;
        return empty_map;
    }
};

struct ArraySubscriptInfo : public IOperationLogEntry {
    static constexpr char TYPE_NAME[] = "array_subscript_info";

    std::string array_name;
    std::string index_text;

    [[nodiscard]] nlohmann::json toJson() const override;
    [[nodiscard]] std::string getTypeName() const override { return TYPE_NAME; }
    [[nodiscard]] const std::map<std::string, std::vector<std::string>>& getUserDefinedFunctionOperators() const override {
        static const std::map<std::string, std::vector<std::string>> empty_map;
        return empty_map;
    }
};

struct OperationLog {
    unsigned int line = 0;
    int branch_number = 0;
    unsigned int column = 0;

    std::string entry_type;
    std::unique_ptr<IOperationLogEntry> entry;

    OperationLog() = default;
    OperationLog(unsigned int line, unsigned int column, const std::string& entry_type_str, std::unique_ptr<IOperationLogEntry> entry_ptr)
        : line(line), column(column), entry_type(entry_type_str), entry(std::move(entry_ptr)) {}

    OperationLog(OperationLog&&) = default;
    OperationLog& operator=(OperationLog&&) = default;
    OperationLog(const OperationLog&) = delete;
    OperationLog& operator=(const OperationLog&) = delete;

    void DecodeEntry(const nlohmann::json& j);
};

class IOperationOutput {
public:
    virtual ~IOperationOutput() = default;
    virtual void pushOperation(const std::string& filename, OperationLog&& op) = 0;

    virtual void addLoop(const clang::Stmt* loop_stmt,
                         const std::string& loop_type,
                         const std::string& initialization_text,
                         const std::string& stop_condition_text,
                         const std::string& increment_decrement_text,
                         long long initial_value,
                         bool initial_value_resolved,
                         long long stop_value,
                         bool stop_value_resolved,
                         const std::string& comparison_operator,
                         long long increment_value,
                         bool increment_value_resolved,
                         long long iteration_count,
                         bool iteration_count_resolved,
                         const clang::SourceManager& source_manager) = 0;

    virtual void addFunctionCall(const clang::Expr* call_expr,
                                 const std::string& function_name,
                                 const std::string& call_result_type,
                                 bool is_system_call,
                                 const clang::SourceManager& source_manager) = 0;
    
    virtual void recordUserDefinedFunctionOperator(const std::string& function_name, const std::string& op_name) = 0;
    [[nodiscard]] virtual const std::map<std::string, std::vector<std::string>>& getUserDefinedFunctionOperators() const = 0;

    virtual void addBinaryOperator(const clang::BinaryOperator* bin_op, const std::string& op_code, const std::string& op_type_name, long long multiplier, const clang::SourceManager& source_manager) = 0;
    virtual void addUnaryOperator(const clang::UnaryOperator* un_op, const std::string& op_code, const std::string& op_type_name, long long multiplier, const clang::SourceManager& source_manager) = 0;
    virtual void addIfStatement(const clang::IfStmt* if_stmt, const std::string& condition_text, const clang::SourceManager& source_manager) = 0;
    virtual void addArraySubscript(const clang::ArraySubscriptExpr* subscript_expr, const std::string& array_name, const clang::SourceManager& source_manager) = 0;
};

inline void to_json(nlohmann::json& j, const IfInfo& ii) {
    j = nlohmann::json{
        {"condition", ii.condition},
        {"has_else_branch", ii.has_else_branch}
    };
}

inline void from_json(const nlohmann::json& j, IfInfo& ii) {
    j.at("condition").get_to(ii.condition);
    j.at("has_else_branch").get_to(ii.has_else_branch);
}

inline void to_json(nlohmann::json& j, const ArraySubscriptInfo& asi) {
    j = nlohmann::json{
        {"array_name", asi.array_name},
        {"index_text", asi.index_text}
    };
}

inline void from_json(const nlohmann::json& j, ArraySubscriptInfo& asi) {
    j.at("array_name").get_to(asi.array_name);
    j.at("index_text").get_to(asi.index_text);
}

inline void to_json(nlohmann::json& j, const OperationLog& l) {
    j = nlohmann::json{
        {"line", l.line},
        {"column", l.column},
        {"entry_type", l.entry_type},
        {"entry", l.entry->toJson()},
        {"branch_number", l.branch_number}
    };
}

inline void from_json(const nlohmann::json& j, OperationLog& l) {
    j.at("line").get_to(l.line);
    j.at("column").get_to(l.column);
    j.at("entry_type").get_to(l.entry_type);
    l.DecodeEntry(j["entry"]);
    j.at("branch_number").get_to(l.branch_number);
}

inline void to_json(nlohmann::json& j, const BasicOperation& bo) {
    j = nlohmann::json{
        {"opcode", bo.opcode},
        {"op_type_name", bo.op_type_name},
        {"operand_type", bo.operand_type},
        {"operand_values", bo.operand_values},
        {"type_result", bo.type_result}
    };
}

inline void from_json(const nlohmann::json& j, BasicOperation& bo) {
    j.at("opcode").get_to(bo.opcode);
    j.at("op_type_name").get_to(bo.op_type_name);
    j.at("operand_type").get_to(bo.operand_type);
    j.at("operand_values").get_to(bo.operand_values);
    j.at("type_result").get_to(bo.type_result);
}

inline void to_json(nlohmann::json& j, const FunctionCall& fcall) {
    j = nlohmann::json{
        {"function_name", fcall.function_name},
        {"call_result_type", fcall.call_result_type},
        {"is_system_call", fcall.is_system_call}
    };
}

inline void from_json(const nlohmann::json& j, FunctionCall& fcall) {
    j.at("function_name").get_to(fcall.function_name);
    j.at("call_result_type").get_to(fcall.call_result_type);
    j.at("is_system_call").get_to(fcall.is_system_call);
}

inline void to_json(nlohmann::json& j, const LoopInfo& li) {
    j = nlohmann::json{
        {"loop_type", li.loop_type},
        {"initialization_text", li.initialization_text},
        {"stop_condition_text", li.stop_condition_text},
        {"increment_decrement_text", li.increment_decrement_text},
        {"initial_value", li.initial_value},
        {"initial_value_resolved", li.initial_value_resolved},
        {"stop_value", li.stop_value},
        {"stop_value_resolved", li.stop_value_resolved},
        {"comparison_operator", li.comparison_operator},
        {"increment_value", li.increment_value},
        {"increment_value_resolved", li.increment_value_resolved},
        {"iteration_count", li.iteration_count},
        {"iteration_count_resolved", li.iteration_count_resolved}
    };
}

inline void from_json(const nlohmann::json& j, LoopInfo& li) {
    j.at("loop_type").get_to(li.loop_type);
    j.at("initialization_text").get_to(li.initialization_text);
    j.at("stop_condition_text").get_to(li.stop_condition_text);
    j.at("increment_decrement_text").get_to(li.increment_decrement_text);
    j.at("initial_value").get_to(li.initial_value);
    j.at("initial_value_resolved").get_to(li.initial_value_resolved);
    j.at("stop_value").get_to(li.stop_value);
    j.at("stop_value_resolved").get_to(li.stop_value_resolved);
    j.at("comparison_operator").get_to(li.comparison_operator);
    j.at("increment_value").get_to(li.increment_value);
    j.at("increment_value_resolved").get_to(li.increment_value_resolved);
    j.at("iteration_count").get_to(li.iteration_count);
    j.at("iteration_count_resolved").get_to(li.iteration_count_resolved);
}

#endif // C_ANALYZER_OPERATIONLOG_HPP
