#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cassert>
#include <iostream>
using namespace std;

enum JsValType
{
	JVT_NULL = 0,
	JVT_TRUE = 1,
	JVT_FALSE = 2,
	JVT_NUMBER = 3,
	JVT_STRING = 4,
	JVT_ARRAY = 5,
	JVT_OBJECT = 6,
};

//					/---->JsLiteral
//	BaseJs -> JsVal-|---->JsNumber
//		|			\---->JsString
//		|
//		|-->JsArray
//		|-->JsObject

class BaseJs
{
public:
	JsValType JsType;
public:
	void SetType(JsValType tarType) { JsType = tarType; };
	JsValType GetType() { return JsType; };
	virtual ~BaseJs() = default;

	virtual void SetValue(const JsValType& tarVal) {};
	virtual void SetValue(const double& tarVal) {};
	virtual void SetValue(const string& tarVal) {};
	virtual void SetValue(const vector<BaseJs>& tarVal) {};
	virtual void SetValue(const map<string, BaseJs>& tarVal) {};

	virtual void GetValue(JsValType& resVal) {};
	virtual void GetValue(double& resVal) {};
	virtual void GetValue(string& resVal) {};
	virtual void GetValue(vector<BaseJs>& resVal) {};
	virtual void GetValue(map<string, BaseJs>& resVal) {};
};

template<typename js_type>
class JsVal : public BaseJs
{

};

class JsLiteral : public JsVal<JsValType>
{
public:
	virtual void SetValue(const JsValType& tarVal) { JsType = tarVal; }
	virtual void GetValue(JsValType& resVal) { resVal = JsType; }
private:
	//禁用的基类函数
	virtual void SetValue(const double& tarVal) {};
	virtual void SetValue(const string& tarVal) {};
	virtual void SetValue(const vector<BaseJs>& tarVal) {};
	virtual void SetValue(const map<string, BaseJs>& tarVal) {};

	virtual void GetValue(double& resVal) {};
	virtual void GetValue(string& resVal) {};
	virtual void GetValue(vector<BaseJs>& resVal) {};
	virtual void GetValue(map<string, BaseJs>& resVal) {};
};

class JsNumber : public JsVal<double>
{
	double mNumber;
public:
	virtual void SetValue(const double& tarVal) { mNumber = tarVal; }
	virtual void GetValue(double& resVal) { resVal = mNumber; }
private:
	//禁用的基类函数
	virtual void SetValue(const JsValType& tarVal) {};
	virtual void SetValue(const string& tarVal) {};
	virtual void SetValue(const vector<BaseJs>& tarVal) {};
	virtual void SetValue(const map<string, BaseJs>& tarVal) {};

	virtual void GetValue(JsValType& resVal) {};
	virtual void GetValue(string& resVal) {};
	virtual void GetValue(vector<BaseJs>& resVal) {};
	virtual void GetValue(map<string, BaseJs>& resVal) {};
};

class JsString : public JsVal<string>
{
	string mStr;
public:
	virtual void SetValue(const string& tarVal) { mStr = tarVal; }
	virtual void GetValue(string& resVal) { resVal = mStr; }
private:
	//禁用的基类函数
	virtual void SetValue(const JsValType& tarVal) {};
	virtual void SetValue(const double& tarVal) {};
	virtual void SetValue(const vector<BaseJs>& tarVal) {};
	virtual void SetValue(const map<string, BaseJs>& tarVal) {};

	virtual void GetValue(JsValType& resVal) {};
	virtual void GetValue(double& resVal) {};
	virtual void GetValue(vector<BaseJs>& resVal) {};
	virtual void GetValue(map<string, BaseJs>& resVal) {};
};

class JsArray : public BaseJs
{
	vector<BaseJs> mArray;
public:
	virtual void SetValue(const vector<BaseJs>& tarVal) { mArray = tarVal; }
	virtual void GetValue(vector<BaseJs>& resVal) { resVal = mArray; }
private:
	//禁用的基类函数
	virtual void SetValue(const JsValType& tarVal) {};
	virtual void SetValue(const double& tarVal) {};
	virtual void SetValue(const string& tarVal) {};
	virtual void SetValue(const map<string, BaseJs>& tarVal) {};

	virtual void GetValue(JsValType& resVal) {};
	virtual void GetValue(double& resVal) {};
	virtual void GetValue(string& resVal) {};
	virtual void GetValue(map<string, BaseJs>& resVal) {};
};

class JsObject : public BaseJs
{
	map<string, BaseJs> mObject;
public:
	virtual void SetValue(const map<string, BaseJs>& tarVal) { mObject = tarVal; }
	virtual void GetValue(map<string, BaseJs>& resVal) { resVal = mObject; }
private:
	//禁用的基类函数
	virtual void SetValue(const JsValType& tarVal) {};
	virtual void SetValue(const double& tarVal) {};
	virtual void SetValue(const string& tarVal) {};
	virtual void SetValue(const vector<BaseJs>& tarVal) {};

	virtual void GetValue(JsValType& resVal) {};
	virtual void GetValue(double& resVal) {};
	virtual void GetValue(string& resVal) {};
	virtual void GetValue(vector<BaseJs>& resVal) {};
};