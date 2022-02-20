export module Http;

import std;
using std::cout, std::endl;

export namespace http {
	// using std::cout, std::endl;

	// auto const status = std::unordered_map<int, std::string>{
	// 	{200, "OK"},
	// 	{201, "Created"},
	// 	{301, "Moved Permanently"},
	//  {401, "Unauthorized"},
	// 	{400, "Bad Request"},
	// 	{403, "Forbidden"},
	// 	{404, "Not Found"},
	//  {409, "Conflict"}
	// 	{505, "HTTP Version Not Supported"}};

	// using status_t = typename decltype(status)::value_type;

	// enum class request_type {
	// 	GET, POST, PUT, DELETE
	// };

	/*
		request type

		url

		HTTP version
	*/
	struct request_line {
		std::string request_type;
		float version;
		std::string url;

		friend auto operator<<(std::ostream &os, request_line const &me) -> std::ostream & {
			os << me.request_type << ' ' << me.url << ' ' << "HTTP/" << std::fixed << std::setprecision(1) << me.version << "\r\n";
			return os;
		}

		static auto parse(std::string const &line_in) -> std::optional<request_line> {
			auto line_out = request_line{};

			if (auto i = line_in.find(' '); i != std::string::npos) {
				line_out.request_type = std::string {line_in.begin (), line_in.begin() + i};
				// cout << "request type:" << line_out.request_type << endl;
				if (auto j = line_in.find ("HTTP/"); j != std::string::npos) {
					line_out.url = std::string {line_in.begin() + i + 1, line_in.begin() + j - 1};
					// cout << "url:" << line_out.url << endl;
					auto version = std::string {line_in.begin() + j + 5, line_in.end()};
					// cout << "version:" << version << endl;

					line_out.version = std::stof (version);
				} else {
					std::cout << "Failed to parse url" << std::endl;
					return std::nullopt;
				}
			} else {
				std::cout << "Failed to parse request type" << std::endl;
				return std::nullopt;
			}
			return line_out;
		}
	};

	
	struct header {
		std::string name;
		std::string value;

		friend auto operator<<(std::ostream &os, header const &me) -> std::ostream & {
			os << me.name << ": " << me.value << "\r\n";
			return os;
		}

		static auto parse(std::string const &header_in) -> std::optional<header> {
			// std::cout << "header in:" << header_in << std::endl;
			auto header_out = header{};

			if (auto i = header_in.find(": "); i != std::string::npos) {
				header_out.name = std::string{header_in.begin(), header_in.begin() + i};
				// std::cout << "name:" << header_out.name << std::endl;

				header_out.value = std::string{header_in.begin() + i + 2, header_in.end()};
				// std::cout << "value:" << header_out.value << std::endl;

				if (header_out.value.size() == 0) {
					std::cout << "error parsing header-value" << std::endl;
					return std::nullopt;
				}
			}
			else {
				std::cout << "error parsing header-name" << std::endl;
				return std::nullopt;
			}

			return header_out;
		}
	};

	/*
		request line

		headers

		empty line 

		maybe data
	*/
	struct request {
		request_line request_line;
		std::vector<header> headers;
		std::string data;

		friend auto operator<<(std::ostream &os, request const &me) -> std::ostream & {
			os << me.request_line;

			for (auto const &header : me.headers) {
				os << header;
			}

			if (me.data.size() > 0) {
				os << "\r\n" << me.data;
			}

			return os;
		}

		static auto parse (std::string s) -> std::optional<request> {			
			auto result = request{};

			// cut json data from string
			if (auto i = s.find ("\r\n\r\n"); i != std::string::npos) {
				result.data = std::string (s.begin () + i + 4, s.end ());
				s.erase (i);

			} else {
				return std::nullopt;
			}

			auto ss = std::stringstream{s};
			auto line = std::string{};

			// get request-line: HTTP/1.1 200 OK\r\n
			std::getline(ss, line); 

			// if parsing request-line successfull
			if (auto rline = http::request_line::parse(line); rline.has_value()) {
				result.request_line = rline.value(); 
			} else { // if parsing request-line successfull
				return std::nullopt;
			}			

			// extract each header

			// "Server: ph"
			// "Content-Type: text/html; charset=UTF-8\r\n"
			// "Content-Length: 200\r\n\r\n"
			while (std::getline(ss, line)) {

				// if header parsed successfull
				if (auto header = header::parse(line); header.has_value()) {
					result.headers.push_back(header.value());
				} else { // if header parsed unsuccessfull
					return std::nullopt;
				}
			}
			return result;
		}

		operator std::string () const {
			auto result = std::string{};
			auto stream = std::stringstream{result};
			stream << *this;
			return stream.str();
		}
	};

	/*
		HTTP version

		status code

		status phrase
	*/
	struct status_line {
		float version;
		int status_code;
		std::string status_phrase;

		static auto parse(std::string const &line_in) -> std::optional<status_line> {
			// cout << line_in << endl;
			auto line_out = status_line{};
			auto version = std::string {};
			auto status_code = std::string {};

			auto stream = std::stringstream {line_in};

			stream >> version;
			stream >> status_code;
			std::getline (stream, line_out.status_phrase);

			if (auto i = version.find ("HTTP/"); i != std::string::npos) {
				line_out.version = std::stof (std::string {version.begin() + i + 5, version.end ()});
			} else {
				std::cout << "Failed to find http version" << std::endl;
				return std::nullopt;
			}

			line_out.status_code = std::stof (status_code);

			return line_out;
		}

		friend auto operator<<(std::ostream &os, status_line const &me) -> std::ostream & {
			os << "HTTP/" << std::fixed << std::setprecision(1) << me.version << ' ' << me.status_code << ' ' << me.status_phrase << "\r\n";
			return os;
		}
	};


	/*
		status line

		headers

		empty line 

		maybe data
	*/
	struct response {
		status_line status_line;
		std::vector<http::header> headers;
		std::string data;

		static auto parse(std::string in) -> std::optional<response> {
			

			auto result = response {};


			// cut json data from string 
			if (auto i = in.find ("\r\n\r\n"); i != std::string::npos) { 
				// std::cout << in << std::endl;
				result.data = std::string (in.begin () + i + 4, in.end ());
				// std::cout << "bajs" << std::endl;
				in.erase (i);
			}

			
			
			// cout << in << endl;
			auto line = std::string {};
			auto stream = std::stringstream {in};

			getline (stream, line);
			// cout << line << endl;
			
			// parse status line success
			if (auto status_line = status_line::parse (line); status_line.has_value ()){
				result.status_line = status_line.value ();

				while (getline (stream, line)){
					if (auto h = http::header::parse (line);h.has_value ()){
						result.headers.push_back (h.value ());
					} else {
						return std::nullopt;
					}
				}
			} else { // parse status line fail
				return std::nullopt;
			}
			return result;
		}

		friend auto operator<<(std::ostream &os, response const &me) -> std::ostream & {
			os << me.status_line;

			for (auto const &header : me.headers) {
				os << header;
			}

			if (me.data.size() > 0) {
				os << "\r\n" << me.data;
			}
			return os;
		}

		operator std::string () const {
			auto result = std::string{};
			auto stream = std::stringstream{result};
			stream << *this;
			return stream.str();
		}
	};

	inline auto to_string(auto const &request)->std::string {
		auto result = std::string{};
		auto stream = std::stringstream{result};
		stream << request;
		return stream.str();
	}
}