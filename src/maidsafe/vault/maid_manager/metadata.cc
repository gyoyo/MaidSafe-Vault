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

#include "maidsafe/vault/maid_manager/metadata.h"

#include <utility>

#include "maidsafe/vault/maid_manager/helpers.h"
#include "maidsafe/vault/maid_manager/maid_manager.pb.h"
#include "maidsafe/vault/pmid_manager/metadata.h"


namespace maidsafe {

namespace vault {

MaidManagerMetadata::MaidManagerMetadata() : total_put_data_(0), pmid_totals_() {}

MaidManagerMetadata::MaidManagerMetadata(int64_t total_put_data,
                                         const std::vector<PmidTotals>& pmid_totals)
    : total_put_data_(total_put_data),
      pmid_totals_(pmid_totals) {}

MaidManagerMetadata::MaidManagerMetadata(const MaidManagerMetadata& other)
    : total_put_data_(other.total_put_data_),
      pmid_totals_(other.pmid_totals_) {}

MaidManagerMetadata::MaidManagerMetadata(MaidManagerMetadata&& other)
    : total_put_data_(std::move(other.total_put_data_)),
      pmid_totals_(std::move(other.pmid_totals_)) {}

MaidManagerMetadata& MaidManagerMetadata::operator=(MaidManagerMetadata other) {
  swap(*this, other);
  return *this;
}

MaidManagerMetadata::MaidManagerMetadata(const std::string& serialised_metadata_value) {
  protobuf::MaidManagerMetadata maid_manager_metadata_proto;
  if (!maid_manager_metadata_proto.ParseFromString(serialised_metadata_value)) {
    LOG(kError) << "Failed to read or parse serialised maid manager value";
    ThrowError(CommonErrors::parsing_error);
  }
  total_put_data_ = maid_manager_metadata_proto.total_put_data();
  for (auto index(0); index < maid_manager_metadata_proto.pmid_totals_size(); ++index) {
    pmid_totals_.emplace_back(
        nfs::PmidRegistration::serialised_type(NonEmptyString(
            maid_manager_metadata_proto.pmid_totals(index).serialised_pmid_registration())),
        PmidManagerMetadata(PmidManagerMetadata::serialised_type(NonEmptyString(
            maid_manager_metadata_proto.pmid_totals(index).serialised_pmid_metadata()))));
  }
  if (total_put_data_ < 0)
    ThrowError(CommonErrors::invalid_parameter);
}

void MaidManagerMetadata::PutData(int32_t cost) {
  total_put_data_ += cost;
}

void MaidManagerMetadata::DeleteData(int32_t cost) {
  total_put_data_ -= cost;
  if (total_put_data_ < 0)
    ThrowError(CommonErrors::invalid_parameter);
}

void MaidManagerMetadata::RegisterPmid(const nfs::PmidRegistration& pmid_registration) {
  auto itr(Find(pmid_registration.pmid_name()));
  if (itr == std::end(pmid_totals_)) {
    nfs::PmidRegistration::serialised_type serialised_pmid_registration(
        pmid_registration.Serialise());
    pmid_totals_.emplace_back(serialised_pmid_registration,
                              PmidManagerMetadata(pmid_registration.pmid_name()));
  }
}

void MaidManagerMetadata::UnregisterPmid(const nfs::PmidRegistration& pmid_registration) {
  auto itr(Find(pmid_registration.pmid_name()));
  if (itr != std::end(pmid_totals_))
    pmid_totals_.erase(itr);
}

void MaidManagerMetadata::UpdatePmidTotals(const PmidManagerMetadata& pmid_metadata) {
  auto itr(Find(pmid_metadata.pmid_name));
  if (itr == std::end(pmid_totals_))
    ThrowError(CommonErrors::no_such_element);
  (*itr).pmid_metadata = pmid_metadata;
}

std::string MaidManagerMetadata::Serialise() const {
  protobuf::MaidManagerMetadata maid_manager_metadata_proto;
  maid_manager_metadata_proto.set_total_put_data(total_put_data_);
  for (const auto& pmid_total : pmid_totals_) {
    auto pmid_total_proto(maid_manager_metadata_proto.add_pmid_totals());
    pmid_total_proto->set_serialised_pmid_registration(
        pmid_total.serialised_pmid_registration->string());
    pmid_total_proto->set_serialised_pmid_metadata(pmid_total.pmid_metadata.Serialise()->string());
  }
  return maid_manager_metadata_proto.SerializeAsString();
}

std::vector<PmidTotals>::iterator MaidManagerMetadata::Find(const PmidName& pmid_name) {
  return std::find_if(std::begin(pmid_totals_),
                      std::end(pmid_totals_),
                      [&pmid_name](const PmidTotals& pmid_totals) {
                        return pmid_name == pmid_totals.pmid_metadata.pmid_name;
                      });
}

void swap(MaidManagerMetadata& lhs, MaidManagerMetadata& rhs) {
  using std::swap;
  swap(lhs.total_put_data_, rhs.total_put_data_);
  swap(lhs.pmid_totals_, rhs.pmid_totals_);
}

bool operator==(const MaidManagerMetadata& lhs, const MaidManagerMetadata& rhs) {
  return lhs.total_put_data_ == rhs.total_put_data_ &&
         lhs.pmid_totals_ == rhs.pmid_totals_;
}

}  // namespace vault

}  // namespace maidsafe
