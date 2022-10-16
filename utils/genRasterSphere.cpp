#include <iostream>
#include <vector>

using namespace std;

struct v3_t {
    double x;
    double y;
    double z;
};

struct triangle_t {
    v3_t p1;
    v3_t p2;
    v3_t p3;
};

double Magnitude(v3_t v){
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

void OutputTriangle(triangle_t t){
    cout << "Triangle: "
         << "{" << t.p1.x << "," << t.p1.y << "," << t.p1.z << "}, "
         << "{" << t.p2.x << "," << t.p2.y << "," << t.p2.z << "}, "
         << "{" << t.p3.x << "," << t.p3.y << "," << t.p3.z << "}; "
         << endl;
}

v3_t Midpoint(v3_t a, v3_t b){
    return {(a.x + b.x)/2.0, (a.y + b.y)/2.0, (a.z + b.z)/2.0};
}

v3_t
Normalize(v3_t v){
    double len = Magnitude(v);
    if (len != 0){
        v.x /= len;
        v.y /= len;
        v.z /= len;
    }
    return v;
}

vector<triangle_t> InitSphere(){
    v3_t v1 = { 1,  0,  0};
    v3_t v2 = { 0,  1,  0};
    v3_t v3 = {-1,  0,  0};
    v3_t v4 = { 0, -1,  0};
    v3_t v5 = { 0,  0,  1};
    v3_t v6 = { 0,  0, -1};

    vector<triangle_t> result;

    result.push_back({v1, v2, v5});
    result.push_back({v2, v3, v5});
    result.push_back({v3, v4, v5});
    result.push_back({v4, v1, v5});

    result.push_back({v1, v2, v6});
    result.push_back({v2, v3, v6});
    result.push_back({v3, v4, v6});
    result.push_back({v4, v1, v6});

    return result;
}

vector<triangle_t> SubdivideTriangle(triangle_t triangle){
    v3_t p4 = Midpoint(triangle.p1, triangle.p2);
    v3_t p5 = Midpoint(triangle.p2, triangle.p3);
    v3_t p6 = Midpoint(triangle.p3, triangle.p1);

    p4 = Normalize(p4);
    p5 = Normalize(p5);
    p6 = Normalize(p6);

    vector<triangle_t> result;
    result.push_back({triangle.p1, p4, p6});
    result.push_back({p4, triangle.p2, p5});
    result.push_back({p6, p4, p5});
    result.push_back({p6, p5, triangle.p3});

    return result;
}

vector<triangle_t> ProcessRecursiveLayers(int d){
    vector<triangle_t> triangleList = InitSphere();
    for(int i = 0; i < d; i++){
        vector<triangle_t> layer2;
        for(auto triangle : triangleList){
            //OutputTriangle(triangle);
            vector<triangle_t> sub = SubdivideTriangle(triangle);
            for(auto subtriangle : sub){
                layer2.push_back(subtriangle);
            }
        }
        triangleList = layer2;
    }
    return triangleList;
}

void OutputHeader(){
    string padding = "  ";
    cout << "{" << endl;
    cout << padding << "\"objects\":[" << endl;
}

void OutputJsonTriangle(triangle_t t, bool comma){
    string padding = "  ";
    cout << padding << padding << "{" << endl;
    cout << padding << padding << padding << "\"type\":\"triangle\"," << endl;
    cout << padding << padding << padding << "\"p1\":[" << t.p1.x << ", " << t.p1.y << ", " << t.p1.z << "], " << endl;
    cout << padding << padding << padding << "\"p2\":[" << t.p2.x << ", " << t.p2.y << ", " << t.p2.z << "], " << endl;
    cout << padding << padding << padding << "\"p3\":[" << t.p3.x << ", " << t.p3.y << ", " << t.p3.z << "], " << endl;
    cout << padding << padding << padding << "\"color\":[241, 156, 187]," << endl;;
    cout << padding << padding << padding << "\"specular\":100," << endl;
    cout << padding << padding << padding << "\"reflection\":0.00" << endl;
    cout << padding << padding << "}";
    if (comma)
        cout << "," << endl;
    else
        cout << endl;
}

void OutputFooter(){
    string padding = "  ";
    cout << padding << padding << "]," << endl;
    cout << padding << "\"lights\":[" << endl;
    cout << padding << padding << "{" << endl;
    cout << padding << padding << "\"type\": \"ambient\"," << endl;
    cout << padding << padding << "\"intensity\": 0.2" << endl;
    cout << padding << padding << "}," << endl;
    cout << padding << padding << "{" << endl;
    cout << padding << padding << "\"type\": \"point\"," << endl;
    cout << padding << padding << "\"intensity\": 0.6," << endl;
    cout << padding << padding << "\"position\": [30, 30, -20]" << endl;
    cout << padding << padding << "}" << endl;
    cout << padding << "]," << endl;
    cout << padding << "\"camera\":{" << endl;
    cout << padding << padding << "\"position\": [0, 0, -3]" << endl;
    cout << padding << "}," << endl;
    cout << padding << "\"settings\":{" << endl;
    cout << padding << padding << "\"numberOfThreads\": 8," << endl;
    cout << padding << padding << "\"subsampling\": true" << endl;
    cout << padding << "}" << endl;
    cout << "}" << endl;
}

int main(int argc, char **argv){
    bool jsonOutput = false;
    int layers = 3;

    if (argc > 1){
        layers = atoi(argv[1]);
    }

    if (argc > 2){
        cout << argv[0] << endl;
        cout << argv[1] << endl;
        cout << argv[2] << endl;
        if (strncmp(argv[2], "json", strlen(argv[2])) == 0){
            jsonOutput = true;
        }
    }

    if (jsonOutput){
        OutputHeader();
    }

    vector<triangle_t> triangleList = ProcessRecursiveLayers(layers);

    for(int i = 0; i < triangleList.size(); i++){
        if (jsonOutput){
            OutputJsonTriangle(triangleList[i], i != triangleList.size() - 1);
        } else {
            OutputTriangle(triangleList[i]);
        }
    }

    if (jsonOutput){
        OutputFooter();
    }
}