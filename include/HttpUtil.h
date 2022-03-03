#ifndef __HTTP_UTIL_H__
#define __HTTP_UTIL_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unordered_map>
#include <functional>

class HttpUtil {
public:
    enum struct Method { GET, POST, PUT, UNKNOWN };
    enum struct Protocol { HTTP_10, HTTP_11, HTTP_20, UNKNOWN };
    enum struct StatusCode {
        OK_200 = 200,
        NOT_FOUND_404 = 404
    };

    static std::unordered_map<StatusCode, const char *> status;

    static std::unordered_map<std::string, const char *> contentTypes;

public:
    static void parseRequestLine(const char *requestLine, Method *method, char *url, Protocol *protocol);

    static size_t parseLine(const char *data);

    static size_t putStatusLine(char *buffer, size_t maxlen, StatusCode code);

    static size_t putResponse(char *buffer, size_t maxlen, StatusCode code, const char *mime, ssize_t length);

private:
    static const char *contentType(const char *type);
};
#endif // !__HTTP_UTIL_H__