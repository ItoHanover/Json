#pragma once
#include "JsValue.h"

enum RETURN_FLAG {
	JSON_PARSE_OK = 0,
	JSON_PARSE_EXPECT_VALUE,
	JSON_PARSE_INVALID_VALUE,
	JSON_PARSE_ROOT_NOT_SINGULAR,
	JSON_PARSE_NUMBER_TOO_BIG,
	JSON_PARSE_MISS_QUOTATION_MARK,
	JSON_PARSE_INVALID_STRING_ESCAPE,
	JSON_PARSE_INVALID_STRING_CHAR,
	JSON_PARSE_INVALID_UNICODE_HEX,
	JSON_PARSE_INVALID_UNICODE_SURROGATE,
	JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
	JSON_PARSE_MISS_KEY,
	JSON_PARSE_MISS_COLON,
	JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET
};

class JsParse
{
public:
	RETURN_FLAG JsonParse(shared_ptr<BaseJs>& resVal, string& tarStr);
private:
	void JsonParseWhitespace(string::iterator& startIter, string::iterator endIter);

	void EXPECT(string::iterator& startIter, char rVal)
	{
		assert(*startIter == (rVal)); startIter++;
	};

	RETURN_FLAG JsonParseValue(string::iterator& startIter, string::iterator endIter, shared_ptr<BaseJs>& resVal);					//解析值
	RETURN_FLAG JsonParseLiteral(string::iterator& startIter, string::iterator endIter, shared_ptr<BaseJs>& resVal, char cType);		//解析字面值类型
	RETURN_FLAG JsonParseNumber(string::iterator& startIter, string::iterator endIter, shared_ptr<BaseJs>& resVal);					//解析数字
	RETURN_FLAG JsonParseStr(string::iterator& startIter, string::iterator endIter, shared_ptr<BaseJs>& resVal);						//解析字符串
	RETURN_FLAG JsonParseArray(string::iterator& startIter, string::iterator endIter, shared_ptr<BaseJs>& resVal);					//解析数组
	RETURN_FLAG JsonParseObject(string::iterator& startIter, string::iterator endIter, shared_ptr<BaseJs>& resVal);					//解析对象

private:
	string::iterator JsonParseHex4(string::iterator& startIter, string::iterator& endIter, string::iterator strIter, unsigned* u);
	string JsonEncodeUtf8(string& tempStr, unsigned u);

};