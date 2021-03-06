/*  Copyright 2013 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

#ifndef MAIDSAFE_VAULT_MAID_MANAGER_ACTION_REGISTER_UNREGISTER_PMID_H_
#define MAIDSAFE_VAULT_MAID_MANAGER_ACTION_REGISTER_UNREGISTER_PMID_H_

#include <string>

#include "boost/optional/optional.hpp"

#include "maidsafe/nfs/types.h"

#include "maidsafe/nfs/vault/pmid_registration.h"

#include "maidsafe/vault/maid_manager/action_register_unregister_pmid.pb.h"


namespace maidsafe {

namespace vault {

class MaidManagerMetadata;

template<bool Unregister>
struct ActionRegisterUnregisterPmid {
  explicit ActionRegisterUnregisterPmid(const nfs_vault::PmidRegistration& pmid_registration);
  explicit ActionRegisterUnregisterPmid(const std::string& serialised_action);
  ActionRegisterUnregisterPmid(const ActionRegisterUnregisterPmid& other);
  ActionRegisterUnregisterPmid(ActionRegisterUnregisterPmid&& other);
  std::string Serialise() const;

  void operator()(MaidManagerMetadata& metadata) const;

  static const nfs::MessageAction kActionId;
  const nfs_vault::PmidRegistration kPmidRegistration;

 private:
  ActionRegisterUnregisterPmid();
  ActionRegisterUnregisterPmid& operator=(ActionRegisterUnregisterPmid other);
};

//template<>
//const nfs::MessageAction ActionRegisterUnregisterPmid<false>::kActionId =
//    nfs::MessageAction::kRegisterPmidRequest;

//template<>
//const nfs::MessageAction ActionRegisterUnregisterPmid<true>::kActionId =
//    nfs::MessageAction::kUnregisterPmidRequest;

template<>
void ActionRegisterUnregisterPmid<false>::operator()(MaidManagerMetadata& metadata) const;

template<>
void ActionRegisterUnregisterPmid<true>::operator()(MaidManagerMetadata& metadata) const;

template<bool Unregister>
bool operator==(const ActionRegisterUnregisterPmid<Unregister>& lhs,
                const ActionRegisterUnregisterPmid<Unregister>& rhs);

template<bool Unregister>
bool operator!=(const ActionRegisterUnregisterPmid<Unregister>& lhs,
                const ActionRegisterUnregisterPmid<Unregister>& rhs);

typedef ActionRegisterUnregisterPmid<false> ActionRegisterPmid;
typedef ActionRegisterUnregisterPmid<true> ActionUnregisterPmid;



// ==================== Implementation =============================================================
template<bool Unregister>
ActionRegisterUnregisterPmid<Unregister>::ActionRegisterUnregisterPmid(
    const nfs_vault::PmidRegistration& pmid_registration_in)
        : kPmidRegistration(pmid_registration_in) {}

template<bool Unregister>
ActionRegisterUnregisterPmid<Unregister>::ActionRegisterUnregisterPmid(
    const std::string& serialised_action)
        : kPmidRegistration([&serialised_action]()->std::string {
            protobuf::ActionRegisterUnregisterPmid action_register_pmid_proto;
            if (!action_register_pmid_proto.ParseFromString(serialised_action))
              ThrowError(CommonErrors::parsing_error);
            return action_register_pmid_proto.serialised_pmid_registration();
          }()) {
  assert(kPmidRegistration.unregister() == Unregister);
}

template<bool Unregister>
ActionRegisterUnregisterPmid<Unregister>::ActionRegisterUnregisterPmid(
    const ActionRegisterUnregisterPmid& other)
        : kPmidRegistration(other.kPmidRegistration) {}

template<bool Unregister>
ActionRegisterUnregisterPmid<Unregister>::ActionRegisterUnregisterPmid(
    ActionRegisterUnregisterPmid&& other)
        : kPmidRegistration(std::move(other.kPmidRegistration)) {}

template<bool Unregister>
std::string ActionRegisterUnregisterPmid<Unregister>::Serialise() const {
  protobuf::ActionRegisterUnregisterPmid action_register_pmid_proto;
  action_register_pmid_proto.set_serialised_pmid_registration(kPmidRegistration.Serialise());
  return action_register_pmid_proto.SerializeAsString();
}

template<bool Unregister>
bool operator==(const ActionRegisterUnregisterPmid<Unregister>& lhs,
                const ActionRegisterUnregisterPmid<Unregister>& rhs) {
  return lhs.kPmidRegistration.maid_name() == rhs.kPmidRegistration.maid_name() &&
         lhs.kPmidRegistration.pmid_name() == rhs.kPmidRegistration.pmid_name() &&
         lhs.kPmidRegistration.unregister() == rhs.kPmidRegistration.unregister();
}

template<bool Unregister>
bool operator!=(const ActionRegisterUnregisterPmid<Unregister>& lhs,
                const ActionRegisterUnregisterPmid<Unregister>& rhs) {
  return !operator==(lhs, rhs);
}

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_MAID_MANAGER_ACTION_REGISTER_UNREGISTER_PMID_H_
