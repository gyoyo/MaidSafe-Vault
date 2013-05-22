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

#ifndef MAIDSAFE_VAULT_UTILS_INL_H_
#define MAIDSAFE_VAULT_UTILS_INL_H_

#include <algorithm>
#include <vector>
#include <set>

#include "maidsafe/routing/parameters.h"

#include "maidsafe/vault/accumulator.h"


namespace maidsafe {

namespace vault {


template<typename Name>
HandledRequest<Name>::HandledRequest(const nfs::MessageId& msg_id_in,
                                     const Name& account_name_in,
                                     const nfs::MessageAction& action_type_in,
                                     const Identity& data_name_in,
                                     const DataTagValue& data_type_in,
                                     const int32_t& size_in,
                                     const maidsafe_error& return_code_in)
    : msg_id(msg_id_in),
      account_name(account_name_in),
      action(action_type_in),
      data_name(data_name_in),
      data_type(data_type_in),
      size(size_in),
      return_code(return_code_in) {}
/*
template<typename Name>
typename HandledRequest<Name>::HandledRequest(const typename HandledRequest<Name>& other)
    : msg_id(other.msg_id),
      account_name(other.account_name),
      action(other.action),
      data_name(other.data_name),
      data_type(other.data_type),
      size(other.size),
      return_code(other.return_code) {}

template<typename Name>
typename HandledRequest<Name>& HandledRequest<Name>::operator=(
    const HandledRequest& other) {
  msg_id = other.msg_id;
  account_name = other.account_name;
  action = other.action;
  data_name = other.data_name,
  data_type = other.data_type,
  size = other.size;
  return_code = other.return_code;
  return *this;
}

template<typename Name>
typename HandledRequest<Name>::HandledRequest(typename HandledRequest<Name>&& other)
    : msg_id(std::move(other.msg_id)),
      account_name(std::move(other.account_name)),
      action(std::move(other.action)),
      data_name(std::move(other.data_name)),
      data_type(std::move(other.data_type)),
      size(std::move(other.size)),
      return_code(std::move(other.return_code)) {}

template<typename Name>
typename HandledRequest<Name>& HandledRequest<Name>::operator=(
    HandledRequest&& other) {
  msg_id = std::move(other.msg_id);
  account_name = std::move(other.account_name);
  action = std::move(other.action);
  data_name = std::move(other.data_name),
  data_type = std::move(data_type);
  size = std::move(other.size);
  return_code = std::move(other.return_code);
  return *this;
}*/

// template<typename Name>
// typename std::deque<typename HandledRequest<Name>>::const_iterator
//     FindHandled<Name>(const nfs::Message& message) const {
//   return std::find_if(std::begin(handled_requests_),
//                       std::end(handled_requests_),
//                       [&message](const HandledRequest& handled_request) {
//                       return (handled_request.msg_id == message.message_id()) &&
//                              (handled_request.account_name ==
//                                  Name(Identity(message.source().node_id.string())));
//                       });
// }

// template<>
// typename std::deque<typename HandledRequest<DataNameVariant>>::const_iterator
//     FindHandled<DataNameVariant>(const nfs::Message& message) const {
//   return std::find_if(std::begin(handled_requests_),
//                       std::end(handled_requests_),
//                       [&message](const HandledRequest& handled_request)->bool {
//                           auto req_name_and_type =
//                               boost::apply_visitor(GetTagValueAndIdentityVisitor(),
//                                                    handled_request.account_name);
//                           return (handled_request.msg_id == message.message_id()) &&
//                               (req_name_and_type.first == message.data().type) &&
//                               (req_name_and_type.second.string() == message.data().name.string());
//                       });
// }

template<typename Message>
inline bool FromMaidAccountHolder(const Message& message) {
  return message.source().persona == nfs::Persona::kMaidAccountHolder;
}

template<typename Message>
inline bool FromMetadataManager(const Message& message) {
  return message.source().persona == nfs::Persona::kMetadataManager;
}

template<typename Message>
inline bool FromPmidAccountHolder(const Message& message) {
  return message.source().persona == nfs::Persona::kPmidAccountHolder;
}

template<typename Message>
inline bool FromDataHolder(const Message& message) {
  return message.source().persona == nfs::Persona::kDataHolder;
}

template<typename Message>
inline bool FromClientMaid(const Message& message) {
  return message.source().persona == nfs::Persona::kClientMaid;
}

template<typename Message>
inline bool FromClientMpid(const Message& message) {
  return message.source().persona == nfs::Persona::kClientMpid;
}

template<typename Message>
inline bool FromOwnerDirectoryManager(const Message& message) {
  return message.source().persona == nfs::Persona::kOwnerDirectoryManager;
}

template<typename Message>
inline bool FromGroupDirectoryManager(const Message& message) {
  return message.source().persona == nfs::Persona::kGroupDirectoryManager;
}

template<typename Message>
inline bool FromWorldDirectoryManager(const Message& message) {
  return message.source().persona == nfs::Persona::kWorldDirectoryManager;
}

template<typename Message>
inline bool FromDataGetter(const Message& message) {
  return message.source().persona == nfs::Persona::kDataGetter;
}

template<>
inline bool PendingRequestsEqual<nfs::Persona::kMaidAccountHolder>(const nfs::Message& lhs,
                                                                   const nfs::Message& rhs) {
  return lhs.message_id() == rhs.message_id() && lhs.source().persona == rhs.source().persona;
}


namespace detail {

template<typename Data>
bool IsDataElement(const typename Data::name_type& name,
                   const DataNameVariant& data_name_variant) {
  return DataNameVariant(name) == data_name_variant;
}

// Returns true if the required successful request count has been reached
template<typename Accumulator>
bool AddResult(const nfs::Message& message,
               const routing::ReplyFunctor& reply_functor,
               const maidsafe_error& return_code,
               Accumulator& accumulator,
               std::mutex& accumulator_mutex,
               int requests_required) {
  std::vector<typename Accumulator::PendingRequest> pending_requests;
  maidsafe_error overall_return_code(CommonErrors::success);
  {
    std::lock_guard<std::mutex> lock(accumulator_mutex);
    auto pending_results(accumulator.PushSingleResult(message, reply_functor, return_code));
    std::vector<nfs::Reply> pending_replies;
    for (auto itr(std::begin(pending_results)); itr != std::end(pending_results); ++itr) {
      pending_replies.emplace_back((*itr).second);
    }

    if (static_cast<int>(pending_results.size()) < requests_required)
      return false;

    auto result(nfs::GetSuccessOrMostFrequentReply(pending_replies, requests_required));
    if (!result.second && pending_results.size() < routing::Parameters::node_group_size)
      return false;

    overall_return_code = (*result.first).error();
    pending_requests = accumulator.SetHandled(message, overall_return_code);
  }

  for (auto& pending_request : pending_requests)
    SendReply(pending_request.msg, overall_return_code, pending_request.reply_functor);

  return true;
}


}  // namespace detail

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_UTILS_INL_H_
