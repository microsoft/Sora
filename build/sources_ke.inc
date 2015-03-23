TARGETPATH          = $(SORA_ROOT)\target\ke\$(BUILD_ALT_DIR)

TARGETLIBS          = $(TARGETLIBS) \
                      $(DDK_LIB_PATH)\ntstrsafe.lib
                          
INCLUDES            = $(SORA_ROOT)\kernel\inc;                  \
                      $(SORA_ROOT)\kernel\inc\bb;               \
                      $(SORA_ROOT)\kernel\core\inc;             \
                      $(SORA_ROOT)\kernel\brick\inc;            \
                      $(DDK_INC_PATH);

MSC_WARNING_LEVEL   = -W3 -WX

USER_C_FLAGS        = $(USER_C_FLAGS) \
                      /D_BB11B_KERNEL
