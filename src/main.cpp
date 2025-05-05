#include "pch.h"
#include "web-socket-client.h"

int main(int argc, char** argv) {
	// Default values
	std::string host = "localhost";
	std::string port = "5000";
	std::string nickname;
	std::string code;

	// Parse command line arguments into a map
	std::unordered_map<std::string, std::string> args;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.starts_with("--")) {
            // Handle flags like --help (no value expected)
            if (arg == "--help") {
                args["help"] = "";
            } else if (i + 1 < argc && argv[i + 1][0] != '-') {
                // Store key-value pairs for arguments that expect a value
                args[arg.substr(2)] = argv[++i];
            }
        }
    }

	// Update default values based on provided arguments
	if (args.count("host")) host = args["host"];
	if (args.count("port")) port = args["port"];
	if (args.count("nickname")) nickname = args["nickname"];
	if (args.count("code")) code = args["code"];

    if (args.count("help") || nickname.empty()){
        std::cout << "--nickname Nickname of the bot that will be displayed in the game.\nThis must be a unique identifier for the bot in the game environment.\nNicknames that are already in use or not unique will cause conflicts.\n--host The IP address or domain name of the server to connect to.\nThe bot will attempt to establish a connection to the specified host.\nIf not provided, it defaults to 'localhost'.\n--port The port on which the server is listening.\nThis specifies the port number that the server is using for communication.\nIf not provided, it defaults to port 5000.\n--code Optional access code required to join the server.\nIf the server enforces an access code for connections, it must be supplied here.\nIf no code is required, this can be left empty (default is an empty string)."
                << std::endl << std::flush;
        return EXIT_SUCCESS;
    };

	// Print the values
	std::cout << "Host: " << host << std::endl << std::flush;
	std::cout << "Port: " << port << std::endl << std::flush;
	std::cout << "Nickname: " << nickname << std::endl << std::flush;
	std::cout << "Code: " << code << std::endl << std::flush;

	// Placeholder for actual websocket client logic
	std::cout << "Starting client..." << std::endl << std::flush;

	// Create the WebSocket client
	WebSocketClient client(host, port, nickname, code);

	std::signal(SIGINT, WebSocketClient::SignalHandler);

	#ifdef _WIN64
		std::signal(SIGBREAK, WebSocketClient::SignalHandler);
	#elif defined(__linux__)
		std::signal(SIGQUIT, WebSocketClient::SignalHandler);
	#else
		#error "Unsupported platform"
	#endif

	// Connect to the WebSocket server asynchronously
	auto connectFuture = client.Connect();

	// Wait for the connection to be established
	if (!connectFuture.get()) {
		std::cerr << "Failed to Connect to the WebSocket server." << std::endl << std::flush;
		WebSocketClient::Stop();
		return EXIT_FAILURE;
	}

	std::cout << "Connected successfully. Running the client..." << std::endl << std::flush;

	return EXIT_SUCCESS;
}
