
#include <stdio.h>
#include "SpUnit.h"

using namespace SparrowUnit;

int Add(int a, int b)
{
    return a+b;
}

int Sub(int a, int b)
{
    printf("call a-b\n");
    return a-b;
}

const char *pGlbStr = "Hanxiangguo";

TEST(NormalTest, test_add_function)
{
    EXPECT_EQ(2, Add(1,1));
    EXPECT_NE(0, Add(10,10));
}

TEST(NormalTest, test_good)
{
    EXPECT_EQ(2, Add(1,1));
    EXPECT_NE(1, Add(1,1));

    EXPECT_GT(3, Add(1,1));
    EXPECT_GE(2, Add(1,1));
    EXPECT_GE(2.1, Add(1,1));

    EXPECT_LT(Add(1,1), 3.5);
    EXPECT_LE(Add(1,1), 2);
    EXPECT_LE(Add(1,1), 2.1);

    EXPECT_STREQ(pGlbStr, "Hanxiangguo");
    EXPECT_STRNE(pGlbStr, "Hanxiangguo1");
    EXPECT_STRCASEEQ(pGlbStr, "HANxiangguo");
    EXPECT_STRCASENE(pGlbStr, "HANxiangguo1");
}

TEST(NormalTest, NormalTest_all_fail)
{
    printf("-------------------------\n");
    EXPECT_EQ(3, Add(1,1));
    printf("-------------------------\n");
    EXPECT_STREQ(pGlbStr, "Hanxiangguo!");
    printf("-------------------------\n");
    EXPECT_NE(2, Add(1,1));
    printf("-------------------------\n");
    EXPECT_GT(Add(1,1), 3);
    printf("-------------------------\n");
    EXPECT_LT(Add(1,1), 1.5);
    printf("-------------------------\n");
    EXPECT_GE(Add(1,1), 2.1);
    printf("-------------------------\n");
    EXPECT_LE(Add(1,1), 1.9);
    printf("-------------------------\n");
    printf("-------------------------\n");
}


TEST(AssertTest, AssertTest_fail)
{
    printf("-------------------------\n");
    ASSERT_EQ(3, Add(1,1));
    printf("-------------------------\n");
    EXPECT_STREQ(pGlbStr, "Hanxiangguo!");
}

class SuiteTest : public testing::Test
{
    void SetUp()
    {
        printf("Run case in suite start.\n");
    }
    void TearDown()
    {
        printf("Run case in suite end.\n");
    }
};

TEST_F(SuiteTest, Test_add_function)
{
    EXPECT_EQ(2, Add(1,1));
}

class MyEnvironment : public testing::Environment
{
    void SetUp()
    {
        printf("Global environment start.\n");
    }
    void TearDown()
    {
        printf("Global environment end.\n");
    }
};

int main(int argc, char* argv[])
{
    SpUnitInit(argc, argv);
    testing::AddGlobalTestEnvironment(new MyEnvironment);
    SpUnitRunAll();
    return 0;
}

TEST_S(Mocker_Simple)
{
    printf("------------------------- Mocker test start \n");
    SPMOCKER(Add).retOnce(10);
    EXPECT_EQ(10, Add(1,1));
    EXPECT_EQ(2, Add(1,1));
    SPMOCKER(Add).retAlways(100);
    printf("------------------------- Mocker test End \n");
}

TEST_S(Mocker_Simple_2)
{
    SPMOCKER_RESET(Add);
    EXPECT_EQ(2, Add(1,1));
}
TEST_S(Mocker_Simple_3)
{
    EXPECT_EQ(2, Add(1,1));
}
TEST_S(Mocker)
{
    printf("------------------------- Mocker test start \n");

    SPMOCKER(Add).retOnce(10).retTimes(100, 2).retOnce(11);
    EXPECT_EQ(10, Add(1,1));
    EXPECT_EQ(100, Add(1,1));
    EXPECT_EQ(100, Add(1,1));
    EXPECT_EQ(11, Add(1,1));
    EXPECT_EQ(2, Add(1,1));

    SPMOCKER(Add).retAlways(100);
    for (int i=0; i<10; i++)
    EXPECT_EQ(100, Add(1,1));

    SPMOCKER_RESET(Add);
    printf("------------------------- Mocker test End \n");
}

TEST_S(Mocker_next_case_make_sure_last_mock_reseted)
{
    SPMOCKER(Sub).retAlways(100);
    EXPECT_EQ(2, Add(1,1));
}

TEST_S(SimpleTestAddFuction)
{
    EXPECT_EQ(2, Add(1,1));
}

void RepFunction(int *a, int *b) {
    *a = 100;
    *b = 200;
}

TEST_S(Mocker_Replace_arg_value)
{
    int a, b;
    int c = 101, d=201;

    RepFunction(&a, &b);
    EXPECT_EQ(a, 100);
    EXPECT_EQ(b, 200);

    SPMOCKER(RepFunction).repArg(0, &c, 4).repArg(1, &d, 4);

    RepFunction(&a, &b);
    EXPECT_EQ(a, 101);
    EXPECT_EQ(b, 201);

    RepFunction(&a, &b);
    EXPECT_EQ(a, 100);
    EXPECT_EQ(b, 200);
}
