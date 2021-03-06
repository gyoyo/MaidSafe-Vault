/*  Copyright 2012 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

#ifndef MAIDSAFE_VAULT_POST_POLICIES_H_
#define MAIDSAFE_VAULT_POST_POLICIES_H_

#include <string>
#include <vector>

#include "maidsafe/common/crypto.h"
#include "maidsafe/common/rsa.h"
#include "maidsafe/common/types.h"
#include "maidsafe/data_types/data_name_variant.h"
#include "maidsafe/data_types/data_type_values.h"
#include "maidsafe/passport/types.h"
#include "maidsafe/routing/api_config.h"
#include "maidsafe/routing/routing_api.h"
#include "maidsafe/nfs/message.h"
#include "maidsafe/nfs/message_wrapper.h"
#include "maidsafe/nfs/types.h"

#include "maidsafe/vault/sync.pb.h"


namespace maidsafe {

namespace vault {

template<typename SyncPolicy, typename PersonaMiscellaneousPolicy>
class VaultPostPolicy : public SyncPolicy, public PersonaMiscellaneousPolicy {
 public:
  VaultPostPolicy(routing::Routing& routing, const passport::Pmid& pmid)
      : SyncPolicy(routing, pmid),
        PersonaMiscellaneousPolicy(routing, pmid) {}
};

template<nfs::Persona source_persona>
class SyncPolicy {
 public:
  SyncPolicy(routing::Routing& routing, const passport::Pmid& pmid)
      : routing_(routing),
        kSource_(source_persona, routing_.kNodeId()),
        kPmid_(pmid) {}

  template<typename UnresolvedAction>
  void Sync(const UnresolvedAction& unresolved_action) {
    protobuf::Sync proto_sync;
    proto_sync.set_action_type(UnresolvedAction::Action::kActionId);
    proto_sync.set_serialised_unresolved_action(unresolved_action.Serialise());
    nfs::Message::Data data(Identity(), proto_sync.SerializeAsString(),
                            nfs::MessageAction::kSynchronise);
    nfs::Message message(source_persona, kSource_, data);
    nfs::MessageWrapper message_wrapper(message.Serialise());
    routing_.SendGroup(NodeId(unresolved_action.key.name.string()),
                       message_wrapper.Serialise()->string(), false, nullptr);
  }

  void TransferRecords(const NodeId& target_node_id, const NonEmptyString& serialised_account) {
    nfs::Message::Data data(Identity(), serialised_account, nfs::MessageAction::kAccountTransfer);
    nfs::Message message(source_persona, kSource_, data);
    nfs::MessageWrapper message_wrapper(message.Serialise());
    routing_.SendDirect(target_node_id, message_wrapper.Serialise()->string(), false, nullptr);
  }

 private:
  routing::Routing& routing_;
  const nfs::PersonaId kSource_;
  const passport::Pmid kPmid_;
};

class MaidManagerMiscellaneousPolicy {
 public:
  MaidManagerMiscellaneousPolicy(routing::Routing& routing, const passport::Pmid& pmid)
      : routing_(routing),
        kSource_(nfs::Persona::kMaidManager, routing_.kNodeId()),
        kPmid_(pmid) {}

  void RequestPmidTotals(const passport::PublicPmid::Name& pmid_name,
                         const routing::ResponseFunctor& callback) {
    nfs::Message::Data data(DataTagValue::kPmidValue, pmid_name.data, NonEmptyString(),
                            nfs::MessageAction::kGetPmidTotals);
    nfs::Message message(nfs::Persona::kPmidManager, kSource_, data, pmid_name);
    nfs::MessageWrapper message_wrapper(message.Serialise());
    routing_.SendGroup(NodeId(pmid_name), message_wrapper.Serialise()->string(),
                       false, callback);
  }

 private:
  routing::Routing& routing_;
  const nfs::PersonaId kSource_;
  const passport::Pmid kPmid_;
};

class DataManagerMiscellaneousPolicy {
 public:
  DataManagerMiscellaneousPolicy(routing::Routing& routing, const passport::Pmid& pmid)
      : routing_(routing),
        kSource_(nfs::Persona::kMaidManager, routing_.kNodeId()),
        kPmid_(pmid) {}

 private:
  routing::Routing& routing_;
  const nfs::PersonaId kSource_;
  const passport::Pmid kPmid_;
};

class PmidManagerMiscellaneousPolicy {
 public:
  PmidManagerMiscellaneousPolicy(routing::Routing& routing, const passport::Pmid& pmid)
      : routing_(routing),
        kSource_(nfs::Persona::kPmidManager, routing_.kNodeId()),
        kPmid_(pmid) {}

  void ReturnPmidTotals(const NodeId& target_node_id,
                        const nfs::Reply::serialised_type& serialised_reply) {
    nfs::Message::Data data(Identity(target_node_id.string()), serialised_reply.data,
                            nfs::MessageAction::kPmidTotals);
    nfs::Message message(nfs::Persona::kMaidManager, kSource_, data);
    nfs::MessageWrapper message_wrapper(message.Serialise());
    routing_.SendGroup(target_node_id, message_wrapper.Serialise()->string(), false, nullptr);
  }

  template<typename Data>
  void SendPutResult(const typename Data::Name& data_name,
                     const NonEmptyString& serialised_put_result) {
    nfs::Message::Data data(data_name, serialised_put_result, nfs::MessageAction::kPutResult);
    nfs::Message message(nfs::Persona::kDataManager, kSource_, data, kPmid_.name());
    nfs::MessageWrapper message_wrapper(message.Serialise());
    routing_.SendGroup(NodeId(data_name), message_wrapper.Serialise()->string(), false, nullptr);
  }

  void ReturnFailure(const nfs::Message& message) {
    nfs::MessageWrapper message_wrapper(message.Serialise());
    nfs::Reply(CommonErrors::unable_to_handle_request, message.Serialise());
    NodeId target_node_id(message.source().node_id);
    routing_.SendDirect(target_node_id, message_wrapper.Serialise()->string(), false, nullptr);
  }

  template<typename Data>
  void AccountTransfer(const typename Data::Name& data_name,
                       const NonEmptyString& serialised_account) {
    nfs::Message::Data data(data_name, serialised_account, nfs::MessageAction::kAccountTransfer);
    nfs::Message message(nfs::Persona::kPmidNode, kSource_, data, kPmid_.name());
    nfs::MessageWrapper message_wrapper(message.Serialise());
    routing_.SendDirect(NodeId(data_name), message_wrapper.Serialise()->string(), false, nullptr);
  }

 private:
  routing::Routing& routing_;
  const nfs::PersonaId kSource_;
  const passport::Pmid kPmid_;
};

class PmidNodeMiscellaneousPolicy {
 public:
  PmidNodeMiscellaneousPolicy(routing::Routing& routing, const passport::Pmid& pmid)
      : routing_(routing),
        kSource_(nfs::Persona::kMaidManager, routing_.kNodeId()),
        kPmid_(pmid) {}

  void RequestPmidNodeAccount(const passport::PublicPmid::Name& pmid_name,
                              const routing::ResponseFunctor& callback) {
    nfs::Message::Data data(DataTagValue::kPmidValue, pmid_name.data, NonEmptyString(),
                            nfs::MessageAction::kGetPmidAccount);
    nfs::Message message(nfs::Persona::kPmidNode, kSource_, data, pmid_name);
    nfs::MessageWrapper message_wrapper(message.Serialise());
    routing_.SendGroup(NodeId(pmid_name), message_wrapper.Serialise()->string(), false, callback);
  }

 private:
  routing::Routing& routing_;
  const nfs::PersonaId kSource_;
  const passport::Pmid kPmid_;
};

class NoPolicy {
 public:
  NoPolicy(routing::Routing& /*routing*/, const passport::Pmid& /*pmid*/) {}
};


typedef VaultPostPolicy<SyncPolicy<nfs::Persona::kMaidManager>,
                        MaidManagerMiscellaneousPolicy> MaidManagerPostPolicy;

typedef VaultPostPolicy<SyncPolicy<nfs::Persona::kDataManager>,
                        DataManagerMiscellaneousPolicy> DataManagerPostPolicy;

typedef VaultPostPolicy<SyncPolicy<nfs::Persona::kPmidManager>,
                        PmidManagerMiscellaneousPolicy> PmidManagerPostPolicy;
// FIXME
typedef VaultPostPolicy<NoPolicy, PmidNodeMiscellaneousPolicy> PmidNodePostPolicy;

typedef VaultPostPolicy<SyncPolicy<nfs::Persona::kVersionManager>,
                        NoPolicy> VersionManagerPostPolicy;

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_POST_POLICIES_H_
