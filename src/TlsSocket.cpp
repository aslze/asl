#define TLSSOCKET_CPP
#include <mbedtls/version.h>
#include <mbedtls/ssl.h>
#include <mbedtls/debug.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <asl/TlsSocket.h>
#if MBEDTLS_VERSION_MAJOR < 3
#include <mbedtls/net.h>
#else
#include <mbedtls/net_sockets.h>
#endif

//#define TLS_DEBUG 3

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

// self-signed cert for testing (servers have this cert by default)

static const char asl_test_srv_crt_rsa[] =
"-----BEGIN CERTIFICATE-----\n"
"MIIDLjCCAhagAwIBAgIBATANBgkqhkiG9w0BAQsFADAyMRIwEAYDVQQDDAlsb2Nh\n"
"bGhvc3QxDjAMBgNVBAoMBWFzbHplMQwwCgYDVQQGEwNPUkcwIBcNMjAwMTAxMDAw\n"
"MDAwWhgPMjA5OTEyMzEyMzU5NTlaMDIxEjAQBgNVBAMMCWxvY2FsaG9zdDEOMAwG\n"
"A1UECgwFYXNsemUxDDAKBgNVBAYTA09SRzCCASIwDQYJKoZIhvcNAQEBBQADggEP\n"
"ADCCAQoCggEBALMXONdv3yCCORcM8sfbFGN7bPsyDuiNrPBoSRiYD59XPJnk1LSV\n"
"93lzFQmX+KeT9sq26CJjIrOrWzcnenRHVUzjLGsfuoRmtTJN9D95cbjF7MyzeK6S\n"
"wEffegcQBBp6l6VR8Rb+EWboctBxiF0wKWYhX000NdJr1bycjRHIUmZEIiavi0FP\n"
"289KuKxU0Wo2q9Ji+fztrr0l4uOHqbzQctcq5ivhVfPUupFURw/+f9aKngpCLFVE\n"
"EXbplUH9mvvAlD21ZEcEmMhbjzN3AEp7dR1dbQv0rBd9Qv/ZBNGQLGvz9186oYs7\n"
"aQSbY1fGwAGb71AhSODTviYbgOg4aX0uRDUCAwEAAaNNMEswCQYDVR0TBAIwADAd\n"
"BgNVHQ4EFgQU4woSIBbbt8SnW6DSRi5Tl6QnvUgwHwYDVR0jBBgwFoAU4woSIBbb\n"
"t8SnW6DSRi5Tl6QnvUgwDQYJKoZIhvcNAQELBQADggEBAK1C06NmU8KfhBhP5tL2\n"
"q/2xr0qxXsGzhM9pU6q2j6itGRPQJlG1Ehvz2PU0Vu0Pqhqwolv9uu4nSdsIk5fI\n"
"hAPPASTn4yuuWFG4gY06O/Nzgg5ayCysGjZEgEB/4i+Wf84WIAaPHtPaXMT3dSwL\n"
"+kuFzv6T1YD44cvFJh4FTTWlhWu/kusvWOX7QOVhAp0Xbm2byowuOJ80wdgyuqHx\n"
"kFSfcFGVLe0NpVu8GMKZ6pQwdB1C5+l4s/8XM6x5wAKWOFo+Gd64VDWxIrDtETID\n"
"CknSPJDfJCxZcmi0d8HSEdlzDuSfENEPrL+hVdGM18EBQticz5ng6bvUZdv7k7Us\n"
"Iok=\n"
"-----END CERTIFICATE-----\n";

