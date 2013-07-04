/* Copyright 2013 MaidSafe.net limited

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

#ifndef MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HOLDER_H_
#define MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HOLDER_H_

#include <cstdint>
#include <utility>

#include "maidsafe/common/types.h"
#include "maidsafe/nfs/types.h"
#include "maidsafe/vault/db_key.h"

namespace maidsafe {

namespace nfs {

template<>
struct PersonaTypes<Persona::kPmidAccountHolder> {
  // typedef DataNameVariant DbKey;
  typedef vault::DbKey DbKey;
  typedef int32_t DbValue;
  typedef std::pair<DbKey, MessageAction> UnresolvedEntryKey;
  typedef DbValue UnresolvedEntryValue;
  static const Persona persona = Persona::kPmidAccountHolder;
};

}  // namespace nfs


namespace vault {

typedef nfs::PersonaTypes<nfs::Persona::kPmidAccountHolder> PmidAccountHolder;

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_PMID_ACCOUNT_HOLDER_PMID_ACCOUNT_HOLDER_H_
