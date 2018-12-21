#define TLSSOCKET_CPP
#include <mbedtls/net.h>
#include <mbedtls/debug.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>
#include <asl/TlsSocket.h>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#else
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#define SOCKOPT int
#else
#ifndef ADDRLEN
#define SOCKOPT char
#endif
#endif

namespace asl {

struct TlsCore
{
	mbedtls_entropy_context entropy;
	mbedtls_net_context net;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	mbedtls_x509_crt cacert;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_x509_crt srvcert;
	mbedtls_pk_context pkey;
	bool bound;
};

mbedtls_ssl_config config;
bool inited = false;

/*
static void my_debug(void *ctx, int level, const char *file, int line, const char *str)
{
	level++;
	printf("%s:%04d: %s", file, line, str);
}
*/

TlsSocket_::TlsSocket_()
{
	_error = false;
	_core = new TlsCore;
	_core->bound = false;
	mbedtls_net_init(&_core->net);
	mbedtls_ssl_init(&_core->ssl);
	mbedtls_ssl_config_init(&_core->conf);
	mbedtls_x509_crt_init(&_core->cacert);
	mbedtls_ctr_drbg_init(&_core->ctr_drbg);
	mbedtls_entropy_init(&_core->entropy);
	mbedtls_ssl_conf_rng(&_core->conf, mbedtls_ctr_drbg_random, &_core->ctr_drbg);
	mbedtls_ssl_set_bio(&_core->ssl, &_core->net, mbedtls_net_send, mbedtls_net_recv, NULL);

	int ret;
	if((ret = mbedtls_ctr_drbg_seed(&_core->ctr_drbg, mbedtls_entropy_func, &_core->entropy, NULL, 0)) != 0)
	{
		_error = true;
		return;
	}
	if((ret = mbedtls_x509_crt_parse(&_core->cacert, (const byte*)mbedtls_test_cas_pem, mbedtls_test_cas_pem_len)) < 0)
	{
		_error = true;
		return;
	}
	
	mbedtls_ssl_conf_ca_chain(&_core->conf, &_core->cacert, NULL);

	_type = TCP;
	_blocking = true;
}

TlsSocket_::~TlsSocket_()
{
	close();
	mbedtls_net_free(&_core->net);
	mbedtls_x509_crt_free(&_core->cacert);
	mbedtls_ssl_free(&_core->ssl);
	mbedtls_ssl_config_free(&_core->conf);
	mbedtls_ctr_drbg_free(&_core->ctr_drbg);
	mbedtls_entropy_free(&_core->entropy);
	if (_core->bound)
	{
		mbedtls_pk_free(&_core->pkey);
		mbedtls_x509_crt_free(&_core->srvcert);
	}
	delete _core;
}

void TlsSocket_::close()
{
	if (_handle >= 0)
		mbedtls_ssl_close_notify(&_core->ssl);
	_handle = -1;
}

int TlsSocket_::handle() const
{
	return _core->net.fd;
}

bool TlsSocket_::bind(const String& ip, int port)
{
	mbedtls_x509_crt_init(&_core->srvcert);
	mbedtls_pk_init(&_core->pkey);
	_core->bound = true;

	int ret;
	if ((ret = mbedtls_x509_crt_parse(&_core->srvcert, (const unsigned char *)mbedtls_test_srv_crt_rsa, mbedtls_test_srv_crt_rsa_len)) != 0)
	{
		printf("TlsSocket: cert parse error 0x%x\n", -ret);
		return false;
	}
	if ((ret = mbedtls_pk_parse_key(&_core->pkey, (const unsigned char *)mbedtls_test_srv_key_rsa, mbedtls_test_srv_key_rsa_len, NULL, 0)) != 0)
	{
		printf("TlsSocket: key parse error 0x%x\n", -ret);
		return false;
	}

	InetAddress here(ip, port);
	String host = here.host();
	ret = mbedtls_net_bind(&_core->net, host, String(port), MBEDTLS_NET_PROTO_TCP);
	if (ret != 0) {
		printf("TlsSocket: bind failed 0x%x\n", -ret);
		return false;
	}
	if ((ret = mbedtls_ssl_config_defaults(&_core->conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
	{
		printf("TlsSocket: config failed 0x%x\n", -ret);
		return false;
	}

	if ((ret = mbedtls_ssl_conf_own_cert(&_core->conf, &_core->srvcert, &_core->pkey)) != 0)
	{
		printf("TlsSocket: setting certificate failed 0x%x\n", ret);
		return false;
	}
	if ((ret = mbedtls_ssl_setup(&_core->ssl, &_core->conf)) != 0)
	{
		printf("TlsSocket: session setup failed 0x%x\n", -ret);
		return false;
	}

	return true;
}


void TlsSocket_::listen(int n)
{
}

Socket_* TlsSocket_::accept()
{
	int ret;
	TlsSocket_* cli = new TlsSocket_();

	mbedtls_net_free(&cli->_core->net);
	ret = mbedtls_ssl_config_defaults(&cli->_core->conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
	if ((ret = mbedtls_ssl_conf_own_cert(&cli->_core->conf, &_core->srvcert, &_core->pkey)) != 0)
	{
		printf("TlsSocket: setting certificate failed -0x%x\n", ret);
		return cli;
	}

	ret = mbedtls_ssl_setup(&cli->_core->ssl, &cli->_core->conf);

	char ip[128];
	size_t ipsize = 0;
	ret = mbedtls_net_accept(&_core->net, &cli->_core->net, ip, sizeof(ip), &ipsize);
	if (ret != 0) {
		printf("TLS accept failed\n");
		return cli;
	}
	ret = mbedtls_net_set_block(&cli->_core->net);

	if (ret) {
		printf("TLS ssl setup failed\n");
		return cli;
	}

	mbedtls_ssl_set_bio(&cli->_core->ssl, &cli->_core->net, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

	do ret = mbedtls_ssl_handshake(&cli->_core->ssl);
	while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);

	if (ret == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED)
	{
		printf("TlsSocket: hello verification requested\n");
		return cli;
	}
	else if (ret != 0)
	{
		printf("TlsSocket: handshake error -0x%x\n", -ret);

#if defined(MBEDTLS_X509_CRT_PARSE_C)
		if (ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED)
		{
			char vrfy_buf[512];
			unsigned flags = mbedtls_ssl_get_verify_result(&cli->_core->ssl);
			mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "", flags);
			//printf("TlsSocket: auth failed: %s\n", vrfy_buf);
		}
#endif
		return cli;
	}
	else // ret == 0
	{
		//printf("Protocol %s\nCiphersuite is %s\n", mbedtls_ssl_get_version(&cli->_core->ssl), mbedtls_ssl_get_ciphersuite(&cli->_core->ssl));
	}

	/*
	if ((ret = mbedtls_ssl_get_record_expansion(&cli->_core->ssl)) >= 0) // returns 29
		printf("Record expansion is %d\n", ret);
	else
		printf("Record expansion is unknown\n");
	*/
	//printf("TLS accepted connection\n");
	return cli;
}

bool TlsSocket_::connect(const InetAddress& addr)
{
	int ret = mbedtls_net_connect(&_core->net, addr.host(), String(addr.port()), MBEDTLS_NET_PROTO_TCP);
	if (ret)
	{
		(*this) = TlsSocket_();
		return false;
	}
	ret = mbedtls_ssl_config_defaults(&_core->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
	mbedtls_ssl_conf_authmode(&_core->conf, MBEDTLS_SSL_VERIFY_NONE);
	ret = mbedtls_ssl_setup(&_core->ssl, &_core->conf);
	if (ret) {
		(*this) = TlsSocket_();
		return false;
	}
	ret = mbedtls_ssl_set_hostname(&_core->ssl, _hostname);

	while ((ret = mbedtls_ssl_handshake(&_core->ssl)) != 0)
	{
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != -0x7000)
		{
			(*this) = TlsSocket_();
			return false;
		}
	}
	/*unsigned flags;
	if ((flags = mbedtls_ssl_get_verify_result(&_core->ssl)) != 0)
	{
		char vrfy_buf[512];
		mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "", flags);
		String verifyInfo = vrfy_buf;
		// wrong certificate
	}
	*/
	_handle = 0;
	return true;
}

bool TlsSocket_::setOption(int level, int opt, const void* val, int n)
{
	return setsockopt(handle(), level, opt, (const SOCKOPT*)val, n)>=0;
}

int TlsSocket_::available()
{
	mbedtls_ssl_read(&_core->ssl, NULL, 0);
	return (int) mbedtls_ssl_get_bytes_avail( &_core->ssl );
}

int TlsSocket_::read(void* data, int size)
{
	if(_blocking) {
	int s = size, n;
	while ((n = mbedtls_ssl_read(&_core->ssl, (unsigned char*)data, size)) < size)
	{
		if (n == MBEDTLS_ERR_SSL_WANT_READ || n == MBEDTLS_ERR_SSL_WANT_WRITE)
			continue;
		if (n < 0) {_error = true; break;}
		else if(n==0)
			return s - size;
		size -= n;
		data = (char*)data + n;
	}
	return s;
	}
	else {
		int n = 1, m = 0;
		do {
			n = mbedtls_ssl_read(&_core->ssl, (unsigned char*)data, size); 
			if (n > 0) m += n;
		}
		while (n > 0 || n == MBEDTLS_ERR_SSL_WANT_READ || n == MBEDTLS_ERR_SSL_WANT_WRITE);
		return m;
	}
}

int TlsSocket_::write(const void* data, int n)
{
	int written = 0;
	while (written < n)
	{
		int m;
		do {
			m = mbedtls_ssl_write(&_core->ssl, (unsigned char*)data, n - written);
			if (m == MBEDTLS_ERR_SSL_WANT_READ || m == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
			if (m <= 0)
				return written;
			else break;
		} while (1);
		written += m;
	}
	return written;
}

bool TlsSocket_::waitInput(double t)
{
	if (available() != 0)
		return true;
	//else return false;
	fd_set rset;
	struct timeval to;
	to.tv_sec = (int)floor(t);
	to.tv_usec = (int)((t - floor(t))*1e6);
	FD_ZERO(&rset);
	FD_SET(handle(), &rset);
	select(handle() + 1, &rset, 0, 0, &to);
	return FD_ISSET(handle(), &rset) != 0;
}

bool TlsSocket_::useCert(const String& cert)
{
	mbedtls_x509_crt_free(&_core->srvcert);
	mbedtls_x509_crt_init(&_core->srvcert);
	int ret = mbedtls_x509_crt_parse(&_core->srvcert, (byte*)*cert, cert.length() + 1);
	if (ret < 0) {
		printf("TlsSocket: cert parse error 0x%x\n", -ret);
		return false;
	}
	char buf[500];
	mbedtls_x509_crt_info(buf, 500, "Tls: ", &_core->srvcert);
	//printf("%s\n", buf);
	return true;
}

bool TlsSocket_::useKey(const String& key)
{
	mbedtls_pk_free(&_core->pkey);
	mbedtls_pk_init(&_core->pkey);
	int ret = mbedtls_pk_parse_key(&_core->pkey, (byte*)*key, key.length() + 1, 0, 0);
	if (ret < 0) {
		printf("TlsSocket: key parse error 0x%x\n", -ret);
		return false;
	}
	ret = mbedtls_ssl_conf_own_cert(&_core->conf, &_core->srvcert, &_core->pkey);
	if (ret) printf("TlsSocket: setting certificate failed 0x%x\n", -ret);
	return ret == 0;
}

}
