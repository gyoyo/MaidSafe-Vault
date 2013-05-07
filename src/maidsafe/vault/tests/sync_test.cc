/***************************************************************************************************
 *  Copyright 2013 MaidSafe.net limited                                                            *
 *                                                                                                 *
 *  The following source code is property of MaidSafe.net limited and is not meant for external    *
 *  use.  The use of this code is governed by the licence file licence.txt found in the root of    *
 *  this directory and also on www.maidsafe.net.                                                   *
 *                                                                                                 *
 *  You are not free to copy, amend or otherwise use this source code without the explicit         *
 *  written permission of the board of directors of MaidSafe.net.                                  *
 **************************************************************************************************/

#include <functional>
#include <memory>

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/progress.hpp"

#include "maidsafe/common/test.h"
#include "maidsafe/common/utils.h"

#include "maidsafe/passport/types.h"

#include "maidsafe/vault/db.h"
#include "maidsafe/vault/account_db.h"
#include "maidsafe/vault/sync.h"
#include "maidsafe/vault/maid_account_holder/maid_account_merge_policy.h"
#include "maidsafe/vault/pmid_account_holder/pmid_account_merge_policy.h"

namespace maidsafe {
namespace vault {
namespace test {

const uint64_t kValueSize(36);

namespace {
  DataNameVariant GetRandomKey() {
    // Currently 15 types are defined, but...
    uint32_t number_of_types = boost::mpl::size<typename DataNameVariant::types>::type::value,
             type_number;
    type_number = RandomUint32() % number_of_types;
    switch (type_number) {
      case  0: return passport::Anmid::name_type();
      case  1: return passport::Ansmid::name_type();
      case  2: return passport::Antmid::name_type();
      case  3: return passport::Anmaid::name_type();
      case  4: return passport::Maid::name_type();
      case  5: return passport::Pmid::name_type();
      case  6: return passport::Mid::name_type();
      case  7: return passport::Smid::name_type();
      case  8: return passport::Tmid::name_type();
      case  9: return passport::Anmpid::name_type();
      case 10: return passport::Mpid::name_type();
      case 11: return ImmutableData::name_type();
      case 12: return OwnerDirectory::name_type();
      case 13: return GroupDirectory::name_type();
      case 14: return WorldDirectory::name_type();
      // default:
        // Throw something!
      //  ;
    }
    return DataNameVariant();
  }
}

template <typename MergePolicy>
class SyncTest : public testing::Test {
 public:
  SyncTest()
      : kTestRoot_(maidsafe::test::CreateTestPath("MaidSafe_Test_Vault")),
        vault_root_directory_(*kTestRoot_ / RandomAlphaNumericString(8)),
        db_(vault_root_directory_),
        account_db_(db_),
        sync_(&account_db_, NodeId(RandomString(64))) {
    boost::filesystem::create_directory(vault_root_directory_);
  }

 protected:
  typename MergePolicy::UnresolvedEntry CreateUnresolvedEntry(nfs::MessageAction action) {
    return typename MergePolicy::UnresolvedEntry(std::make_pair(GetRandomKey(), action),
                                                 32, NodeId(RandomString(64)));
  }

  const maidsafe::test::TestPath kTestRoot_;
  boost::filesystem::path vault_root_directory_;
  Db db_;
  AccountDb account_db_;
  Sync<MergePolicy> sync_;
};

TYPED_TEST_CASE_P(SyncTest);

TYPED_TEST_P(SyncTest, BEH_GetUnresolvedCount) {
  EXPECT_EQ(0, this->sync_.GetUnresolvedCount());
}

TYPED_TEST_P(SyncTest, BEH_AddUnresolvedEntry) {
  EXPECT_EQ(0, this->sync_.GetUnresolvedCount());
  {
    typename TypeParam::UnresolvedEntry unresolved_entry;
    EXPECT_FALSE(this->sync_.AddUnresolvedEntry(unresolved_entry));
  }
  {
    typename TypeParam::UnresolvedEntry unresolved_entry(
        this->CreateUnresolvedEntry(nfs::MessageAction::kPut));
    EXPECT_TRUE(this->sync_.AddUnresolvedEntry(unresolved_entry));
  }
}

REGISTER_TYPED_TEST_CASE_P(SyncTest, BEH_GetUnresolvedCount,
                                     BEH_AddUnresolvedEntry);

typedef ::testing::Types<MaidAccountMergePolicy,
                         PmidAccountMergePolicy> MergePolicies;
INSTANTIATE_TYPED_TEST_CASE_P(Sync, SyncTest, MergePolicies);

}  // namespace test
}  // namespace vault
}  // namespace maidsafe
