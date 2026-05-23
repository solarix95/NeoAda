#ifndef NEOADA_ADDON_ADAIOFILE_H
#define NEOADA_ADDON_ADAIOFILE_H

#include <string>

class NdaState;
class NdaVariant;

namespace Nda {

void add_AdaIoFile_symbols(NdaState *state);
bool adaIoReadAllTextFile(NdaState *state, const NdaVariant &file, std::string &text);
bool adaIoWriteTextFile(NdaState *state, const NdaVariant &file, const std::string &text);

}

#endif // NEOADA_ADDON_ADAIOFILE_H
