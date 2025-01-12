#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <cstring>
#include <iostream>
#include <string>
#include <set>
#include <mutex>

// Define the server type using Asio without TLS
typedef websocketpp::server<websocketpp::config::asio> server;
typedef websocketpp::connection_hdl connection_hdl;

std::set<connection_hdl, std::owner_less<connection_hdl>> clients;
std::mutex clients_mutex;

// Define a custom handler for messages
void on_message(server* s, websocketpp::connection_hdl hdl, server::message_ptr msg) {
    std::string payload = msg->get_payload();
    std::cout << "Received message: " << payload  << std::endl;

    std::lock_guard<std::mutex> lock(clients_mutex);
    for (auto it : clients) {
        try {
            s->send(it, payload, msg->get_opcode());
        }
        catch (websocketpp::exception &e) {
            std::cout << "Error cannot emmit the payload." << e.what() << std::endl;
        }
    }
}

void on_open(server* s, connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    clients.insert(hdl);
    std::string alert_message = " New Client has connected.";
    std::cerr << alert_message << std::endl;


    for (auto it : clients) {
        try {
           s->send(it, alert_message, websocketpp::frame::opcode::text);
        }
        catch (websocketpp::exception& e) {
            std::cerr << "Error cannot sending to the server" << e.what() << std::endl;
        }
    }
}

void on_close(connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    clients.erase(hdl);
    std::cout << "Client has disconnected total connected : " << clients.size() << std::endl;
}

int main() {
    // Create the server
    server ws_server;
    std::string ipv4_addr = "192.168.0.139";
    int port_number = 9002;

    try {
        // Initialize Asio
        ws_server.init_asio();

        ws_server.set_open_handler(std::bind(&on_open, &ws_server, std::placeholders::_1));
        ws_server.set_close_handler(&on_close);

        // Set the message handler
        ws_server.set_message_handler(std::bind(&on_message, &ws_server, std::placeholders::_1, std::placeholders::_2));

        // Listen on port 9002
        ws_server.listen(port_number);

        // Start the server accept loop
        ws_server.start_accept();

        // Start the Asio io_service run loop
        std::cout << "start listening on ws://localhost:9002" << std::endl;
        ws_server.run();
    }
    catch (const websocketpp::exception& e) {
        std::cerr << "WebSocket++ exception: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
    }

    return 0;
}
