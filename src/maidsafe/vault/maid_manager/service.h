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

#ifndef MAIDSAFE_VAULT_MAID_MANAGER_SERVICE_H_
#define MAIDSAFE_VAULT_MAID_MANAGER_SERVICE_H_

#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <vector>

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/on_scope_exit.h"
#include "maidsafe/common/types.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/data_types/data_name_variant.h"
#include "maidsafe/passport/types.h"
#include "maidsafe/routing/routing_api.h"
#include "maidsafe/nfs/message.h"
#include "maidsafe/nfs/public_key_getter.h"
#include "maidsafe/nfs/reply.h"
#include "maidsafe/nfs/utils.h"

#include "maidsafe/vault/accumulator.h"
#include "maidsafe/vault/group_db.h"
#include "maidsafe/vault/sync.h"
#include "maidsafe/vault/types.h"
#include "maidsafe/vault/unresolved_action.h"
#include "maidsafe/vault/utils.h"
#include "maidsafe/vault/maid_manager/action_create_remove_account.h"
#include "maidsafe/vault/maid_manager/action_put.h"
#include "maidsafe/vault/maid_manager/action_delete.h"
#include "maidsafe/vault/maid_manager/action_register_unregister_pmid.h"
#include "maidsafe/vault/maid_manager/maid_manager.h"
#include "maidsafe/vault/maid_manager/maid_manager.pb.h"


namespace maidsafe {

class OwnerDirectory;
class GroupDirectory;
class WorldDirectory;

namespace vault {

class AccountDb;
struct PmidRegistrationOp;
struct GetPmidTotalsOp;
class MaidManagerMetadata;

class MaidManagerService {
 public:
  MaidManagerService(const passport::Pmid& pmid,
                     routing::Routing& routing,
                     nfs::PublicKeyGetter& public_key_getter);
  // Handling of received requests (sending of requests is done via nfs_ object).
  template<typename Data>
  void HandleMessage(const nfs::Message& message, const routing::ReplyFunctor& reply_functor);
  void HandleMessage(const nfs::Message& message, const routing::ReplyFunctor& reply_functor);
  void HandleChurnEvent(std::shared_ptr<routing::MatrixChange> matrix_change);
  static int DefaultPaymentFactor() { return kDefaultPaymentFactor_; }

 private:
  MaidManagerService(const MaidManagerService&);
  MaidManagerService& operator=(const MaidManagerService&);
  MaidManagerService(MaidManagerService&&);
  MaidManagerService& operator=(MaidManagerService&&);

  void CheckSenderIsConnectedMaidNode(const nfs::Message& message) const;
  void CheckSenderIsConnectedMaidManager(const nfs::Message& message) const;
  void ValidateDataSender(const nfs::Message& message) const;
  void ValidateGenericSender(const nfs::Message& message) const;

  // =============== Put/Delete data ===============================================================
  template<typename Data>
  void HandlePut(const nfs::Message& message, const routing::ReplyFunctor& reply_functor);

  // Only Maid and Anmaid can create account; for all others this is a no-op.
  typedef std::true_type AllowedAccountCreationType;
  typedef std::false_type DisallowedAccountCreationType;
  template<typename Data>
  void CreateAccount(const MaidName& account_name, AllowedAccountCreationType);
  template<typename Data>
  void CreateAccount(const MaidName& /*account_name*/, DisallowedAccountCreationType) {}

  template<typename Data>
  void HandleDelete(const nfs::Message& message, const routing::ReplyFunctor& reply_functor);
  typedef std::true_type UniqueDataType;
  template<typename Data>
  void SendEarlySuccessReply(const nfs::Message& /*message*/,
                             const routing::ReplyFunctor& /*reply_functor*/,
                             bool /*low_space*/,
                             UniqueDataType) {}
  typedef std::false_type NonUniqueDataType;
  template<typename Data>
  void SendEarlySuccessReply(const nfs::Message& message,
                             const routing::ReplyFunctor& reply_functor,
                             bool low_space,
                             NonUniqueDataType);
  void SendReplyAndAddToAccumulator(const nfs::Message& message,
                                    const routing::ReplyFunctor& reply_functor,
                                    const nfs::Reply& reply);

  template<typename Data>
  void HandlePutResult(const nfs::Reply& overall_result,
                       const nfs::Message& message,
                       routing::ReplyFunctor client_reply_functor,
                       bool low_space,
                       UniqueDataType);

  template<typename Data>
  void HandlePutResult(const nfs::Reply& overall_result,
                       const nfs::Message& message,
                       routing::ReplyFunctor client_reply_functor,
                       bool low_space,
                       NonUniqueDataType);

  template<typename Data, nfs::MessageAction action>
  void AddLocalUnresolvedActionThenSync(const nfs::Message& message, int32_t cost);

  template<typename Data>
  void HandleVersionMessage(const nfs::Message& message,
                            const routing::ReplyFunctor& reply_functor);

