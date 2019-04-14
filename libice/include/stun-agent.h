#ifndef _stun_agent_h_
#define _stun_agent_h_

#include "sys/sock.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct stun_agent_t stun_agent_t;
typedef struct stun_transaction_t stun_transaction_t;

enum { STUN_RFC_3489, STUN_RFC_5389 };

/// @param[in] req transaction request
/// @param[in] resp transaction response, NULL if timeout
/// @param[in] code http like code, 2xx-ok, 4xx/5xx-error
/// @param[in] phrase error code phrase
/// @return 0-ok, other-error
typedef int (*stun_transaction_handler)(void* param, const stun_transaction_t* req, const stun_transaction_t* resp, int code, const char* phrase);

/// @param[in] rfc STUN rfc version: STUN_RFC_3489/STUN_RFC_5389
stun_transaction_t* stun_transaction_create(stun_agent_t* stun, int rfc, stun_transaction_handler handler, void* param);

/// @param[in] protocol 1-UDP, 2-TCP
int stun_transaction_setaddr(stun_transaction_t* req, int protocol, const struct sockaddr_storage* local, const struct sockaddr_storage* remote);
int stun_transaction_getaddr(stun_transaction_t* req, int* protocol, struct sockaddr_storage* local, struct sockaddr_storage* remote, struct sockaddr_storage* reflexive);

/// @param[in] credential 0-Short-Term Credential Mechanism, 1-Long-Term Credential Mechanism
/// @param[in] realm Long-Term Credential only
/// @param[in] nonce Long-Term Credential only
/// @return 0-ok, other-error
int stun_transaction_setauth(stun_transaction_t* req, int credential, const char* usr, const char* pwd, const char* realm, const char* nonce);
int stun_transaction_getauth(stun_transaction_t* req, int *credential, char usr[256], char pwd[256], char realm[256], char nonce[256]);


struct stun_agent_handler_t
{
	/// @return 0-ok, other-error
	int (*send)(void* param, int protocol, const struct sockaddr_storage* local, const struct sockaddr_storage* remote, const void* data, int bytes);

	/// @param[out] pwd password of the usr
	/// @param[out] cred STUN_CREDENTIAL_SHORT_TERM/STUN_CREDENTIAL_LONG_TERM
	/// @return 0-ok, other-error
	int (*auth)(void* param, const char* usr, const char* realm, const char* nonce, char pwd[256], int *cred);
	
	// stun
	int (*onbind)(void* param, const stun_transaction_t* req, struct stun_transaction_t* resp);
	int (*onsharedsecret)(void* param, const stun_transaction_t* req, struct stun_transaction_t* resp);
	int (*onindication)(void* param, const stun_transaction_t* req);

	// turn
	int (*onallocate)(void* param, const stun_transaction_t* req, struct stun_transaction_t* resp);
	int (*onrefresh)(void* param, const stun_transaction_t* req, int expired);
	int (*onpermission)(void* param, const stun_transaction_t* req, const struct sockaddr_storage* peer, struct stun_transaction_t* resp);
	int (*onchannel)(void* param, const stun_transaction_t* req, const struct sockaddr_storage* peer, uint16_t channel, struct stun_transaction_t* resp);
	int (*onsend)(void* param, const stun_transaction_t* req, const struct sockaddr* peer, const void* data, int bytes);
};

stun_agent_t* stun_agent_create(struct stun_agent_handler_t* handler, void* param);
int stun_agent_destroy(stun_agent_t** stun);

int stun_agent_input(stun_agent_t* stun, int protocol, const struct sockaddr_storage* local, const struct sockaddr_storage* remote, const void* data, int bytes);

// STUN
int stun_agent_bind(stun_transaction_t* req);
int stun_agent_shared_secret(stun_transaction_t* req);
int stun_agent_indication(stun_transaction_t* req);

// TURN
int turn_agent_allocate(stun_transaction_t* req);
int turn_agent_refresh(stun_transaction_t* req, int expired);
int turn_agent_permission(stun_transaction_t* req, const struct sockaddr_storage* peer);
int turn_agent_channel(stun_transaction_t* req, const struct sockaddr_storage* peer, uint16_t channel);
int turn_agent_send(stun_transaction_t* req, const struct sockaddr_storage* peer, const void* data, int bytes);

// RESPONSE

/// ignore request
int stun_agent_discard(const struct stun_transaction_t* resp);
int stun_agent_bind_response(const struct stun_transaction_t* resp, int code);
int stun_agent_shared_secret_response(const struct stun_transaction_t* resp, int code, const char* usr, const char* pwd);
int turn_agent_allocate_response(const struct stun_transaction_t* resp, int code);
int turn_agent_refresh_response(const struct stun_transaction_t* resp, int code);
int turn_agent_permission_response(const struct stun_transaction_t* resp, int code);
int turn_agent_channel_response(const struct stun_transaction_t* resp, int code);

#ifdef __cplusplus
}
#endif

#endif /* !_stun_agent_h_ */