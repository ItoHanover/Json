#pragma once
#ifndef ITOJSON_H__
#define ITOJSON_H__
#include <string>	//for std::string
#include <cassert>	//for assert
#include <sstream>	//for std::istringstream
#include <vector>	//for std::vector
using namespace std;
enum JsonType{ JSON_NULL, JSON_FALSE, JSON_TRUE, JSON_NUMBER, JSON_STRING, JSON_ARRAY, JSON_OBJECT };

/*
//返回JSON_PARSE_OK时表示函数成功
//返回JSON_PARSE_EXPECT_VALUE时表示一个JSON只含有空白
//返回JSON_PARSE_INVALID_VALUE时表示一个值在空白之后还有其他字符
//返回JSON_PARSE_ROOT_NOT_SINGULAR时表示不是对应字面值
//返回JSON_PARSE_NUMBER_TOO_BIG表示数值过大
//返回JSON_PARSE_MISS_QUOTATION_MARK表示未以\"结尾，或在字符串中出现\0
//返回JSON_PARSE_INVALID_STRING_ESCAPE表示错误出现不支持转义的字符
//返回JSON_PARSE_INVALID_STRING_CHAR表示出现非法字符
//返回JSON_PARSE_INVALID_UNICODE_SURROGATE表示出现UNICODE解析错误
//返回JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET表示解析数组遇到错误结尾
*/
enum {
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
	JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET
};

//
class JsonContext
{
public:
	string::iterator json;
	string::iterator jsonEnd;
};

class JsonValue
{
public:
	vector<JsonValue> arr;
	
	string str;
	
	double n;
	
	JsonType type;
};

class ItoJson
{
public:
	int JsonParse(JsonValue* tarVal, string JsonStr);					//解析JSON

	JsonType JsonGetType(const JsonValue* tarVal);						//获取JSON VALUE的Type
	double JsonGetNumber(const JsonValue* tarVal);						//获取数值形式的JSON VALUE
	string JsonGetString(const JsonValue* tarVal);						//获取字符串形式的JSON VALUE

	void JsonSetType(JsonValue* tarVal);								//设置JSON Type
	void JsonSetNumber(JsonValue* tarVal);								//设置JSON VALUE的数值
	void JsonSetString(JsonValue* tarVal);								//设置JSON VALUE的字符串

	string::iterator JsonParseHex4(JsonContext* tarContext,string::iterator str, unsigned* u);
	string JsonEncodeUtf8(string& tempStr, unsigned u);

	size_t JsonGetArraySize(JsonValue* tarVal);

	JsonValue& JsonGetArrayElement(JsonValue* tarVal,size_t iLocation);
	vector<JsonValue>& JsonGetArray(JsonValue* tarVal);

private:
	void EXPECT(JsonContext* tarContext, char rVal) 
	{
		assert(*tarContext->json == (rVal)); tarContext->json++;
	};
	void JsonParseWhitespace(JsonContext* tarContext);					//解析空格

	int JsonParseLiteral(JsonContext* tarContext, JsonValue* tarVal, char cType);	//解析NULL/False/True
	int JsonParseValue(JsonContext* tarContext, JsonValue* tarVal);					//解析值
	int JsonParseNumber(JsonContext* tarContext, JsonValue* tarVal);				//解析数字
	int JsonParseStr(JsonContext* tarContext, JsonValue* tarVal);					//解析字符串
	int JsonParseArray(JsonContext* tarContext, JsonValue* tarVal);					//解析数组

};

#endif /* ITOJSON_H__ */