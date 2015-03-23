#include <Windows.h>
#include <Sddl.h>

static inline BOOL CreateDACLWithAllAccess(SECURITY_ATTRIBUTES * pSA)
{
	char * szSD = "D:"       // Discretionary ACL
		"(A;OICI;GA;;;BG)"     // full control to built-in guests
		"(A;OICI;GA;;;AN)"     // full control to anonymous logon
		"(A;OICI;GA;;;AU)"     // Allow full control to authenticated users
		"(A;OICI;GA;;;BA)";    // Allow full control to administrators

	if (NULL == pSA)
		return FALSE;

	return ConvertStringSecurityDescriptorToSecurityDescriptorA(
		szSD,
		SDDL_REVISION_1,
		&(pSA->lpSecurityDescriptor),
		NULL);
}
