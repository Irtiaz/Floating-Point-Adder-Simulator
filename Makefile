# project name and directory
export ROOT        :="$(realpath .)"
export PROJECTNAME := main
export BINNAME     := $(PROJECTNAME).exe

# all directories
export DEBUG_DIR  :=Debug
export RELEASE_DIR:=Release

export SRCDIR    :=src
export INCDIR    :=-isystem include
export LIBDIR    :=-L$(ROOT)/lib
export ASSETS_DIR:=assets
export DLL_DIR   := dll

# to be overrriden
_DEPDIR    :=.dep
_OBJDIR    :=.obj
_BINDIR    :=bin


export dir_guard=mkdir -p $(@D)

# compiler, linker, tar and rm
export CC_cpp  := g++
export CC_c    := gcc
export LD      := g++
export TAR     := tar
export RM      := rm


export LDLIBS  :=


export CSTANDARD  :=-ansi
export CPPSTANDARD:=-std=c++17

# flags
_FLAGS   :=-c -Wall -Wextra -pedantic -Wparentheses -Wformat=2 \
           -Wshadow -Wwrite-strings -Wredundant-decls -Wmissing-include-dirs \
					 -pedantic-errors -Wuninitialized -Wmissing-declarations -Wconversion \
					 -Wdouble-promotion -Wfloat-equal
_CFLAGS  :=$(CSTANDARD) -Wstrict-prototypes -Wnested-externs -Wold-style-definition \
          -Wbad-function-cast -Wno-unknown-pragmas -Wno-unused-function
# GCC warnings that Clang doesn't provide:
ifeq ($(CC),gcc)
	_CFLAGS+=-Wjump-misses-init -Wlogical-op
endif

_CPPFLAGS:=$(CPPSTANDARD) -Wimport -fcheck-new -Wlogical-op \
           -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self \
					 -Wnoexcept -Wold-style-cast -Woverloaded-virtual \
					 -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel \
					 -Wstrict-overflow=5 -Wswitch-default -Wundef -Wwrite-strings \
					 -Waggregate-return -Weffc++ -Winline -Winvalid-pch \
					 -Wnormalized=nfc -Wpadded -Wstack-protector -Wpointer-arith \
					 -Wswitch-enum -Wunsafe-loop-optimizations -Wuseless-cast \
					 -Wzero-as-null-pointer-constant -Wformat-nonliteral \
					 -Wformat-security -Wformat-y2k -Wcast-align -Wcast-qual -Winit-self \
					 -Wlong-long -Wmissing-field-initializers -Wmissing-format-attribute \
					 -Wmissing-noreturn -Wpacked -Wunreachable-code -Wunused \
					 -Wunused-parameter -Wvariadic-macros -fno-gnu-keywords

export DEBUG_OPTIMIZATION    :=-g3 -fno-defer-pop -fno-inline -fno-function-cse \
                               -fno-zero-initialized-in-bss -fno-guess-branch-probability
export DEBUG_OPTIMIZATION_C  :=
export DEBUG_OPTIMIZATION_CPP:=-fno-elide-constructors

export RELEASE_OPTIMIZATION    :=-O3 -fforce-addr -fmerge-all-constants
export RELEASE_OPTIMIZATION_C  :=
export RELEASE_OPTIMIZATION_CPP:=


export LDFLAGS  :=

# defines
_DEFINES   :=
export CDEFINES  :=
export CPPDEFINES:=

export DEBUG_DEFINE  :=-D_DEBUG
export RELEASE_DEFINE:=-D_RELEASE

export rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))
export SRCS_c    :=$(call rwildcard,$(SRCDIR),*.c)
export SRCS_cpp  :=$(call rwildcard,$(SRCDIR),*.cpp)

export COMPILE_c   =$(CC_c) $(DEFINES) $(CDEFINES) $(DEPFLAGS) $(INCDIR) $(FLAGS) $(CFLAGS) -o $@ $<
export COMPILE_cpp =$(CC_cpp) $(DEFINES) $(CPPDEFINES) $(DEPFLAGS) $(INCDIR) $(FLAGS) $(CPPFLAGS) -o $@ $<
export LINK_o      =$(LD) $(LDFLAGS) -o $@ $^ $(LIBDIR) $(LDLIBS)


