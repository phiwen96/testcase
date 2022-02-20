import Http;
import Common;
import RemoteServer;
import Email;
import std;

#include <nlohmann/json.hpp>
using namespace nlohmann;

using std::cout, std::cin, std::endl, std::string, std::unordered_map;

// sets us up with an access token from the server
auto authenticate = [](auto& remoteServer, auto& user) {
	auto input = std::string {};
	// get username and password from user
	cout << "username >> ";
	cin >> input;
	user["username"] = input;
	cout << "password >> ";
	cin >> input;
	user["password"] = input;

	// send http request for login
	remoteServer << http::request{{"GET", 1.1, "/authenticate"}, {{"Content-Type", "application/json; charset-UTF-8"}}, user.dump()};

	// loops until we have got an access token for user
	while (true)
	{
		// get response from server
		remoteServer >> input;

		// parse response from server
		if (auto parsed = http::response::parse(input); parsed.has_value())
		{
			// get http data from server in json format
			auto data = json::parse(parsed->data);

			// get status code from server
			auto status_code = data["status_code"];

			// on success, get our access_token and user ID (for further communication with the server)
			if (status_code == 1)
			{
				user["access_token"] = data["access_token"];
				user["id"] = data["id"];
				break;
			}

			// username doesn't exist
			else if (status_code == 4)
			{
				// maybe new user?
				cout << "new user? [y/n] >> ";
				cin >> input;

				while (input != "y" and input != "n")
				{
					cout << "try again, new user? [y/n] >> ";
					cin >> input;
				}

				// if new user, get further info
				if (input == "y")
				{
					// start registration
					cout << "first name >> ";
					cin >> input;
					input += " ";
					cout << "last name >> ";
					auto &&temp = std::string{};
					cin >> temp;
					input += std::move(temp);
					user["name"] = input;
					cout << "email >> ";
					cin >> input;
					user["email"] = input;

					// change the request so that the server may create the new user
					remoteServer << http::request{{"POST", 1.1, "/register"}, {{"Content-Type", "application/json; charset-UTF-8"}}, user.dump()};
				}

				// if old user, just start over
				else if (input == "n")
				{
					cout << "error >> wrong credentials" << endl;
				}
				// cout << data["status_phrase"] << endl;
			}
			// wrong password
			else if (status_code == 5)
			{
				cout << "error >> wrong credentials" << endl;
			}
			// we tried to create user, but username already exists
			else if (status_code == 7)
			{
				cout << "error >> failed to create user" << endl;
			}
			else
			{
				cout << "error >> " << data["status_message"] << endl;
			}
		}

		// parsing failed
		else
		{
			cout << "error >> parsing message from server" << endl;
		}

		// try again
		cout << "username >> ";
		cin >> input;
		user["username"] = input;
		cout << "password >> ";
		cin >> input;
		user["password"] = input;
	}
};

// functions that will map into commands on console

auto onHelp = [] (auto& remoteServer, auto& user)
{
	cout << "Commands\n\n";
	cout << std::setw(20) << std::left << "list" << endl;
	cout << "\tlist >> prints all users" << endl;
	cout << "\tsearch >> start a seach" << endl;
	cout << "\thelp >> if u need help" << endl;
	cout << "\texit >> exits application" << endl;
	cout << "\treset >> resets password" << endl;
	cout << "\tunregister >> unregisters user" << endl;
};

auto onList = [] (auto& remoteServer, auto& user)
{
	auto input = std::string {};

	// get list of users from server
	remoteServer << http::request{{"GET", 1.1, "/list"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"access_token", user["access_token"]}}.dump()};
	remoteServer >> input;
	// parse response from server
	if (auto parsed = http::response::parse(input); parsed.has_value())
	{
		auto status = json::parse(parsed->data);
		auto status_code = status["status_code"];
		// on success
		if (status_code == 1)
		{
			// print list of users
			cout << status["users"].dump(4) << endl;
		}
		else
		{
			cout << "error >> " << status["status_message"] << endl;
		}
	}
	else
	{
		cout << "error >> failed to parse response from server" << endl;
	}
};

auto onExit = [] (auto& remoteServer, auto& user)
{
	exit(0);
};

