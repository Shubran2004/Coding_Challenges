#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
using namespace std;

bool pass = true;

static int depth=0;


bool validstr( string& content);
bool validobject(string &content,int u);
bool validint( string &content);
bool validarray(string &content,int u);

// Helper functions to remove whitespace
void remove_front(string &content) {
    int i = 0;
    while (i < content.size() && std::isspace(static_cast<unsigned char>(content[i]))) {
        i++;
    }
    content.erase(0, i);
}

void remove_last(string &content) {
    while (!content.empty() && std::isspace(static_cast<unsigned char>(content.back()))) {
        content.pop_back();
    }
}

void remove_newline(string &tstring) {
    for (size_t i = 0; i < tstring.size(); i++) {
        if (tstring[i] == '\n' || tstring[i] == '\r') {
            tstring[i] = ' ';
        }
    }
}

string read(const string& filename) {
    ifstream myfile(filename, ios::binary);
    if (myfile.is_open()) {
        stringstream buffer;
        buffer << myfile.rdbuf();
        return buffer.str();
    }
    return "";
}

void cleanup(string &content) {
    // First, normalize whitespace except within strings
    string temp = "";
    bool in_quotes = false;
    bool escape_next = false;

    for (size_t i = 0; i < content.size(); i++) {
        char c = content[i];

        if (escape_next) {
            temp.push_back(c);
            escape_next = false;
            continue;
        }

        if (c == '\\') {
            escape_next = true;
            temp.push_back(c);
            continue;
        }

        if (c == '"') {
            in_quotes = !in_quotes;
            temp.push_back(c);
            continue;
        }

        if (in_quotes) {
            temp.push_back(c);
        } else {
            // Outside quotes, keep only necessary characters
            if (!std::isspace(static_cast<unsigned char>(c))) {
                temp.push_back(c);
            } else if (c == '\n' || c == '\r') {
                // Convert newlines to spaces for easier parsing
                if (!temp.empty() && temp.back() != ' ') {
                    temp.push_back(' ');
                }
            }
        }
    }

    content = temp;
    // Remove leading/trailing spaces that might have been added
    remove_front(content);
    remove_last(content);
}

