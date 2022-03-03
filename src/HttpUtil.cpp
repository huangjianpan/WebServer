#include "HttpUtil.h"
#include "Utils.h"

std::unordered_map<HttpUtil::StatusCode, const char *> HttpUtil::status = {
    { HttpUtil::StatusCode::OK_200, "OK" },
    { HttpUtil::StatusCode::NOT_FOUND_404, "Not Found"}
};

std::unordered_map<std::string, const char *> HttpUtil::contentTypes {
    { ".html", "text/html; charset=utf-8" },
    { ".xml", "text/xml; charset=utf-8" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt", "text/plain; charset=utf-8" },
    { ".rtf", "application/rtf" },
    { ".pdf", "application/pdf" },
    { ".word", "application/msword" },
    { ".png", "image/png" },
    { ".gif", "image/gif" },
    { ".jpg", "image/jpeg" },
    { ".jpeg", "image/jpeg" },
    { ".au", "audio/basic" },
    { ".mpeg", "video/mpeg" },
    { ".mpg", "video/mpeg" },
    { ".avi", "video/x-msvideo" },
    { ".gz", "application/x-gzip" },
    { ".tar", "application/x-tar" },
    { ".css", "text/css; charset=utf-8" }
};

void HttpUtil::parseRequestLine(const char *requestLine, Method *method, char *url, Protocol *protocol) {
    char m[10], p[15];
    sscanf(requestLine, "%[^ ] %[^ ] %[^ ]", m, url, p);
    Utils::utf8Decode(url, url);
    if (method) {
        if (strcmp(m, "GET") == 0) {
            *method = Method::GET;
        } else if (strcmp(m, "POST") == 0) {
            *method = Method::POST;
        } else if (strcmp(m, "PUT") == 0) {
            *method = Method::PUT;
        } else {
            *method = Method::UNKNOWN;
        }
    }
    if (protocol) {
        if (strcasecmp(p, "HTTP/1.0") == 0) {
            *protocol = Protocol::HTTP_10;
        } else if (strcasecmp(p, "HTTP/1.1") == 0) {
            *protocol = Protocol::HTTP_11;
        } else if (strcasecmp(p, "HTTP/2.0") == 0) {
            *protocol = Protocol::HTTP_20;
        } else {
            *protocol = Protocol::UNKNOWN;
        }
    }
}

size_t HttpUtil::parseLine(const char *data) {
    const char *ptr = data;
    while (*ptr != '\0') {
        ++ptr;
        if (*ptr == '\n' && *(ptr - 1) == '\r') {
            ++ptr;
            break;
        }
    }
    return static_cast<size_t>(ptr - data);
}

size_t HttpUtil::putStatusLine(char *buffer, size_t maxlen, StatusCode code) {
    return (size_t) snprintf(buffer, maxlen, "%s %d %s\r\n", "HTTP/1.1", (int) code, status[code]);
}

size_t HttpUtil::putResponse(char *buffer, size_t maxlen, StatusCode code, const char *mime, ssize_t length) {
    char *head = buffer;
    if (code == StatusCode::NOT_FOUND_404) {
        mime = ".html";
    }
    buffer += putStatusLine(buffer, maxlen, code);
    buffer += snprintf(buffer, maxlen - (buffer - head), 
        "Content-Encoding: null\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: %s\r\n"
        "\r\n"
        , contentType(mime), length, code == StatusCode::NOT_FOUND_404 ? "close" : "keep-alive"
    );
    return static_cast<size_t>(buffer - head);
}

const char *HttpUtil::contentType(const char *type) {
    if (!type) {
        return "text/plain; charset=utf-8";
    }
    auto it = contentTypes.find(type);
    if (it == contentTypes.end()) {
        return "text/plain; charset=utf-8";
    }
    return it->second;
}