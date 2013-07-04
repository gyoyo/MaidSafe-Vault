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

#include "maidsafe/vault/pmid_account_holder/pmid_account_holder_service.h"

#include "maidsafe/common/error.h"

#include "maidsafe/vault/pmid_account_holder/pmid_account.pb.h"
#include "maidsafe/vault/sync.pb.h"

namespace fs = boost::filesystem;

namespace maidsafe {
namespace vault {

const int PmidAccountHolderService::kDeleteRequestsRequired_(3);
const int PmidAccountHolderService::kPutRepliesSuccessesRequired_(1);

namespace detail {

PmidName GetPmidAccountName(const nfs::Message& message) {
  return PmidName(Identity(message.data().name));
}

template<typename Message>
inline bool ForThisPersona(const Message& message) {
  return message.destination_persona() != nfs::Persona::kPmidAccountHolder;
}

}  // namespace detail

PmidAccountHolderService::PmidAccountHolderService(const passport::Pmid& pmid,
                                                   routing::Routing& routing,
                                                   Db& db)
    : routing_(routing),
      accumulator_mutex_(),
      accumulator_(),
      pmid_account_handler_(db, routing.kNodeId()),
      nfs_(routing, pmid) {}

void PmidAccountHolderService::HandleMessage(const nfs::Message& message,
                                             const routing::ReplyFunctor& /*reply_functor*/) {
  ValidateGenericSender(message);
  nfs::Reply reply(CommonErrors::success);
  nfs::MessageAction action(message.data().action);
  switch (action) {
    case nfs::MessageAction::kCreatePmidAccount:
      return CreatePmidAccount(message);
    case nfs::MessageAction::kGetPmidTotals:
      return GetPmidTotals(message);
    case nfs::MessageAction::kSynchronise:
      return HandleSync(message);
    case nfs::MessageAction::kAccountTransfer:
      return HandleAccountTransfer(message);
    case nfs::MessageAction::kGetPmidTotals:
      return HandleGetPmidTotals(message, reply_functor);
    default:
      LOG(kError) << "Unhandled Post action type";
  }
}

void PmidAccountHolderService::CreatePmidAccount(const nfs::Message& message) {
  try {
    pmid_account_handler_.CreateAccount(message.data_holder());
  }
  catch(const maidsafe_error& error) {
    LOG(kWarning) << error.what();
  }
  catch(...) {
    LOG(kError) << "Unknown error.";
  }
}

void PmidAccountHolderService::GetPmidTotals(const nfs::Message& message) {
  try {
    PmidRecord pmid_record(pmid_account_handler_.GetPmidRecord(PmidName(message.data().name)));
    if (!pmid_record.pmid_name.data.string().empty()) {
      nfs::Reply reply(CommonErrors::success, pmid_record.Serialise());
      nfs_.ReturnPmidTotals(message.source().node_id, reply.Serialise());
    }
  }
  catch(const maidsafe_error& error) {
    LOG(kWarning) << error.what();
  }
  catch(...) {
    LOG(kWarning) << "Unknown error.";
  }
}

void PmidAccountHolderService::HandleChurnEvent(routing::MatrixChange matrix_change) {
  auto account_names(pmid_account_handler_.GetAccountNames());
  auto itr(std::begin(account_names));
  while (itr != std::end(account_names)) {
    auto check_holders_result(CheckHolders(matrix_change, routing_.kNodeId(),
                                           NodeId((*itr)->string())));
    // Delete accounts for which this node is no longer responsible.
    if (check_holders_result.proximity_status != routing::GroupRangeStatus::kInRange) {
      pmid_account_handler_.DeleteAccount(*itr);
      itr = account_names.erase(itr);
      continue;
    }
    // Replace old_node(s) in sync object and send AccountTransfer to new node(s).
    assert(check_holders_result.old_holders.size() == check_holders_result.new_holders.size());
    for (auto i(0U); i != check_holders_result.old_holders.size(); ++i) {
      pmid_account_handler_.ReplaceNodeInSyncList(*itr, check_holders_result.old_holders[i],
                                                  check_holders_result.new_holders[i]);
      TransferAccount(*itr, check_holders_result.new_holders[i]);
    }

    ++itr;
  }
}

void PmidAccountHolderService::ValidateDataSender(const nfs::Message& message) const {
  if (!message.HasDataHolder()
      || !routing_.IsConnectedVault(NodeId(message.data_holder()->string()))
      || routing_.EstimateInGroup(message.source().node_id, NodeId(message.data().name)))
    ThrowError(VaultErrors::permission_denied);

  if (!FromMetadataManager(message) || !detail::ForThisPersona(message))
    ThrowError(CommonErrors::invalid_parameter);
}

void PmidAccountHolderService::ValidateGenericSender(const nfs::Message& message) const {
  if (!routing_.IsConnectedVault(message.source().node_id)
      || routing_.EstimateInGroup(message.source().node_id, NodeId(message.data().name)))
    ThrowError(VaultErrors::permission_denied);

  if (!FromMetadataManager(message) || !detail::ForThisPersona(message))
    ThrowError(CommonErrors::invalid_parameter);
}

// =============== Sync ===========================================================================

void PmidAccountHolderService::Sync(const PmidName& account_name) {
  auto serialised_sync_data(pmid_account_handler_.GetSyncData(account_name));
  if (!serialised_sync_data.IsInitialised())  // Nothing to sync
    return;

  protobuf::Sync proto_sync;
  proto_sync.set_account_name(account_name->string());
  proto_sync.set_serialised_unresolved_entries(serialised_sync_data.string());

  nfs_.Sync(account_name, NonEmptyString(proto_sync.SerializeAsString()));
}

void PmidAccountHolderService::HandleSync(const nfs::Message& message) {
  std::vector<PmidAccountResolvedEntry> resolved_entries;
  protobuf::Sync proto_sync;
  if (!proto_sync.ParseFromString(message.data().content.string())) {
    LOG(kError) << "Error parsing Synchronise message.";
    return;
  }

  pmid_account_handler_.ApplySyncData(PmidName(Identity(proto_sync.account_name())),
                                      NonEmptyString(proto_sync.serialised_unresolved_entries()));
  //ReplyToMetadataManagers(resolved_entries);
}

// =============== Account transfer ===============================================================

void PmidAccountHolderService::TransferAccount(const PmidName& account_name,
                                               const NodeId& new_node) {
  protobuf::PmidAccount pmid_account;
  pmid_account.set_pmid_name(account_name.data.string());
  PmidAccount::serialised_type
    serialised_account_details(pmid_account_handler_.GetSerialisedAccount(account_name));
  pmid_account.set_serialised_account_details(serialised_account_details.data.string());
  nfs_.TransferAccount(new_node, NonEmptyString(pmid_account.SerializeAsString()));
}

void PmidAccountHolderService::HandleAccountTransfer(const nfs::Message& message) {
  protobuf::PmidAccount pmid_account;
  NodeId source_id(message.source().node_id);
  if (!pmid_account.ParseFromString(message.data().content.string()))
    return;

  PmidName account_name(Identity(pmid_account.pmid_name()));
  bool finished_all_transfers(
      pmid_account_handler_.ApplyAccountTransfer(account_name, source_id,
         PmidAccount::serialised_type(NonEmptyString(pmid_account.serialised_account_details()))));
  if (finished_all_transfers)
    return;    // TODO(Team) Implement whatever else is required here?
}

// =============== DataHolder =====================================================================


}  // namespace vault
}  // namespace maidsafe
