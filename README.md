# Introduction
This project was made for the OOP C++ course assignment.

## Class structure
I decided to use filePath instead of fileName, because I wanted to check whether the file existed and if I used them both, I would have to update them at the same time which would be inconvenient and clutter the code.

```
classDiagram   
    class Image {
        -id: int
        -filePath: string
        -width: int
        -height: int
        -refreshDimensions()
        +setFilePath(path: string)
        +setWidth(width: int)
        +setHeight(width: int)
        +getId(): int
        +getFilePath(): string
        +getWidth(): int
        +getHeight(): int
        +toString(): string
   }
```


## Validation methods
These are the methods that were used for validation of the image file.

### 1. This part checks whether the path exists based on the operating system
```cpp
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
```

### 2. This function checks whether the file has a valid image extension
```cpp
bool isValidImageExtension(string path) {
    return path.substr(path.find_last_of('.') + 1) == "jpg"  ||
           path.substr(path.find_last_of('.') + 1) == "jpeg" ||
           path.substr(path.find_last_of('.') + 1) == "png"  ||
           path.substr(path.find_last_of('.') + 1) == "gif"  ||
           path.substr(path.find_last_of('.') + 1) == "bmp";
}
```

### 3. This function checks if the file has a proper image magic number and is not corrupted
```cpp
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
```

### 4. This function combines functions 2 and 3 to check whether the image file is valid
```cpp
bool isValidImage(string path) {
    return !path.empty() &&
           isValidImageExtension(path) &&
           isValidImageMagicNumber(path);
}
```

## Core feature: parsing image dimensions from binary headers
```cpp
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
```


