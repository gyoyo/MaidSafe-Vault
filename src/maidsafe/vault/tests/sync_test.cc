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

namespace maidsafe {
namespace vault {
namespace test {

const uint64_t kValueSize(36);

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

REGISTER_TYPED_TEST_CASE_P(SyncTest, BEH_GetUnresolvedCount);

typedef ::testing::Types<MaidAccountMergePolicy> MergePolicies;
INSTANTIATE_TYPED_TEST_CASE_P(Sync, SyncTest, MergePolicies);

}  // namespace test
}  // namespace vault
}  // namespace maidsafe
