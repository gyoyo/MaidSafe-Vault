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

#include "maidsafe/vault/data_holder/data_holder_service.h"

#include <memory>

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"

#include "maidsafe/common/log.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/test.h"

#include "maidsafe/passport/passport.h"

#include "maidsafe/data_types/data_type_values.h"
#include "maidsafe/data_types/immutable_data.h"
#include "maidsafe/data_types/owner_directory.h"
#include "maidsafe/data_types/group_directory.h"
#include "maidsafe/data_types/world_directory.h"

#include "maidsafe/nfs/message.h"
#include "maidsafe/nfs/reply.h"


namespace maidsafe {
namespace vault {
namespace test {

template<typename Data>
std::pair<Identity, NonEmptyString> GetNameAndContent();

template<typename Fob>
std::pair<Identity, NonEmptyString> MakeNameAndContentPair(const Fob& fob) {
  maidsafe::passport::detail::PublicFob<typename Fob::name_type::tag_type> public_fob(fob);
  return std::make_pair(public_fob.name().data, public_fob.Serialise().data);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<passport::PublicAnmid>() {
  passport::Anmid anmid;
  return MakeNameAndContentPair(anmid);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<passport::PublicAnsmid>() {
  passport::Ansmid ansmid;
  return MakeNameAndContentPair(ansmid);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<passport::PublicAntmid>() {
  passport::Antmid antmid;
  return MakeNameAndContentPair(antmid);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<passport::PublicAnmaid>() {
  passport::Anmaid anmaid;
  return MakeNameAndContentPair(anmaid);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<passport::PublicMaid>() {
  passport::Anmaid anmaid;
  passport::Maid maid(anmaid);
  return MakeNameAndContentPair(maid);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<passport::PublicPmid>() {
  passport::Anmaid anmaid;
  passport::Maid maid(anmaid);
  passport::Pmid pmid(maid);
  return MakeNameAndContentPair(pmid);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<passport::PublicAnmpid>() {
  passport::Anmpid anmpid;
  return MakeNameAndContentPair(anmpid);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<passport::PublicMpid>() {
  passport::Anmpid anmpid;
  passport::Mpid mpid(NonEmptyString("Test"), anmpid);
  return MakeNameAndContentPair(mpid);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<passport::Mid>() {
  const passport::detail::Keyword kKeyword(RandomAlphaNumericString(20));
  const passport::detail::Password kPassword(RandomAlphaNumericString(20));
  const passport::detail::Pin kPin(std::to_string(RandomUint32() % 9999 + 1));
  const NonEmptyString kMasterData(RandomString(34567));
  auto encrypted_session(passport::EncryptSession(kKeyword, kPin, kPassword, kMasterData));
  passport::Antmid antmid;
  passport::Tmid tmid(encrypted_session, antmid);

  passport::Anmid anmid;
  passport::Mid mid(passport::MidName(kKeyword, kPin),
                    passport::EncryptTmidName(kKeyword, kPin, tmid.name()),
                    anmid);
  return std::make_pair(mid.name().data, mid.Serialise().data);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<passport::Smid>() {
  const passport::detail::Keyword kKeyword(RandomAlphaNumericString(20));
  const passport::detail::Password kPassword(RandomAlphaNumericString(20));
  const passport::detail::Pin kPin(std::to_string(RandomUint32() % 9999 + 1));
  const NonEmptyString kMasterData(RandomString(34567));
  auto encrypted_session(passport::EncryptSession(kKeyword, kPin, kPassword, kMasterData));
  passport::Antmid antmid;
  passport::Tmid tmid(encrypted_session, antmid);

  passport::Ansmid ansmid;
  passport::Smid smid(passport::SmidName(kKeyword, kPin),
                      passport::EncryptTmidName(kKeyword, kPin, tmid.name()),
                      ansmid);
  return std::make_pair(smid.name().data, smid.Serialise().data);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<passport::Tmid>() {
  const passport::detail::Keyword kKeyword(RandomAlphaNumericString(20));
  const passport::detail::Password kPassword(RandomAlphaNumericString(20));
  const passport::detail::Pin kPin(std::to_string(RandomUint32() % 9999 + 1));
  const NonEmptyString kMasterData(RandomString(34567));
  auto encrypted_session(passport::EncryptSession(kKeyword, kPin, kPassword, kMasterData));
  passport::Antmid antmid;
  passport::Tmid tmid(encrypted_session, antmid);
  return std::make_pair(tmid.name().data, tmid.Serialise().data);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<ImmutableData>() {
  NonEmptyString value(RandomString(RandomUint32() % 10000 + 10));
  ImmutableData immutable(value);
  return std::make_pair(immutable.name().data, immutable.Serialise().data);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<OwnerDirectory>() {
  NonEmptyString value(RandomString(RandomUint32() % 10000 + 10));
  Identity name(crypto::Hash<crypto::SHA512>(value));
  OwnerDirectory owner_directory(OwnerDirectory::name_type(name), value);
  return std::make_pair(owner_directory.name().data, owner_directory.Serialise().data);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<GroupDirectory>() {
  NonEmptyString value(RandomString(RandomUint32() % 10000 + 10));
  Identity name(crypto::Hash<crypto::SHA512>(value));
  GroupDirectory group_directory(GroupDirectory::name_type(name), value);
  return std::make_pair(group_directory.name().data, group_directory.Serialise().data);
}

template<>
std::pair<Identity, NonEmptyString> GetNameAndContent<WorldDirectory>() {
  NonEmptyString value(RandomString(RandomUint32() % 10000 + 10));
  Identity name(crypto::Hash<crypto::SHA512>(value));
  WorldDirectory world_directory(WorldDirectory::name_type(name), value);
  return std::make_pair(world_directory.name().data, world_directory.Serialise().data);
}


template<class T>
class DataHolderTest : public testing::Test {
 public:

  DataHolderTest()
      : vault_root_directory_(maidsafe::test::CreateTestPath("MaidSafe_Test_DataHolder")),
        passport_(),
        routing_(),
        data_holder_() {}

 protected:
  void SetUp() {
    passport_.CreateFobs();
    routing_.reset(new routing::Routing(passport_.Get<passport::Maid>(false)));
    data_holder_.reset(new DataHolderService(passport_.Get<passport::Pmid>(false),
                                             *routing_,
                                             *vault_root_directory_));
  }

  void HandlePutMessage(const nfs::Message& message,
                        const routing::ReplyFunctor& reply_functor) {
    data_holder_->HandlePutMessage<T>(message, reply_functor);
  }

  void HandleGetMessage(const nfs::Message& message,
                        const routing::ReplyFunctor& reply_functor) {
    data_holder_->HandleGetMessage<T>(message, reply_functor);
  }

  void HandleDeleteMessage(const nfs::Message& message,
                           const routing::ReplyFunctor& reply_functor) {
    data_holder_->HandleDeleteMessage<T>(message, reply_functor);
  }

  maidsafe::test::TestPath vault_root_directory_;
  passport::Passport passport_;
  std::unique_ptr<routing::Routing> routing_;
  std::unique_ptr<DataHolderService> data_holder_;
};

TYPED_TEST_CASE_P(DataHolderTest);

TYPED_TEST_P(DataHolderTest, BEH_HandlePutMessage) {
  nfs::PersonaId source(nfs::Persona::kPmidAccountHolder, NodeId(NodeId::kRandomId));
  std::pair<Identity, NonEmptyString> name_and_content(GetNameAndContent<TypeParam>());
  nfs::Message::Data data(TypeParam::name_type::tag_type::kEnumValue,
                          name_and_content.first,
                          name_and_content.second,
                          nfs::MessageAction::kPut);
  nfs::Message message(nfs::Persona::kDataHolder, source, data);
  std::string retrieved;
  for (uint32_t i = 0; i != DataHolderService::kPutRequestsRequired; ++i)
    this->HandlePutMessage(message, [&](const std::string&) {});
  this->HandleGetMessage(message, [&](const std::string& result) {
                                      retrieved = result;
                                  });
  EXPECT_NE(retrieved.find(name_and_content.second.string()), -1);
}

TYPED_TEST_P(DataHolderTest, BEH_HandleGetMessage) {
  nfs::PersonaId source(nfs::Persona::kPmidAccountHolder, NodeId(NodeId::kRandomId));
  std::pair<Identity, NonEmptyString> name_and_content(GetNameAndContent<TypeParam>());
  nfs::Message::Data data(TypeParam::name_type::tag_type::kEnumValue,
                          name_and_content.first,
                          name_and_content.second,
                          nfs::MessageAction::kGet);
  nfs::Message message(nfs::Persona::kDataHolder, source, data);
  std::string retrieved;
  this->HandleGetMessage(message, [&](const std::string& result) {
                                      retrieved = result;
                                  });
  EXPECT_EQ(retrieved, nfs::Reply(CommonErrors::unknown, message.Serialise().data).Serialise()->string());
}

TYPED_TEST_P(DataHolderTest, BEH_HandleDeleteMessage) {
  nfs::PersonaId source(nfs::Persona::kPmidAccountHolder, NodeId(NodeId::kRandomId));
  std::pair<Identity, NonEmptyString> name_and_content(GetNameAndContent<TypeParam>());
  nfs::Message::Data data(TypeParam::name_type::tag_type::kEnumValue,
                          name_and_content.first,
                          name_and_content.second,
                          nfs::MessageAction::kPut);
  nfs::Message message(nfs::Persona::kDataHolder, source, data);
  std::string retrieved;
  for (uint32_t i = 0; i != DataHolderService::kPutRequestsRequired; ++i)
    this->HandlePutMessage(message, [&](const std::string&) {});
  this->HandleGetMessage(message, [&](const std::string& result) {
                                      retrieved = result;
                                   });
  EXPECT_NE(retrieved.find(name_and_content.second.string()), -1);

  nfs::Message::Data delete_data(TypeParam::name_type::tag_type::kEnumValue,
                                 name_and_content.first,
                                 name_and_content.second,
                                 nfs::MessageAction::kDelete);
  nfs::Message delete_message(nfs::Persona::kDataHolder, source, delete_data);
  for (uint32_t i = 0; i != DataHolderService::kDeleteRequestsRequired; ++i)
    this->HandleDeleteMessage(delete_message, [&](const std::string& result) {
                                                  retrieved = result;
                                              });
  EXPECT_EQ(retrieved, nfs::Reply(CommonErrors::success).Serialise()->string());
  this->HandleGetMessage(message, [&](const std::string& result) {
                                      retrieved = result;
                                  });
  EXPECT_EQ(retrieved, nfs::Reply(CommonErrors::unknown, message.Serialise().data).Serialise()->string());
}

TYPED_TEST_P(DataHolderTest, BEH_RandomAsync) {
  typedef std::vector<std::pair<Identity, NonEmptyString>> NameContentContainer;
  typedef typename NameContentContainer::value_type value_type;

  uint32_t events(RandomUint32() % 100);
  std::vector<std::future<void>> future_puts, future_deletes, future_gets;
  NameContentContainer name_content_pairs;

  for (uint32_t i = 0; i != events; ++i) {
    nfs::PersonaId source(nfs::Persona::kPmidAccountHolder, NodeId(NodeId::kRandomId));
    std::pair<Identity, NonEmptyString> name_and_content(GetNameAndContent<TypeParam>());
    name_content_pairs.push_back(name_and_content);

    uint32_t event(RandomUint32() % 3);
    switch (event) {
      case 0: {
        if (RandomUint32() % 2 == 0) {
          value_type name_content_pair(name_content_pairs[RandomUint32() % name_content_pairs.size()]);
          nfs::Message::Data data(TypeParam::name_type::tag_type::kEnumValue,
                                  name_content_pair.first,
                                  NonEmptyString("A"),
                                  nfs::MessageAction::kDelete);
          nfs::Message message(nfs::Persona::kDataHolder, source, data);
          future_deletes.push_back(std::async([this, message] {
              for (uint32_t i = 0; i != DataHolderService::kDeleteRequestsRequired; ++i)
                  this->HandleDeleteMessage(message, [&](const std::string& result) {
                                                        assert(!result.empty());
                                                        static_cast<void>(result);
                                                      });
                                            }));
        } else {
          nfs::Message::Data data(TypeParam::name_type::tag_type::kEnumValue,
                                  name_and_content.first,
                                  NonEmptyString("A"),
                                  nfs::MessageAction::kDelete);
          nfs::Message message(nfs::Persona::kDataHolder, source, data);
          future_deletes.push_back(std::async([this, message] {
              for (uint32_t i = 0; i != DataHolderService::kDeleteRequestsRequired; ++i)
                  this->HandleDeleteMessage(message, [&](const std::string& result) {
                                                        assert(!result.empty());
                                                        static_cast<void>(result);
                                                     });
                                            }));
        }
        break;
      }
      case 1: {
        nfs::Message::Data data(TypeParam::name_type::tag_type::kEnumValue,
                                name_and_content.first,
                                name_and_content.second,
                                nfs::MessageAction::kPut);
        nfs::Message message(nfs::Persona::kDataHolder, source, data);
        future_puts.push_back(std::async([this, message] {
            for (uint32_t i = 0; i != DataHolderService::kPutRequestsRequired; ++i)
               this->HandlePutMessage(message, [&](const std::string& result) {
                                                  assert(!result.empty());
                                                  static_cast<void>(result);
                                               });
                              }));
        break;
      }
      case 2: {
        if (RandomUint32() % 2 != 0) {
          value_type name_content_pair(name_content_pairs[RandomUint32() % name_content_pairs.size()]);
          nfs::Message::Data data(TypeParam::name_type::tag_type::kEnumValue,
                                  name_content_pair.first,
                                  NonEmptyString("A"),
                                  nfs::MessageAction::kGet);
          nfs::Message message(nfs::Persona::kDataHolder, source, data);
          future_gets.push_back(std::async([this, message, name_content_pair] {
                this->HandleGetMessage(message,
                                       [&](const std::string& result)->void {
                                          assert(!result.empty());
                                          NonEmptyString serialised_result(result);
                                          nfs::Reply::serialised_type serialised_reply(serialised_result);
                                          nfs::Reply reply(serialised_reply);
                                          if (reply.IsSuccess())
                                            ASSERT_EQ(name_content_pair.second, reply.data());
                                        });
            }));
        } else {
          nfs::Message::Data data(TypeParam::name_type::tag_type::kEnumValue,
                                  name_and_content.first,
                                  NonEmptyString("A"),
                                  nfs::MessageAction::kGet);
          nfs::Message message(nfs::Persona::kDataHolder, source, data);
          future_gets.push_back(std::async([this, message, name_and_content] {
                this->HandleGetMessage(message,
                                       [&](const std::string& result)->void {
                                          assert(!result.empty());
                                          NonEmptyString serialised_result(result);
                                          nfs::Reply::serialised_type serialised_reply(serialised_result);
                                          nfs::Reply reply(serialised_reply);
                                          if (reply.IsSuccess())
                                            ASSERT_EQ(name_and_content.second, reply.data());
                                        });
            }));
        }
        break;
      }
    }
  }

  for (auto& future_put : future_puts)
    EXPECT_NO_THROW(future_put.get());

  for (auto& future_delete : future_deletes) {
    try {
      future_delete.get();
    }
    catch(const std::exception& e) {
      std::string msg(e.what());
      LOG(kError) << msg;
    }
  }

  for (auto& future_get : future_gets) {
    try {
      future_get.get();
    }
    catch(const std::exception& e) {
      std::string msg(e.what());
      LOG(kError) << msg;
    }
  }
}

REGISTER_TYPED_TEST_CASE_P(DataHolderTest,
                           BEH_HandlePutMessage,
                           BEH_HandleGetMessage,
                           BEH_HandleDeleteMessage,
                           BEH_RandomAsync);

typedef testing::Types<passport::PublicAnmid,
                       passport::PublicAnsmid,
                       passport::PublicAntmid,
                       passport::PublicAnmaid,
                       passport::PublicMaid,
                       passport::PublicPmid,
                       passport::Mid,
                       passport::Smid,
                       passport::Tmid,
                       passport::PublicAnmpid,
                       passport::PublicMpid,
                       ImmutableData,
                       OwnerDirectory,
                       GroupDirectory,
                       WorldDirectory> AllTypes;

INSTANTIATE_TYPED_TEST_CASE_P(NoCache, DataHolderTest, AllTypes);


template<class T>
class DataHolderCacheableTest : public DataHolderTest<T> {
 protected:
  NonEmptyString GetFromCache(nfs::Message& message) {
    return this->data_holder_->template GetFromCache<T>(message);
  }

  void StoreInCache(const nfs::Message& message) {
    this->data_holder_->template StoreInCache<T>(message);
  }
};

TYPED_TEST_CASE_P(DataHolderCacheableTest);

TYPED_TEST_P(DataHolderCacheableTest, BEH_StoreInCache) {
  nfs::PersonaId source(nfs::Persona::kPmidAccountHolder, NodeId(NodeId::kRandomId));
  std::pair<Identity, NonEmptyString> name_and_content(GetNameAndContent<TypeParam>());
  nfs::Message::Data data(TypeParam::name_type::tag_type::kEnumValue,
                          name_and_content.first,
                          name_and_content.second,
                          nfs::MessageAction::kPut);
  nfs::Message message(nfs::Persona::kDataHolder, source, data);
  EXPECT_THROW(this->GetFromCache(message), maidsafe_error);
  this->StoreInCache(message);
  EXPECT_EQ(message.data().content, this->GetFromCache(message));
}

REGISTER_TYPED_TEST_CASE_P(DataHolderCacheableTest, BEH_StoreInCache);

typedef testing::Types<passport::PublicAnmid,
                       passport::PublicAnsmid,
                       passport::PublicAntmid,
                       passport::PublicAnmaid,
                       passport::PublicMaid,
                       passport::PublicPmid,
                       passport::PublicAnmpid,
                       passport::PublicMpid,
                       ImmutableData,
                       OwnerDirectory,
                       GroupDirectory,
                       WorldDirectory> CacheableTypes;

INSTANTIATE_TYPED_TEST_CASE_P(Cache, DataHolderCacheableTest, CacheableTypes);


}  // namespace test
}  // namespace vault
}  // namespace maidsafe
