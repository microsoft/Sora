TARGETPATH              = $(SORA_ROOT)\target\usr\$(BUILD_ALT_DIR)

USE_MSVCRT              = 1

TARGETLIBS              = $(TARGETLIBS) \
                          $(DDK_LIB_PATH)\ntstrsafe.lib \
                          $(SDK_LIB_PATH)\Kernel32.lib \
                          $(SDK_LIB_PATH)\User32.lib
                          
INCLUDES                = $(SORA_ROOT)\kernel\bb; \
                          $(SORA_ROOT)\kernel\inc; \
                          $(SORA_ROOT)\kernel\core\inc; \
                          $(SORA_ROOT)\kernel\bb\Dot11b; \
                          $(SORA_ROOT)\kernel; \
                          $(SORA_ROOT)\usr\inc; \
                          $(SORA_ROOT)\usr\Oscillograph\inc; \
                          $(DDK_INC_PATH); \
                          $(SDK_INC_PATH)\crt\stl70;

LINKLIBS                = $(LINKLIBS) \
                          $(SORA_ROOT)\target\ke\$(BUILD_ALT_DIR)\*\dot11bbbu.lib       \
                          $(SORA_ROOT)\target\ke\$(BUILD_ALT_DIR)\*\usora.lib           \
                          $(SORA_ROOT)\target\ke\$(BUILD_ALT_DIR)\*\sora_utility.lib    \

USE_NATIVE_EH           = 1

MSC_WARNING_LEVEL       = -W3 -WX
USER_C_FLAGS            = $(USER_C_FLAGS) \
                          /DUSER_MODE
