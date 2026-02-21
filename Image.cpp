#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

using namespace std;

#ifdef _WIN32
    #include <windows.h>
    bool pathExists(string path) {
        DWORD attributes = GetFileAttributesA(path.c_str());
        return (attributes != INVALID_FILE_ATTRIBUTES);
    }
#else
    #include <sys/stat.h>
    bool pathExists(string path) {
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
    }
#endif

class Image {
    private:
        static int lastId;
        static int objectCount;

        int id;
        string filePath;
        int width, height;

       public:
            // Constructors
            Image(string path, int width, int height) {
                init(path, width, height);
            }

            Image(string path) {
                init(path, 150, 150);
            }

            // Destructor
            ~Image() {
                --objectCount;
            }

    private:
        void init(string path, int width, int height) {
            setFilepath(path);
            setWidth(width);
            setHeight(height);
            id = ++lastId;
            ++objectCount;
        }

    public:
        // Setters
        void setFilepath(string path) {
            if (pathExists(path)) {
                this->filePath = path;
            }
            
        }

        void setWidth(int width) {
            this->width = width;
        }

        void setHeight(int height) {
            this->height = height;
        }
        
        // Getters
        static int getObjectCount() { 
            return objectCount; 
        }

        int getId() {
            return id;
        }

        string getFilePath() {
            return filePath;
        }

        int getWidth() {
            return width;
        }

        int getHeight() {
            return height;
        }

        string toString() {
            stringstream ss;
            ss << getId() << " " << getFilePath() 
                << " " << getWidth() << "" << getHeight();
            return ss.str();
        }
};
int Image::lastId = 0;
int Image::objectCount = 0;

int main() {
    return 0;
}