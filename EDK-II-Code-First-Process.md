The EDK II Code First Process is a process by which new features can be added
to UEFI Forum specifications after first having been designed and prototyped
in the open.

This process lets changes and the development of new features happen in the
open, without violating the UEFI forum bylaws which prevent publication of
code for in-draft features/changes.

The process does not in fact change the UEFI bylaws - the change is that the
development (of both specification and code) happens in the open. The resulting
specification update is then submitted to the appropriate working group as an
Engineering Change Request (ECR), and voted on. For the UEFI Forum, this is a
change in workflow, not a change in process.

ECRs are tracked in a UEFI Forum Mantis instance, access restricted to UEFI
Forum Members. TianoCore enables this new process by providing areas on
[TianoCore Bugzilla](https://bugzilla.tianocore.org) to track both specification
updates and reference implementations and new repositories under
[TianoCore GitHub](https://github.com/tianocore) dedicated to hold "code first".

# TianoCore Bugzilla

[TianoCore Bugzilla](bugzilla.tianocore.org) has a product categories for
  * ACPI Specification
  * UEFI Shell Specification 
  * UEFI Platform Initialization Distribution Packaging Specification
  * UEFI Platform Initialization Specification Specification
  * UEFI Specification

Each product category has separate components for
  * Specification
  * Reference implementation

# TianoCore GitHub

Reference implementations targeting the EDK II open source project are held
in branches in the [edk2-staging](https://github.com/tianocore/edk2-staging)
repository.

Additional repositories for implementing reference features in additional open
source projects can be added in the future, as required.

Specification text changes are held within the affected source repository,
using the GitHub flavor of markdown, in a file (or split across several files)
with .md suffix.  Multiple files are required if changes impact multiple
specifications or if the specification is large and is easier to maintain
if the changes are split across multiple files.

* NOTE: This one may break down where we have a specification change affecting
  multiple specifications, but at that point we can track it with multiple 
  TianoCore Bugzilla entries.

## Specification Text Template

The following is a template of specification text changes using the GitHub 
flavor of markdown.  The title and complete description of the specification
changes must be provided in the specification text along with the name and
version of the specification the change applies.  The `Status` of the
specification change always starts in the `Draft` state and is updated based
on feedback from the industry standard forums.  The contents of the specification
text are required to use the
[Creative Commons Attribution 4.0 International](https://spdx.org/licenses/CC-BY-4.0.html)
license using a `SPDX-License-Identifier` statement.

```
# Title: [Must be Filled In]

# Status: [Status]

[Status] must be one of the following:
* Draft
* Submitted to industry standard forum
* Accepted by industry standard forum
* Accepted by industry standard forum with modifications
* Rejected by industry standard forum

# Document: [Title and Version]

Here are some examples of [Title and Version]:
* UEFI Specification Version 2.8
* ACPI Specification Version 6.3
* UEFI Shell Specification Version 2.2
* UEFI Platform Initialization Specification Version 1.7
* UEFI Platform Initialization Distribution Packaging Specification Version 1.1

# License

SPDX-License-Identifier: CC-BY-4.0

# Submitter: [TianoCore Community](https://www.tianocore.org)

# Summary of the change

Required Section

# Benefits of the change

Required Section

# Impact of the change

Required Section

# Detailed description of the change [normative updates]

Required Section

# Special Instructions

Optional Section
```

# Intended workflow

The entity initiating a specification change enters a Bugzilla in the appropriate
area of [TianoCore Bugzilla](bugzilla.tianocore.org). This entry contains the
outline of the change, and the full initial draft text is attached.

If multiple specification updates are interdependent, especially if between
different specifications, then multiple Bugzilla entries should be created.
These Bugzilla entries *must* be linked together with dependencies.

After the Bugzillas have been created, new branches should be created in the
relevant repositories for each Bugzilla.  The branch names must use the following
format where #### is the Bugzilla ID and <Brief Description> is an optional
description of the change.

    BZ####-<Brief Description>

If multiple Bugzilla entries must coexist on a single branch, one of them is
designated the _top-level_, with dependencies properly tracked. That Bugzilla
is be the one naming the branch.

# Source Code

In order to ensure draft code does not accidentally leak into production use,
and to signify when the changeover from draft to final happens, *all* new or
modified[1] identifiers must be prefixed with the relevant BZ#### identifiers.

* [1] Modified in a non-backwards-compatible way. If, for example, a statically
      sized array is grown - this does not need to be prefixed. But a tag in a
      comment would be *highly* recommended.

## File names

New public header files require the prefix (i.e. `Bz1234MyNewProtocol.h`).
Private header files do not need the prefix.

## Contents

The tagging must follow the coding style used by each affected code base.
Examples:

| Released in spec | Draft version in tree | Comment |
| ---              | ---                   | ---     |
| `FunctionName`   | `Bz1234FunctionName`  |         |
| `HEADER_MACRO`   | `BZ1234_HEADER_MACRO` |         |

For data structures or enums, any new or non-backwards-compatible structs or
fields require a prefix. As above, growing an existing array in an existing
struct requires no prefix.

| Released in spec      | Draft version in tree | Comment               |
| ---                   | ---                   | ---                   |
| `typedef SOME_STRUCT` | `BZ1234_SOME_STRUCT`  | Typedef only [2]      |
| `StructField`         | `Bz1234StructField`   | In existing struct[3] |
| `typedef SOME_ENUM`   | `BZ1234_SOME_ENUM`    | Typedef only [2]      |
| `EnumValue`           | `BzEnumValue`         | In existing enum[3]   |

* [2] If the struct or enum definition is separate from the typedef in the public
      header, the definition does not need the prefix.
* [3] Individual fields in newly added struct or enum do not need prefix, the
      struct or enum already carried the prefix.

Variable prefixes indicating global scope ('g' or 'm') go before the BZ prefix.

| Released in spec | Draft version in tree | Comment |
| ---              | ---                   | ---     |
| `gSomeGuid`      | `gBz1234SomeGuid`     |         |

Local identifiers, including module-global ones (m-prefixed) do not require a
BZ prefix.
