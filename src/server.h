#pragma once

#include <boost/asio.hpp>
#include <memory>
#include "session.h"
#include "database.h"

class Server
{
public:
    Server(boost::asio::io_context& ioContext, short port, std::shared_ptr<Database> db)
        : m_acceptor(ioContext, tcp::endpoint(tcp::v4(), port)), m_db(std::move(db))
    {
        doAccept();
    }

private:
    void doAccept()
    {
        m_acceptor.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    std::make_shared<Session>(std::move(socket), m_db)->start();
                }
                doAccept();
            });
    }

    tcp::acceptor m_acceptor;
    std::shared_ptr<Database> m_db;
};