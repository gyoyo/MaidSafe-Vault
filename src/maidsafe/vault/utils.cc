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

#include "maidsafe/vault/utils.h"

#include <string>

#include "boost/filesystem/operations.hpp"
#include "leveldb/status.h"

#include "maidsafe/common/types.h"
#include "maidsafe/nfs/types.h"
#include "maidsafe/vault/parameters.h"

#include "maidsafe/vault/operations_visitor.h"


namespace fs = boost::filesystem;

namespace maidsafe {

namespace vault {

namespace detail {

template<typename T>
DataNameVariant GetNameVariant(const T&) {
  T::invalid_parameter;
  return DataNameVariant();
}

template<>
DataNameVariant GetNameVariant(const nfs_vault::DataName& data) {
   return GetDataNameVariant(data.type, data.raw_name);
}

template<>
DataNameVariant GetNameVariant(const nfs_vault::DataNameAndContent& data) {
   return GetNameVariant(data.name);
}

template<>
DataNameVariant GetNameVariant(const nfs_vault::DataAndPmidHint& data) {
   return GetNameVariant(data.data.name);
}

template<>
DataNameVariant GetNameVariant(const nfs_client::DataAndReturnCode& data) {
   return GetNameVariant(data.data.name);
}

template<>
DataNameVariant GetNameVariant(const nfs_client::DataNameAndContentOrReturnCode& data) {
  if (data.data)
    return GetNameVariant(data.data->name);
  else
    return GetNameVariant(data.data_name_and_return_code->name);
}

template<typename ServiceHandlerType>
void DoOperation(ServiceHandlerType* service,
                 const nfs::PutRequestFromMaidNodeToMaidManager& message,
                 const nfs::PutRequestFromMaidNodeToMaidManager::Sender& sender,
                 const nfs::PutRequestFromMaidNodeToMaidManager::Receiver & /*receiver*/) {
  auto data_name(detail::GetNameVariant(*message.contents));
  MaidManagerPutVisitor<ServiceHandlerType> put_visitor(service,
                                                        message.contents->data.content,
                                                        sender.data,
                                                        message.contents->pmid_hint,
                                                        message.message_id);
  boost::apply_visitor(put_visitor, data_name);
}

template<typename ServiceHandlerType>
void DoOperation(ServiceHandlerType* service,
                 const nfs::PutRequestFromMaidManagerToDataManager& message,
                 const typename nfs::PutRequestFromMaidManagerToDataManager::Sender& sender,
                 const typename nfs::PutRequestFromMaidManagerToDataManager::Receiver&) {
  auto data_name(detail::GetNameVariant(*message.contents));
  DataManagerPutVisitor<ServiceHandlerType> put_visitor(service,
                                                        message.contents->data.content,
                                                        sender.group_id,
                                                        message.contents->pmid_hint,
                                                        message.message_id);
  boost::apply_visitor(put_visitor, data_name);
}

template<typename ServiceHandlerType>
void DoOperation(ServiceHandlerType* service,
                 const nfs::PutRequestFromDataManagerToPmidManager& message,
                 const nfs::PutRequestFromDataManagerToPmidManager::Sender& /*sender*/,
                 const nfs::PutRequestFromDataManagerToPmidManager::Receiver& receiver) {
  auto data_name(detail::GetNameVariant(*message.contents));
  PmidManagerPutVisitor<ServiceHandlerType> put_visitor(service,
                                                        message.contents->content,
                                                        message.message_id,
                                                        receiver);
  boost::apply_visitor(put_visitor, data_name);
}


template<typename ServiceHandlerType>
void DoOperation(ServiceHandlerType* service,
                 const nfs::PutRequestFromPmidManagerToPmidNode& message,
                 const nfs::PutRequestFromPmidManagerToPmidNode::Sender& /*sender*/,
                 const nfs::PutRequestFromPmidManagerToPmidNode::Receiver& /*receiver*/) {
  auto data_name(detail::GetNameVariant(*message.contents));
  PmidNodePutVisitor<ServiceHandlerType> put_visitor(service,
                                                     message.contents->content,
                                                     message.message_id);
  boost::apply_visitor(put_visitor, data_name);
}

template<typename ServiceHandlerType>
void DoOperation(ServiceHandlerType* service,
                 const nfs::PutResponseFromPmidNodeToPmidManager& message,
                 const nfs::PutResponseFromPmidNodeToPmidManager::Sender& sender,
                 const nfs::PutResponseFromPmidNodeToPmidManager::Receiver& /*receiver*/) {
  auto data_name(detail::GetNameVariant(*message.contents));
  if (message.contents->content) {
    PutResponseFailureVisitor<ServiceHandlerType> put_visitor(
        service,
        message.contents->content,
        sender,
        message.contents->return_code,
        message.message_id);
    boost::apply_visitor(put_visitor, data_name);
  } else {
    PutResponseSuccessVisitor<ServiceHandlerType> put_response_visitor(
        service, sender, message.message_id);
    boost::apply_visitor(put_response_visitor, data_name);
  }
}

template<typename ServiceHandlerType>
void DoOperation(ServiceHandlerType* service,
                 const nfs::PutResponseFromPmidManagerToDataManager& message,
                 const nfs::PutResponseFromPmidManagerToDataManager::Sender& sender,
                 const nfs::PutResponseFromPmidManagerToDataManager::Receiver& /*receiver*/) {
  auto data_name(detail::GetNameVariant(*message.contents));
  if (message.contents->content) {
    PutResponseFailureVisitor<ServiceHandlerType> put_visitor(
        service,
        message.contents->content,
        sender,
        message.contents->return_code,
        message.message_id);
    boost::apply_visitor(put_visitor, data_name);
  } else {
    PutResponseSuccessVisitor<ServiceHandlerType> put_response_visitor(
        service, sender, message.message_id);
    boost::apply_visitor(put_response_visitor, data_name);
  }
}

template<typename ServiceHandlerType>
void DoOperation(ServiceHandlerType* service,
                 const nfs::PutResponseFromDataManagerToMaidManager& message,
                 const nfs::PutResponseFromDataManagerToMaidManager::Sender& /*sender*/,
                 const nfs::PutResponseFromDataManagerToMaidManager::Receiver& receiver) {
  auto data_name(detail::GetNameVariant(message.contents->name));
  MaidManagerPutResponseVisitor<ServiceHandlerType> put_response_visitor(
      service,
      receiver,
      message.contents->cost,
      message.message_id);
  boost::apply_visitor(put_response_visitor, data_name);
}


void InitialiseDirectory(const boost::filesystem::path& directory) {
  if (fs::exists(directory)) {
    if (!fs::is_directory(directory))
      ThrowError(CommonErrors::not_a_directory);
  } else {
    fs::create_directory(directory);
  }
}

bool ShouldRetry(routing::Routing& routing, const NodeId& source_id, const NodeId& data_name) {
  return routing.network_status() >= Parameters::kMinNetworkHealth &&
         routing.EstimateInGroup(source_id, data_name);
}

/* Commented by Mahmoud on 2 Sep -- It may be of no use any more or needs refactoring
void SendReply(const nfs::Message& original_message,
               const maidsafe_error& return_code,
               const routing::ReplyFunctor& reply_functor) {
  if (!reply_functor)
    return;
  nfs::Reply reply(CommonErrors::success);
  if (return_code.code() != CommonErrors::success)
    reply = nfs::Reply(return_code, original_message.Serialise().data);
  reply_functor(reply.Serialise()->string());
} */

}  // namespace detail


std::unique_ptr<leveldb::DB> InitialiseLevelDb(const boost::filesystem::path& db_path) {
  if (boost::filesystem::exists(db_path))
    boost::filesystem::remove_all(db_path);
  leveldb::DB* db(nullptr);
  leveldb::Options options;
  options.create_if_missing = true;
  options.error_if_exists = true;
  leveldb::Status status(leveldb::DB::Open(options, db_path.string(), &db));
  if (!status.ok())
    ThrowError(CommonErrors::filesystem_io_error);
  assert(db);
  return std::move(std::unique_ptr<leveldb::DB>(db));
}

// To be moved to Routing
bool operator ==(const routing::GroupSource& lhs,  const routing::GroupSource& rhs) {
  return lhs.group_id == rhs.group_id &&
         lhs.sender_id == rhs.sender_id;
}

}  // namespace vault

}  // namespace maidsafe
