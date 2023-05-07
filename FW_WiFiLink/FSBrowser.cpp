#include "FSBrowser.h"
#include <base64.hpp>

File fsUploadFile;

//Requires that http server is started afterwards
void Init_FSBrowser()
{
      //list directory
    HTTP_SERVER.on(StringF("/list"), HTTP_GET, handleFileList);
    //load editor
    HTTP_SERVER.on(StringF("/edit"), HTTP_GET, [](){
      if (!handleFileRead(StringF("/edit.htm"))) handleBasicUpload();
    });
    //create file
    HTTP_SERVER.on(StringF("/edit"), HTTP_PUT, handleFileCreate);
    //delete file
    HTTP_SERVER.on(StringF("/edit"), HTTP_DELETE, handleFileDelete);
    //first callback is called after the request has ended with all parsed arguments
    //second callback handles file uploads at that location
    HTTP_SERVER.on(StringF("/edit"), HTTP_POST, [](){ HTTP_SERVER.send(200, StringF("text/plain"), ""); }, handleFileUpload);
  
    //called when the url is not defined here
    //use it to load content from SPIFFS
    HTTP_SERVER.onNotFound([](){
      if(!handleFileRead(HTTP_SERVER.uri()))
        HTTP_SERVER.send(404, StringF("text/plain"), StringF("FileNotFound"));
    });
  
    //get heap status, analog input value and all GPIO statuses in one json call
    HTTP_SERVER.on(StringF("/all"), HTTP_GET, [](){
      String json = StringF("{");
      json += StringF("\"heap\":")+String(ESP.getFreeHeap());
      json += StringF(", \"analog\":")+String(analogRead(A0));
      json += StringF(", \"gpio\":")+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
      json += StringF("}");
      HTTP_SERVER.send(200, StringF("text/json"), json);
      json = String();
    });

}

void handleBasicUpload()
{
    String BasicUploadPage = StringF(\
    "<!DOCTYPE html>\
    <html><head><title>Basic Upload</title></head>\
    <body>\
      <form method='POST' action='/edit' enctype='multipart/form-data'>\
        <input type = 'file' name = 'file'>\
        <input type = 'submit' value = 'Upload'>\
      </form>\
    </body>\
    </html>");
      HTTP_SERVER.send(200, StringF("text/html"), BasicUploadPage);
}

String formatBytes(size_t bytes){
    if (bytes < 1024){
        return String(bytes)+ StringF("B");
    } else if(bytes < (1024 * 1024)){
        return String(bytes/1024.0)+ StringF("KB");
    } else if(bytes < (1024 * 1024 * 1024)){
        return String(bytes/1024.0/1024.0)+ StringF("MB");
    } else {
        return String(bytes/1024.0/1024.0/1024.0)+ StringF("GB");
    }
}

String getContentType(String filename){
    if(HTTP_SERVER.hasArg(StringF("download"))) return StringF("application/octet-stream");
    else if(filename.endsWith(StringF(".htm"))) return StringF("text/html");
    else if(filename.endsWith(StringF(".html"))) return StringF("text/html");
    else if(filename.endsWith(StringF(".css"))) return StringF("text/css");
    else if(filename.endsWith(StringF(".js"))) return StringF("application/javascript");
    else if(filename.endsWith(StringF(".png"))) return StringF("image/png");
    else if(filename.endsWith(StringF(".gif"))) return StringF("image/gif");
    else if(filename.endsWith(StringF(".jpg"))) return StringF("image/jpeg");
    else if(filename.endsWith(StringF(".ico"))) return StringF("image/x-icon");
    else if(filename.endsWith(StringF(".xml"))) return StringF("text/xml");
    else if(filename.endsWith(StringF(".pdf"))) return StringF("application/x-pdf");
    else if(filename.endsWith(StringF(".zip"))) return StringF("application/x-zip");
    else if(filename.endsWith(StringF(".gz"))) return StringF("application/x-gzip");
    return StringF("text/plain");
}

