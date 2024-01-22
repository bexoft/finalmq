//MIT License

//Copyright (c) 2020 bexoft GmbH (mail@bexoft.de)

//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include "finalmq/protocols/ProtocolHttpClient.h"

#include "finalmq/helpers/Utils.h"
#include "finalmq/protocolsession/ProtocolMessage.h"
#include "finalmq/protocolsession/ProtocolRegistry.h"
#include "finalmq/protocolsession/ProtocolSession.h"
#include "finalmq/streamconnection/Socket.h"

//#include "finalmq/helpers/ModulenameFinalmq.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>

#include <fcntl.h>
#include <time.h>

namespace finalmq
{
static const char tabDecToHex[] = {
    '0',
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    'A',
    'B',
    'C',
    'D',
    'E',
    'F'};

static void encode(std::string& dest, const std::string& src)
{
    dest.reserve(src.size() * 3);
    char c;
    unsigned char uc;

    for (size_t i = 0; i < src.size(); ++i)
    {
        c = src[i];
        if (isalnum(c) || c == '/' || c == '-' || c == '_' || c == '.' || c == '~')
        {
            dest += c;
        }
        else
        {
            uc = c;
            int first = (uc >> 4) & 0x0f;
            int second = uc & 0x0f;
            assert(0 <= first && first < 16);
            assert(0 <= second && second < 16);
            dest += '%';
            dest += tabDecToHex[first];
            dest += tabDecToHex[second];
        }
    }
}

static void decode(std::string& dest, const std::string& src)
{
    dest.reserve(src.size());
    char code[3] = {0};
    unsigned long c = 0;

    for (size_t i = 0; i < src.size(); ++i)
    {
        if (src[i] == '%')
        {
            ++i;
            memcpy(code, &src[i], 2);
            c = strtoul(code, NULL, 16);
            dest += static_cast<char>(c);
            ++i;
        }
        else
        {
            dest += src[i];
        }
    }
}

//---------------------------------------
// Cookie
//---------------------------------------
class Cookie
{
public:
    inline Cookie()
        : m_expirationDate(-1), m_secure(false), m_httpOnly(false)
    {
    }

    static std::vector<Cookie> parseSetCookieHeaderLine(const std::string& cookieString, const std::string& hostname)
    {
        // According to http://wp.netscape.com/newsref/std/cookie_spec.html,<
        // the Set-Cookie response header is of the format:
        //
        //   Set-Cookie: NAME=VALUE; expires=DATE; path=PATH; domain=DOMAIN_NAME; secure
        //
        // where only the NAME=VALUE part is mandatory
        //
        // We do not support RFC 2965 Set-Cookie2-style cookies

        std::vector<Cookie> result;

        int position = 0;
        const int length = static_cast<int>(cookieString.length());
        while (position < length)
        {
            Cookie cookie;

            // The first part is always the "NAME=VALUE" part
            std::pair<std::string, std::string> field = nextField(cookieString, position, true);
            if (field.first.empty())
                // parsing error
                break;
            cookie.m_name = field.first;
            cookie.m_value = field.second;
            cookie.m_domain = hostname;
            cookie.m_domainDot = false;

            position = nextNonWhitespace(cookieString, position);
            while (position < length)
            {
                switch(cookieString.at(position++))
                {
                    case ';':
                        // new field in the cookie
                        field = nextField(cookieString, position, false);
                        std::transform(field.first.begin(), field.first.end(), field.first.begin(), [](unsigned char c) { return std::tolower(c); }); // everything but the NAME=VALUE is case-insensitive

                        if (field.first == "expires")
                        {
                            position -= static_cast<int>(field.second.length());
                            int end;
                            for (end = position; end < length; ++end)
                                if (isValueSeparator(cookieString.at(end)))
                                    break;

                            // remove last spaces
                            int endDateStringWithoutSpace = nextNonWhitespaceReverse(cookieString, end, position);
                            std::string dateString = cookieString.substr(position, endDateStringWithoutSpace - position);
                            position = end;
                            std::transform(dateString.begin(), dateString.end(), dateString.begin(), [](unsigned char c) { return std::tolower(c); });
                            time_t date = parseDateString(dateString);
                            if (date == -1)
                            {
                                return result;
                            }
                            cookie.m_expirationDate = date;
                        }
                        else if (field.first == "domain")
                        {
                            std::string rawDomain = field.second;
                            bool maybeLeadingDot = false;
                            if (!rawDomain.empty() && rawDomain[0] == '.')
                            {
                                maybeLeadingDot = true;
                                rawDomain = rawDomain.substr(1);
                            }

                            std::string normalizedDomain = rawDomain; // QUrl::fromAce(QUrl::toAce(rawDomain));
                            if (normalizedDomain.empty() && !rawDomain.empty())
                            {
                                return result;
                            }
                            cookie.m_domain = normalizedDomain;
                            cookie.m_domainDot = maybeLeadingDot;
                        }
                        else if (field.first == "max-age")
                        {
                            if (field.second.empty())
                            {
                                return result;
                            }
                            char* res = nullptr;
                            long long secs = strtoll(field.second.c_str(), &res, 10);
                            if (res != &field.second[field.second.size()])
                            {
                                return result;
                            }
                            const time_t now = time(nullptr);
                            cookie.m_expirationDate = now + secs;
                        }
                        else if (field.first == "path")
                        {
                            decode(cookie.m_path, field.second);
                        }
                        else if (field.first == "secure")
                        {
                            cookie.m_secure = true;
                        }
                        else if (field.first == "httponly")
                        {
                            cookie.m_httpOnly = true;
                        }
                        else if (field.first == "comment")
                        {
                            cookie.m_comment = field.second;
                        }
                        else if (field.first == "version")
                        {
                            if (field.second != "1")
                            {
                                // oops, we don't know how to handle this cookie
                                return result;
                            }
                        }
                        else
                        {
                            // got an unknown field in the cookie
                            // what do we do?
                        }

                        position = nextNonWhitespace(cookieString, position);
                }
            }

            if (!cookie.m_name.empty())
            {
                result.push_back(cookie);
            }
        }

        return result;
    }

    time_t getExpirationDate() const
    {
        return m_expirationDate;
    }

    const std::string& getDomain() const
    {
        return m_domain;
    }
    bool getDomainDot() const
    {
        return m_domainDot;
    }
    const std::string& getPath() const
    {
        return m_path;
    }
    const std::string& getComment() const
    {
        return m_comment;
    }
    const std::string& getName() const
    {
        return m_name;
    }
    const std::string& getValue() const
    {
        return m_value;
    }
    bool getSecure() const
    {
        return m_secure;
    }
    bool getHttpOnly() const
    {
        return m_httpOnly;
    }

