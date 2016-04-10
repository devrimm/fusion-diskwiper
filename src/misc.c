#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <Wbemcli.h>
#include <sha1.h>

#include "misc.h"

#pragma comment (lib, "wbemuuid.lib")
#pragma comment (lib, "Ws2_32.lib")

char *dupncat(const char *s1, unsigned int n) {
	char *p, *q;

	p = (char*)malloc(n + 1 * sizeof(char));
	q = p;
	for (size_t i = 0; i < n; i++) {
		strcpy(q + i, s1);
	}

	return p;
}

char *dupcat(const char *s1, ...) {
	int len;
	char *p, *q, *sn;
	va_list ap;

	len = strlen(s1);
	va_start(ap, s1);
	while (1) {
		sn = va_arg(ap, char *);
		if (!sn)
			break;
		len += strlen(sn);
	}
	va_end(ap);

	p = (char*)malloc(len + 1 * sizeof(char));
	strcpy(p, s1);
	q = p + strlen(p);

	va_start(ap, s1);
	while (1) {
		sn = va_arg(ap, char *);
		if (!sn)
			break;
		strcpy(q, sn);
		q += strlen(q);
	}
	va_end(ap);

	return p;
}

char** str_split(char* a_str, const char a_delim)
{
	char** result = 0;
	size_t count = 0;
	char* tmp = a_str;
	char* last_comma = 0;
	char delim[2];
	delim[0] = a_delim;
	delim[1] = 0;

	/* Count how many elements will be extracted. */
	while (*tmp)
	{
		if (a_delim == *tmp)
		{
			count++;
			last_comma = tmp;
		}
		tmp++;
	}

	/* Add space for trailing token. */
	count += last_comma < (a_str + strlen(a_str) - 1);

	/* Add space for terminating null string so caller
	knows where the list of returned strings ends. */
	count++;

	result = (char**)malloc(sizeof(char*) * count);

	if (result)
	{
		size_t idx = 0;
		char* token = strtok(a_str, delim);

		while (token)
		{
			assert(idx < count);
			*(result + idx++) = _strdup(token);
			token = strtok(0, delim);
		}
		assert(idx == count - 1);
		*(result + idx) = 0;
	}

	return result;
}

//
//
//void my_sleep(__in LONG dwTime)
//{
//	if (!hScoutMessageWindow)
//		return Sleep(dwTime * 1000);
//
//	LARGE_INTEGER fTime;
//	__int64 qwDueTime = ((dwTime * -1L)) * 10000000;
//	fTime.LowPart = (DWORD)(qwDueTime & 0xFFFFFFFF);
//	fTime.HighPart = (LONG)(qwDueTime >> 32);
//
//	if (!hMsgTimer)
//		hMsgTimer = CreateWaitableTimer(NULL, FALSE, L"MSG_TIMER");
//
//	if (hMsgTimer && SetWaitableTimer(hMsgTimer, &fTime, 0, NULL, NULL, FALSE))
//		WaitForSingleObject(hMsgTimer, INFINITE);
//	else
//		Sleep(dwTime * 1000);
//}

VOID CalculateSHA1(__out PBYTE pSha1Buffer, __in PBYTE pBuffer, __in ULONG uBufflen)
{
	SHA1Context pSha1Context;

	SHA1Reset(&pSha1Context);
	SHA1Input(&pSha1Context, pBuffer, uBufflen);
	SHA1Result(&pSha1Context);

	for (ULONG x = 0; x<5; x++)
		((PULONG)pSha1Buffer)[x] = ntohl(pSha1Context.Message_Digest[x]);
}

BOOL wmiexec_getprop(IWbemServices *pSvc, LPWSTR strQuery, LPWSTR strField, LPVARIANT lpVar)
{
	BOOL bRet = FALSE;
	IEnumWbemClassObject *pEnum;

	BSTR bstrQuery = SysAllocString(strQuery);
	BSTR bstrField = SysAllocString(strField);

	WCHAR strWQL[] = { L'W', L'Q', L'L', L'\0' };
	BSTR bWQL = SysAllocString(strWQL);

	HRESULT hr = pSvc->lpVtbl->ExecQuery(pSvc, bWQL, bstrQuery, 0, NULL, &pEnum);
	if (hr == S_OK)
	{
		ULONG uRet;
		IWbemClassObject *apObj;
		hr = pEnum->lpVtbl->Next(pEnum, 5000, 1, &apObj, &uRet);
		if (hr == S_OK)
		{
			hr = apObj->lpVtbl->Get(apObj, bstrField, 0, lpVar, NULL, NULL);
			if (hr == WBEM_S_NO_ERROR)
				bRet = TRUE;

			apObj->lpVtbl->Release(apObj);
		}
		pEnum->lpVtbl->Release(pEnum);
	}

	SysFreeString(bstrQuery);
	SysFreeString(bstrField);
	SysFreeString(bWQL);

	return bRet;
}

BOOL wmiexec_searchash(IWbemServices *pSvc, LPWSTR strQuery, LPWSTR strField, LPBYTE pSearchHash, LPVARIANT lpVar)
{
	BOOL bFound = FALSE;
	IEnumWbemClassObject *pEnum;
	WCHAR strWQL[] = { L'W', L'Q', L'L', L'\0' };

	BSTR bWQL = SysAllocString(strWQL);
	BSTR bstrQuery = SysAllocString(strQuery);
	BSTR bstrField = SysAllocString(strField);

	HRESULT hr = pSvc->lpVtbl->ExecQuery(pSvc,bWQL, bstrQuery, 0, NULL, &pEnum);
	if (hr == S_OK)
	{
		ULONG uRet;
		IWbemClassObject *apObj;

		while (pEnum->lpVtbl->Next(pEnum, 5000, 1, &apObj, &uRet) == S_OK)
		{
			hr = apObj->lpVtbl->Get(apObj,bstrField, 0, lpVar, NULL, NULL);
			if (hr != WBEM_S_NO_ERROR || lpVar->vt != VT_BSTR)
				continue;

			BYTE pSha1Buffer[20];
			CalculateSHA1(pSha1Buffer, (LPBYTE)lpVar->bstrVal, 21 * sizeof(WCHAR));
			if (!memcmp(pSha1Buffer, pSearchHash, 20))
				bFound = TRUE;

			apObj->lpVtbl->Release(apObj);
		}

		pEnum->lpVtbl->Release(pEnum);
	}

	SysFreeString(bstrQuery);
	SysFreeString(bstrField);
	SysFreeString(bWQL);

	return bFound;
}