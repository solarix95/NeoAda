#ifndef NEOADA_ADDON_ADATEXTENCODING_H
#define NEOADA_ADDON_ADATEXTENCODING_H

#include <string>

class NdaState;
class NdaVariant;

namespace Nda {

void add_AdaTextEncoding_symbols(NdaState *state);
bool encodeTextBytes(NdaState *state, const std::string &text, const std::string &encoding, NdaVariant &ret);
bool decodeTextBytes(const NdaVariant &bytes, const std::string &encoding, std::string &ret);

}

#endif // NEOADA_ADDON_ADATEXTENCODING_H
