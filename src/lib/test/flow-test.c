/*
  SPDX-License-Identifier: Apache-2.0
  Copyright (c) 2021 Nordix Foundation
*/

#include <flow.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <arpa/inet.h>

#define D(x)
#define Dx(x) x

int main(int argc, char* argv[])
{
	struct FlowSet* f;
	int rc;
	char name[32];
	char port[32];
	struct ctKey key;

	// Basic
	flowSetDelete(NULL);
	f = flowSetCreate(NULL);
	assert(f != NULL);
	assert(flowSetSize(f) == 0);
	flowSetDelete(f);

	// Add and delete
	f = flowSetCreate(NULL);
	rc = flowDefine(f, "f100", 100, NULL, NULL, NULL, NULL, NULL, NULL);
	assert(rc == 0);
	rc = flowDefine(f, "f200", 200, NULL, NULL, NULL, NULL, NULL, NULL);
	assert(rc == 0);
	rc = flowDefine(f, "f300", 300, NULL, NULL, NULL, NULL, NULL, NULL);
	assert(rc == 0);
	assert(flowSetSize(f) == 3);
	flowDelete(f, "f500");		/* no-op */
	assert(flowSetSize(f) == 3);
	flowDelete(f, "f200");
	assert(flowSetSize(f) == 2);
	flowDelete(f, "f100");
	assert(flowSetSize(f) == 1);
	rc = flowDefine(f, "f200", 200, NULL, NULL, NULL, NULL, NULL, NULL);
	assert(flowSetSize(f) == 2);
	flowSetDelete(f);

	// Simple priority
	extern int flowSetIsSorted(struct FlowSet* set);
	f = flowSetCreate(NULL);
	void* user_ref = (void*)500;
	for (unsigned int i = 100; i > 0; i--) {
		sprintf(name, "flow%u", i);
		sprintf(port, "%u", i + 20);
		rc = flowDefine(
			f, name, i, user_ref++, NULL, port, NULL, NULL, NULL);
		assert(rc == 0);
	}
	assert(flowSetSize(f) == 100);
	assert(flowSetIsSorted(f));
	for (unsigned int i = 100; i > 0; i--) {
		sprintf(name, "flow%u", i);
		sprintf(port, "%u", i + 20);
		rc = flowDefine(
			f, name, 101-i, user_ref++, NULL, port, NULL, NULL, NULL);
		assert(rc == 0);
	}
	assert(flowSetSize(f) == 100);
	assert(flowSetIsSorted(f));
	flowSetDelete(f);

	// Re-prioritize
	f = flowSetCreate(NULL);
	rc = flowDefine(f, "p200", 200, (void*)1, NULL, NULL, NULL, NULL, NULL);
	assert(rc == 0);
	rc = flowDefine(f, "pX", 100, (void*)2, NULL, NULL, NULL, NULL, NULL);
	assert(rc == 0);
	assert(flowSetSize(f) == 2);
	memset(&key, 0, sizeof(key));
	assert(flowLookup(f, &key) == (void*)1);
	rc = flowDefine(f, "pX", 400, (void*)2, NULL, NULL, NULL, NULL, NULL);
	assert(rc == 0);
	assert(flowSetSize(f) == 2);
	assert(flowLookup(f, &key) == (void*)2);
	flowSetDelete(f);

	// Protocols only
	f = flowSetCreate(NULL);
	char const* ptcp[] = { "tcp", NULL };
	rc = flowDefine(f, "tcp100", 100, (void*)1, ptcp, NULL, NULL, NULL, NULL);
	assert(rc == 0);
	char const* pudp[] = { "udp", NULL };
	rc = flowDefine(f, "udp100", 100, (void*)1, pudp, NULL, NULL, NULL, NULL);
	assert(rc == 0);
	memset(&key, 0, sizeof(key));
	key.ports.proto = IPPROTO_SCTP;
	assert(flowLookup(f, &key) == NULL);
	char const* psctp[] = { "sctp", NULL };
	rc = flowDefine(f, "sctp100", 100, (void*)1, psctp, NULL, NULL, NULL, NULL);
	assert(rc == 0);
	assert(flowLookup(f, &key) == (void*)1);
	char const* ptipc[] = { "tipc", NULL };
	rc = flowDefine(f, "tipc100", 100, (void*)1, ptipc, NULL, NULL, NULL, NULL);
	assert(rc != 0);
	char const* pall[] = { "TCP", "udP", "ScTp", NULL };
	rc = flowDefine(f, "all10", 200, (void*)2, pall, NULL, NULL, NULL, NULL);
	assert(rc == 0);
	assert(flowLookup(f, &key) == (void*)2);
	flowSetDelete(f);

	// Basic address
	f = flowSetCreate(NULL);
	char const* adr01[] = {"1111:2222::5555/64", "10.10.10.10/16", NULL};
	rc = flowDefine(f, "adr01", 1, (void*)2, NULL, NULL, NULL, adr01, NULL);
	assert(rc == 0);
	memset(&key, 0, sizeof(key));
	D(flowSetPrint(stdout, f, NULL));
	assert(flowLookup(f, &key) == NULL);
	assert(inet_pton(AF_INET6, "::ffff:10.10.222.4", &key.dst) == 1);
	assert(flowLookup(f, &key) == (void*)2);
	assert(inet_pton(AF_INET6, "1111:2222:0:0:ffff::", &key.dst) == 1);
	assert(flowLookup(f, &key) == (void*)2);
	flowSetDelete(f);

	// Basic port
	f = flowSetCreate(NULL);
	char const* ports = "22-30, 1025, 20000-30000, 22000-23000, 25, 27";
	rc = flowDefine(f, "adr01", 1, (void*)2, NULL, ports, NULL, NULL, NULL);
	assert(rc == 0);
	memset(&key, 0, sizeof(key));
	assert(flowLookup(f, &key) == NULL);
	key.ports.dst = htons(23);
	assert(flowLookup(f, &key) == (void*)2);
	rc = flowDefine(f, "adr01", 1, (void*)2, NULL, ports, ports, NULL, NULL);
	assert(rc == 0);
	assert(flowSetSize(f) == 1);
	D(flowSetPrint(stdout, f, NULL));
	assert(flowLookup(f, &key) == NULL);
	key.ports.src = htons(22000);
	assert(flowLookup(f, &key) == (void*)2);
	flowSetDelete(f);

	printf("=== flow-test OK\n");
	return 0;
}
