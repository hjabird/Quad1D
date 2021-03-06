#include "Token.h"
/*////////////////////////////////////////////////////////////////////////////
Token.cpp

Represents a token generated by a lexer (Tokeniser).

Copyright 2018 HJA Bird

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/////////////////////////////////////////////////////////////////////////////


const std::string & HBTK::Token::value() const {
	return m_value;
}

const int HBTK::Token::line() const {
	return m_line;
}

const int HBTK::Token::char_idx() const {
	return m_char;
}

bool HBTK::Token::isnum() const
{
	return m_token_type == INTEGER || m_token_type == FLOAT;
}

bool HBTK::Token::isvar() const
{
	return m_token_type == VARIABLE;
}

bool HBTK::Token::isword() const
{
	return m_token_type == WORD;	
}

bool HBTK::Token::isstr() const
{
	return m_token_type == STRING;
}

bool HBTK::Token::ispunct() const
{
	return m_token_type == PUNCTUATION;
}

bool HBTK::Token::iswhitespace() const
{
	return m_token_type == WHITE_SPACE;
}

bool HBTK::Token::isbracket() const
{
	if (m_token_type != PUNCTUATION) 
	{
		return false;
	}
	else {
		if (m_value == "(" ||
			m_value == ")" ||
			m_value == "{" ||
			m_value == "}" ||
			m_value == "[" ||
			m_value == "]") {
			return true;
		}
		else {
			return false;
		}
	}
}

bool HBTK::Token::isopenbracket() const 
{
	if (m_token_type != PUNCTUATION) {
		return false;
	}
	else {
		if (m_value == "(" ||
			m_value == "{" ||
			m_value == "[") {
			return true;
		}
		else {
			return false;
		}
	}
}

bool HBTK::Token::isclosebracket() const 
{
	if (m_token_type != PUNCTUATION) {
		return false;
	}
	else {
		if (m_value == ")" ||
			m_value == "}" ||
			m_value == "]") {
			return true;
		}
		else {
			return false;
		}
	}
}


bool HBTK::Token::isinteger() const {
	if (m_token_type != INTEGER) {
		return false;
	}
	else {
		if (value().find('.') != std::string::npos)
			return false;
		else {
			return true;
		}
	}
}

bool HBTK::Token::isfloat() const
{
	if (m_token_type != FLOAT) {
		return false;
	}
	else {
		if (value().find('.') != std::string::npos)
			return true;
		else {
			return false;
		}
	}
}

bool HBTK::Token::isnewline() const
{
	return m_token_type == WHITE_SPACE && m_value[0] == '\n';
}

