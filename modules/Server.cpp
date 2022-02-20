#include <signal.h>
#include <poll.h>
#include <fcntl.h>

export module Server;
export import Common;
export import RemoteClient;
import std;
import Darwin;

#define fwd(x) std::forward<decltype(x)>(x)

using namespace std::experimental;
using namespace std::chrono;;

auto is_socket_non_blocking(int sockid) -> bool
{

	return fcntl(sockid, F_GETFL) & O_NONBLOCK;
}

auto make_socket_non_blocking(int sockid) -> bool
{
	return fcntl(sockid, F_SETFL, fcntl(sockid, F_GETFL) | O_NONBLOCK) == -1;
}

export
{
	/*
		A "Server" should be able to listen to a specific port
		and handle connections and communications with clients.
		It doesn't really care for the actual messages sent
		back and forth, it focuses more on the networking fancy stuff.
		Therefore, it must be initialized with a "Messenger" object
		which can take care on the gossip part.
		You should also be able to "start()" and "stop()" a "Server".

		TODO
		* use coroutines
		* use concepts
	*/

	template <
		typename accept_connection,
		typename on_disconnect,
		typename incoming_message>
	struct server
	{
		server(server &&) = delete;
		server(server const &) = delete;

		server(
			char const* port,
			accept_connection &acceptConnection,
			on_disconnect &onDisconnect,
			incoming_message &incomingMessage) : acceptConnection{acceptConnection}, onDisconnect{onDisconnect}, incomingMessage{incomingMessage}
		{

			addrinfo *servinfo{nullptr};

			auto hints = addrinfo{
				.ai_family = AF_UNSPEC,
				.ai_socktype = SOCK_STREAM,
				.ai_flags = AI_PASSIVE};

			if (auto r = getaddrinfo(NULL, port, &hints, &servinfo); r != 0)
			{
				fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
				throw;
			}

			auto *i = servinfo;

			// loop through all the results and connect to the first we can
			for (; i != NULL; i = i->ai_next)
			{
				if ((_sockid = socket(i->ai_family, i->ai_socktype,
									 i->ai_protocol)) == -1)
				{
					perror("socket");
					continue;
				}

				if (int yes = 1; setsockopt(_sockid, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
				{
					perror("setsockopt");
					continue;
				}

				// fcntl (_sockid, F_SETFL, O_NONBLOCK | FASYNC);

				if (bind(_sockid, i->ai_addr, i->ai_addrlen) == -1) 
				{
					close(_sockid);
					perror("server: bind");
					continue;
				}

				break;
			}

			freeaddrinfo (servinfo);

			if (i == NULL)
			{
				std::cout << "client failed to connect" << std::endl;
				throw;
			}

			if (listen (_sockid, 20) == -1)
				{
					perror ("listen");
					throw;
				}
		}

		auto start()
		{
			auto polls = std::vector<pollfd>{
				pollfd{
					.fd = _sockid,
					.events = POLLIN}};

			polls.reserve(10);

			struct
			{
				sockaddr_storage addr;
				unsigned int len = sizeof(addr);
				int sockid;
				char ip_address[INET6_ADDRSTRLEN];
			} remote;

			int len = getpagesize();
			char buf[len];
			int numbytes;

			while (true)
			{
				// std::cout << polls.size() << std::endl;

				// wait for something to happen
				if (poll(polls.data(), polls.size(), -1) == -1)
				{
					perror("poll error");
					// throw;
				}

				// new connection
				if (polls[0].revents & POLLIN)
				{
					// accept connection
					if ((remote.sockid = accept(_sockid, (struct sockaddr *)&remote.addr, &remote.len)) == -1)
					{
						perror("accept error");
						throw;
					}

					// keep it ?
					polls.push_back(pollfd{.fd = remote.sockid, .events = POLLIN});
					acceptConnection(remote_client_t{remote.sockid});
					
					

					// break;
				}

				for (auto i = polls.begin() + 1; i != polls.end(); ++i)
				{
					if (i->revents & POLLIN)
					{
						if ((numbytes = recv(i->fd, buf, sizeof(buf), 0)) == -1)
						{
							perror("recv error");
						}

						// disconnection
						else if (numbytes == 0)
						{
							// std::cout << "client hung up" << std::endl;

							if (close(i->fd) == -1)
							{
								perror("close error");
								// throw;
							}
							polls.erase(i);

							// onDisconnect(connection{remote.sockid});
						}

						// message
						else
						{
							buf[numbytes] = '\0';


							incomingMessage(remote_client_t{remote.sockid}, std::string{buf});

							
							
							// std::thread{
							// 	[&] // lambda
							// 	{
							// 		std::string &&response = incomingMessage(connection{remote.sockid}, std::string{buf});

							// 		if (sendall(remote.sockid, response.c_str(), response.size()) == -1)
							// 		{
							// 			perror("sendall error");
							// 			// throw;
							// 		}
							// 	}}
							// 	.detach();
						}
						break;
					}
				}
			}
		}

		auto stop()
		{
			_running = false;
		}

		auto port() const
		{
			struct sockaddr_in inf;
			unsigned int len = sizeof(inf);
			if (getsockname(_sockid, (struct sockaddr *)&inf, &len) == -1)
			{
				perror("getsockname error");
				throw;
			}
			return ntohs(inf.sin_port);
		}

	private:
		accept_connection &acceptConnection;
		on_disconnect &onDisconnect;
		incoming_message &incomingMessage;

		sockaddr_in _addrport{
			.sin_family = AF_UNSPEC,
			.sin_port = htons(INADDR_ANY),
			.sin_addr.s_addr = htonl(INADDR_ANY)};
		int _sockid;
		bool _running;
	};

	template <
		typename accept_connection,
		typename on_disconnect,
		typename incoming_message>
	auto make_server(
		char const* port,
		accept_connection &acceptConnection,
		on_disconnect &onDisconnect,
		incoming_message &incomingMessage)
		->auto
	{

		return server<accept_connection, on_disconnect, incoming_message>{port, acceptConnection, onDisconnect, incomingMessage};
	}
}
