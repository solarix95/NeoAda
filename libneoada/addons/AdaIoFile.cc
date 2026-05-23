#include "AdaIoFile.h"
#include "../state.h"

#include <cassert>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

#define CHECK_INSTANCE_CALL if (args.find("this") == args.end()) return false

namespace {

struct FileHandle {
    std::fstream stream;
};

std::unordered_map<int64_t, std::unique_ptr<FileHandle>>& fileHandles()
{
    static std::unordered_map<int64_t, std::unique_ptr<FileHandle>> handles;
    return handles;
}

int64_t& nextFileHandle()
{
    static int64_t nextHandle = 1;
    return nextHandle;
}

FileHandle *handleById(int64_t id)
{
    auto &handles = fileHandles();
    auto it = handles.find(id);
    if (it == handles.end())
        return nullptr;
    if (!it->second->stream.is_open())
        return nullptr;
    return it->second.get();
}

int64_t openFile(const std::string &path, std::ios_base::openmode mode)
{
    std::unique_ptr<FileHandle> handle(new FileHandle());
    handle->stream.open(path.c_str(), mode);
    if (!handle->stream.is_open())
        return 0;

    int64_t id = nextFileHandle()++;
    fileHandles()[id] = std::move(handle);
    return id;
}

NdaVariant stringValue(NdaState *state, const std::string &value)
{
    NdaVariant ret;
    ret.fromString(state->stringType(), value);
    return ret;
}

NdaVariant naturalValue(NdaState *state, int64_t value)
{
    NdaVariant ret;
    ret.fromNatural(state->naturalType(), value);
    return ret;
}

NdaVariant byteValue(NdaState *state, unsigned char value)
{
    NdaVariant ret;
    ret.fromByte(state->typeByName("byte"), value);
    return ret;
}

NdaVariant boolValue(NdaState *state, bool value)
{
    NdaVariant ret;
    ret.fromBool(state->booleanType(), value);
    return ret;
}

NdaVariant fileKey(NdaState *state, const std::string &key)
{
    return stringValue(state, key);
}

int64_t fileIdFromSelf(NdaState *state, const NdaVariant &self)
{
    if (self.type() != Nda::Dict)
        return 0;

    auto key = fileKey(state, "__handle");
    if (!self.contains(key))
        return 0;

    bool ok;
    auto id = const_cast<NdaVariant&>(self).writeDictAccess(key).toInt64(&ok);
    return ok ? id : 0;
}

void setFileMember(NdaState *state, NdaVariant &file, const std::string &key, const NdaVariant &value)
{
    file.appendToDict(fileKey(state, key), value);
}

void setFileReturn(NdaState *state, NdaVariant &ret, const std::string &runtimeType, const std::string &displayType,
                   int64_t id, const std::string &path, const std::string &mode, const std::string &encoding = "")
{
    ret.initType(state->typeByName(runtimeType));
    setFileMember(state, ret, "__handle", naturalValue(state, id));
    setFileMember(state, ret, "__type", stringValue(state, displayType));
    setFileMember(state, ret, "path", stringValue(state, path));
    setFileMember(state, ret, "mode", stringValue(state, mode));
    if (!encoding.empty())
        setFileMember(state, ret, "encoding", stringValue(state, encoding));
    setFileMember(state, ret, "closed", boolValue(state, id == 0));
}

bool fileExists(const std::string &path)
{
    std::ifstream f(path.c_str(), std::ios::in | std::ios::binary);
    return f.good();
}

bool closeFile(NdaState *state, const Nda::FncValues& args)
{
    CHECK_INSTANCE_CALL;
    auto id = fileIdFromSelf(state, args.at("this"));
    auto &handles = fileHandles();
    auto it = handles.find(id);
    if (it == handles.end())
        return false;
    if (it->second->stream.is_open())
        it->second->stream.close();
    handles.erase(it);

    auto self = args.at("this");
    setFileMember(state, self, "__handle", naturalValue(state, 0));
    setFileMember(state, self, "closed", boolValue(state, true));
    return true;
}

bool flushFile(NdaState *state, const Nda::FncValues& args)
{
    CHECK_INSTANCE_CALL;
    auto *handle = handleById(fileIdFromSelf(state, args.at("this")));
    if (!handle)
        return false;
    handle->stream.flush();
    return handle->stream.good();
}

bool eofFile(NdaState *state, const Nda::FncValues& args, NdaVariant &ret)
{
    CHECK_INSTANCE_CALL;
    auto *handle = handleById(fileIdFromSelf(state, args.at("this")));
    ret.fromBool(state->booleanType(), !handle || handle->stream.eof());
    return true;
}

bool isOpenFile(NdaState *state, const Nda::FncValues& args, NdaVariant &ret)
{
    CHECK_INSTANCE_CALL;
    auto id = fileIdFromSelf(state, args.at("this"));
    ret.fromBool(state->booleanType(), handleById(id) != nullptr);
    return true;
}

NdaVariant readAllBytes(NdaState *state, std::fstream &stream)
{
    NdaVariant ret;
    ret.initType(state->bytesType());

    stream.flush();
    stream.clear();
    stream.seekg(0, std::ios::beg);

    char c;
    while (stream.get(c))
        ret.appendToBytes(byteValue(state, static_cast<unsigned char>(c)));

    return ret;
}

NdaVariant readBlockBytes(NdaState *state, std::fstream &stream, int size)
{
    NdaVariant ret;
    ret.initType(state->bytesType());

    char c;
    while ((size-- > 0) && stream.get(c))
        ret.appendToBytes(byteValue(state, static_cast<unsigned char>(c)));

    return ret;
}


std::string readAllText(std::fstream &stream)
{
    stream.flush();
    stream.clear();
    stream.seekg(0, std::ios::beg);
    return std::string((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
}

void bindCommonFileMethods(NdaState *state, const std::string &typeName)
{
    state->bindFnc(typeName, "isOpen", {}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        return isOpenFile(state, args, ret);
    });

    state->bindPrc(typeName, "close", {}, [state](const Nda::FncValues& args) -> bool {
        return closeFile(state, args);
    });

    state->bindPrc(typeName, "flush", {}, [state](const Nda::FncValues& args) -> bool {
        return flushFile(state, args);
    });

    state->bindFnc(typeName, "eof", {}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        return eofFile(state, args, ret);
    });
}

}

