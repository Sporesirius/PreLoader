/*
 * Copyright 2012 <James.Bottomley@HansenPartnership.com>
 *
 * see COPYING file
 *
 */

#include <efi.h>
#include <efilib.h>

#include <console.h>
#include <errors.h>
#include <guid.h>
#include <security_policy.h>
#include <execute.h>

#ifdef AARCH64
CHAR16* loader = L"YAMS_GRUBAA64.EFI";
#elif ARM
CHAR16* loader = L"YAMS_GRUBARM.EFI";
#elif X86_64
CHAR16* loader = L"YAMS_GRUBX64.EFI";
#else
CHAR16* loader = L"YAMS_GRUBIA32.EFI";
#endif

EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *systab)
{
	EFI_STATUS status;
	UINT8 SecureBoot;
	UINTN DataSize = sizeof(SecureBoot);

	InitializeLib(image, systab);

	console_reset();

	status = RT->GetVariable(L"SecureBoot",
				 &GV_GUID, NULL, &DataSize, &SecureBoot);
	if (status != EFI_SUCCESS) {
		Print(L"Not a Secure Boot Platform %d\n", status);
		goto override;
	}

	if (!SecureBoot) {
		Print(L"Secure Boot Disabled\n");
		goto override;
	}

	status = security_policy_install(security_policy_mok_override,
					 security_policy_mok_allow,
					 security_policy_mok_deny);
	if (status != EFI_SUCCESS) {
		console_error(L"Failed to install override security policy",
			      status);
		/* Don't die, just try to execute without security policy */
		goto override;
	}

	status = execute(image, loader);

	if (status == EFI_SUCCESS)
		goto out;

	if (status != EFI_SECURITY_VIOLATION && status != EFI_ACCESS_DENIED) {
		CHAR16 buf[256];

		StrCpy(buf, L"Failed to start ");
		StrCat(buf, loader);
		console_error(buf, status);

		goto out;
	}

 out:
	status = security_policy_uninstall();
	if (status != EFI_SUCCESS)
		console_error(L"Failed to uninstall security policy.  Platform needs rebooting", status);

	return status;
 override:
	status = execute(image, loader);
	
	if (status != EFI_SUCCESS) {
		CHAR16 buf[256];

		StrCpy(buf, L"Failed to start ");
		StrCat(buf, loader);
		console_error(buf, status);
	}

	return status;
}
