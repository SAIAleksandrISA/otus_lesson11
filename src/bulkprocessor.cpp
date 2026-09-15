// src/core/bulkprocessor.cpp

#include "bulkprocessor.h"
#include <sstream>
#include <memory>
#include <iostream> // дКЪ std::cout Х std::cerr
#include <string>   // дКЪ std::string

namespace bulk
{

    BulkProcessor::BulkProcessor(int nSize,
        ThreadSafeQueue<Block>& log_queue,
        ThreadSafeQueue<Block>& file_queue)
        : m_nSize(nSize)
        , m_depth(0)
        , m_isTimeSet(false)
        , m_log_queue(log_queue)
        , m_file_queue(file_queue)
    {
        // !!! днаюбкемн дкъ нркюдйх !!!
        std::cout << "[DEBUG] BulkProcessor created with nSize: " << m_nSize << std::endl;
        // !!! йнмеж днаюбкемн !!!
    }

    void BulkProcessor::processBuffer(const char* data, size_t size)
    {
        // !!! днаюбкемн дкъ нркюдйх !!!
        std::string input_chunk(data, size);
        std::cout << "[DEBUG] BulkProcessor::processBuffer received chunk of size " << size << ": '" << input_chunk << "'" << std::endl;
        // !!! йнмеж днаюбкемн !!!

        std::string input = m_bufferRemainder + std::string(data, size);

        size_t last_newline = input.find_last_of('\n');

        if (last_newline == std::string::npos)
        {
            m_bufferRemainder = input;
            return;
        }

        m_bufferRemainder = input.substr(last_newline + 1);
        std::string processable = input.substr(0, last_newline);
        std::stringstream ss(processable);
        std::string command;

        while (std::getline(ss, command))
        {
            if (!command.empty() && command.back() == '\r')
            {
                command.pop_back();
            }
            if (!command.empty())
            {
                // !!! днаюбкемн дкъ нркюдйх !!!
                std::cout << "[DEBUG] BulkProcessor::processBuffer about to call processCommand for: '" << command << "'" << std::endl;
                // !!! йнмеж днаюбкемн !!!
                processCommand(command);
            }
        }
    }

    void BulkProcessor::commitBlock(bool ignore)
    {
        // !!! днаюбкемн дкъ нркюдйх !!!
        std::cout << "[DEBUG] BulkProcessor::commitBlock CALLED. ignore: " << ignore << ", current commands size: " << m_currentcommands.size() << ", depth: " << m_depth << std::endl;
        // !!! йнмеж днаюбкемн !!!

        if (m_currentcommands.empty())
        {
            // !!! днаюбкемн дкъ нркюдйх !!!
            std::cout << "[DEBUG] commitBlock: currentcommands is empty, returning." << std::endl;
            // !!! йнмеж днаюбкемн !!!
            return;
        }

        if (ignore && m_depth > 0)
        {
            // !!! днаюбкемн дкъ нркюдйх !!!
            std::cout << "[DEBUG] commitBlock: Ignoring block (ignore=true, depth>0)." << std::endl;
            // !!! йнмеж днаюбкемн !!!
        }
        else
        {
            Block block{ m_currentcommands, m_currenttimes };
            // !!! днаюбкемн дкъ нркюдйх !!!
            std::cout << "[DEBUG] commitBlock: Pushing block with " << block.m_commands.size() << " commands to queues." << std::endl;
            // !!! йнмеж днаюбкемн !!!
            m_log_queue.push(block);
            m_file_queue.push(block);
        }

        resetBlockState();
        // !!! днаюбкемн дкъ нркюдйх !!!
        std::cout << "[DEBUG] BulkProcessor::commitBlock finished. State reset." << std::endl;
        // !!! йнмеж днаюбкемн !!!
    }

    void BulkProcessor::processCommand(const std::string& command)
    {
        // !!! днаюбкемн дкъ нркюдйх !!!
        std::cout << "[DEBUG] BulkProcessor::processCommand called with: '" << command << "', current commands size: " << m_currentcommands.size() << ", depth: " << m_depth << std::endl;
        // !!! йнмеж днаюбкемн !!!

        if (command == "{")
        {
            if (m_depth == 0)
            {
                // !!! днаюбкемн дкъ нркюдйх !!!
                std::cout << "[DEBUG] BulkProcessor::processCommand found '{' at depth 0, about to commit block." << std::endl;
                // !!! йнмеж днаюбкемн !!!
                commitBlock(false);
            }
            m_depth++;
        }
        else if (command == "}")
        {
            if (m_depth > 0)
            {
                m_depth--;
                if (m_depth == 0)
                {
                    // !!! днаюбкемн дкъ нркюдйх !!!
                    std::cout << "[DEBUG] BulkProcessor::processCommand found '}' and returned to depth 0, about to commit block." << std::endl;
                    // !!! йнмеж днаюбкемн !!!
                    commitBlock(false);
                }
            }
            else
            {
                // !!! днаюбкемн дкъ нркюдйх !!!
                std::cout << "[DEBUG] BulkProcessor::processCommand found '}' with depth 0 (likely an error or extra '}' for this block)." << std::endl;
                // !!! йнмеж днаюбкемн !!!
                if (!m_isTimeSet)
                {
                    m_currenttimes = std::chrono::system_clock::now();
                    m_isTimeSet = true;
                }
                m_currentcommands.push_back(command);
            }
        }
        else // щРН НАШВМЮЪ ЙНЛЮМДЮ
        {
            if (!m_isTimeSet)
            {
                m_currenttimes = std::chrono::system_clock::now();
                m_isTimeSet = true;
            }
            m_currentcommands.push_back(command);

            if (m_depth == 0 && (int)m_currentcommands.size() == m_nSize)
            {
                // !!! днаюбкемн дкъ нркюдйх !!!
                std::cout << "[DEBUG] BulkProcessor::processCommand reached block size (" << m_nSize << ") at depth 0, about to commit block." << std::endl;
                // !!! йнмеж днаюбкемн !!!
                commitBlock(false);
            }
        }
    }

    void BulkProcessor::processEOF()
    {
        // !!! днаюбкемн дкъ нркюдйх !!!
        std::cout << "[DEBUG] BulkProcessor::processEOF CALLED. Current commands size: " << m_currentcommands.size() << ", depth: " << m_depth << std::endl;
        // !!! йнмеж днаюбкемн !!!
        if (!m_currentcommands.empty())
            commitBlock(true);
    }

    void BulkProcessor::resetBlockState()
    {
        // !!! днаюбкемн дкъ нркюдйх !!!
        // std::cout << "[DEBUG] BulkProcessor::resetBlockState called." << std::endl; // лНФЕР АШРЭ ЯКХЬЙНЛ ЛМНЦН БШБНДЮ
        // !!! йнмеж днаюбкемн !!!
        m_currentcommands.clear();
        m_currenttimes = std::chrono::time_point<std::chrono::system_clock>();
        m_isTimeSet = false;
    }

}