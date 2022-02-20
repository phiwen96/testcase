import Usr;
import Server;
import Http;
import std;
import Darwin;

using std::cout, std::endl, std::move, std::string, std::vector, std::tuple, std::pair, std::unordered_map;

#include <nlohmann/json.hpp>
using namespace nlohmann;

#define DATA_FILE "data/users.json"

// every access token is related with a user id
auto access_tokens = unordered_map<string, int>{};

// helps generate an random access token that doesn't already exist
auto generate_access_token = []()
{
	auto res = std::string{};
	// generate an random sequence of numbers/digits
	char random_letter = random_int(97, 123);
	for (auto i = 0; i < 10; ++i)
	{
		res += random_int(97, 123);
	}
	return res;
};

// will hold all users
auto users = json{};

// handle ^C
void sigint_handler(int sig)
{
    // save users to file
	auto file = std::ofstream {DATA_FILE};
	file << users;
	file.close();
	cout << "exiting server..." << endl;
	exit (0);
}


// do nothing yet
auto newConnection = [](auto &&remote) {

};
// do nothing yet
auto onDisconnect = [](auto &&remote) {

};
auto resetPassword = [](auto &&remote, auto &&data)
{
	// find access token in http request
	auto i = data.find("access_token");

	// if http request does not contain any access token
	if (i == data.end())
	{
		// send response with error code
		remote << http::response{{1.1, 401, "Unauthorized"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 10}, {"status_message", "Access token needed for operation"}}.dump()};
	}
	// if clients access token match with one we have
	else if (auto j = access_tokens.find(*i); j != access_tokens.end())
	{
		// get the user id
		auto userID = j->second;
		for (auto &user : users)
		{
			if (user["id"] == userID)
			{
				// change its password
				user["password"] = data["password"];
				// respond
				remote << http::response{{1.1, 200, "OK"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", true}, {"status_code", 1}, {"status_message", "Success"}}.dump()};
				return;
			}
		}
		// respond with error code that we didnt find userID
		remote << http::response{{1.1, 404, "Not Found"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 8}, {"status_message", "User not found"}}.dump()};

		// if clients access does not match with one we have
	}
	else
	{
		remote << http::response{{1.1, 401, "Unauthorized"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 11}, {"status_message", "Invalid access token"}}.dump()};
	}
};
auto registerUser = [](auto &&remote, auto &&data)
{
	// check if username already exists
	for (auto const &user : users)
	{
		// already exists2
		if (user["username"] == data["username"])
		{
			remote << http::response{{1.1, 409, "Conflict"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 7}, {"status_message", "Registration declined, username already exists"}}.dump()};
			return;
		}
	}

	// assert that all neccessary fields are given to create a new user
	if (data.contains("username") and data.contains("name") and data.contains("password") and data.contains("email"))
	{
		// ok
	}
	else
	{
		remote << http::response{{1.1, 400, "Bad Request"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 13}, {"status_message", "The request is missing a required parameter"}}.dump()};
		return;
	}

	auto new_user = json{{"id", users.size()}, {"username", data["username"]}, {"name", data["name"]}, {"email", data["email"]}, {"password"}, data["password"]};

	// add it to existing ones
	users.push_back(new_user);

	// generate an access token for it
	auto access_token = std::to_string(random_int(1000, 9999));

	// remember the access_token and it's related user
	access_tokens[access_token] = new_user["id"];

	// respond with access token and userID
	remote << http::response{{1.1, 200, "OK"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", true}, {"status_code", 1}, {"status_message", "success"}, {"access_token", access_token}, {"id", new_user["id"]}}.dump()};
};
auto getUser = [](auto &&remote, auto &&data)
{
	// find access token in http request
	auto i = data.find("access_token");

	// if http request does not contain any access token
	if (i == data.end())
	{
		// send response with error code
		remote << http::response{{1.1, 401, "Unauthorized"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 10}, {"status_message", "Access token needed for operation"}}.dump()};
	}
	// if clients access token match with one we have
	else if (auto j = access_tokens.find(*i); j != access_tokens.end())
	{
		// assert that all neccessary fields are given find the user
		if (data.contains("id"))
		{
			// find user
			for (auto const &user : users)
			{
				if (user["id"] == data["id"])
				{
					// copy it
					auto u = user;
					// remove password
					u.erase("password");
					// fill it with status code
					u["success"] = true;
					u["status_message"] = "Success";
					u["status_code"] = 1;
					// respond
					remote << http::response{{1.1, 200, "OK"}, {{"Content-Type", "application/json; charset-UTF-8"}}, u.dump()
				};
				return;
			}
		}
		// couldn't find user with given userID
		remote << http::response{{1.1, 404, "Not Found"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 14}, {"status_message", "User not found"}}.dump()};

		// userID not given in request
	}
	else
	{
		remote << http::response{{1.1, 400, "Bad Request"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 13}, {"status_message", "The request is missing a required parameter"}}.dump()};
	}
	// if clients access token does not match with one we have
} else
{
	remote << http::response{{1.1, 401, "Unauthorized"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 11}, {"status_message", "Invalid access token"}}.dump()};
}
}
;
auto listUsers = [](auto &&remote, auto &&data)
{
	// find access token in http request
	auto i = data.find("access_token");

	// if http request does not contain any access token
	if (i == data.end())
	{
		// send response with error code
		remote << http::response{{1.1, 401, "Unauthorized"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 10}, {"status_message", "Access token needed for operation"}}.dump()};
	}
	// if clients access token match with one we have
	else if (auto j = access_tokens.find(*i); j != access_tokens.end())
	{
		// make a copy of users
		auto &&safe_users = json{{"success", true}, {"status_code", 1}, {"status_message", "Success"}};
		safe_users["users"] = users;
		// auto safe_users = users;
		// cout << users << endl;
		// remove sensitive info for every user such as passwords
		for (auto &u : safe_users["users"])
		{
			u.erase("password");
		}
		// respond
		remote << http::response{{1.1, 200, "OK"}, {{"Content-Type", "application/json; charset-UTF-8"}}, safe_users.dump()};

		// else if clients access does not match with one we have
	}
	else
	{
		remote << http::response{{1.1, 401, "Unauthorized"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 11}, {"status_message", "Invalid access token"}}.dump()};
	}
};
auto authenticateUser = [](auto &&remote, auto &&data)
{
	// find the user
	for (auto &user : users)
	{
		// on username found
		if (user["username"] == data["username"])
		{
			// on password matching
			if (user["password"] == data["password"])
			{
				// generate an access token
				auto access_token = std::to_string(random_int(1000, 9999));
				// add access token to existing ones
				access_tokens[access_token] = user["id"];

				// respond with it
				remote << http::response{{1.1, 200, "OK"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", true}, {"status_code", 1}, {"status_message", "success"}, {"access_token", access_token}, {"id", user["id"]}}.dump()};

				// wrong password
			}
			else
			{
				// send response with error code
				remote << http::response{{1.1, 401, "Unauthorized"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 5}, {"status_message", "Incorrect password"}}.dump()};
			}
			return;
		}
	}

	// username not found so respond with an error code
	remote << http::response{{1.1, 401, "Unauthorized"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 4}, {"status_message", "Incorrect username"}}.dump()};
};
auto unregisterUser = [](auto &&remote, auto &&data)
{
	// find access token in http request
	auto i = data.find("access_token");

	// if http request does not contain any access token
	if (i == data.end())
	{
		// send response with error code
		remote << http::response{{1.1, 401, "Unauthorized"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 10}, {"status_message", "Access token needed for operation"}}.dump()};

	// else if clients access token match with one we have
	} else if (auto j = access_tokens.find(*i); j != access_tokens.end())
	{
		// copy its ID
		auto userID = j -> second;

		// remove its access token
		access_tokens.erase(j);

		// remove the users data
		for (auto i = users.begin (); i < users.end (); ++i) {
			if ((*i)["id"] == userID) {
				users.erase (i);
				// respond with success code
				remote << http::response{{1.1, 200, "OK"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", true}, {"status_code", 1}, {"status_message", "Success"}}.dump()};
				return;
			}
		}
		// if user not found, respond with error code
		remote << http::response{{1.1, 404, "Not Found"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 12}, {"status_message", "Cannot remove a user that doesn't exist"}}.dump()};
	}
	else
	{
		remote << http::response{{1.1, 401, "Unauthorized"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 11}, {"status_message", "Invalid access token"}}.dump()};
	}
};

