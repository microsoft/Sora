#include "stdafx.h"
#include "HelperFunc.h"

void FormatCommaString(CString & str, __int64 num)
{
	if (num == 0)
		str.Append(L"0");
	else
	{
		int digitState = 0;

		bool minus = false;
		if (num < 0)
		{
			num = -num;
			minus = true;
		}

		int level = 0;
		while(num >= 1000)
		{
			if (num / 1000 * 1000 == num)
			{
				num /= 1000;
				level++;
			}
			else
				break;
		}
		
		if (minus)
		{
			str.Append(L"-");
		}
		
		CString strDigit;
		if (num >= 1000)
		{
			double numF = (double)num;
			numF /= 1000;
			level++;
			strDigit.Format(L"%.1f", numF);
			str.Append(strDigit);
		}
		else
		{
			strDigit.Format(L"%d", num);
			str.Append(strDigit);
		}

		CString strUnit;
		switch(level)
		{
		case 1:
			strUnit = L"k";
			break;
		case 2:
			strUnit = L"M";
			break;
		case 3:
			strUnit = L"G";
			break;
		default:
			strUnit = L"";
		}

		str.Append(strUnit);
	/*	while(num > 0)
		{
			strDigit.Format(L"%d", num%10);
			str.Append(strDigit);
			num /= 10;
			digitState++;
			if (digitState == 3)
			{
				digitState = 0;
				if (num)
					str.Append(L",");
			}
		}

		if (minus)
		{
			str.Append(L"-");
		}

		for (int i = 0; i < str.GetLength()/2; i++)
		{
			unsigned short c = str[i];
			str.SetAt(i, str[str.GetLength()-i-1]);
			str.SetAt(str.GetLength()-i-1, c);
		}*/
	}
}
