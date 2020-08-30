#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

#define VAP_OFFSET 8

struct session {
	uint32_t portmap;
	uint32_t subif;
};

unsigned long find_first_zero_bit(unsigned long *pool, unsigned long size)
{
	unsigned long candidate = 0;

	while (candidate < size && (*pool & (1 << candidate)))
		candidate++;

	return candidate;
}

unsigned long test_and_set_bit(unsigned long candidate, unsigned long *pool)
{
	unsigned long old_value = *pool & (1 << candidate);

	*pool |= 1 << candidate;

	return old_value;
}

void clear_bit(unsigned long handle, unsigned long *pool) {
	*pool &= ~(1 << handle);
}

unsigned long handle_alloc(unsigned long *pool, unsigned long size)
{
	unsigned long candidate;

	do {
		candidate = find_first_zero_bit(pool, size);
	} while (candidate < size && test_and_set_bit(candidate, pool));

	return candidate;
}

unsigned long handle_free(unsigned long *pool, unsigned long handle)
{
	clear_bit(handle, pool);
	return 0UL;
}

#define LOCAL_SESSIONS_NUM 7
unsigned long local_sessions = 0;
struct session *vap_to_session[LOCAL_SESSIONS_NUM] = { 0, 0, 0, 0, 0, 0, 0};

void resolve_netdev(uint32_t netdev, uint32_t *portmap, uint32_t *subif)
{
	if( netdev == 3 ) {
		*portmap = 0x0f;
		*subif = rand() % 260;
	} else {
		*portmap = 0x80;
		*subif = rand() % 190;
	}
}

int session_dst(struct session *s, char session_type)
{
	uint32_t netdev = 0;
	uint32_t portmap, subif;
	uint8_t local_session;

	switch(session_type) {
	case 'o':
		netdev = 3;
		break;
	case 'l':
	case 'r':
		netdev = 2;
		local_session = handle_alloc(&local_sessions, LOCAL_SESSIONS_NUM);
		if (local_session >= LOCAL_SESSIONS_NUM) {
			printf("Local sessions exhausted.\n");
			return -1;
		}
		s->subif |= local_session << VAP_OFFSET;
		break;
	default:
		printf("Unknown session type '%c'\n",session_type);
		return -1;
	}

	resolve_netdev(netdev, &portmap, &subif);

	s->portmap |= portmap;
	s->subif |= subif;

	return 0;
}

void local_session_free(char *local_session_number)
{
	int i = atoi(local_session_number);
	if (i < 1 || i >= LOCAL_SESSIONS_NUM) {
		printf("Session %d out of bounds to be freed.\n",i);
		return;
	}

	free(vap_to_session[i]);
	vap_to_session[i] = NULL;
	handle_free(&local_sessions, i);
}

void session_build(char session_type)
{
	struct session *s = malloc(sizeof(struct session));
	if (!s)
		exit(1);

	s->portmap = 0;
	s->subif = 0;

	if (session_dst(s, session_type)) {
		free(s);
		return;
	}

	if (s->subif >> VAP_OFFSET) {
		if ((s->subif >> VAP_OFFSET) >= ARRAY_SIZE(vap_to_session)) {
			printf("VAP %d out of bounds!\n",s->subif >> VAP_OFFSET);
			free(s);
			return;
		}

		vap_to_session[s->subif >> VAP_OFFSET] = s;
	}

	printf("Build Session type %c / %p\n",session_type, (void*)s);
}

void op(char *cmd)
{
	switch(*cmd) {
	case 'b':
		session_build(cmd[1]);
		break;
	case 'f':
		local_session_free(cmd+1);
		break;
	default:
		printf("Unknown command '%s'.\n",cmd);
	}
}

int main(int argc, char *argv[])
{
	int i;

	handle_alloc(&local_sessions, LOCAL_SESSIONS_NUM);

	if( RAND_MAX < 7823 ) {
		printf("RAND_MAX too small! (= %d )\n",RAND_MAX);
		exit(1);
	}

	for (i=1; i < argc; i++) {
		op(argv[i]);
	}
}
