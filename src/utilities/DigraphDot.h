#pragma once

#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace Scorpion {

struct DotNode;

using DotAttr = std::map<std::string, std::string>;
using HandleNode = std::function<void(DotNode *)>;
using HandleLine = std::function<std::string(DotNode *, DotNode *)>;

struct DotNode {
    const std::string _name;
    std::string _label;
    DotAttr _attr;

    explicit DotNode(const std::string &name);
};

struct DefaultNode {
    void operator()(DotNode *node) {
        node->_label = node->_name;
        node->_attr["shape"] = "box";
        node->_attr["style"] = "filled";
        node->_attr["fontsize"] = "18";
    }
};

struct DefaultLine {
    std::string operator()(DotNode *, DotNode *) {
        return R"(["style"="bold","len"="3"])";
    }
};

class DigraphDot {
public:
    DigraphDot(const std::vector<std::string> &layouts, const std::string &format);
    ~DigraphDot();

public:
    int ReadFile(const std::string &file, const HandleNode &handleNode);
    int WriteSVC(const std::string &svc, const HandleLine &handleLine);
    int WriteALL(const HandleLine &handleLine);

private:
    void doWriteLine(const std::string &svc, std::set<std::string> &visited, std::ofstream &io,
                     const HandleLine &handleLine);
    void doWriteNode(const std::set<std::string> &visited, std::ofstream &io);

private:
    const std::vector<std::string> _layouts;
    const std::string _format;
    // key: node->_label value: node_ptr
    std::map<std::string, std::unique_ptr<DotNode>> _nodes;
    // key: node->_label value: key -> {node->_label, ...}
    std::map<std::string, std::set<std::string>> _relation;
};

} // namespace Scorpion
