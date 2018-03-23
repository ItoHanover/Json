#include "itojson.h"

/*
//返回JSON_PARSE_OK时表示函数成功
//返回JSON_PARSE_EXPECT_VALUE时表示一个JSON只含有空白
//返回JSON_PARSE_INVALID_VALUE时表示一个值在空白之后还有其他字符
//返回JSON_PARSE_ROOT_NOT_SINGULAR时表示不是对应字面值

//enum {
//	JSON_PARSE_OK = 0,
//	JSON_PARSE_EXPECT_VALUE,
//	JSON_PARSE_INVALID_VALUE,
//	JSON_PARSE_ROOT_NOT_SINGULAR
//};
*/

//解析入口
int ItoJson::JsonParse(JsonValue* tarVal, string JsonStr)
{
	JsonContext tempContext;
	tempContext.json = JsonStr.begin();
	tempContext.jsonEnd = JsonStr.end();
	tarVal->type = JSON_NULL;
	JsonParseWhitespace(&tempContext);
	return JsonParseValue(&tempContext, tarVal);
}

//解析空格等空白字符
void ItoJson::JsonParseWhitespace(JsonContext* tarContext)
{
	string::iterator jsonIter = tarContext->json;
	while (jsonIter != tarContext->jsonEnd && (*jsonIter == ' ' || *jsonIter == '\t' || *jsonIter == '\n' || *jsonIter == '\r'))
		jsonIter++;
	tarContext->json = jsonIter;
}

//解析NULL与BOOL
int ItoJson::JsonParseLiteral(JsonContext* tarContext, JsonValue* tarVal, char cType)
{
	EXPECT(tarContext, cType);
	if (cType == 'f')
	{
		if (tarContext->json == tarContext->jsonEnd || ((tarContext->json) + 1) == tarContext->jsonEnd || ((tarContext->json) + 2) == tarContext->jsonEnd || ((tarContext->json) + 3) == tarContext->jsonEnd)
			return JSON_PARSE_INVALID_VALUE;
		if (*tarContext->json != 'a' || *((tarContext->json) + 1) != 'l' || *((tarContext->json) + 2) != 's' || *((tarContext->json) + 3) != 'e')
			return JSON_PARSE_INVALID_VALUE;
		tarContext->json += 4;
		tarVal->type = JSON_FALSE;
	}
	else if (cType == 't')
	{	
		if (tarContext->json == tarContext->jsonEnd || ((tarContext->json) + 1) == tarContext->jsonEnd || ((tarContext->json) + 2) == tarContext->jsonEnd)
			return JSON_PARSE_INVALID_VALUE;
		if (*tarContext->json != 'r' || *((tarContext->json) + 1) != 'u' || *((tarContext->json) + 2) != 'e')
			return JSON_PARSE_INVALID_VALUE;
		tarContext->json += 3;
		tarVal->type = JSON_TRUE;
	}
	else
	{	
		if (tarContext->json == tarContext->jsonEnd || ((tarContext->json) + 1) == tarContext->jsonEnd || ((tarContext->json) + 2) == tarContext->jsonEnd)
			return JSON_PARSE_INVALID_VALUE;
		if (*tarContext->json != 'u' || *((tarContext->json) + 1) != 'l' || *((tarContext->json) + 2) != 'l')
			return JSON_PARSE_INVALID_VALUE;
		tarContext->json += 3;
		tarVal->type = JSON_NULL;
	}
	return JSON_PARSE_OK;

}

