/* Copyright 2012 MaidSafe.net limited

This MaidSafe Software is licensed under the MaidSafe.net Commercial License, version 1.0 or later,
and The General Public License (GPL), version 3. By contributing code to this project You agree to
the terms laid out in the MaidSafe Contributor Agreement, version 1.0, found in the root directory
of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also available at:

http://www.novinet.com/license

Unless required by applicable law or agreed to in writing, software distributed under the License is
distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
implied. See the License for the specific language governing permissions and limitations under the
License.
*/

#ifndef MAIDSAFE_VAULT_MOCK_ROUTING_MOCK_ROUTING_H_
#define MAIDSAFE_VAULT_MOCK_ROUTING_MOCK_ROUTING_H_

#include <string>

#include "gmock/gmock.h"

#include "maidsafe/routing/routing_api.h"
#include "maidsafe/routing/node_info.h"


namespace maidsafe {

namespace vault {

namespace test {

class MockRouting : public maidsafe::routing::Routing {
 public:
  template<typename FobType>
  explicit MockRouting(const FobType& fob) : Routing(fob) {}
  virtual ~MockRouting() {}

  MOCK_METHOD2(Join, void(routing::Functors functors,
                          std::vector<boost::asio::ip::udp::endpoint> peer_endpoints));
  MOCK_METHOD4(ZeroStateJoin, int(routing::Functors functors,
                                  const boost::asio::ip::udp::endpoint& local_endpoint,
                                  const boost::asio::ip::udp::endpoint& peer_endpoint,
                                  const routing::NodeInfo& peer_info));

  MOCK_METHOD4(SendDirect, void(const NodeId& destination_id,           // ID of final destination
                                const std::string& message,
                                const bool& cacheable,                  // to cache message content
                                routing::ResponseFunctor response_functor));
  MOCK_METHOD4(SendGroup, void(const NodeId& destination_id,          // ID of final destination or group centre
                               const std::string& message,
                               const bool& cacheable,                 // to cache message content
                               routing::ResponseFunctor response_functor));

  MOCK_METHOD1(ClosestToId, bool(const NodeId& target_id));
  MOCK_METHOD2(IsNodeIdInGroupRange, routing::GroupRangeStatus(const NodeId& group_id,
                                                               const NodeId& node_id));
  MOCK_METHOD1(IsNodeIdInGroupRange, routing::GroupRangeStatus(const NodeId& group_id));
  MOCK_METHOD0(RandomConnectedNode, NodeId());
  MOCK_METHOD2(EstimateInGroup, bool(const NodeId& sender_id,
                                     const NodeId& info_id));
//  MOCK_METHOD1(GetGroup, std::future<std::vector<NodeId>>(const NodeId& group_id));

  MOCK_METHOD0(kNodeId, NodeId());
  MOCK_METHOD0(network_status, int());
  MOCK_METHOD0(ClosestNodes, std::vector<routing::NodeInfo>());
  MOCK_METHOD1(IsConnectedVault, bool(const NodeId& node_id));
  MOCK_METHOD1(IsConnectedClient, bool(const NodeId& node_id));

 private:
  MockRouting &operator=(const MockRouting&);
  MockRouting(const MockRouting&);
};

template<>
MockRouting::MockRouting(const NodeId& node_id);

}  // namespace test

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_MOCK_ROUTING_MOCK_ROUTING_H_
