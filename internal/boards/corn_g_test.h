#include "gtest.h"

typedef unsigned char _UU8;
typedef unsigned short _UU16;
typedef unsigned int _UU32;
typedef _UU32 _U_UINT;

#define DUMP_RESULT_TEST_RESULT_IGNORE 0x00U
#define DUMP_RESULT_TEST_RESULT_PASS 0x01U
#define DUMP_RESULT_TEST_RESULT_FAIL 0x02U
#define DUMP_RESULT_TEST_RESULT_SUMMARY 0x03U
#define DUMP_RESULT_TEST_RESULT_MASK 0x03U
#define DUMP_RESULT_BIT_NUM 2U
#define CASE_NUM_IN_BYTE 4U
#define DUMP_RESULT_ARRAY_SIZE 50U // default result array size

class CornTestingFrameworkPrint : public ::testing::EmptyTestEventListener
{
    int testCaseFailed;
    int indexCase;

   public:
    virtual void OnTestProgramStart(const ::testing::UnitTest &unit_test);

    // TestCase is equal to Unity Test
    virtual void OnTestCaseStart(const ::testing::TestCase &test_case);

    // TestStart equal to Unity TestCase
    virtual void OnTestStart(const ::testing::TestInfo &test_info);

    // Called after a failed assertion or a SUCCEED() invocation.
    // TestPartResult is equal to Unity TestAssert
    virtual void OnTestPartResult(const ::testing::TestPartResult &test_part_result);

    // Called after a test ends.
    virtual void OnTestEnd(const ::testing::TestInfo &test_info);

    virtual void OnTestCaseEnd(const ::testing::TestCase &test_case);
};
