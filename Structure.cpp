#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>

using namespace std;
using namespace std::filesystem;

void process_directory(const path& dir, const string& indent, ofstream& file) {
  for (const auto& entry : directory_iterator(dir)) {
    if (is_directory(entry)) {
      file << indent << "/" << entry.path().filename() << endl;
      process_directory(entry.path(), indent + "  ", file);
    } else {
      file << indent << "-" << entry.path().filename() << endl;
    }
  }
}

int main() {
  string filename = "Structure.txt";
  ofstream file(filename);

  if (file.is_open()) {
    process_directory(current_path(), "", file);
    file.close();
    cout << "Structure of paths & files saved to " << filename << endl;
  } else {
    cerr << "Unable to open " << filename << endl;
  }

  return 0;
}
