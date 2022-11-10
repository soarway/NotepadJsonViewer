#include "JsonHandler.h"
#include <locale>

namespace rj = rapidjson;


JsonHandler::JsonHandler(const ParseOptions &options)
    : m_parseOptions(options)
{
}

auto JsonHandler::GetCompressedJson(const std::string &jsonText) -> const Result
{
    rj::StringBuffer                                                                                sb;
    rj::Writer<rj::StringBuffer, rj::UTF8<>, rj::UTF8<>, rj::CrtAllocator, rj::kWriteNanAndInfFlag> handler(sb);

    return ParseJson<flgBaseWriter>(jsonText, sb, handler);
}

auto JsonHandler::FormatJson(const std::string &jsonText, LE le, LF lf, char indentChar, unsigned indentLen) -> const Result
{
    rj::StringBuffer                                                                                      sb;
    rj::PrettyWriter<rj::StringBuffer, rj::UTF8<>, rj::UTF8<>, rj::CrtAllocator, rj::kWriteNanAndInfFlag> handler(sb);
    handler.SetLineEnding(le);
    handler.SetFormatOptions(lf);
    handler.SetIndent(indentChar, indentLen);

    return ParseJson<flgBaseWriter>(jsonText, sb, handler);
}

auto JsonHandler::FormatCode(const std::string &codeText, LE le, LF lf, char indentChar, unsigned indentLen) -> const Result
{
    rj::StringBuffer                                                                                      sb;
    rj::PrettyWriter<rj::StringBuffer, rj::UTF8<>, rj::UTF8<>, rj::CrtAllocator, rj::kWriteNanAndInfFlag> handler(sb);
    handler.SetLineEnding(le);
    handler.SetFormatOptions(lf);
    handler.SetIndent(indentChar, indentLen);

    return ParseCode<flgBaseWriter>(codeText, sb, handler);
}

auto JsonHandler::ValidateJson(const std::string &jsonText) -> const Result
{
    rj::StringBuffer                                                                                sb;
    rj::Writer<rj::StringBuffer, rj::UTF8<>, rj::UTF8<>, rj::CrtAllocator, rj::kWriteNanAndInfFlag> handler(sb);

    return ParseJson<flgBaseWriter>(jsonText, sb, handler);
}




char *JsonHandler::getNextLine(char *buf, int buflen, int *datalen, char *ptr)
{
    char *pend = ptr;
    memset(buf, 0, buflen);

    if (*ptr == '\n')
    {
        buf[0]   = '\n';
        *datalen = 1;
        return buf;
    }
    else if (*ptr == '\0')
    {
        *datalen = 0;
        return NULL;
    }

    while (*pend != '\n' && *pend != '\0')
        pend++;

    assert(pend - ptr < buflen-1);

    if (*pend == '\0') {
        *datalen = pend - ptr ;    //+1代表把 \n也复制到buf里
    }
    else
    {
        *datalen = pend - ptr + 1;    //+1代表把 \n也复制到buf里
    }
    
    memcpy(buf, ptr, *datalen);


    return buf;
}

//----------------------------------------------------------------------------------------
//判断 } ,
static bool isDhGh(char* ps , char *pe) {
    bool b = false;
    while (pe > ps) {
        pe--;
        if (std::isblank(*pe, std::locale("en_US.UTF-8"))) {
            continue;
        }
        else if (*pe == '}')
        {
            b = true;
            break;
        }else
        {
            break;
        }
    }

    return b;
}

//判断 = {
static bool isGhDh(char* ps, char* pe) {
    bool bf = false;
    while (pe > ps)
    {
        pe--;
        if (std::isblank(*pe, std::locale("en_US.UTF-8")))
        {
            continue;
        }
        else if (*pe == '=')
        {
            bf = true;
            break;
        }
        else
        {
            break;
        }
    }

    return bf;
}