auto onUnregister = [] (auto& remoteServer, auto& user) {
	remoteServer << http::request {{"GET", 1.1, "/unregister"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"access_token", user["access_token"]}}.dump()};
	auto input = std::string {};
	remoteServer >> input;
	// parse response from server
	if (auto parsed = http::response::parse(input); parsed.has_value())
	{
		auto status = json::parse(parsed->data);
		auto status_code = status["status_code"];
		// on success
		if (status_code == 1)
		{

		} else
		{
			cout << "error >> " << status["status_message"] << endl;
		}

	} else
	{
		cout << "error >> failed to parse response from server" << endl;
	}

	// run authentication from start
	authenticate (remoteServer, user);
};

auto onSearch = [] (auto& remoteServer, auto& user)
{
	auto input = std::string {};
	auto searchParams = std::string {};
	cout << "enter search params >> ";
	cin >> searchParams;
	// get list of users from server
	remoteServer << http::request{{"GET", 1.1, "/list"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"access_token", user["access_token"]}}.dump()};
	remoteServer >> input;
	// parse response from server
	if (auto parsed = http::response::parse(input); parsed.has_value())
	{
		auto status = json::parse(parsed->data);
		auto status_code = status["status_code"];
		// on success
		if (status_code == 1)
		{
			// loop through users in the list we got from server
			for (auto const &u : status["users"])
			{
				// if anything match, print it out
				if (u["username"].get<std::string>().find(searchParams) != std::string::npos)
				{
					cout << u.dump(4) << endl;
				}
				else if (u["name"].get<std::string>().find(input) != std::string::npos)
				{
					cout << u.dump(4) << endl;
				}
				else if (u["email"].get<std::string>().find(input) != std::string::npos)
				{
					cout << u.dump(4) << endl;
				} 
			}
		}
		else
		{
			cout << "error >> " << status["status_message"] << endl;
		}
	}
	else
	{
		cout << "error >> failed to parse response from server" << endl;
	}
};

auto onReset = [] (auto& remoteServer, auto& user)
{
	auto input = std::string {};
	// query for a new password
	cout << "new password >> ";
	cin >> input;

	// if user entered same password as before, warn and try again
	while (input == user["password"])
	{
		cout << "error >> new password must be NEW" << endl;
		cout << "new password >> ";
		cin >> input;
	}

	auto const new_password = input;

	// send request to get userinfo and email from server
	remoteServer << http::request {{"GET", 1.1, "/get"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"access_token", user["access_token"]}, {"id", user["id"]}}.dump()};
	// get response
	remoteServer >> input;
	// parse http request
	if (auto parsed = http::response::parse(input); parsed.has_value())
	{
		// http data in json format
		auto status = json::parse(parsed->data);
		// cout << status << endl;
		// response status code from server
		auto status_code = status["status_code"];
		// users email
		auto email_addr = status["email"];
		// generate a random verification code
		auto verification_code = std::to_string(random_int(1000, 9999));
		// send verification code to user email address
		send_email(email_addr.get<std::string>().c_str(), verification_code.c_str());

		// let user enter verification code
		cout << "email verification code >> ";
		cin >> input;

		// warn and try again when wrong verification code
		while (input != verification_code)
		{
			cout << "error >> wrong verification code";
			cout << "email verification code >> ";
			cin >> input;
		}

		// send request for resetting user password
		remoteServer << http::request {{"PUT", 1.1, "/reset"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"access_token", user["access_token"]}, {"id", user ["id"]}, {"password", new_password}}.dump()};

		// get response from server
		remoteServer >> input;

		// parse response
		if (parsed = http::response::parse(input); parsed.has_value())
		{
			// http data in json format
			status = json::parse(parsed->data);
			// response status code from server
			status_code = status["status_code"];
			// on success
			if (status_code == 1)
			{
				user["password"] = new_password;
				cout << "success >> password changed" << endl;
			}
			else
			{
				cout << "error >> failed to change password" << endl;
			}
		}
		else
		{
			cout << "error >> failed to parse response from server" << endl;
		}
	}
	else
	{
		cout << "error >> failed to parse response from server" << endl;
	}
};

auto onWrongInput = [] (auto& remoteServer, auto& user) {
	cout << "error >> command not found" << endl;
};

auto commands = unordered_map<string, void (*)(remote_server_t&, json&)>{
	{"help", onHelp},
	{"list", onList},
	{"exit", onExit},
	{"unregister", onUnregister},
	{"search", onSearch},
	{"reset", onReset},
	{"wrongInput", onWrongInput}};


