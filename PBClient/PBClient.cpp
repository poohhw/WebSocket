#include "addressbook.pb.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using namespace std;
using namespace google;


void Send(websocket::stream<tcp::socket>*);

int main()
{
	try
	{
		string host = "127.0.0.1";
		auto const port = "3100";

		// The io_context is required for all I/O
		net::io_context ioc;

		// These objects perform our I/O
		tcp::resolver resolver{ ioc };
		websocket::stream<tcp::socket> ws{ ioc };

		// Look up the domain name
		auto const results = resolver.resolve(host, port);

		// Make the connection on the IP address we get from a lookup
		auto ep = net::connect(ws.next_layer(), results);

		// Update the host_ string. This will provide the value of the
		// Host HTTP header during the WebSocket handshake.
		// See https://tools.ietf.org/html/rfc7230#section-5.4
		host += ':' + std::to_string(ep.port());

		// Set a decorator to change the User-Agent of the handshake
		ws.set_option(websocket::stream_base::decorator(
			[](websocket::request_type& req)
			{
				req.set(http::field::user_agent,
					std::string(BOOST_BEAST_VERSION_STRING) +
					" websocket-client-coro");
			}));

		// Perform the websocket handshake
		ws.handshake(host, "/");
		ws.binary(true);

		Send(&ws);

		// This buffer will hold the incoming message
		beast::flat_buffer buffer;

		// Read a message into our buffer
		size_t size = ws.read(buffer);

		while (size > 0)
		{			
			std::cout << beast::make_printable(buffer.data()) << std::endl;
			string recvMsg = to_string(buffer.size());

	


			buffer.clear();
			size = ws.read(buffer);
		}

		// Close the WebSocket connection
		ws.close(websocket::close_code::normal);
		

		// If we get here then the connection is closed gracefully

		// The make_printable() function helps print a ConstBufferSequence

	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;


}

void Send(websocket::stream<tcp::socket>* ws)
{
	tutorial::Person person;

	person.set_id(200);
	person.set_name("GCSC RND2");
	int bufSize = 0;
	bufSize = person.ByteSizeLong();
	protobuf::uint8* outputBuf = new protobuf::uint8[bufSize];

	protobuf::io::ArrayOutputStream output_array_stream(outputBuf, bufSize);
	protobuf::io::CodedOutputStream output_coded_stream(&output_array_stream, bufSize);

	person.SerializeToCodedStream(&output_coded_stream);

	//ws.write(net::buffer(string(text)));
	ws->write(net::buffer(outputBuf, bufSize));
}

