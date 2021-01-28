#include "DigraphDot.h"

#include <regex>
#include <unistd.h>
#include <vector>

using namespace std;

namespace Scorpion {

vector<string> StringSplit(const string &str, const string &ch) {
    regex re{ch};
    return vector<string>{sregex_token_iterator(str.begin(), str.end(), re, -1), sregex_token_iterator()};
}

string AttrString(const unique_ptr<DotNode> &node) {
    if (node->_attr.empty()) {
        return "";
    }

    string res;
    for (const auto &item : node->_attr) {
        res += item.first + "=" + item.second;
        res += ",";
    }
    res.resize(res.size() - 1);
    return res;
}

DotNode::DotNode(const string &name)
    : _name(name)
    , _label(name) {
    _attr["style"] = "filled";
    _attr["shape"] = "box";
}

DigraphDot::DigraphDot(const vector<string> &layouts, const string &format)
    : _layouts(layouts)
    , _format(format) {}

DigraphDot::~DigraphDot() = default;

int DigraphDot::ReadFile(const string &file, const HandleNode &handleNode) {
    if (file.empty()) {
        printf("empty file\n");
        return -1;
    }

    ifstream io;
    io.open(file);
    if (!io.is_open()) {
        printf("%s cannot open\n", file.c_str());
        return -1;
    }

    string line;
    while (getline(io, line)) {
        auto slice = StringSplit(line, " ");
        if (slice.size() < 2) {
            printf("wrong format\n");
            continue;
        }
        auto svr = make_unique<DotNode>(slice[0]);
        DefaultNode()(svr.get()); // set label and attr
        for (auto i = 1u; i < slice.size(); ++i) {
            if (slice[i] == slice[0]) {
                continue;
            }
            auto node = make_unique<DotNode>(slice[i]);
            handleNode(node.get()); // set label and attr
            _relation[svr->_label].insert(node->_label);
            _nodes.emplace(node->_label, move(node));
        }
        _nodes.emplace(svr->_label, move(svr));
    }

    io.close();
    return 0;
}

int DigraphDot::WriteSVC(const string &svc, const HandleLine &handleLine) {
    ofstream io;
    auto out = svc + ".dot";
    io.open(out);
    if (!io.is_open()) {
        printf("%s cannot open\n", out.c_str());
        return -1;
    }
    set<string> visited;
    io << "digraph{" << endl;
    doWriteLine(svc, visited, io, handleLine);
    io << endl;
    doWriteNode(visited, io);
    io << "}" << endl;
    io.close();
    if (visited.empty()) {
        unlink(out.c_str());
    }

    char cmd[128]{0};
    for (const auto &layout : _layouts) {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "dot -T%s -K%s -o %s.%s.%s %s", _format.c_str(), layout.c_str(), svc.c_str(),
                 layout.c_str(), _format.c_str(), out.c_str());
        int ret = system(cmd);
        if (ret != 0) {
            printf("cmd %s run fail\n", cmd);
            return -1;
        }
    }
    return 0;
}

void DigraphDot::doWriteLine(const string &svc, set<string> &visited, ofstream &io, const HandleLine &handleLine) {
    if (visited.find(svc) != visited.end()) {
        return;
    }
    visited.insert(svc);
    auto iter = _relation.find(svc);
    if (iter == _relation.end()) {
        return;
    }
    for (const auto &node : iter->second) {
        io << "\t\"" << svc << "\"->\"" << node << "\"" << handleLine(_nodes[svc].get(), _nodes[node].get()) << endl;
        doWriteLine(node, visited, io, handleLine);
    }
}

void DigraphDot::doWriteNode(const set<string> &visited, ofstream &io) {
    for (const auto &node : visited) {
        auto iter = _nodes.find(node);
        if (iter == _nodes.end()) {
            continue;
        }
        io << "\t\"" << iter->second->_label << "\"[" << AttrString(iter->second) << "];" << endl;
    }
}
int DigraphDot::WriteALL(const HandleLine &handleLine) {
    for (const auto &pair : _relation) {
        int ret = WriteSVC(pair.first, handleLine);
        if (ret != 0) {
            printf("write svr %s fail\n", pair.first.c_str());
            return -1;
        }
    }
    return 0;
}

} // namespace Scorpion