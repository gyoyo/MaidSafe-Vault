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

#include "maidsafe/vault/data_manager/data_manager_value.h"

#include <string>

//#include "maidsafe/vault/utils.h"

namespace maidsafe {

namespace vault {

namespace delete_me {

DataManagerValue::DataManagerValue(const serialised_type& serialised_metadata_value)
  : data_size_(),
    subscribers_(),
    online_pmids_(),
    offline_pmids_() {
  protobuf::DataManagerValue metadata_value_proto;
  if (!metadata_value_proto.ParseFromString(serialised_metadata_value->string()) ||
      metadata_value_proto.size() != 0 ||
      metadata_value_proto.subscribers() < 1) {
    LOG(kError) << "Failed to read or parse serialised metadata value";
    ThrowError(CommonErrors::parsing_error);
  } else {
    data_size_ = metadata_value_proto.size();
    subscribers_ = metadata_value_proto.subscribers();
    for (auto& i : metadata_value_proto.online_pmid_name())
      online_pmids_.insert(PmidName(Identity(i)));
    for (auto& i : metadata_value_proto.offline_pmid_name())
      offline_pmids_.insert(PmidName(Identity(i)));
  }
}

DataManagerValue::DataManagerValue(const PmidName& pmid_name, int size)
    : data_size_(size),
      subscribers_(1),
      online_pmids_(),
      offline_pmids_() {
  if (size < 1)
    ThrowError(CommonErrors::invalid_parameter);
  online_pmids_.insert(pmid_name);
}

void DataManagerValue::AddPmid(const PmidName& pmid_name) {
  online_pmids_.insert(pmid_name);
  offline_pmids_.erase(pmid_name);
}

void DataManagerValue::RemovePmid(const PmidName& pmid_name) {
  online_pmids_.erase(pmid_name);
  offline_pmids_.erase(pmid_name);
}

void DataManagerValue::Increamentsubscribers() {
  ++subscribers_;
}

void DataManagerValue::Decreamentsubscribers() {
  --subscribers_;
}

void DataManagerValue::SetPmidOnline(const PmidName& pmid_name) {
  auto deleted = offline_pmids_.erase(pmid_name);
  if (deleted == 1)
    online_pmids_.insert(pmid_name);
}

void DataManagerValue::SetPmidOffline(const PmidName& pmid_name) {
  auto deleted = online_pmids_.erase(pmid_name);
  if (deleted == 1)
    offline_pmids_.insert(pmid_name);
}

DataManagerValue::serialised_type DataManagerValue::Serialise() const {
  if ((subscribers_ <= 0) || online_pmids_.empty())
    ThrowError(CommonErrors::uninitialised);  // Cannot serialise if not a complete db value

  protobuf::DataManagerValue metadata_value_proto;
  metadata_value_proto.set_size(data_size_);
  metadata_value_proto.set_subscribers(subscribers_);
  for (const auto& i: online_pmids_)
    metadata_value_proto.add_online_pmid_name(i->string());
  for (const auto& i: offline_pmids_)
    metadata_value_proto.add_offline_pmid_name(i->string());
  assert(metadata_value_proto.IsInitialized());
  return serialised_type(NonEmptyString(metadata_value_proto.SerializeAsString()));
}

bool operator==(const DataManagerValue& lhs, const DataManagerValue& rhs) {
  return lhs.data_size_ == rhs.data_size_ &&
         lhs.subscribers_ == rhs.subscribers_ &&
         lhs.online_pmids_ == rhs.online_pmids_ &&
         lhs.offline_pmids_ == rhs.offline_pmids_;
}

} // delete_me

}  // namespace vault

}  // namespace maidsafe