    bool match(const std::string& domain, const std::string& path) const
    {
        const bool domainFits = matchDomain(domain);

        if (domainFits)
        {
            if (path == m_path)
            {
                return true;
            }
            else
            {
                if (path.size() > m_path.size())
                {
                    if (!m_path.empty() && m_path[m_path.size() - 1] == '/')
                    {
                        if (path[m_path.size() - 1] == '/')
                        {
                            if (memcmp(&path[0], m_path.c_str(), m_path.size()) == 0)
                            {
                                // m_path = "/hallo/"
                                //   path = "/hello/..."
                                return true;
                            }
                        }
                    }
                    else
                    {
                        if (!m_path.empty())
                        {
                            if (path[m_path.size()] == '/')
                            {
                                if (memcmp(&path[0], m_path.c_str(), m_path.size()) == 0)
                                {
                                    // m_path = "/hallo"
                                    //   path = "/hello/..."
                                    return true;
                                }
                            }
                        }
                        else
                        {
                            if (path[0] == '/')
                            {
                                // m_path = ""
                                //   path = "/..."
                                return true;
                            }
                        }
                    }
                }
                else if (path.size() == 0 && m_path.size() == 1 && m_path[0] == '/')
                {
                    // m_path = "/"
                    //   path = ""
                    return true;
                }
            }
        }

        return false;
    }

    bool matchExactPath(const std::string& domain, const std::string& path) const
    {
        const bool domainFits = matchDomain(domain);

        if (domainFits)
        {
            if (path == m_path)
            {
                return true;
            }
            else
            {
                if (path.size() == m_path.size() + 1)
                {
                    if (m_path.empty() || m_path[m_path.size() - 1] != '/')
                    {
                        if (!m_path.empty())
                        {
                            if (path[m_path.size()] == '/')
                            {
                                if (memcmp(&path[0], m_path.c_str(), path.size()) == 0)
                                {
                                    // m_path = "/hallo"
                                    //   path = "/hello/"
                                    return true;
                                }
                            }
                        }
                        else
                        {
                            if (path[0] == '/')
                            {
                                // m_path = ""
                                //   path = "/"
                                return true;
                            }
                        }
                    }
                }
                else if (path.size() == 0 && m_path.size() == 1 && m_path[0] == '/')
                {
                    // m_path = "/"
                    //   path = ""
                    return true;
                }
            }
        }

        return false;
    }

    bool isExpired() const
    {
        const time_t now = time(nullptr);
        if (m_expirationDate != -1 && now > m_expirationDate)
        {
            return true;
        }
        return false;
    }

private:
    bool matchDomain(const std::string& domain) const
    {
        bool domainFits = false;
        if (m_domain == domain)
        {
            domainFits = true;
        }
        else
        {
            if (m_domainDot)
            {
                if (domain.size() > m_domain.size())
                {
                    const size_t index = domain.size() - m_domain.size() - 1;
                    if (domain[index] == '.')
                    {
                        if (memcmp(&domain[index + 1], m_domain.c_str(), m_domain.size()) == 0)
                        {
                            domainFits = true;
                        }
                    }
                }
            }
        }
        return domainFits;
    }

    static bool checkStaticArray(int& val, const std::string& dateString, int at, const char* array, int size)
    {
        if (dateString[at] < 'a' || dateString[at] > 'z')
            return false;
        if (val == -1 && dateString.length() >= static_cast<size_t>(at + 3))
        {
            int j = 0;
            int i = 0;
            while (i <= size)
            {
                const char* str = array + i;
                if (str[0] == dateString[at]
                   && str[1] == dateString[at + 1]
                   && str[2] == dateString[at + 2])
                {
                    val = j;
                    return true;
                }
                i += static_cast<int>(strlen(str)) + 1;
                ++j;
            }
        }
        return false;
    }

    static inline bool isNumber(char s)
    {
        return s >= '0' && s <= '9';
    }

    static inline bool isWhitespace(char c)
    {
        return c == ' ' || c == '\t';
    }

#ifndef WIN32
    static constexpr int SecondsPerMinute = 60;
    static constexpr int SecondsPerHour = 3600;
    static constexpr int SecondsPerDay = 86400;
    static const int DaysOfMonth[12];

    static bool IsLeapYear(short year)
    {
        if (year % 4 != 0)
            return false;
        if (year % 100 != 0)
            return true;
        return (year % 400) == 0;
    }

    static time_t my_mkgmtime(const struct tm* ptm)
    {
        time_t secs = 0;
        // tm_year is years since 1900
        int year = ptm->tm_year + 1900;
        for (int y = 1970; y < year; ++y)
        {
            secs += (IsLeapYear(static_cast<short>(y)) ? 366 : 365) * SecondsPerDay;
        }
        // tm_mon is month from 0..11
        for (int m = 0; m < ptm->tm_mon; ++m)
        {
            secs += DaysOfMonth[m] * SecondsPerDay;
            if (m == 1 && IsLeapYear(static_cast<short>(year)))
                secs += SecondsPerDay;
        }
        secs += (ptm->tm_mday - 1) * SecondsPerDay;
        secs += ptm->tm_hour * SecondsPerHour;
        secs += ptm->tm_min * SecondsPerMinute;
        secs += ptm->tm_sec;
        return secs;
    }

    //    time_t my_mkgmtime(struct tm * pt) {
    //        time_t ret = mktime(pt);
    //        /* GMT and local time */
    //        struct tm pgt = *gmtime(&ret);
    //        struct tm plt = *localtime(&ret);
    //        plt.tm_year -= pgt.tm_year - plt.tm_year;
    //        plt.tm_mon -= pgt.tm_mon - plt.tm_mon;
    //        plt.tm_mday -= pgt.tm_mday - plt.tm_mday;
    //        plt.tm_hour -= pgt.tm_hour - plt.tm_hour;
    //        plt.tm_min -= pgt.tm_min - plt.tm_min;
    //        plt.tm_sec -= pgt.tm_sec - plt.tm_sec;
    //        ret = mktime(&plt);
    //        return ret;
    //    }
#endif

#define ADAY 1
#define AMONTH 2
#define AYEAR 4

