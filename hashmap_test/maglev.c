#include "maglev.h"


const unsigned long long ETH_HEADER_LEN = 14;
const unsigned long long UDP_HEADER_LEN = 8;

// https://en.wikipedia.org/wiki/IPv4
const unsigned long long IPV4_PROTO_OFFSET = 9;
const unsigned long long IPV4_LENGTH_OFFSET = 2;
const unsigned long long IPV4_CHECKSUM_OFFSET = 10;
const unsigned long long IPV4_SRCDST_OFFSET = 12;
const unsigned long long IPV4_SRCDST_LEN = 8;
const unsigned long long UDP_LENGTH_OFFSET = 4;
const unsigned long long UDP_CHECKSUM_OFFSET = 6;

#define TABLE_SIZE 65537

typedef int Node;
typedef int8_t LookUpTable[TABLE_SIZE];

// MAGLEV: Connection tracking table
struct maglev_kv_pair maglev_kv_pairs[CAPACITY]__attribute__ ((aligned (4096)));
static struct maglev_hashmap map = {.pairs = maglev_kv_pairs};

// MAGLEV: Lookup table
static LookUpTable maglev_lookup;

const unsigned long long FNV_BASIS = 0xcbf29ce484222325ull;
const unsigned long long FNV_PRIME = 0x100000001b3;



static __inline__ unsigned long long fnv_1_multi(char *data, unsigned long long len, unsigned long long state) {
	for (unsigned long long i = 0; i < len; ++i) {
		state *= FNV_PRIME;
		state ^= data[i];
	}
	return state;
}
static __inline__ unsigned long long fnv_1(char *data, unsigned long long len) {
	return fnv_1_multi(data, len, FNV_BASIS);
}
static __inline__ unsigned long long fnv_1a(char *data, unsigned long long len) {
	return fnv_1_multi(data, len, FNV_BASIS);
}

static __inline__ unsigned long long fnv_1a_multi(char *data, unsigned long long len, unsigned long long state) {
	for (unsigned long long i = 0; i < len; ++i) {
		state ^= data[i];
		state *= FNV_PRIME;
	}
	return state;
}


static __inline__ unsigned long long flowhash(void *frame, unsigned long long len) {
	static int packets = 0;
	// Warning: This implementation returns 0 for invalid inputs
	char *f = (char*)frame;

	if ((len > 0) && (packets < 256)) {
		//packets++;
		//printf("len %d\n", len);
		//hexdump(f, len);
	}
	// Fail early
	if (f[ETH_HEADER_LEN] >> 4 != 4) {
        // This shitty implementation can only handle IPv4 :(
		printf("unhandled! not ipv4?\n");
		return 0;
	}

	char proto = f[ETH_HEADER_LEN + IPV4_PROTO_OFFSET];
	if (proto != 6 && proto != 17) {
        // This shitty implementation can only handle TCP and UDP
		printf("Unhandled proto %x\n", proto);
		return 0;
	}

	unsigned long long v4len = 4 * (f[ETH_HEADER_LEN] & 0b1111);
	if (len < (ETH_HEADER_LEN + v4len + 4)) {
		// Not long enough
		//printf("header len short len %d expected %d\n", len, (ETH_HEADER_LEN + v4len + 4));
		//hexdump(f, 64);
		return 0;
	}

	unsigned long long hash = FNV_BASIS;

    // Hash source/destination IP addresses
	hash = fnv_1_multi(f + ETH_HEADER_LEN + IPV4_SRCDST_OFFSET, IPV4_SRCDST_LEN, hash);

	// Hash IP protocol number
	hash = fnv_1_multi(f + ETH_HEADER_LEN + IPV4_PROTO_OFFSET, 1, hash);

    // Hash source/destination port
	hash = fnv_1_multi(f + ETH_HEADER_LEN + v4len, 4, hash);

	return hash;
}


__inline__ static void get_offset_skip(Node node, unsigned long long *offset, unsigned long long *skip) {
	*offset = fnv_1((void*)&node, sizeof(node)) % (TABLE_SIZE - 1) + 1;
	*skip = fnv_1a((void*)&node, sizeof(node)) % TABLE_SIZE;
}

__inline__ static unsigned long long get_permutation(Node node, unsigned long long j) {
	unsigned long long offset, skip;
	get_offset_skip(node, &offset, &skip);
	return (offset + j * skip) % TABLE_SIZE;
}

// Eisenbud 3.4
void populate_lut(LookUpTable lut) {
	// The nodes are meaningless, just like my life
	Node nodes[3] = {800, 273, 8255};
	unsigned long long next[3] = {0, 0, 0};

	for (unsigned long long i = 0; i < TABLE_SIZE; ++i) {
		lut[i] = -1;
	}

	unsigned long long n = 0;

	for (;;) {
		for (unsigned long long i = 0; i < 3; ++i) {
			unsigned long long c = get_permutation(nodes[i], next[i]);
			while (lut[c] >= 0) {
				next[i]++;
				c = get_permutation(nodes[i], next[i]);
			}

			lut[c] = i;
			next[i]++;
			n++;

			if (n == TABLE_SIZE) return;
		}
	}
}

void maglev_hashmap_insert(unsigned long long key, unsigned long long value)
{
	//unsigned long long hash = hash_fn(key);
	unsigned long long hash = fnv_1((char*)&key, sizeof(key));
	for (unsigned long long i = 0; i < CAPACITY; ++i) {
		unsigned long long probe = hash + i;
		struct maglev_kv_pair* pair = &map.pairs[probe % CAPACITY];
		if (pair->key == key || pair->key == 0) {
			pair->key = key;
			pair->value = value;
			break;
		}
	}
}

struct maglev_kv_pair* maglev_hashmap_get( unsigned long long key)
{
	//unsigned long long hash = hash_fn(key);
	unsigned long long hash = fnv_1((char*)&key, sizeof(key));
	for (unsigned long long i = 0; i < CAPACITY; ++i) {
		unsigned long long probe = hash + i;
		struct maglev_kv_pair *pair = &map.pairs[probe % CAPACITY];
		if (pair->key == 0) {
			return NULL;
		}
		if (pair->key == key) { // hacky, uses zero key as empty marker
			return pair;
		}
	}

	return NULL;
}




void maglev_init(void) {

	populate_lut(maglev_lookup);
}
