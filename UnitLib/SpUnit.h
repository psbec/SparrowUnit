
#ifndef _SpUnit_h
#define _SpUnit_h

#include <iostream>
#include <sstream>
#include <string>

/**
    SparrowUnit is fully compatible with gtest.
*/

namespace SparrowUnit {

#define SpVersionMain       1
#define SpVersionSub        2

/*******************************************************************//**
    Define class
 ***********************************************************************/
class SpUnit;
class SpCaseDB {
public:
    static int  Register(SpUnit* p);
    static void writeData(const std::string &tFileName);
};

class SpEnv {
public:
    virtual void SetUp() {}
    virtual void TearDown() {}
    virtual ~SpEnv() {}
};

class SpUnit : public SpEnv {
public:
    SpUnit();

    const std::string  &getSuiteName() const { return tTestSuiteName; }
    const std::string  &getTestName() const { return tTestCaseName; }
    const std::string  &getTestFile() const { return tTestCaseFile; }
    const std::string  &getFailInfo() const { return tFailInfo; }

    int     getFailCount() const { return FailTestCount; }
    int     getSuccessCount() const { return SuccessTestCount; }

    bool    isMatch(const std::string &filter);

    void    addFailInfo(const std::string &tErr) { tFailInfo += tErr; }
    void    addResult(bool blRet);
    void    showResult() const ;
    int     runTest();

protected:
    std::string tTestSuiteName;
    std::string tTestCaseName;
    std::string tTestCaseFile;

