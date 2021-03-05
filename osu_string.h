//
// Created by hsyuan on 2021-03-05.
//

#ifndef PROJECT_OSU_STRING_H
#define PROJECT_OSU_STRING_H
#include <vector>
#include "osu_buffer.h"
namespace osu {

    static bool start_with(const std::string &str, const std::string &head)
    {
        return str.compare(0, head.size(), head) == 0;
    }

    static std::string file_name_from_path(const std::string& path, bool hasExt){
        int pos = path.find_last_of('/');
        std::string str = path.substr(pos+1, path.size());
        if (!hasExt) {
            pos = str.find_last_of('.');
            if (std::string::npos != pos) {
                str = str.substr(0, pos);
            }
        }
        return str;
    }

    static std::string file_ext_from_path(const std::string& str){
        std::string ext;
        auto pos = str.find_last_of('.');
        if (std::string::npos != pos) {
            ext = str.substr(0, pos);
        }
        return ext;
    }

    static std::vector<std::string> split(const std::string& str1, const std::string& pattern)
    {
        std::string::size_type pos;
        std::vector<std::string> result;
        std::string str = str1 + pattern;
        size_t size = str.size();

        for (size_t i = 0; i < size; i++)
        {
            pos = str.find(pattern, i);
            if (pos < size)
            {
                std::string s = str.substr(i, pos - i);
                result.push_back(s);
                i = pos + pattern.size() - 1;
            }
        }
        return result;
    }

    static std::string format(const char *fmt, ...) {

        AutoBuffer<char, 1024> buf;

        for ( ; ; )
        {
            va_list va;
            va_start(va, fmt);
            int bsize = static_cast<int>(buf.size());
            int len = vsnprintf(buf.data(), bsize, fmt, va);
            va_end(va);

            assert(len >= 0 && "Check format string for errors");
            if (len >= bsize)
            {
                buf.resize(len + 1);
                continue;
            }
            buf[bsize - 1] = 0;
            return std::string(buf.data(), len);
        }
    }
}
#endif //PROJECT_OSU_STRING_H
