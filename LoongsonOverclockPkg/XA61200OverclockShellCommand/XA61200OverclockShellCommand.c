#include <Uefi.h>

#include <Library/DebugLib.h>
#include <Library/HiiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiHiiServicesLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Protocol/HiiPackageList.h>
#include <Protocol/ShellDynamicCommand.h>

#include "Overclock.h"

STATIC EFI_HII_HANDLE mXA61200OverclcokShellCommandHiiHandle;

STATIC CONST SHELL_PARAM_ITEM ParamList[] = {
    {L"-p", TypeFlag },
    {L"-vcore", TypeValue},
    {L"-vsa", TypeValue},
    {L"-f", TypeValue},
    {L"-e", TypeValue},
    {L"-ndiv", TypeValue},
    {L"-nmode", TypeValue},
    {L"-htdiv", TypeValue},
    {L"-htmode", TypeValue},
    {NULL , TypeMax}
};

#define APPLICATION_NAME L"overclock"

#define COMMAND_VALUE_LIMIT_NOLIMIT -1
#define COMMAND_VALUE_SET(Package, Name, Value, Min, Max) do { \
    const CHAR16 *ValueStr = ShellCommandLineGetValue(Package, L"-" #Name); \
    UINT16 ValueSet; \
    if (ValueStr != NULL) { \
        if (!StringToUint16(ValueStr, &ValueSet)) { \
            ReturnStatus = SHELL_INVALID_PARAMETER; \
            goto Done; \
        } \
        if ((Min != COMMAND_VALUE_LIMIT_NOLIMIT && ValueSet < Min) || (Max != COMMAND_VALUE_LIMIT_NOLIMIT && ValueSet > Max)) { \
            ShellPrintHiiEx( \
                -1, \
                -1, \
                NULL, \
                STRING_TOKEN(STR_GEN_PARAM_INV), \
                mXA61200OverclcokShellCommandHiiHandle, \
                APPLICATION_NAME, \
                ValueStr); \
            ReturnStatus = SHELL_INVALID_PARAMETER; \
            goto Done; \
        } \
        Value = ValueSet; \
        Status = WriteOverclockConfig(); \
        if (EFI_ERROR(Status)) { \
            ReturnStatus = SHELL_DEVICE_ERROR; \
        } \
    } \
} while(0)

/**
  Check and convert the UINT16 option values of the 'tftp' command

  @param[in]  ValueStr  Value as an Unicode encoded string
  @param[out] Value     UINT16 value

  @return     TRUE      The value was returned.
  @return     FALSE     A parsing error occurred.
**/
STATIC
BOOLEAN
StringToUint16(
    IN CONST CHAR16 *ValueStr,
    OUT UINT16 *Value)
{
    UINTN Val;

    Val = ShellStrToUintn(ValueStr);
    if (Val > MAX_UINT16)
    {
        ShellPrintHiiEx(
            -1,
            -1,
            NULL,
            STRING_TOKEN(STR_GEN_PARAM_INV),
            mXA61200OverclcokShellCommandHiiHandle,
            APPLICATION_NAME,
            ValueStr);
        return FALSE;
    }

    *Value = (UINT16)Val;
    return TRUE;
}