auto mappedFunctions = tuple{
	pair{string{"/register"}, registerUser},
	pair{string{"/get"}, getUser},
	pair{string{"/list"}, listUsers},
	pair{string{"/unregister"}, unregisterUser},
	pair{string{"/authenticate"}, authenticateUser},
	pair{string{"/reset"}, resetPassword}};
auto method = []<typename T, typename... U>(T && input, U &&...params)
{
	auto methodHelper = [&]<typename TupleT, std::size_t... Is>(const TupleT &tp, std::index_sequence<Is...>)
	{
		auto found = false;
		auto maybeCall = [&](auto const &keyValue)
		{
			if (not found)
			{
				auto const &[key, value] = keyValue;
				if (input == key)
				{
					found = true;
					value(std::forward<U>(params)...);
				}
			}
		};
		(maybeCall(std::get<Is>(tp)), ...);
		return found;
	};
	return methodHelper(mappedFunctions, std::make_index_sequence<std::tuple_size_v<decltype(mappedFunctions)>>{});
};
auto incomingMessage = [](auto &&remote, std::string msg)
{
	// cout << msg << endl;

	// try to parse incoming message
	auto parsed = http::request::parse(msg);

	// if there is a parsing error
	if (not parsed)
	{
		remote << http::response{{1.1, 400, "Bad Request"}, {{"Content-Type: ", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", 3}, {"status_message", "Could not interpret the request"}}.dump()};
	}

	// call the right url method if found
	else if (method(parsed.value().request_line.url, std::move(remote), json::parse(parsed.value().data)))
	{
	}

	// if url method not found
	else
	{
		remote << http::response{{1.1, 400, "Bad Request"}, {{"Content-Type", "application/json; charset-UTF-8"}}, json{{"success", false}, {"status_code", "3"}, {"status_message", "Could not interpret the request"}}.dump()};
	}
};
auto main(int argc, char **argv) -> int
{
	if (argc != 2)
	{
		cout << "usage >> "
			 << "<localPORT>" << endl;
		return 1;
	}

	cout << "starting server" << endl;

	// setup ^C interrupt function
	[&]{
		struct sigaction sa {};
		sa.sa_handler = sigint_handler;
    	sa.sa_flags = 0;

		sigemptyset(&sa.sa_mask);

		if (sigaction(SIGINT, &sa, NULL) == -1) {
			perror("sigaction");
			exit(1);
		}
	}();
	// load users from file
	auto file_users = std::ifstream{DATA_FILE};
	file_users >> users;
	file_users.close();

	auto s = make_server(
		argv[1],
		newConnection,
		onDisconnect,
		incomingMessage);

	s.start();

	return 0;
}