export COPY_ASSETS = cp $(ASSETS_DIR)/* $(BINDIR)/$(ASSETS_DIR)/ 2>/dev/null || :
export COPY_DLLS   = cp $(DLL_DIR)/* $(BINDIR)/ 2>/dev/null || :
export POSTCOMPILE = mv $(DEPDIR)/$*.Td $(DEPDIR)/$*.d 2>/dev/null || :

export DEPFLAGS  = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

export setModeCall=$(if $(MODE),,$(if $(findstring Debug,$@),\
                   $(call setMode,DEBUG,$(DEBUG_DIR),$(DEBUG_OPTIMIZATION),$(DEBUG_OPTIMIZATION_C),$(DEBUG_OPTIMIZATION_CPP),$(DEBUG_DEFINE)),\
						       $(call setMode,RELEASE,$(RELEASE_DIR),$(RELEASE_OPTIMIZATION),$(RELEASE_OPTIMIZATION_C),$(RELEASE_OPTIMIZATION_CPP),$(RELEASE_DEFINE))))
define setMode
$(eval export MODE:=${1})
$(eval export DEPDIR    :=${2}/$(_DEPDIR))
$(eval export OBJDIR    :=${2}/$(_OBJDIR))
$(eval export BINDIR    :=${2}/$(_BINDIR))
$(eval export FLAGS     :=${3} $(_FLAGS))
$(eval export CFLAGS    :=${4} $(_CFLAGS))
$(eval export CPPFLAGS  :=${5} $(_CPPFLAGS))
$(eval export DEFINES   :=${6} $(_DEFINES))
$(eval export OBJS_c    :=$(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS_c)))
$(eval export DEPS_c    :=$(patsubst $(SRCDIR)/%.c,$(DEPDIR)/%.d,$(SRCS_c)))
$(eval export OBJS_cpp  :=$(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS_cpp)))
$(eval export DEPS_cpp  :=$(patsubst $(SRCDIR)/%.cpp,$(DEPDIR)/%.d,$(SRCS_cpp)))
$(eval export BIN       :=$(BINDIR)/$(BINNAME))
endef


.PHONY: build clean check memwatch lint run
build clean check memwatch lint run:
	$(MAKE) $@Debug


.PHONY:buildDebug buildRelease
buildDebug buildRelease:
	$(setModeCall)
	$(MAKE) $(BIN)


.PHONY: cleanDebug cleanRelease
cleanDebug cleanRelease:
	$(setModeCall)
	$(RM) -r -f $(OBJDIR)/* $(DEPDIR)/*
	touch $(OBJDIR)/.gitkeep
	touch $(DEPDIR)/.gitkeep
	$(RM) -r -f $(BINDIR)/$(BINNAME) $(BINDIR)/$(ASSETS_DIR)/*
	touch $(BINDIR)/.gitkeep
	touch $(BINDIR)/$(ASSETS_DIR)/.gitkeep

.PHONY: checkDebug checkRelease
checkDebug checkRelease:
	cppcheck --force $(INCDIR) $(call rwildcard,$(SRCDIR),*.c *.h *.cpp *.hpp)

.PHONY: memwatchDebug memwatchRelease
memwatchDebug memwatchRelease:
	$(setModeCall)
	drmemory $(BIN)

.PHONY: lintDebug lintRelease
lintDebug lintRelease:
	cpplint $(call rwildcard,$(SRCDIR),*.c *.h *.cpp *.hpp)

.PHONY: runDebug runRelease
runDebug runRelease:
	$(setModeCall)
	$(BIN)

$(BIN): $(OBJS_c) $(OBJS_cpp)
	$(LINK_o)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPDIR)/%.d
	$(dir_guard)
	$(COPY_ASSETS)
	$(COPY_DLLS)
	$(COMPILE_c)
	$(POSTCOMPILE)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(DEPDIR)/%.d
	$(dir_guard)
	$(COPY_ASSETS)
	$(COPY_DLLS)
	$(COMPILE_cpp)
	$(POSTCOMPILE)

.PRECIOUS: $(DEPDIR)/%.d
$(DEPDIR)/%.d: ;
	$(dir_guard)


-include $(DEPS_c)
-include $(DEPS_cpp)


