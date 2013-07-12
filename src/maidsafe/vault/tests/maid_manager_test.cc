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

#include "maidsafe/vault/maid_manager/service.h"

#include <vector>

#include "maidsafe/common/test.h"

#include "maidsafe/nfs/public_key_getter.h"

#include "maidsafe/passport/passport.h"

#include "maidsafe/vault/mock_routing/mock_routing.h"

namespace maidsafe {

namespace vault {

namespace test {

typedef std::vector<passport::PublicPmid> PublicPmidVector;


class MaidManagerTest : public testing::Test {
 public:
  MaidManagerTest()
      : vault_root_directory_(maidsafe::test::CreateTestPath("MaidSafe_Test_MaidAccountHandler")),
        db_(*vault_root_directory_),
        mock_routing_(),
        public_key_getter_(),
        maid_manager_service_() {
    passport_.CreateFobs();
    mock_routing_.reset(new MockRouting(passport_.Get<passport::Maid>(false)));
    public_key_getter_.reset(new nfs::PublicKeyGetter(*mock_routing_, PublicPmidVector()));
    maid_manager_service_.reset(new MaidManagerService(passport_.Get<passport::Pmid>(false),
                                                       *mock_routing_,
                                                       *public_key_getter_,
                                                       db_));
  }

  ~MaidManagerTest() {}

  MaidName GenerateMaidName() {
    return MaidName(Identity(RandomAlphaNumericString(64)));
  }

 protected:
  maidsafe::test::TestPath vault_root_directory_;
  Db db_;
  maidsafe::passport::Passport passport_;
  std::unique_ptr<MockRouting> mock_routing_;
  std::unique_ptr<nfs::PublicKeyGetter> public_key_getter_;
  std::unique_ptr<MaidManagerService> maid_manager_service_;
};

TEST_F(MaidManagerTest, BEH_DefaultPaymentFactor) {
  EXPECT_EQ(4, maid_manager_service_->DefaultPaymentFactor());
}

}  //  namespace test

}  //  namespace vault

}  //  namespace maidsafe
