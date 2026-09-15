#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include "database.h"
#include "server.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    try
    {
        unsigned short port = static_cast<unsigned short>(std::stoi(argv[1]));
        boost::asio::io_context ioContext;
        auto db = std::make_shared<Database>();

        Server server(ioContext, port, db);

        std::vector<std::thread> threads;
        unsigned int threadCount = std::thread::hardware_concurrency();
        if (threadCount == 0) threadCount = 2;

        std::cout << "Starting server on port " << port << " with " << threadCount << " threads." << std::endl;

        for (unsigned int i = 0; i < threadCount; ++i)
        {
            threads.emplace_back([&ioContext]()
                {
                    ioContext.run();
                });
        }

        for (auto& t : threads)
        {
            t.join();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}