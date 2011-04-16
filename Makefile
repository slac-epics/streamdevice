TOP = .

DIRS = configure src

include $(TOP)/configure/CONFIG
ifneq ($(words $(CALC) $(SYNAPPS)), 0)
  # with synApps calc module (contains scalcout)
  DIRS += srcSynApps
endif

DIRS += streamApp

include $(CONFIG)/RULES_TOP
