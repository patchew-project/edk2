## @file
#  GNUmakefile for building C utilities.
#
#  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
#  This software and associated documentation (if any) is furnished
#  under a license and may only be used or copied in accordance
#  with the terms of the license. Except as permitted by such
#  license, no part of this software or documentation may be
#  reproduced, stored in a retrieval system, or transmitted in any
#  form or by any means without the express written consent of
#  Intel Corporation.
#
##

MAKEROOT = $(EDK_TOOLS_PATH)/Source/C

APPLICATIONS = \
  FitGen \

SUBDIRS := $(APPLICATIONS)

$(APPLICATIONS): $(MAKEROOT)/bin

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
$(SUBDIRS):
    $(MAKE) -C $@

.PHONY: $(patsubst %,%-clean,$(sort $(SUBDIRS)))
$(patsubst %,%-clean,$(sort $(SUBDIRS))):
    -$(MAKE) -C $(@:-clean=) clean

clean: $(patsubst %,%-clean,$(sort $(SUBDIRS)))

