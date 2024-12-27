#pragma once

#include <iostream>
#include <fstream>

namespace _NES
{
    enum Level
    {
        None,
        Error,
        Info,
        InfoVerbose,
        CpuTrace
    };

    class Log
    {
    public:
        ~Log();
        void setLogStream(std::ostream& stream);
        void setCpuTraceStream(std::ostream& stream);
        Log& setLevel(Level level);
        Level getLevel();

        std::ostream& getStream();
        std::ostream& getCpuTraceStream();

        static Log& get();
    private:
        Level m_logLevel;
        std::ostream* m_logStream;
        std::ostream* m_cpuTrace;
    };

    //Courtesy of http://wordaligned.org/articles/cpp-streambufs#toctee-streams
    // 分流输入数据，使两个流同步，将日志分别输出到控制台和文件中
    class TeeBuf : public std::streambuf
    {
    public:
        // Construct a streambuf which tees output to both input
        // streambufs.
        TeeBuf(std::streambuf* sb1, std::streambuf* sb2);
    private:
        // This tee buffer has no buffer. So every character "overflows"
        // and can be put directly into the teed buffers.
        virtual int overflow(int c);
        // Sync both teed buffers.
        virtual int sync();
    private:
        std::streambuf* m_sb1;
        std::streambuf* m_sb2;
    };

    class TeeStream : public std::ostream
    {
    public:
        // Construct an ostream which tees output to the supplied
        // ostreams.
        TeeStream(std::ostream& o1, std::ostream& o2);
    private:
        TeeBuf m_tbuf;
    };
}