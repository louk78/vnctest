#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define VERBOSE 0
#ifdef VERBOSE
int verbose = VERBOSE;
#define _dprintf(l,...) do{if (l <= verbose){printf(__VA_ARGS__);}}while(0)
#define dprintf(l,...) do{if (l <= verbose){printf( "%d/%d:%3d ", l, verbose, __LINE__);printf(__VA_ARGS__);}}while(0)
#else
#define dprintf(...) do{}while(0)
#endif

enum { handshake = 'H', security = 'S', security_result = 'R', init = 'I', normal = 'N' } state = handshake;
enum { v00, v33 = 0x0303, v37, v38 } version = v00;

ssize_t complete_read( int fd, void *buf, size_t count)
{
	ssize_t result = 0;

	dprintf( 5, "%s: about to complete read %d bytes..\n", __func__, (int)count);
	while (result != count)
	{
		int n;
		n = read( fd, buf + result, count - result);
		if (n <= 0)
			return n;
		result += n;
		dprintf( 5, "%s: read %d bytes..\n", __func__, (int)result);
	}
	dprintf( 5, "%s: done reading %d bytes [%s]\n", __func__, (int)result, (char *)buf);
	return result;
}
// way : 0=server, 1=client
int get_message( int way, char *who, int s, char *buf, int len)
{
	int result = 0;
	int n;
	static int count = 0;
	int ver;

	switch (state)
	{
		case handshake:
			dprintf( 5, "%s: reading handshake..\n", __func__);
			n = 12;
			n = complete_read( s, buf, n);
			if (n == -1)
			{
				printf( "%s ", who);
				perror( "read");
				return -__LINE__;
			}
			if (!n)
			{
				printf( "%s hangup\n", who);
				return -__LINE__;
			}
			result += n;
			int verma, vermi;
			sscanf( buf + 4, "%03d.%03d", &verma, &vermi);
			dprintf( 5, "verma=%d vermi=%d\n", verma, vermi);
			ver = verma * 1000 + vermi;
			dprintf( 4, "%c: %s read handshake returned ver %d\n", state, who, ver);
			switch (ver)
			{
				default:
					printf( "version %d unsupported !\n", ver);
					return -__LINE__;
				case 3003:
					version = v33;
					break;
				case 3007:
					version = v37;
					break;
				case 3008:
					version = v38;
					break;
				case 4001:
#if 1
					verma = 3;
					vermi = 3;
					int ver2 = verma * 1000 + vermi; 
					dprintf( 1, "spoofing ver %d to %d\n", ver, ver2);
					sprintf( buf, "RFB %03d.%03d\n", verma, vermi);
					version = v33;
#endif
					break;
			}
#if 1
					verma = 3;
					vermi = 3;
					int ver2 = verma * 1000 + vermi; 
					dprintf( 1, "spoofing ver %d to %d\n", ver, ver2);
					sprintf( buf, "RFB %03d.%03d\n", verma, vermi);
					version = v33;
#endif
			if (++count == 2)
			{
				dprintf( 4, "%c: %s switch to security\n", state, who);
				count = 0;
				state = security;
			}
			break;
		case security:
			switch (version)
			{
				case v33:
					n = 4;
					n = complete_read( s, buf, n);
					if (n == -1)
					{
						printf( "%s ", who);
						perror( "read");
						return -__LINE__;
					}
					if (!n)
					{
						printf( "%s hangup\n", who);
						return -__LINE__;
					}
					result += n;
					dprintf( 4, "%c: %s read security returned type %08" PRIx32 "\n", state, who, *(uint32_t *)(buf));
					if (!*(uint32_t *)buf)
					{
						dprintf( 5, "server refused security\n");
						uint32_t len;
						n = 4;
						n = complete_read( s, buf + result, n);
						if (n == -1)
						{
							printf( "%s ", who);
							perror( "read");
							return -__LINE__;
						}
						if (!n)
						{
							printf( "%s hangup\n", who);
							return -__LINE__;
						}
						len = *(uint32_t *)(buf + result);
						dprintf( 5, "reason len is %08" PRIx32 "\n", len);
						result += n;
						n = len;
						n = complete_read( s, buf + result, n);
						if (n == -1)
						{
							printf( "%s ", who);
							perror( "read");
							return -__LINE__;
						}
						if (!n)
						{
							printf( "%s hangup\n", who);
							return -__LINE__;
						}
						dprintf( 5, "reason is [%s]\n", buf + result);
						result += n;
					}
					if (++count == 1)
					{
						dprintf( 4, "%c: %s switch to init\n", state, who);
						count = 0;
						state = init;
					}
					break;
				case v37...v38:
					n = 1;
					n = complete_read( s, buf, n);
					if (n == -1)
					{
						printf( "%s ", who);
						perror( "read");
						return -__LINE__;
					}
					if (!n)
					{
						printf( "%s hangup\n", who);
						return -__LINE__;
					}
					result += n;
					n = buf[0];
					if (way == 0)
					{
						dprintf( 4, "%c: %s read security returned %d types\n", state, who, n);
						n = complete_read( s, buf + result, n);
						if (n == -1)
						{
							printf( "%s ", who);
							perror( "read");
							return -__LINE__;
						}
						if (!n)
						{
							printf( "%s hangup\n", who);
							return -__LINE__;
						}
						dprintf( 5, "sec type 0 is %02x\n", *(int *)(buf + result));
#if 1
						dprintf( 1, "spoofing sec type to %02x..\n", 1);
						memset( buf + result, 0x1, n);
#endif
						result += n;
					}
					else
					{
						if (!n)
						{
							dprintf( 5, "client refused security\n");
							uint32_t len;
							n = 4;
							n = complete_read( s, buf + result, n);
							if (n == -1)
							{
								printf( "%s ", who);
								perror( "read");
								return -__LINE__;
							}
							if (!n)
							{
								printf( "%s hangup\n", who);
								return -__LINE__;
							}
							len = *(uint32_t *)(buf + result);
							dprintf( 5, "reason len is %08" PRIx32 "\n", len);
							result += n;
							n = len;
							n = complete_read( s, buf + result, n);
							if (n == -1)
							{
								printf( "%s ", who);
								perror( "read");
								return -__LINE__;
							}
							if (!n)
							{
								printf( "%s hangup\n", who);
								return -__LINE__;
							}
							dprintf( 5, "reason is [%s]\n", buf + result);
							result += n;
						}
						else
						{
							dprintf( 4, "%c: %s read security returned type %d\n", state, who, n);
							dprintf( 5, "result=%d\n", result);
						}
					}
					if (++count == 2)
					{
						dprintf( 4, "%c: %s switch to security_result\n", state, who);
						count = 0;
						state = security_result;
					}
					break;
				default:
					printf( "version unknown\n");
					return -__LINE__;
			}
			break;
		case security_result:
			n = 4;
			n = complete_read( s, buf, n);
			if (n == -1)
			{
				printf( "%s ", who);
				perror( "read");
				return -__LINE__;
			}
			if (!n)
			{
				printf( "%s hangup\n", who);
				return -__LINE__;
			}
			result += n;
			dprintf( 4, "%c: %s read security result returned %08" PRIx32 "\n", state, who, *(uint32_t *)(buf));
			if (++count == 1)
			{
				dprintf( 4, "%c: %s switch to init\n", state, who);
				count = 0;
				state = init;
			}
			break;
		case init:
			switch (way)
			{
				case 1:
					n = 1;
					n = complete_read( s, buf, n);
					if (n == -1)
					{
						printf( "%s ", who);
						perror( "read");
						return -__LINE__;
					}
					if (!n)
					{
						printf( "%s hangup\n", who);
						return -__LINE__;
					}
					result += n;
					dprintf( 4, "%c: %s read init returned %d\n", state, who, buf[0]);
					break;
				case 0:
					n = 20;
					n = complete_read( s, buf, n);
					if (n == -1)
					{
						printf( "%s ", who);
						perror( "read");
						return -__LINE__;
					}
					if (!n)
					{
						printf( "%s hangup\n", who);
						return -__LINE__;
					}
					result += n;
					dprintf( 4, "%c: %s read init header returned %d\n", state, who, n);
					uint32_t len;
					n = 4;
					n = complete_read( s, buf + result, n);
					if (n == -1)
					{
						printf( "%s ", who);
						perror( "read");
						return -__LINE__;
					}
					if (!n)
					{
						printf( "%s hangup\n", who);
						return -__LINE__;
					}
					len = htonl( *(uint32_t *)(buf + result));
					result += n;
					dprintf( 4, "%c: %s read init returned len %d\n", state, who, len);
					n = len;
					n = complete_read( s, buf + result, n);
					if (n == -1)
					{
						printf( "%s ", who);
						perror( "read");
						return -__LINE__;
					}
					if (!n)
					{
						printf( "%s hangup\n", who);
						return -__LINE__;
					}
					result += n;
					dprintf( 4, "%c: %s read init returned name %d\n", state, who, n);
					break;
			}
			if (++count == 2)
			{
				dprintf( 4, "%c: %s switch to normal\n", state, who);
				count = 0;
				state = normal;
			}
			break;
		default:
			printf( "unknown state %d\n", state);
			return -__LINE__;
			break;
	}
	dprintf( 5, "%s: read %d bytes [%s]\n", __func__, (int)result, (char *)buf);
	return result;
}

