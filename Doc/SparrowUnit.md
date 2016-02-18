
# SparrowUnit

## Introduction
SparrowUnit is a lightweight C/C++ test framework, fully compatible with Gtest. Combine with Mock function, easily simplify your case writing work.

### Why SparrowUnit

Before I created SparrowUnit, Gtest is my test framework, but one day I found it could not work under C++11, to fix the problem, it cost me couple of hours, finally, I decided to give up Gtest.

Of course, Gtest is a good project, but it’s too heavy, only 20% feature or less is necessary to me. And more, I need Mock function, MockCpp is a good choice, but like Gtest, I only use a little feature of MockCpp.

Finally, I decide to make a lightweight C/C++ test framework, the following conditions must met:

    1. Very small, easily embedded to any project.
    2. User interface compatible with Gtest.
    3. Simple mock function. 

At last, SparrowUnit was born. 

    1. SparrowUnit is very small, only 1 header file and 1 source file, you can embedded it into your project, and also you can build it as a lib.
    2. SparrowUnit is fully compatible with Gtest, all the case that written in Gtest will work under SparrowUnit without any modification. 
    3. SparrowUnit define a Mocker interface, you can mock any function to return any value you like.

### Supported Platform

* Windows (Mingw32)
* Linux

## How to write case

To use SparrowUnit, you must include SpUnit.h in your source file; to use GTest interface, use SparrowUnit namespace, as follows:  

```
#include "SpUnit.h"

using namespace SparrowUnit;
```

### Normal case

Use the TEST() macro to define a test function, The first argument is the name of the test suite, and the second argument is the test's case name within the test suite, both names must be valid C++ identifiers.
```
TEST(TestSuiteName, TestCaseName) {
    // Test body...
}
```

For example, let's take a simple function: 
```
int Add(int a, int b)
{
    return a+b;
}

TEST(NormalTest, test_add_function)
{
    EXPECT_EQ(2, Add(1,1));
    EXPECT_NE(0, Add(10,10));
}
```

Result:
```
[ RUN      ] NormalTest.test_add_function
[----------] Case include 2 test, success 2, fail 0
[       OK ] NormalTest.test_add_function
```

If error occurs, SparrowUnit will tell you where the error found, here is a fail case: 
```
TEST(NormalTest, test_add_function_fail)
{
    EXPECT_EQ(3, Add(1,1));
}
```

Result:
```
[ RUN      ] NormalTest.test_add_function_fail
D:\wk_SVN_Code\GitRepoWork\Project\SparrowUnit\Sample\main.cpp:27 Failure
Expression expect [ 3 ] equal to [ Add(1,1) ]
Expr left  = [3]
Expr right = [2]
[----------] Case include 1 test, success 0, fail 1
[     FAIL ] NormalTest.test_add_function_fail
```

### Case in fixture

If you find yourself writing two or more tests that operate on similar data, you can use a test fixture. It allows you to reuse the same configuration of objects for several different tests. 

Now we uset TEST_F macro instead of TEST, first create a class derived from testing::Test, make this class name as the first argument of TEST_F (the first argument is the name of the test suite), fill the second argument as your case name.

```
class TestFixtureName : public testing::Test {
    void SetUp() { /* initialize code */ }
    void TearDown() { /* clean up code */ }
}
TEST_F(TestFixtureName, TestCaseName) {
    // Test body...
}
```

SparrowUnit will run as follows:

    1. call fixture SetUp() method;
    2. run user test 
    3. call fixtrue TearDown() method.

Here is an example:
```
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
```

Result:
```
[ RUN      ] SuiteTest.Test_add_function
Run case in suite start.
Run case in suite end.
[----------] Case include 1 test, success 1, fail 0
[       OK ] SuiteTest.Test_add_function
```

### Simplify your case definition

If you find TEST and TEST_F are tediously long and insipid, TEST_S is your new choice. 

TEST_S has only 1 argument: your test case name, all case defined in TEST_S will gathered into “Default” suite.
```
TEST_S(TestCaseName) {
    // Test body...
}
```

For example:
```
TEST_S(SimpleTestAddFuction)
{
    EXPECT_EQ(2, Add(1,1));
}
```

Result:
```
[ RUN      ] Default.SimpleTestAddFuction
[----------] Case include 1 test, success 1, fail 0
[       OK ] Default.SimpleTestAddFuction
```

### Test assertions

Like Gtest, SparrowUnit have two type of Verifies, Fatal and Nonfatal. The different between this two assertion is that, when verify fail, Nonfatal will go test next one, but Fatal will return from this test case.

Here is the assertion list:

Nonfatal assertion      | Fatal assertion         | Verifies
------------------      | ---------------         | --------
EXPECT_EQ(a, b)         | ASSERT_EQ(a, b)         | Expect a == b
EXPECT_NE(a, b)         | ASSERT_NE(a, b)         | Expect a != b
EXPECT_GT(a, b)         | ASSERT_GT(a, b)         | Expect a > b
EXPECT_GE(a, b)         | ASSERT_GE(a, b)         | Expect a >= b
EXPECT_LT(a, b)         | ASSERT_LT(a, b)         | Expect a < b
EXPECT_LE(a, b)         | ASSERT_LE(a, b)         | Expect a <= b
EXPECT_STREQ(a, b)      | ASSERT_STREQ(a, b)      | Expect the two C strings have the same content
EXPECT_STRNE(a, b)      | ASSERT_STRNE(a, b)      | Expect the two C strings have different content
EXPECT_STRCASEEQ(a, b)  | ASSERT_STRCASEEQ(a, b)  | Expect the two C strings have the same content, ignoring case
EXPECT_STRCASENE(a, b)  | ASSERT_STRCASENE(a, b)  | Expect the two C strings have different content, ignoring case