    static time_t parseDateString(const std::string& dateString)
    {
        static const char zones[] =
            "pst\0" // -8
            "pdt\0"
            "mst\0" // -7
            "mdt\0"
            "cst\0" // -6
            "cdt\0"
            "est\0" // -5
            "edt\0"
            "ast\0" // -4
            "nst\0" // -3
            "gmt\0" // 0
            "utc\0"
            "bst\0"
            "met\0" // 1
            "eet\0" // 2
            "jst\0" // 9
            "\0";
        static int zoneOffsets[] = {-8, -8, -7, -7, -6, -6, -5, -5, -4, -3, 0, 0, 0, 1, 2, 9};

        static const char months[] =
            "jan\0"
            "feb\0"
            "mar\0"
            "apr\0"
            "may\0"
            "jun\0"
            "jul\0"
            "aug\0"
            "sep\0"
            "oct\0"
            "nov\0"
            "dec\0"
            "\0";

        struct tm tm;
        tm.tm_sec = 0;
        tm.tm_min = 0;
        tm.tm_hour = 0;
        tm.tm_mday = 0;
        tm.tm_mon = 0;
        tm.tm_year = 0;
        tm.tm_wday = 0;
        tm.tm_yday = 0;
        tm.tm_isdst = 0;

        time_t time = -1;
        // placeholders for values when we are not sure it is a year, month or day
        int unknown[3] = {-1, -1, -1};
        int month = -1;
        int day = -1;
        int year = -1;
        int zoneOffset = -1;

        // hour:minute:second.ms pm
        //QRegExp timeRx(QLatin1String("(\\d{1,2}):(\\d{1,2})(:(\\d{1,2})|)(\\.(\\d{1,3})|)((\\s{0,}(am|pm))|)"));

        int at = 0;
        while (static_cast<size_t>(at) < dateString.length())
        {
            bool isNum = isNumber(dateString[at]);

            // Month
            if (!isNum
               && checkStaticArray(month, dateString, at, months, sizeof(months) - 1))
            {
                ++month;
                at += 3;
                continue;
            }
            // Zone
            if (!isNum
               && zoneOffset == -1
               && checkStaticArray(zoneOffset, dateString, at, zones, sizeof(zones) - 1))
            {
                int sign = (at >= 0 && dateString[at - 1] == '-') ? -1 : 1;
                zoneOffset = sign * zoneOffsets[zoneOffset] * 60 * 60;
                at += 3;
                continue;
            }
            // Zone offset
            if (!isNum
               && (zoneOffset == -1 || zoneOffset == 0) // Can only go after gmt
               && (dateString[at] == '+' || dateString[at] == '-')
               && (at == 0
                   || isWhitespace(dateString[at - 1])
                   || dateString[at - 1] == ','
                   || (at >= 3
                       && (dateString[at - 3] == 'g')
                       && (dateString[at - 2] == 'm')
                       && (dateString[at - 1] == 't'))))
            {
                int end = 1;
                while (end < 5 && dateString.length() > static_cast<size_t>(at + end)
                      && dateString[at + end] >= '0' && dateString[at + end] <= '9')
                    ++end;
                int minutes = 0;
                int hours = 0;
                switch(end - 1)
                {
                    case 4:
                        minutes = atoi(dateString.substr(at + 3, 2).c_str());
                        // fall through
                    case 2:
                        hours = atoi(dateString.substr(at + 1, 2).c_str());
                        break;
                    case 1:
                        hours = atoi(dateString.substr(at + 1, 1).c_str());
                        break;
                    default:
                        at += end;
                        continue;
                }
                if (end != 1)
                {
                    int sign = dateString[at] == '-' ? -1 : 1;
                    zoneOffset = sign * ((minutes * 60) + (hours * 60 * 60));
                    at += end;
                    continue;
                }
            }

            // Time hh:mm:ss.ms pm
            if (isNum && time == -1)
            {
                std::vector<std::string> timeSplit;
                Utils::split(dateString, at, dateString.length(), ':', timeSplit);

                if (timeSplit.size() >= 2 && (timeSplit[0].size() == 1 || timeSplit[0].size() == 2))
                {
                    int h = 0;
                    int m = 0;
                    int s = 0;
                    //                int ms = 0;

                    if (timeSplit.size() >= 1)
                    {
                        h = atoi(timeSplit[0].c_str());
                        at += static_cast<int>(timeSplit[0].length());
                    }
                    if (timeSplit.size() >= 2)
                    {
                        m = atoi(timeSplit[1].c_str());
                        at += 1; // for ':'
                        at += static_cast<int>(timeSplit[1].length());
                    }
                    if (timeSplit.size() >= 3)
                    {
                        at += 1; // for ':'
                        std::vector<std::string> secSplit;
                        Utils::split(timeSplit[2], 0, timeSplit[2].length(), '.', secSplit);
                        if (secSplit.size() >= 1)
                        {
                            s = atoi(secSplit[0].c_str());
                            at += static_cast<int>(secSplit[0].length());
                        }
                        if (secSplit.size() >= 2)
                        {
                            at += 1; // for '.'
                            std::vector<std::string> msSplit;
                            Utils::split(secSplit[1], 0, secSplit[1].length(), ' ', msSplit);
                            if (msSplit.size() >= 1)
                            {
                                //                            ms = atoi(msSplit[0].c_str());
                                at += static_cast<int>(msSplit[0].length());
                            }
                            if (msSplit.size() >= 2)
                            {
                                at += 1; // for ' '
                                if (h < 12 && msSplit[1] == "pm")
                                {
                                    h += 12;
                                }
                                at += static_cast<int>(msSplit[1].length());
                            }
                        }
                    }
                    tm.tm_hour = h;
                    tm.tm_min = m;
                    tm.tm_sec = s;
                    continue;
                }
            }

            // 4 digit Year
            if (isNum
               && year == -1
               && dateString.length() > static_cast<size_t>(at + 3))
            {
                if (isNumber(dateString[at + 1])
                   && isNumber(dateString[at + 2])
                   && isNumber(dateString[at + 3]))
                {
                    year = atoi(dateString.substr(at, 4).c_str());
                    at += 4;
                    continue;
                }
            }

            // a one or two digit number
            // Could be month, day or year
            if (isNum)
            {
                int length = 1;
                if (dateString.length() > static_cast<size_t>(at + 1)
                   && isNumber(dateString[at + 1]))
                    ++length;
                int x = atoi(dateString.substr(at, length).c_str());
                if (year == -1 && (x > 31 || x == 0))
                {
                    year = x;
                }
                else
                {
                    if (unknown[0] == -1)
                        unknown[0] = x;
                    else if (unknown[1] == -1)
                        unknown[1] = x;
                    else if (unknown[2] == -1)
                        unknown[2] = x;
                }
                at += length;
                continue;
            }

            // Unknown character, typically a weekday such as 'Mon'
            ++at;
        }

        // Once we are done parsing the string take the digits in unknown
        // and determine which is the unknown year/month/day

        int couldBe[3] = {0, 0, 0};
        int unknownCount = 3;
        for (int i = 0; i < unknownCount; ++i)
        {
            if (unknown[i] == -1)
            {
                couldBe[i] = ADAY | AYEAR | AMONTH;
                unknownCount = i;
                continue;
            }

            if (unknown[i] >= 1)
                couldBe[i] = ADAY;

            if (month == -1 && unknown[i] >= 1 && unknown[i] <= 12)
                couldBe[i] |= AMONTH;

            if (year == -1)
                couldBe[i] |= AYEAR;
        }

        // For any possible day make sure one of the values that could be a month
        // can contain that day.
        // For any possible month make sure one of the values that can be a
        // day that month can have.
        // Example: 31 11 06
        // 31 can't be a day because 11 and 6 don't have 31 days
        for (int i = 0; i < unknownCount; ++i)
        {
            int currentValue = unknown[i];
            bool findMatchingMonth = couldBe[i] & ADAY && currentValue >= 29;
            bool findMatchingDay = couldBe[i] & AMONTH;
            if (!findMatchingMonth || !findMatchingDay)
                continue;
            for (int j = 0; j < 3; ++j)
            {
                if (j == i)
                    continue;
                for (int k = 0; k < 2; ++k)
                {
                    if (k == 0 && !(findMatchingMonth && (couldBe[j] & AMONTH)))
                        continue;
                    else if (k == 1 && !(findMatchingDay && (couldBe[j] & ADAY)))
                        continue;
                    int m = currentValue;
                    int d = unknown[j];
                    if (k == 0)
                        std::swap(m, d);
                    if (m == -1)
                        m = month;
                    bool found = true;
                    switch(m)
                    {
                        case 2:
                            // When we get 29 and the year ends up having only 28
                            // See date.isValid below
                            // Example: 29 23 Feb
                            if (d <= 29)
                                found = false;
                            break;
                        case 4:
                        case 6:
                        case 9:
                        case 11:
                            if (d <= 30)
                                found = false;
                            break;
                        default:
                            if (d > 0 && d <= 31)
                                found = false;
                    }
                    if (k == 0)
                        findMatchingMonth = found;
                    else if (k == 1)
                        findMatchingDay = found;
                }
            }
            if (findMatchingMonth)
                couldBe[i] &= ~ADAY;
            if (findMatchingDay)
                couldBe[i] &= ~AMONTH;
        }

        // First set the year/month/day that have been deduced
        // and reduce the set as we go along to deduce more
        for (int i = 0; i < unknownCount; ++i)
        {
            int unset = 0;
            for (int j = 0; j < 3; ++j)
            {
                if (couldBe[j] == ADAY && day == -1)
                {
                    day = unknown[j];
                    unset |= ADAY;
                }
                else if (couldBe[j] == AMONTH && month == -1)
                {
                    month = unknown[j];
                    unset |= AMONTH;
                }
                else if (couldBe[j] == AYEAR && year == -1)
                {
                    year = unknown[j];
                    unset |= AYEAR;
                }
                else
                {
                    // common case
                    break;
                }
                couldBe[j] &= ~unset;
            }
        }

        // Now fallback to a standardized order to fill in the rest with
        for (int i = 0; i < unknownCount; ++i)
        {
            if (couldBe[i] & AMONTH && month == -1)
                month = unknown[i];
            else if (couldBe[i] & ADAY && day == -1)
                day = unknown[i];
            else if (couldBe[i] & AYEAR && year == -1)
                year = unknown[i];
        }

        if (year == -1 || month == -1 || day == -1)
        {
            return -1;
        }

        // Y2k behavior
        int y2k = 0;
        if (year < 70)
            y2k = 2000;
        else if (year < 100)
            y2k = 1900;

        tm.tm_year = year + y2k - 1900;
        tm.tm_mon = month;
        tm.tm_mday = day;

#ifdef WIN32
        time = _mkgmtime(&tm);
#else
        time = my_mkgmtime(&tm);
#endif

        if (zoneOffset != -1)
        {
            time += zoneOffset;
        }
        return time;
    }

