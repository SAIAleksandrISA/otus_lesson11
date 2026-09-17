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

        boost::asio::async_read_until(m_socket, m_buffer, '\n',
            [this, self](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    std::istream is(&m_buffer);
                    std::string raw_command;
                    std::getline(is, raw_command);
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
        size_t first_space = command.find(' ');
        std::string cmd_token;
        std::string args_str;
        std::string final_response;

        if (first_space == std::string::npos)
        {
            cmd_token = command;
            args_str = "";
        }
        else
        {
            cmd_token = command.substr(0, first_space);
            args_str = command.substr(first_space + 1);
        }

        if (cmd_token == "INSERT")
        {
            size_t second_space = args_str.find(' ');
            if (second_space != std::string::npos)
            {
                std::string table_name = args_str.substr(0, second_space);
                std::string rest_of_args = args_str.substr(second_space + 1);

                size_t third_space = rest_of_args.find(' ');
                if (third_space != std::string::npos)
                {
                    std::string id_str = rest_of_args.substr(0, third_space);
                    std::string name_val = rest_of_args.substr(third_space + 1);

                    try
                    {
                        int id = std::stoi(id_str);
                        if (m_db->insert(table_name, id, name_val))
                        {
                            final_response = "OK\n";
                        }
                        else
                        {
                            final_response = "ERR duplicate " + id_str + "\n";
                        }
                    }
                    catch (...) { final_response = "ERR invalid arguments\n"; }
                }
                else { final_response = "ERR invalid command format\n"; }
            }
            else { final_response = "ERR invalid command format\n"; }
        }
        else if (cmd_token == "TRUNCATE")
        {
            if (args_str.find(' ') == std::string::npos && !args_str.empty())
            {
                m_db->truncate(args_str);
                final_response = "OK\n";
            }
            else { final_response = "ERR invalid command format\n"; }
        }
        else if (cmd_token == "INTERSECTION")
        {
            if (args_str.empty())
            {
                auto result = m_db->intersect();
                std::string response_data;
                for (const auto& line : result)
                {
                    response_data += line + "\n";
                }
                final_response = response_data + "OK\n";
            }
            else
            {
                final_response = "ERR invalid command format\n";
            }
        }
        else if (cmd_token == "SYMMETRIC_DIFFERENCE")
        {
            if (args_str.empty())
            {
                auto result = m_db->symmetricDifference();
                std::string response_data;
                for (const auto& line : result)
                {
                    response_data += line + "\n";
                }
                final_response = response_data + "OK\n";
            }
            else
            {
                final_response = "ERR invalid command format\n";
            }
        }
        else if (cmd_token.empty())
        {
            final_response = "ERR empty command\n";
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