#include "dot11config.h"
#include "args.h"

BOOLEAN BeginWith(const char *szStr, const char *szWith);
void UnitTestBeginWith()
{
    BOOLEAN ret;
    ret = BeginWith("", "");
    assert(!ret);
    ret = BeginWith("abcd", "");
    assert(!ret);
    ret = BeginWith("a", "abcd");
    assert(!ret);
    ret = BeginWith("abcd", "abcd");
    assert(ret);
}

BOOLEAN ParseHexValue(IN const char *szHex, OUT int *p);
void UnitTestParseHex()
{
    int value;
    BOOLEAN ret;
    ret = ParseHexValue("80000", &value);
    assert(value == 0x80000);
    ret = ParseHexValue("", &value);
    assert(!ret);
    ret = ParseHexValue("fg00", &value);
    assert(!ret);
}

void UnitTest()
{
    UnitTestBeginWith();
    UnitTestParseHex();
}