    static std::pair<std::string, std::string> nextField(const std::string& text, int& position, bool isNameValue)
    {
        // format is one of:
        //    (1)  token
        //    (2)  token = token
        //    (3)  token = quoted-string
        const int length = static_cast<int>(text.size());
        position = nextNonWhitespace(text, position);

        int semiColonPosition = static_cast<int>(text.find_first_of(';', position));
        if (semiColonPosition < 0)
            semiColonPosition = length; //no ';' means take everything to end of string

        int equalsPosition = static_cast<int>(text.find_first_of('=', position));
        if (equalsPosition < 0 || equalsPosition > semiColonPosition)
        {
            if (isNameValue)
                return {};                      //'=' is required for name-value-pair (RFC6265 section 5.2, rule 2)
            equalsPosition = semiColonPosition; //no '=' means there is an attribute-name but no attribute-value
        }

        // remove last spaces
        int endFirstWithoutSpace = nextNonWhitespaceReverse(text, equalsPosition, position);

        std::string first = text.substr(position, endFirstWithoutSpace - position);
        std::string second;
        if (semiColonPosition > equalsPosition + 1)
        {
            // remove last spaces
            int endSecondWithoutSpace = nextNonWhitespaceReverse(text, semiColonPosition, equalsPosition + 1);

            second = text.substr(equalsPosition + 1, endSecondWithoutSpace - (equalsPosition + 1));
        }

        position = semiColonPosition;
        return std::make_pair(first, second);
    }

    static inline bool isTerminator(char c)
    {
        return c == '\n' || c == '\r';
    }

    static inline bool isValueSeparator(char c)
    {
        return isTerminator(c) || c == ';';
    }

    static inline bool isLWS(char c)
    {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    static int nextNonWhitespace(const std::string& text, int from)
    {
        // RFC 2616 defines linear whitespace as:
        //  LWS = [CRLF] 1*( SP | HT )
        // We ignore the fact that CRLF must come as a pair at this point
        // It's an invalid HTTP header if that happens.
        while (static_cast<size_t>(from) < text.length())
        {
            if (isLWS(text.at(from)))
                ++from;
            else
                return from; // non-whitespace
        }

        // reached the end
        return static_cast<int>(text.length());
    }

    static int nextNonWhitespaceReverse(const std::string& text, int from, int till)
    {
        --from;
        while (from >= till)
        {
            if (isLWS(text.at(from)))
                --from;
            else
                return from + 1; // non-whitespace + 1
        }

        // reached the end
        return till;
    }

    time_t m_expirationDate{};
    std::string m_domain{};
    bool m_domainDot{};
    std::string m_path{};
    std::string m_comment{};
    std::string m_name{};
    std::string m_value{};
    bool m_secure{};
    bool m_httpOnly{};
};
#ifndef WIN32
const int Cookie::DaysOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
#endif

//---------------------------------------
// CookieStore
//---------------------------------------

void CookieStore::add(const Cookie& cookie)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::shared_ptr<Cookie>& cookieExists = getCookieInternExactPath(cookie.getDomain(), cookie.getPath());
    if (cookieExists)
    {
        cookieExists = std::make_shared<Cookie>(cookie);
    }
    else
    {
        m_cookies.push_back(std::make_shared<Cookie>(cookie));
    }
}

void CookieStore::add(const std::vector<Cookie>& cookies)
{
    for (const auto& cookie : cookies)
    {
        add(cookie);
    }
}

const std::vector<std::shared_ptr<Cookie>> CookieStore::getCookies(const std::string& host, const std::string& path) const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    std::vector<std::shared_ptr<Cookie>> cookies = const_cast<CookieStore*>(this)->getCookiesIntern(host, path);
    return cookies;
}

