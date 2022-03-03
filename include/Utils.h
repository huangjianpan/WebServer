#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>
#include <stdio.h>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <dirent.h>

class TcpConnection;

class Utils {
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

    static size_t putNotFoundPage(char *buffer, size_t maxlen, const char *text);

    static ssize_t sendFile(const TcpConnectionPtr &connection, const char *filePath);

    static size_t putDirectory(char *buffer, size_t maxlen, const char *dirPath);

    static void utf8Encode(char *to, size_t tosize, const char *from);

    static void utf8Decode(char *to, char *from);

    static int hexit(char c);
};
#endif // !__UTILS_H__