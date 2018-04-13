#include "JsParse.h"

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
	shared_ptr<BaseJs> Jsptr;
	JsParse mainJS;
	string str = "null";
	EXPECT_EQ_INT(JSON_PARSE_OK, mainJS.JsonParse(Jsptr,str));
	EXPECT_EQ_INT(JVT_NULL, Jsptr->GetType());
}

static void test_parse_true() {
	shared_ptr<BaseJs> Jsptr;
	JsParse mainJS;
	string str = "true";
	EXPECT_EQ_INT(JSON_PARSE_OK, mainJS.JsonParse(Jsptr, str));
	EXPECT_EQ_INT(JVT_TRUE, Jsptr->GetType());
}

static void test_parse_false() {
	shared_ptr<BaseJs> Jsptr;
	JsParse mainJS;
	string str = "false";
	EXPECT_EQ_INT(JSON_PARSE_OK, mainJS.JsonParse(Jsptr, str));
	EXPECT_EQ_INT(JVT_FALSE, Jsptr->GetType());
}

int main()
{
	test_parse_null();
	test_parse_true();
	test_parse_false();

	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);

	return main_ret;
}