std::vector<std::shared_ptr<Cookie>> CookieStore::getCookiesIntern(const std::string& host, const std::string& path)
{
    std::vector<std::shared_ptr<Cookie>> cookies;
    for (auto it = m_cookies.begin(); it != m_cookies.end();)
    {
        std::shared_ptr<Cookie>& cookie = *it;
        if (cookie->match(host, path))
        {
            if (cookie->isExpired())
            {
                it = m_cookies.erase(it);
            }
            else
            {
                cookies.push_back(cookie);
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }
    return cookies;
}

std::shared_ptr<Cookie>& CookieStore::getCookieInternExactPath(const std::string& host, const std::string& path)
{
    for (auto it = m_cookies.begin(); it != m_cookies.end();)
    {
        std::shared_ptr<Cookie>& cookie = *it;
        if (cookie->matchExactPath(host, path))
        {
            if (cookie->isExpired())
            {
                it = m_cookies.erase(it);
            }
            else
            {
                return cookie;
            }
        }
        else
        {
            ++it;
        }
    }
    static std::shared_ptr<Cookie> nullPointer;
    return nullPointer;
}

//---------------------------------------
// ProtocolHttpClient
//---------------------------------------

const std::uint32_t ProtocolHttpClient::PROTOCOL_ID = 7;
const std::string ProtocolHttpClient::PROTOCOL_NAME = "httpclient";

const std::string ProtocolHttpClient::FMQ_HTTP = "fmq_http";
const std::string ProtocolHttpClient::FMQ_METHOD = "fmq_method";
const std::string ProtocolHttpClient::FMQ_PROTOCOL = "fmq_protocol";
const std::string ProtocolHttpClient::FMQ_PATH = "fmq_path";
const std::string ProtocolHttpClient::FMQ_QUERY_PREFIX = "QUERY_";
const std::string ProtocolHttpClient::FMQ_HTTP_STATUS = "fmq_http_status";
const std::string ProtocolHttpClient::FMQ_HTTP_STATUSTEXT = "fmq_http_statustext";
const std::string ProtocolHttpClient::HTTP_REQUEST = "request";
const std::string ProtocolHttpClient::HTTP_RESPONSE = "response";

static const std::string CONTENT_LENGTH = "Content-Length";
//static const std::string FMQ_SESSIONID = "fmq_sessionid";
static const std::string HTTP_COOKIE = "Cookie";

static const std::string FMQ_CREATESESSION = "fmq_createsession";
static const std::string FMQ_SET_SESSION = "fmq_setsession";
static const std::string HTTP_SET_COOKIE = "Set-Cookie";
static const std::string COOKIE_PREFIX = "fmq=";

static const std::string FMQ_PATH_POLL = "/fmq/poll";
static const std::string FMQ_PATH_PING = "/fmq/ping";
static const std::string FMQ_PATH_CONFIG = "/fmq/config";
static const std::string FMQ_PATH_CREATESESSION = "/fmq/createsession";
static const std::string FMQ_PATH_REMOVESESSION = "/fmq/removesession";
static const std::string FMQ_MULTIPART_BOUNDARY = "B9BMAhxAhY.mQw1IDRBA";

//enum ChunkedState
//{
//    STATE_STOP = 0,
//    STATE_START_CHUNK_STREAM = 1,
//    STATE_FIRST_CHUNK = 2,
//    STATE_BEGIN_MULTIPART = 3,
//    STATE_CONTINUE = 4,
//};

ProtocolHttpClient::ProtocolHttpClient()
    : m_randomDevice(), m_randomGenerator(m_randomDevice())
{
}

ProtocolHttpClient::~ProtocolHttpClient()
{
    if (m_connection)
    {
        m_connection->disconnect();
    }
}

// IProtocol
void ProtocolHttpClient::setCallback(const std::weak_ptr<IProtocolCallback>& callback)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_callback = callback;
    std::shared_ptr<IProtocolCallback> cb = callback.lock();
    lock.unlock();
    if (cb)
    {
        // 5 minutes session timeout
        cb->setActivityTimeout(5 * 60000);
    }
}

void ProtocolHttpClient::setConnection(const IStreamConnectionPtr& connection)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_connection = connection;
}

IStreamConnectionPtr ProtocolHttpClient::getConnection() const
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_connection;
}

void ProtocolHttpClient::disconnect()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    IStreamConnectionPtr conn = m_connection;
    lock.unlock();
    if (conn)
    {
        conn->disconnect();
    }
}

std::uint32_t ProtocolHttpClient::getProtocolId() const
{
    return PROTOCOL_ID;
}

bool ProtocolHttpClient::areMessagesResendable() const
{
    return false;
}

bool ProtocolHttpClient::doesSupportMetainfo() const
{
    return true;
}

bool ProtocolHttpClient::doesSupportSession() const
{
    return true;
}

bool ProtocolHttpClient::needsReply() const
{
    return true;
}

bool ProtocolHttpClient::isMultiConnectionSession() const
{
    return true;
}

bool ProtocolHttpClient::isSendRequestByPoll() const
{
    return true;
}

bool ProtocolHttpClient::doesSupportFileTransfer() const
{
    return true;
}

bool ProtocolHttpClient::isSynchronousRequestReply() const
{
    return true;
}

IProtocol::FuncCreateMessage ProtocolHttpClient::getMessageFactory() const
{
    return []() {
        return std::make_shared<ProtocolMessage>(PROTOCOL_ID);
    };
}

static void splitOnce(const std::string& src, ssize_t indexBegin, ssize_t indexEnd, char delimiter, std::vector<std::string>& dest)
{
    size_t pos = src.find_first_of(delimiter, indexBegin);
    if (pos == std::string::npos || static_cast<ssize_t>(pos) > indexEnd)
    {
        pos = indexEnd;
    }
    ssize_t len = pos - indexBegin;
    assert(len >= 0);
    dest.emplace_back(&src[indexBegin], len);
    ++pos;

    if (static_cast<ssize_t>(pos) < indexEnd)
    {
        len = indexEnd - pos;
        assert(len >= 0);
        dest.emplace_back(&src[pos], len);
    }
}

