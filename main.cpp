#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "TcpServer.h"
#include "HttpUtil.h"
#include "Utils.h"

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

void messageCallback(const TcpConnectionPtr &conn, Buffer &inputBuffer) {
    HttpUtil::Method method;
    char url[4096];
    HttpUtil::parseRequestLine(inputBuffer.first(), &method, url, nullptr);
    inputBuffer.retrieveAll();
    if (method == HttpUtil::Method::GET) {
        Utils::utf8Decode(url, url);
        const char *file = url + 1;
        if (strcmp(url, "/") == 0) file = "./";
        struct stat st;
        char buffer[4096];
        if(stat(file, &st) == -1) {     
            size_t length = HttpUtil::putResponse(buffer, sizeof(buffer), HttpUtil::StatusCode::NOT_FOUND_404, ".html", -1);
            length += Utils::putNotFoundPage(buffer + length, sizeof(buffer) - length, "No such file or directory");
            conn->send(buffer, length);
            conn->close();
        } else {
            if (S_ISDIR(st.st_mode)) {
                char response[1024];
                size_t bufLen = Utils::putDirectory(buffer, sizeof(buffer), file);
                size_t responseLen = HttpUtil::putResponse(response, sizeof(response), HttpUtil::StatusCode::OK_200, ".html", bufLen);
                conn->send(response, responseLen);
                conn->send(buffer, bufLen);
            } else if (S_ISREG(st.st_mode)) {
                const char *mime;
                mime = strrchr(url, '.'); 
                size_t length = HttpUtil::putResponse(buffer, sizeof(buffer), HttpUtil::StatusCode::OK_200, mime, st.st_size);
                conn->send(buffer, length);
                Utils::sendFile(conn, file);
            }
        }
    } else {
        conn->close();
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("usage: ./main <port> <directory>\n");
        exit(-1);
    }
    if (chdir(argv[2]) == -1) {
        perror("chdir error");
        exit(1);
    }
    TcpServer tcpServer(InetAddress(atoi(argv[1])), TcpServer::MessageCallback(messageCallback));
    tcpServer.start(3);
    return 0;
}