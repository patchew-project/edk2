## Introduction
There are situations where a platform may have separately updatable firmware components (e.g. motherboard, BMC, EC, etc.) and in some cases, there may be dependencies among them. For instance, FWx requires FWy to be at least version 2.0 to install. Today, we don’t have a way to express that in our infrastructure. Fmp capsule dependency attempts to add that capability through minor changes in the FMP capsule as well as a minor enhancement to FmpDxe driver and FmpDeviceLib.
Capsule Dependency is an incremental change of FMP capsule (Signed Capsule) to evaluate the capsule’s version dependency requirement is satisfied or not before applying the update.
Full feature is defined in UEFI Spec 2.8.

## Implementation Details
### Updated Capsule Update Work Flow
![Figure 1](https://github.com/tianocore/tianocore.github.io/wiki/images/Fmp-Capsule-Dependency-Capsule-Update-Workflow.JPG)

#### Last Attempt Status Extension
Extend ESRT status information to express if a capsule could not applied because its dependency could not be satisfied.
> `#define LAST_ATTEMPT_STATUS_ERROR_UNSATISFIED_DEPENDENCIES 0x00000008`

#### Dependency Evaluation
Dependency evaluation process is a cross check between the capsule data and all existing FMP protocal instances in system.
- Check 1 : Validate platform exsiting Fmp Images' version to satisfy the dependency expression in capsule image.
- Check 2 : Validate the capsule image version to satify all the platform existing Fmp images' dependency expression.

#### FmpDeviceLib Extension
To support the Dependency Evaluation Check 2, Fmp device must have the capability to save its own dependency expression and provide the dependency expression to Fmp DXE driver.
The parameter `Image` of `FmpDeviceSetImage` and `FmpDeviceGetImage` function is extended to contain the dependency expression op-codes.
![Figure 2](https://github.com/tianocore/tianocore.github.io/wiki/images/Fmp-Capsule-Dependency-FmpDeviceLib-Extension.jpg)
1. `FmpDeviceSetImage` is responsible for retrieving the dependency from the parameter `Image` and saving it to a protected storage.
2. `FmpDeviceGetImage` is responsible for retrieving the dependency from the storage where `FmpDeviceSetImage` saves dependency and combining it with the Fmp Payload Image into one buffer which is returned to the caller. This dependency will be populated into `EFI_FIRMWARE_IMAGE_DESCRIPTOR` and used for Dependency Evaluation Check 2.
3. `FmpDeviceGetAttributes` must set the bit `IMAGE_ATTRIBUTE_DEPENDENCY` to indicate the Fmp device has dependency expression associcated with the Fmp image and supports Fmp Capsule Dependency feature.


## Enable Fmp Capsule Dependency
### How to enable capsule dependency feature for an Fmp Device:
Please refer to the following sample code which uses EFI variable as the storage of Fmp dependency op-codes. Notice: The EFI variable must be locked before EndOfDxe.
If no such implementation, only Dependency Evaluation Check 1 is supported.

#### 1. FmpDeviceSetImage
```c
EFI_STATUS
SaveFmpDependencyToStorage (
  IN  EFI_FIRMWARE_IMAGE_DEP  *Depex,
  IN  UINTN                   DepexSize
  )
{
  EFI_STATUS Status;

  if (DepexSize > 0 && Depex == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Save dependency op-codes to "FmpDepex" variable.
  //
  Status = gRT->SetVariable (
                  L"FmpDepex",
                  &gEfiCallerIdGuid,
                  EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  DepexSize,
                  (VOID *)Depex
                  );

  return Status;
}

EFI_STATUS
EFIAPI
FmpDeviceSetImage (
  IN  CONST VOID                                     *Image,
  IN  UINTN                                          ImageSize,
  IN  CONST VOID                                     *VendorCode,
  IN  EFI_FIRMWARE_MANAGEMENT_UPDATE_IMAGE_PROGRESS  Progress,
  IN  UINT32                                         CapsuleFwVersion,
  OUT CHAR16                                         **AbortReason
  )
{
  EFI_STATUS Status;
  UINTN      FmpDepexSize;
  UINTN      FmpPayloadImageSize;

  Status = FmpDeviceGetSize (&FmpPayloadImageSize);
  if (!EFI_ERROR (FmpPayloadImageSize) && ImageSize >= FmpPayloadImageSize) {
    //
    // Save dependency op-codes.
    //
    FmpDepexSize = ImageSize - FmpPayloadImageSize;
    Status = SaveFmpDependencyToStorage ((EFI_FIRMWARE_IMAGE_DEP *)Image, FmpDepexSize);
  } 

  //
  // Continue to set image...
  //
}
```
#### 2. FmpDeviceGetImage
```c
EFI_FIRMWARE_IMAGE_DEP *
GetFmpDependencyFromStorage (
  OUT UINTN  *DepexSize
  )
{
  EFI_STATUS             Status;
  EFI_FIRMWARE_IMAGE_DEP *Depex;

  Depex = NULL;

  //
  // Get dependency from variable.
  //
  Status = GetVariable2 (
             L"FmpDepex",
             &gEfiCallerIdGuid,
             (VOID **) &Depex,
             DepexSize
             );

  return Depex;
}

EFI_STATUS
EFIAPI
FmpDeviceGetImage (
  IN OUT VOID   *Image,
  IN OUT UINTN  *ImageSize
  )
{
  EFI_FIRMWARE_IMAGE_DEP  *FmpDepex;
  UINTN                   FmpDepexSize;
  UINTN                   FmpPayloadImageSize;

  FmpDepex = GetFmpDependencyFromStorage (&FmpDepexSize);
  Status = FmpDeviceGetSize (&FmpPayloadImageSize);
  if (EFI_ERROR(Status)) {
    return EFI_ABORTED;
  }

  //
  // Make sure the buffer is big enough to hold the device image
  //
  if (*ImageSize < FmpPayloadImageSize + FmpDepexSize) {
    *ImageSize = FmpPayloadImageSize + FmpDepexSize;
    return EFI_BUFFER_TOO_SMALL;
  }

  *ImageSize = FmpPayloadImageSize + FmpDepexSize;
  //
  // Copy the FmpDepex to the buffer
  //
  CopyMem (Image, FmpDepex, FmpDepexSize);
  //
  // Continue to Copy the Fmp Payload image to the buffer...
  //

}
```
#### 3. FmpDeviceGetAttributes
```c
EFI_STATUS
EFIAPI
FmpDeviceGetAttributes (
  IN OUT UINT64  *Supported,
  IN OUT UINT64  *Setting
  )
{
  if (Supported == NULL || Setting == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Set IMAGE_ATTRIBUTE_DEPENDENCY to support capsule dependency.
  //
  *Supported = (IMAGE_ATTRIBUTE_IMAGE_UPDATABLE         |
                IMAGE_ATTRIBUTE_RESET_REQUIRED          |
                IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED |
                IMAGE_ATTRIBUTE_DEPENDENCY              |
                IMAGE_ATTRIBUTE_IN_USE
                );
  *Setting   = (IMAGE_ATTRIBUTE_IMAGE_UPDATABLE         |
                IMAGE_ATTRIBUTE_RESET_REQUIRED          |
                IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED |
                IMAGE_ATTRIBUTE_DEPENDENCY
                );

  return EFI_SUCCESS;
}
```

### How to generate capsule with dependency feature:
#### 1.	Encode
Capsule generate tool supports to encode capsule dependencies through `'-j'`
command with a JSON file, for example, to type following command in command prompt:
`GenerateCapsule.py -e -j Example.json -o Example.cap`
To add dependency feature, a `"Dependencies"` field in JSON file is required.
For example:

```json
{
    "Payloads": [
        {
            "Guid": "d9ca4062-985c-4084-8445-e104691dd66b",
            "FwVersion": "1",
            "LowestSupportedVersion": "1",
            "MonotonicCount": "3",
            "HardwareInstance": "0",
            "UpdateImageIndex": "3",
            "Payload": "Payload1.bin",
            "OpenSslSignerPrivateCertFile": "TestCert.pem",
            "OpenSslOtherPublicCertFile": "TestSub.pub.pem",
            "OpenSslTrustedPublicCertFile": "TestRoot.pub.pem",
            "SigningToolPath": "C:\\OpenSSL",
            "Dependencies": "aa2fd162-59d1-4d73-bd2c-c6f9f353cdda >= 0x00000001 && 58e21611-44c0-44b7-bc43-488f45cd1e97 >= 0x00000002"
        },
        {
            "Guid": "1559cc9e-ee39-4ae7-aa1f-1b84570da3cf",
            "FwVersion": "1",
            "LowestSupportedVersion": "1",
            "MonotonicCount": "2",
            "HardwareInstance": "0",
            "UpdateImageIndex": "1",
            "Payload": "Payload2.bin",
            "SignToolPfxFile": "TestCert.pfx",
            "SigningToolPath": "C:\\SigningTools",
            "Dependencies": "TRUE"
        }
    ]
}
```

The value of `“Dependencies”` field should be C style infix notation expression, the relations between firmware opcodes and expression operators/operands are listed below:

| Fimware Opcode | Infix notation expression | Dependency Binary |
| --- | --- | --- |
| 0x00 (PUSH_GUID) | 03e6ed9c-afd6-4762-9de7-d78da3c7179e | {0x00, {0x03e6ed9c, 0xafd6, 0x4762, {0x9d, 0xe7, 0xd7, 0x8d, 0xa3, 0xc7, 0x17, 0x9e}} |
| 0x01 (PUSH_VERSION) | 0x00000001 | {0x01, 0x00000001} |
| 0x02 (DECLARE_VERSION_NAME) | DECLARE “Fmp Device 1” | {0x02, “Fmp Device 1”} |
| 0x03 (AND) | && | {0x03} |
| 0x04 (OR) | \|\|  | {0x04} |
| 0x05 (NOT) | ~ | {0x05} |
| 0x06 (TRUE) | TRUE | {0x06} |
| 0x07 (FALSE) | FALSE | {0x07} |
| 0x08 (EQ) | == | {0x08} |
| 0x09 (GT) | > | {0x09} |
| 0x0A (GTE) | >= | {0x0A} |
| 0x0B (LT) | < | {0x0B} |
| 0x0C (LTE) | <= | {0x0C} |
| 0x0D (END) |   | {0x0D} |

>Noted that Opcode 0x0D (END) will automatically added after dependency encoding.

The precedence of infix notation expression operators is listed below from high to low, and it’s followed the C language operator precedence.
* () (brackets)
* ~ (NOT)
* \>=, >, <=, < (GTE, GT, LTE, LT)
* == (EQ)
* && (OR)
* || (AND)

>DECLARE \"xxxx\" is acting like comments, wherever it inserted in the infix notation expression, it will be converted as {DECLARE_VERSION_STRING, xxxx}.

>All operators/operands in infix notation expression should split with spaces, except brackets.

Here are some example of dependency entry for all kinds of operators:
```json
"Dependencies": "TRUE"
```
```json
"Dependencies": "aa2fd162-59d1-4d73-bd2c-c6f9f353cdda >= 0x00000001 && 58e21611-44c0-44b7-bc43-488f45cd1e97 < 0x00000002"
```
```json
"Dependencies": "aa2fd162-59d1-4d73-bd2c-c6f9f353cdda >= 0x00000001 || (58e21611-44c0-44b7-bc43-488f45cd1e97 < 0x00000002 && 567e834b-8310-4b33-ac76-967fbe51132c >= 0x00000003)"
```
```json
"Dependencies": "aa2fd162-59d1-4d73-bd2c-c6f9f353cdda >= 0x00000001 DECLARE \"Fmp Device 1\""
```

#### 2.	Decode
Capsule dependency tool supports the dependencies decoding, for example, to type following command in command prompt:
	`GenerateCapsule.py -d Example.cap -o Test`
All Decoded dependency expressions are written to `Test.json` for each payload.

#### 3.	Dump information
Dump info leverages Decode, for example, to type following command in command prompt:
	`GenerateCapsule.py --dump-info Example.cap`
Here is an example for output dump information.
```
--------
EFI_FIRMWARE_IMAGE_AUTHENTICATION.MonotonicCount                = 0000000000000000
EFI_FIRMWARE_IMAGE_AUTHENTICATION.AuthInfo.Hdr.dwLength         = 00000B03
EFI_FIRMWARE_IMAGE_AUTHENTICATION.AuthInfo.Hdr.wRevision        = 0200
EFI_FIRMWARE_IMAGE_AUTHENTICATION.AuthInfo.Hdr.wCertificateType = 0EF1
EFI_FIRMWARE_IMAGE_AUTHENTICATION.AuthInfo.CertType             = 4AAFD29D-68DF-49EE-8AA9-347D375665A7
sizeof (EFI_FIRMWARE_IMAGE_AUTHENTICATION.AuthInfo.CertData)    = 00000AEB
sizeof (Payload)                                                = 00000053
--------
EFI_FIRMWARE_IMAGE_DEP.Dependencies = {
    00, 582DF9AB-E626-42A8-A11C-3FEA098FF3FA,
    01, 0x00000001,
    02, Fmp Device 1,
    0B,
    0D,
}
sizeof (EFI_FIRMWARE_IMAGE_DEP.Dependencies)    = 0000002E
sizeof (Payload)                                = 00000025
--------
FMP_PAYLOAD_HEADER.Signature              = 3153534D (MSS1)
FMP_PAYLOAD_HEADER.HeaderSize             = 00000010
```