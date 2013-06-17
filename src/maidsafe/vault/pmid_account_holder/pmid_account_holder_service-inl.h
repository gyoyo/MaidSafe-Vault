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

#ifndef MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HOLDER_SERVICE_INL_H_
#define MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HOLDER_SERVICE_INL_H_

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

template<typename Data, nfs::MessageAction Action>
PmidAccountUnresolvedEntry CreateUnresolvedEntry(const nfs::Message& message,
                                                 const NodeId& this_id) {
  static_assert(Action == nfs::MessageAction::kPut || Action == nfs::MessageAction::kDelete,
                "Action must be either kPut of kDelete.");
  return PmidAccountUnresolvedEntry(
            std::make_pair(DbKey(GetDataNameVariant(DataTagValue(message.data().type.get()),
                                                    Identity(message.data().name))),
                           Action),
            message.data().content.string().size(),
            this_id);
}

PmidName GetPmidAccountName(const nfs::Message& message);

}  // namespace detail

template<typename Data>
void PmidAccountHolderService::HandleMessage(const nfs::Message& message,
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
void PmidAccountHolderService::HandlePut(const nfs::Message& message) {
  try {
    Data data(typename Data::name_type(message.data().name),
              typename Data::serialised_type(message.data().content));

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
void PmidAccountHolderService::HandleDelete(const nfs::Message& message,
                                            const routing::ReplyFunctor& /*reply_functor*/) {
//  SendReplyAndAddToAccumulator(message, reply_functor, nfs::Reply(CommonErrors::success));
  try {
    auto account_name(detail::GetPmidAccountName(message));
    typename Data::name_type data_name(message.data().name);
    pmid_account_handler_.Delete<Data>(account_name, data_name);
    AddLocalUnresolvedEntryThenSync<Data, nfs::MessageAction::kDelete>(message);
    nfs_.Delete<Data>(message.data_holder(), data_name, [](std::string) {});
  }
  catch(const maidsafe_error& error) {
    LOG(kWarning) << error.what();
  }
  catch(...) {
    LOG(kWarning) << "Unknown error.";
  }
}

template<typename Data>
void PmidAccountHolderService::HandlePutCallback(const std::string& serialised_reply,
                                                 const nfs::Message& message) {
  nfs::Reply reply((nfs::Reply::serialised_type(NonEmptyString(serialised_reply))));
  if (reply.IsSuccess()) {
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
void PmidAccountHolderService::AddLocalUnresolvedEntryThenSync(const nfs::Message& message) {
  auto account_name(detail::GetPmidAccountName(message));
  auto unresolved_entry(detail::CreateUnresolvedEntry<Data, Action>(message, routing_.kNodeId()));
  pmid_account_handler_.AddLocalUnresolvedEntry(account_name, unresolved_entry);
  Sync(account_name);
}

}  // namespace vault
}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HOLDER_SERVICE_INL_H_
