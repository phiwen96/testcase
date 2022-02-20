export module Common;

import std;
import Darwin;

// using namespace std;
// using namespace nlohmann;

export{
	// helper function to generate an access token
	inline auto random_int (auto min, auto max) noexcept
	{
		// One engine instance per thread
		static thread_local auto engine = std::default_random_engine{std::random_device{}()};
		auto dist = std::uniform_int_distribution<>{min, max};
		return dist(engine);
	}


	// get sockaddr, IPv4 or IPv6
	inline auto get_in_addr(sockaddr * sa)->void *
	{
		if (sa->sa_family == AF_INET)
		{
			return &(((struct sockaddr_in *)sa)->sin_addr);
		}

		return &(((struct sockaddr_in6 *)sa)->sin6_addr);
	}

	inline auto get_socket(char const *hostname, char const *port)
	{
		int sockfd, numbytes;
		// char buf[MAXDATASIZE];
		struct addrinfo hints, *servinfo, *p;
		int rv;
		char s[INET6_ADDRSTRLEN];

		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;

		if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0)
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}

		// loop through all the results and connect to the first we can
		for (p = servinfo; p != NULL; p = p->ai_next)
		{
			if ((sockfd = socket(p->ai_family, p->ai_socktype,
								 p->ai_protocol)) == -1)
			{
				perror("client: socket");
				continue;
			}

			if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
			{
				close(sockfd);
				perror("client: connect");
				continue;
			}

			break;
		}

		if (p == NULL)
		{
			fprintf(stderr, "client: failed to connect\n");
			return 2;
		}

		// inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
		// 		  s, sizeof s);
		// printf("client: connecting to %s\n", s);

		freeaddrinfo(servinfo); // all done with this structure

		return sockfd;
	}

	template <typename T, typename U>
	concept Same_as = std::is_same_v<T, U>;

	template <typename T>
	concept String = requires(T a, T && b)
	{
		std::string{a};
		std::string{std::forward<T &&>(b)};
	};

#define fwd(x) std::forward<x>(x)

	inline auto sendall(int sock, char const *buf)->int
	{
		int len = strlen(buf);
		int total = 0;
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

	inline auto nr_of_threads() noexcept->auto
	{
		return std::thread::hardware_concurrency();
	}
}
