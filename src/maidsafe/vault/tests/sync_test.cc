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
#include "maidsafe/vault/maid_account_holder/maid_account.pb.h"

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

  struct GenerateKeyValuePair : public boost::static_visitor<NonEmptyString>
  {
    GenerateKeyValuePair() {}

    template<typename T>
    NonEmptyString operator()(T& key) {
      protobuf::MaidAccountDbValue proto_db_value;
      proto_db_value.set_average_cost(33);
      proto_db_value.set_count(2);
      NonEmptyString value(proto_db_value.SerializeAsString());

      key.data = Identity(crypto::Hash<crypto::SHA512>(value));
      return value;
    }
  };

  NonEmptyString GenerateKeyValueData(DataNameVariant& key) {
    GenerateKeyValuePair generate_key_value_pair_;
    return boost::apply_visitor(generate_key_value_pair_, key);
  }
}

template <typename MergePolicy>
class SyncTest : public testing::Test {
 public:
  SyncTest()
      : kTestRoot_(maidsafe::test::CreateTestPath("MaidSafe_Test_Vault")),
        vault_root_directory_(*kTestRoot_ / RandomAlphaNumericString(8)),
        node_id_(RandomString(64)),
        db_(vault_root_directory_),
        account_db_(db_),
        sync_(&account_db_, node_id_) {
    boost::filesystem::create_directory(vault_root_directory_);
  }

 protected:
  typename MergePolicy::UnresolvedEntry CreateUnresolvedEntry(nfs::MessageAction action,
                                                              DataNameVariant key,
                                                              NodeId node_id) {
    return typename MergePolicy::UnresolvedEntry(std::make_pair(key, action),
                                                 32, node_id);
  }

  const maidsafe::test::TestPath kTestRoot_;
  boost::filesystem::path vault_root_directory_;
  NodeId node_id_;
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
        this->CreateUnresolvedEntry(nfs::MessageAction::kPut,
                                    GetRandomKey(),
                                    NodeId(RandomString(64))));
    EXPECT_FALSE(this->sync_.AddUnresolvedEntry(unresolved_entry));
    EXPECT_TRUE(this->sync_.AddUnresolvedEntry(unresolved_entry));
  }
//   {
//     typename TypeParam::UnresolvedEntry unresolved_entry(
//         this->CreateUnresolvedEntry(nfs::MessageAction::kPut, GetRandomKey(), this->node_id_));
//     EXPECT_FALSE(this->sync_.AddUnresolvedEntry(unresolved_entry));
//     // assertion failure
//     EXPECT_THROW(this->sync_.AddUnresolvedEntry(unresolved_entry));
//   }
  {
    DataNameVariant key(GetRandomKey());
    NonEmptyString value(GenerateKeyValueData(key));
    this->account_db_.Put(std::make_pair(key, value));

    typename TypeParam::UnresolvedEntry unresolved_entry(
        this->CreateUnresolvedEntry(nfs::MessageAction::kPut, key, NodeId(RandomString(64))));
    for (size_t i(0); i < ((routing::Parameters::node_group_size + 1U) / 2 - 1); ++i) {
      unresolved_entry.messages_contents.emplace_back();
      unresolved_entry.messages_contents.back().peer_id = NodeId(RandomString(64));
      unresolved_entry.messages_contents.back().entry_id = RandomInt32();
      unresolved_entry.messages_contents.back().value = (RandomInt32() % 64 + 16);
    }

    EXPECT_FALSE(this->sync_.AddUnresolvedEntry(unresolved_entry));
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
