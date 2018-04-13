#include "JsParse.h"

RETURN_FLAG JsParse::JsonParse(shared_ptr<BaseJs>& resVal,string& tarStr)
{
	if (resVal != nullptr)
		resVal.reset();
	string::iterator tempStartIter = tarStr.begin();
	JsonParseWhitespace(tempStartIter,tarStr.end());
	return JsonParseValue(tempStartIter, tarStr.end(), resVal);
}

void JsParse::JsonParseWhitespace(string::iterator& startIter, string::iterator endIter)
{
	string::iterator tempIter = startIter;
	for (; startIter != endIter && (*tempIter == ' ' || *tempIter == '\t' || *tempIter == '\n' || *tempIter == '\r'); tempIter++);
	startIter = tempIter;
}

RETURN_FLAG JsParse::JsonParseValue(string::iterator& startIter, string::iterator endIter, shared_ptr<BaseJs>& tarVal)
{
	if (startIter == endIter)
		return JSON_PARSE_EXPECT_VALUE;
	switch (*startIter)
	{
	case 'n':	return JsonParseLiteral(startIter, endIter, tarVal, 'n');
	case 'f':	return JsonParseLiteral(startIter, endIter, tarVal, 'f');
	case 't':	return JsonParseLiteral(startIter, endIter, tarVal, 't');
	case '"':	return JsonParseStr(startIter, endIter, tarVal);
	case '[':	return JsonParseArray(startIter, endIter, tarVal);
	case '{':	return JsonParseObject(startIter, endIter, tarVal);
	default:	return JsonParseNumber(startIter, endIter, tarVal);
	}
}

//解析NULL TRUE与FALSE类型
RETURN_FLAG JsParse::JsonParseLiteral(string::iterator& startIter, string::iterator endIter, shared_ptr<BaseJs>& tarVal, char cType)
{
	EXPECT(startIter, cType);

	if (cType == 'f')
	{
		if (startIter == endIter || (startIter + 1) == endIter || (startIter + 2) == endIter || (startIter + 3) == endIter)
			return JSON_PARSE_INVALID_VALUE;
		if (*startIter != 'a' || *(startIter + 1) != 'l' || *(startIter + 2) != 's' || *(startIter + 3) != 'e')
			return JSON_PARSE_INVALID_VALUE;
		startIter += 4;
		tarVal = make_shared<JsLiteral>();
		tarVal->SetType(JVT_FALSE);
	}
	else if (cType == 't')
	{
		if (startIter == endIter || (startIter + 1) == endIter || (startIter + 2) == endIter )
			return JSON_PARSE_INVALID_VALUE;
		if (*startIter != 'r' || *(startIter + 1) != 'u' || *(startIter + 2) != 'e')
			return JSON_PARSE_INVALID_VALUE;
		startIter += 3;
		tarVal = make_shared<JsLiteral>();
		tarVal->SetType(JVT_TRUE);
	}
	else
	{
		if (startIter == endIter || (startIter + 1) == endIter || (startIter + 2) == endIter)
			return JSON_PARSE_INVALID_VALUE;
		if (*startIter != 'u' || *(startIter + 1) != 'l' || *(startIter + 2) != 'l')
			return JSON_PARSE_INVALID_VALUE;
		startIter += 3;
		tarVal = make_shared<JsLiteral>();
		tarVal->SetType(JVT_NULL);
	}
	return JSON_PARSE_OK;

}

//解析数字
RETURN_FLAG JsParse::JsonParseNumber(string::iterator& startIter, string::iterator endIter, shared_ptr<BaseJs>& tarVal)
{
	string::iterator end;
	end = startIter;
	//开始格式判断
	if (end != endIter && *end == '-')
		end++;
	if (end != endIter && *end == '0')
		end++;
	else
	{
		if (!((*end) >= '0' && (*end) <= '9'))
			return JSON_PARSE_INVALID_VALUE;
		for (end++; end != endIter && ((*end) >= '1' && (*end) <= '9'); end++);
	}

	if (end != endIter && *end == '.')
	{
		end++;
		if (end == endIter || (!((*end) >= '0' && (*end) <= '9')))
			return JSON_PARSE_INVALID_VALUE;

		for (end++; end != endIter && ((*end) >= '0' && (*end) <= '9'); end++);
	}

	if (end != endIter && (*end == 'e' || *end == 'E'))
	{
		end++;
		if (end != endIter && (*end == '+' || *end == '-'))
			end++;

		if (end != endIter && (!((*end) >= '0' && (*end) <= '9')))
			return JSON_PARSE_INVALID_VALUE;

		for (end++; end != endIter && ((*end) >= '0' && (*end) <= '9'); end++);
	}

	//开始转换
	string NumStr(startIter, end);
	//istringstream tempIstrStream(NumStr);

	if (startIter == end)
		return JSON_PARSE_INVALID_VALUE;

	char* tempCA = new char[end - startIter];
	int numFlag = 0;
	for (string::iterator tIter = startIter; tIter != end; tIter++)
	{
		tempCA[numFlag] = *tIter;
		numFlag = numFlag + 1;
	}

	double dNum = strtod(tempCA, nullptr);

	if (fabs(dNum) == HUGE_VAL)
		return JSON_PARSE_NUMBER_TOO_BIG;

	//tempIstrStream >> dNum;

	startIter = end;
	tarVal = make_shared<JsNumber>();
	tarVal->SetType(JVT_NUMBER);
	tarVal->SetValue(dNum);
	return JSON_PARSE_OK;
}

