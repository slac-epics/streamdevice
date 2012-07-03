TOP = .

DIRS = configure src
streamApp_DEPEND_DIRS  = src

# EPICS R3.14
include $(TOP)/configure/CONFIG
ifneq ($(words $(CALC) $(SYNAPPS)), 0)
  # with synApps calc module (contains scalcout)
  DIRS += srcSynApps
  srcSynApps_DEPEND_DIRS  = src
  streamApp_DEPEND_DIRS  += srcSynApps
endif

DIRS += streamApp

include $(CONFIG)/RULES_TOP
