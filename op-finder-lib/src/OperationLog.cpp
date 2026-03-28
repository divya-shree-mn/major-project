#include "OperationLog.hpp"
#include <utility>

nlohmann::json BasicOperation::toJson() const {
    nlohmann::json j;
    to_json(j, *this);
    return j;
}

nlohmann::json FunctionCall::toJson() const {
    nlohmann::json j;
    to_json(j, *this);
    return j;
}

nlohmann::json LoopInfo::toJson() const {
    nlohmann::json j;
    to_json(j, *this);
    return j;
}

nlohmann::json IfInfo::toJson() const {
    nlohmann::json j;
    to_json(j, *this);
    return j;
}

nlohmann::json ArraySubscriptInfo::toJson() const {
    nlohmann::json j;
    to_json(j, *this);
    return j;
}

void OperationLog::DecodeEntry(const nlohmann::json& j) {
    if (entry_type == BasicOperation::TYPE_NAME) {
        auto derived_entry = std::make_unique<BasicOperation>();
        j.get_to(*derived_entry);
        entry = std::move(derived_entry);
    } else if (entry_type == FunctionCall::TYPE_NAME) {
        auto derived_entry = std::make_unique<FunctionCall>();
        j.get_to(*derived_entry);
        entry = std::move(derived_entry);
    } else if (entry_type == LoopInfo::TYPE_NAME) {
        auto derived_entry = std::make_unique<LoopInfo>();
        j.get_to(*derived_entry);
        entry = std::move(derived_entry);
    } else if (entry_type == IfInfo::TYPE_NAME) {
        auto derived_entry = std::make_unique<IfInfo>();
        j.get_to(*derived_entry);
        entry = std::move(derived_entry);
    } else if (entry_type == ArraySubscriptInfo::TYPE_NAME) {
        auto derived_entry = std::make_unique<ArraySubscriptInfo>();
        j.get_to(*derived_entry);
        entry = std::move(derived_entry);
    } else {
        entry = nullptr;
    }
}