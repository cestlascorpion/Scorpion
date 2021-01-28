#include "DigraphDot.h"

using namespace std;
using namespace Scorpion;

struct MySQLNode {
    void operator()(DotNode *node) {
        node->_label = "mysql\n" + node->_name;
        node->_attr["shape"] = "ellipse";
        node->_attr["style"] = "solid";
        node->_attr["fontsize"] = "14";
    }
};

struct RedisNode {
    void operator()(DotNode *node) {
        node->_label = "redis\n" + node->_name;
        node->_attr["shape"] = "ellipse";
        node->_attr["style"] = "solid";
        node->_attr["fontsize"] = "14";
    }
};

struct KafkaNode {
    void operator()(DotNode *node) {
        node->_label = "kafka\n" + node->_name;
        node->_attr["shape"] = "ellipse";
        node->_attr["style"] = "solid";
        node->_attr["fontsize"] = "14";
    }
};

struct MongoNode {
    void operator()(DotNode *node) {
        node->_label = "mongo\n" + node->_name;
        node->_attr["shape"] = "ellipse";
        node->_attr["style"] = "solid";
        node->_attr["fontsize"] = "14";
    }
};

struct TiDBNode {
    void operator()(DotNode *node) {
        node->_label = "tidb\n" + node->_name;
        node->_attr["shape"] = "ellipse";
        node->_attr["style"] = "solid";
        node->_attr["fontsize"] = "14";
    }
};

struct SpecialLine {
    string operator()(DotNode *, DotNode *to) {
        if (to->_label.find("mysql") != string::npos) {
            return R"(["color"="green","style"="dashed","len"="3"])";
        }
        if (to->_label.find("redis") != string::npos) {
            return R"(["color"="blue","style"="dashed","len"="3"])";
        }
        if (to->_label.find("kafka") != string::npos) {
            return R"(["color"="red","style"="dashed","len"="3"])";
        }
        if (to->_label.find("mongo") != string::npos) {
            return R"(["color"="brown","style"="dashed","len"="3"])";
        }
        if (to->_label.find("tidb") != string::npos) {
            return R"(["color"="purple","style"="dashed","len"="3"])";
        }
        return R"(["color"="black","style"="bold","splines"="line","len"="3"])";
    }
};

int main() {
    DigraphDot dot(vector<string>{"dot", "fdp"}, "svg");
    dot.ReadFile("relation.txt", DefaultNode());
    dot.ReadFile("cppmysql.txt", MySQLNode());
    dot.ReadFile("cppredis.txt", RedisNode());
    dot.ReadFile("cppkafka.txt", KafkaNode());
    dot.ReadFile("cppmongo.txt", MongoNode());
    dot.ReadFile("cpptidb.txt", TiDBNode());
    // dot.WriteSVC("accountlogic", SpecialLine());
    dot.WriteALL(SpecialLine());
}