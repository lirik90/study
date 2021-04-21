#ifndef HZ_NET_DTLS_TOOLS_H
#define HZ_NET_DTLS_TOOLS_H

#include <botan-2/botan/rng.h>
#include <botan-2/botan/tls_session_manager.h>
#include <botan-2/botan/tls_policy.h>

#include "hz_net_dtls_credentials_manager.h"

namespace hz {
namespace Net {
namespace Dtls {

class Tools
{
public:
    Tools(const std::string& tls_policy_file_name,
          const std::string& crt_file_name = std::string(),
          const std::string& key_file_name = std::string(),
          const std::chrono::milliseconds ocsp_timeout = std::chrono::milliseconds{50},
          const std::vector<std::string>& cert_paths = {});

    std::chrono::milliseconds _ocsp_timeout;

    std::unique_ptr<Botan::RandomNumberGenerator> rng_;
    std::unique_ptr<Credentials_Manager> creds_;
    std::unique_ptr<Botan::TLS::Session_Manager_In_Memory> session_manager_;
    std::unique_ptr<Botan::TLS::Text_Policy> policy_; // TODO: read policy from file
};

} // namespace Dtls
} // namespace Net
} // namespace hz

#endif // HZ_NET_DTLS_TOOLS_H
