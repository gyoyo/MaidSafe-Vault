///*  Copyright 2012 MaidSafe.net limited

//    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
//    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
//    licence you accepted on initial access to the Software (the "Licences").

//    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
//    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
//    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
//    available at: http://www.maidsafe.net/licenses

//    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
//    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
//    OF ANY KIND, either express or implied.

//    See the Licences for the specific language governing permissions and limitations relating to
//    use of the MaidSafe Software.                                                                 */

//#include "maidsafe/vault/accumulator.h"

//#include "maidsafe/common/log.h"
//#include "maidsafe/common/test.h"
//#include "maidsafe/common/utils.h"
//#include "maidsafe/data_types/data_type_values.h"
//#include "maidsafe/nfs/types.h"


//namespace maidsafe {

//namespace vault {

//namespace test {

//namespace {

//nfs::MessageAction GenerateAction() {
//  return static_cast<nfs::MessageAction>(RandomUint32() % 3);
//}

//nfs::PersonaId GenerateSource() {
//  nfs::PersonaId source;
//  // matches Persona enum in types.h
//  source.persona = static_cast<nfs::Persona>(RandomUint32() % 7);
//  source.node_id = NodeId(NodeId::kRandomId);
//  return source;
//}
//nfs::Message MakeMessage() {
//  nfs::Message::Data data(static_cast<DataTagValue>(RandomUint32() % 13),
//                              Identity(RandomString(NodeId::kSize)),
//                              NonEmptyString(RandomString(1 + RandomUint32() % 50)),
//                              GenerateAction());
//  return nfs::Message(static_cast<nfs::Persona>(RandomUint32() % 7), GenerateSource(), data,
//                      passport::PublicPmid::Name(Identity(RandomString(NodeId::kSize))));
//}

//}  // unnamed namespace

//TEST(AccumulatorTest, BEH_PushSingleResult) {
//  nfs::Message message = MakeMessage();
//  nfs::Reply reply(CommonErrors::success);
//  Accumulator<passport::PublicMaid::Name> accumulator;
//  accumulator.PushSingleResult(message, [](const std::string&) {}, reply);
//  EXPECT_EQ(accumulator.pending_requests_.size(), 1);
//  // auto request_identity(accumulator.pending_requests_.at(0).first);
//  EXPECT_FALSE(accumulator.CheckHandled(message, reply));
//  accumulator.SetHandled(message, reply);
//  EXPECT_EQ(accumulator.pending_requests_.size(), 0);
//  EXPECT_TRUE(accumulator.CheckHandled(message, reply));
//}

//TEST(AccumulatorTest, BEH_PushSingleResultThreaded) {
//  maidsafe::test::RunInParallel(10, [] {
//      nfs::Message message = MakeMessage();
//      nfs::Reply reply(CommonErrors::success);
//      Accumulator<passport::PublicMaid::Name> accumulator;
//      accumulator.PushSingleResult(message, [](const std::string&) {}, reply);
//      EXPECT_EQ(accumulator.pending_requests_.size(), 1);
//      // auto request_identity(accumulator.pending_requests_.at(0).first);
//      EXPECT_FALSE(accumulator.CheckHandled(message, reply));
//      accumulator.SetHandled(message, reply);
//      EXPECT_EQ(accumulator.pending_requests_.size(), 0);
//      EXPECT_TRUE(accumulator.CheckHandled(message, reply));
//    });
//}

//TEST(AccumulatorTest, BEH_CheckPendingRequestsLimit) {
//  Accumulator<passport::PublicPmid::Name> accumulator;
//  //  Pending list limit 300
//  size_t pending_request_max_limit = accumulator.kMaxPendingRequestsCount_;
//  for (size_t index = 0; index < pending_request_max_limit; ++index) {
//    nfs::Message message = MakeMessage();
//    nfs::Reply reply(CommonErrors::success);
//    accumulator.PushSingleResult(message, [](const std::string&) {}, reply);
//    EXPECT_EQ(accumulator.pending_requests_.size(), (index + 1));
//  }
//  // Try to add request beyond the limit and it should fail
//  EXPECT_EQ(accumulator.pending_requests_.size(), pending_request_max_limit);

//  nfs::Message message = MakeMessage();
//  nfs::Reply reply(CommonErrors::success);
//  accumulator.PushSingleResult(message, [](const std::string&) {}, reply);
//  EXPECT_EQ(accumulator.pending_requests_.size(), pending_request_max_limit);
//}

//TEST(AccumulatorTest, BEH_CheckHandled) {
//  nfs::Message message = MakeMessage();
//  nfs::Reply reply(CommonErrors::success);
//  Accumulator<passport::PublicMaid::Name> accumulator;
//  EXPECT_FALSE(accumulator.CheckHandled(message, reply));
//  accumulator.PushSingleResult(message, [](const std::string&) {}, reply);
//  accumulator.SetHandled(message, reply);
//  EXPECT_TRUE(accumulator.CheckHandled(message, reply));
//}

//TEST(AccumulatorTest, BEH_SetHandled) {
//  nfs::Message message = MakeMessage();
//  nfs::Reply reply(CommonErrors::success);
//  Accumulator<passport::PublicPmid::Name> accumulator;
//  EXPECT_TRUE(accumulator.handled_requests_.empty());
//  accumulator.SetHandled(message, reply);
//  EXPECT_EQ(accumulator.handled_requests_.size(), 1);
//  accumulator.PushSingleResult(message, [](const std::string&) {}, reply);
//  EXPECT_TRUE(accumulator.pending_requests_.empty());
//  accumulator.SetHandled(message, reply);
//  EXPECT_EQ(accumulator.handled_requests_.size(), 2);
//  EXPECT_TRUE(accumulator.pending_requests_.empty());
//}

//TEST(AccumulatorTest, BEH_FindHandled) {
//  nfs::Message message = MakeMessage();
//  nfs::Reply reply(CommonErrors::success);
//  Accumulator<passport::PublicPmid::Name> accumulator;
//  EXPECT_TRUE(accumulator.handled_requests_.empty());
//  auto itr_handle = accumulator.FindHandled(message);
//  EXPECT_TRUE(itr_handle == accumulator.handled_requests_.end());
//  accumulator.SetHandled(message, reply);
//  EXPECT_EQ(accumulator.handled_requests_.size(), 1);
//  itr_handle = accumulator.FindHandled(message);
//  EXPECT_TRUE(itr_handle != accumulator.handled_requests_.end());
//}


//}  // namespace test

//}  // namespace vault

//}  // namespace maidsafe
