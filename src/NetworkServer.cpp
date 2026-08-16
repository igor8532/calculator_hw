#include "NetworkServer.h"

#include "Logger.h"
#include "NetworkConfig.h"
#include "TaskJson.h" // подтягивает nlohmann/json.hpp

#include <istream>

namespace calculator
{

using boost::asio::ip::tcp;

NetworkServer::NetworkServer(Application& application,
                             ShutdownCoordinator& coordinator) :
    application_(application), coordinator_(coordinator), acceptor_(ioContext_)
{
    coordinator_.onStop([this] { requestServerStop(); });
}

void NetworkServer::start()
{
    auto& logger = Logger::getInstance();

    tcp::endpoint endpoint(boost::asio::ip::make_address(SERVER_HOST),
                           SERVER_PORT);

    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    logger.info("NetworkServer::start: Listening on " +
                std::string(SERVER_HOST) + ":" + std::to_string(SERVER_PORT));

    doAccept();
}

void NetworkServer::doAccept()
{
    auto& logger = Logger::getInstance();

    if (coordinator_.stopRequested())
    {
        logger.info("NetworkServer::doAccept: Shutdown requested, not "
                    "accepting further connections");
        return;
    }

    auto socket = std::make_shared<tcp::socket>(ioContext_);

    acceptor_.async_accept(*socket, [this,
                                     socket](boost::system::error_code ec) {
        auto& acceptLogger = Logger::getInstance();

        if (ec == boost::asio::error::operation_aborted ||
            coordinator_.stopRequested())
        {
            acceptLogger.info("NetworkServer::doAccept: Acceptor closed, "
                              "stopping accept loop");
            return; // shutdown: больше accept не планируем
        }

        if (ec)
        {
            acceptLogger.error("NetworkServer::doAccept: accept failed: " +
                               ec.message());
            doAccept(); // ошибка accept — не мешает принять следующего клиента
            return;
        }

        // Следующий accept планируется только из handleSession(), когда
        // текущая сессия полностью завершится — сервер обслуживает клиентов
        // строго по одному.
        handleSession(socket);
    });
}

void NetworkServer::handleSession(std::shared_ptr<tcp::socket> socket)
{
    auto buffer = std::make_shared<boost::asio::streambuf>();

    boost::asio::async_read_until(
        *socket, *buffer, '\n',
        [this, socket, buffer](boost::system::error_code ec, std::size_t) {
            auto& logger = Logger::getInstance();

            if (ec)
            {
                logger.error("NetworkServer::handleSession: read failed: " +
                             ec.message());
                doAccept();
                return;
            }

            std::istream stream(buffer.get());
            std::string requestLine;
            std::getline(stream, requestLine);

            std::string responseLine;
            try
            {
                const Task request =
                    nlohmann::json::parse(requestLine).get<Task>();
                const Task response = application_.processTask(request);
                responseLine = nlohmann::json(response).dump();
            }
            catch (const std::exception& e)
            {
                logger.error("NetworkServer::handleSession: " +
                             std::string(e.what()));
                responseLine = nlohmann::json{{"error", e.what()}}.dump();
            }
            responseLine += '\n';

            auto responseBuffer =
                std::make_shared<std::string>(std::move(responseLine));

            boost::asio::async_write(
                *socket, boost::asio::buffer(*responseBuffer),
                [this, socket, responseBuffer](boost::system::error_code,
                                               std::size_t) {
                    boost::system::error_code ignored;
                    // Ошибка уже обрабатывается через выходной параметр
                    // ignored (неисключающая перегрузка Boost.Asio) —
                    // возвращаемое значение сознательно не используется.
                    // NOLINTNEXTLINE(bugprone-unused-return-value)
                    socket->shutdown(tcp::socket::shutdown_both, ignored);
                    doAccept(); // сессия завершена — можно принимать следующего
                });
        });
}

void NetworkServer::requestServerStop()
{
    // post(), а не прямой вызов — acceptor_ живёт в потоке io_context,
    // а этот callback вызывается из потока SignalHandler.
    boost::asio::post(ioContext_, [this] { acceptor_.close(); });
}

void NetworkServer::runEventLoop()
{
    ioContext_.run();
    Logger::getInstance().info("NetworkServer::runEventLoop: Stopped");
}

} // namespace calculator
