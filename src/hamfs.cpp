#include "../include/hamfs.hpp"
#include "../include/errors.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <memory>
#include <array>

namespace fs = std::filesystem;

namespace Hammer {

void registerHamFS(std::shared_ptr<Environment> env) {
    // hamfs_read(path)
    auto fnRead = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeString("");
        std::string path = args[0].toString();
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            throw HammerError(0, 0, "HamFS Error", "Failed to read file: '" + path + "'.",
                "The file does not exist, or you lack read permissions.",
                "Verify that the file path is correct.", "data = hamfs_read(\"file.txt\")");
        }
        std::stringstream ss;
        ss << file.rdbuf();
        return Value::makeString(ss.str());
    };

    // hamfs_write(path, text)
    auto fnWrite = [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) {
            throw HammerError(0, 0, "HamFS Error", "hamfs_write requires 2 arguments: hamfs_write(path, text).",
                "You must specify both the path and the text content to write.",
                "Call with file path and string.", "hamfs_write(\"output.txt\", \"Hello!\")");
        }
        std::string path = args[0].toString();
        std::string content = args[1].toString();
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            throw HammerError(0, 0, "HamFS Error", "Failed to create or open file: '" + path + "'.",
                "Check folder permissions or ensure directory exists.",
                "Make sure the target directory exists.", "hamfs_write(\"log.txt\", \"data\")");
        }
        file << content;
        return Value::makeBoolean(true);
    };

    // hamfs_append(path, text)
    auto fnAppend = [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::makeBoolean(false);
        std::string path = args[0].toString();
        std::string content = args[1].toString();
        std::ofstream file(path, std::ios::app | std::ios::binary);
        if (!file.is_open()) return Value::makeBoolean(false);
        file << content;
        return Value::makeBoolean(true);
    };

    // hamfs_delete(path)
    auto fnDelete = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeBoolean(false);
        std::string path = args[0].toString();
        std::error_code ec;
        return Value::makeBoolean(fs::remove_all(path, ec) > 0);
    };

    // hamfs_make_dir(path)
    auto fnMakeDir = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeBoolean(false);
        std::string path = args[0].toString();
        std::error_code ec;
        return Value::makeBoolean(fs::create_directories(path, ec));
    };

    // hamfs_list(path)
    auto fnList = [](const std::vector<Value>& args) -> Value {
        std::string path = args.empty() ? "." : args[0].toString();
        std::vector<Value> items;
        std::error_code ec;
        if (!fs::exists(path, ec) || !fs::is_directory(path, ec)) {
            return Value::makeList({});
        }
        for (const auto& entry : fs::directory_iterator(path, ec)) {
            items.push_back(Value::makeString(entry.path().filename().string()));
        }
        return Value::makeList(items);
    };

    // hamfs_exists(path)
    auto fnExists = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeBoolean(false);
        std::error_code ec;
        return Value::makeBoolean(fs::exists(args[0].toString(), ec));
    };

    // hamfs_is_folder(path)
    auto fnIsFolder = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeBoolean(false);
        std::error_code ec;
        return Value::makeBoolean(fs::is_directory(args[0].toString(), ec));
    };

    // hamfs_is_file(path)
    auto fnIsFile = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeBoolean(false);
        std::error_code ec;
        return Value::makeBoolean(fs::is_regular_file(args[0].toString(), ec));
    };

    // hamfs_size(path)
    auto fnSize = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeNumber(0);
        std::error_code ec;
        auto sz = fs::file_size(args[0].toString(), ec);
        return Value::makeNumber(ec ? 0 : static_cast<double>(sz));
    };

    // hamfs_copy(src, dst)
    auto fnCopy = [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::makeBoolean(false);
        std::error_code ec;
        fs::copy(args[0].toString(), args[1].toString(), fs::copy_options::recursive | fs::copy_options::overwrite_existing, ec);
        return Value::makeBoolean(!ec);
    };

    // hamfs_move(src, dst)
    auto fnMove = [](const std::vector<Value>& args) -> Value {
        if (args.size() < 2) return Value::makeBoolean(false);
        std::error_code ec;
        fs::rename(args[0].toString(), args[1].toString(), ec);
        return Value::makeBoolean(!ec);
    };

    // hamfs_cwd()
    auto fnCwd = [](const std::vector<Value>&) -> Value {
        std::error_code ec;
        return Value::makeString(fs::current_path(ec).string());
    };

    // hamfs_exec(command)
    auto fnExec = [](const std::vector<Value>& args) -> Value {
        if (args.empty()) return Value::makeMap();
        std::string cmd = args[0].toString();
        std::array<char, 256> buffer;
        std::string output;

        FILE* pipe = _popen(cmd.c_str(), "r");
        if (!pipe) {
            Value m = Value::makeMap();
            (*m.mapVal)["exit_code"] = Value::makeNumber(-1);
            (*m.mapVal)["output"] = Value::makeString("Failed to run command");
            return m;
        }

        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
            output += buffer.data();
        }

        int exitCode = _pclose(pipe);
        Value m = Value::makeMap();
        (*m.mapVal)["exit_code"] = Value::makeNumber(exitCode);
        (*m.mapVal)["output"] = Value::makeString(output);
        return m;
    };

    // Register top-level functions
    env->set("hamfs_read", Value::makeNative(fnRead));
    env->set("hamfs_write", Value::makeNative(fnWrite));
    env->set("hamfs_append", Value::makeNative(fnAppend));
    env->set("hamfs_delete", Value::makeNative(fnDelete));
    env->set("hamfs_remove", Value::makeNative(fnDelete));
    env->set("hamfs_make_dir", Value::makeNative(fnMakeDir));
    env->set("hamfs_create_folder", Value::makeNative(fnMakeDir));
    env->set("hamfs_list", Value::makeNative(fnList));
    env->set("hamfs_exists", Value::makeNative(fnExists));
    env->set("hamfs_is_folder", Value::makeNative(fnIsFolder));
    env->set("hamfs_is_file", Value::makeNative(fnIsFile));
    env->set("hamfs_size", Value::makeNative(fnSize));
    env->set("hamfs_copy", Value::makeNative(fnCopy));
    env->set("hamfs_move", Value::makeNative(fnMove));
    env->set("hamfs_rename", Value::makeNative(fnMove));
    env->set("hamfs_cwd", Value::makeNative(fnCwd));
    env->set("hamfs_exec", Value::makeNative(fnExec));
    env->set("hamfs_run", Value::makeNative(fnExec));

    // Register 'hamfs' map object
    Value hamfsObj = Value::makeMap();
    (*hamfsObj.mapVal)["read"] = Value::makeNative(fnRead);
    (*hamfsObj.mapVal)["write"] = Value::makeNative(fnWrite);
    (*hamfsObj.mapVal)["append"] = Value::makeNative(fnAppend);
    (*hamfsObj.mapVal)["delete"] = Value::makeNative(fnDelete);
    (*hamfsObj.mapVal)["make_dir"] = Value::makeNative(fnMakeDir);
    (*hamfsObj.mapVal)["create_folder"] = Value::makeNative(fnMakeDir);
    (*hamfsObj.mapVal)["list"] = Value::makeNative(fnList);
    (*hamfsObj.mapVal)["exists"] = Value::makeNative(fnExists);
    (*hamfsObj.mapVal)["is_folder"] = Value::makeNative(fnIsFolder);
    (*hamfsObj.mapVal)["is_file"] = Value::makeNative(fnIsFile);
    (*hamfsObj.mapVal)["size"] = Value::makeNative(fnSize);
    (*hamfsObj.mapVal)["copy"] = Value::makeNative(fnCopy);
    (*hamfsObj.mapVal)["move"] = Value::makeNative(fnMove);
    (*hamfsObj.mapVal)["cwd"] = Value::makeNative(fnCwd);
    (*hamfsObj.mapVal)["exec"] = Value::makeNative(fnExec);
    env->set("hamfs", hamfsObj);
}

} // namespace Hammer
