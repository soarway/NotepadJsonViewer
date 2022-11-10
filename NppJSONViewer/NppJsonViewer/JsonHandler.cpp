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
        *datalen = pend - ptr ;    //+1����� \nҲ���Ƶ�buf��
    }
    else
    {
        *datalen = pend - ptr + 1;    //+1����� \nҲ���Ƶ�buf��
    }
    
    memcpy(buf, ptr, *datalen);


    return buf;
}

//----------------------------------------------------------------------------------------
//�ж� } ,
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

//�ж� = {
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

        //ͬһ����/**/������е�ע��, ������ for (int a=0; /*AAA*/ a<100; a++) { Ҳ��������ֱ��д��
        if (strstr(ptr, "/*") && strstr(ptr, "*/"))
        {
            // fwrite(buf, sizeof(char), strlen(buf), fpnew);
            retstring.append(buf);
            // retstring.append("\n");
            continue;
        }

        if (strstr(ptr, "/*"))
        {
            bInMark = true;    //��ʼ����ע��ģʽ
            // fwrite(buf, sizeof(char), strlen(buf), fpnew);
            retstring.append(buf);
            // retstring.append("\n");
            continue;
        }

        if (strstr(ptr, "*/"))
        {
            bInMark = false;    //��ʼ�˳�ע��ģʽ
            // fwrite(buf, sizeof(char), strlen(buf), fpnew);
            retstring.append(buf);
            // retstring.append("\n");
            continue;
        }

        //��ע��ģʽ�£����ı��У� ֱ��ԭ��д���ļ�
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
            //�հ���, ֱ��д��հ���
            // fwrite(buf, sizeof(char), strlen(buf), fpnew);
            retstring.append(buf);
            // retstring.append("\n");
            continue;
        }


        //,���Ž�β�� )С�ҺŽ�β
        if (*(ptr + len - 1) == ',')    //,�Ž�β
        {
            if (isDhGh(ptr, ptr + len - 1)) 
            //if (*(ptr + len - 2) == '}')//�ų�  "}  ," ��β�����  ���д���Ҫ�Ż�����һ�м��пո�
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
                        //���Ž�β
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
                //д���ļ�
                // fwrite(newLine, sizeof(char), strlen(newLine), fpnew);
                retstring.append(newLine);
            }
        }
        else if (*(ptr + len - 1) == '{')//�� { ��β
        {
            //char *pe = ptr + len - 1;
            //char *ps = ptr;
            //���Ų���=
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
                //�ҵ� ��= {�� ���ָ�ʽ������Ҫ�ҵ� ; �м�Ĳ��ֲ�����ʽ������
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
        else if (*(ptr + len - 1) == '=')    //�� = ��β
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
                    //���Ž�β
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
            //д���ļ�
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
