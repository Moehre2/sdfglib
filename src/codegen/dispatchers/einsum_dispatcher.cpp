#include "sdfg/codegen/dispatchers/einsum_dispatcher.h"

#include <string>
#include <utility>
#include <vector>

#include "sdfg/codegen/dispatchers/node_dispatcher.h"
#include "sdfg/codegen/instrumentation/instrumentation.h"
#include "sdfg/codegen/language_extension.h"
#include "sdfg/codegen/utils.h"
#include "sdfg/data_flow/memlet.h"
#include "sdfg/schedule.h"
#include "sdfg/structured_control_flow/einsum.h"
#include "sdfg/symbolic/symbolic.h"

namespace sdfg {
namespace codegen {

EinsumDispatcher::EinsumDispatcher(LanguageExtension& language_extension, Schedule& schedule,
                                   structured_control_flow::Einsum& node,
                                   Instrumentation& instrumentation)
    : NodeDispatcher(language_extension, schedule, node, instrumentation),
      node_(node),
      function_(schedule.sdfg()) {}

std::string EinsumDispatcher::containerSubset(const std::string& container,
                                              const std::vector<std::string>& indices) {
    data_flow::Subset subset(indices.size());
    for (size_t i = 0; i < indices.size(); ++i) {
        subset[i] = symbolic::symbol(indices[i]);
    }
    return container + this->language_extension_.subset(this->function_,
                                                        this->function_.type(container), subset);
}

void EinsumDispatcher::dispatch_node(PrettyPrinter& main_stream, PrettyPrinter& globals_stream,
                                     PrettyPrinter& library_stream) {
    size_t nested = 0;

    // Print outer for loops
    for (std::string out_index : this->node_.out_indices()) {
        for (std::pair<symbolic::Symbol, symbolic::Expression>& loop : this->node_.loops()) {
            if (loop.first->get_name() != out_index) continue;
            main_stream << "for";
            main_stream << "(";
            main_stream << loop.first->get_name();
            main_stream << " = 0; ";
            main_stream << loop.first->get_name();
            main_stream << " < ";
            main_stream << this->language_extension_.expression(loop.second);
            main_stream << "; ";
            main_stream << loop.first->get_name();
            main_stream << "++)" << std::endl;
            main_stream << "{" << std::endl;
            main_stream.setIndent(main_stream.indent() + 4);
            ++nested;
        }
    }

    // Set out container entry to 0
    // TODO: mt einsum - Maybe set 0 according to correct type.
    main_stream << this->containerSubset(this->node_.out_container(), this->node_.out_indices())
                << " = 0;" << std::endl;

    // Print inner for loops
    for (std::pair<symbolic::Symbol, symbolic::Expression>& loop : this->node_.loops()) {
        for (std::string out_index : this->node_.out_indices()) {
            if (loop.first->get_name() == out_index) continue;
            main_stream << "for";
            main_stream << "(";
            main_stream << loop.first->get_name();
            main_stream << " = 0; ";
            main_stream << loop.first->get_name();
            main_stream << " < ";
            main_stream << this->language_extension_.expression(loop.second);
            main_stream << "; ";
            main_stream << loop.first->get_name();
            main_stream << "++)" << std::endl;
            main_stream << "{" << std::endl;
            main_stream.setIndent(main_stream.indent() + 4);
            ++nested;
        }
    }

    // Calculate one entry
    main_stream << this->containerSubset(this->node_.out_container(), this->node_.out_indices())
                << " = "
                << this->containerSubset(this->node_.out_container(), this->node_.out_indices())
                << " + ";
    for (size_t i = 0; i < this->node_.in_containers().size(); ++i) {
        if (i > 0) main_stream << " * ";
        main_stream << this->containerSubset(this->node_.in_containers().at(i),
                                             this->node_.in_indices().at(i));
    }

    // Closing brackets
    for (size_t i = 0; i < nested; ++i) {
        main_stream.setIndent(main_stream.indent() - 4);
        main_stream << "}" << std::endl;
    }
}

}  // namespace codegen
}  // namespace sdfg