string::iterator JsParse::JsonParseHex4(string::iterator& startIter, string::iterator& endIter, string::iterator strIter, unsigned* u)
{
	*u = 0;
	for (int i = 0; i < 4; i++)
	{
		char ch = *strIter++;
		*u <<= 4;
		if (ch >= '0' && ch <= '9')  *u |= ch - '0';
		else if (ch >= 'A' && ch <= 'F')  *u |= ch - ('A' - 10);
		else if (ch >= 'a' && ch <= 'f')  *u |= ch - ('a' - 10);
		else return endIter;
	}
	return strIter;
}

//解析Utf8
string JsParse::JsonEncodeUtf8(string& tempStr, unsigned u)
{
	if (u <= 0x7F)
		tempStr.push_back(u & 0xFF);
	else if (u <= 0x7FF) {
		tempStr.push_back(0xC0 | ((u >> 6) & 0xFF));
		tempStr.push_back(0x80 | (u & 0x3F));
	}
	else if (u <= 0xFFFF) {
		tempStr.push_back(0xE0 | ((u >> 12) & 0xFF));
		tempStr.push_back(0x80 | ((u >> 6) & 0x3F));
		tempStr.push_back(0x80 | (u & 0x3F));
	}
	else {
		assert(u <= 0x10FFFF);
		tempStr.push_back(0xF0 | ((u >> 18) & 0xFF));
		tempStr.push_back(0x80 | ((u >> 12) & 0x3F));
		tempStr.push_back(0x80 | ((u >> 6) & 0x3F));
		tempStr.push_back(0x80 | (u & 0x3F));
	}
	return tempStr;
}

//解析字符串
RETURN_FLAG JsParse::JsonParseStr(string::iterator& startIter, string::iterator endIter, shared_ptr<BaseJs>& resVal)
{
	resVal = make_shared<JsString>();
	
	EXPECT(startIter, '\"');
	unsigned u;
	unsigned u2;
	string::iterator end = startIter;
	string tempStr = "";
	for (;;)
	{
		if (end == endIter)
		{
			startIter = endIter;
			return JSON_PARSE_MISS_QUOTATION_MARK;
		}
		char tempCh = *end++;
		switch (tempCh)
		{
		case '\"':
			resVal->SetValue(tempStr);
			startIter = end;
			resVal->SetType(JVT_STRING);
			return JSON_PARSE_OK;
			break;
		case '\\':
			if (end == endIter)
				return JSON_PARSE_INVALID_STRING_ESCAPE;
			switch (*end++)
			{
			case '\"':
				tempStr.push_back('\"');
				break;
			case '\\':
				tempStr.push_back('\\');
				break;
			case '/':
				tempStr.push_back('/');
				break;
			case 'b':
				tempStr.push_back('\b');
				break;
			case 'n':
				tempStr.push_back('\n');
				break;
			case 'r':
				tempStr.push_back('\r');
				break;
			case 't':
				tempStr.push_back('\t');
				break;
			case 'f':
				tempStr.push_back('\f');
				break;
			case 'u':
				if ((end = JsonParseHex4(startIter, endIter, end, &u)) == endIter)
				{
					startIter = end;
					return JSON_PARSE_INVALID_UNICODE_HEX;
				}
				if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
					if (end == endIter || *end++ != '\\')
					{
						startIter = end;
						return JSON_PARSE_INVALID_UNICODE_SURROGATE;
					}
					if (end == endIter || *end++ != 'u')
					{
						startIter = end;
						return JSON_PARSE_INVALID_UNICODE_SURROGATE;
					}
					if (end == endIter || (end = JsonParseHex4(startIter, endIter, end, &u2)) == endIter)
					{
						startIter = end;
						return JSON_PARSE_INVALID_UNICODE_HEX;
					}
					if (u2 < 0xDC00 || u2 > 0xDFFF)
					{
						startIter = end;
						return JSON_PARSE_INVALID_UNICODE_SURROGATE;
					}
					u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
				}
				JsonEncodeUtf8(tempStr, u);
				break;
			default:
				startIter = end;
				return JSON_PARSE_INVALID_STRING_ESCAPE;
			}
			break;
		default:
			if (end == endIter)
			{
				startIter = end;
				return JSON_PARSE_MISS_QUOTATION_MARK;
			}
			if ((unsigned char)*end < 0x20)
			{
				startIter = end;
				return JSON_PARSE_INVALID_STRING_CHAR;
			}
			tempStr.push_back(tempCh);
			break;
		}
	}
}


