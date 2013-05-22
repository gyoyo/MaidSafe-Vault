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

#include "maidsafe/vault/accumulator.h"



namespace maidsafe {

namespace vault {

template<>
std::vector<typename Accumulator<DataNameVariant>::PendingRequest>
    Accumulator<DataNameVariant>::SetHandled(const nfs::Message& message,
                                             const maidsafe_error& return_code) {
  std::vector<PendingRequest> ret_requests;
  auto itr = pending_requests_.begin();
  while (itr != pending_requests_.end()) {
    if ((*itr).msg.message_id() == message.message_id() &&
        (*itr).msg.source().node_id == message.source().node_id) {
      ret_requests.push_back(*itr);
      itr = pending_requests_.erase(itr);
    } else {
      ++itr;
    }
  }

  handled_requests_.push_back(
      HandledRequest<DataNameVariant>(message.message_id(),
                                      GetDataNameVariant(*message.data().type,
                                                         message.data().name),
                                      message.data().action,
                                      message.data().name,
                                      *message.data().type,
                                      static_cast<int32_t>(
                                          message.data().content.string().size()),
                                      return_code));
  if (handled_requests_.size() > kMaxHandledRequestsCount_)
    handled_requests_.pop_front();
  return ret_requests;
}

template<>
std::vector<typename Accumulator<PmidName>::PendingRequest> Accumulator<PmidName>::SetHandled(
    const nfs::Message& message,
    const maidsafe_error& return_code) {
  std::vector<PendingRequest> ret_requests;
  auto itr = pending_requests_.begin();
  while (itr != pending_requests_.end()) {
    if ((*itr).msg.message_id() == message.message_id() &&
        (*itr).msg.source().node_id == message.source().node_id) {
      ret_requests.push_back(*itr);
      itr = pending_requests_.erase(itr);
    } else {
      ++itr;
    }
  }

  handled_requests_.push_back(
      HandledRequest<PmidName>(message.message_id(),
                               message.data_holder(),
                               message.data().action,
                               message.data().name,
                               *message.data().type,
                               static_cast<int32_t>(message.data().content.string().size()),
                               return_code));
  if (handled_requests_.size() > kMaxHandledRequestsCount_)
    handled_requests_.pop_front();
  return ret_requests;
}

}  // namespace vault

}  // namespace maidsafe