SHELL_STATUS
EFIAPI
OverclockCommandHandler (
    IN EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL *This,
    IN EFI_SYSTEM_TABLE *SystemTable,
    IN EFI_SHELL_PARAMETERS_PROTOCOL *ShellParameters,
    IN EFI_SHELL_PROTOCOL *Shell
) {
    gEfiShellParametersProtocol = ShellParameters;
    gEfiShellProtocol = Shell;
    EFI_STATUS Status = ShellInitialize();
    ASSERT_EFI_ERROR (Status);

    EFI_STATUS ReturnStatus = EFI_SUCCESS;

    LIST_ENTRY *Package;
    CHAR16 *ProblemParam;
    Status = ShellCommandLineParse(ParamList, &Package, &ProblemParam, TRUE);
    if (EFI_ERROR(Status)) {
        if (Status == EFI_VOLUME_CORRUPTED && ProblemParam != NULL) {
            ShellPrintHiiEx(
                -1, -1, NULL, STRING_TOKEN(STR_GEN_PROBLEM),
                mXA61200OverclcokShellCommandHiiHandle,
                APPLICATION_NAME,
                ProblemParam
            );
            FreePool(ProblemParam);
            return SHELL_INVALID_PARAMETER;
        } else {
            ReturnStatus = EFI_LOAD_ERROR;
            ASSERT(FALSE);
        }
        goto Done;
    } else if (Package == NULL) {
        ShellPrintHiiEx(
            -1, -1, NULL, STRING_TOKEN(STR_GEN_TOO_FEW),
            mXA61200OverclcokShellCommandHiiHandle,
            APPLICATION_NAME
        );
        ReturnStatus = EFI_INVALID_PARAMETER;
        goto Done;
    }

    if (ShellCommandLineGetCount(Package) > 2) {
        ShellPrintHiiEx(
            -1, -1, NULL, STRING_TOKEN(STR_GEN_TOO_MANY),
            mXA61200OverclcokShellCommandHiiHandle,
            APPLICATION_NAME
        );
        ReturnStatus = SHELL_INVALID_PARAMETER;
        goto Done;
    }

    COMMAND_VALUE_SET(Package, vcore, ocConfig->VoltageCore, LS_OVERCLOCK_LIMIT_VOLT_MIN, LS_OVERCLOCK_LIMIT_VOLT_MAX);
    COMMAND_VALUE_SET(Package, vsa, ocConfig->VoltageSA, LS_OVERCLOCK_LIMIT_VOLT_MIN, LS_OVERCLOCK_LIMIT_VOLT_MAX);
    COMMAND_VALUE_SET(Package, f, ocConfig->MainFreq, LS_OVERCLOCK_LIMIT_FREQ_MIN, LS_OVERCLOCK_LIMIT_FREQ_MAX);
    COMMAND_VALUE_SET(Package, e, ocConfig->Enable, 0, 1);
    COMMAND_VALUE_SET(Package, ndiv, ocConfig->NodeFreqDiv, 0, 7);
    COMMAND_VALUE_SET(Package, nmode, ocConfig->NodeFreqMode, 0, 1);
    COMMAND_VALUE_SET(Package, htdiv, ocConfig->HTFreqDiv, 0, 7);
    COMMAND_VALUE_SET(Package, htmode, ocConfig->HTFreqMode, 0, 1);

    if (ShellCommandLineGetFlag(Package, L"-p")) {
        int nodeFreq = FREQ_DIV(ocConfig->NodeFreqMode, ocConfig->MainFreq, ocConfig->NodeFreqDiv);
        int htFreq = FREQ_DIV(ocConfig->HTFreqMode, nodeFreq, ocConfig->HTFreqDiv);
        ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN(STR_OVERCLOCK_SETTINGS),
            mXA61200OverclcokShellCommandHiiHandle,
            ocConfig->Enable?L"ENABLE":L"DISABLE",
            ocConfig->MainFreq,
            ocConfig->VoltageCore,
            ocConfig->VoltageSA,
            ocConfig->NodeFreqDiv,
            freqControlModes[ocConfig->NodeFreqMode%2],
            nodeFreq,
            ocConfig->HTFreqDiv,
            freqControlModes[ocConfig->HTFreqMode%2],
            htFreq
        );
    } else {
        ReturnStatus = SHELL_INVALID_PARAMETER;
        goto Done;
    }

Done:
    if (Package != NULL) {
        ShellCommandLineFreeVarList (Package);
    }

    return ReturnStatus;
}

STATIC
CHAR16 *
EFIAPI
OverclockCommandGetHelp(
    IN EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL *This,
    IN CONST CHAR8 *Language
) {
    return HiiGetString(
        mXA61200OverclcokShellCommandHiiHandle,
        STRING_TOKEN(STR_GET_HELP_OVERCLOCK),
        Language
    );
}

STATIC EFI_SHELL_DYNAMIC_COMMAND_PROTOCOL mXA61200OverclockDynamicCommand = {
    APPLICATION_NAME,
    OverclockCommandHandler,
    OverclockCommandGetHelp
};

STATIC
EFI_HII_HANDLE
InitializeHiiPackage (
  EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageList;
  EFI_HII_HANDLE               HiiHandle;

  //
  // Retrieve HII package list from ImageHandle
  //
  Status = gBS->OpenProtocol (
                  ImageHandle,
                  &gEfiHiiPackageListProtocolGuid,
                  (VOID **)&PackageList,
                  ImageHandle,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  //
  // Publish HII package list to HII Database.
  //
  Status = gHiiDatabase->NewPackageList (
                           gHiiDatabase,
                           PackageList,
                           NULL,
                           &HiiHandle
                           );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  return HiiHandle;
}

EFI_STATUS
EFIAPI
XA61200OverclockShellCommandEntryPoint(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
) {
    DebugPrint(EFI_D_INFO, "overclock shell command entrypoint\n");

    EFI_STATUS Status = OverclockModuleInit();
    if (EFI_ERROR(Status)) {
        DebugPrint(EFI_D_ERROR, "oc module init failed: %d\n", Status);
    }

    mXA61200OverclcokShellCommandHiiHandle = InitializeHiiPackage(ImageHandle);
    if (mXA61200OverclcokShellCommandHiiHandle == NULL) {
        return EFI_ABORTED;
    }

    Status = gBS->InstallProtocolInterface(
        &ImageHandle,
        &gEfiShellDynamicCommandProtocolGuid,
        EFI_NATIVE_INTERFACE,
        &mXA61200OverclockDynamicCommand
    );
    ASSERT_EFI_ERROR(Status);
    DebugPrint(EFI_D_INFO, "overclock shell command entrypoint: %d\n", Status);
    return Status;
}

EFI_STATUS
EFIAPI
XA61200OverclockShellCommandUnload(
    IN EFI_HANDLE ImageHandle
) {
    EFI_STATUS Status = gBS->UninstallProtocolInterface(
        ImageHandle,
        &gEfiShellDynamicCommandProtocolGuid,
        &mXA61200OverclockDynamicCommand
    );

    if (EFI_ERROR(Status)) {
        return Status;
    }

    HiiRemovePackages(mXA61200OverclcokShellCommandHiiHandle);
    return EFI_SUCCESS;
}