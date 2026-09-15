#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include "database.h"

using boost::asio::ip::tcp;

class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(tcp::socket socket, std::shared_ptr<Database> db)
        : m_socket(std::move(socket)), m_db(std::move(db))
    {
    }

    ~Session() {
    }

    void start()
    {
        doRead();
    }

private:
    void doRead()
    {
        auto self(shared_from_this());

        boost::asio::async_read_until(m_socket, m_buffer, ".",
            [this, self](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    std::istream is(&m_buffer);
                    std::string raw_command;
                    std::getline(is, raw_command, '.');

                    if (m_buffer.size() > 0) {
                        std::string dummy;
                        std::getline(is, dummy);
                    }

                    std::string trimmed_command = trim(raw_command);
                    processCommand(trimmed_command);
                }
                else if (ec == boost::asio::error::eof) {
                }
                else if (ec != boost::asio::error::operation_aborted)
                {
                }
                else
                {
                }
            });
    }

    void processCommand(const std::string& command)
    {
        std::stringstream ss(command);
        std::string segment;
        std::vector<std::string> tokens;

        while (std::getline(ss, segment, ' '))
        {
            if (!segment.empty())
            {
                tokens.push_back(segment);
            }
        }

        std::string response_data;
        std::string final_response;

        if (tokens.empty())
        {
            final_response = "ERR empty command\n";
        }
        else if (tokens[0] == "INSERT")
        {
            if (tokens.size() == 4)
            {
                try
                {
                    int id = std::stoi(tokens[2]);
                    if (m_db->insert(tokens[1], id, tokens[3]))
                    {
                        final_response = "OK\n";
                    }
                    else
                    {
                        final_response = "ERR duplicate " + tokens[2] + "\n";
                    }
                }
                catch (...) { final_response = "ERR invalid arguments\n"; }
            }
            else { final_response = "ERR invalid command format\n"; }
        }
        else if (tokens[0] == "TRUNCATE")
        {
            if (tokens.size() == 2)
            {
                m_db->truncate(tokens[1]);
                final_response = "OK\n";
            }
            else { final_response = "ERR invalid command format\n"; }
        }
        else if (tokens[0] == "INTERSECTION")
        {
            if (tokens.size() == 1)
            {
                auto result = m_db->intersect();
                for (const auto& line : result)
                {
                    response_data += line + "\n";
                }
                final_response = response_data + "OK\n";
            }
            else { final_response = "ERR invalid command format\n"; }
        }
        else if (tokens[0] == "SYMMETRIC_DIFFERENCE")
        {
            if (tokens.size() == 1)
            {
                auto result = m_db->symmetric_difference();
                for (const auto& line : result)
                {
                    response_data += line + "\n";
                }
                final_response = response_data + "OK\n";
            }
            else { final_response = "ERR invalid command format\n"; }
        }
        else
        {
            final_response = "ERR unknown command\n";
        }

        doWrite(final_response);
    }

    void doWrite(const std::string& response)
    {
        auto self(shared_from_this());
        auto responsePtr = std::make_shared<std::string>(response);

        boost::asio::async_write(m_socket, boost::asio::buffer(*responsePtr),
            [this, self, responsePtr](boost::system::error_code ec_write, std::size_t length)
            {
                if (ec_write)
                {
                }
                else {
                }
                doRead();
            });
    }

    std::string trim(const std::string& str)
    {
        size_t first = str.find_first_not_of(" \t\n\r\f\v");
        if (std::string::npos == first)
        {
            return "";
        }
        size_t last = str.find_last_not_of(" \t\n\r\f\v");
        return str.substr(first, (last - first + 1));
    }

    tcp::socket m_socket;
    std::shared_ptr<Database> m_db;
    boost::asio::streambuf m_buffer;
};