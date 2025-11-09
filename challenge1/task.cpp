#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include <string>
using namespace std;

struct gout{
  int c=0;  // file size in bytes (for -c)
  int w=0;  // word count
  int l=0;  // line count
  int m=0;  // character count (for -m, considering multibyte)
};

// More robust UTF-8 character counting
int count_utf8_chars(const string& str) {
    int count = 0;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        if (c <= 0x7F) { // ASCII character (0xxxxxxx)
            i += 1;
        } else if ((c & 0xE0) == 0xC0) { // 2-byte sequence (110xxxxx)
            i += 2;
        } else if ((c & 0xF0) == 0xE0) { // 3-byte sequence (1110xxxx)
            i += 3;
        } else if ((c & 0xF8) == 0xF0) { // 4-byte sequence (11110xxx)
            i += 4;
        } else {
            // Invalid, skip one byte
            i += 1;
        }
        count++;
    }
    return count;
}

gout readfile(string &filename){
    gout out;

    // Get file size for -c option
    ifstream size_file(filename, ios::binary);
    size_file.seekg(0, ios::end);
    out.c = size_file.tellg();
    size_file.close();

    // Read the entire file for accurate character counting
    ifstream myfile(filename, ios::binary);
    if (myfile.is_open()) {
        stringstream buffer;
        buffer << myfile.rdbuf();
        string content = buffer.str();
        myfile.close();

        // Count characters in entire content
        out.m = count_utf8_chars(content);

        // Now count lines and words by processing the content
        stringstream ss(content);
        string line;

        while (getline(ss, line)) {
            out.l++;

            // Count words in this line
            stringstream line_ss(line);
            string word;
            while (line_ss >> word) {
                out.w++;
            }
        }
    }

    return out;
}

int main(){
    string instruction;
    string operation;
    string filename;
    cin >> instruction;


    gout file_content = readfile(filename);

    if(operation == "-c"){
        cout << file_content.c << endl;
    }
    else if(operation == "-w"){
        cout << file_content.w << endl;
    }
    else if(operation == "-l"){
        cout << file_content.l << endl;
    }
    else if(operation == "-m"){
        cout << file_content.m << endl;
    }
    else{
        return -1;
    }

    return 0;
}