auto main(int argc, char **argv) -> int
{
	if (argc != 3)
	{
		cout << "usage >> "
			 << "<remoteIP> <remotePORT>" << endl;
		return 1;
	}

	auto remoteIP = argv[1];
	auto remotePORT = argv[2];

	// setup remote server 
	auto remote = remote_server_t{remoteIP, remotePORT};

	// this will help us remember info about user
	auto user = json {};

	// start authentication procedure so that
	authenticate (remote, user);

	// gonna hold input from user
	auto input = std::string {};

	// start program loop
	while (true)
	{
		cout << "online >> ";
		// enter command
		cin >> input;
		// on command found
		if (auto i = commands.find(input); i != commands.end())
		{
			// call command
			(*i->second)(remote, user);
		}
		else
		{
			// on command not found, call wrong input function
			(*commands["wrongInput"])(remote, user);
		}
	}

	// wait for a command from user
	// while (true)
	// {
	// 	cout << "online >> ";
	// 	cin >> input;
	// 	if (input == "exit")
	// 	{
	// 		break;
	// 	}
	// 	else if (input == "list")
	// 	{
	// 		// get list of users from server
	// 		remote << http::request{{"GET", 1.1, "/list"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"access_token", access_token}}.dump()};
	// 		remote >> response;
	// 		// parse response from server
	// 		if (auto parsed = http::response::parse(response); parsed.has_value())
	// 		{
	// 			auto status = json::parse(parsed->data);
	// 			auto status_code = status["status_code"];
	// 			// on success
	// 			if (status_code == 1)
	// 			{
	// 				// print list of users
	// 				cout << status["users"].dump(4);
	// 			}
	// 			else
	// 			{
	// 				cout << "error >> " << status["status_message"] << endl;
	// 			}
	// 		}
	// 		else
	// 		{
	// 			cout << "error >> failed to parse response from server" << endl;
	// 			continue;
	// 		}
	// 	}
	// 	else if (input == "reset")
	// 	{
	// 		// query for a new password
	// 		cout << "new password >> ";
	// 		cin >> input;

	// 		// if user entered same password as before, warn and try again
	// 		while (input == user["password"])
	// 		{
	// 			cout << "error >> new password must be NEW" << endl;
	// 			cout << "new password >> ";
	// 			cin >> input;
	// 		}

	// 		auto const new_password = input;

	// 		// prepare request to get userinfo and email from server
	// 		request = {{"GET", 1.1, "/get"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"access_token", access_token}, {"id", user_id}}.dump()};
	// 		// send request
	// 		remote << request;
	// 		// get response
	// 		remote >> response;
	// 		// parse http request
	// 		if (auto parsed = http::response::parse(response); parsed.has_value())
	// 		{
	// 			// http data in json format
	// 			auto status = json::parse(parsed->data);
	// 			// cout << status << endl;
	// 			// response status code from server
	// 			auto status_code = status["status_code"];
	// 			// users email
	// 			auto email_addr = status["email"];
	// 			// generate a random verification code
	// 			auto verification_code = std::to_string(random_int());
	// 			// send verification code to user email address
	// 			send_email(email_addr.get<std::string>().c_str(), verification_code.c_str());

	// 			// let user enter verification code
	// 			cout << "email verification code >> ";
	// 			cin >> input;

	// 			// warn and try again when wrong verification code
	// 			while (input != verification_code)
	// 			{
	// 				cout << "error >> wrong verification code";
	// 				cout << "email verification code >> ";
	// 				cin >> input;
	// 			}

	// 			// prepare request for resetting user password
	// 			request = {{"PUT", 1.1, "/reset"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"access_token", access_token}, {"id", user_id}, {"password", new_password}}.dump()};

	// 			// send request
	// 			remote << request;

	// 			// get response from server
	// 			remote >> response;

	// 			// parse response
	// 			if (parsed = http::response::parse(response); parsed.has_value())
	// 			{
	// 				// http data in json format
	// 				status = json::parse(parsed->data);
	// 				// response status code from server
	// 				status_code = status["status_code"];
	// 				// on success
	// 				if (status_code == 1)
	// 				{
	// 					user["password"] = new_password;
	// 					cout << "success >> password changed" << endl;
	// 				}
	// 				else
	// 				{
	// 					cout << "error >> failed to change password" << endl;
	// 				}
	// 			}
	// 			else
	// 			{
	// 				cout << "error >> failed to parse response from server" << endl;
	// 				continue;
	// 			}
	// 		}
	// 		else
	// 		{
	// 			cout << "error >> failed to parse response from server" << endl;
	// 		}
	// 	}
	// 	else if (input == "help")
	// 	{
	// 		cout << "Commands\n\n";
	// 		cout << std::setw(20) << std::left << "list" << endl;
	// 		cout << "\tlist >> prints all users" << endl;
	// 		cout << "\tsearch >> start a seach" << endl;
	// 		cout << "\thelp >> if u need help" << endl;
	// 		cout << "\texit >> exits application" << endl;
	// 		cout << "\treset >> resets password" << endl;
	// 		cout << "\tunregister >> unregisters user" << endl;
	// 	}
	// 	else if (input == "search")
	// 	{
	// 		cout << "enter search params >> ";
	// 		cin >> input;
	// 		// get list of users from server
	// 		remote << http::request{{"GET", 1.1, "/list"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"access_token", access_token}}.dump()};
	// 		remote >> response;
	// 		// parse response from server
	// 		if (auto parsed = http::response::parse(response); parsed.has_value())
	// 		{
	// 			auto status = json::parse(parsed->data);
	// 			auto status_code = status["status_code"];
	// 			// on success
	// 			if (status_code == 1)
	// 			{
	// 				// loop through users in the list we got from server
	// 				for (auto const &u : status["users"])
	// 				{
	// 					// if anything match, print it out
	// 					if (u["username"].get<std::string>().find(input) != std::string::npos)
	// 					{
	// 						cout << u.dump(4) << endl;
	// 					}
	// 					else if (u["name"].get<std::string>().find(input) != std::string::npos)
	// 					{
	// 						cout << u.dump(4) << endl;
	// 					}
	// 					else if (u["email"].get<std::string>().find(input) != std::string::npos)
	// 					{
	// 						cout << u.dump(4) << endl;
	// 					}
	// 				}
	// 			}
	// 			else
	// 			{
	// 				cout << "error >> " << status["status_message"] << endl;
	// 			}
	// 		}
	// 		else
	// 		{
	// 			cout << "error >> failed to parse response from server" << endl;
	// 			continue;
	// 		}
	// 	}
	// }

	// cout << response << endl;

	// auto response = std::string {};

	// remote >> response;

	// auto parsed = http_repsonse::parse (response);

	// if (not parsed)
	// {
	// 	cout << "could not interpret response from server" << endl;
	// }

	return 0;
}

