#include <stdlib.h>
#include <Arduino.h>
#include "FS.h"
#include <LittleFS.h>
 
#define FORMAT_LITTLEFS_IF_FAILED true
void deleteFile2(fs::FS &fs, const char *path) {
   if (fs.remove(path)) {
    Serial.println("- file deleted");
  } else {
    Serial.println("- delete failed");
  }
}
void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      char fn[40];
      strcpy(fn, file.name());
      Serial.print("  FILE: ");
      Serial.print(fn);
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
      if ((file.size() > 100) || strstr(fn, "'") || strstr(fn, "\"") || strstr(fn, "&") || strstr(fn, "\n") || strstr(fn, "\r")) {
        strcpy(fn, file.path());
        file.close();
        deleteFile2(LittleFS, fn);
      }
      file.close();
    }
    file = root.openNextFile();
  }
}
void listDirHTML(char *tmp,int sta) {
  short n;
  n = 0;
  File root = LittleFS.open("/");
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory() ) {
      char fn[50];
      if (n>sta)
      {strcpy(fn, file.name());
       if (n == sta) strcat(tmp, "<tr>");
       strcat(tmp,         "<td onclick='ab(\"logfile?file=");strcat(tmp, fn);strcat(tmp,"\")'>");strcat(tmp, fn);
       strcat(tmp, "</td><td onclick='cd(\"logfiledel?file=");strcat(tmp, fn);strcat(tmp,"\")'> Del  </td>");
       strcat(tmp,    "<td><a download href=\"logfile?file=");strcat(tmp, fn);strcat(tmp,"\"> Load </a></td>");
       if ( ((++n)% 5)==0 ){
         strcat(tmp, "</tr>");
       }
      }
       else n++; 
       file.close();
      if (n ==(sta+10)) break;
    }
    file = root.openNextFile();
  }
  if (n >= sta)
    strcat(tmp,"</tr></table>");
    if(sta>0) strcat(tmp,"<a align=\"left\" href=/logs?min>Previous</a>");
    if (n==sta+10) strcat(tmp,"<a align=\"right\" href=logs?plus >Next</a> ");
  return;
}
 void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return;
  }

  Serial.println("- read from file:");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = (fs.open(path, FILE_APPEND));
  if (!file) file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("- failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("- message appended");
  } else {
    Serial.println("- append failed");
  }
  file.close();
}

// SPIFFS-like write and delete file, better use #define CONFIG_LITTLEFS_SPIFFS_COMPAT 1

void writeFile2(fs::FS &fs, const char *path, const char *message) {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  }
  if (!fs.exists(path)) {
    if (strchr(path, '/')) {
      Serial.printf("Create missing folders of: %s\r\n", path);
      char *pathStr = strdup(path);
      if (pathStr) {
        char *ptr = strchr(pathStr, '/');
        while (ptr) {
          *ptr = 0;
          fs.mkdir(pathStr);
          *ptr = '/';
          ptr = strchr(ptr + 1, '/');
        }
      }
      free(pathStr);
    }
  }

  Serial.printf("Writing file to: %s\r\n", path);
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}
 char cfn[20];
short cfni;
void setup_SD() {
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }
 // listDir(LittleFS, "/", 1);
  
}
