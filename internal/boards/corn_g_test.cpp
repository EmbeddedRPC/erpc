#include "corn_g_test.h"

// initialize resultarray elements to 0
_UU8 g_dumpResultArray[DUMP_RESULT_ARRAY_SIZE] = {0};

static void UnityDumpFillResult(const _UU8 result, _U_UINT id, _U_UINT numInByte)
{
    _UU16 dumpResultIndex = id / numInByte;
    if (dumpResultIndex < DUMP_RESULT_ARRAY_SIZE)
    {
        // 8 bit _UU8 variable stores 4 asserts/cases result, from high to low
        // assert/case id -- 0, stored in bits 8~7, margintRight is 6
        // assert/case id -- 1, stored in bits 6~5, margintRight is 4
        // assert/case id -- 2, stored in bits 4~3, marginRight is 2
        // assert/case id -- 3, stored in bits 2~1, marginRight is 0

        _UU8 marginRight = DUMP_RESULT_BIT_NUM * (numInByte - 1) - ((id % numInByte) << 1);

        // get the old result, check whether it is failed, don't set pass/ignore if
        // it has failed.
        if (((g_dumpResultArray[dumpResultIndex] >> marginRight) & DUMP_RESULT_TEST_RESULT_MASK) !=
            DUMP_RESULT_TEST_RESULT_FAIL)
        {
            // clear the old result
            g_dumpResultArray[dumpResultIndex] &= ((_UU8)(~((_UU8)(DUMP_RESULT_TEST_RESULT_MASK << marginRight))));
            // set result
            g_dumpResultArray[dumpResultIndex] |= ((_UU8)(result << marginRight));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

void CornTestingFrameworkPrint::OnTestProgramStart(const ::testing::UnitTest &unit_test)
{
    testCaseFailed = 0;
    indexCase = 0;
}

// TestCase is equal to Unity Test
void CornTestingFrameworkPrint::OnTestCaseStart(const ::testing::TestCase &test_case)
{
    indexCase = 0;
}

// TestStart equal to Unity TestCase
void CornTestingFrameworkPrint::OnTestStart(const ::testing::TestInfo &test_info)
{
    indexCase++;
}

// Called after a failed assertion or a SUCCEED() invocation.
// TestPartResult is equal to Unity TestAssert
void CornTestingFrameworkPrint::OnTestPartResult(const ::testing::TestPartResult &test_part_result)
{
    if (test_part_result.failed())
        testCaseFailed = 1;
}

// Called after a test ends.
void CornTestingFrameworkPrint::OnTestEnd(const ::testing::TestInfo &test_info)
{
    if (testCaseFailed)
        UnityDumpFillResult(DUMP_RESULT_TEST_RESULT_FAIL, (indexCase - 1), CASE_NUM_IN_BYTE);
    else
        UnityDumpFillResult(DUMP_RESULT_TEST_RESULT_PASS, (indexCase - 1), CASE_NUM_IN_BYTE);
    testCaseFailed = 0;
}

void CornTestingFrameworkPrint::OnTestCaseEnd(const ::testing::TestCase &test_case)
{
    UnityDumpFillResult(DUMP_RESULT_TEST_RESULT_SUMMARY, (indexCase), CASE_NUM_IN_BYTE);
}
