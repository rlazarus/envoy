#pragma once

#include "envoy/upstream/cluster_manager.h"

#include "common/upstream/cluster_manager_impl.h"

#include "server/config_validation/async_client.h"

namespace Upstream {

/**
 * Config-validation-only implementation of ClusterManagerFactory, which creates
 * ValidationClusterManagers. It also creates, but never returns, CdsApiImpls.
 */
class ValidationClusterManagerFactory : public ProdClusterManagerFactory {
public:
  ValidationClusterManagerFactory(Runtime::Loader& runtime, Stats::Store& stats,
                                  ThreadLocal::Instance& tls, Runtime::RandomGenerator& random,
                                  Network::DnsResolver& dns_resolver,
                                  Ssl::ContextManager& ssl_context_manager,
                                  Event::Dispatcher& primary_dispatcher,
                                  const LocalInfo::LocalInfo& local_info);

  ClusterManagerPtr clusterManagerFromJson(const Json::Object& config, Stats::Store& stats,
                                           ThreadLocal::Instance& tls, Runtime::Loader& runtime,
                                           Runtime::RandomGenerator& random,
                                           const LocalInfo::LocalInfo& local_info,
                                           AccessLog::AccessLogManager& log_manager) override;

  // Delegates to ProdClusterManagerFactory::createCds, but discards the result and returns nullptr
  // unconditionally.
  CdsApiPtr createCds(const Json::Object& config, ClusterManager& cm) override;
};

/**
 * Config-validation-only implementation of ClusterManager, which opens no upstream connections.
 */
class ValidationClusterManager : public ClusterManagerImpl {
public:
  ValidationClusterManager(const Json::Object& config, ClusterManagerFactory& factory,
                           Stats::Store& stats, ThreadLocal::Instance& tls,
                           Runtime::Loader& runtime, Runtime::RandomGenerator& random,
                           const LocalInfo::LocalInfo& local_info,
                           AccessLog::AccessLogManager& log_manager);

  Http::ConnectionPool::Instance* httpConnPoolForCluster(const std::string&, ResourcePriority,
                                                         LoadBalancerContext*) override;
  Host::CreateConnectionData tcpConnForCluster(const std::string&) override;
  Http::AsyncClient& httpAsyncClientForCluster(const std::string&) override;

private:
  // ValidationAsyncClient always returns null on send() and start(), so it has
  // no internal state -- we might as well just keep one and hand out references
  // to it.
  Http::ValidationAsyncClient async_client_;
};

} // Upstream
