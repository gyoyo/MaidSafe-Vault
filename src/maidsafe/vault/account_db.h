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

#ifndef MAIDSAFE_VAULT_ACCOUNT_DB_H_
#define MAIDSAFE_VAULT_ACCOUNT_DB_H_

#include <vector>

#include "maidsafe/common/types.h"
#include "maidsafe/vault/db.h"


namespace maidsafe {

namespace vault {

class AccountDb {
 public:
  explicit AccountDb(Db& db);
  ~AccountDb();
  void Put(const Db::KvPair& key_value_pair);
  void Delete(const Db::KvPair::first_type& key);
  NonEmptyString Get(const Db::KvPair::first_type& key);
  std::vector<Db::KvPair> Get();

 private:
  Db& db_;
  Db::AccountId account_id_;
};

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_ACCOUNT_DB_H_
