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

int ItoJson::JsonParse(JsonValue* tarVal, string JsonStr)
{
	JsonContext tempContext;
	tempContext.json = JsonStr.begin();
	tempContext.jsonEnd = JsonStr.end();
	tarVal->type = JSON_NULL;
	JsonParseWhitespace(&tempContext);
	return JsonParseValue(&tempContext, tarVal);
}

void ItoJson::JsonParseWhitespace(JsonContext* tarContext)
{
	string::iterator jsonIter = tarContext->json;
	while (jsonIter != tarContext->jsonEnd && (*jsonIter == ' ' || *jsonIter == '\t' || *jsonIter == '\n' || *jsonIter == '\r'))
		jsonIter++;
	tarContext->json = jsonIter;
}

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

	if(end != tarContext->jsonEnd && (!((*end) >= '0' && (*end) <= '9')))
		return JSON_PARSE_INVALID_VALUE;
	//完成格式判断

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

int ItoJson::JsonParseStr(JsonContext* tarContext, JsonValue* tarVal)
{
	tarVal->str.clear();
	EXPECT(tarContext, '\"');
	string::iterator endIter = tarContext->json;
	string tempStr = "";
	for (;;)
	{
		if(endIter == tarContext->jsonEnd)
			return JSON_PARSE_MISS_QUOTATION_MARK;
		switch (*endIter++)
		{
		case '\"':
			tarVal->str = tempStr;
			return JSON_PARSE_OK;
			break;
		case '\\' :
			if(endIter == tarContext->jsonEnd)
				return JSON_PARSE_INVALID_STRING_ESCAPE;
			switch (*endIter++) 
			{
			case '\"':
				tempStr.insert(tempStr.begin(), '\"');
				break;
			case '\\':
				tempStr.insert(tempStr.begin(), '\\');
			case '/':
				tempStr.insert(tempStr.begin(), '/');
				break;
			case 'b':
				tempStr.insert(tempStr.begin(), '\b');
				break;
			case 'n':
				tempStr.insert(tempStr.begin(), '\n');
				break;
			case 'r':
				tempStr.insert(tempStr.begin(), '\r');
				break;
			case 't':
				tempStr.insert(tempStr.begin(), '\t');
				break;
			case 'f':
				tempStr.insert(tempStr.begin(), '\f');
				break;
			default:
				return JSON_PARSE_INVALID_STRING_ESCAPE;
			}
			break;
		default:
			if(endIter == tarContext->jsonEnd)
				return JSON_PARSE_MISS_QUOTATION_MARK;
			if ((unsigned char)*endIter < 0x20)
			{
				return JSON_PARSE_INVALID_STRING_CHAR;
			}
			tempStr.insert(tempStr.begin(), *endIter);
			break;
		}
	}
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

int ItoJson::JsonParseValue(JsonContext* tarContext, JsonValue* tarVal)
{
	if(tarContext->json == tarContext->jsonEnd)
		return JSON_PARSE_EXPECT_VALUE;
	switch (*tarContext->json) {
	case 'n':	return JsonParseLiteral(tarContext, tarVal, 'n');
	case 'f':	return JsonParseLiteral(tarContext, tarVal, 'f');
	case 't':	return JsonParseLiteral(tarContext, tarVal, 't');
	case '"':	return JsonParseStr(tarContext, tarVal);
	default:	return JsonParseNumber(tarContext, tarVal);
	}
}

