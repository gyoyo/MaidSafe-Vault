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

#ifndef MAIDSAFE_VAULT_PMID_MANAGER_SERVICE_INL_H_
#define MAIDSAFE_VAULT_PMID_MANAGER_SERVICE_INL_H_

#include <exception>
#include <string>

#include "maidsafe/common/error.h"
#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/vault/utils.h"
#include "maidsafe/vault/unresolved_element.h"

namespace maidsafe {
namespace vault {

namespace detail {

<<<<<<< HEAD:src/maidsafe/vault/pmid_account_holder/pmid_account_holder_service-inl.h
template<typename Data, nfs::MessageAction Action>
PmidAccountUnresolvedEntry CreateUnresolvedEntry(const nfs::Message& message,
=======
template<typename Data, nfs::MessageAction action>
PmidManagerUnresolvedEntry CreateUnresolvedEntry(const nfs::Message& message,
>>>>>>> next:src/maidsafe/vault/pmid_manager/service-inl.h
                                                 const NodeId& this_id) {
  static_assert(Action == nfs::MessageAction::kPut || Action == nfs::MessageAction::kDelete,
                "Action must be either kPut of kDelete.");
<<<<<<< HEAD:src/maidsafe/vault/pmid_account_holder/pmid_account_holder_service-inl.h
  return PmidAccountUnresolvedEntry(
            std::make_pair(DbKey(GetDataNameVariant(DataTagValue(message.data().type.get()),
                                                    Identity(message.data().name))),
                           Action),
            message.data().content.string().size(),
            this_id);
=======
  return PmidManagerUnresolvedEntry(
      std::make_pair(GetDataNameVariant(DataTagValue(message.data().type.get()),
                                        Identity(message.data().name)), action),
      static_cast<int32_t>(message.data().content.string().size()),
      this_id);
>>>>>>> next:src/maidsafe/vault/pmid_manager/service-inl.h
}

PmidName GetPmidAccountName(const nfs::Message& message);

}  // namespace detail

template<typename Data>
void PmidManagerService::HandleMessage(const nfs::Message& message,
                                             const routing::ReplyFunctor& reply_functor) {
  ValidateDataSender(message);
  nfs::Reply reply(CommonErrors::success);
  {
    std::lock_guard<std::mutex> lock(accumulator_mutex_);
    if (accumulator_.CheckHandled(message, reply))
      return;
  }

  if (message.data().action == nfs::MessageAction::kPut) {
    HandlePut<Data>(message);
  } else if (message.data().action == nfs::MessageAction::kDelete) {
    HandleDelete<Data>(message, reply_functor);
  } else {
    LOG(kError) << "Unsupported operation.";
  }
}

template<typename Data>
<<<<<<< HEAD:src/maidsafe/vault/pmid_account_holder/pmid_account_holder_service-inl.h
void PmidAccountHolderService::HandlePut(const nfs::Message& message) {
  try {
    Data data(typename Data::name_type(message.data().name),
              typename Data::serialised_type(message.data().content));
=======
void PmidManagerService::HandlePut(const nfs::Message& message,
                                         const routing::ReplyFunctor& reply_functor) {
  maidsafe_error return_code(CommonErrors::success);
  try {
    Data data(typename Data::name_type(message.data().name),
              typename Data::serialised_type(message.data().content));

    auto put_op(std::make_shared<nfs::OperationOp>(
        kPutRepliesSuccessesRequired_,
        [this, message, reply_functor](nfs::Reply overall_result) {
            this->HandlePutResult<Data>(overall_result, message, reply_functor);
        }));
>>>>>>> next:src/maidsafe/vault/pmid_manager/service-inl.h

    nfs_.Put(PmidName(detail::GetPmidAccountName(message)),
             data,
             [this, message](std::string reply) {
                this->HandlePutCallback<Data>(reply, message);
             });
    nfs::Reply reply(maidsafe::CommonErrors::success);
    accumulator_.SetHandled(message, reply);
    return;
  }
  catch(const maidsafe_error& error) {
    LOG(kWarning) << error.what();
  }
  catch(...) {
    LOG(kWarning) << "Unknown error.";
  }
  SendPutResult<Data>(message, false);
}

template<typename Data>
<<<<<<< HEAD:src/maidsafe/vault/pmid_account_holder/pmid_account_holder_service-inl.h
void PmidAccountHolderService::HandleDelete(const nfs::Message& message,
                                            const routing::ReplyFunctor& /*reply_functor*/) {
//  SendReplyAndAddToAccumulator(message, reply_functor, nfs::Reply(CommonErrors::success));
=======
void PmidManagerService::HandleDelete(const nfs::Message& message,
                                            const routing::ReplyFunctor& reply_functor) {
>>>>>>> next:src/maidsafe/vault/pmid_manager/service-inl.h
  try {
    auto account_name(detail::GetPmidAccountName(message));
    typename Data::name_type data_name(message.data().name);
<<<<<<< HEAD:src/maidsafe/vault/pmid_account_holder/pmid_account_holder_service-inl.h
    pmid_account_handler_.Delete<Data>(account_name, data_name);
    AddLocalUnresolvedEntryThenSync<Data, nfs::MessageAction::kDelete>(message);
    nfs_.Delete<Data>(message.data_holder(), data_name, [](std::string) {});
=======
    if (detail::AddResult(message, reply_functor, MakeError(CommonErrors::success),
                          accumulator_, accumulator_mutex_, kDeleteRequestsRequired_)) {
      pmid_account_handler_.Delete<Data>(message.pmid_node(), data_name);
      nfs_.Delete<Data>(message.pmid_node(), data_name, nullptr);
    }
>>>>>>> next:src/maidsafe/vault/pmid_manager/service-inl.h
  }
  catch(const maidsafe_error& error) {
    LOG(kWarning) << error.what();
  }
  catch(...) {
    LOG(kWarning) << "Unknown error.";
  }
}

template<typename Data>
<<<<<<< HEAD:src/maidsafe/vault/pmid_account_holder/pmid_account_holder_service-inl.h
void PmidAccountHolderService::HandlePutCallback(const std::string& serialised_reply,
                                                 const nfs::Message& message) {
  nfs::Reply reply((nfs::Reply::serialised_type(NonEmptyString(serialised_reply))));
  if (reply.IsSuccess()) {
=======
void PmidManagerService::HandlePutResult(const nfs::Reply& overall_result,
                                               const nfs::Message& message,
                                               routing::ReplyFunctor reply_functor) {
  if (overall_result.IsSuccess()) {
    nfs::Reply reply(CommonErrors::success);
>>>>>>> next:src/maidsafe/vault/pmid_manager/service-inl.h
    AddLocalUnresolvedEntryThenSync<Data, nfs::MessageAction::kPut>(message);
    SendPutResult<Data>(message, true);
  } else {
    SendPutResult<Data>(message, false);
  }
}

template<typename Data>
void PmidAccountHolderService::SendPutResult(const nfs::Message& message, bool result) {
  std::lock_guard<std::mutex> lock(accumulator_mutex_);
  protobuf::PutResult proto_put_result;

  proto_put_result.set_result(result);
  proto_put_result.set_pmid_name(message.data_holder()->string());
  if (result) {
    proto_put_result.set_data_size(message.data().content.string().size());
  } else {
    proto_put_result.set_serialised_data(message.Serialise()->string());
  }
  nfs_.SendPutResult<Data>(Data::name_type(message.data().name),
                           NonEmptyString(proto_put_result.SerializeAsString()));
}

template<typename Data, nfs::MessageAction Action>
void PmidManagerService::AddLocalUnresolvedEntryThenSync(const nfs::Message& message) {
  auto account_name(detail::GetPmidAccountName(message));
  auto unresolved_entry(detail::CreateUnresolvedEntry<Data, Action>(message, routing_.kNodeId()));
  pmid_account_handler_.AddLocalUnresolvedEntry(account_name, unresolved_entry);
  Sync(account_name);
}

<<<<<<< HEAD:src/maidsafe/vault/pmid_account_holder/pmid_account_holder_service-inl.h
=======
template<typename Data, nfs::MessageAction Action>
void PmidManagerService::ReplyToDataManagers(
      const std::vector<PmidManagerResolvedEntry>& /*resolved_entries*/,
      const PmidName& /*pmid_name*/) {
  GetTagValueAndIdentityVisitor type_and_name_visitor;
  //for (auto& resolved_entry : resolved_entries) {
  //  auto type_and_name(boost::apply_visitor(type_and_name_visitor, resolved_entry.key.first));
  //  nfs::Message::Data data(type_and_name.first, type_and_name.second, NonEmptyString(), Action);
  //  nfs::Message meassage(nfs::Persona::kDataManager,
  //                        nfs::PersonaId(nfs::Persona::kPmidManager, routing_.kNodeId()),
  //                        data,
  //                        pmid_name);
  //  message.
  //}
}

>>>>>>> next:src/maidsafe/vault/pmid_manager/service-inl.h
}  // namespace vault
}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_PMID_MANAGER_SERVICE_INL_H_
