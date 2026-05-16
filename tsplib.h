#include <string>
#include <unordered_map>
#include <cmath>
#include <fstream>
#include <sstream>

struct Node {
    int id;
    double x, y;
};

class TSPLIBInstance {
public:
    std::string name;
    int dimension = 0;
    std::string edge_weight_type;
    std::unordered_map<int, Node> nodes;

    int distance(int i, int j) const {
        const auto& a = nodes.at(i+1);
        const auto& b = nodes.at(j+1);
        double dx = a.x - b.x;
        double dy = a.y - b.y;
        return static_cast<int>(std::round(std::sqrt(dx*dx + dy*dy)));
    }
};

TSPLIBInstance parse_tsplib(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("No se pudo abrir el archivo");
    }

    TSPLIBInstance instance;
    std::string line;
    bool reading_coords = false;

    while (getline(file, line)) {
        if (line.empty()) continue;

        if (line.find("NAME") == 0) {
            instance.name = line.substr(line.find(":") + 1);
        }
        else if (line.find("DIMENSION") == 0) {
            instance.dimension = stoi(line.substr(line.find(":") + 1));
        }
        else if (line.find("EDGE_WEIGHT_TYPE") == 0) {
            instance.edge_weight_type = line.substr(line.find(":") + 1);
        }
        else if (line.find("NODE_COORD_SECTION") == 0) {
            reading_coords = true;
        }
        else if (line.find("EOF") == 0) {
            break;
        }
        else if (reading_coords) {
            std::istringstream iss(line);
            int id;
            double x, y;
            if (iss >> id >> x >> y) {
                instance.nodes[id] = Node{id, x, y};
            }
        }
    }
    return instance;
}