std::string JsonHandler::parse_code_text(char *codeText)
{
    std::string retstring;

    char *ptr;
    int   len = 0, datalen = 0, line = 0;
    char  buf[2048] = {0};

    char *pend = codeText;


    if (*pend == '\0')
        return retstring;

    bool bInMark = false;
    //bool bInGH   = false;

    while (getNextLine(buf, sizeof(buf), &datalen, pend) != NULL)
    {
        if (buf[0] == '\n')
        {
            retstring.append(buf);
            pend += 1;
            // pstart = pend;
            continue;
        }


        line++;
        ptr = buf;
        pend += datalen;

        //同一行用/**/完成整行的注释, 哪怕是 for (int a=0; /*AAA*/ a<100; a++) { 也不做处理，直接写入
        if (strstr(ptr, "/*") && strstr(ptr, "*/"))
        {
            // fwrite(buf, sizeof(char), strlen(buf), fpnew);
            retstring.append(buf);
            // retstring.append("\n");
            continue;
        }

        if (strstr(ptr, "/*"))
        {
            bInMark = true;    //开始进入注释模式
            // fwrite(buf, sizeof(char), strlen(buf), fpnew);
            retstring.append(buf);
            // retstring.append("\n");
            continue;
        }

        if (strstr(ptr, "*/"))
        {
            bInMark = false;    //开始退出注释模式
            // fwrite(buf, sizeof(char), strlen(buf), fpnew);
            retstring.append(buf);
            // retstring.append("\n");
            continue;
        }

        //在注释模式下，不改变行， 直接原样写入文件
        if (bInMark)
        {
            // fwrite(buf, sizeof(char), strlen(buf), fpnew);
            retstring.append(buf);
            // retstring.append("\n");
            continue;
        }

        while (std::isblank(*ptr, std::locale("en_US.UTF-8")))
            ptr++;
        len = strlen(ptr);
        while (len > 0 && (std::isblank(*(ptr + len - 1), std::locale("en_US.UTF-8")) || (*(ptr + len - 1) == '\r') || (*(ptr + len - 1) == '\n')))
            len--;
        if (len <= 1)
        {
            //空白行, 直接写入空白行
            // fwrite(buf, sizeof(char), strlen(buf), fpnew);
            retstring.append(buf);
            // retstring.append("\n");
            continue;
        }


        //,逗号结尾和 )小挂号结尾
        if (*(ptr + len - 1) == ',')    //,号结尾
        {
            if (isDhGh(ptr, ptr + len - 1)) 
            //if (*(ptr + len - 2) == '}')//排除  "}  ," 结尾的情况  这行代码要优化，万一中间有空格
            {    
                // fwrite(buf, sizeof(char), strlen(buf), fpnew);
                retstring.append(buf);
                // retstring.append("\n");
                continue;
            }
            else
            {
                
                char newLine[4096] = {0};
                char nextBuf[2048] = {0};

                *(ptr + len) = '\0';
                strcat(newLine, buf);
                strcat(newLine, " ");
                while (getNextLine(nextBuf, sizeof(nextBuf), &datalen, pend))
                {
                    pend += datalen;
                    ptr = nextBuf;
                    while (std::isblank(*ptr, std::locale("en_US.UTF-8")))
                        ptr++;
                    len = strlen(ptr);
                    while (len > 0 && (std::isblank(*(ptr + len - 1), std::locale("en_US.UTF-8")) || (*(ptr + len - 1) == '\r') || (*(ptr + len - 1) == '\n')))
                        len--;
                    if (*(ptr + len - 1) == ',')
                    {
                        //逗号结尾
                        *(ptr + len) = '\0';
                        strcat(newLine, ptr);
                        strcat(newLine, " ");
                        continue;
                    }
                    else
                    {
                        strcat(newLine, ptr);
                        //strcat(newLine, "\n");
                        break;
                    }
                }
                //写入文件
                // fwrite(newLine, sizeof(char), strlen(newLine), fpnew);
                retstring.append(newLine);
            }
        }
        else if (*(ptr + len - 1) == '{')//以 { 结尾
        {
            //char *pe = ptr + len - 1;
            //char *ps = ptr;
            //倒着查找=
            bool bf = isGhDh(ptr, ptr+len-1);
            //while  ( pe > ps ) {
            //    pe--;
            //    if (std::isblank(*pe, std::locale("en_US.UTF-8"))) {
            //        continue;
            //    }
            //    else if (*pe == '=')
            //    {
            //        bf = true;
            //        break;
            //    }
            //    else
            //    {
            //        break;
            //    }
            //}

            if (!bf) {//enum bq2589x_vbus_type {
                if ( strstr(ptr, "enum") ) {
                    bf = true;
                }
            }

            if (bf) {
                //找到 “= {” 这种格式，则需要找到 ; 中间的部分不做格式化处理
                //bInGH = true;
                retstring.append(buf);

                
                char nextBuf[2048] = {0};
                while (getNextLine(nextBuf, sizeof(nextBuf), &datalen, pend))
                {
                    pend += datalen;
                    ptr = nextBuf;
                    //while (std::isblank(*ptr, std::locale("en_US.UTF-8")))
                    //    ptr++;
                    len = strlen(ptr);
                    while (len > 0 && (std::isblank(*(ptr + len - 1), std::locale("en_US.UTF-8")) || (*(ptr + len - 1) == '\r') || (*(ptr + len - 1) == '\n')))
                        len--;
                    if (*(ptr + len - 1) == ';')
                    {
                        retstring.append(nextBuf);
                        break;
                    }
                    else
                    {
                        retstring.append(nextBuf);
                        continue;
                    }
                }// end of while
            }//end of if
            else
            {
                retstring.append(buf);
            }

        }
        else if (*(ptr + len - 1) == '=')    //以 = 结尾
        {
            char newLine[4096] = {0};
            char nextBuf[2048] = {0};

            *(ptr + len) = '\0';
            strcat(newLine, buf);
            strcat(newLine, " ");
            while (getNextLine(nextBuf, sizeof(nextBuf), &datalen, pend))
            {
                pend += datalen;
                ptr = nextBuf;
                while (std::isblank(*ptr, std::locale("en_US.UTF-8")))
                    ptr++;
                len = strlen(ptr);
                while (len > 0 && (std::isblank(*(ptr + len - 1), std::locale("en_US.UTF-8")) || (*(ptr + len - 1) == '\r') || (*(ptr + len - 1) == '\n')))
                    len--;
                if (*(ptr + len - 1) == ',')
                {
                    //逗号结尾
                    *(ptr + len) = '\0';
                    strcat(newLine, ptr);
                    strcat(newLine, " ");
                    continue;
                }
                else
                {
                    strcat(newLine, ptr);
                    //strcat(newLine, "\n");
                    break;
                }
            }
            //写入文件
            // fwrite(newLine, sizeof(char), strlen(newLine), fpnew);
            retstring.append(newLine);
        }
        else
        {
            retstring.append(buf);
        }

    }

    return retstring;
}