//解析数字
int ItoJson::JsonParseNumber(JsonContext* tarContext, JsonValue* tarVal)
{
	string::iterator end;
	end = tarContext->json;
	//开始格式判断
	if (end != tarContext->jsonEnd && *end == '-')
		end++;
	if (end != tarContext->jsonEnd && *end == '0')
		end++;
	else
	{
		if (!((*end) >= '0' && (*end) <= '9'))
			return JSON_PARSE_INVALID_VALUE;
		for (end++; end != tarContext->jsonEnd && ((*end) >= '1' && (*end) <= '9');end++);
	}

	if (end != tarContext->jsonEnd && *end == '.')
	{
		end++;
		if (end == tarContext->jsonEnd || (!((*end) >= '0' && (*end) <= '9')))
			return JSON_PARSE_INVALID_VALUE;

		for (end++; end != tarContext->jsonEnd && ((*end) >= '0' && (*end) <= '9'); end++);
	}

	if (end != tarContext->jsonEnd && (*end == 'e' || *end == 'E'))
	{
		end++;
		if (end != tarContext->jsonEnd && (*end == '+' || *end == '-'))
			end++;

		if (end != tarContext->jsonEnd && (!((*end) >= '0' && (*end) <= '9')))
			return JSON_PARSE_INVALID_VALUE;

		for (end++; end != tarContext->jsonEnd && ((*end) >= '0' && (*end) <= '9'); end++);
	}

	//开始转换
	string NumStr(tarContext->json, end);
	istringstream tempIstrStream(NumStr);

	if (tarContext->json == end)
		return JSON_PARSE_INVALID_VALUE;

	char* tempCA = new char[end - tarContext->json];
	int numFlag = 0;
	for (string::iterator tIter = tarContext->json; tIter != end; tIter++)
	{
		tempCA[numFlag] = *tIter;
		numFlag = numFlag + 1;
	}

	double dNum = strtod(tempCA, nullptr);

	if (fabs(dNum) == HUGE_VAL)
		return JSON_PARSE_NUMBER_TOO_BIG;

	//tempIstrStream >> dNum;

	tarContext->json = end;
	tarVal->type = JSON_NUMBER;
	tarVal->n = dNum;
	return JSON_PARSE_OK;
}

string::iterator ItoJson::JsonParseHex4(JsonContext* tarContext ,string::iterator strIter, unsigned* u)
{
	*u = 0;
	for (int i = 0; i < 4; i++) 
	{
		char ch = *strIter++;
		*u <<= 4;
		if (ch >= '0' && ch <= '9')  *u |= ch - '0';
		else if (ch >= 'A' && ch <= 'F')  *u |= ch - ('A' - 10);
		else if (ch >= 'a' && ch <= 'f')  *u |= ch - ('a' - 10);
		else return tarContext->jsonEnd;
	}
	return strIter;
}

//解析Utf8
string ItoJson::JsonEncodeUtf8(string& tempStr, unsigned u)
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
int ItoJson::JsonParseStr(JsonContext* tarContext, JsonValue* tarVal)
{

	tarVal->str.clear();
	EXPECT(tarContext, '\"');
	unsigned u;
	unsigned u2;
	string::iterator endIter = tarContext->json;
	string tempStr = "";
	for (;;)
	{
		if (endIter == tarContext->jsonEnd)
		{
			tarContext->json = endIter;
			return JSON_PARSE_MISS_QUOTATION_MARK;
		}
		char tempCh =  *endIter++;
		switch (tempCh)
		{
		case '\"':
			tarVal->str = tempStr;
			tarContext->json = endIter;
			tarVal->type = JSON_STRING;
			return JSON_PARSE_OK;
			break;
		case '\\' :
			if(endIter == tarContext->jsonEnd)
				return JSON_PARSE_INVALID_STRING_ESCAPE;
			switch (*endIter++) 
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
				if ((endIter = JsonParseHex4(tarContext,endIter, &u)) == tarContext->jsonEnd)
				{
					tarContext->json = endIter;
					return JSON_PARSE_INVALID_UNICODE_HEX;
				}
				if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
					if (endIter == tarContext->jsonEnd || *endIter++ != '\\')
					{
						tarContext->json = endIter;
						return JSON_PARSE_INVALID_UNICODE_SURROGATE;
					}
					if (endIter == tarContext->jsonEnd || *endIter++ != 'u')
					{
						tarContext->json = endIter;
						return JSON_PARSE_INVALID_UNICODE_SURROGATE;
					}
					if (endIter == tarContext->jsonEnd || (endIter = JsonParseHex4(tarContext,endIter, &u2)) == tarContext->jsonEnd)
					{
						tarContext->json = endIter;
						return JSON_PARSE_INVALID_UNICODE_HEX;
					}
					if (u2 < 0xDC00 || u2 > 0xDFFF)
					{
						tarContext->json = endIter;
						return JSON_PARSE_INVALID_UNICODE_SURROGATE;
					}
					u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
				}
				JsonEncodeUtf8(tempStr, u);
				break;
			default:
				tarContext->json = endIter;
				return JSON_PARSE_INVALID_STRING_ESCAPE;
			}
			break;
		default:
			if (endIter == tarContext->jsonEnd)
			{
				tarContext->json = endIter;
				return JSON_PARSE_MISS_QUOTATION_MARK;
			}
			if ((unsigned char)*endIter < 0x20)
			{
				tarContext->json = endIter;
				return JSON_PARSE_INVALID_STRING_CHAR;
			}
			tempStr.push_back(tempCh);
			break;
		}
	}
}

