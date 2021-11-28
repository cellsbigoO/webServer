#ifndef _HTTP_SESSION_H_
#define _HTTP_SESSION_H_

#include <string>
#include <sstream>
#include <map>

typedef struct HttpRequestContext {
    std::string method_;
    std::string url_;
    std::string version_;
    std::map<std::string, std::string> header_;
    std::string body_;
};

typedef struct  HttpResponseContext{
    std::string version_;
    std::string statecode;
    std::string statemsg;
    std::map<std::string, std::string> header;
    std::string body;
};

class HttpSession {
private:
    HttpRequestContext httpprequestcontext_;
    bool praseresult_;
    std::string responsecontext_;
    std::string responsebody_;
    std::string errormsg;
    std::string path_;
    std::string querystring_;

    bool keepalive_;
    std::string body_buff;

public:
    HttpSession();
    ~HttpSession();
    bool PraseHttpRequest(std::string &s, HttpRequestContext &httpResponseContext);
    void HttpProcess(const HttpRequestContext &httpPrequestcontext, std::string &responsecontext);
    void HttpError(const int err_num, const std::string short_msg, const HttpRequestContext &httpRequestContext, std::string &responsecontext);
    bool KeepAlive() {
        return keepalive_;
    }
    
};
#endif