const char favicon_base64[] PROGMEM = R"====(iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAABHNCSVQICAgIfAhkiAAAAAlwSFlzAAAETwAABE8BkMxxGAAAABl0RVh0U29mdHdhcmUAd3d3Lmlua3NjYXBlLm9yZ5vuPBoAAASQSURBVFiF5Zf7b1N1FMA/t++u7/drry5jgwEKU56ChuArAgnGxJj4mwnG+IOJf4CJJkajv5v4E5EQJPyGghqiRoGFSFAJIm4wNtaNsXZr19uuu117b1d/6Che2g5WIPzgSW5uzun5nvP5vs65FcrlcpnHKJrHmRxAd7fh0pdfIM/PE9q2ncj2HQAMHT1C7takyk9jMGByu4ls34Ej2vXwAAaPHEaamUZjMFQBRk4cJ/HnH3UDCFot2z74kNVvvPlwAJYT/5MbcfWuBqCQEZk8expZkvjt449o37WblkDw0QJ0vPQy6986UNXF68Mcf3Uvi7LM+C8/N7UKD3QInd2rsHd0ArCQTjcV44EASgsLSNMJAByd0aZirGgLpESC5JW/AVAkiX8Of0Uxm8UR7aJ99wuPHuDKoYNcOXRQZWvd+Rw7PvkMrcHQFMCKtkBrMmG0OzDaHWhNJgBuDpzhwuefsijLTQGsaAX633v/zi0ol5m6cJ6f3n2bkRPf4Orp5YkD76wYoPlDKAiENm+l57XXARj9/mRTYWoAirkcAEa7/b4C6K1WAPIzMw8OkBkdQclLAFiCoXsOLmRERk9+C4Ctta0pAN3kuQFiP55iUVG4efrXilUQ8K5bX+N89dhRJgfOAFBWSswODVLIZgDo2rO3OQBx+BpDR4+ojNH9+7FGWqu6IhcByMbGyMbGaoLY1q5G6Wnj8u+VCbh9YSIdPYwN/8VcZlblq9Xq8PjDBCKVDqpzr+kj+speZmOjyJoy5r5efPv2qQbZNz2F4q2cCavdhcffSqmkcHNiGGNbGHNvNzOJiaq/3mAEQEwlSE2r2zjA1MR1smKKVWs3oQtt3kpo81auXT7PrfFhtDod6VRCNcDQ30eov4+8lCPcvoo1G56hWMiTO3UMi81Bz7otKn+j2aLS12/ahdlsZbG8iJiKMzJ4kYnRQdqia+6qA4KA0xNk+tYY83MiFpuThXyOBSlHqK2bvHS9ZjY6nQG3L1xj/69YbE4sVgcADpePucwsickbZMVk7TV0eSo9PZ2KV97J+JI9sGySlYjBaAZAlou1AE5vQJU4nYxjsTnRLw26W8rlMopcrD6lkrJscrlYIBkfB6DFYqstxVabC4PBhLgEIKbiuP2RhgGzYpLTP3xd1YOtXaztf1blM3hxAK1OT6mkMJ9NoygyFpsTpydQvxc4PQGmp2IkExPkpVx1W+qJRqPFaGqp6nqDqcYnk1ZXSYvNyYYtzyMImgYA3iDTUzFuXL20BBRkLpOqC2BzuHl6556GgAAbt72I2WJDLixw4ex3aLU6TC2VEl63Gd2ecVZM0mK1YzTV3//7FaPZgrnFht3lw2p3kRWTFPLzjQGsdld1KZdb/mbEG6j0jNuFq2E7drr9lfc9AIqFPJOxa6pnNjnV0N8TqJT4ZLwC0PCDxOUNMhMfx3mP+5+XcgxdOqeyBSJR3N763dTh8qE3mJhNTqHIxTsAVocbn9x+h9QfIdLRg2mprBpNZvzhTmxOD1A5/f5wZ8MkULlNWp0enU5f/U0QBDq615EVk+SlHML//t/xv+fDlKMwB1K2AAAAAElFTkSuQmCC)====";
void handleFavicon()
{
    String base64 = String(FPSTR(favicon_base64));
    unsigned char* binary;
    unsigned int binary_length = decode_base64_length((unsigned char*)base64.c_str());

    binary = (unsigned char*)malloc(binary_length + 1);  //+1 probably not needed, but I am not 100% sure
    binary_length = decode_base64((unsigned char*)base64.c_str(), binary);

    HTTP_SERVER.send(200, StringF("image/png").c_str(), (const char*)binary, (size_t)binary_length);
    free(binary);
}

