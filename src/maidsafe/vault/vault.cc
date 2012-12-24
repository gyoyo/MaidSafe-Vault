/***************************************************************************************************
 *  Copyright 2012 MaidSafe.net limited                                                            *
 *                                                                                                 *
 *  The following source code is property of MaidSafe.net limited and is not meant for external    *
 *  use.  The use of this code is governed by the licence file licence.txt found in the root of    *
 *  this directory and also on www.maidsafe.net.                                                   *
 *                                                                                                 *
 *  You are not free to copy, amend or otherwise use this source code without the explicit         *
 *  written permission of the board of directors of MaidSafe.net.                                  *
 **************************************************************************************************/

#include "maidsafe/vault/vault.h"

#include "maidsafe/routing/routing_api.h"

//#include "maidsafe/nfs/nfs.h"

namespace maidsafe {

namespace vault {
Vault::Vault(passport::Pmid pmid,
             boost::filesystem::path vault_root_dir,
             std::function<void(boost::asio::ip::udp::endpoint)> on_new_bootstrap_endpoint,
             const std::vector<boost::asio::ip::udp::endpoint>& peer_endpoints)
    : network_status_mutex_(),
      routing_(new routing::Routing(&pmid)),
      key_getter_(routing_),
      maid_account_holder_(*routing_, vault_root_dir),
      meta_data_manager_(*routing_, vault_root_dir),
      pmid_account_holder_(*routing_, vault_root_dir),
      data_holder_(*routing_, vault_root_dir),
      demux_(maid_account_holder_, meta_data_manager_, pmid_account_holder_, data_holder_) {
  InitRouting(peer_endpoints);
}

int Vault::InitRouting(const std::vector<boost::asio::ip::udp::endpoint>& peer_endpoints) {
  routing::Functors functors(InitialiseRoutingCallbacks());
  routing_->Join(functors, peer_endpoints);
  return 0;
}

routing::Functors Vault::InitialiseRoutingCallbacks() {
  routing::Functors functors;
  functors.message_received = [this] (const std::string& message,
                                      const NodeId& /*group_claim*/, // to be removed
                                      bool /*cache_lookup*/,
                                      const routing::ReplyFunctor& reply_functor) {
    demux_.HandleMessage(message, reply_functor);
  };

  functors.network_status = [this] (const int& network_health) {
                              OnNetworkStatusChange(network_health);
                            };
  functors.close_node_replaced = [this] (const std::vector<routing::NodeInfo>& new_close_nodes) {
                                   OnCloseNodeReplaced(new_close_nodes);
                                 };
  functors.request_public_key = [this] (const NodeId& node_id,
                                        const routing::GivePublicKeyFunctor& give_key) {
                                  OnPublicKeyRequested(node_id, give_key);
                                };
  functors.new_bootstrap_endpoint = [this] (const boost::asio::ip::udp::endpoint& endpoint) {
                                      OnNewBootstrapEndpoint(endpoint);
                                    };
  functors.store_cache_data = [this] (const std::string& message) {
                                OnStoreCacheData(message);
                              };
  functors.have_cache_data = [this] (std::string& message) {
                                return OnHaveCacheData(message);
                              };
  return functors;
}

void Vault::OnNetworkStatusChange(const int& network_health) {
  if (network_health >= 0) {
    if (network_health >= network_health_)
      LOG(kVerbose) << "Init - " /*<< kDebugId*/ << " - Network health is " << network_health
                    << "% (was " << network_health_ << "%)";
    else
      LOG(kWarning) << "Init - " /*<< kDebugId*/ << " - Network health is " << network_health
                    << "% (was " << network_health_ << "%)";
  } else {
    LOG(kWarning) << "Init - " /*<< kDebugId*/ << " - Network is down (" << network_health << ")";
  }
  network_health_ = network_health;
  // TODO (Team) : actions when network is down/up
}

void Vault::OnPublicKeyRequested(const NodeId& /*node_id*/,
                                 const routing::GivePublicKeyFunctor& give_key) {
  key_getter_.HandleGetKey(node_id, give_key);
}

void Vault::OnCloseNodeReplaced(const std::vector<routing::NodeInfo>& /*new_close_nodes*/) {

}

void Vault::OnStoreCacheData(const std::string& message) {
  demux_.StoreCache(message);
}

bool Vault::OnHaveCacheData(std::string& message) {
  return demux_.HaveCache(message);
}

void Vault::OnNewBootstrapEndpoint(const boost::asio::ip::udp::endpoint& endpoint) {
  on_new_bootstrap_endpoint(endpoint);
}

}  // namespace vault

}  // namespace maidsafe