vector<string> splitByDelimiter(const string& str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;

    while (getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

vector<string> final_form(string &content) {
    return splitByDelimiter(content, ',');
}

void find_array_and_objects(string &content) {
    int depth = 0;
    bool in_quotes = false;
    bool escape_next = false;

    for (size_t i = 0; i < content.size(); i++) {
        char c = content[i];

        if (escape_next) {
            escape_next = false;
            continue;
        }

        if (c == '\\') {
            escape_next = true;
            continue;
        }

        if (c == '"') {
            in_quotes = !in_quotes;
            continue;
        }

        if (!in_quotes) {
            if (c == '{' || c == '[') {
                depth++;
            } else if (c == '}' || c == ']') {
                depth--;
            }   if(c==','){
                if(i==content.size()-1 || i==0){
                    pass=false;
                    continue;
                }
                else if(content[i+1]==','){
                    pass=false;
                }

            } if (c == ',' && depth == 0) {
                content[i] = '\n';
            }


        }
    }
}

vector<string> lexer(string &content) {
    vector<string> out, temp;

    cleanup(content);
    find_array_and_objects(content);
    temp = splitByDelimiter(content, '\n');

    return temp;
}

vector<string> splitKeys(string &content,int u) {
    remove_front(content);
    remove_last(content);
    vector<string> temp;
    size_t colon_pos ;

    colon_pos=string::npos;
    int n=content.size();

    for(int i=1;i<n;i++){
        if(content[i]==':'  && content[i-1]=='"'){
            colon_pos=i;
            string a = content.substr(0, colon_pos);
            string b = content.substr(colon_pos + 1);
            if(validstr(a) && (validstr(b)||validarray(b,u+1)||validint(b)||validobject(b,u+1))){
                break;
            }
        }
    }

    if (colon_pos == string::npos) {
        // No colon found, return the whole content as key with empty value
        temp.push_back(content);
        temp.push_back("");
    } else {
        temp.push_back(content.substr(0, colon_pos));
        temp.push_back(content.substr(colon_pos + 1));
    }
  //  cout<<"-----------"<<endl<<"KEY: "<<temp[0]<<endl<<"VALUE: "<<temp[1]<<endl<<"-----"<<endl;

    return temp;
}

bool unescaped(char &c) {
    if (static_cast<int>(c) == 32) {
    return true;
}
    switch (c) {
        case '"':   // Quotation Mark
        case '\\':  // Reverse Solidus (Backslash)
        case '\b':  // Backspace
        case '\f':  // Form Feed
        case '\n':  // Newline
        case '\r':  // Carriage Return
        case '\t':  // Horizontal Tab
            return true;
        default:
            return false;
    }
}

bool validstr(string &content) {
    remove_front(content);
    remove_last(content);
    //cout<<content<<endl;
    if (content.size() < 2) return false;
    if (content[0] != '"' || content.back() != '"') { //cout<<content<<"*"<<endl;
    return false;}

    // Check for valid escape sequences
    for (size_t i = 1; i < content.size() - 1; i++) {
        unsigned char c = static_cast<unsigned char>(content[i]);

        // --- THE FIX: Reject unescaped control characters ---
        // This catches literal newlines (\n), tabs (\t), etc.
        if (c < 32) {
            return false;
        }

        if (content[i] == '\\') {
            i++;
            if (i >= content.size() - 1) return false;

            char next = content[i];
            switch (next) {
                case '"': case '\\': case '/': case 'b':
                case 'f': case 'n': case 'r': case 't':
                    break;
                case 'u':
                    // Unicode escape - check next 4 hex digits
                    if (i + 4 >= content.size() - 1) return false;
                    for (int j = 0; j < 4; j++) {
                        char hex = content[i + 1 + j];
                        if (!isxdigit(static_cast<unsigned char>(hex))) { // distinct check
                             return false;
                        }
                    }
                    i += 4;
                    break;
                default:
                    return false;
            }
        } else if (content[i] == '"') {
            // Unescaped quote inside string
            return false;
        }
    }

    return true;
}

bool validint( string& content) {
    remove_front(content);
    remove_last(content);
    if (content.empty()) return false;
   // cout<<content<<endl;
    string num = content;
    size_t i = 0;

    // Optional leading minus



    if (num[0] == '-') {
        i = 1;
        if (num.size() == 1) return false; // Just "-" is invalid
    }

    bool has_digit = false;
    bool has_dot = false;
    bool has_exp = false;

    while (i < num.size()) {
        char c = num[i];

        if (c >= '0' && c <= '9') {
            has_digit = true;
            i++;
        } else if (c == '.') {
            if (has_dot || has_exp) return false;
            has_dot = true;
            i++;
        } else if (c == 'e' || c == 'E') {
            if (has_exp || !has_digit) return false;
            has_exp = true;
            i++;

            // Optional sign after exponent
            if (i < num.size() && (num[i] == '+' || num[i] == '-')) {
                i++;
            }

            // Must have at least one digit after exponent
            if (i >= num.size() || !isdigit(num[i])) {
                return false;
            }
        } else {
            return false;
        }
    }

     if(num[0]=='0' && has_dot==false && num.size()!=1){return false;}

    return has_digit;
}




bool validarray(string &content,int u) {
  // cout<<content<<",,u"<<endl;
  // cout<<u<<"^^"<<endl;
   if(u>=20){return false;}
    if (content.empty() || content[0] != '[' || content.back() != ']') {
        return false;
    }

    string inner = content.substr(1, content.size() - 2);
    if (inner.empty()) {
        return true; // Empty array is valid
    }



    vector<string> elements = lexer(inner);




    for (auto &elem : elements) {
        if (elem.empty()) return false;
        remove_front(elem);
        remove_last(elem);
        //cout<<elem<<"*"<<endl;
        if (elem[0] == '"') {
            if (!validstr(elem)){  return false;}
        } else if (elem[0] == '[') {
            if (!validarray(elem,u+1)){ return false;}
        } else if (elem[0] == '{') {
            if (!validobject(elem,u+1)) { return false;}
        } else if ((elem[0] >= '0' && elem[0] <= '9') || elem[0] == '-') {
            if (!validint(elem)) return false;
        } else if (elem == "true" || elem == "false" || elem == "null") {
            continue; // Valid literals
        } else {
            return false;
        }
    }

    return true;
}

bool validobject(string &content,int u) {

    //cout<<u<<"**"<<endl;
    //cout<<content<<":Object"<<endl;
    if(u>=20){return false;}
    remove_front(content);
    remove_last(content);
    //cout<<content<<endl;

    if (content.empty() || content[0] != '{' || content.back() != '}') {
        return false;
    }
   // cout<<content<<endl;
    string inner = content.substr(1, content.size() - 2);
    if (inner.empty()) {
        return true; // Empty object is valid
    }

    vector<string> pairs = lexer(inner);

    for (auto &pair : pairs) {
        if (pair.empty()) continue;

        vector<string> key_value = splitKeys(pair,u);
        if (key_value.size() != 2) {
            return false;
        }

        // Validate key (must be a string)
        string key = key_value[0];
        string value = key_value[1];


        if (!validstr(key)) {
              //  cout<<"Problematic Key"<<endl;
            return false;
        }

        // Validate value
        if (value.empty()) {
            return false;
        } else if (value[0] == '"') {
            if (!validstr(value)) return false;
        } else if (value[0] == '[') {
            if (!validarray(value,u+1)) {return false;}
        } else if (value[0] == '{') {
            if (!validobject(value,u+1)) {  return false;}
        } else if ((value[0] >= '0' && value[0] <= '9') || value[0] == '-') {
            if (!validint(value)) return false;
        } else if (value == "true" || value == "false" || value == "null") {
            continue; // Valid literals
        } else {
            return false;
        }
    }

    return true;
}

int main() {
    string filename;
    cin >> filename;
    cout<<"Reading from "<<filename<<endl;
    string tstring = read(filename);

    if (tstring.empty()) {
        cout << "Error: Could not read file." << endl;
        return 1;
    }

    // Initial cleanup
    remove_front(tstring);
    remove_last(tstring);
    //remove_newline(tstring);
    //cout<<tstring<<endl;
    if (tstring.empty()) {
        cout << "Result: false" << endl;
        return 0;
    }



    if (tstring[0] == '{') {
        pass = validobject(tstring,1);
    } else if (tstring[0] == '[') {
        pass = validarray(tstring,1);
    } else {
        pass = false;
    }

    cout << "Result: " << (pass ? "true" : "false") << endl;
    return 0;
}
