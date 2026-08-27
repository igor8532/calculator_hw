#pragma once

#include "Application.h"
#include "ShutdownCoordinator.h"

#include <boost/asio.hpp>

#include <memory>

namespace calculator
{

// Однопоточный сервер: один io_context, весь цикл accept/read/write
// выполняется в потоке, вызвавшем runEventLoop(). Одновременно
// обслуживается только один клиент — доступ к application_/DataBase
// изнутри поэтому безопасен без мьютексов.
class NetworkServer
{
  public:
    NetworkServer(Application& application, ShutdownCoordinator& coordinator);

    NetworkServer(const NetworkServer&) = delete;
    NetworkServer& operator=(const NetworkServer&) = delete;

    // Синхронно открывает/биндит/переводит acceptor в listen.
    // Возвращается только когда сервер уже реально готов принимать
    // подключения.
    void start();

    // Блокирующий вызов: крутит io_context (accept/read/write).
    // Возвращается сам, когда запрошена остановка и текущая сессия
    // (если была) завершилась.
    void runEventLoop();

  private:
    void doAccept();
    void handleSession(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void requestServerStop();

    Application& application_;
    ShutdownCoordinator& coordinator_;

    boost::asio::io_context ioContext_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

} // namespace calculator
