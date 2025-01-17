#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_TYPE_TRAITS_
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/common/asio.hpp>
#include <iostream>
#include <string>
#include <functional>
#include <nlohman/json.hpp>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> Ctx;

Ctx on_tls_init() {
    Ctx ctx = websocketpp::lib::make_shared<websocketpp::lib::asio::ssl::context>(websocketpp::lib::asio::ssl::context::tlsv12);

    try {
        ctx->set_options(websocketpp::lib::asio::ssl::context::default_workarounds |
            websocketpp::lib::asio::ssl::context::no_sslv2 |
            websocketpp::lib::asio::ssl::context::no_sslv3 |
            websocketpp::lib::asio::ssl::context::single_dh_use);
    }
    catch (std::exception& e) {
        std::cout << "Error setting TLS context options: " << e.what() << std::endl;
    }
    return ctx;
}

class BinanceWebSocket {
public:
    BinanceWebSocket() {
        // Initialize ASIO
        wsClient.init_asio();

        // Set up TLS
        wsClient.set_tls_init_handler([](websocketpp::connection_hdl) {
            return on_tls_init();
            });

        // Bind the message handler
        wsClient.set_message_handler([this](websocketpp::connection_hdl hdl, client::message_ptr msg) {
            onMessage(hdl, msg);
            });

        // Bind the open handler
        wsClient.set_open_handler([this](websocketpp::connection_hdl hdl) {
            onOpen(hdl);
            });

        // Bind the close handler
        wsClient.set_close_handler([this](websocketpp::connection_hdl hdl) {
            onClose(hdl);
            });

        // Bind the fail handler
        wsClient.set_fail_handler([this](websocketpp::connection_hdl hdl) {
            onFail(hdl);
            });
    }

    void connect(const std::string& uri) {
        websocketpp::lib::error_code ec;

        // Create a connection
        auto conn = wsClient.get_connection(uri, ec);
        if (ec) {
            std::cerr << "Connection error: " << ec.message() << std::endl;
            return;
        }

        // Save the connection handle
        connectionHandle = conn->get_handle();

        // Connect to the server
        wsClient.connect(conn);

        // Run the ASIO event loop
        wsClient.run();
    }

private:
    void onOpen(websocketpp::connection_hdl hdl) {
        std::cout << "Connected to Binance WebSocket!" << std::endl;
    }

    void onMessage(websocketpp::connection_hdl, client::message_ptr msg) {
        system("cls");
      //  std::cout << "Received message: " << msg->get_payload() << std::endl;
        nlohmann::json toJson = nlohmann::json::parse(msg->get_payload());

        std::string btc_price = toJson["c"];

        std::cout << "Bitcoin price : " << btc_price << std::endl;
    }

    void onClose(websocketpp::connection_hdl hdl) {
        std::cout << "Connection closed!" << std::endl;
    }

    void onFail(websocketpp::connection_hdl hdl) {
        std::cerr << "Connection failed!" << std::endl;
    }

    client wsClient;
    websocketpp::connection_hdl connectionHandle;
};

int main() {
    // Replace with the desired trading pair, e.g., btcusdt for Bitcoin/USDT
    std::string cryptoSymbol = "btc";
    std::string cryptoCompare = "usdt";

    std::string query = cryptoSymbol + cryptoCompare;
    std::string binanceUri = "wss://stream.binance.com:9443/ws/" + query + "@miniTicker";

    BinanceWebSocket binanceWs;
    binanceWs.connect(binanceUri);

    return 0;
}