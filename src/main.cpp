#include "itojson.h"

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == alength && memcmp(expect, actual, alength) == 0, expect, actual, "%s")
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", "%s")
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", "%s")

#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%Iu")
#else
#define EXPECT_EQ_SIZE_T(expect, actual) EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, "%zu")
#endif

static void test_parse_null() {
	JsonValue v;
	ItoJson itojson;
	v.type = JSON_TRUE;
	EXPECT_EQ_INT(JSON_PARSE_OK, itojson.JsonParse(&v, "null"));
	EXPECT_EQ_INT(JSON_NULL, itojson.JsonGetType(&v));
}

static void test_parse_true() {
	JsonValue v;
	ItoJson itojson;
	v.type = JSON_FALSE;
	EXPECT_EQ_INT(JSON_PARSE_OK, itojson.JsonParse(&v, "true"));
	EXPECT_EQ_INT(JSON_TRUE, itojson.JsonGetType(&v));
}

static void test_parse_false() {
	JsonValue v;
	ItoJson itojson;
	v.type = JSON_FALSE;
	EXPECT_EQ_INT(JSON_PARSE_OK, itojson.JsonParse(&v, "false"));
	EXPECT_EQ_INT(JSON_FALSE, itojson.JsonGetType(&v));
}

#define TEST_NUMBER(expect, json)\
    do {\
		ItoJson itojson;\
        JsonValue v;\
        EXPECT_EQ_INT(JSON_PARSE_OK, itojson.JsonParse(&v, json));\
        EXPECT_EQ_INT(JSON_NUMBER, itojson.JsonGetType(&v));\
        EXPECT_EQ_DOUBLE(expect, itojson.JsonGetNumber(&v));\
    } while(0)

static void test_parse_number() {
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(1.5, "1.5");
	TEST_NUMBER(-1.5, "-1.5");
	TEST_NUMBER(3.1416, "3.1416");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1e10, "1e10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1E-10, "1E-10");
	TEST_NUMBER(-1E10, "-1E10");
	TEST_NUMBER(-1e10, "-1e10");
	TEST_NUMBER(-1E+10, "-1E+10");
	TEST_NUMBER(-1E-10, "-1E-10");
	TEST_NUMBER(1.234E+10, "1.234E+10");
	TEST_NUMBER(1.234E-10, "1.234E-10");
	TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

	TEST_NUMBER(1.0000000000000002, "1.0000000000000002"); /* the smallest number > 1 */
	TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
	TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308");  /* Max subnormal double */
	TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
	TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");  /* Min normal positive double */
	TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");  /* Max double */
	TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json)\
    do {\
        JsonValue v;\
        ItoJson itojson;\
        EXPECT_EQ_INT(JSON_PARSE_OK, itojson.JsonParse(&v, json));\
        EXPECT_EQ_INT(JSON_STRING, itojson.JsonGetType(&v));\
        EXPECT_EQ_STRING(expect, getPtrTest(itojson.JsonGetString(&v)), (itojson.JsonGetString(&v)).length());\
    } while(0)

const char* getPtrTest(string s)
{
	char* ptr = new char[s.length()];
	int i = 0;
	for (string::iterator iter = s.begin(); iter != s.end(); iter++)
	{
		ptr[i] = *iter;
		i++;
	}
	return ptr;
}

static void test_parse_string() {
	TEST_STRING("", "\"\"");
	TEST_STRING("Hello", "\"Hello\"");
	TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
	TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
	TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
	TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
	TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
	TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
}

