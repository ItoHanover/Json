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
//����JSON_PARSE_OKʱ��ʾ�����ɹ�
//����JSON_PARSE_EXPECT_VALUEʱ��ʾһ��JSONֻ���пհ�
//����JSON_PARSE_INVALID_VALUEʱ��ʾһ��ֵ�ڿհ�֮���������ַ�
//����JSON_PARSE_ROOT_NOT_SINGULARʱ��ʾ���Ƕ�Ӧ����ֵ
//����JSON_PARSE_NUMBER_TOO_BIG��ʾ��ֵ����
//����JSON_PARSE_MISS_QUOTATION_MARK��ʾδ��\"��β�������ַ����г���\0
//����JSON_PARSE_INVALID_STRING_ESCAPE��ʾ������ֲ�֧��ת����ַ�
//����JSON_PARSE_INVALID_STRING_CHAR��ʾ���ַǷ��ַ�
//����JSON_PARSE_INVALID_UNICODE_SURROGATE��ʾ����UNICODE��������
//����JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET��ʾ�����������������β
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
	int JsonParse(JsonValue* tarVal, string JsonStr);					//����JSON

	JsonType JsonGetType(const JsonValue* tarVal);						//��ȡJSON VALUE��Type
	double JsonGetNumber(const JsonValue* tarVal);						//��ȡ��ֵ��ʽ��JSON VALUE
	string JsonGetString(const JsonValue* tarVal);						//��ȡ�ַ�����ʽ��JSON VALUE

	void JsonSetType(JsonValue* tarVal);								//����JSON Type
	void JsonSetNumber(JsonValue* tarVal);								//����JSON VALUE����ֵ
	void JsonSetString(JsonValue* tarVal);								//����JSON VALUE���ַ���

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
	void JsonParseWhitespace(JsonContext* tarContext);					//�����ո�

	int JsonParseLiteral(JsonContext* tarContext, JsonValue* tarVal, char cType);	//����NULL/False/True
	int JsonParseValue(JsonContext* tarContext, JsonValue* tarVal);					//����ֵ
	int JsonParseNumber(JsonContext* tarContext, JsonValue* tarVal);				//��������
	int JsonParseStr(JsonContext* tarContext, JsonValue* tarVal);					//�����ַ���
	int JsonParseArray(JsonContext* tarContext, JsonValue* tarVal);					//��������

};

#endif /* ITOJSON_H__ */