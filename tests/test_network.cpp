#include <gtest/gtest.h>

#include "Application.h"
#include "NetworkConfig.h"
#include "NetworkServer.h"
#include "ShutdownCoordinator.h"
#include "TaskJson.h"

#include <boost/asio.hpp>

#include <chrono>
#include <memory>
#include <string>
#include <thread>

namespace
{

using boost::asio::ip::tcp;

// Busy-loop с ограничением по времени вместо sleep_for: неблокирующее
// чтение сокета, между попытками — yield(), не фиксированная пауза.
std::string readLineWithTimeout(tcp::socket& socket,
                                std::chrono::milliseconds timeout)
{
    socket.non_blocking(true);

    std::string data;
    char chunk[256];
    const auto deadline = std::chrono::steady_clock::now() + timeout;

    while (data.find('\n') == std::string::npos)
    {
        boost::system::error_code ec;
        const std::size_t n = socket.read_some(boost::asio::buffer(chunk), ec);

        if (!ec)
        {
            data.append(chunk, n);
        }
        else if (ec != boost::asio::error::would_block)
        {
            ADD_FAILURE() << "Socket error: " << ec.message();
            return data;
        }

        if (data.find('\n') == std::string::npos &&
            std::chrono::steady_clock::now() >= deadline)
        {
            ADD_FAILURE() << "Timed out waiting for server response";
            return data;
        }

        std::this_thread::yield();
    }

    return data;
}

class NetworkServerTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        server_ = std::make_unique<calculator::NetworkServer>(application_,
                                                               coordinator_);
        server_->start(); // возвращается только когда сервер уже слушает
        serverThread_ = std::thread([this] { server_->runEventLoop(); });
    }

    void TearDown() override
    {
        coordinator_.requestStop();
        if (serverThread_.joinable())
        {
            serverThread_.join();
        }
    }

    calculator::Application application_;
    calculator::ShutdownCoordinator coordinator_;
    std::unique_ptr<calculator::NetworkServer> server_;
    std::thread serverThread_;
};

TEST_F(NetworkServerTest, ComputesAdditionOverNetwork)
{
    boost::asio::io_context clientContext;
    tcp::socket socket(clientContext);
    socket.connect(
        tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"),
                     calculator::SERVER_PORT));

    const nlohmann::json request{
        {"firstValue", 4}, {"secondValue", 5}, {"operation", "+"}};
    const std::string requestLine = request.dump() + "\n";

    boost::asio::write(socket, boost::asio::buffer(requestLine));

    const std::string responseLine =
        readLineWithTimeout(socket, std::chrono::seconds(5));
    const auto response = nlohmann::json::parse(responseLine);

    EXPECT_EQ(response.at("result").get<int>(), 9);
    EXPECT_EQ(response.at("status").get<int>(), 0);
}

TEST_F(NetworkServerTest, ComputesFactorialOverNetwork)
{
    boost::asio::io_context clientContext;
    tcp::socket socket(clientContext);
    socket.connect(
        tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"),
                     calculator::SERVER_PORT));

    const nlohmann::json request{{"firstValue", 5}, {"operation", "!"}};
    const std::string requestLine = request.dump() + "\n";

    boost::asio::write(socket, boost::asio::buffer(requestLine));

    const std::string responseLine =
        readLineWithTimeout(socket, std::chrono::seconds(5));
    const auto response = nlohmann::json::parse(responseLine);

    EXPECT_EQ(response.at("result").get<int>(), 120);
    EXPECT_EQ(response.at("status").get<int>(), 0);
}

} // namespace
