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
                setFilepath(path);
                setWidth(width);
                setHeight(height);
                init();
            }

            Image(string path) {
                setFilepath(path);
                refreshDimensions();
                init();
            }

            // Destructor
            ~Image() {
                --objectCount;
            }

    private:
        void init() {
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

        // Refreshes the image dimensions from binary headers 
        void refreshDimensions() {
            ifstream file(filePath, ios::binary);
            if (!file) return;

            unsigned char header[8];
            file.read(reinterpret_cast<char*>(header), 8);

            //PNG: Dimensions start at offset 16
            if (header[0] == 0x89 && header[1] == 0x50 && header[2] == 0x4E && header[3] == 0x47) {
                file.seekg(16);
                unsigned char buffer[8];
                file.read(reinterpret_cast<char*>(buffer), 8);

                // PNG stores width/height as 4-byte Big-Endian integers
                width = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
                height = (buffer[4] << 24) | (buffer[5] << 16) | (buffer[6] << 8) | buffer[7];
            }

            // BMP: Dimensions start at offset 18
            else if (header[0] == 'B' && header[1] == 'M') {
                file.seekg(18);
                unsigned char buffer[8];
                file.read(reinterpret_cast<char*>(buffer), 8);

                // BMP stores width/height as 4-byte Little-Endian integers
                width = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
                height = buffer[4] | (buffer[5] << 8) | (buffer[6] << 16) | (buffer[7] << 24);
            }

            // JPEG: Requires searching for the SOF (Start of Frame) marker
            else if (header[0] == 0xFF && header[1] == 0xD8) {
                file.seekg(2);
                while (file) {
                    unsigned char marker[2];
                    file.read(reinterpret_cast<char*>(marker), 2);
            
                    // Check for Start of Frame markers (SOF0, SOF1, SOF2)
                    if (marker[0] == 0xFF && (marker[1] >= 0xC0 && marker[1] <= 0xC2)) {
                        file.seekg(3, ios::cur); // Skip length and precision
                        unsigned char buffer[4];
                        file.read(reinterpret_cast<char*>(buffer), 4);

                        // JPEG is Big-Endian: [Height High][Height Low][Width High][Width Low]
                        height = (buffer[0] << 8) | buffer[1];
                        width = (buffer[2] << 8) | buffer[3];
                        break;
                    } else {
                        // Skip segment: Read length, then seek
                        unsigned char lengthBuffer[2];
                        file.read(reinterpret_cast<char*>(lengthBuffer), 2);
                        unsigned short length = (lengthBuffer[0] << 8) | lengthBuffer[1];
                        if (length < 2) break;
                        file.seekg(length - 2, ios::cur);
                    }
                }
            }
        }

        string toString() {
            stringstream ss;
            ss << getId() << " " << getFilePath() 
                << " " << getWidth() << "x" << getHeight();
            return ss.str();
        }
};
int Image::lastId = 0;
int Image::objectCount = 0;

int main() {
    return 0;
}