namespace Nda {

bool adaIoReadAllTextFile(NdaState *state, const NdaVariant &file, std::string &text)
{
    assert(state);
    auto *handle = handleById(fileIdFromSelf(state, file));
    if (!handle)
        return false;

    text = readAllText(handle->stream);
    return true;
}

bool adaIoWriteTextFile(NdaState *state, const NdaVariant &file, const std::string &text)
{
    assert(state);
    auto *handle = handleById(fileIdFromSelf(state, file));
    if (!handle)
        return false;

    handle->stream << text;
    return handle->stream.good();
}

void add_AdaIoFile_symbols(NdaState *state)
{
    assert(state);

    state->registerType("File", "dict");
    state->registerType("TextFile", "dict");

    // ------------------ File:Open(path) ---------------------------------------------------------
    state->bindFnc("file", "open", {{"path", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        auto path = args.at("path").toString();
        auto id = openFile(path, std::ios::in | std::ios::out | std::ios::binary);
        setFileReturn(state, ret, "file", "File", id, path, "readwrite");
        return id != 0;
    });

    // ------------------ File:OpenRead(path) -----------------------------------------------------
    state->bindFnc("file", "openRead", {{"path", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        auto path = args.at("path").toString();
        auto id = openFile(path, std::ios::in | std::ios::binary);
        setFileReturn(state, ret, "file", "File", id, path, "read");
        return id != 0;
    });

    // ------------------ File:Create(path) -------------------------------------------------------
    state->bindFnc("file", "create", {{"path", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        auto path = args.at("path").toString();
        auto id = openFile(path, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary);
        setFileReturn(state, ret, "file", "File", id, path, "create");
        return id != 0;
    });

    // ------------------ File:Append(path) -------------------------------------------------------
    state->bindFnc("file", "append", {{"path", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        auto path = args.at("path").toString();
        auto id = openFile(path, std::ios::in | std::ios::out | std::ios::app | std::ios::binary);
        setFileReturn(state, ret, "file", "File", id, path, "append");
        return id != 0;
    });

    // ------------------ File:Exists(path) -------------------------------------------------------
    state->bindFnc("file", "exists", {{"path", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        ret.fromBool(state->booleanType(), fileExists(args.at("path").toString()));
        return true;
    });

    bindCommonFileMethods(state, "file");

    // ------------------ File.Write(data) --------------------------------------------------------
    state->bindPrc("file", "write", {{"data", "bytes", Nda::InMode}}, [state](const Nda::FncValues& args) -> bool {
        CHECK_INSTANCE_CALL;
        auto *handle = handleById(fileIdFromSelf(state, args.at("this")));
        auto data = args.at("data");
        if (!handle || data.type() != Nda::Bytes)
            return false;

        for (int i = 0; i < data.lengthOperator(); ++i) {
            bool ok;
            auto byte = data.readBytesAccess(i).toInt64(&ok);
            if (!ok)
                return false;
            handle->stream.put(static_cast<char>(static_cast<unsigned char>(byte)));
        }
        return handle->stream.good();
    });

    // ------------------ File.ReadAll() ----------------------------------------------------------
    state->bindFnc("file", "readAll", {}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        auto *handle = handleById(fileIdFromSelf(state, args.at("this")));
        if (!handle)
            return false;

        ret = readAllBytes(state, handle->stream);
        return true;
    });

    // ------------------ File.ReadAll() ----------------------------------------------------------
    state->bindFnc("file", "read", {{"blockSize", "natural", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        auto *handle = handleById(fileIdFromSelf(state, args.at("this")));
        auto blockSize = args.at("blockSize");
        if (!handle || blockSize.type() != Nda::Natural)
            return false;

        ret = readBlockBytes(state, handle->stream, blockSize.toInt64());
        return true;
    });

    // ------------------ TextFile:Open(path) -----------------------------------------------------
    state->bindFnc("textfile", "open", {{"path", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        auto path = args.at("path").toString();
        auto id = openFile(path, std::ios::in | std::ios::out);
        setFileReturn(state, ret, "textfile", "TextFile", id, path, "readwrite", "utf-8");
        return id != 0;
    });

    // ------------------ TextFile:OpenRead(path) -------------------------------------------------
    state->bindFnc("textfile", "openRead", {{"path", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        auto path = args.at("path").toString();
        auto id = openFile(path, std::ios::in);
        setFileReturn(state, ret, "textfile", "TextFile", id, path, "read", "utf-8");
        return id != 0;
    });

    // ------------------ TextFile:Create(path) ---------------------------------------------------
    state->bindFnc("textfile", "create", {{"path", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        auto path = args.at("path").toString();
        auto id = openFile(path, std::ios::in | std::ios::out | std::ios::trunc);
        setFileReturn(state, ret, "textfile", "TextFile", id, path, "create", "utf-8");
        return id != 0;
    });

    // ------------------ TextFile:Append(path) ---------------------------------------------------
    state->bindFnc("textfile", "append", {{"path", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        auto path = args.at("path").toString();
        auto id = openFile(path, std::ios::in | std::ios::out | std::ios::app);
        setFileReturn(state, ret, "textfile", "TextFile", id, path, "append", "utf-8");
        return id != 0;
    });

    // ------------------ TextFile:Exists(path) ---------------------------------------------------
    state->bindFnc("textfile", "exists", {{"path", "string", Nda::InMode}}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        ret.fromBool(state->booleanType(), fileExists(args.at("path").toString()));
        return true;
    });

    bindCommonFileMethods(state, "textfile");

    // ------------------ TextFile.Write(s) -------------------------------------------------------
    state->bindPrc("textfile", "write", {{"s", "string", Nda::InMode}}, [state](const Nda::FncValues& args) -> bool {
        CHECK_INSTANCE_CALL;
        auto *handle = handleById(fileIdFromSelf(state, args.at("this")));
        if (!handle)
            return false;
        handle->stream << args.at("s").toString();
        return handle->stream.good();
    });

    // ------------------ TextFile.WriteLine(s) ---------------------------------------------------
    state->bindPrc("textfile", "writeLine", {{"s", "string", Nda::InMode}}, [state](const Nda::FncValues& args) -> bool {
        CHECK_INSTANCE_CALL;
        auto *handle = handleById(fileIdFromSelf(state, args.at("this")));
        if (!handle)
            return false;
        handle->stream << args.at("s").toString() << '\n';
        return handle->stream.good();
    });

    // ------------------ TextFile.ReadAll() ------------------------------------------------------
    state->bindFnc("textfile", "readAll", {}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        auto *handle = handleById(fileIdFromSelf(state, args.at("this")));
        if (!handle)
            return false;

        ret.fromString(state->stringType(), readAllText(handle->stream));
        return true;
    });

    // ------------------ TextFile.ReadLine() -----------------------------------------------------
    state->bindFnc("textfile", "readLine", {}, [state](const Nda::FncValues& args, NdaVariant &ret) -> bool {
        CHECK_INSTANCE_CALL;
        auto *handle = handleById(fileIdFromSelf(state, args.at("this")));
        if (!handle)
            return false;

        std::string line;
        if (!std::getline(handle->stream, line))
            line.clear();
        ret.fromString(state->stringType(), line);
        return true;
    });
}

}