//解析数组
RETURN_FLAG JsParse::JsonParseArray(string::iterator& startIter, string::iterator endIter, shared_ptr<BaseJs>& resVal)
{
	RETURN_FLAG ret;
	vector<BaseJs> tarVector;
	EXPECT(startIter, '[');
	JsonParseWhitespace(startIter, endIter);

	if (startIter != endIter && *startIter == ']')
	{
		startIter++;
		resVal = make_shared<JsArray>();
		resVal->SetType(JVT_ARRAY);
		return JSON_PARSE_OK;
	}

	for (;;)
	{
		shared_ptr<BaseJs> tempVal = make_shared<BaseJs>();
		if ((ret = JsonParseValue(startIter,endIter, tempVal)) != JSON_PARSE_OK)
			break;
		tarVector.push_back(*tempVal);
		JsonParseWhitespace(startIter, endIter);
		if (startIter != endIter && *startIter == ',')
		{
			startIter++;
			JsonParseWhitespace(startIter, endIter);
		}
		else if (startIter != endIter && *startIter == ']')
		{
			startIter++;
			resVal->SetType(JVT_ARRAY);
			resVal->SetValue(tarVector);
			return JSON_PARSE_OK;
		}
		else
		{
			ret = JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
			break;
		}
	}
	return ret;
}

//解析对象
RETURN_FLAG JsParse::JsonParseObject(string::iterator& startIter, string::iterator endIter, shared_ptr<BaseJs>& resVal)
{
	map<string, BaseJs> tempVJM;
	pair<string, BaseJs> m;
	RETURN_FLAG ret;
	EXPECT(startIter, '{');
	JsonParseWhitespace(startIter, endIter);

	if (startIter != endIter && *startIter == '}')
	{
		startIter++;
		resVal->SetType(JVT_OBJECT);
		return JSON_PARSE_OK;
	}

	m.first = "";

	for (;;)
	{
		shared_ptr<BaseJs> tempKey = make_shared<BaseJs>();
		shared_ptr<BaseJs> tempVal = make_shared<BaseJs>();
		//解析KEY
		if (startIter == endIter || *startIter != '"')
		{
			ret = JSON_PARSE_MISS_KEY;
			break;
		}
		if ((ret = JsonParseStr(startIter,endIter, tempKey)) != JSON_PARSE_OK)
			break;
		tempKey->GetValue(m.first);

		//判断是否存在冒号
		JsonParseWhitespace(startIter, endIter);
		if (startIter == endIter || *startIter != ':')
		{
			ret = JSON_PARSE_MISS_COLON;
			break;
		}
		startIter++;
		JsonParseWhitespace(startIter, endIter);

		//解析Value
		if ((ret = JsonParseValue(startIter, endIter, tempVal)) != JSON_PARSE_OK)
			break;
		m.second = *tempVal;
		tempVJM.insert(m);
		m.first = "";//将m压入临时vector后要恢复初始化状态

		JsonParseWhitespace(startIter, endIter);

		//完成一个部分的解析 开始检测接下来是','还是'}'
		//','则继续解析 '}'则完成解析
		//否则结束解析 报错
		if (startIter != endIter && *startIter == ',')
		{
			startIter++;
			JsonParseWhitespace(startIter, endIter);
		}
		else if (startIter != endIter && *startIter == '}')
		{
			startIter++;
			resVal->SetValue(tempVJM);
			resVal->SetType(JVT_OBJECT);
			return JSON_PARSE_OK;
		}
		else
		{
			ret = JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET;
			break;
		}


	}
	return ret;
}