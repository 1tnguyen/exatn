#include "directed_boost_graph.hpp"

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"

#include <memory>
#include <set>

using namespace cppmicroservices;

namespace {

/**
 */
class US_ABI_LOCAL GraphActivator : public BundleActivator {

public:
  GraphActivator() {}

  /**
   */
  void Start(BundleContext context) {

    auto g = std::make_shared<exatn::runtime::DirectedBoostGraph>();
    context.RegisterService<exatn::runtime::TensorGraph>(g);
  }

  /**
   */
  void Stop(BundleContext /*context*/) {}
};

} // namespace

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(GraphActivator)
