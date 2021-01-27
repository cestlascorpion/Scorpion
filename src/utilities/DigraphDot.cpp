#include "DigraphDot.h"
#include <regex>
#include <unistd.h>
#include <vector>

using namespace std;

namespace Scorpion {

vector<string> RegexSplit(const string &str, const string &ch) {
    regex re{ch};
    return vector<string>{sregex_token_iterator(str.begin(), str.end(), re, -1), sregex_token_iterator()};
}

string Attribute(const unique_ptr<DotNode> &node) {
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

DigraphDot::DigraphDot() = default;

DigraphDot::~DigraphDot() = default;

int DigraphDot::ReadFile(const string &file) {
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
        auto slice = RegexSplit(line, " ");
        if (slice.size() < 2) {
            printf("wrong format\n");
            continue;
        }
        auto svr = make_unique<DotNode>(slice[0]);
        for (auto i = 1u; i < slice.size(); ++i) {
            if (slice[i] == slice[0]) {
                continue;
            }
            auto node = make_unique<DotNode>(slice[i]);
            _relation[svr->_name].insert(node->_name);
            _nodes.emplace(node->_name, move(node));
        }
        _nodes.emplace(svr->_name, move(svr));
    }

    io.close();
    return 0;
}

int DigraphDot::WriteSVC(const string &svc) {
    ofstream io;
    auto out = svc + ".dot";
    io.open(out);
    if (!io.is_open()) {
        printf("%s cannot open\n", out.c_str());
        return -1;
    }
    set<string> visited;
    io << "digraph{" << endl;
    doWriteLine(svc, visited, io);
    io << endl;
    doWriteNode(visited, io);
    io << "}" << endl;
    io.close();
    if (visited.empty()) {
        unlink(out.c_str());
    }

    static const vector<string> layouts{"dot", "neato", "circo", "fdp"};
    char cmd[128]{0};
    for (const auto &layout : layouts) {
        memset(cmd, 0, sizeof(cmd));
        snprintf(cmd, sizeof(cmd), "dot -Tsvg -K%s -o %s.%s.svg %s", layout.c_str(), svc.c_str(), layout.c_str(),
                 out.c_str());
        int ret = system(cmd);
        if (ret != 0) {
            printf("cmd %s run fail\n", cmd);
            return -1;
        }
    }
    return 0;
}

void DigraphDot::doWriteLine(const string &svc, set<string> &visited, ofstream &io) {
    if (visited.find(svc) != visited.end()) {
        return;
    }
    visited.insert(svc);
    auto iter = _relation.find(svc);
    if (iter == _relation.end()) {
        return;
    }
    for (const auto &node : iter->second) {
        io << "\t" << svc << "->" << node << "[len=3];" << endl;
        doWriteLine(node, visited, io);
    }
}

void DigraphDot::doWriteNode(const set<std::string> &visited, ofstream &io) {
    for (const auto &node : visited) {
        auto iter = _nodes.find(node);
        if (iter == _nodes.end()) {
            printf("unexpected node %s", node.c_str());
            continue;
        }
        io << "\t" << iter->second->_name << "[" << Attribute(iter->second) << "];" << endl;
    }
}
int DigraphDot::WriteALL() {
    for (const auto &item : _relation) {
        if (WriteSVC(item.first) != 0) {
            return -1;
        }
    }
    return 0;
}

} // namespace Scorpion