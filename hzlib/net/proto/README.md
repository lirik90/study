# How to generate self-signed certificate via Botan
## Generate CA cert:
```sh
mkdir certdir
botan keygen > ca_key.pem
botan gen_self_signed --ca ca_key.pem my_root_authority > certdir/ca_cert.pem
```

## Generate server cert:
```sh
botan keygen > server_key.pem
botan gen_pkcs10 server_key.pem localhost > server_csr.pem
botan sign_cert certdir/ca_cert.pem ca_key.pem server_csr.pem > server_cert.pem
```
