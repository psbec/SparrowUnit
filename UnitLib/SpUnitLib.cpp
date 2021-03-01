
#include <string>
#include <vector>
#include <list>
#include <map>
#include <stdio.h>
#include <time.h>

#ifdef __MINGW32__
#include <direct.h>
#include <windows.h>
#endif

#ifndef __MINGW32__
#include <sys/mman.h>
#include <limits.h>
#endif

#include "SpUnit.h"

///////////////////////////////////////////////////////////////////////////////

using namespace std;

namespace SparrowUnit {

/******************************************************************************
    functions...
******************************************************************************/
void SpOSHookReset();
bool SpOSHookApi(void* ApiFun,void* HookFun);
bool SpOSUnHookApi(void* ApiFun);


/******************************************************************************
    global variable
******************************************************************************/
struct SPUDB {
    std::vector<SpUnit*> cases;
    std::vector<testing::Environment*> env;
};

SPUDB*  spudb = NULL;

/******************************************************************************
    Sparrow DB
******************************************************************************/
int SpCaseDB::Register(SpUnit *p)
{
    if (!spudb)
        spudb = new SPUDB;
    spudb->cases.push_back(p);
    return 0;
}

SpUnit* currentUnitCase=NULL;

namespace Compare {
    std::string ToLower(std::string str)
    {
        for (std::string::iterator it=str.begin(); it!=str.end(); it++)
            if (((*it)>='A') && ((*it)<='Z'))
                (*it) += 'a'-'A';
        return str;
    }
}

static std::string int2String(int value) {
    char abStr[12];
    sprintf(abStr, "%d", value);
    return abStr;
}

static bool writeStringToFile(const std::string &tFile, const std::string &tContent) {
    FILE *fp = fopen(tFile.c_str(), "wb");
    if (!fp)
        return false;

    fwrite(tContent.c_str(), 1, tContent.size(), fp);
    fclose(fp);
    return true;
}


/******************************************************************************
    compatible with gtest
******************************************************************************/
namespace testing {
    Environment* AddGlobalTestEnvironment(Environment* env) {
        spudb->env.push_back(env);
        return env;
    }
}

class CaseStat {
public:
    CaseStat(const std::string &tCase, int runTime, const std::string &tFailMsg) :
            tCase(tCase), tFailMsg(tFailMsg), runTime(runTime) {}

    std::string genXml(const std::string &tSuiteName) {
        std::string tXmlStr  = "        <testcase name='" + tCase + "'";
        tXmlStr += " status='run'";
        tXmlStr += " time='" + int2String(runTime) + "'";
        tXmlStr += " classname='" +tSuiteName + "'";
        if (!tFailMsg.size()) {
            tXmlStr += " />\n";
            return tXmlStr;
        }

        tXmlStr += ">\n";
        tXmlStr += "            <failure message='Failed' type=''>";
        tXmlStr += "<![CDATA[" + tFailMsg + "]]> </failure>\n";
        tXmlStr += "        </testcase>\n";
        return tXmlStr;
    }

    std::string tCase;
    std::string tFailMsg;
    int runTime;
};

class SuiteStat {
public:
    SuiteStat(const std::string &tSuite) :
            tSuiteName(tSuite), failCount(0), timeCost(0) {}

    void add(const std::string &tCase, int runTime, const std::string &tFailMsg) {
        tCaseDB.push_back(new CaseStat(tCase, runTime, tFailMsg));
        timeCost += runTime;
        if (tFailMsg.size())
            failCount++;
    }

    ~SuiteStat() {
        for (size_t i=0, j=tCaseDB.size(); i<j; i++)
            delete tCaseDB[i];
    }

    std::string genXml() {
        std::string tXmlStr  = "    <testsuite name='" + tSuiteName + "'";
        tXmlStr += " tests='" + int2String(tCaseDB.size()) + "'";
        tXmlStr += " failures='" + int2String(failCount) + "'";
        tXmlStr += " time='" + int2String(timeCost) + "'>\n";

        for (size_t i=0, j=tCaseDB.size(); i<j; i++)
            tXmlStr += tCaseDB[i]->genXml(tSuiteName);

        tXmlStr += "    </testsuite>\n";
        return tXmlStr;
    }

private:
    std::string             tSuiteName;
    std::vector<CaseStat*>  tCaseDB;
    int                     failCount;
    int                     timeCost;
};

class SpStat {
public:
    void addStat(const std::string &tSuite, const std::string &tCase, int runTime, const std::string &tFailMsg="");
    bool writeFile(const std::string &tFileName);