int main( int argc, char *argv[])
{
	char *saddr = 0;
	int sport = 0, cport = 0;
	int sock;
	int ss, cs;
	struct sockaddr_in sa;
	struct sockaddr_in ssa;
	int use_get_message = 1;
	int arg = 1;
	int n;

#ifdef VERBOSE
	char *env = getenv( "VERBOSE");
	if (env)
	{
		sscanf( env, "%d", &verbose);
	}
#endif
	if (arg < argc)
	{
		char *port = strrchr( argv[arg], ':');
		if (port)
		{
			*port++ = 0;
			sscanf( port, "%d", &sport);
			saddr = argv[arg++];
			if (arg < argc)
			{
				sscanf( argv[arg++], "%d", &cport);
				if (arg < argc)
				{
					sscanf( argv[arg++], "%d", &use_get_message);
				}
			}
		}
	}
	if (!cport || !sport)
	{
		printf( "Usage: %s <server_addr:server_port> <client_port> [use_get_message]\n"
				"\tserver_addr:server_port\t\twe will connect to this VNC server\n"
				"\tclient_port\t\t\twe will listen for a VNC client to connect on this port\n"
				"\tuse_get_message\t\t\tshould we use get_message to read socket (default=1)\n",
				argv[0]);
		exit( 1);
	}
	if (!saddr[0])
		saddr = "127.0.0.1";
	printf( "Hello - server=%s:%d - client=%d\n", saddr, sport, cport);
	sock = socket( PF_INET, SOCK_STREAM, 0);
	int on = 1;
	setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on));
	memset( &sa, 0, sizeof( ssa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons( cport);
	sa.sin_addr.s_addr = INADDR_ANY;
	bind( sock, (struct sockaddr *)&sa, sizeof (sa));
	listen( sock, 1);
	while (1)
	{
		cs = accept( sock, 0, 0);
		if (cs == -1)
		{
			perror( "accept");
			break;
		}
		dprintf( 5, "accept returned %d\n", cs);
		ss = socket( PF_INET, SOCK_STREAM, 0);
		memset( &ssa, 0, sizeof( ssa));
		ssa.sin_family = AF_INET;
		ssa.sin_port = htons( sport);
		ssa.sin_addr.s_addr = inet_addr( saddr);
		n = connect( ss, (struct sockaddr *)&ssa, sizeof( ssa));
		if (n == -1)
		{
			perror( "connect");
			close( cs);
			break;
		}
		dprintf( 5, "connected to server !\n");
		while (1)
		{
			int m, i;
			int max = -1;
			fd_set rfds;
			unsigned char buf[1024];

			FD_ZERO( &rfds);
			if (ss != -1)
			{
				FD_SET( ss, &rfds);
				if (max < ss)
					max = ss;
			}
			if (cs != -1)
			{
				FD_SET( cs, &rfds);
				if (max < cs)
					max = cs;
			}
			dprintf( 5, "selecting..\n");
			n = select( max + 1, &rfds, 0, 0, 0);
			if (n == -1)
			{
				perror( "select");
				break;
			}
			if (FD_ISSET( cs, &rfds))
			{
				memset( buf, 0, sizeof( buf));
				if (use_get_message)
					n = get_message( 1, "client", cs, (char *)buf, sizeof( buf));
				else
					n = read( cs, buf, sizeof( buf));
				if (!n)
				{
					printf( "client hangup\n");
					break;
				}
				else if (n < 0)
				{
					printf( "cli get_message returned %d\n", n);
					break;
				}
//				dprintf( 0, "client write %3d bytes to server : %02x %02x %02x.. [%s]\n", n, buf[0], buf[1], buf[2], buf);
				dprintf( 0, "client write %3d bytes to server :", n);
				for (i = 0; i < n; i++)
				{
					_dprintf( 0, " %02x", buf[i]);
				}
				dprintf( 0, "\n");
				m = write( ss, buf, n);
				if (m != n)
				{
					printf( "AAARGH m=%d n=%d\n", m, n);
				}
			}
			if (FD_ISSET( ss, &rfds))
			{
				memset( buf, 0, sizeof( buf));
				if (use_get_message)
					n = get_message( 0, "server", ss, (char *)buf, sizeof( buf));
				else
					n = read( ss, buf, sizeof( buf));
				if (!n)
				{
					printf( "server hangup\n");
					break;
				}
				else if (n < 0)
				{
					printf( "ser get_message returned %d\n", n);
					break;
				}
//				dprintf( 0, "server write %3d bytes to client : %02x %02x %02x.. [%s]\n", n, buf[0], buf[1], buf[2], buf);
				dprintf( 0, "server write %3d bytes to client :", n);
				for (i = 0; i < n; i++)
				{
					_dprintf( 0, " %02x", buf[i]);
				}
				dprintf( 0, "\n");
				m = write( cs, buf, n);
				if (m != n)
				{
					printf( "AAARGH m=%d n=%d\n", m, n);
				}
			}
			
			static int count = 0;
			if (count++ > 5)
			{
				printf( "watchdog\n");
				break;
			}
		}
		close( ss);
		close( cs);
	}
	close( sock);

	return 0;
}
