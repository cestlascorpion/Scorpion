/**
 * Resolving the calling relationship between services.
 * file example:
 *     |---------|
 *     |A B C    | -> means svc.A call svc.B svc.C
 *     |B D      |    use " " to separate services, the first one is the Caller, others are callees 
 *     |D E      |
 *     |---------|
 * which means the service invocation relationship is as follows:
 *     A --> B --> D --> E
 *     |
 *     \ (arrow here!)
 *      C
 */

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

// Caller Node will be handled by DefaultNode()
// Callee Node will be handled by @handleNode, the parameter
// Note: _label must be unique!
struct DefaultNode {
    void operator()(DotNode *node) {
        node->_label = node->_name;
        node->_attr["shape"] = "box";
        node->_attr["style"] = "filled";
        node->_attr["fontsize"] = "18";
    }
};

// Set Line's Attribute, here is an example
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
    // eg: dot neato twopi circo osage fdp sfdp
    const std::vector<std::string> _layouts;
    // eg: svg png jpg 
    const std::string _format;
    // key: node->_label value: node_ptr
    std::map<std::string, std::unique_ptr<DotNode>> _nodes;
    // key: node->_label value: key -> {node->_label, ...}
    std::map<std::string, std::set<std::string>> _relation;
};

} // namespace Scorpion