    static SpStat &getStat() {
        if (!pStat)
            pStat = new SpStat;
        return *pStat;
    }
    ~SpStat() {
        std::map<std::string,SuiteStat*>::iterator it = gtSuiteDB.begin();
        for (; it!=gtSuiteDB.end(); it++)
            delete it->second;
    }

private:
    SpStat() : caseCount(0), failCount(0), timeCost(0) {}

    int caseCount;
    int failCount;
    int timeCost;
    std::map<std::string,SuiteStat*>    gtSuiteDB;
    static SpStat *pStat;
};

SpStat *SpStat::pStat = NULL;

void SpStat::addStat(const std::string &tSuite, const std::string &tCase, int runTime, const std::string &tFailMsg)
{
    std::map<std::string,SuiteStat*>::iterator it = gtSuiteDB.find(tSuite);
    SuiteStat *pSuite;

    if (it == gtSuiteDB.end()) {
        pSuite = new SuiteStat(tSuite);
        gtSuiteDB.insert(std::pair<std::string,SuiteStat*>(tSuite, pSuite));
    } else
        pSuite = it->second;

    pSuite->add(tCase, runTime, tFailMsg);
    timeCost += runTime;
    caseCount++;
    if (tFailMsg.size())
        failCount++;
}

bool SpStat::writeFile(const std::string &tFileName)
{
    string tXmlStr = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

    tXmlStr += "<testsuites";
    tXmlStr += " tests='" + int2String(caseCount) + "'";
    tXmlStr += " failures='" + int2String(failCount) + "'";
    tXmlStr += " time='" + int2String(timeCost) + "'";
    tXmlStr += " name='AllTests'>\n";

    std::map<std::string,SuiteStat*>::iterator it = gtSuiteDB.begin();
    for (; it!=gtSuiteDB.end(); it++)
        tXmlStr += it->second->genXml();
    tXmlStr += "</testsuites>\n";

    delete pStat;
    pStat = NULL;

    return writeStringToFile(tFileName, tXmlStr);
}

/******************************************************************************
    Sparrow Uint main class
******************************************************************************/
SpUnit::SpUnit() { reset(); }
void SpUnit::reset()
{
     SuccessTestCount = 0;
     FailTestCount = 0;
     tFailInfo.clear();
}

bool SpUnit::isMatch(const std::string &filter)
{
    if (!filter.size())
        return true;
    if(tTestCaseName.find(filter) != string::npos)
        return true;
    return false;
}

int SpUnit::runTest()
{
    _SpRunLog("\n[ RUN      ] %s.%s\n", tTestSuiteName.c_str(), tTestCaseName.c_str());
    currentUnitCase = this;
    time_t tStart = clock();

    try {
        reset();
        SetUp();
        TestBody();
        TearDown();
    }
    catch (...) {
        _SpErrorLog("Catch assert Fail!!\n");
    }

    showResult();
    time_t tCost = clock()-tStart;
    SpUnitPrintf(FailTestCount==0?ColorType_Green:ColorType_Red,
                 "%s %s.%s (%d ms total)\n", FailTestCount==0?"[       OK ]":"[     FAIL ]",
                 tTestSuiteName.c_str(), tTestCaseName.c_str(), tCost);
    SpStat::getStat().addStat(tTestSuiteName, tTestCaseName, tCost, tFailInfo);
    return FailTestCount;
}

void SpUnit::addResult(bool blRet)
{
    blRet?SuccessTestCount++:FailTestCount++;
}

void SpUnit::showResult() const
{
    ColorType tColor = ColorType_Cyan;
    if (FailTestCount)
        tColor = (SuccessTestCount==0)?ColorType_Red:ColorType_Yellow;

    SpUnitPrintf(tColor, "[----------] Case include %d test, success %d, fail %d\n",
            SuccessTestCount+FailTestCount, SuccessTestCount, FailTestCount);
}

/******************************************************************************
    Sparrow console print
******************************************************************************/
void SpSetConsoleColor(ColorType t)
{
#ifdef __MINGW32__
    HANDLE consolehwnd;
    unsigned short wColor = FOREGROUND_RED |
                            FOREGROUND_GREEN |
                            FOREGROUND_BLUE;
    switch (t)
    {
    case ColorType_Green:
        wColor = FOREGROUND_GREEN;
        break;
    case ColorType_Red:
        wColor = FOREGROUND_RED;
        break;
    case ColorType_Yellow:
        wColor = FOREGROUND_RED | FOREGROUND_GREEN;
        break;
    case ColorType_Cyan:
        wColor = FOREGROUND_GREEN | FOREGROUND_BLUE;
        break;
    default:
        break;
    }

    if (t != ColorType_White)
        wColor |= FOREGROUND_INTENSITY;

    consolehwnd = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(consolehwnd, wColor);
#else
    switch (t)
    {
    case ColorType_Green:
        printf("\033[1;32m");
        return;
    case ColorType_Red:
        printf("\033[1;31m");
        return;
    case ColorType_Yellow:
        printf("\033[1;33m");
        return;
    case ColorType_Cyan:
        printf("\033[1;36m");
        return;
    }
    printf("\033[m");  // Resets the terminal to default.
#endif
}

void SpUnitPrintf(ColorType Color, const char *pFormat, ...)
{
    static char  sPrtBuf[65536] = {0};

    register int __retval;
    __builtin_va_list __local_argv; __builtin_va_start( __local_argv, pFormat );
    __retval = vsnprintf( sPrtBuf, 65535, (char *)pFormat, __local_argv );
    sPrtBuf[__retval] = '\0';
    __builtin_va_end( __local_argv );

    if (Color != ColorType_White)
        SpSetConsoleColor(Color);

    printf(sPrtBuf);
    SpSetConsoleColor(ColorType_White);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/******************************************************************************
    Mocker interface
******************************************************************************/
#define _SpMockInvalidSlotId     ((int)-1)
#define _SpMockSlots             32
#define _SpMockRetAlways        -1
#define _SpMockMaxArgs          8

/* x86 instruction */
#define FLATJMPCODE_LENGTH      5
#define FLATJMPCMD_LENGTH       1
#define FLATJMPCMD              0xe9

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

static int MockGetSlot();
static void MockFreeSlot(int slot);
static SpMockImp *SpGetMockImp(void *pFunc);
static int sgMockFlag = 0;

/******************************************************************************
    Mocker implement
******************************************************************************/
class SpMockImp {
public:
    SpMockImp();

    int     getRet();
    void    getArg(void *p0, void *p1, void *p2, void *p3, void *p4, void *p5, void *p6, void *p7);

    void    setSlot(int slot);
    void    hook(void *pHook);
    void    reset();
    void    addRet(int value, int times);
    bool    match(void *pFunc) { return pHookFunc==pFunc; }
    bool    repArg(int no, void *prep, int len);

private:
    bool    hookApi(void* ApiFun, void* HookFun);
    bool    unhookApi(void* ApiFun);

private:
    void*       pHookFunc;
    bool        blAlwaysRet;
    int         retValue;
    int         slot;

    BYTE        ApiBackup[FLATJMPCODE_LENGTH];  /* code backup */
    list<int>   tRetDB;
    void*       apArgRepVal[_SpMockMaxArgs];
    int         adwArgLen[_SpMockMaxArgs];

private:
    static void *sgMockFuncSlot[_SpMockSlots];
};

SpMockImp::SpMockImp() : pHookFunc(NULL) { reset(); }
void SpMockImp::setSlot(int slot) { this->slot = slot; }
void SpMockImp::reset()
{
    if (pHookFunc) {
        unhookApi(pHookFunc);
        MockFreeSlot(slot);
    }

    pHookFunc = NULL;
    blAlwaysRet = false;
    retValue = 0;
    tRetDB.clear();
    memset(apArgRepVal, 0, sizeof(apArgRepVal));
    memset(adwArgLen, 0, sizeof(adwArgLen));
}

void SpMockImp::hook(void *pHook)
{
    pHookFunc = pHook;
    hookApi(pHookFunc, sgMockFuncSlot[slot]);
}

int SpMockImp::getRet()
{
    if (blAlwaysRet)
        return retValue;

    if (!tRetDB.size()) {
        reset();
        return -1;
    }

    int ret = tRetDB.front();
    tRetDB.pop_front();
    if (!tRetDB.size())
        reset();
    return ret;
}

void SpMockImp::getArg(void *p0, void *p1, void *p2, void *p3, void *p4, void *p5, void *p6, void *p7)
{
    #define _SpFSetArg(No)  if (adwArgLen[No])   memcpy(p##No, apArgRepVal[No], adwArgLen[No])

    _SpFSetArg(0);  _SpFSetArg(1);  _SpFSetArg(2);  _SpFSetArg(3);
    _SpFSetArg(4);  _SpFSetArg(5);  _SpFSetArg(6);  _SpFSetArg(7);
}

bool SpMockImp::repArg(int no, void *prep, int len)
{
    if (no>=_SpMockMaxArgs)
        return false;
    apArgRepVal[no] = prep;
    adwArgLen[no] = len;
    return true;
}

void SpMockImp::addRet(int value, int times)
{
    if (times == _SpMockRetAlways) {
        retValue = value;
        blAlwaysRet = true;
        return;
    }

    for (int i=0; i<times; i++)
        tRetDB.push_back(value);
}

bool SpMockImp::hookApi(void* ApiFun, void* HookFun)
{
    #ifdef __MINGW32__
	bool    IsSuccess = FALSE;
	DWORD   TempProtectVar=PAGE_READWRITE;
	MEMORY_BASIC_INFORMATION MemInfo;

	VirtualQuery(ApiFun,&MemInfo,sizeof(MEMORY_BASIC_INFORMATION));
	if(VirtualProtect(MemInfo.BaseAddress,MemInfo.RegionSize,
			TempProtectVar,&MemInfo.Protect)) {
        memcpy(ApiBackup,(const void*)ApiFun, sizeof(ApiBackup));
		*(BYTE*)ApiFun = FLATJMPCMD;
		*(DWORD*)((BYTE*)ApiFun + FLATJMPCMD_LENGTH) = (DWORD)HookFun -(DWORD)ApiFun - FLATJMPCODE_LENGTH;

		VirtualProtect(MemInfo.BaseAddress,MemInfo.RegionSize,
				MemInfo.Protect,&TempProtectVar);
		IsSuccess = TRUE;
	}
	#else
	bool    IsSuccess = FALSE;
	BYTE    *p;
	p = (char *)(((long) ApiFun) & ~(PAGESIZE-1));
	if( mprotect( p, PAGESIZE, PROT_READ|PROT_WRITE|PROT_EXEC ) == 0 )
    {
        memcpy(ApiBackup,(const void*)ApiFun, sizeof(ApiBackup));
		*(BYTE*)ApiFun = FLATJMPCMD;
		*(DWORD*)((BYTE*)ApiFun + FLATJMPCMD_LENGTH) = (DWORD)HookFun -(DWORD)ApiFun - FLATJMPCODE_LENGTH;

        mprotect( p, PAGESIZE, PROT_READ|PROT_EXEC);
        IsSuccess = TRUE;
    }
    else
    {
        perror("Errno mprotect");
    }
	#endif
	return IsSuccess;
}

bool SpMockImp::unhookApi(void* ApiFun)
{
    #ifdef __MINGW32__
	bool    IsSuccess = FALSE;
	DWORD   TempProtectVar=PAGE_READWRITE;
	MEMORY_BASIC_INFORMATION MemInfo;

	VirtualQuery(ApiFun,&MemInfo,sizeof(MEMORY_BASIC_INFORMATION));
	if(VirtualProtect(MemInfo.BaseAddress,MemInfo.RegionSize,
			TempProtectVar,&MemInfo.Protect))	{
	    memcpy((void*)ApiFun, ApiBackup, sizeof(ApiBackup));
		VirtualProtect(MemInfo.BaseAddress,MemInfo.RegionSize,
				MemInfo.Protect,&TempProtectVar);

		IsSuccess = TRUE;
	}
    #else
	bool    IsSuccess = FALSE;
	BYTE    *p;
	p = ((WORDPTR)ApiFun) & ~(PAGESIZE-1);

	if( mprotect( p, PAGESIZE, PROT_READ|PROT_WRITE|PROT_EXEC ) == 0 )  {
		memcpy((void*)ApiFun, ApiBackup, sizeof(ApiBackup));
        mprotect( p, PAGESIZE, PROT_READ|PROT_EXEC);
        IsSuccess = TRUE;
    }
    #endif
	return IsSuccess;
}

/******************************************************************************
    Mocker user interface
******************************************************************************/
SpMock::SpMock(void *pFunc, bool reset)
{
    pImp = SpGetMockImp(pFunc);
    if (!pImp)
        throw 1000;
    if (!reset)
        pImp->hook(pFunc);
}

SpMock &SpMock::retAlways(int value)
{
    pImp->addRet(value, _SpMockRetAlways);
    return *this;
}

SpMock &SpMock::retOnce(int value)
{
    pImp->addRet(value, 1);
    return *this;
}

SpMock &SpMock::retTimes(int value, int times)
{
    pImp->addRet(value, times);
    return *this;
}

SpMock &SpMock::retRange(int valStart, int valEnd)
{
    for (int i=0; i<=valEnd; i++)
        pImp->addRet(i, 1);
    return *this;
}

SpMock &SpMock::repArg(int no, void *prep, int len)
{
    pImp->repArg(no, prep, len);
    return *this;
}

/******************************************************************************
    static Mocker implement
******************************************************************************/
static SpMockImp sMockImp[_SpMockSlots];

#define _SpSemicolon     ;
#define _SpComma         ,
#define _SpFGen32Imp(Macro, Sep) \
				Macro(0) Sep	Macro(1) Sep	Macro(2) Sep	Macro(3) Sep	Macro(4) Sep	Macro(5) Sep	Macro(6) Sep	Macro(7) Sep	Macro(8) Sep	Macro(9) Sep	Macro(10) Sep	Macro(11) Sep	Macro(12) Sep	Macro(13) Sep	Macro(14) Sep	Macro(15) Sep \
                Macro(16) Sep	Macro(17) Sep	Macro(18) Sep	Macro(19) Sep	Macro(20) Sep	Macro(21) Sep	Macro(22) Sep	Macro(23) Sep	Macro(24) Sep	Macro(25) Sep	Macro(26) Sep	Macro(27) Sep	Macro(28) Sep	Macro(29) Sep	Macro(30) Sep	Macro(31) Sep

#define _SpFMockFuncUse(No)         (void*)SpMockFunc##No
#define _SpFMockFuncImp(No)         static int SpMockFunc##No(void *p0, void *p1, void *p2, void *p3, \
                                                           void *p4, void *p5, void *p6, void *p7) { \
                                       sMockImp[No].getArg(p0,p1,p2,p3,p4,p5,p6,p7);\
                                       return sMockImp[No].getRet(); \
                                    }

_SpFGen32Imp(_SpFMockFuncImp, _SpSemicolon)
void *SpMockImp::sgMockFuncSlot[_SpMockSlots] = {
    _SpFGen32Imp(_SpFMockFuncUse, _SpComma)
};

/******************************************************************************
    Mocker slots
******************************************************************************/
static int MockGetSlot()
{
    int iPos = __builtin_ffs(~sgMockFlag)-1;
    if (iPos<0)
        return _SpMockInvalidSlotId;
    sgMockFlag |= 1<<iPos;
    return iPos;
}

static void MockFreeSlot(int slot)
{
    if (slot >= _SpMockSlots) {
        _SpWarnLog("MockFreeSlot: invalid slot %d, shoule between 0 and %d.\n", slot, _SpMockSlots-1);
        return;
    }
    sgMockFlag &= ~(1<<slot);
}

static SpMockImp *SpGetMockImp(void *pFunc)
{
    int slot = _SpMockInvalidSlotId;
    for (int i=0; i<_SpMockSlots; i++)
        if (sMockImp[i].match(pFunc)) {
            slot = i;
            break;
        }

    if (slot==_SpMockInvalidSlotId)
        slot = MockGetSlot();
    if (slot==_SpMockInvalidSlotId)
        return NULL;

    sMockImp[slot].reset();
    sMockImp[slot].setSlot(slot);
    return &sMockImp[slot];
}

void SpMockReset(void *pFunc)
{
    for (int i=0; i<_SpMockSlots; i++)
        if (sMockImp[i].match(pFunc))
            sMockImp[i].reset();
}

void SpMockResetAll()
{
    for (int i=0; i<_SpMockSlots; i++)
        sMockImp[i].reset();
}

/******************************************************************************
    Sparrow User interface
******************************************************************************/
static bool gArgShowHelp = false;
static bool gArgShowCaseList = false;
static std::string  gArgFilter;
static std::string  gArgXmlFile;

static void SpParseArg(int argc, char **argv)
{
#define _SpParseSwitchArg(pStr, arg, set)      {\
                    if (!strcmp(argv[dwCurArg], (char *)(pStr))) { \
                        arg = set; \
                    }}
#define _SpParseComplxArg(pStr, arg, func)      {\
                    if (!strncmp(argv[dwCurArg], (char *)(pStr), strlen(pStr))) { \
                        arg = func(&argv[dwCurArg][strlen(pStr)+1]); \
                    }}

    int dwCurArg = 1;
    while(dwCurArg < argc) {
        _SpParseSwitchArg("--help",                 gArgShowHelp,       true);
        _SpParseSwitchArg("--gtest_list_tests",     gArgShowCaseList,   true);
        _SpParseComplxArg("--vague-match",          gArgFilter,         std::string);
        _SpParseComplxArg("--gtest_output=xml",     gArgXmlFile,        std::string);
        dwCurArg++;
    }
}

