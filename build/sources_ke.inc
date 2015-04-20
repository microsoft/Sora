TARGETPATH          = $(SORA_ROOT)\target\$(BUILD_ALT_DIR)

TARGETLIBS          = $(TARGETLIBS) \
                      $(DDK_LIB_PATH)\ntstrsafe.lib
                          
INCLUDES            = $(SORA_ROOT)\src\inc; \
                      $(SORA_ROOT)\inc; \
                      $(SORA_ROOT)\inc\brick; \
                      $(DDK_INC_PATH);

MSC_WARNING_LEVEL   = -W3 -WX

USER_C_FLAGS        = $(USER_C_FLAGS) \
                      /D_BB11B_KERNEL \
					  /GL-
