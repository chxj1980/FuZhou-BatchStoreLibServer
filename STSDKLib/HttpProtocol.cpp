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

bool HttpProtocol::setRequestMethod(E_HTTP_METHOD method)
{
    switch(method)
    {
    case HL_HTTP_POST:
        m_sHttpHead += HL_HTTP_POST_STR;
        break;

    case HL_HTTP_GET:
        m_sHttpHead += HL_HTTP_GET_STR;
        break;
    };

    m_sHttpHead += " " + m_sURL;
    m_sHttpHead += HL_HTTP_VERSION_STR;
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
