# args
STATIC_CPP := 0
# used clang?
CL  	 := 0
# build with -m32?
M32      := 0
# build for out min size
MINS     := 0
ifeq ($(OS),Windows_NT) # mingw?
  CC    := gcc
endif


HDP_PATH := ../HDiffPatch
HDP_OBJ := \
	$(HDP_PATH)/libHDiffPatch/HDiff/private_diff/libdivsufsort/divsufsort.o \
    $(HDP_PATH)/libHDiffPatch/HPatch/patch.o \
    $(HDP_PATH)/file_for_patch.o

TINY_OBJ := \
    decompress/tuz_dec.o \
	compress/tuz_enc.o \
	compress/tuz_enc_private/tuz_enc_clip.o \
	compress/tuz_enc_private/tuz_enc_code.o \
	compress/tuz_enc_private/tuz_enc_match.o \
	compress/tuz_enc_private/tuz_sstring.o \
	$(HDP_OBJ)

DEF_FLAGS := \
    -O3 -DNDEBUG -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

ifeq ($(M32),0)
else
  DEF_FLAGS += -m32
endif
ifeq ($(MINS),0)
else
  DEF_FLAGS += \
    -s \
    -Wno-error=format-security \
    -fvisibility=hidden  \
    -ffunction-sections -fdata-sections \
    -ffat-lto-objects -flto
  CXXFLAGS += -fvisibility-inlines-hidden
endif

TINY_LINK  := 
ifeq ($(M32),0)
else
  TINY_LINK += -m32
endif
ifeq ($(MINS),0)
else
  TINY_LINK += -Wl,--gc-sections,--as-needed
endif
ifeq ($(CL),1)
  CXX := clang++
  CC  := clang
endif
ifeq ($(STATIC_CPP),0)
  TINY_LINK += -lstdc++
else
  TINY_LINK += -static-libstdc++
endif

CFLAGS   += $(DEF_FLAGS) 
CXXFLAGS += $(DEF_FLAGS)

.PHONY: all install clean

all: libtinyuz.a tinyuz mostlyclean

libtinyuz.a: $(TINY_OBJ)
	$(AR) rcs $@ $^

tinyuz: libtinyuz.a
	$(CXX) tinyuz_demo.cpp libtinyuz.a $(CXXFLAGS) $(TINY_LINK) -o tinyuz

ifeq ($(OS),Windows_NT) # mingw?
  RM := del /Q /F
  DEL_TINY_OBJ := $(subst /,\,$(TINY_OBJ))
else
  RM := rm -f
  DEL_TINY_OBJ := $(TINY_OBJ)
endif

mostlyclean: tinyuz
	$(RM) $(DEL_TINY_OBJ)
clean:
	$(RM) libtinyuz.a tinyuz $(DEL_TINY_OBJ)
