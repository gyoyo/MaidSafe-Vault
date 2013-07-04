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

#ifndef MAIDSAFE_VAULT_TOOLS_COMMANDER_H_
#define MAIDSAFE_VAULT_TOOLS_COMMANDER_H_

#include <signal.h>

#include <string>
#include <vector>

#include "boost/filesystem/operations.hpp"
#include "boost/program_options/parsers.hpp"

#include "maidsafe/common/config.h"

#include "maidsafe/vault/tools/key_helper_operations.h"

namespace maidsafe {

namespace vault {

namespace tools {

const std::string kHelperVersion = "MaidSafe Vault KeysHelper " + kApplicationVersion;

class SelectedOperationsContainer {
 public:
  SelectedOperationsContainer()
      : do_create(false),
        do_load(false),
        do_bootstrap(false),
        do_store(false),
        do_verify(false),
        do_test(false),
        do_test_with_delete(false),
        do_generate_chunks(false),
        do_test_store_chunk(false),
        do_test_fetch_chunk(false),
        do_test_delete_chunk(false),
        do_delete(false),
        do_print(false) {}
  bool do_create, do_load, do_bootstrap, do_store, do_verify, do_test, do_test_with_delete,
       do_generate_chunks, do_test_store_chunk, do_test_fetch_chunk, do_test_delete_chunk,
       do_delete, do_print;
  bool InvalidOptions(const boost::program_options::variables_map& variables_map,
                      const std::vector<boost::asio::ip::udp::endpoint>& peer_endpoints);

 private:
  bool NoOptionsSelected() const;
  bool ConflictedOptions(const std::vector<boost::asio::ip::udp::endpoint>& peer_endpoints) const;
};

class Commander {
 public:
  explicit Commander(size_t pmids_count);
  void AnalyseCommandLineOptions(int argc, char* argv[]);

 private:
  size_t pmids_count_;
  size_t key_index_;
  int chunk_set_count_;
  int chunk_index_;
  KeyChainVector all_keychains_;
  boost::filesystem::path keys_path_;
  std::vector<boost::asio::ip::udp::endpoint> peer_endpoints_;
  SelectedOperationsContainer selected_ops_;

  boost::asio::ip::udp::endpoint GetBootstrapEndpoint(const std::string& peer);
  boost::program_options::options_description AddGenericOptions(const std::string& title);
  boost::program_options::options_description AddConfigurationOptions(const std::string& title);
  void CheckOptionValidity(boost::program_options::options_description& cmdline_options,
                           int argc,
                           char* argv[]);

  void ChooseOperations();
  void HandleKeys();
  void HandleSetupBootstraps();
  void HandleStore();
  void HandleVerify();
  void HandleDoTest(size_t client_index);
  void HandleDoTestWithDelete(size_t client_index);
  void HandleStoreChunk(size_t client_index);
  void HandleFetchChunk(size_t client_index);
  void HandleDeleteChunk(size_t client_index);
  void HandleGenerateChunks();
  void HandleDeleteKeys();

  void CreateKeys();
  void GetPathFromProgramOption(const std::string& option_name,
                                boost::program_options::variables_map& variables_map);
};

}  // namespace tools

}  // namespace vault

}  // namespace maidsafe

#endif  // MAIDSAFE_VAULT_TOOLS_COMMANDER_H_
