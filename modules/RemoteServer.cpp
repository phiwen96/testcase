export module RemoteServer;
import std;
export import Darwin;

using std::cout, std::endl;

#define MAX_EPOLL_EVENTS 64

inline auto sendall(int sock, char const *buf)->int
	{
		int total = 0;
		int len = strlen (buf);
		int bytesleft = len;
		int n;

		while (total < len)
		{
			n = send(sock, buf + total, bytesleft, 0);
			if (n == -1)
			{
				break;
			}
			total += n;
			bytesleft -= n;
		}

		// *len = total;

		return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
	}

export 
{
	struct remote_server_t
	{
		remote_server_t(auto ip_address, auto port)
		{
			addrinfo *servinfo{nullptr};

			auto hints = addrinfo{
				.ai_family = AF_UNSPEC,
				.ai_socktype = SOCK_STREAM};

			if (auto r = getaddrinfo(ip_address, port, &hints, &servinfo); r != 0)
			{
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
				throw;
			}

			addrinfo *i = servinfo;

			

			// loop through all the results and connect to the first we can
			for (; i != NULL; i = i->ai_next)
			{
				if ((sockid = socket(i->ai_family, i->ai_socktype,
									 i->ai_protocol)) == -1)
				{
					perror("socket");
					continue;
				}

				 /*---Add socket to epoll---*/

				if (connect(sockid, i->ai_addr, i->ai_addrlen) == -1)
				{				
					perror ("connect");

					continue;
				}



				break;

				// fcntl (sockid, F_SETFL, O_NONBLOCK | FASYNC);
			
			}

			if (i == NULL)
			{
				// std::cout << "client failed to connect" << std::endl;
				perror ("client failed to connect");
				throw;
			}

				
		}

		friend auto operator>>(remote_server_t &me, std::string& dst) -> remote_server_t &
		{
			// fcntl(me.sockid, F_SETFL, O_NONBLOCK | FASYNC);

			int len = getpagesize();
					char buf[len];
					int numbytes;

					if ((numbytes = recv(me.sockid, buf, sizeof(buf), 0)) == -1)
						{
							perror("recv error");
						}

					else 
					{
						buf [numbytes] = '\0';
						dst = buf;
					}
			

			

			

			return me;
		}

		friend auto operator<<(remote_server_t &me, std::string const& msg) -> remote_server_t &
		{
			sendall(me.sockid, msg.c_str());

			return me;
		}

	private:
		int sockid;
	};
}