//解析数组
int ItoJson::JsonParseArray(JsonContext* tarContext, JsonValue* tarVal)
{
	int ret;
	EXPECT(tarContext, '[');
	JsonParseWhitespace(tarContext);
	if (tarContext->json != tarContext->jsonEnd && *tarContext->json == ']')
	{
		tarContext->json++;
		tarVal->type = JSON_ARRAY;
		return JSON_PARSE_OK;
	}

	for (;;)
	{
		JsonValue tempVal;
		if ((ret = JsonParseValue(tarContext, &tempVal)) != JSON_PARSE_OK)
			break;
		tarVal->arr.push_back(tempVal);
		JsonParseWhitespace(tarContext);
		if (tarContext->json != tarContext->jsonEnd && *tarContext->json == ',')
		{			
			tarContext->json++;
			JsonParseWhitespace(tarContext);
		}
		else if (tarContext->json != tarContext->jsonEnd && *tarContext->json == ']')
		{
			tarContext->json++;
			tarVal->type = JSON_ARRAY;
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

double ItoJson::JsonGetNumber(const JsonValue* tarVal)
{
	assert(tarVal != NULL && tarVal->type == JSON_NUMBER);
	return tarVal->n;
}

JsonType ItoJson::JsonGetType(const JsonValue* tarVal)
{
	assert(tarVal != NULL);
	return tarVal->type;
}

string ItoJson::JsonGetString(const JsonValue* tarVal)
{
	assert(tarVal != NULL && tarVal->type == JSON_STRING);
	return tarVal->str;
}

size_t ItoJson::JsonGetArraySize(JsonValue* tarVal)
{
	assert(tarVal != NULL && tarVal->type == JSON_ARRAY);
	return tarVal->arr.size();
}

JsonValue& ItoJson::JsonGetArrayElement(JsonValue* tarVal, size_t iLocation)
{
	assert(tarVal != NULL && tarVal->type == JSON_ARRAY);
	assert(iLocation < tarVal->arr.size());
	return tarVal->arr[iLocation];
}

vector<JsonValue>& JsonFetArray(JsonValue* tarVal)
{
	return tarVal->arr;
}

//遇到字符后 根据首字符的不同调用对应解析函数。
int ItoJson::JsonParseValue(JsonContext* tarContext, JsonValue* tarVal)
{
	if(tarContext->json == tarContext->jsonEnd)
		return JSON_PARSE_EXPECT_VALUE;
	switch (*tarContext->json) {
	case 'n':	return JsonParseLiteral(tarContext, tarVal, 'n');
	case 'f':	return JsonParseLiteral(tarContext, tarVal, 'f');
	case 't':	return JsonParseLiteral(tarContext, tarVal, 't');
	case '"':	return JsonParseStr(tarContext, tarVal);
	case '[':  return JsonParseArray(tarContext, tarVal);
	default:	return JsonParseNumber(tarContext, tarVal);
	}
}

