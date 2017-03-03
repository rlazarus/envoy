#include <iostream>
#include <memory>

#include "common/event/libevent.h"
#include "common/local_info/local_info_impl.h"
#include "common/network/utility.h"
#include "common/stats/thread_local_store.h"

#include "exe/hot_restart.h"

#include "server/config_validation/hot_restart.h"
#include "server/config_validation/server.h"
#include "server/drain_manager_impl.h"
#include "server/options_impl.h"
#include "server/server.h"
#include "server/test_hooks.h"

#include "ares.h"
#include "spdlog/spdlog.h"

namespace Server {

class ProdComponentFactory : public ComponentFactory {
public:
  // Server::DrainManagerFactory
  DrainManagerPtr createDrainManager(Instance& server) override {
    return DrainManagerPtr{new DrainManagerImpl(server)};
  }

  Runtime::LoaderPtr createRuntime(Server::Instance& server,
                                   Server::Configuration::Initial& config) override {
    return Server::InstanceUtil::createRuntime(server, config);
  }
};

} // Server

int validateConfigAndExit(OptionsImpl& options, Server::ProdComponentFactory& component_factory,
                          const LocalInfo::LocalInfoImpl& local_info) {
  Server::ValidationHotRestart restarter;
  Logger::Registry::initialize(options.logLevel(), restarter.logLock());
  Stats::HeapRawStatDataAllocator alloc;
  Stats::ThreadLocalStoreImpl stats_store(alloc);

  Server::ValidationInstance server(options, restarter, stats_store, restarter.accessLogLock(),
                                    component_factory, local_info);
  std::cerr << "configuration '" << options.configPath() << "' OK" << std::endl;
  server.shutdown();
  return 0;
}

int main(int argc, char** argv) {
  Event::Libevent::Global::initialize();
  OptionsImpl options(argc, argv, Server::SharedMemory::version(), spdlog::level::warn);
  Server::ProdComponentFactory component_factory;
  LocalInfo::LocalInfoImpl local_info(Network::Utility::getLocalAddress(), options.serviceZone(),
                                      options.serviceClusterName(), options.serviceNodeName());

  if (options.mode() != Server::Mode::Serve) {
    return validateConfigAndExit(options, component_factory, local_info);
  }

  ares_library_init(ARES_LIB_INIT_ALL);

  std::unique_ptr<Server::HotRestartImpl> restarter;
  try {
    restarter.reset(new Server::HotRestartImpl(options));
  } catch (EnvoyException& e) {
    std::cerr << "unable to initialize hot restart: " << e.what() << std::endl;
    return 1;
  }

  Logger::Registry::initialize(options.logLevel(), restarter->logLock());
  DefaultTestHooks default_test_hooks;
  Stats::ThreadLocalStoreImpl stats_store(*restarter);

  Server::InstanceImpl server(options, default_test_hooks, *restarter, stats_store,
                              restarter->accessLogLock(), component_factory, local_info);
  server.run();
  ares_library_cleanup();
  return 0;
}
