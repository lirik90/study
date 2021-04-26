#include <iostream>
#include <fstream>

#include <botan-2/botan/version.h>
#include <botan-2/botan/loadstor.h>
#include <botan-2/botan/hash.h>
#include <botan-2/botan/pkcs8.h>
#include <botan-2/botan/hex.h>

#include <botan-2/botan/x509self.h>
#include <botan-2/botan/data_src.h>

#if defined(BOTAN_HAS_HMAC_DRBG)
#include <botan-2/botan/hmac_drbg.h>
#endif

#if defined(BOTAN_HAS_SYSTEM_RNG)
#include <botan-2/botan/system_rng.h>
#endif

#if defined(BOTAN_HAS_AUTO_SEEDING_RNG)
  #include <botan-2/botan/auto_rng.h>
#endif

#include <botan-2/botan/pbkdf.h>

#include "hz_net_dtls_tools.h"

namespace hz {
namespace Net {
namespace Dtls {

Tools::Tools(const std::chrono::milliseconds ocsp_timeout) :
	_ocsp_timeout{ocsp_timeout} {}

std::string Tools::init(
		const std::string &tls_policy_file_name, const std::string &crt_file_name, const std::string &key_file_name,
		const std::vector<std::string> &cert_paths)
{
	std::string rnd_type = init_rnd();

	_session_manager.reset(new Botan::TLS::Session_Manager_In_Memory(*_rng));

	_creds.reset(crt_file_name.empty() ?
			new Credentials_Manager(cert_paths) :
			new Credentials_Manager(*_rng, crt_file_name, key_file_name));

	init_policy(tls_policy_file_name);
	return rnd_type;
}

std::string Tools::init_rnd(const std::string drbg_seed)
{
	std::string type_str;
#if defined(BOTAN_HAS_HMAC_DRBG) && defined(BOTAN_HAS_SHA2_64)
	std::vector<uint8_t> seed = Botan::hex_decode(drbg_seed);
	if (seed.empty())
	{
		auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
		const uint64_t ts = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();

		seed.resize(8);
		Botan::store_be(ts, seed.data());
	}

	type_str = "rng:HMAC_DRBG with seed " + Botan::hex_encode(seed);

	// Expand out the seed to 512 bits to make the DRBG happy
	std::unique_ptr<Botan::HashFunction> sha512(Botan::HashFunction::create("SHA-512"));
	sha512->update(seed);
	seed.resize(sha512->output_length());
	sha512->final(seed.data());

	std::unique_ptr<Botan::HMAC_DRBG> drbg(new Botan::HMAC_DRBG("SHA-384"));
	drbg->initialize_with(seed.data(), seed.size());
	_rng.reset(new Botan::Serialized_RNG(drbg.release()));
#else
	if(drbg_seed != "")
		throw std::runtime_error("HMAC_DRBG disabled in build, cannot specify DRBG seed");

#if defined(BOTAN_HAS_SYSTEM_RNG)
	type_str = "rng:system";
	rng.reset(new Botan::System_RNG);
#elif defined(BOTAN_HAS_AUTO_SEEDING_RNG)
	type_str = "rng:autoseeded";
	rng.reset(new Botan::Serialized_RNG(new Botan::AutoSeeded_RNG));
#endif

#endif

	if(!_rng)
		throw std::runtime_error("No usable RNG enabled in build");

	return type_str;
}

void Tools::init_policy(const std::string &tls_policy_file_name)
{
	try {
		if (!tls_policy_file_name.empty())
		{
			std::ifstream policy_is(tls_policy_file_name);
			if (policy_is.fail())
				throw std::runtime_error("Can't open");
			else if (policy_is.peek() == std::ifstream::traits_type::eof())
				throw std::runtime_error("File empty");
			_policy.reset(new Botan::TLS::Text_Policy(policy_is));
		}
	}
	catch(const std::exception& e) {
		std::cerr << "Fail to read TLS policy: " << e.what() << ' ' << tls_policy_file_name << std::endl;
		_policy.reset(new Botan::TLS::Text_Policy(std::string()));

		try {
			std::ofstream ofs (tls_policy_file_name);
			_policy->print(ofs);
			ofs.close();
		} catch (...) {}
	}

	if (!_policy)
		_policy.reset(new Botan::TLS::Text_Policy(std::string()));
}

} // namespace Dtls
} // namespace Net
} // namespace hz

