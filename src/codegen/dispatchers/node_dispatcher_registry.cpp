#include "sdfg/codegen/dispatchers/node_dispatcher_registry.h"

#include "sdfg/codegen/dispatchers/block_dispatcher.h"
#include "sdfg/codegen/dispatchers/for_dispatcher.h"
#include "sdfg/codegen/dispatchers/if_else_dispatcher.h"
#include "sdfg/codegen/dispatchers/map_dispatcher.h"
#include "sdfg/codegen/dispatchers/sequence_dispatcher.h"
#include "sdfg/codegen/dispatchers/while_dispatcher.h"

#include "sdfg/data_flow/library_nodes/barrier_local_node.h"
#include "sdfg/data_flow/library_nodes/math/math.h"
#include "sdfg/data_flow/library_nodes/metadata_node.h"

namespace sdfg {
namespace codegen {

std::unique_ptr<NodeDispatcher> create_dispatcher(
    LanguageExtension& language_extension,
    StructuredSDFG& sdfg,
    structured_control_flow::ControlFlowNode& node,
    Instrumentation& instrumentation
) {
    auto dispatcher = NodeDispatcherRegistry::instance().get_dispatcher(typeid(node));
    if (dispatcher) {
        return dispatcher(language_extension, sdfg, node, instrumentation);
    }

    throw std::runtime_error("Unsupported control flow node: " + std::string(typeid(node).name()));
};

void register_default_dispatchers() {
    /* Control flow dispatchers */
    NodeDispatcherRegistry::instance().register_dispatcher(
        typeid(structured_control_flow::Block),
        [](LanguageExtension& language_extension,
           StructuredSDFG& sdfg,
           structured_control_flow::ControlFlowNode& node,
           Instrumentation& instrumentation) {
            return std::make_unique<BlockDispatcher>(
                language_extension, sdfg, static_cast<structured_control_flow::Block&>(node), instrumentation
            );
        }
    );
    NodeDispatcherRegistry::instance().register_dispatcher(
        typeid(structured_control_flow::Sequence),
        [](LanguageExtension& language_extension,
           StructuredSDFG& sdfg,
           structured_control_flow::ControlFlowNode& node,
           Instrumentation& instrumentation) {
            return std::make_unique<SequenceDispatcher>(
                language_extension, sdfg, static_cast<structured_control_flow::Sequence&>(node), instrumentation
            );
        }
    );
    NodeDispatcherRegistry::instance().register_dispatcher(
        typeid(structured_control_flow::IfElse),
        [](LanguageExtension& language_extension,
           StructuredSDFG& sdfg,
           structured_control_flow::ControlFlowNode& node,
           Instrumentation& instrumentation) {
            return std::make_unique<IfElseDispatcher>(
                language_extension, sdfg, static_cast<structured_control_flow::IfElse&>(node), instrumentation
            );
        }
    );
    NodeDispatcherRegistry::instance().register_dispatcher(
        typeid(structured_control_flow::While),
        [](LanguageExtension& language_extension,
           StructuredSDFG& sdfg,
           structured_control_flow::ControlFlowNode& node,
           Instrumentation& instrumentation) {
            return std::make_unique<WhileDispatcher>(
                language_extension, sdfg, static_cast<structured_control_flow::While&>(node), instrumentation
            );
        }
    );
    NodeDispatcherRegistry::instance().register_dispatcher(
        typeid(structured_control_flow::For),
        [](LanguageExtension& language_extension,
           StructuredSDFG& sdfg,
           structured_control_flow::ControlFlowNode& node,
           Instrumentation& instrumentation) {
            return std::make_unique<ForDispatcher>(
                language_extension, sdfg, static_cast<structured_control_flow::For&>(node), instrumentation
            );
        }
    );
    NodeDispatcherRegistry::instance().register_dispatcher(
        typeid(structured_control_flow::Map),
        [](LanguageExtension& language_extension,
           StructuredSDFG& sdfg,
           structured_control_flow::ControlFlowNode& node,
           Instrumentation& instrumentation) {
            return std::make_unique<MapDispatcher>(
                language_extension, sdfg, static_cast<structured_control_flow::Map&>(node), instrumentation
            );
        }
    );
    NodeDispatcherRegistry::instance().register_dispatcher(
        typeid(structured_control_flow::Return),
        [](LanguageExtension& language_extension,
           StructuredSDFG& sdfg,
           structured_control_flow::ControlFlowNode& node,
           Instrumentation& instrumentation) {
            return std::make_unique<ReturnDispatcher>(
                language_extension, sdfg, static_cast<structured_control_flow::Return&>(node), instrumentation
            );
        }
    );
    NodeDispatcherRegistry::instance().register_dispatcher(
        typeid(structured_control_flow::Break),
        [](LanguageExtension& language_extension,
           StructuredSDFG& sdfg,
           structured_control_flow::ControlFlowNode& node,
           Instrumentation& instrumentation) {
            return std::make_unique<BreakDispatcher>(
                language_extension, sdfg, static_cast<structured_control_flow::Break&>(node), instrumentation
            );
        }
    );
    NodeDispatcherRegistry::instance().register_dispatcher(
        typeid(structured_control_flow::Continue),
        [](LanguageExtension& language_extension,
           StructuredSDFG& sdfg,
           structured_control_flow::ControlFlowNode& node,
           Instrumentation& instrumentation) {
            return std::make_unique<ContinueDispatcher>(
                language_extension, sdfg, static_cast<structured_control_flow::Continue&>(node), instrumentation
            );
        }
    );

    /* Map dispatchers */
    MapDispatcherRegistry::instance().register_map_dispatcher(
        structured_control_flow::ScheduleType_Sequential.value(),
        [](LanguageExtension& language_extension,
           StructuredSDFG& sdfg,
           structured_control_flow::Map& node,
           Instrumentation& instrumentation) {
            return std::make_unique<SequentialMapDispatcher>(language_extension, sdfg, node, instrumentation);
        }
    );
    MapDispatcherRegistry::instance().register_map_dispatcher(
        structured_control_flow::ScheduleType_CPU_Parallel.value(),
        [](LanguageExtension& language_extension,
           StructuredSDFG& sdfg,
           structured_control_flow::Map& node,
           Instrumentation& instrumentation) {
            return std::make_unique<CPUParallelMapDispatcher>(language_extension, sdfg, node, instrumentation);
        }
    );

    /* Librarynode dispatchers */
    LibraryNodeDispatcherRegistry::instance().register_library_node_dispatcher(
        data_flow::LibraryNodeType_BarrierLocal.value(),
        [](LanguageExtension& language_extension,
           const Function& function,
           const data_flow::DataFlowGraph& data_flow_graph,
           const data_flow::LibraryNode& node) {
            return std::make_unique<data_flow::BarrierLocalNodeDispatcher>(
                language_extension, function, data_flow_graph, dynamic_cast<const data_flow::BarrierLocalNode&>(node)
            );
        }
    );
    LibraryNodeDispatcherRegistry::instance().register_library_node_dispatcher(
        data_flow::LibraryNodeType_Metadata.value(),
        [](LanguageExtension& language_extension,
           const Function& function,
           const data_flow::DataFlowGraph& data_flow_graph,
           const data_flow::LibraryNode& node) {
            return std::make_unique<data_flow::MetadataDispatcher>(
                language_extension, function, data_flow_graph, dynamic_cast<const data_flow::MetadataNode&>(node)
            );
        }
    );

    /* Math */
    LibraryNodeDispatcherRegistry::instance().register_library_node_dispatcher(
        math::ml::LibraryNodeType_ReLU.value(),
        [](LanguageExtension& language_extension,
           const Function& function,
           const data_flow::DataFlowGraph& data_flow_graph,
           const data_flow::LibraryNode& node) {
            return std::make_unique<math::ml::ReLUNodeDispatcher>(
                language_extension, function, data_flow_graph, dynamic_cast<const math::ml::ReLUNode&>(node)
            );
        }
    );
}

} // namespace codegen
} // namespace sdfg
