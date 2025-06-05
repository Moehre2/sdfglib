#include "sdfg/structured_control_flow/einsum.h"

#include <symengine/basic-inl.h>
#include <symengine/symbol.h>
#include <symengine/symengine_rcp.h>

#include <utility>
#include <vector>

#include "sdfg/element.h"
#include "sdfg/exceptions.h"
#include "sdfg/structured_control_flow/control_flow_node.h"
#include "sdfg/symbolic/symbolic.h"

namespace sdfg {
namespace structured_control_flow {

Einsum::Einsum(const DebugInfo& debug_info, std::vector<std::string> in_containers,
               std::string out_container,
               std::vector<std::pair<symbolic::Symbol, symbolic::Expression>> loops,
               std::vector<std::vector<std::string>> in_indices,
               std::vector<std::string> out_indices)
    : ControlFlowNode(debug_info),
      in_containers_(in_containers),
      out_container_(out_container),
      loops_(loops),
      in_indices_(in_indices),
      out_indices_(out_indices) {};

const std::vector<std::string>& Einsum::in_containers() const { return this->in_containers_; };

std::vector<std::string>& Einsum::in_containers() { return this->in_containers_; };

const std::string& Einsum::out_container() const { return this->out_container_; };

std::string& Einsum::out_container() { return this->out_container_; };

const std::vector<std::pair<symbolic::Symbol, symbolic::Expression>>& Einsum::loops() const {
    return this->loops_;
};

std::vector<std::pair<symbolic::Symbol, symbolic::Expression>>& Einsum::loops() {
    return this->loops_;
};

const std::vector<std::vector<std::string>>& Einsum::in_indices() const {
    return this->in_indices_;
};

std::vector<std::vector<std::string>>& Einsum::in_indices() { return this->in_indices_; };

const std::vector<std::string>& Einsum::out_indices() const { return this->out_indices_; };

std::vector<std::string>& Einsum::out_indices() { return this->out_indices_; };

void Einsum::replace(const symbolic::Expression& old_expression,
                     const symbolic::Expression& new_expression) {
    for (auto& loop : this->loops_) {
        if (symbolic::eq(loop.first, old_expression)) {
            if (!SymEngine::is_a<SymEngine::Symbol>(*new_expression)) {
                throw InvalidSDFGException("Einsum: New indvar must be a symbol");
            }
            loop.first = SymEngine::rcp_static_cast<const SymEngine::Symbol>(new_expression);
            loop.second = symbolic::subs(loop.second, old_expression, new_expression);
        } else if (symbolic::eq(loop.second, old_expression)) {
            loop.second = symbolic::subs(loop.second, old_expression, new_expression);
        }
    }
};

}  // namespace structured_control_flow
}  // namespace sdfg
