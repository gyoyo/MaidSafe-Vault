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

#ifndef MAIDSAFE_VAULT_MPID_ACCOUNT_HOLDER_MPID_ACCOUNT_HOLDER_H_
#define MAIDSAFE_VAULT_MPID_ACCOUNT_HOLDER_MPID_ACCOUNT_HOLDER_H_

#include <utility>

#include "maidsafe/common/types.h"
#include "maidsafe/nfs/types.h"


namespace maidsafe {

namespace nfs {

template<>
struct PersonaTypes<Persona::kMpidAccountHolder> {
  typedef DataNameVariant DbKey;
  typedef NonEmptyString DbValue;
  typedef std::pair<DbKey, MessageAction> UnresolvedEntryKey;
  typedef DbValue UnresolvedEntryValue;
  static const Persona persona = Persona::kMpidAccountHolder;
};

}  // namespace nfs


namespace vault {

typedef nfs::PersonaTypes<nfs::Persona::kMpidAccountHolder> MpidAccountHolder;

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_MPID_ACCOUNT_HOLDER_MPID_ACCOUNT_HOLDER_H_