static const char asl_test_srv_key_rsa[] =
"-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpAIBAAKCAQEAsxc412/fIII5Fwzyx9sUY3ts+zIO6I2s8GhJGJgPn1c8meTU\n"
"tJX3eXMVCZf4p5P2yrboImMis6tbNyd6dEdVTOMsax+6hGa1Mk30P3lxuMXszLN4\n"
"rpLAR996BxAEGnqXpVHxFv4RZuhy0HGIXTApZiFfTTQ10mvVvJyNEchSZkQiJq+L\n"
"QU/bz0q4rFTRajar0mL5/O2uvSXi44epvNBy1yrmK+FV89S6kVRHD/5/1oqeCkIs\n"
"VUQRdumVQf2a+8CUPbVkRwSYyFuPM3cASnt1HV1tC/SsF31C/9kE0ZAsa/P3Xzqh\n"
"iztpBJtjV8bAAZvvUCFI4NO+JhuA6DhpfS5ENQIDAQABAoIBADFYsrbaAncoqqZp\n"
"UPQ0r3eB6NOGRYlakE5lzc5TB+r11KLq5JklwVzbku5jy4YRRS0yHOBsxIERND8M\n"
"R7eGeECJUBHsWi5lRoQn6qcaxXUORGNbCGPB1+117F/Jz/ej0+kfnPii5RSf9BLv\n"
"VY2n2aBkjafuPO5P/ELOOCiwM9Qtd4TaukhZjovse+QOriQR8fXw+h6ZoHiaRAR4\n"
"xDrc6TOOv0WWxJbFB1YthNvYZTf2vti3EN8RIIsRrOsuLuQ8cVGm/glYTSsULKK+\n"
"N1/s6PJ5a6Iy+Ub0oHlxHxlDeHf5IoQ4nZ3ibses2fcdLtBvNyHtLW1UsZg8kEDo\n"
"1AOeADkCgYEA58tiEN9/mJhqO6dkUPwSDJMQQQMKSXiKeUDrEhippXlOn51QXH6e\n"
"KflphZySfI036HT5x2YJgeP+Fdp4YfnIXscrGQNyMHlMsf5gFkA4PhtGGVP+O60D\n"
"7FhV5C40uwmlC/z1x47thTC/LcDZfKGQXWnjqwM+5iojjJ+sllIN+b8CgYEAxcrl\n"
"msY+D7REmxDeNgyp91eNPhJNyh3dx0ZldskF06BDnGB98m7Pjnpq7N5S2TvT2Jta\n"
"AeUxGK/Hvp6fiIg7GZ50+hWUErD4eB2Z4B1P0dMiGYPeksCj7tMKOOckL8Enq/7Y\n"
"1BwAlwu/1T8LWMmErA2XIvKeIWCQJYpIGsgztwsCgYBUfP2xyMVpiaSvOcSHAFpT\n"
"2wcBq2oEfbt7lv4YCoVLm3vdEipIjJ56Dj84RGngnFjUkk65L6gngEMNFCTtEW7H\n"
"nTFIXMkyggRCnMXJVn8ppCdY9BSnC9lyPICSO4Vc55cRV5L+uko5UhtdQf4EP5+v\n"
"bPlfTD+RBasPhuQRprcRYQKBgQCpY9ArMufnjxzKKDIF2+ab2zEtHYPdOqK6jMFM\n"
"b3A8Ax6kB8cVHm1GufRkkyokvKX69WCqCtx3JeNMjpBV30Wt9RR9MIm1UDYauE8V\n"
"rkSzj7u+Wj79M1mxqK8yeFF3TFZraD/Nt4WR1hAd0nYnPb4PkzwCRAHE1+vbGogR\n"
"167ibQKBgQCAg1aqztbGtZ3/BjrxZACyHiHUV1q/VIHApYw32a0Np+63CJVxPD5z\n"
"LdB+RXqfmLk37emEuiRjEC3akPpHQugI+Hhb6bxGA7kpg7i0X8IieGdAHbaXcn49\n"
"Svk0j0PwEtSdW5qpDtSg4mJ3EJRB+4MgrKQ8tdDuQU7QN8gKYveC5w==\n"
"-----END RSA PRIVATE KEY-----\n";


namespace asl {

enum SocketError
{
	SOCKET_OK,
	SOCKET_BAD_INIT,
	SOCKET_BAD_DNS,
	SOCKET_BAD_CONNECT,
	SOCKET_BAD_LINE,
	SOCKET_BAD_RECV,
	SOCKET_BAD_DATA,
	SOCKET_BAD_WAIT,
	SOCKET_BAD_BIND,

