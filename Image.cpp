#include <fstream>
#include <iostream>

#include <exception>
#include <stdexcept>
#include <assert.h>

#include <string>
#include <cstring>
#include <sstream>

#include <vector>

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
                width = ((unsigned int)buffer[0] << 24) | ((unsigned int)buffer[1] << 16) | 
                        ((unsigned int)buffer[2] << 8)  | (unsigned int)buffer[3];

                height = ((unsigned int)buffer[4] << 24) | ((unsigned int)buffer[5] << 16) | 
                         ((unsigned int)buffer[6] << 8)  | (unsigned int)buffer[7];
            }

            // GIF: Dimensions start at offset 6
            else if (strncmp(reinterpret_cast<char*>(header), "GIF", 3) == 0) {
                file.seekg(6);
                unsigned char buffer[4];
                file.read(reinterpret_cast<char*>(buffer), 4);

                // GIF is Little-Endian (Width is 6-7, Height is 8-9)
                width = (unsigned int)buffer[0] | ((unsigned int)buffer[1] << 8);
                height = (unsigned int)buffer[2] | ((unsigned int)buffer[3] << 8);
            }

            // BMP: Dimensions start at offset 18
            else if (header[0] == 'B' && header[1] == 'M') {
                file.seekg(18);
                unsigned char buffer[8];
                file.read(reinterpret_cast<char*>(buffer), 8);

                // BMP stores width/height as 4-byte Little-Endian integers
                width = (unsigned int)buffer[0] | ((unsigned int)buffer[1] << 8) | 
                        ((unsigned int)buffer[2] << 16) | ((unsigned int)buffer[3] << 24);
                height = (unsigned int)buffer[4] | ((unsigned int)buffer[5] << 8) |
                         ((unsigned int)buffer[6] << 16) | ((unsigned int)buffer[7] << 24);
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
                        height = ((unsigned int)buffer[0] << 8) | (unsigned int)buffer[1];
                        width  = ((unsigned int)buffer[2] << 8) | (unsigned int)buffer[3];
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

        string toString() {
            stringstream ss;
            ss << getId() << " " << getFilePath() 
                << " " << getWidth() << "x" << getHeight();
            return ss.str();
        }
};
int Image::lastId = 0;
int Image::objectCount = 0;

/*
    Testing
*/

// Checks if the image dimension detection works as expected
void testRealImageDetection() {
    cout << "--- Testing Real Image Detection ---" << endl;

    // 1. Testing a JPG 
    try {
        Image imgJpg("input.jpg"); 
        cout << "JPG detected: " << imgJpg.getWidth() << "x" << imgJpg.getHeight() << endl;
        assert(imgJpg.getWidth() > 0);
    } catch (...) {
        cout << "Skipping JPG test: input.jpg not found." << endl;
    }

    // 2. Testing a PNG 
    try {
        Image imgPng("input.png");
        cout << "PNG detected: " << imgPng.getWidth() << "x" << imgPng.getHeight() << endl;
        assert(imgPng.getWidth() > 0);
    } catch (...) {
        cout << "Skipping PNG test: input.png not found." << endl;
    }

    // 3. Testing a GIF 
    try {
        Image imgPng("input.gif");
        cout << "GIF detected: " << imgPng.getWidth() << "x" << imgPng.getHeight() << endl;
        assert(imgPng.getWidth() > 0);
    } catch (...) {
        cout << "Skipping GIF test: input.gif not found." << endl;
    }

    // 4. Testing a BMP
    try {
        Image imgPng("input.bmp");
        cout << "BMP detected: " << imgPng.getWidth() << "x" << imgPng.getHeight() << endl;
        assert(imgPng.getWidth() > 0);
    } catch (...) {
        cout << "Skipping BMP test: input.bmp not found." << endl;
    }
}

int main() {

    string validPath = "test.jpg";

    // Create dummy file for testing purposes if it doesn't exist
    ofstream outfile(validPath, ios::binary);
    unsigned char jpgHeader[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 'J', 'F', 'I', 'F'};
    outfile.write(reinterpret_cast<char*>(jpgHeader), 10);
    outfile.close();

    cout << "Starting tests..." << endl;

    // Test 1: Getters and toString
    Image img1(validPath, 800, 600);
    assert(img1.getWidth() == 800);
    assert(img1.getFilePath() == validPath);
    string expected = to_string(img1.getId()) + " " + validPath + " 800x600";
    assert(img1.toString() == expected);
    cout << "Test 1 Passed: Getters and toString valid." << endl;

    // Test 2: Setters
    img1.setWidth(1024);
    img1.setHeight(768);
    assert(img1.getWidth() == 1024);
    assert(img1.getHeight() == 768);
    cout << "Test 2 Passed: Setters working." << endl;

    // Test 3: Exception Validation
    try {
        img1.setFilepath("non_existent.png");
        assert(false); // Should not reach here
    } catch (const invalid_argument& e) {
        cout << "Test 3 Passed: Caught expected exception: " << e.what() << endl;
    }

    // Test 4: Automatic Numbering (ID)
    Image img2(validPath);
    assert(img2.getId() > img1.getId());
    cout << "Test 4 Passed: ID auto-incrementing." << endl;

    // Test 5: Dynamic allocation and Object Count
    assert(Image::getObjectCount() == 2); // img1 and img2 are on stack
    
    Image* dynamicImg = new Image(validPath, 100, 100);
    assert(Image::getObjectCount() == 3);

    // List of dynamic objects
    int initialCount = Image::getObjectCount();
    vector<Image*> imageList;
    for(int i=0; i<5; i++) {
        imageList.push_back(new Image(validPath));
    }
    assert(Image::getObjectCount() == initialCount + 5);

    // Cleanup
    delete dynamicImg;
    for(auto p : imageList) delete p;
    
    cout << "Test 5 Passed: Dynamic memory and object counting valid." << endl;
    cout << "Final Object Count: " << Image::getObjectCount() << " (Memory Clean)" << endl;

    testRealImageDetection();

    return 0;
}