bool handleFileRead(String path){
    SERIAL_DBG_PORT.println(StringF("handleFileRead: ") + path);
    if(path.endsWith("/")) path += StringF("index.htm");
    String contentType = getContentType(path);
    String pathWithGz = path + StringF(".gz");
    if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
        if(SPIFFS.exists(pathWithGz))
          path += StringF(".gz");
        File file = SPIFFS.open(path, "r");
        HTTP_SERVER.streamFile(file, contentType);
        file.close();
        return true;
    }
    else if (path == StringF("/favicon.ico"))
    {
        handleFavicon();
        return true;
    }
    return false;
}

void handleFileUpload(){
    if(HTTP_SERVER.uri() != StringF("/edit")) return;
    HTTPUpload& upload = HTTP_SERVER.upload();
    if(upload.status == UPLOAD_FILE_START){
        String filename = upload.filename;
        if(!filename.startsWith("/")) filename = StringF("/")+filename;
        SERIAL_DBG_PORT.printF("handleFileUpload Name: "); SERIAL_DBG_PORT.println(filename);
        fsUploadFile = SPIFFS.open(filename, "w");
        filename = String();
    } else if(upload.status == UPLOAD_FILE_WRITE){
        //SERIAL_DBG_PORT.print("handleFileUpload Data: "); SERIAL_DBG_PORT.println(upload.currentSize);
        if(fsUploadFile)
            fsUploadFile.write(upload.buf, upload.currentSize);
    } else if(upload.status == UPLOAD_FILE_END){
        if(fsUploadFile)
            fsUploadFile.close();
        SERIAL_DBG_PORT.printF("handleFileUpload Size: "); SERIAL_DBG_PORT.println(upload.totalSize);
    }
}

void handleFileDelete(){
    if(HTTP_SERVER.args() == 0)
        return HTTP_SERVER.send(500, StringF("text/plain"), StringF("BAD ARGS"));
    String path = HTTP_SERVER.arg(0);
    SERIAL_DBG_PORT.println(StringF("handleFileDelete: ") + path);
    if(path == "/")
        return HTTP_SERVER.send(500, StringF("text/plain"), StringF("BAD PATH"));
    if(!SPIFFS.exists(path))
        return HTTP_SERVER.send(404, StringF("text/plain"), StringF("FileNotFound"));
    SPIFFS.remove(path);
    HTTP_SERVER.send(200, StringF("text/plain"), "");
    path = String();
}

void handleFileCreate(){
    if(HTTP_SERVER.args() == 0)
        return HTTP_SERVER.send(500, StringF("text/plain"), StringF("BAD ARGS"));
    String path = HTTP_SERVER.arg(0);
    SERIAL_DBG_PORT.println(StringF("handleFileCreate: ") + path);
    if(path == "/")
        return HTTP_SERVER.send(500, StringF("text/plain"), StringF("BAD PATH"));
    if(SPIFFS.exists(path))
        return HTTP_SERVER.send(500, StringF("text/plain"), StringF("FILE EXISTS"));
    File file = SPIFFS.open(path, "w");
    if(file)
        file.close();
    else
        return HTTP_SERVER.send(500, StringF("text/plain"), StringF("CREATE FAILED"));
    HTTP_SERVER.send(200, StringF("text/plain"), "");
    path = String();
}

void handleFileList() {
    if(!HTTP_SERVER.hasArg(StringF("dir"))) {HTTP_SERVER.send(500, StringF("text/plain"), StringF("BAD ARGS")); return;}
    
    String path = HTTP_SERVER.arg(StringF("dir"));
    SERIAL_DBG_PORT.println(StringF("handleFileList: ") + path);
    Dir dir = SPIFFS.openDir(path);
    path = String();
  
    String output = StringF("[");
    while(dir.next()){
        File entry = dir.openFile("r");
        if (output != StringF("[")) output += ',';
        bool isDir = false;
        output += StringF("{\"type\":\"");
        output += (isDir)? StringF("dir"): StringF("file");
        output += StringF("\",\"name\":\"");
        output += String(entry.name()).substring(1);
        output += StringF("\"}");
        entry.close();
    }
    output += StringF("]");
    HTTP_SERVER.send(200, StringF("text/json"), output);
}