	SOCKET_BAD_CONFIG = 12,
	SOCKET_BAD_CERT,
	SOCKET_BAD_SESSION,
	SOCKET_BAD_ACCEPT,
	SOCKET_BAD_HELLO,
	SOCKET_BAD_HANDSHAKE,
	SOCKET_BAD_TLS
};

static const char* messages[] = {
	"OK",
	"SOCKET_BAD_INIT",
	"SOCKET_BAD_DNS",
	"SOCKET_BAD_CONNECT",
	"SOCKET_BAD_LINE",
	"SOCKET_BAD_RECV",
	"SOCKET_BAD_DATA",
	"SOCKET_BAD_WAIT",
	"SOCKET_BAD_BIND",
	"",
	"",
	"",
	"SOCKET_BAD_CONFIG",
	"SOCKET_BAD_CERT",
	"SOCKET_BAD_SESSION",
	"SOCKET_BAD_ACCEPT",
	"SOCKET_BAD_HELLO",
	"SOCKET_BAD_HANDSHAKE",
	"SOCKET_BAD_TLS"
};

static void my_debug(void* ctx, int level, const char* file, int line, const char* str)
{
	for (const char* p = file; *p != '\0'; p++)
		if (*p == '/' || *p == '\\')
			file = p + 1;

	printf("%s:%04i: [%i] %s", file, line, level, str);
}

static void verbose_print(...) {}
//#define verbose_print printf

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

TlsSocket_::TlsSocket_()
{
	_error = 0;
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
	mbedtls_pk_init(&_core->pkey);
	mbedtls_x509_crt_init(&_core->srvcert);
	mbedtls_ssl_conf_read_timeout(&_core->conf, 0);

	int ret;
	if((ret = mbedtls_ctr_drbg_seed(&_core->ctr_drbg, mbedtls_entropy_func, &_core->entropy, NULL, 0)) != 0)
	{
		_error = SOCKET_BAD_INIT;
		return;
	}
#ifdef TLS_DEBUG
	mbedtls_ssl_conf_dbg(&_core->conf, my_debug, stdout);
	mbedtls_debug_set_threshold(TLS_DEBUG);
#endif

	/*if ((ret = mbedtls_x509_crt_parse(&_core->cacert, (const byte*)mbedtls_test_cas_pem, mbedtls_test_cas_pem_len)) < 0)
	{
		_error = true;
		return;
	}
	
	mbedtls_ssl_conf_ca_chain(&_core->conf, &_core->cacert, NULL);
	*/
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
	if (port < 0 || port > 65535)
	{
		_error = SOCKET_BAD_BIND;
		return false;
	}
	mbedtls_x509_crt_init(&_core->srvcert);
	mbedtls_pk_init(&_core->pkey);
	_core->bound = true;

	useCert(asl_test_srv_crt_rsa);
	useKey(asl_test_srv_key_rsa);

	InetAddress here(ip, port);
	String host = here.host();
	int ret = mbedtls_net_bind(&_core->net, host, String(port), MBEDTLS_NET_PROTO_TCP);
	if (ret != 0) {
		verbose_print("TlsSocket: bind failed 0x%x\n", -ret);
		_error = SOCKET_BAD_BIND;
		return false;
	}
	if ((ret = mbedtls_ssl_config_defaults(&_core->conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
	{
		verbose_print("TlsSocket: config failed 0x%x\n", -ret);
		_error = SOCKET_BAD_CONFIG;
		return false;
	}

	if ((ret = mbedtls_ssl_conf_own_cert(&_core->conf, &_core->srvcert, &_core->pkey)) != 0)
	{
		verbose_print("TlsSocket: setting certificate failed 0x%x\n", ret);
		_error = SOCKET_BAD_CERT;
		return false;
	}
	if ((ret = mbedtls_ssl_setup(&_core->ssl, &_core->conf)) != 0)
	{
		verbose_print("TlsSocket: session setup failed 0x%x\n", -ret);
		_error = SOCKET_BAD_SESSION;
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
		verbose_print("TlsSocket: setting certificate failed -0x%x\n", ret);
		_error = SOCKET_BAD_CERT;
		return cli;
	}

	ret = mbedtls_ssl_setup(&cli->_core->ssl, &cli->_core->conf);

	char ip[128];
	size_t ipsize = 0;
	ret = mbedtls_net_accept(&_core->net, &cli->_core->net, ip, sizeof(ip), &ipsize);
	if (ret != 0) {
		_error = SOCKET_BAD_ACCEPT;
		verbose_print("TLS accept failed\n");
		return cli;
	}
	ret = mbedtls_net_set_block(&cli->_core->net);

	if (ret) {
		verbose_print("TLS ssl setup failed\n");
		_error = SOCKET_BAD_SESSION;
		return cli;
	}

	mbedtls_ssl_set_bio(&cli->_core->ssl, &cli->_core->net, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

	do ret = mbedtls_ssl_handshake(&cli->_core->ssl);
#ifdef MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS
	while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE || ret == MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS);
#else
	while (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE);
#endif

	if (ret == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED)
	{
		verbose_print("TlsSocket: hello verification requested\n");
		_error = SOCKET_BAD_HELLO;
		return cli;
	}
	else if (ret != 0)
	{
		_error = SOCKET_BAD_HANDSHAKE;
		verbose_print("TlsSocket: handshake error -0x%x\n", -ret);

#if defined(MBEDTLS_X509_CRT_PARSE_C)
		if (ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED)
		{
			char vrfy_buf[512];
			unsigned flags = mbedtls_ssl_get_verify_result(&cli->_core->ssl);
			mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "", flags);
			//verbose_print("TlsSocket: auth failed: %s\n", vrfy_buf);
		}
#endif
		return cli;
	}
	else // ret == 0
	{
		//verbose_print("Protocol %s\nCiphersuite is %s\n", mbedtls_ssl_get_version(&cli->_core->ssl), mbedtls_ssl_get_ciphersuite(&cli->_core->ssl));
	}

	/*
	if ((ret = mbedtls_ssl_get_record_expansion(&cli->_core->ssl)) >= 0) // returns 29
		verbose_print("Record expansion is %d\n", ret);
	else
		verbose_print("Record expansion is unknown\n");
	*/
	//verbose_print("TLS accepted connection\n");
	return cli;
}

bool TlsSocket_::connect(const InetAddress& addr)
{
	int ret = mbedtls_net_connect(&_core->net, addr.host(), String(addr.port()), MBEDTLS_NET_PROTO_TCP);
	if (ret)
	{
		(*this) = TlsSocket_();
		_error = SOCKET_BAD_CONNECT;
		return false;
	}
	ret = mbedtls_ssl_config_defaults(&_core->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
	mbedtls_ssl_conf_authmode(&_core->conf, MBEDTLS_SSL_VERIFY_NONE);
	ret = mbedtls_ssl_setup(&_core->ssl, &_core->conf);
	if (ret) {
		(*this) = TlsSocket_();
		_error = SOCKET_BAD_TLS;
		return false;
	}
	ret = mbedtls_ssl_set_hostname(&_core->ssl, _hostname);

	while ((ret = mbedtls_ssl_handshake(&_core->ssl)) != 0)
	{
		if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE && ret != -0x7000)
		{
			(*this) = TlsSocket_();
			_error = SOCKET_BAD_TLS;
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
		if (n < 0) {_error = SOCKET_BAD_RECV; break;}
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
			m = mbedtls_ssl_write(&_core->ssl, (unsigned char*)data + written, n - written);
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

String TlsSocket_::errorMsg() const
{
	return messages[_error];
}

bool TlsSocket_::useCert(const String& cert)
{
	mbedtls_x509_crt_free(&_core->srvcert);
	mbedtls_x509_crt_init(&_core->srvcert);
	int ret = mbedtls_x509_crt_parse(&_core->srvcert, (byte*)*cert, cert.length() + 1);
	if (ret < 0) {
		verbose_print("TlsSocket: cert parse error 0x%x\n", -ret);
		_error = SOCKET_BAD_CERT;
		return false;
	}
	//char buf[500];
	//mbedtls_x509_crt_info(buf, 500, "Tls: ", &_core->srvcert);
	//printf("%s\n", buf);
	return true;
}

bool TlsSocket_::useKey(const String& key)
{
	mbedtls_pk_free(&_core->pkey);
	mbedtls_pk_init(&_core->pkey);
#if MBEDTLS_VERSION_MAJOR < 3
	int ret = mbedtls_pk_parse_key(&_core->pkey, (byte*)*key, key.length() + 1, NULL, 0);
#else
	int ret = mbedtls_pk_parse_key(&_core->pkey, (byte*)*key, key.length() + 1, NULL, 0, mbedtls_ctr_drbg_random, &_core->ctr_drbg);
#endif
	if (ret < 0) {
		verbose_print("TlsSocket: key parse error 0x%x\n", -ret);
		_error = SOCKET_BAD_CERT;
		return false;
	}
	ret = mbedtls_ssl_conf_own_cert(&_core->conf, &_core->srvcert, &_core->pkey);
	if (ret)
	{
		verbose_print("TlsSocket: setting certificate failed 0x%x\n", -ret);
		_error = SOCKET_BAD_CERT;
	}
	return ret == 0;
}

}
