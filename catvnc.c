#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

enum { handshake = 'H', security = 'S', init = 'I', normal = 'N' } state = handshake;
enum { v00, v33 = 0x0303, v37, v38 } version = v00;

ssize_t complete_read( int fd, void *buf, size_t count)
{
	ssize_t result = 0;

	while (result != count)
	{
		int n;
		n = read( fd, buf + result, count - result);
		if (n <= 0)
			return n;
		result += n;
	}
	printf( "%s: read %d bytes [%s]\n", __func__, (int)result, (char *)buf);
	return result;
}
// way : 0=server, 1=client
int get_message( int way, char *who, int s, char *buf, int len)
{
	int result = 0;
	int n;
	static int count = 0;
	int ver;

	memset( buf, 0, len);
	switch (state)
	{
		case handshake:
			printf( "%s: reading handshake..\n", __func__);
			n = 12;
			n = complete_read( s, buf, n);
			if (n == -1)
			{
				printf( "%s ", who);
				perror( "read");
				return -1;
			}
			if (!n)
			{
				printf( "%s hangup\n", who);
				return -2;
			}
			result += n;
			int verma, vermi;
			sscanf( buf + 4, "%03d.%03d", &verma, &vermi);
			printf( "verma=%d vermi=%d\n", verma, vermi);
			ver = verma * 1000 + vermi;
			printf( "%c: %s read handshake returned ver %d\n", state, who, ver);
			switch (ver)
			{
				default:
					printf( "version %d unsupported !\n", ver);
					return -3;
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
#if 0
					verma = 3;
					vermi = 3;
					int ver2 = verma * 1000 + vermi; 
					printf( "spoofing ver %d to %d\n", ver, ver2);
					sprintf( buf, "RFB %03d.%03d\n", verma, vermi);
#endif
					version = v38;
					break;
			}
			if (++count == 2)
			{
				printf( "%c: %s switch to security\n", state, who);
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
						return -4;
					}
					if (!n)
					{
						printf( "%s hangup\n", who);
						return -5;
					}
					result += n;
					printf( "%c: %s read security returned type %08" PRIx32 "\n", state, who, *(uint32_t *)(buf));
					if (!*(uint32_t *)buf)
					{
						printf( "server refused security\n");
						uint32_t len;
						n = 4;
						n = complete_read( s, buf + result, n);
						if (n == -1)
						{
							printf( "%s ", who);
							perror( "read");
							return -8;
						}
						if (!n)
						{
							printf( "%s hangup\n", who);
							return -9;
						}
						len = *(uint32_t *)(buf + result);
						printf( "reason len is %08" PRIx32 "\n", len);
						result += n;
						n = len;
						n = complete_read( s, buf + result, n);
						if (n == -1)
						{
							printf( "%s ", who);
							perror( "read");
							return -8;
						}
						if (!n)
						{
							printf( "%s hangup\n", who);
							return -9;
						}
						printf( "reason is [%s]\n", buf + result);
						result += n;
					}
					break;
				case v37...v38:
					n = 1;
					n = complete_read( s, buf, n);
					if (n == -1)
					{
						printf( "%s ", who);
						perror( "read");
						return -6;
					}
					if (!n)
					{
						printf( "%s hangup\n", who);
						return -7;
					}
					result += n;
					n = buf[0];
					if (way == 0)
					{
						printf( "%c: %s read security returned %d types\n", state, who, n);
						n = complete_read( s, buf + result, n);
						if (n == -1)
						{
							printf( "%s ", who);
							perror( "read");
							return -8;
						}
						if (!n)
						{
							printf( "%s hangup\n", who);
							return -9;
						}
						printf( "sec type 0 is %02x\n", *(int *)(buf + result));
						memset( buf + result, 0x1, n);	// XXX: fake security types to trigger error
						result += n;
					}
					else
					{
						if (!n)
						{
							printf( "client refused security\n");
							uint32_t len;
							n = 4;
							n = complete_read( s, buf + result, n);
							if (n == -1)
							{
								printf( "%s ", who);
								perror( "read");
								return -8;
							}
							if (!n)
							{
								printf( "%s hangup\n", who);
								return -9;
							}
							len = *(uint32_t *)(buf + result);
							printf( "reason len is %08" PRIx32 "\n", len);
							result += n;
							n = len;
							n = complete_read( s, buf + result, n);
							if (n == -1)
							{
								printf( "%s ", who);
								perror( "read");
								return -8;
							}
							if (!n)
							{
								printf( "%s hangup\n", who);
								return -9;
							}
							printf( "reason is [%s]\n", buf + result);
							result += n;
						}
						else
						{
							printf( "%c: %s read security returned type %d\n", state, who, n);
							printf( "result=%d\n", result);
						}
					}
					break;
				default:
					printf( "version unknown\n");
					return -10;
			}
			if (++count == 2)
			{
				printf( "%c: %s switch to init\n", state, who);
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
						return -6;
					}
					if (!n)
					{
						printf( "%s hangup\n", who);
						return -7;
					}
					result += n;
					printf( "%c: %s read init returned %d\n", state, who, buf[0]);
					break;
				case 0:
					n = 20;
					n = complete_read( s, buf, n);
					if (n == -1)
					{
						printf( "%s ", who);
						perror( "read");
						return -6;
					}
					if (!n)
					{
						printf( "%s hangup\n", who);
						return -7;
					}
					result += n;
					printf( "%c: %s read init header returned %d\n", state, who, n);
					uint32_t len;
					n = 20;
					n = complete_read( s, buf + result, n);
					if (n == -1)
					{
						printf( "%s ", who);
						perror( "read");
						return -6;
					}
					if (!n)
					{
						printf( "%s hangup\n", who);
						return -7;
					}
					len = *(uint32_t *)(buf + result);
					result += n;
					printf( "%c: %s read init returned len %d\n", state, who, len);
					n = len;
					n = complete_read( s, buf + result, n);
					if (n == -1)
					{
						printf( "%s ", who);
						perror( "read");
						return -6;
					}
					if (!n)
					{
						printf( "%s hangup\n", who);
						return -7;
					}
					result += n;
					printf( "%c: %s read init returned name %d\n", state, who, n);
					break;
			}
			if (++count == 2)
			{
				printf( "%c: %s switch to normal\n", state, who);
				count = 0;
				state = normal;
			}
			break;
		default:
			printf( "unknown state %d\n", state);
			return -11;
			break;
	}
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
//	struct sockaddr_in csa;
	int arg = 1;
	int n;

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
			}
		}
	}
	if (!cport || !sport)
	{
		printf( "Usage: %s <server_addr:server_port> <client_port>\n"
				"\tserver_addr:server_port\t\twe will connect to this VNC server\n"
				"\tclient_port\t\t\twe will listen for a VNC client to connect on this port\n",
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
		printf( "accept returned %d\n", cs);
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
		printf( "connected to server !\n");
		while (1)
		{
			int m;
			int max = -1;
			fd_set rfds;
			char buf[1024];

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
			printf( "selecting..\n");
			n = select( max + 1, &rfds, 0, 0, 0);
			if (n == -1)
			{
				perror( "select");
				break;
			}
			if (FD_ISSET( cs, &rfds))
			{
				n = get_message( 1, "client", cs, buf, sizeof( buf));
				if (n < 0)
				{
					printf( "cli get_message returned %d\n", n);
					break;
				}
				m = write( ss, buf, n);
				if (m != n)
				{
					printf( "AAARGH m=%d n=%d\n", m, n);
				}
			}
			if (FD_ISSET( ss, &rfds))
			{
				n = get_message( 0, "server", ss, buf, sizeof( buf));
				if (n < 0)
				{
					printf( "ser get_message returned %d\n", n);
					break;
				}
				m = write( cs, buf, n);
				if (m != n)
				{
					printf( "AAARGH m=%d n=%d\n", m, n);
				}
			}
		}
		close( ss);
		close( cs);
	}
	close( sock);

	return 0;
}
