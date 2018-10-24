#include "stdafx.h"
#include "HttpProtocol.h"

HttpProtocol::HttpProtocol()
{
    m_sHttpHead = "";
    m_sURL = "";
}

HttpProtocol::~HttpProtocol()
{
}
bool HttpProtocol::SetAddr(string sAddr)
{
    m_sServerAddr = sAddr;
    return true;
}
bool HttpProtocol::SetURL(string sURL)
{
    m_sURL = sURL;
    return true;
}   

bool HttpProtocol::setRequestMethod(E_HTTP_METHOD method, E_HTTP_CODE ResCode)
{
    switch(method)
    {
    case HL_HTTP_POST:
        m_sHttpHead += HL_HTTP_POST_STR;
        break;

    case HL_HTTP_GET:
        m_sHttpHead += HL_HTTP_GET_STR;
        break;
    case HL_HTTP_RESPONSE:
        m_sHttpHead += HL_HTTP_VERSION_STR;
        m_sHttpHead += HL_HTTP_REPOK;
        break;
    };

    if (method != HL_HTTP_RESPONSE)
    {
        m_sHttpHead += " " + m_sURL;
        m_sHttpHead += " " + (string)HL_HTTP_VERSION_STR;
    }
    m_sHttpHead += HL_HTTP_BREAK_LINE;

    return true;
}

bool HttpProtocol::setRequestProperty(string property, string value)
{
    if(property.size() == 0 || value.size() == 0)
    {
        return false;
    }

    m_sHttpHead += property;
    m_sHttpHead += HL_HTTP_TWO_POINTS;
    m_sHttpHead += value;
    m_sHttpHead += HL_HTTP_BREAK_LINE;

    return true;
}
string HttpProtocol::GetHttpHead()
{
    m_sHttpHead += HL_HTTP_BREAK_LINE;
    return m_sHttpHead;
}
string HttpProtocol::GetHttpBody(string sHttpRequest)
{
    string sBody = "";
    size_t nPos = sHttpRequest.find(HL_HTTP_BODYBOUNDARY);
    if (nPos != string::npos)
    {
        string sBody(sHttpRequest, nPos + 4, sHttpRequest.size() - nPos - 4);
    }
    return sBody;
}
bool HttpProtocol::SetHttpBody(string sBody)
{
    if (sBody != "")
    {
        char pBodyLen[12] = { 0 };
        sprintf_s(pBodyLen, sizeof(pBodyLen), "%d", sBody.size());
        setRequestProperty("Content-Length", pBodyLen);

        m_sHttpHead += HL_HTTP_BREAK_LINE;
        m_sHttpHead += sBody;
        m_sHttpHead += HL_HTTP_BREAK_LINE;
    }
    return true;
}
string HttpProtocol::GetHTTPMsg()
{
    return m_sHttpHead;
}