/*

auto msg = std::string
		{
			"POST /login HTTP/1.1\r\n"
			"Content-type: application/json; charset-UTF-8\r\n"
			"Content-Length: "
		}
			+ std::to_string (user.dump().size())
			+ "\n\n"
			+ user.dump();
*/

/*
std::string inp;

	// cout << send ("127.0.0.1", "8080", "hello world") << endl;
	auto user = nlohmann::json{};

	auto register_user = [&]() -> bool // true if success (user is then logged in)
	{
		cout << "first name >> ";
		cin >> inp;
		auto name = inp;
		name += ' ';
		cout << "last name >> ";
		cin >> inp;
		name += inp;
		user["name"] = name;
		cout << "email >> ";
		cin >> inp;
		user["email"] = inp;

		auto request = http_request{
			.request_line =
				{
					.request_type = "POST",
					.version = 1.0,
					.url = "/register"},

			.headers =
				{
					{"Content-Type", "application/json; charset-UTF-8"}},

			.data = user.dump()};

		auto from_server = send(SERVER_ADDRESS, PORT, to_string(request));

		auto response = http_response::parse(from_server);

		if (not response) // Cannot interpret response from client
		{
			std::cout << "error, cant interpret response from server, starting over..." << std::endl;
			return false;
		}

		auto status_json = nlohmann::json::parse(response->data);

		auto status_code = status_json["status code"].get<int>();

		switch (status_code)
		{
			case 1: // success
			{
				return true;
			}

			case 7: // username already taken
			{

			}
		}

		if (status_code == 1)
		{
			return true;
		}
		else
		{
			std::cout << "error, cant interpret response from server, starting over..." << std::endl;
			return false;
		}
	};

	auto authenticate = [&]() -> bool // true on success
	{
		std::cout << "username >> ";
		std::cin >> inp;
		user["username"] = inp;

		std::cout << "password >> ";
		std::cin >> inp;
		user["password"] = inp;

		auto request = http_request{
			.request_line =
				{
					.request_type = "POST",
					.version = 1.0,
					.url = "/login"},

			.headers =
				{
					{"Content-Type", "application/json; charset-UTF-8"}},

			.data = user.dump()};

		auto from_server = send(SERVER_ADDRESS, PORT, to_string(request));
		auto response = http_response::parse(from_server);

		if (not response) // Cannot interpret response from client
		{
			std::cout << "error, cant interpret response from server, starting over..." << std::endl;
			return false;
		}

		auto status_json = nlohmann::json::parse(response->data);

		auto status_code = status_json["status code"].get<int>();

		switch (status_code)
		{
		case 1: // success
		{
			break;
		}

		case 4: // incorrect username
		{
			// check if new user
			while (true)
			{
				std::cout << "new user? [y/n] >> ";
				std::cin >> inp;

				if (inp == "y") // register user
				{
					return register_user();
				}
				else if (inp == "n")
				{
					std::cout << "try again, ";
					return false;
				}
				else
				{
					std::cout << "try again, ";
				}
			}

			break;
		}

		case 5: // incorrect password
		{
			std::cout << "incorrect password, ";
			return false;
		}
		}
		return true;
	};

AUTH:

	while (not authenticate())
	{

	}

	auto close = false;

	std::cout << "online >> ";

	while (not close)
	{
		std::cin >> inp;

		if (inp == "logout")
		{
			// std::cout << "logging u off" << std::endl;

			auto request = http_request{
				.request_line =
					{
						.request_type = "POST",
						.version = 1.0,
						.url = "/logout"},

				.headers =
					{
						{"Content-Type", "application/json; charset-UTF-8"}},

				.data = user.dump()};

			auto from_server = send(SERVER_ADDRESS, PORT, to_string(request));
			auto response = http_response::parse(from_server);

			if (not response) // Cannot interpret response from client
			{
				std::cout << "error, cant interpret response from server, exiting..." << std::endl;
				exit(1);
			}

			auto status_json = nlohmann::json::parse(response->data);

			auto status_code = status_json["status code"].get<int>();

			switch (status_code)
			{
			case 1: // success
			{
				goto AUTH;
				// close = true;
			}

			default: // we assume everything else is an error
			{
				std::cout << "error, skipping command" << std::endl;
				break;
			}
			}

		} else if (inp == "delete")
		{
			auto request = http_request
			{
				.request_line =
				{
					.request_type = "POST",
					.version = 1.0,
					.url = "/delete"
				},

				.headers =
				{
					{"Content-Type", "application/json; charset-UTF-8"}
				},

				.data = user.dump()
			};

			auto from_server = send(SERVER_ADDRESS, PORT, to_string(request));
			auto response = http_response::parse(from_server);

			if (not response) // Cannot interpret response from client
			{
				std::cout << "error, cant interpret response from server, exiting..." << std::endl;
				// exit(1);
				continue;
			}

			auto status_json = nlohmann::json::parse(response->data);

			auto status_code = status_json["status code"].get<int>();

			switch (status_code)
			{
			case 1: // success, user deleted, start over authentication
			{
				goto AUTH;
			}

			default: // we assume everything else is an error
			{
				std::cout << "error, skipping command" << std::endl;
				break;
			}
			}
		} else if (inp == "list")
		{
			auto request = http_request
			{
				.request_line =
				{
					.request_type = "GET",
					.version = 1.0,
					.url = "/list"
				},

				.headers =
				{
					{"Content-Type", "application/json; charset-UTF-8"}
				},

				.data = user.dump()
			};

			auto from_server = send(SERVER_ADDRESS, PORT, to_string(request));
			auto response = http_response::parse(from_server);

			if (not response) // Cannot interpret response from client
			{
				std::cout << "error, cant interpret response from server, exiting..." << std::endl;
				// exit(1);
				continue;
			}

			auto data_json = nlohmann::json::parse(response->data);

			auto status_code = data_json["status code"].get<int>();

			switch (status_code)
			{
			case 1: // success
				std::cout << data_json ["users"].dump (4) << std::endl << "online >> ";
				break;

			default:
				std::cout << data_json ["status message"] << std::endl << "online >> ";
				break;
			}

		} else if (auto i = inp.find("search "); i == 0) // starts with 'search '
		{
			std::cout << "searching!" << std::endl;
		} else
		{
			std::cout << "not a valid command, type \"help\" for a list of available commands." << std::endl;
		}
	}
*/