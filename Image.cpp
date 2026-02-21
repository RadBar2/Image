#include <fstream>

#include <exception>
#include <stdexcept>

#include <string>
#include <cstring>
#include <sstream>

using namespace std;

/*
   Validation
*/

// Checks if filePath exists
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

// Checks if the file extension is an image extension
bool isValidImageExtension(string path) {
    return path.substr(path.find_last_of('.') + 1) == "jpg"  ||
           path.substr(path.find_last_of('.') + 1) == "jpeg" ||
           path.substr(path.find_last_of('.') + 1) == "png"  ||
           path.substr(path.find_last_of('.') + 1) == "gif"  ||
           path.substr(path.find_last_of('.') + 1) == "bmp";
}

// Checks if the file signature is an image magic number
bool isValidImageMagicNumber(string path) {
    ifstream file(path, ios::binary);
    unsigned char buffer[8] = {0};
    file.read(reinterpret_cast<char*>(buffer), 8);

    // Check for JPG/JPEG (FF D8 FF)
    if (buffer[0] == 0xFF && buffer[1] == 0xD8 && buffer[2] == 0xFF)
        return true;
    // Check for PNG (89 50 4E 47)
    if (buffer[0] == 0x89 && buffer[1] == 0x50 && buffer[2] == 0x4E && buffer[3] == 0x47)
        return true;
    // Check for GIF (GIF87a / GIF89a)
    if (strncmp(reinterpret_cast<char*>(buffer), "GIF", 3) == 0)
        return true;
    // Check for BMP (BM)
    if (buffer[0] == 'B' && buffer[1] == 'M')
        return true;

    return false;
}

// Checks if the image is valid
bool isValidImage(string path) {
    return !path.empty() &&
           isValidImageExtension(path) &&
           isValidImageMagicNumber(path);
}


/*
    Class definition
*/

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
            if (!pathExists(path)) {
                throw invalid_argument("The provided path does not exist");
            } else if (!isValidImage(path)) {
                throw invalid_argument("The file is not a valid image");
            }

            this->filePath = path;
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