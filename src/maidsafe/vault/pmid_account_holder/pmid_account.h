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

#ifndef MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_H_
#define MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_H_

#include <cstdint>
#include <deque>
#include <vector>
#include <utility>

#include "boost/filesystem/path.hpp"

#include "maidsafe/common/types.h"
#include "maidsafe/data_types/data_name_variant.h"
#include "maidsafe/nfs/types.h"

#include "maidsafe/vault/pmid_account_holder/pmid_account_merge_policy.h"
#include "maidsafe/vault/pmid_account_holder/pmid_account.pb.h"
#include "maidsafe/vault/pmid_account_holder/pmid_record.h"
#include "maidsafe/vault/sync.h"
#include "maidsafe/vault/types.h"


namespace maidsafe {

namespace vault {

class Db;
class AccountDb;

class PmidAccount {
 public:
  typedef PmidName name_type;
  typedef TaggedValue<NonEmptyString, struct SerialisedPmidAccountTag> serialised_type;

  enum class DataHolderStatus : int32_t { kDown, kGoingDown, kUp, kGoingUp };

  PmidAccount(const PmidName& pmid_name, Db &db, const NodeId &this_node_id);
  PmidAccount(const PmidName& pmid_name,
              Db& db,
              const NodeId& this_node_id,
              const NodeId& source_id,
              const serialised_type& serialised_pmid_account_details);

  PmidAccount(PmidAccount&& other);
  PmidAccount& operator=(PmidAccount&& other);

  serialised_type Serialise();

  void SetDataHolderUp() { data_holder_status_ = DataHolderStatus::kUp; }
  void SetDataHolderDown() { data_holder_status_ = DataHolderStatus::kDown; }

  void PutData(int32_t size);
  template<typename Data>
  void DeleteData(const typename Data::name_type& name);

  bool ApplyAccountTransfer(const NodeId& source_id,
                            const serialised_type& serialised_pmid_account_details);

  void AddLocalUnresolvedEntry(const PmidAccountUnresolvedEntry& unresolved_entry);
  NonEmptyString GetSyncData();
  void ApplySyncData(const NonEmptyString& serialised_unresolved_entries);
  void ReplaceNodeInSyncList(const NodeId& old_node, const NodeId& new_node);
  void IncrementSyncAttempts();

  PmidRecord pmid_record() const;
  name_type name() const;
  DataHolderStatus data_holder_status() const;
  int64_t total_data_stored_by_pmids() const;

 private:
  PmidAccount(const PmidAccount&);
  PmidAccount& operator=(const PmidAccount&);

  name_type pmid_name_;
  PmidRecord pmid_record_;
  DataHolderStatus data_holder_status_;
  std::unique_ptr<AccountDb> account_db_;
  Sync<PmidAccountMergePolicy> sync_;
  uint16_t account_transfer_nodes_;
  static const size_t kSyncTriggerCount_;
};

}  // namespace vault

}  // namespace maidsafe

#include "maidsafe/vault/pmid_account_holder/pmid_account-inl.h"

#endif  // MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_H_