bool ProtocolHttpClient::receiveHeaders(ssize_t bytesReceived)
{
    bool ok = true;
    bytesReceived += m_sizeRemaining;
    assert(bytesReceived <= static_cast<ssize_t>(m_receiveBuffer.size()));
    while (m_offsetRemaining < bytesReceived && ok)
    {
        size_t index = m_receiveBuffer.find_first_of('\n', m_offsetRemaining);
        if (index != std::string::npos)
        {
            ssize_t indexEndLine = index;
            --indexEndLine; // goto '\r'
            ssize_t len = indexEndLine - m_offsetRemaining;
            if (len < 0 || m_receiveBuffer[indexEndLine] != '\r')
            {
                ok = false;
            }
            if (ok)
            {
                if (m_state == State::STATE_FIND_FIRST_LINE)
                {
                    m_contentLength = 0;
                    if (len < 4)
                    {
                        ok = false;
                    }
                    else
                    {
                        // is response
                        if (m_receiveBuffer[m_offsetRemaining] == 'H' && m_receiveBuffer[m_offsetRemaining + 1] == 'T')
                        {
                            std::vector<std::string> lineSplit;
                            Utils::split(m_receiveBuffer, m_offsetRemaining, indexEndLine, ' ', lineSplit);
                            if (lineSplit.size() >= 2)
                            {
                                m_message = std::make_shared<ProtocolMessage>(0);
                                Variant& controlData = m_message->getControlData();
                                controlData.add(FMQ_HTTP, std::string(HTTP_RESPONSE));
                                controlData.add(FMQ_PROTOCOL, std::move(lineSplit[0]));
                                controlData.add(FMQ_HTTP_STATUS, std::move(lineSplit[1]));
                                std::string statusText;
                                for (size_t i = 2; i < lineSplit.size(); ++i)
                                {
                                    statusText += lineSplit[i];
                                    if (i < lineSplit.size() - 1)
                                    {
                                        statusText += ' ';
                                    }
                                }
                                controlData.add(FMQ_HTTP_STATUSTEXT, std::move(statusText));
                                m_state = State::STATE_FIND_HEADERS;
                            }
                            else
                            {
                                ok = false;
                            }
                        }
                        else
                        {
                            ok = false;
                        }
                    }
                }
                else if (m_state == State::STATE_FIND_HEADERS)
                {
                    if (len == 0)
                    {
                        if (m_contentLength == 0)
                        {
                            m_state = State::STATE_CONTENT_DONE;
                        }
                        else
                        {
                            m_state = State::STATE_CONTENT;
                            m_message->resizeReceiveBuffer(m_contentLength);
                        }
                        m_indexFilled = 0;
                        m_offsetRemaining += 2;
                        break;
                    }
                    else
                    {
                        std::vector<std::string> lineSplit;
                        splitOnce(m_receiveBuffer, m_offsetRemaining, indexEndLine, ':', lineSplit);
                        if (lineSplit.size() == 2)
                        {
                            std::string& value = lineSplit[1];
                            if (!value.empty() && value[0] == ' ')
                            {
                                value.erase(value.begin());
                            }
                            if (lineSplit[0] == CONTENT_LENGTH)
                            {
                                m_contentLength = std::atoll(value.c_str());
                            }
                            else if (lineSplit[0] == HTTP_SET_COOKIE)
                            {
                                const std::vector<Cookie> cookies = Cookie::parseSetCookieHeaderLine(value, m_hostname);
                                m_cookieStore->add(cookies);
                            }
                            m_message->addMetainfo(std::move(lineSplit[0]), std::move(value));
                        }
                        else if (lineSplit.size() == 1)
                        {
                            m_message->addMetainfo(std::move(lineSplit[0]), "");
                        }
                        else
                        {
                            assert(false);
                            ok = false;
                        }
                    }
                }
                m_offsetRemaining += len + 2;
            }
        }
        else
        {
            break;
        }
    }
    m_sizeRemaining = bytesReceived - m_offsetRemaining;
    assert(m_sizeRemaining >= 0);
    return ok;
}

void ProtocolHttpClient::reset()
{
    m_offsetRemaining = 0;
    m_sizeRemaining = 0;
    m_contentLength = 0;
    m_indexFilled = 0;
    m_message = nullptr;
    m_state = State::STATE_FIND_FIRST_LINE;
}

static std::string HEADER_KEEP_ALIVE = "Connection: keep-alive\r\n";

