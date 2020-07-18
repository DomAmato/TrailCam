#include <SPI.h>
#include <Wire.h>
#include <ArduCAM.h>

Camera * myCAM = ArduCAM::createCamera( CameraModel::OV2640 );

bool take_photo;
uint32_t bytesSent = 0;

void setup() {
  HologramCloud.setRGB("RED");
  HologramCloud.setVerboseErrors();
  uint8_t temp;

  Wire.begin();
  Serial.begin(115200);
  // initialize SPI:
  SPI.begin();
  myCAM->resetCPLD();
  delay(100);

  while (true) {
    if (!myCAM->checkSPIBusStatus()) {
      Dash.pulseLED(100, 100);
      Serial.println("ACK CMD SPI interface Error! END");
      delay(1000);
      continue;
    } else {
      Serial.println("ACK CMD SPI interface OK. END");
      break;
    }
  }
  Dash.offLED();
  HologramCloud.setRGB("ORANGE");

  while (true) {
    if (myCAM->checkModule()) {
      Serial.println("ACK CMD Can't find OV2640 module! END");
      Dash.pulseLED(500, 500);
      delay(1000);
      continue;
    } else {
      Serial.println("ACK CMD OV2640 detected. END");
      break;
    }
  }

  Dash.offLED();

  myCAM->SetFormat(Format::JPEG_FMT);
  myCAM->InitCAM();
  myCAM->SetJPEGsize(JPEG_Size::p1024x768);

  delay(1000);
  myCAM->clear_fifo_flag();

  HologramCloud.setRGB("YELLOW");

  // put your setup code here, to run once:
  while (!HologramCloud.isConnected()) {
    Dash.snooze(1000);
  }

  HologramCloud.sendMessage("Hello Hologram");
  HologramCloud.setRGB("GREEN");
  Dash.offLED();
}

