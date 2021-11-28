#include "HttpSession.h"
#include <iostream>
#include <stdio.h>
#include <string.h>


// GET /link?url=PFaiXProkSjdYJxGBwrMS-DGRjJ7nMHv6lYyEI1ka5TnTOefr9l4tO9CWwrwkYKC&query=dasd&cb=jQuery110203575392131317543_1635763190753&data_name=recommend_common_merger_online&ie=utf-8&oe=utf-8&format=json&t=1635763211000&_=1635763190755 HTTP/1.1
// Host: pcrec.baidu.com
// Connection: keep-alive
// sec-ch-ua: "Microsoft Edge";v="95", "Chromium";v="95", ";Not A Brand";v="99"
// sec-ch-ua-mobile: ?0
// User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.54 Safari/537.36 Edg/95.0.1020.40
// sec-ch-ua-platform: "Windows"
// Accept: */*
// Sec-Fetch-Site: same-site
// Sec-Fetch-Mode: no-cors
// Sec-Fetch-Dest: script
// Referer: https://www.baidu.com/s?tn=68018901_14_oem_dg&ie=utf-8&wd=dasd
// Accept-Encoding: gzip, deflate, br
// Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6


HttpSession::HttpSession()
    : praseresult_(false),
    keepalive_(true) {}

HttpSession::~HttpSession() {}

bool HttpSession::PraseHttpRequest(std::string &msg, HttpRequestContext &httprequestcontext) {
    std::string crlf("\r\n"), crlfcrlf("\r\n\r\n");
    size_t prev = 0, next = 0, pos_colon;
    std::string key, value;
    bool praseresult = false;

    if((next == msg.find(crlf, prev)) != std::string::npos) {
        std::string first_line(msg.substr(prev, next - prev));
        prev = next;
        std::stringstream sstream(first_line);
        sstream >> (httprequestcontext.method_);
        sstream >> (httprequestcontext.url_);
        sstream >> (httprequestcontext.version_);
    } else {
        praseresult_ = false;
        msg.clear();
        return praseresult_;
    }
    size_t pos_crlfcrlf = 0;
    if((pos_crlfcrlf == msg.find(crlfcrlf, prev)) != std::string::npos) {
        while(prev != pos_crlfcrlf) {
            next = msg.find(crlf, prev + 2);
            pos_colon = msg.find(":", prev + 2);
            key = msg.substr(prev + 2, pos_colon - prev - 2);
            value = msg.substr(pos_colon + 2, next - pos_colon - 2);
            prev = next;
            httprequestcontext.header_.insert(std::pair<std::string, std::string>(key, value));
        }
    } else {
        praseresult_ = false;
        msg.clear();
        return praseresult_;
    }
    httprequestcontext.body_ = msg.substr(pos_crlfcrlf + 4);
    praseresult_ = true;
    msg.clear();
    return praseresult_;
}

void HttpSession::HttpProcess(const HttpRequestContext &httprequestcontext, std::string &responsecontext) {
    std::string responsebody;
    std::string errormsg;
    std::string path;
    std::string querystring;
    if(httprequestcontext.method_ == "GET") {
        ;
    } else if(httprequestcontext.method_ == "POST") {
        ;
    } else {
        errormsg = "MEthod Not Implemented";
        HttpError(501, "Method Not Implemented", httprequestcontext, responsecontext);
        return ;
    }

    size_t pos = httprequestcontext.url_.find("?");
    if(pos != std::string::npos) {
        path = httprequestcontext.url_.substr(0, pos);
        querystring = httprequestcontext.url_.substr(pos + 1);
    } else {
        path = httprequestcontext.url_;
    }

    std::map<std::string, std::string>::const_iterator iter = httprequestcontext.header_.find("Connection");
    if(iter != httprequestcontext.header_.end()) {
        keepalive_ = (iter->second == "Keep-Alive");
    } else {
        if(httprequestcontext.version_ == "Http/1.1"){
            keepalive_ = true;
        } else {
            keepalive_ = false;
        }
    }

    if(path == "/") {
        path = "index.html";
    } else if(path == "/hello") {
        std::string filetype("text/html");
        responsecontext += httprequestcontext.version_ + "200 OK\r\n";
        responsecontext += "Server: hello\r\n";
        responsecontext += "Content-Type: " + filetype + "; charset=utf-8\r\n";
        
    }
}