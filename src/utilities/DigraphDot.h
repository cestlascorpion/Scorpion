#pragma once

#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>

namespace Scorpion {

struct DotNode {
    std::string _name;
    std::map<std::string, std::string> _attr;

    explicit DotNode(std::string name)
        : _name(std::move(name)) {
        _attr["style"] = "filled";
        _attr["shape"] = "box";
        _attr["mode"] = "circuit";
    }
};

class DigraphDot {
public:
    DigraphDot();
    ~DigraphDot();

public:
    int ReadFile(const std::string &file);
    int WriteSVC(const std::string &svc);
    int WriteALL();

private:
    void doWriteLine(const std::string &svc, std::set<std::string> &visited, std::ofstream &io);
    void doWriteNode(const std::set<std::string> &visited, std::ofstream &io);

private:
    std::map<std::string, std::unique_ptr<DotNode>> _nodes;
    std::map<std::string, std::set<std::string>> _relation;
};

} // namespace Scorpion
