#include "usereg.h"
#include "usereg_imp.h"
#include <sora.h>
#include <_private_ext_u.h>
#include <__reg_file.h>
#include <_radio_manager.h>

CSoraPHYMem* 
CSoraPHYMem::AllocSoraPHYMem() {

	return new CSoraPHYMemImp();
}

void 
CSoraPHYMem::FreeSoraPHYMem(
	CSoraPHYMem* spm) {

	delete (CSoraPHYMemImp*)spm;
}

void 
CSoraRCBMem::FreeSoraRCBMem(
	CSoraRCBMem* srm) {

	delete (CSoraRCBMemImp*)srm;
}

void 
CSoraRadio::FreeSoraRadio(
	CSoraRadio* sr) {

	delete (CSoraRadioImp*)sr;
}

CSoraRegister* 
CSoraRegister::AllocSoraRegister() {

	return new CSoraRegisterImp();
}

void 
CSoraRegister::FreeSoraRegister(
	CSoraRegister* sr) {

	delete (CSoraRegisterImp*)sr;
}