    virtual void TestBody() = 0;

private:
    void    reset();
    int     SuccessTestCount;
    int     FailTestCount;
    std::string tFailInfo;
};

/*******************************************************************//**
    For the sake of gtest
 ***********************************************************************/
namespace testing {
    typedef SpUnit  Test;
    typedef SpEnv   Environment;
    Environment* AddGlobalTestEnvironment(Environment* env);
}

typedef enum {
    ColorType_White=0,
    ColorType_Green,
    ColorType_Red,
    ColorType_Yellow,
    ColorType_Cyan,
}ColorType;

void SpSetConsoleColor(ColorType t);
void SpUnitPrintf(ColorType Color, const char *pFormat, ...);
extern SpUnit *currentUnitCase;

#define _SpErrorLog(...)         SpUnitPrintf(ColorType_Red, __VA_ARGS__)
#define _SpWarnLog(...)          SpUnitPrintf(ColorType_Yellow, __VA_ARGS__)
#define _SpRunLog(...)           SpUnitPrintf(ColorType_Green, __VA_ARGS__)

namespace Compare {
    template <typename T, typename U>
    void Show2Arg(const T &t1, const U &t2, const std::string &expr1, const std::string &expr2, const std::string &content, const std::string &file, int line) {
        SpSetConsoleColor(ColorType_Red);
        std::stringstream tStrStream;
        tStrStream << file << ":" << line << "Failure" << std::endl;
        tStrStream << "Expression expect [ " << expr1 << " ] "<< content << " [ " << expr2 << " ] " << std::endl;
        tStrStream << "Expr left  = [" << t1 << "]" << std::endl;
        tStrStream << "Expr right = [" << t2 << "]" << std::endl;

        if (currentUnitCase)
            currentUnitCase->addFailInfo(tStrStream.str());
        std::cout << tStrStream.str();
        SpSetConsoleColor(ColorType_White);
    }
    std::string ToLower(std::string str);

#define _SpGenCompareCode(FuncName, Expr, Description)\
            template <typename T, typename U>\
            bool FuncName(const T &t1, const U &t2, const std::string &expr1, const std::string &expr2, \
                          const std::string &file, int line) {\
                if(Expr)  return true;\
                Show2Arg(t1, t2, expr1, expr2, Description, file, line);\
                return false;\
            }
    _SpGenCompareCode(CheckEqu, t1==t2, "equal to");
    _SpGenCompareCode(CheckNotEqu, t1!=t2, "not equal to");
    _SpGenCompareCode(CheckGreatThan, t1>t2, "greater than");
    _SpGenCompareCode(CheckGreatEqualThan, t1>=t2, "great or equal than");
    _SpGenCompareCode(CheckLessThan, t1<t2, "less than");
    _SpGenCompareCode(CheckLessEqualThan, t1<=t2, "less or equal than");
}

/*******************************************************************//**
    auxiliary macros
 ***********************************************************************/
#define _SpGetTestCName(test_suite_name, test_name)  CUT##test_suite_name##test_name
#define TEST_FORMAT(test_suite_name, test_name, BaseClass) \
                class _SpGetTestCName(test_suite_name, test_name) : public BaseClass {\
                public:\
                    void TestBody(); \
                    _SpGetTestCName(test_suite_name, test_name)() {\
                        tTestSuiteName = #test_suite_name; \
                        tTestCaseName = #test_name; \
                        tTestCaseFile = __FILE__; \
                    } \
                    static int myTempData; \
                }; \
                int _SpGetTestCName(test_suite_name, test_name)::myTempData = \
                            SpCaseDB::Register(new _SpGetTestCName(test_suite_name, test_name)());\
                void _SpGetTestCName(test_suite_name, test_name)::TestBody()

#define EXPECT_FORMAT(a, b, cond, errorret)     do { \
                if (!cond(a, b, #a, #b, __FILE__, __LINE__)) { \
                    if (currentUnitCase) currentUnitCase->addResult(false); \
                    if (errorret) throw 1; \
                } else { \
                    if (currentUnitCase) currentUnitCase->addResult(true); \
                }}while(0); std::ostringstream()

/*******************************************************************//**
    Define test case
 ***********************************************************************/
#define TEST(test_case_name, test_name) TEST_FORMAT(test_case_name, test_name, SpUnit)
#define TEST_S(test_case_name)          TEST_FORMAT(Default, test_case_name, SpUnit)
#define TEST_F(test_class, test_name)   TEST_FORMAT(test_class, test_name, test_class)

/*******************************************************************//**
  Compare instructions
 ***********************************************************************/
#define EXPECT_TRUE(a)              EXPECT_FORMAT(a, false, Compare::CheckNotEqu, false)
#define EXPECT_FALSE(a)             EXPECT_FORMAT(a, false, Compare::CheckEqu, false)
#define EXPECT_EQ(a, b)         	EXPECT_FORMAT(a, b, Compare::CheckEqu, false)
#define EXPECT_NE(a, b)         	EXPECT_FORMAT(a, b, Compare::CheckNotEqu, false)
#define EXPECT_GT(a, b)         	EXPECT_FORMAT(a, b, Compare::CheckGreatThan, false)
#define EXPECT_GE(a, b)         	EXPECT_FORMAT(a, b, Compare::CheckGreatEqualThan, false)
#define EXPECT_LT(a, b)         	EXPECT_FORMAT(a, b, Compare::CheckLessThan, false)
#define EXPECT_LE(a, b)         	EXPECT_FORMAT(a, b, Compare::CheckLessEqualThan, false)
#define EXPECT_STREQ(a, b)      	EXPECT_EQ(std::string(a), std::string(b))
#define EXPECT_STRNE(a, b)      	EXPECT_NE(std::string(a), std::string(b))
#define EXPECT_STRCASEEQ(a, b)      EXPECT_EQ(Compare::ToLower(std::string(a)), Compare::ToLower(std::string(b)))
#define EXPECT_STRCASENE(a, b)      EXPECT_NE(Compare::ToLower(std::string(a)), Compare::ToLower(std::string(b)))

#define ASSERT_TRUE(a, b)         	EXPECT_FORMAT(a, false, Compare::CheckNotEqu, true)
#define ASSERT_FALSE(a, b)         	EXPECT_FORMAT(a, false, Compare::CheckEqu, true)
#define ASSERT_EQ(a, b)         	EXPECT_FORMAT(a, b, Compare::CheckEqu, true)
#define ASSERT_NE(a, b)         	EXPECT_FORMAT(a, b, Compare::CheckNotEqu, true)
#define ASSERT_GT(a, b)         	EXPECT_FORMAT(a, b, Compare::CheckGreatThan, true)
#define ASSERT_GE(a, b)         	EXPECT_FORMAT(a, b, Compare::CheckGreatEqualThan, true)
#define ASSERT_LT(a, b)         	EXPECT_FORMAT(a, b, Compare::CheckLessThan, true)
#define ASSERT_LE(a, b)         	EXPECT_FORMAT(a, b, Compare::CheckLessEqualThan, true)
#define ASSERT_STREQ(a, b)      	ASSERT_EQ(std::string(a), std::string(b))
#define ASSERT_STRNE(a, b)      	ASSERT_NE(std::string(a), std::string(b))
#define ASSERT_STRCASEEQ(a, b)      ASSERT_EQ(Compare::ToLower(std::string(a)), Compare::ToLower(std::string(b)))
#define ASSERT_STRCASENE(a, b)      ASSERT_NE(Compare::ToLower(std::string(a)), Compare::ToLower(std::string(b)))


/*******************************************************************//**
    Mock start
 ***********************************************************************/
class SpMockImp;
class SpMock {
public:
    SpMock(void *pFunc, bool reset=false);
    SpMock &retAlways(int value);
    SpMock &retOnce(int value);
    SpMock &retTimes(int value, int times);
    SpMock &retRange(int valStart, int valEnd);
    SpMock &repArg(int no, void *prep, int len);
private:
    SpMockImp *pImp;
};

#define SPMOCKER(Func)            SpMock((void *)Func, false)
#define SPMOCKER_RESET(Func)      SpMock((void *)Func, true)

/*******************************************************************//**
    Sparrow Unit interface
 ***********************************************************************/
int SpUnitInit(int argc, char* argv[]);
int SpUnitRunAll(void);

}

#endif
