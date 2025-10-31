#include "ReadScript.h"
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <cstdio>

#define DEBUG_READSCRIPT 0

CReadScript::CReadScript()
{
    this->m_file = nullptr;
    memset(this->m_path, 0, sizeof(this->m_path));
    this->m_number = -1;
    memset(this->m_string, 0, sizeof(this->m_string));
    memset(this->m_error, 0, sizeof(this->m_error));
    this->m_line = 0;
    this->m_lastToken = TOKEN_END;
}

CReadScript::~CReadScript()
{
    if (this->m_file != nullptr)
    {
        fclose(this->m_file);
    }
}

bool CReadScript::Load(const char* path)
{
    strncpy(this->m_path, path, sizeof(this->m_path) - 1);
    this->m_path[sizeof(this->m_path) - 1] = '\0';

#ifdef _WIN32
    if (fopen_s(&this->m_file, path, "rt") != 0)
    {
        return false;
    }
#else
    this->m_file = fopen(path, "rt");
    if (this->m_file == nullptr)
    {
        return false;
    }
#endif

    uint8_t buffer[4];
    fread(buffer, 1, 4, this->m_file);
    rewind(this->m_file);

    // Skip UTF-8 BOM if present
    if (buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF)
    {
        fseek(this->m_file, 3, SEEK_SET);
    }

    this->m_line = 1;
    return true;
}

eTokenResult CReadScript::GetToken(bool wReturn)
{
#if DEBUG_READSCRIPT
    printf("[ReadScript::GetToken] Called, wReturn=%d, line=%d\n", wReturn, this->m_line);
#endif
    
    this->m_number = -1;
    memset(this->m_string, 0, sizeof(this->m_string));
    memset(this->m_error, 0, sizeof(this->m_error));

    char ch, *p, str[100];

    do
    {
        if ((ch = fgetc(this->m_file)) == EOF)
        {
            this->m_lastToken = TOKEN_END;
            return this->m_lastToken;
        }

        if (ch == '/')
        {
            if ((ch = fgetc(this->m_file)) == '/')
            {
                while ((ch != '\n') && (ch != EOF))
                {
                    ch = fgetc(this->m_file);
                }

                if (ch == EOF)
                {
                    this->m_lastToken = TOKEN_END;
                    return this->m_lastToken;
                }
            }
        }

        if (ch == '\n')
        {
            this->m_line++;

            if (wReturn)
            {
                this->m_lastToken = TOKEN_END_LINE;
                return this->m_lastToken;
            }

            continue;
        }
    }
    while (isspace(ch) != 0);

    if (isdigit(ch) != 0 || ch == '.' || ch == '-' || ch == '*')
    {
        ungetc(ch, this->m_file);

        p = str;

        while (((ch = getc(this->m_file)) != EOF) && (isdigit(ch) != 0 || ch == '.' || ch == '-' || ch == '*'))
        {
            if (p - str < sizeof(str) - 1)
            {
                *p++ = ch;
            }
            else
            {
                break;
            }
        }

        *p = 0;

        if (strcmp(str, "*") == 0)
        {
            this->m_number = -1;
        }
        else
        {
            this->m_number = (float)atof(str);
        }

        this->m_lastToken = TOKEN_NUMBER;
        
#if DEBUG_READSCRIPT
        printf("[ReadScript::GetToken] Returning TOKEN_NUMBER, value=%.2f\n", this->m_number);
#endif
        
        return this->m_lastToken;
    }
    else if (ch == '"')
    {
        p = this->m_string;

        while (((ch = getc(this->m_file)) != EOF) && (ch != '"') && (ch != '\n'))
        {
            *p++ = ch;
        }

        if (ch != '"')
        {
            ungetc(ch, this->m_file);
        }

        *p = 0;

        if (ch == EOF)
        {
            this->m_lastToken = TOKEN_END;
        }
        else if (ch == '\n')
        {
            this->m_lastToken = TOKEN_END_LINE;
        }
        else
        {
            this->m_lastToken = TOKEN_STRING;
        }

        return this->m_lastToken;
    }
    else if (isalpha(ch) != 0)
    {
        p = this->m_string;

        *p++ = ch;

        while (((ch = getc(this->m_file)) != EOF) && (ch == '.' || ch == '_' || isalnum(ch) != 0))
        {
            *p++ = ch;
        }

        ungetc(ch, this->m_file);

        *p = 0;

        if (strcmp("end", this->m_string) == 0)
        {
            this->m_lastToken = TOKEN_END_SECTION;
        }
        else if (ch == EOF)
        {
            this->m_lastToken = TOKEN_END;
        }
        else
        {
            this->m_lastToken = TOKEN_STRING;
        }

        return this->m_lastToken;
    }

    this->m_lastToken = TOKEN_ERROR;
    return this->m_lastToken;
}

int CReadScript::GetNumber()
{
#if DEBUG_READSCRIPT
    printf("[ReadScript::GetNumber] Called, m_lastToken=%d, m_number=%.2f\n", 
           (int)this->m_lastToken, this->m_number);
#endif
    
    if (this->m_lastToken != TOKEN_NUMBER)
    {
        snprintf(this->m_error, sizeof(this->m_error), 
                "[ReadScript] GetNumber() called but m_lastToken=%d (expected 0=NUMBER). File: '%s' line %d.\n", 
                (int)this->m_lastToken, this->m_path, this->m_line);
        throw 1;
    }

    return (int)this->m_number;
}

int CReadScript::GetAsNumber()
{
    eTokenResult token = this->GetToken(true);

    if (token == TOKEN_END_LINE || token != TOKEN_NUMBER)
    {
        snprintf(this->m_error, sizeof(this->m_error), READ_SCRIPT_LINE_ERROR, this->m_path, this->m_line);
        throw 1;
    }

    return (int)this->m_number;
}

float CReadScript::GetFloatNumber()
{
    if (this->m_lastToken != TOKEN_NUMBER)
    {
        snprintf(this->m_error, sizeof(this->m_error), READ_SCRIPT_LINE_ERROR, this->m_path, this->m_line);
        throw 1;
    }

    return this->m_number;
}

float CReadScript::GetAsFloatNumber()
{
    eTokenResult token = this->GetToken(true);

    if (token == TOKEN_END_LINE || token != TOKEN_NUMBER)
    {
        snprintf(this->m_error, sizeof(this->m_error), READ_SCRIPT_LINE_ERROR, this->m_path, this->m_line);
        throw 1;
    }

    return this->m_number;
}

const char* CReadScript::GetString()
{
    if (this->m_lastToken != TOKEN_STRING)
    {
        snprintf(this->m_error, sizeof(this->m_error), READ_SCRIPT_LINE_ERROR, this->m_path, this->m_line);
        throw 1;
    }

    return this->m_string;
}

const char* CReadScript::GetAsString()
{
    eTokenResult token = this->GetToken(true);

    if (token == TOKEN_END_LINE || token != TOKEN_STRING)
    {
        snprintf(this->m_error, sizeof(this->m_error), READ_SCRIPT_LINE_ERROR, this->m_path, this->m_line);
        throw 1;
    }

    return this->m_string;
}

char* CReadScript::GetError()
{
    return this->m_error;
}
