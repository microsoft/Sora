#_LNG=$(LANGUAGE)
#_INX=.
#STAMP=stampinf -f $@ -a $(_BUILDARCH) -k $(KMDF_VERSION_MAJOR).$(KMDF_VERSION_MINOR)
#$(TARGETPATH)\amd64\$(INF_NAME).inf: $(_INX)\$(INF_NAME).inx 
#    copy $(_INX)\$(@B).inx $@
#    $(STAMP)
    
#$(TARGETPATH)\i386\$(INF_NAME).inf: $(_INX)\$(INF_NAME).inx 
#    copy $(_INX)\$(@B).inx $@
#    $(STAMP)

#_LNG=$(LANGUAGE)

_WXS=.


$(TARGETPATH)\i386\$(MSI_NAME).msi: _ui.wixobj _files.wixobj soradk.wixobj
    light.exe _ui.wixobj _files.wixobj soradk.wixobj -o $(MSI_NAME).msi
    copy $(MSI_NAME).msi $(TARGETPATH)\i386\
    
    
soradk.wixobj: soradk.wxs
    copy soradk.wxs $@
    candle.exe $@
    
_files.wixobj: _files.wxs
    copy _files.wxs $@
    candle.exe $@ 

_ui.wixobj: _ui.wxs
    copy _ui.wxs $@
    candle.exe $@ 
    
clean: 
    del *.msi
    del *.wixobj
    del *.wixpdb
    del *.log
    