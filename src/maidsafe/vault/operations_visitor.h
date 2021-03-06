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

#ifndef MAIDSAFE_VAULT_OPERATIONS_VISITOR_H_
#define MAIDSAFE_VAULT_OPERATIONS_VISITOR_H_

#include "maidsafe/common/types.h"
#include "maidsafe/nfs/types.h"
#include "maidsafe/vault/types.h"
#include "maidsafe/common/node_id.h"


namespace maidsafe {

namespace vault {


namespace detail {

template <typename ServiceHandlerType>
class MaidManagerPutVisitor : public boost::static_visitor<> {
 public:
  MaidManagerPutVisitor(ServiceHandlerType* service,
                        NonEmptyString content,
                        NodeId sender,
                        Identity pmid_hint,
                        nfs::MessageId message_id)
      : service_(service),
        kContent_(std::move(content)),
        kSender(std::move(sender)),
        kPmidHint_(std::move(pmid_hint)),
        kMessageId(std::move(message_id)) {}

  template <typename Name>
  void operator()(const Name& data_name) {
    service_->template HandlePut(MaidName(kSender),
                                 Name::data_type(data_name, kContent_),
                                 kPmidHint_,
                                 kMessageId);
  }
  private:
   ServiceHandlerType* service_;
   const NonEmptyString kContent_;
   const Identity kPmidHint_;
   const nfs::MessageId kMessageId;
   const NodeId kSender;
};

template <typename ServiceHandlerType>
class DataManagerPutVisitor : public boost::static_visitor<> {
 public:
  DataManagerPutVisitor(ServiceHandlerType* service,
                        NonEmptyString content,
                        Identity maid_name,
                        Identity pmid_name,
                        nfs::MessageId message_id)
      : service_(service),
        kContent_(std::move(content)),
        kMaidName(std::move(maid_name)),
        kPmidName_(std::move(pmid_name)),
        kMessageId(std::move(message_id)) {}

  template <typename Name>
  void operator()(const Name& data_name) {
    service_->template HandlePut(Name::data_type(data_name, kContent_), kMaidName, kPmidName_,
                                 kMessageId);
  }

 private:
  ServiceHandlerType* service_;
  const NonEmptyString kContent_;
  const MaidName kMaidName;
  const PmidName kPmidName_;
  const nfs::MessageId kMessageId;
};


template <typename ServiceHandlerType>
class PmidManagerPutVisitor : public boost::static_visitor<> {
 public:
  PmidManagerPutVisitor(ServiceHandlerType* service,
                        const NonEmptyString& content,
                        const Identity& pmid_name,
                        const nfs::MessageId& message_id)
      : service_(service),
        kContent_(content),
        kPmidName_(pmid_name) ,
        kMessageId(message_id) {}

  template <typename Name>
  void operator()(const Name& data_name) {
    service_->template HandlePut(Name::data_type(data_name, kContent_), kPmidName_, kMessageId);
  }

 private:
  ServiceHandlerType* service_;
  const NonEmptyString kContent_;
  const PmidName kPmidName_;
  const nfs::MessageId kMessageId;
};

template <typename ServiceHandlerType>
class PmidNodePutVisitor : public boost::static_visitor<> {
 public:
  PmidNodePutVisitor(ServiceHandlerType* service,
                     const NonEmptyString& content,
                     const nfs::MessageId& message_id)
      : service_(service),
        kContent_(content),
        kMessageId(message_id) {}

  template <typename Name>
  void operator()(const Name& data_name) {
    service_->template HandlePut(Name::data_type(data_name, kContent_), kMessageId);
  }

 private:
  ServiceHandlerType* service_;
  const NonEmptyString kContent_;
  const nfs::MessageId kMessageId;
};


template <typename ServiceHandlerType>
class PutResponseFailureVisitor : public boost::static_visitor<> {
 public:
  PutResponseFailureVisitor(ServiceHandlerType* service,
                            const Identity& pmid_node,
                            const NonEmptyString& content,
                            const maidsafe_error& return_code,
                            const nfs::MessageId& message_id)
      : service_(service),
        kContent_(content),
        kPmidNode_(pmid_node),
        kMessageId(message_id),
        kReturnCode_(return_code) {}

  template <typename Name>
  void operator()(const Name& data_name) {
    service_->template HandlePutResponse(Name::data_type(data_name, kContent_),
                                         kPmidNode_,
                                         kMessageId,
                                         maidsafe_error(kReturnCode_));
  }

  private:
   ServiceHandlerType* service_;
   const NonEmptyString kContent_;
   const PmidName kPmidNode_;
   const nfs::MessageId kMessageId;
   const maidsafe_error kReturnCode_;
};

template <typename ServiceHandlerType>
class PutResponseSuccessVisitor : public boost::static_visitor<> {
 public:
  PutResponseSuccessVisitor(ServiceHandlerType* service,
                                       const Identity& pmid_node,
                                       const nfs::MessageId& message_id)
      : service_(service),
        kPmidNode_(pmid_node),
        kMessageId(message_id) {}

  template <typename Name>
  void operator()(const Name& data_name) {
    service_->template HandlePutResponse<typename Name::data_type>(data_name,
                                                                   kPmidNode_,
                                                                   kMessageId);
  }
  private:
   ServiceHandlerType* service_;
   const PmidName kPmidNode_;
   const nfs::MessageId kMessageId;
};

template <typename ServiceHandlerType>
class MaidManagerPutResponseVisitor : public boost::static_visitor<> {
 public:
  MaidManagerPutResponseVisitor(ServiceHandlerType* service,
                                const Identity& maid_node,
                                const int32_t& cost)
      : service_(service),
        kMaidNode_(maid_node),
        kCost_(cost) {}

  template <typename Name>
  void operator()(const Name& data_name) {
    service_->template HandlePutResponse(kMaidNode_, data_name, kCost_);
  }
  private:
   ServiceHandlerType* service_;
   const MaidName kMaidNode_;
   const int32_t kCost_;
};


}  // namespace detail

}  // namespace vault

}  // namespace maidsafe

#include "maidsafe/vault/utils-inl.h"

#endif  // MAIDSAFE_VAULT_OPERATIONS_VISITOR_H_