void ProtocolHttpClient::sendMessage(IMessagePtr message)
{
    IStreamConnectionPtr connection = getConnection();
    if (connection == nullptr)
    {
        return;
    }
    if (message == nullptr)
    {
        return;
    }
    assert(!message->wasSent());
    std::string firstLine;
    const Variant& controlData = message->getControlData();
    const std::string* filename = controlData.getData<std::string>("filetransfer");
    ssize_t filesize = -1;
    if (filename)
    {
        struct stat statdata;
        memset(&statdata, 0, sizeof(statdata));
        int res = OperatingSystem::instance().stat(filename->c_str(), &statdata);
        if (res == 0)
        {
            filesize = statdata.st_size;
            message->downsizeLastSendPayload(0);
        }
    }

    if (filename && filesize == -1)
    {
        filesize = 0;
    }

    ssize_t sizeBody = message->getTotalSendPayloadSize();
    if (filesize != -1)
    {
        sizeBody = filesize;
    }

    static const std::string METHOD_GET = "GET";
    static const std::string METHOD_POST = "POST";
    static const std::string PATH_ROOT = "/";
    const std::string* method = controlData.getData<std::string>(FMQ_METHOD);
    const std::string* path = controlData.getData<std::string>(FMQ_PATH);
    if (method == nullptr || method->empty())
    {
        if (sizeBody > 0)
        {
            method = &METHOD_POST;
        }
        else
        {
            method = &METHOD_GET;
        }
    }
    if (path == nullptr || path->empty())
    {
        path = &PATH_ROOT;
    }

    std::string pathEncode;
    encode(pathEncode, *path);
    firstLine = *method;
    firstLine += ' ';
    firstLine += pathEncode;

    const VariantStruct* queries = controlData.getData<VariantStruct>("queries");
    if (queries)
    {
        for (auto it = queries->begin(); it != queries->end(); ++it)
        {
            if (it == queries->begin())
            {
                firstLine += '?';
            }
            else
            {
                firstLine += '&';
            }
            std::string key;
            std::string value;

            encode(key, it->first);
            encode(value, it->second);
            firstLine += key;
            firstLine += '=';
            firstLine += value;
        }
    }

    firstLine += " HTTP/1.1";

    if (m_headerHost.empty())
    {
        const ConnectionData& connectionData = connection->getConnectionData();
        m_hostname = connectionData.hostname;
        m_headerHost = "Host: " + connectionData.hostname + ":" + std::to_string(connectionData.port) + "\r\n";
    }

    size_t sumHeaderSize = 0;
    sumHeaderSize += firstLine.size() + 2 + 2; // 2 = '\r\n' and 2 = last empty line
    sumHeaderSize += HEADER_KEEP_ALIVE.size(); // Connection: keep-alive\r\n

    sumHeaderSize += m_headerHost.size(); // Host: hostname\r\n

    ProtocolMessage::Metainfo& metainfo = message->getAllMetainfo();

    assert(m_cookieStore);
    const std::vector<std::shared_ptr<Cookie>> cookies = m_cookieStore->getCookies(m_hostname, *path);
    std::string cookieString;
    for (const auto& cookie : cookies)
    {
        cookieString += cookie->getName() + '=' + cookie->getValue() + "; ";
    }
    if (!cookieString.empty())
    {
        metainfo[HTTP_COOKIE] = cookieString;
    }

    metainfo[CONTENT_LENGTH] = std::to_string(sizeBody);
    if (!m_headerSendNext.empty())
    {
        metainfo.insert(m_headerSendNext.begin(), m_headerSendNext.end());
        m_headerSendNext.clear();
    }
    for (auto it = metainfo.begin(); it != metainfo.end(); ++it)
    {
        const std::string& key = it->first;
        const std::string& value = it->second;
        if (!key.empty())
        {
            sumHeaderSize += key.size() + value.size() + 4; // 4 = ': ' and '\r\n'
        }
    }

    char* headerBuffer = nullptr;
    size_t index = 0;
    headerBuffer = message->addSendHeader(sumHeaderSize);
    index = 0;
    assert(index + firstLine.size() + 2 <= sumHeaderSize);
    memcpy(headerBuffer + index, firstLine.data(), firstLine.size());
    index += firstLine.size();
    memcpy(headerBuffer + index, "\r\n", 2);
    index += 2;

    // Host: hostname\r\n
    assert(index + m_headerHost.size() <= sumHeaderSize);
    memcpy(headerBuffer + index, m_headerHost.data(), m_headerHost.size());
    index += m_headerHost.size();

    // Connection: keep-alive\r\n
    assert(index + HEADER_KEEP_ALIVE.size() <= sumHeaderSize);
    memcpy(headerBuffer + index, HEADER_KEEP_ALIVE.data(), HEADER_KEEP_ALIVE.size());
    index += HEADER_KEEP_ALIVE.size();

    for (auto it = metainfo.begin(); it != metainfo.end(); ++it)
    {
        const std::string& key = it->first;
        const std::string& value = it->second;
        if (!key.empty())
        {
            assert(index + key.size() + value.size() + 4 <= sumHeaderSize);
            memcpy(headerBuffer + index, key.data(), key.size());
            index += key.size();
            memcpy(headerBuffer + index, ": ", 2);
            index += 2;
            memcpy(headerBuffer + index, value.data(), value.size());
            index += value.size();
            memcpy(headerBuffer + index, "\r\n", 2);
            index += 2;
        }
    }
    assert(index + 2 == sumHeaderSize);
    memcpy(headerBuffer + index, "\r\n", 2);

    message->prepareMessageToSend();

    connection->sendMessage(message);

    if (filename && filesize > 0)
    {
        std::string file = *filename;
        std::weak_ptr<ProtocolHttpClient> pThisWeak = shared_from_this();
        GlobalExecutorWorker::instance().addAction([pThisWeak, file, filesize]() {
            std::shared_ptr<ProtocolHttpClient> pThis = pThisWeak.lock();
            if (!pThis)
            {
                return;
            }
            int flags = O_RDONLY;
#ifdef WIN32
            flags |= O_BINARY;
#endif
            int fd = OperatingSystem::instance().open(file.c_str(), flags);
            if (fd != -1)
            {
                int len = static_cast<int>(filesize);
                int err = 0;
                int lenReceived = 0;
                bool ex = false;
                while (!ex)
                {
                    int size = std::min(1024, len);
                    IMessagePtr messageData = std::make_shared<ProtocolMessage>(0);
                    char* buf = messageData->addSendPayload(size);
                    do
                    {
                        err = OperatingSystem::instance().read(fd, buf, size);
                    } while (err == -1 && OperatingSystem::instance().getLastError() == SOCKETERROR(EINTR));

                    if (err > 0)
                    {
                        assert(err <= size);
                        if (err < size)
                        {
                            messageData->downsizeLastSendPayload(err);
                        }
                        pThis->m_connection->sendMessage(messageData);
                        buf += err;
                        len -= err;
                        lenReceived += err;
                        err = 0;
                        assert(len >= 0);
                        if (len == 0)
                        {
                            ex = true;
                        }
                    }
                    else
                    {
                        ex = true;
                    }
                }
                if (lenReceived < filesize)
                {
                    pThis->m_connection->disconnect();
                }
            }
            else
            {
                pThis->m_connection->disconnect();
            }
        });
    }
}

void ProtocolHttpClient::moveOldProtocolState(IProtocol& /*protocolOld*/)
{
}

//bool ProtocolHttpClient::handleInternalCommands(const std::shared_ptr<IProtocolCallback>& callback, bool& ok)
//{
//    assert(callback);
//    bool handled = false;
//    if (m_path)
//    {
//        if (*m_path == FMQ_PATH_POLL)
//        {
//            m_chunkedState = STATE_START_CHUNK_STREAM;
//            assert(m_message);
//            handled = true;
//            std::int32_t timeout = -1;
//            std::int32_t pollCountMax = 1;
//            const std::string* strTimeout = m_message->getMetainfo("QUERY_timeout");
//            const std::string* strCount = m_message->getMetainfo("QUERY_count");
//            const std::string* strMultipart = m_message->getMetainfo("QUERY_multipart");
//            if (strTimeout)
//            {
//                timeout = std::atoi(strTimeout->c_str());
//            }
//            if (strCount)
//            {
//                pollCountMax = std::atoi(strCount->c_str());
//            }
//            if (strMultipart)
//            {
//                m_multipart = (*strMultipart == "true") ? true : false;
//            }
//            //if (strTimeout && !strCount)
//            //{
//            //    pollCountMax = -1;
//            //}
//            //if (!strTimeout && strCount)
//            //{
//            //    timeout = -1;
//            //}
//            IMessagePtr message = getMessageFactory()();
//            std::string contentType;
//            if (m_multipart)
//            {
//                contentType = "multipart/x-mixed-replace; boundary=";
//                contentType += FMQ_MULTIPART_BOUNDARY;
//            }
//            else
//            {
//                contentType = "text/event-stream";
//            }
//            message->addMetainfo("Content-Type", contentType);
//            message->addMetainfo("Transfer-Encoding", "chunked");
//            sendMessage(message);
//            m_chunkedState = STATE_FIRST_CHUNK;
//            callback->pollRequest(shared_from_this(), timeout, pollCountMax);
//        }
//        else if (*m_path == FMQ_PATH_PING)
//        {
//            handled = true;
//            assert(m_connection);
//            sendMessage(getMessageFactory()());
//            callback->activity();
//        }
//        else if (*m_path == FMQ_PATH_CONFIG)
//        {
//            handled = true;
//            const std::string* timeout = m_message->getMetainfo("QUERY_activitytimeout");
//            if (timeout)
//            {
//                callback->setActivityTimeout(std::atoi(timeout->c_str()));
//            }
//            const std::string* pollMaxRequests = m_message->getMetainfo("QUERY_pollmaxrequests");
//            if (pollMaxRequests)
//            {
//                callback->setPollMaxRequests(std::atoi(pollMaxRequests->c_str()));
//            }
//            sendMessage(getMessageFactory()());
//            callback->activity();
//        }
//        else if (*m_path == FMQ_PATH_CREATESESSION)
//        {
//            handled = true;
//            sendMessage(getMessageFactory()());
//            callback->activity();
//        }
//        else if (*m_path == FMQ_PATH_REMOVESESSION)
//        {
//            handled = true;
//            ok = false;
//            sendMessage(getMessageFactory()());
//            callback->disconnected();
//        }
//    }
//
//    return handled;
//}

