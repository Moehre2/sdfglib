#pragma once

#include <string>
#include <vector>

#include "sdfg/codegen/dispatchers/node_dispatcher.h"
#include "sdfg/codegen/instrumentation/instrumentation.h"
#include "sdfg/codegen/language_extension.h"
#include "sdfg/codegen/utils.h"
#include "sdfg/function.h"
#include "sdfg/schedule.h"
#include "sdfg/structured_control_flow/einsum.h"

namespace sdfg {
namespace codegen {

class EinsumDispatcher : public NodeDispatcher {
   private:
    structured_control_flow::Einsum& node_;
    const Function& function_;

    std::string containerSubset(const std::string& container,
                                const std::vector<std::string>& indices);

   public:
    EinsumDispatcher(LanguageExtension& language_extension, Schedule& schedule,
                     structured_control_flow::Einsum& node, Instrumentation& instrumentation);

    void dispatch_node(PrettyPrinter& main_stream, PrettyPrinter& globals_stream,
                       PrettyPrinter& library_stream) override;
};

}  // namespace codegen
}  // namespace sdfg
