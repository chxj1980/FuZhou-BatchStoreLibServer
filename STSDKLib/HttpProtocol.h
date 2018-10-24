#pragma once
#include <string>
using namespace std;

#define HL_HTTP_VERSION_STR " HTTP/1.1"
#define HL_HTTP_BREAK_LINE  "\r\n"
#define HL_HTTP_POST_STR    "POST"
#define HL_HTTP_GET_STR     "GET"
#define HL_HTTP_TWO_POINTS  ": "
#define HL_HTTP_ASS         " http://"
enum E_HTTP_METHOD
{
    HL_HTTP_POST = 0,
    HL_HTTP_GET
};

enum E_HTTP_RESP
{
    HL_HTTP_HEADER = 0,
    HL_HTTP_BODY,
    HL_HTTP_ALL
};

enum E_HTTP_CODE
{
    HL_HTTP_CONTINUE              = 100,
    HL_HTTP_SWITCHING_PROTOCOLS   = 101,
    HL_HTTP_SUCCESS               = 200,
    HL_HTTP_CREATED               = 201,
    HL_HTTP_ACCEPTED              = 202,
    HL_HTTP_NO_CONTENT            = 204,
    HL_HTTP_RESET_CONTENT         = 205,
    HL_HTTP_PARTIAL_CONTENT       = 206,
    HL_HTTP_FOUND                 = 302,
    HL_HTTP_NOT_MODIFIED          = 304,
    HL_HTTP_USE_PROXY             = 305,
    HL_HTTP_BAD_REQUEST           = 400,
    HL_HTTP_UNAUTHORIZED          = 401,
    HL_HTTP_FORBIDDEN             = 403,
    HL_HTTP_NOT_FOUND             = 404,
    HL_HTTP_REQUEST_TIMEOUT       = 408,
    HL_HTTP_INTERNAL_SERVER_ERROR = 500,
    HL_HTTP_BAD_GATEWAY           = 502,
    HL_HTTP_SERVICE_UNAVAILABLE   = 503,
    HL_HTTP_VERSION_NOT_SUPPORTED = 505
};

class HttpProtocol
{
public:
    HttpProtocol(void);
    ~HttpProtocol(void);
    bool SetAddr(string sAddr);
    bool SetURL(string sURL);
    bool setRequestMethod(E_HTTP_METHOD method);
    bool setRequestProperty(string property, string value);
    string GetHttpHead();
public:
    string m_sServerAddr;       //服务地址IP:Port
    string m_sURL;              //服务参数
    string m_sHttpHead;
};
