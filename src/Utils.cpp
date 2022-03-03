#include "Utils.h"
#include "TcpConnection.h"

size_t Utils::putNotFoundPage(char *buffer, size_t maxlen, const char *text) {
    int ret = snprintf(buffer, maxlen, 
        "<html>"
            "<head>"
                "<title>404 Not Found</title>"
            "</head>"
            "<body>"
                "<h1 align=\"center\">404 Not Found</h1>"
                "<hr>"
                "<h2 align=\"center\">%s</h2>"
            "</body>"
        "</html>"
        , text ? text : ""
    );
    return static_cast<size_t>(ret);
}

ssize_t Utils::sendFile(const TcpConnectionPtr &connection, const char *filePath) {
    int fd = ::open(filePath, O_RDONLY);
    if (fd < 0) return -1;
    char buf[65536];
    ssize_t nread, total = 0;
    while ((nread = ::read(fd, buf, sizeof(buf))) > 0) {
        if (!connection->isConnected()) break;
        connection->send(buf, nread);
        total += nread;
    }
    ::close(fd);
    return total;
}

size_t Utils::putDirectory(char *buffer, size_t maxlen, const char *dirPath) {
    const char *head = buffer;
    buffer += snprintf(buffer, maxlen, 
        "<html>"
            "<head><title>Directory: %s</title></head>"
            "<body>"
                "<h1>Directory: %s</h1>"
                "<table>"
        , dirPath, dirPath
    );
    char path[1024];
    char code[1024];
    struct dirent **namelist;
    int num = scandir(dirPath, &namelist, NULL, alphasort);
    if (num < 0) return -1;
    for (int i = 0; i < num; ++i) {
        char *name = namelist[i]->d_name;
        snprintf(path, sizeof(path), "%s%s", dirPath, name);
        struct stat st;
        if (stat(path, &st) < 0) continue;
        Utils::utf8Encode(code, sizeof(code), name);
        if (S_ISREG(st.st_mode)) {
            buffer += snprintf(buffer, maxlen - (buffer - head), 
                "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>"
                , code, name, st.st_size
            );
        } else if (S_ISDIR(st.st_mode)) {
            buffer += snprintf(buffer, maxlen - (buffer - head), 
                "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>"
                , code, name, st.st_size
            );
        }
    }
    buffer += snprintf(buffer, maxlen - (buffer - head), "</table></body></html>");
    return static_cast<size_t>(buffer - head);
}

void Utils::utf8Encode(char *to, size_t tosize, const char *from) {
    for (size_t tolen = 0; *from != '\0' && tolen + 4 < tosize; ++from) {    
        if (isalnum(*from) || strchr("/_.-~", *from) != (char*) 0) {      
            *to = *from;
            ++to;
            ++tolen;
        } else {
            sprintf(to, "%%%02x", (int) *from & 0xff);
            to += 3;
            tolen += 3;
        }
    }
    *to = '\0';
}

void Utils::utf8Decode(char *to, char *from) {
    for ( ; *from != '\0'; ++to, ++from) {     
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {       
            *to = hexit(from[1]) * 16 + hexit(from[2]);
            from += 2;                      
        } else {
            *to = *from;
        }
    }
    *to = '\0';
}

int Utils::hexit(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0;
}