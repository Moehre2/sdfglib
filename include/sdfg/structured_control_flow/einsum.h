#pragma once

#include <utility>
#include <vector>

#include "sdfg/element.h"
#include "sdfg/structured_control_flow/control_flow_node.h"
#include "sdfg/symbolic/symbolic.h"

namespace sdfg {

namespace builder {
class StructuredSDFGBuilder;
}

namespace structured_control_flow {

class Einsum : public ControlFlowNode {
    friend class sdfg::builder::StructuredSDFGBuilder;

   private:
    std::vector<std::string> in_containers_;
    std::string out_container_;

    std::vector<std::pair<symbolic::Symbol, symbolic::Expression>> loops_;

    std::vector<std::vector<std::string>> in_indices_;
    std::vector<std::string> out_indices_;

    Einsum(const DebugInfo& debug_info, std::vector<std::string> in_containers,
           std::string out_container,
           std::vector<std::pair<symbolic::Symbol, symbolic::Expression>> loops,
           std::vector<std::vector<std::string>> in_indices, std::vector<std::string> out_indices);

   public:
    Einsum(const Einsum& node) = delete;
    Einsum& operator=(const Einsum&) = delete;

    const std::vector<std::string>& in_containers() const;
    std::vector<std::string>& in_containers();

    const std::string& out_container() const;
    std::string& out_container();

    const std::vector<std::pair<symbolic::Symbol, symbolic::Expression>>& loops() const;
    std::vector<std::pair<symbolic::Symbol, symbolic::Expression>>& loops();

    const std::vector<std::vector<std::string>>& in_indices() const;
    std::vector<std::vector<std::string>>& in_indices();

    const std::vector<std::string>& out_indices() const;
    std::vector<std::string>& out_indices();

    void replace(const symbolic::Expression& old_expression,
                 const symbolic::Expression& new_expression) override;
};

}  // namespace structured_control_flow

}  // namespace sdfg