  // =============== Pmid registration =============================================================
  void HandlePmidRegistration(const nfs::Message& message,
                              const routing::ReplyFunctor& reply_functor);
  template<typename PublicFobType>
  void ValidatePmidRegistration(const nfs::Reply& reply,
                                typename PublicFobType::name_type public_fob_name,
                                std::shared_ptr<PmidRegistrationOp> pmid_registration_op);
  void FinalisePmidRegistration(std::shared_ptr<PmidRegistrationOp> pmid_registration_op);

  // =============== Sync ==========================================================================
  void DoSync();
  void HandleSync(const nfs::Message& message);

  // =============== Account transfer ==============================================================
  void TransferAccount(const MaidName& account_name, const NodeId& new_node);
  void HandleAccountTransfer(const nfs::Message& message);

  // =============== PMID totals ===================================================================
  void UpdatePmidTotals(const MaidName& account_name);
  void UpdatePmidTotalsCallback(const std::string& serialised_reply,
                                std::shared_ptr<GetPmidTotalsOp> op_data);

  routing::Routing& routing_;
  nfs::PublicKeyGetter& public_key_getter_;
  GroupDb<MaidManager> group_db_;
  std::mutex accumulator_mutex_;
  Accumulator<MaidName> accumulator_;
  MaidManagerNfs nfs_;
  Sync<MaidManager::UnresolvedCreateAccount> sync_create_accounts_;
  Sync<MaidManager::UnresolvedRemoveAccount> sync_remove_accounts_;
  Sync<MaidManager::UnresolvedPut> sync_puts_;
  Sync<MaidManager::UnresolvedDelete> sync_deletes_;
  Sync<MaidManager::UnresolvedRegisterPmid> sync_register_pmids_;
  Sync<MaidManager::UnresolvedUnregisterPmid> sync_unregister_pmids_;
  static const int kPutRepliesSuccessesRequired_;
  static const int kDefaultPaymentFactor_;
};



// ==================== Implementation =============================================================
namespace detail {

template<typename T>
struct can_create_account : public std::false_type {};

template<>
struct can_create_account<passport::PublicAnmaid> : public std::true_type {};

template<>
struct can_create_account<passport::PublicMaid> : public std::true_type {};

template<typename Data>
int32_t EstimateCost(const Data& data) {
  static_assert(!std::is_same<Data, passport::PublicAnmaid>::value, "Cost of Anmaid should be 0.");
  static_assert(!std::is_same<Data, passport::PublicMaid>::value, "Cost of Maid should be 0.");
  static_assert(!std::is_same<Data, passport::PublicPmid>::value, "Cost of Pmid should be 0.");
  return static_cast<int32_t>(MaidManagerService::DefaultPaymentFactor() *
                              data.content.string().size());
}

template<>
int32_t EstimateCost<passport::PublicAnmaid>(const passport::PublicAnmaid&);

template<>
int32_t EstimateCost<passport::PublicMaid>(const passport::PublicMaid&);

template<>
int32_t EstimateCost<passport::PublicPmid>(const passport::PublicPmid&);

MaidName GetMaidAccountName(const nfs::Message& message);

template<typename Data>
typename Data::name_type GetDataName(const nfs::Message& message) {
  // Hash the data name to obfuscate the list of chunks associated with the client.
  return typename Data::name_type(crypto::Hash<crypto::SHA512>(message.data().name));
}

}  // namespace detail


template<typename Data>
void MaidManagerService::HandleMessage(const nfs::Message& message,
                                       const routing::ReplyFunctor& reply_functor) {
  ValidateDataSender(message);
  nfs::Reply reply(CommonErrors::success);
  {
    std::lock_guard<std::mutex> lock(accumulator_mutex_);
    if (accumulator_.CheckHandled(message, reply))
      return reply_functor(reply.Serialise()->string());
  }
  switch (message.data().action) {
    case nfs::MessageAction::kPut:
      return HandlePut<Data>(message, reply_functor);
    case nfs::MessageAction::kDelete:
      return HandleDelete<Data>(message, reply_functor);
    case nfs::MessageAction::kGet:        // intentional fallthrough
    case nfs::MessageAction::kGetBranch:  // intentional fallthrough
    case nfs::MessageAction::kDeleteBranchUntilFork:
      return HandleVersionMessage<Data>(message, reply_functor);
    default:
      reply = nfs::Reply(VaultErrors::operation_not_supported, message.Serialise().data);
      SendReplyAndAddToAccumulator(message, reply_functor, reply);
  }
}

template<typename Data>
void MaidManagerService::HandlePut(const nfs::Message& message,
                                   const routing::ReplyFunctor& reply_functor) {
  maidsafe_error return_code(CommonErrors::success);
  try {
    Data data(typename Data::name_type(message.data().name),
              typename Data::serialised_type(message.data().content));
    auto account_name(detail::GetMaidAccountName(message));
    auto estimated_cost(detail::EstimateCost(message.data()));
    CreateAccount<Data>(account_name);





  }
  catch(const maidsafe_error& error) {
    LOG(kWarning) << error.what();
    return_code = error;
  }
  catch(...) {
    LOG(kWarning) << "Unknown error.";
    return_code = MakeError(CommonErrors::unknown);
  }
  nfs::Reply reply(return_code, message.Serialise().data);
  SendReplyAndAddToAccumulator(message, reply_functor, reply);
}

template<>
void MaidManagerService::HandlePut<OwnerDirectory>(const nfs::Message& message,
                                                   const routing::ReplyFunctor& reply_functor);

template<>
void MaidManagerService::HandlePut<GroupDirectory>(const nfs::Message& message,
                                                   const routing::ReplyFunctor& reply_functor);

template<>
void MaidManagerService::HandlePut<WorldDirectory>(const nfs::Message& message,
                                                   const routing::ReplyFunctor& reply_functor);

template<typename Data>
void MaidManagerService::CreateAccount(const MaidName& account_name, AllowedAccountCreationType) {
  sync_create_accounts_.AddLocalAction();
  DoSync();
}

template<typename Data>
void MaidManagerService::HandleDelete(const nfs::Message& message,
                                      const routing::ReplyFunctor& reply_functor) {
  SendReplyAndAddToAccumulator(message, reply_functor, nfs::Reply(CommonErrors::success));
  try {
    auto account_name(detail::GetMaidAccountName(message));
    typename Data::name_type data_name(message.data().name);
    maid_account_handler_.DeleteData<Data>(account_name, data_name);
    AddLocalUnresolvedActionThenSync<Data, nfs::MessageAction::kDelete>(message, 0);
    nfs_.Delete<Data>(data_name, [](std::string /*serialised_reply*/) {});
  }
  catch(const maidsafe_error& error) {
    LOG(kWarning) << error.what();
    // Always return success for Deletes
  }
  catch(...) {
    LOG(kWarning) << "Unknown error.";
    // Always return success for Deletes
  }
}

template<typename Data>
void MaidManagerService::SendEarlySuccessReply(const nfs::Message& message,
                                               const routing::ReplyFunctor& reply_functor,
                                               bool low_space,
                                               NonUniqueDataType) {
  nfs::Reply reply(CommonErrors::success);
  if (low_space)
    reply = nfs::Reply(VaultErrors::low_space);
  SendReplyAndAddToAccumulator(message, reply_functor, reply);
}

template<typename Data>
void MaidManagerService::HandlePutResult(const nfs::Reply& overall_result,
                                         const nfs::Message& message,
                                         routing::ReplyFunctor client_reply_functor,
                                         bool low_space,
                                         UniqueDataType) {
  if (overall_result.IsSuccess()) {
    nfs::Reply reply(CommonErrors::success);
    if (low_space)
      reply = nfs::Reply(VaultErrors::low_space);
    AddLocalUnresolvedActionThenSync<Data, nfs::MessageAction::kPut>(
        message,
        detail::EstimateCost(message.data()));
    SendReplyAndAddToAccumulator(message, client_reply_functor, reply);
  } else {
    SendReplyAndAddToAccumulator(message, client_reply_functor, overall_result);
  }
}

template<typename Data>
void MaidManagerService::HandlePutResult(const nfs::Reply& overall_result,
                                         const nfs::Message& message,
                                         routing::ReplyFunctor /*client_reply_functor*/,
                                         bool /*low_space*/,
                                         NonUniqueDataType) {
  try {
    if (overall_result.IsSuccess()) {
      protobuf::Cost proto_cost;
      proto_cost.ParseFromString(overall_result.data().string());
      // TODO(Fraser#5#): 2013-05-09 - The client's reply should only be sent *after* this call.
      AddLocalUnresolvedActionThenSync<Data, nfs::MessageAction::kPut>(message, proto_cost.cost());
    }
  }
  catch(const std::exception& e) {
    LOG(kError) << "Failed to Handle Put result: " << e.what();
  }
}

template<typename Data, nfs::MessageAction action>
void MaidManagerService::AddLocalUnresolvedActionThenSync(const nfs::Message& message,
                                                          int32_t cost) {
  auto account_name(detail::GetMaidAccountName(message));
  auto unresolved_action(detail::CreateUnresolvedAction<Data, action>(message, cost,
                                                                      routing_.kNodeId()));
  maid_account_handler_.AddLocalUnresolvedAction(account_name, unresolved_action);
  DoSync(account_name);
}


template<typename Data>
void MaidManagerService::HandleVersionMessage(const nfs::Message& message,
                                              const routing::ReplyFunctor& reply_functor) {

}

template<typename PublicFobType>
void MaidManagerService::ValidatePmidRegistration(
    const nfs::Reply& reply,
    typename PublicFobType::name_type public_fob_name,
    std::shared_ptr<PmidRegistrationOp> pmid_registration_op) {
  std::unique_ptr<PublicFobType> public_fob;
  try {
    public_fob.reset(new PublicFobType(public_fob_name,
                                       typename PublicFobType::serialised_type(reply.data())));
  }
  catch(const std::exception& e) {
    public_fob.reset();
    LOG(kError) << e.what();
  }
  bool finalise(false);
  {
    std::lock_guard<std::mutex> lock(pmid_registration_op->mutex);
    pmid_registration_op->SetPublicFob(std::move(public_fob));
    finalise = (++pmid_registration_op->count == 2);
  }
  if (finalise)
    FinalisePmidRegistration(pmid_registration_op);
}

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_MAID_MANAGER_SERVICE_H_