bool ProtocolHttpClient::received(const IStreamConnectionPtr& /*connection*/, const SocketPtr& socket, int bytesToRead)
{
    bool ok = true;

    if (m_state != State::STATE_CONTENT)
    {
        if (m_offsetRemaining == 0 || m_sizeRemaining == 0)
        {
            m_receiveBuffer.resize(m_sizeRemaining + bytesToRead);
        }
        else
        {
            std::string temp = std::move(m_receiveBuffer);
            m_receiveBuffer.clear();
            m_receiveBuffer.resize(m_sizeRemaining + bytesToRead);
            memcpy(&m_receiveBuffer[0], &temp[m_offsetRemaining], m_sizeRemaining);
        }
        m_offsetRemaining = 0;

        ssize_t bytesReceived = 0;
        int res = 0;
        do
        {
            res = socket->receive(const_cast<char*>(m_receiveBuffer.data() + bytesReceived + m_sizeRemaining), static_cast<int>(bytesToRead - bytesReceived));
            if (res > 0)
            {
                bytesReceived += res;
            }
        } while (res > 0 && bytesReceived < bytesToRead);
        if (res >= 0)
        {
            assert(bytesReceived <= bytesToRead);
            ok = receiveHeaders(bytesReceived);
            if (ok && m_state == State::STATE_CONTENT)
            {
                assert(m_message != nullptr);
                BufferRef payload = m_message->getReceivePayload();
                assert(payload.second == m_contentLength);
                if (m_sizeRemaining <= m_contentLength)
                {
                    memcpy(payload.first, m_receiveBuffer.data() + m_offsetRemaining, m_sizeRemaining);
                    m_indexFilled = m_sizeRemaining;
                    assert(m_indexFilled <= m_contentLength);
                    if (m_indexFilled == m_contentLength)
                    {
                        m_state = State::STATE_CONTENT_DONE;
                    }
                }
                else
                {
                    // too much content
                    ok = false;
                }
            }
        }
    }
    else
    {
        BufferRef payload = m_message->getReceivePayload();
        assert(payload.second == m_contentLength);
        ssize_t remainingContent = m_contentLength - m_indexFilled;
        if (bytesToRead <= remainingContent)
        {
            ssize_t bytesReceived = 0;
            int res = 0;
            do
            {
                res = socket->receive(payload.first + bytesReceived + m_indexFilled, static_cast<int>(bytesToRead - bytesReceived));
                if (res > 0)
                {
                    bytesReceived += res;
                }
            } while (res > 0 && bytesReceived < bytesToRead);
            if (res >= 0)
            {
                m_indexFilled += bytesReceived;
                assert(m_indexFilled <= m_contentLength);
                if (m_indexFilled == m_contentLength)
                {
                    m_state = State::STATE_CONTENT_DONE;
                }
            }
        }
        else
        {
            // too much content
            ok = false;
        }
    }

    if (ok)
    {
        if (m_state == State::STATE_CONTENT_DONE)
        {
            auto callback = m_callback.lock();
            if (callback)
            {
                //bool handled = handleInternalCommands(callback, ok);
                //if (!handled)
                {
                    callback->received(m_message, m_connectionId);
                }
            }
            reset();
        }
    }
    return ok;
}

hybrid_ptr<IStreamConnectionCallback> ProtocolHttpClient::connected(const IStreamConnectionPtr& connection)
{
    ConnectionData connectionData = connection->getConnectionData();
    if (m_headerHost.empty())
    {
        m_headerHost = "Host: " + connectionData.hostname + ":" + std::to_string(connectionData.port) + "\r\n";
    }
    m_connectionId = connectionData.connectionId;

    auto callback = m_callback.lock();
    if (callback)
    {
        callback->connected();
    }
    return nullptr;
}

const std::string FMQ_HTTP_STATUS = "fmq_http_status";
const std::string FMQ_HTTP_STATUSTEXT = "fmq_http_statustext";
const std::string FMQ_DISCONNECTED = "fmq_disconnected";

void ProtocolHttpClient::disconnected(const IStreamConnectionPtr& connection)
{
    auto callback = m_callback.lock();
    if (callback)
    {
        IMessagePtr message = std::make_shared<ProtocolMessage>(0);
        IMessage::Metainfo& metainfo = message->getAllMetainfo();
        metainfo[FMQ_HTTP] = HTTP_RESPONSE;
        metainfo[FMQ_PROTOCOL] = "HTTP/1.1";
        metainfo[FMQ_HTTP_STATUS] = "404";
        metainfo[FMQ_HTTP_STATUSTEXT] = "Not Found";
        metainfo[FMQ_DISCONNECTED] = true;
        callback->received(message, connection->getConnectionId());

        callback->disconnectedMultiConnection(shared_from_this());
    }
}

IMessagePtr ProtocolHttpClient::pollReply(std::deque<IMessagePtr>&& /*messages*/)
{
    return nullptr;
}

void ProtocolHttpClient::subscribe(const std::vector<std::string>& /*subscribtions*/)
{
}

void ProtocolHttpClient::cycleTime()
{
}

IProtocolSessionDataPtr ProtocolHttpClient::createProtocolSessionData()
{
    return std::make_shared<CookieStore>();
}

void ProtocolHttpClient::setProtocolSessionData(const IProtocolSessionDataPtr& protocolSessionData)
{
    m_cookieStore = std::static_pointer_cast<CookieStore>(protocolSessionData);
}

//---------------------------------------
// ProtocolHttpFactory
//---------------------------------------

struct RegisterProtocolHttpClientFactory
{
    RegisterProtocolHttpClientFactory()
    {
        ProtocolRegistry::instance().registerProtocolFactory(ProtocolHttpClient::PROTOCOL_NAME, ProtocolHttpClient::PROTOCOL_ID, std::make_shared<ProtocolHttpClientFactory>());
    }
} g_registerProtocolHttpClientFactory;

// IProtocolFactory
IProtocolPtr ProtocolHttpClientFactory::createProtocol(const Variant& /*data*/)
{
    return std::make_shared<ProtocolHttpClient>();
}

} // namespace finalmq