static void test_parse_array() {
	size_t i, j;
	JsonValue* v;
	ItoJson itojson;

	v = new JsonValue;
	EXPECT_EQ_INT(JSON_PARSE_OK, itojson.JsonParse(v, "[ ]"));
	EXPECT_EQ_INT(JSON_ARRAY, itojson.JsonGetType(v));
	EXPECT_EQ_SIZE_T(0, itojson.JsonGetArraySize(v));
	delete v;

	v = new JsonValue;
	EXPECT_EQ_INT(JSON_PARSE_OK, itojson.JsonParse(v, "[ null , false , true , 123 , \"abc\" ]"));
	EXPECT_EQ_INT(JSON_ARRAY, itojson.JsonGetType(v));
	EXPECT_EQ_SIZE_T(5, itojson.JsonGetArraySize(v));
	EXPECT_EQ_INT(JSON_NULL, itojson.JsonGetType(&itojson.JsonGetArrayElement(v, 0)));
	EXPECT_EQ_INT(JSON_FALSE, itojson.JsonGetType(&itojson.JsonGetArrayElement(v, 1)));
	EXPECT_EQ_INT(JSON_TRUE, itojson.JsonGetType(&itojson.JsonGetArrayElement(v, 2)));
	EXPECT_EQ_INT(JSON_NUMBER, itojson.JsonGetType(&itojson.JsonGetArrayElement(v, 3)));
	EXPECT_EQ_INT(JSON_STRING, itojson.JsonGetType(&itojson.JsonGetArrayElement(v, 4)));
	EXPECT_EQ_DOUBLE(123.0, itojson.JsonGetNumber(&itojson.JsonGetArrayElement(v, 3)));
	EXPECT_EQ_STRING("abc", getPtrTest(itojson.JsonGetString(&itojson.JsonGetArrayElement(v, 4))), itojson.JsonGetArrayElement(v, 4).str.length());
	delete v;

	v = new JsonValue;
	EXPECT_EQ_INT(JSON_PARSE_OK, itojson.JsonParse(v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
	EXPECT_EQ_INT(JSON_ARRAY, itojson.JsonGetType(v));
	EXPECT_EQ_SIZE_T(4, itojson.JsonGetArraySize(v));
	for (i = 0; i < 4; i++) {
		JsonValue* a = &(itojson.JsonGetArrayElement(v, i));
		EXPECT_EQ_INT(JSON_ARRAY, itojson.JsonGetType(a));
		EXPECT_EQ_SIZE_T(i, itojson.JsonGetArraySize(a));
		for (j = 0; j < i; j++) {
			JsonValue* e = &(itojson.JsonGetArrayElement(a, j));
			EXPECT_EQ_INT(JSON_NUMBER, itojson.JsonGetType(e));
			EXPECT_EQ_DOUBLE((double)j, itojson.JsonGetNumber(e));
		}
	}
	delete v;
}

static void test_parse_object() {
	JsonValue* v;
	size_t i;
	ItoJson itojson;

	v = new JsonValue;
	EXPECT_EQ_INT(JSON_PARSE_OK, itojson.JsonParse(v, " { } "));
	EXPECT_EQ_INT(JSON_OBJECT, itojson.JsonGetType(v));
	EXPECT_EQ_SIZE_T(0, itojson.JsonGetObjectSize(v));
	delete v;

	v = new JsonValue;
	EXPECT_EQ_INT(JSON_PARSE_OK, itojson.JsonParse(v,
		" { "
		"\"n\" : null , "
		"\"f\" : false , "
		"\"t\" : true , "
		"\"i\" : 123 , "
		"\"s\" : \"abc\", "
		"\"a\" : [ 1, 2, 3 ],"
		"\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
		" } "
	));
	EXPECT_EQ_INT(JSON_OBJECT, itojson.JsonGetType(v));
	EXPECT_EQ_SIZE_T(7, itojson.JsonGetObjectSize(v));
	EXPECT_EQ_STRING("n", getPtrTest(itojson.JsonGetObjectKey(v, 0)), itojson.JsonGetObjectKeyLength(v, 0));
	EXPECT_EQ_INT(JSON_NULL, itojson.JsonGetType(&itojson.JsonGetObjectValue(v, 0)));
	EXPECT_EQ_STRING("f", getPtrTest(itojson.JsonGetObjectKey(v, 1)), itojson.JsonGetObjectKeyLength(v, 1));
	EXPECT_EQ_INT(JSON_FALSE, itojson.JsonGetType(&itojson.JsonGetObjectValue(v, 1)));
	EXPECT_EQ_STRING("t", getPtrTest(itojson.JsonGetObjectKey(v, 2)), itojson.JsonGetObjectKeyLength(v, 2));
	EXPECT_EQ_INT(JSON_TRUE, itojson.JsonGetType(&itojson.JsonGetObjectValue(v, 2)));
	EXPECT_EQ_STRING("i", getPtrTest(itojson.JsonGetObjectKey(v, 3)), itojson.JsonGetObjectKeyLength(v, 3));
	EXPECT_EQ_INT(JSON_NUMBER, itojson.JsonGetType(&itojson.JsonGetObjectValue(v, 3)));
	EXPECT_EQ_DOUBLE(123.0, itojson.JsonGetNumber(&itojson.JsonGetObjectValue(v, 3)));
	EXPECT_EQ_STRING("s", getPtrTest(itojson.JsonGetObjectKey(v, 4)), itojson.JsonGetObjectKeyLength(v, 4));
	EXPECT_EQ_INT(JSON_STRING, itojson.JsonGetType(&itojson.JsonGetObjectValue(v, 4)));
	EXPECT_EQ_STRING("abc", getPtrTest(itojson.JsonGetString(&(itojson.JsonGetObjectValue(v, 4)))), itojson.JsonGetObjectValue(v, 4).str.length());
	EXPECT_EQ_STRING("a", getPtrTest(itojson.JsonGetObjectKey(v, 5)), itojson.JsonGetObjectKeyLength(v, 5));
	EXPECT_EQ_INT(JSON_ARRAY, itojson.JsonGetType(&itojson.JsonGetObjectValue(v, 5)));
	EXPECT_EQ_SIZE_T(3, itojson.JsonGetArraySize(&itojson.JsonGetObjectValue(v, 5)));
	for (i = 0; i < 3; i++) {
		JsonValue* e = &itojson.JsonGetArrayElement(&itojson.JsonGetObjectValue(v, 5), i);
		EXPECT_EQ_INT(JSON_NUMBER, itojson.JsonGetType(e));
		EXPECT_EQ_DOUBLE(i + 1.0, itojson.JsonGetNumber(e));
	}
	EXPECT_EQ_STRING("o", getPtrTest(itojson.JsonGetObjectKey(v, 6)), itojson.JsonGetObjectKeyLength(v, 6));
	{
		JsonValue* o = &itojson.JsonGetObjectValue(v, 6);
		EXPECT_EQ_INT(JSON_OBJECT, itojson.JsonGetType(o));
		for (i = 0; i < 3; i++) {
			JsonValue* ov = &itojson.JsonGetObjectValue(o, i);
			EXPECT_TRUE('1' + i == itojson.JsonGetObjectKey(o, i)[0]);
			EXPECT_EQ_SIZE_T(1, itojson.JsonGetObjectKeyLength(o, i));
			EXPECT_EQ_INT(JSON_NUMBER, itojson.JsonGetType(ov));
			EXPECT_EQ_DOUBLE(i + 1.0, itojson.JsonGetNumber(ov));
		}
	}
	delete v;
}

#define TEST_ERROR(error, json)\
    do {\
        JsonValue v;\
		ItoJson itojson;\
        v.type = JSON_FALSE;\
        EXPECT_EQ_INT(error, itojson.JsonParse(&v, json));\
        EXPECT_EQ_INT(JSON_NULL, itojson.JsonGetType(&v));\
    } while(0)

static void test_parse_expect_value() {
	TEST_ERROR(JSON_PARSE_EXPECT_VALUE, "");
	TEST_ERROR(JSON_PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "nul");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "?");

	/* invalid number */
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "+0");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "+1");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "INF");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "inf");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "NAN");
	TEST_ERROR(JSON_PARSE_INVALID_VALUE, "nan");
}

