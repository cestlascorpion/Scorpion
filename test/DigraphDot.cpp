#include "DigraphDot.h"

using namespace std;
using namespace Scorpion;

int main() {
    DigraphDot dot;
    dot.ReadFile("relation.txt");
    dot.WriteALL();
}