static void SpShowHelp()
{
    const char    *pUsage =
    "Help Options:\n"
    "    --help                     Print this help\n"
    "    --gtest_list_tests         Show test case list\n"
    "    --vague-match=FILTER       Run test case that can vague match\n"
    "    --gtest_output=xml:FILE    Write result to xml file\n"
    "\n";
    printf(pUsage);
}
static void SpShowCaseList()
{
    std::vector<SpUnit*>::iterator it = spudb->cases.begin();
    printf("Test case list: \n");
    for (; it!=spudb->cases.end(); it++)
        printf("    %s\n", (*it)->getTestName().c_str());
}

static bool SpPreprocess()
{
#define _SpCheckFlagAndCall(flag, call)   if (flag) {call();return false;}

    _SpCheckFlagAndCall(gArgShowHelp, SpShowHelp);
    _SpCheckFlagAndCall(gArgShowCaseList, SpShowCaseList);

    return true;
}

int SpUnitInit(int argc, char* argv[])
{
    printf("Welcome to Sparrow Unit v%d.%d\n\n", SpVersionMain, SpVersionSub);
    SpParseArg(argc, argv);
    return 0;
}

int SpUnitRunAll(void)
{
    if (!SpPreprocess())
        return 0;

    int iRetFinal = 0;
    size_t i;
    int iCounter = 0;

    for (i=0; i<spudb->env.size(); i++)
        spudb->env[i]->SetUp();

    std::vector<SpUnit*>::iterator it = spudb->cases.begin();
    for (; it!=spudb->cases.end(); it++) {
        if (!(*it)->isMatch(gArgFilter))
            continue;

        iCounter++;
        if ((*it)->runTest())
            iRetFinal++;
    }

    for (i=0; i<spudb->env.size(); i++)
        spudb->env[i]->TearDown();

    ColorType tColor = ColorType_Green;
    if (iRetFinal)
        tColor = (iRetFinal==iCounter)?ColorType_Red:ColorType_Yellow;

    SpUnitPrintf(tColor, "\n[==========] All case %d, success %d, failed %d.\n",
            iCounter, iCounter-iRetFinal, iRetFinal);

    if (gArgXmlFile.size())
        SpStat::getStat().writeFile(gArgXmlFile);

    return iRetFinal;
}

}