## Global environment

If you want to do something before/after all case run, you can all your function in the main program, but it too inflexible, another better way is to use global environment.

To use global environment, you must implement a class derived from testing::Environment, implement SetUp and TearDown method.

For example:
```
class MyEnvironment : public testing::Environment {
    void SetUp() {
        printf("Global environment start.\n");
    }
    void TearDown() {
        printf("Global environment end.\n");
    }
};
```

Use testing::AddGlobalTestEnvironment to register your Environment: 
```
// statements...
testing::AddGlobalTestEnvironment(new MyEnvironment);
```

SparrowUnit will:

    1. Run global environment SetUp method;
    2. Run all cases;
    3. Run global environment TearDown method;

## Mock function

### Why mocking?

When developing a module, it depends on an existing module. But you just want to test your code, and you don't want to bring in all the dependencies of the additional module, because it's too complex and requires a lot of other dependencies. So cut the dependencies chain is a best way.

The traditional way of dealing with this is to stub your dependencies. That is, writing a “fake” implementation of the dependency and link with it. This works well, but with C and C++, you can have only a single stub implementation in the same build. If you want to switch the stub implementation at runtime, you have to fall back to dirty tricks like global variables so that the stub knows how to answer. The only other way would be to have multiple builds, each one with a different stub implementation.

In this situation, automatically generating stubs and/or mocks saves a lot of time. 

### SparrowUnit Mock Method

SparrowUnit got a simple implementation of Mock:

```
SPMOCKER(FunctionName).retAlways(value)
                      .retOnce(value)
                      .retTimes(value, times)
                      .retRange(start, end)
                      .repArg(argno, prepvalue, len);
PMOCKER_RESET(FunctionName);
```

Where **SPMOCKER** used to Mock function, **PMOCKER_RESET** used to unMock function manually.

SparrowUnit have several method for you:

Method                  | Explanation
------                  | -----------
retAlways(value)        | set function alway return *walue*,  when you mock a function with *retAlways*, mock won't reset automatically, you may use **PMOCKER_RESET** to unMock function manually.
retOnce(value)          | set function return *walue* for 1 time, then unMock the function
retTimes(value, times)  | set function return *walue* for N time, then unMock the function
retRange(start, end)    | set function return *start* at first call, then *start+1*, until *end*, then unMock the function
repArg(argno, prepvalue, len) | replace the *argno* 's arg with *prepvalue*, data length is specified by *len*

For example:
```
TEST_S(Mocker_Simple)
{
    printf("------------------------- Mocker test start \n");
    SPMOCKER(Add).retOnce(10);
    EXPECT_EQ(10, Add(1,1));
    EXPECT_EQ(2, Add(1,1));
    printf("------------------------- Mocker test End \n");
}
TEST_S(Mocker_Simple_2)
{
    EXPECT_EQ(2, Add(1,1));
}
```

Result:
```
[ RUN      ] Default.Mocker_Simple
------------------------- Mocker test start
------------------------- Mocker test End
[----------] Case include 2 test, success 2, fail 0
[       OK ] Default.Mocker_Simple

[ RUN      ] Default.Mocker_Simple_2
D:\wk_SVN_Code\GitRepoWork\Project\SparrowUnit\Sample\main.cpp:122 Failure
Expression expect [ 2 ] equal to [ Add(1,1) ]
Expr left  = [2]
Expr right = [100]
[----------] Case include 1 test, success 0, fail 1
[     FAIL ] Default.Mocker_Simple_2
```

You see, in case “Mocker_Simple_2” we got an error, because in case “Mocker_Simple” we use *retAlways* method, and did not call PMOCKER_RESET after run.

Now modify “Mocker_Simple_2” like this:
```
TEST_S(Mocker_Simple_2)
{
    SPMOCKER_RESET(Add);
    EXPECT_EQ(2, Add(1,1));
}
```

Reslut:
```
[ RUN      ] Default.Mocker_Simple_2
[----------] Case include 1 test, success 1, fail 0
[       OK ] Default.Mocker_Simple_2
```

## Writing the main() Function

You can start from this boilerplate:

```
int main(int argc, char* argv[])
{
    SpUnitInit(argc, argv);
    SpUnitRunAll();
    return 0;
}
```

The SpUnitInit function parses the command line for SparrowUnit Test flags, this allows the user to control a test program's behavior via various flags.
You must call this function before calling SpUnitRunAll(), or the flags won't be properly initialized. 

Flag list:
Falg                        | Explanation
------                      | -----------
`--help`                    | Print this help
`--gtest_list_tests`        | Show test case list
`--vague-match=FILTER`      | Run test case that can vague match
`--gtest_output=xml:FILE`   | Write result to xml file

# Copyright 
Copyright (c) 2015-2020 Han.psbec(psbec@126.com), See LICENSE for details.