static void test_parse_root_not_singular() {
	TEST_ERROR(JSON_PARSE_ROOT_NOT_SINGULAR, "null x");

	/* invalid number */
	TEST_ERROR(JSON_PARSE_ROOT_NOT_SINGULAR, "0123"); /* after zero should be '.' or nothing */
	TEST_ERROR(JSON_PARSE_ROOT_NOT_SINGULAR, "0x0");
	TEST_ERROR(JSON_PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void test_parse_number_too_big() {
	TEST_ERROR(JSON_PARSE_NUMBER_TOO_BIG, "1e309");
	TEST_ERROR(JSON_PARSE_NUMBER_TOO_BIG, "-1e309");
}

static void test_parse_missing_quotation_mark() {
	TEST_ERROR(JSON_PARSE_MISS_QUOTATION_MARK, "\"");
	TEST_ERROR(JSON_PARSE_MISS_QUOTATION_MARK, "\"abc");
} 

static void test_parse_invalid_string_escape() {
	TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
	TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
	TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
	TEST_ERROR(JSON_PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
	TEST_ERROR(JSON_PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	TEST_ERROR(JSON_PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex() {
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
	TEST_ERROR(JSON_PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse_miss_comma_or_square_bracket() {
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_parse_miss_key() {
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{1:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{true:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{false:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{null:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{[]:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{{}:1,");
	TEST_ERROR(JSON_PARSE_MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {
	TEST_ERROR(JSON_PARSE_MISS_COLON, "{\"a\"}");
	TEST_ERROR(JSON_PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
	TEST_ERROR(JSON_PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

static void test_parse() {
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_parse_number();
	test_parse_string();
	test_parse_array();
	test_parse_object();

	test_parse_expect_value();
	test_parse_invalid_value();
	test_parse_root_not_singular();
	test_parse_number_too_big();

	test_parse_missing_quotation_mark();
	test_parse_invalid_string_escape();
	test_parse_invalid_string_char();

	test_parse_invalid_unicode_hex();
	test_parse_invalid_unicode_surrogate();

	test_parse_miss_comma_or_square_bracket();
	test_parse_miss_key();
	test_parse_miss_colon();
	test_parse_miss_comma_or_curly_bracket();
}

int main() {
	test_parse();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}