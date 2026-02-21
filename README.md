# Introduction
This project was made for the OOP C++ course assignment.

## Class structure
I decided to use filePath instead of fileName, because I wanted to check whether the file existed and if I used them both, I would have to update them at the same time which would be inconvenient and clutter the code.

Image
-id: int
-filePath: string
-width: int
-height: int
-------------------
-refreshDimensions()
+setFilePath(path: string)
+setWidth(width: int)
+setHeight(width: int)
+getId(): int
+getFilePath(): string
+getWidth(): int
+getHeight(): int
+toString(): string

## Validation methods
These are the methods that were used for validation of the image file.

### This part checks whether the path exists based on the operating system
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

### This method checks whether the file has a valid image extension
```cpp
bool isValidImageExtension(string path) {
    return path.substr(path.find_last_of('.') + 1) == "jpg"  ||
           path.substr(path.find_last_of('.') + 1) == "jpeg" ||
           path.substr(path.find_last_of('.') + 1) == "png"  ||
           path.substr(path.find_last_of('.') + 1) == "gif"  ||
           path.substr(path.find_last_of('.') + 1) == "bmp";
}
```