void loop() {
  //The maximum message content is 4096 bytes.
  //The maximum number of topics (tags) is 10. The maximum size of a topic (tag) is 63.
  //HologramCloud.sendMessage(const uint8_t* content, uint32_t length, const char* tag)

  if (Serial.available())
  {
    char temp = Serial.read();
    if (temp == 'c') {
      take_photo = true;
      HologramCloud.setRGB("BLUE");
    } else if (temp == 's') {
      HologramCloud.resetHttpProfile(0);
      HologramCloud.setHttpParam(0, 1, "\"api.cloudinary.com\"");
      HologramCloud.setHttpParam(0, 6, 1);
      HologramCloud.sendHttpRequest(0, 4,
                                    "\"/v1_1/birdfeedercam/image/upload\"",
                                    "\"response.ffs\"",
                                    "\"img.ffs\"",
                                    6,
                                    "\"multipart/form-data;boundary=ZZYZZX\"");
    } else if (temp == 't') {
      const char * fileContents = "--ZZYZZX\r\nContent-Disposition: form-data; name=\"upload_preset\"\r\n\r\ndefault\r\n--ZZYZZX\r\nContent-Disposition: form-data; name=\"file\"\r\nContent-Type: image/jpeg\r\n\r\nabcdef123456\r\n--ZZYZZX--\r\n";
      if (HologramCloud.prepareSendFile(strlen(fileContents), "\"test.ffs\"")) {
        delay(500);
        HologramCloud.writeToStream(fileContents);
        delay(500);
        HologramCloud.resetHttpProfile(0);
        HologramCloud.setHttpParam(0, 1, "\"httpbin.org\"");
        HologramCloud.sendHttpRequest(0, 4,
                                      "\"/post\"",
                                      "\"resp.ffs\"",
                                      "\"test.ffs\"",
                                      6,
                                      "\"multipart/form-data;boundary=ZZYZZX\"");
      }
    } else if (temp == 'y') {
      const char * fileContents = "--ZZYZZX\r\nContent-Disposition: form-data; name=\"upload_preset\"\r\n\r\ndefault\r\n--ZZYZZX\r\nContent-Disposition: form-data; name=\"file\"\r\nContent-Type: image/jpeg\r\n\r\nabcdef123456\r\n--ZZYZZX--\r\n";
      if (HologramCloud.prepareSendFile(strlen(fileContents), "\"test.ffs\"")) {
        delay(500);
        HologramCloud.writeToStream(fileContents);
        delay(500);
        HologramCloud.resetHttpProfile(0);
        HologramCloud.setHttpParam(0, 1, "\"httpbin.org\"");
        HologramCloud.setHttpParam(0, 6, 1);
        HologramCloud.sendHttpRequest(0, 4,
                                      "\"/post\"",
                                      "\"resp.ffs\"",
                                      "\"test.ffs\"",
                                      6,
                                      "\"multipart/form-data;boundary=ZZYZZX\"");
      }
    } else if (temp == 'r') {
      HologramCloud.listFiles();
      HologramCloud.checkFileSize("\"test.ffs\"");
      HologramCloud.readFileContents("\"test.ffs\"");
      HologramCloud.checkFileSize("\"resp.ffs\"");
      HologramCloud.readFileContents("\"resp.ffs\"");
    }
    else if (temp == 'd') {
      HologramCloud.listFiles();
      HologramCloud.checkFileSize("\"img.ffs\"");
      HologramCloud.readFileContents("\"img.ffs\"");
      HologramCloud.checkFileSize("\"response.ffs\"");
      HologramCloud.readFileContents("\"response.ffs\"");
    }
  }

  bool is_header = false;

  if (take_photo)
  {
    //Flush the FIFO
    myCAM->flush_fifo();
    myCAM->clear_fifo_flag();
    //Start capture
    myCAM->start_capture();
    take_photo = false;
  }
  else if (myCAM->get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK))
  {
    Serial.println("ACK CMD CAM Capture Done. END");
    bytesSent = 0;
    Dash.pulseLED(100, 100);

    uint8_t temp = 0, temp_last = 0;
    uint32_t imglength = 0;
    imglength = myCAM->read_fifo_length();
    if (imglength >= MAX_FIFO_SIZE) //512 kb
    {
      Serial.println("ACK CMD Over size. END");
      return;
    }
    if (imglength == 0 ) //0 kb
    {
      Serial.println("ACK CMD Size is 0. END");
      return;
    }

    const char * bodystart = "--ZZYZZX\r\nContent-Disposition: form-data; name=\"upload_preset\"\r\n\r\ndefault\r\n--ZZYZZX\r\nContent-Disposition: form-data; name=\"file\"; filename=\"%d.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";

    const int headsize = strlen(bodystart) + 10;
    char dest[headsize];
    sprintf(dest, bodystart, Clock.counter());

    long postFileSize = imglength + headsize + strlen("\r\n--ZZYZZX--\r\n");

    if (!HologramCloud.prepareSendFile(postFileSize, "\"img.ffs\"")) {
      HologramCloud.setRGB("RED");
      return;
    }

    delay(500);
    writeToBuffer(dest);

    SPI.beginTransaction(SPI_CS);
    myCAM->set_fifo_burst();//Set fifo burst mode
    temp =  SPI.transfer(0x00);
    imglength --;
    while ( imglength-- )
    {
      temp_last = temp;
      temp =  SPI.transfer(0x00);
      if (is_header == true)
      {
        writeToBuffer(temp);
      }
      else if ((temp == 0xD8) & (temp_last == 0xFF))
      {
        is_header = true;
        writeToBuffer(temp_last);
        writeToBuffer(temp);
      }
      if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
        break;
      delayMicroseconds(15);
    }
    writeToBuffer("\r\n--ZZYZZX--");
    HologramCloud.endStream();
    SPI.endTransaction();
    is_header = false;
    myCAM->clear_fifo_flag();
    Serial.println();
    Serial.print("Captured image sent ");
    Serial.print(bytesSent);
    Serial.println(" Bytes");
    while (bytesSent <= postFileSize) {
      HologramCloud.writeToStream(' ');
      bytesSent++;
    }
    Serial.println();
    Serial.print("HTTP post size is ");
    Serial.print(postFileSize);
    Serial.println(" Bytes");
    delay(500);
    HologramCloud.setRGB("GREEN");
    Dash.offLED();
  }
}

void writeToBuffer(char data) {
  HologramCloud.writeToStream(data);
  bytesSent++;
}

void writeToBuffer(const char * data) {
  HologramCloud.writeToStream(data);
  bytesSent += strlen(data);
}
