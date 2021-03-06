// acessing doGet function in a google sheet
// https://script.google.com/macros/s/google_sheet_key/exec?node=node_name&action=action_keyword




#define SHEET_SERVER  "script.google.com"
// need about 30K of ram !!!!!! WiFiClientSecure
bool dialWithGoogle(const String aNode, const String aAction, JSONVar &jsonParam) {
  // (global) HTTPClient http;  //Declare an object of class HTTPClient
  Serial.print(F("Dial With gSheet as '"));
  Serial.print(aNode);
  Serial.print(':');
  Serial.print(aAction);
  Serial.println('\'');
  D_println(helperFreeRam() + 000);
  if (helperFreeRam() < 41000) {
    Serial.println(F("https need more memory"));
    return (false);
  }
  String bigString = F("https://" SHEET_SERVER "/macros/s/");
  {
    String GKey = jobGetConfigStr(F("gkey"));
    if (aNode.length() == 0 || GKey.length() == 0 || aAction.length() == 0) return (false);
    //bigString = F("https://" SHEET_SERVER "/macros/s/");
    bigString += GKey;

    bigString += F("/exec?node=");
    bigString += encodeUri(aNode);;

    bigString += F("&action=");
    bigString += encodeUri(aAction);;

    //D_println(JSON.typeof(jsonParam));
    // les parametres eventuels sont passées en JSON dans le parametre '&json='
    if (JSON.typeof(jsonParam) == F("object") ) {
      bigString += F("&json=");
      //D_println(JSON.stringify(jsonParam));
      bigString += encodeUri(JSON.stringify(jsonParam));
    }
    D_println(helperFreeRam() + 0001);
    jsonParam = undefined;
    D_println(helperFreeRam() + 0002);
    WiFiClientSecure wifiSecure;
    //  HTTPClient http;  //Declare an object of class HTTPClient
    // !!! TODO get a set of valid root certificate for google !!!!
    wifiSecure.setInsecure(); //the magic line, use with caution  !!! certificate not checked
    //   D_println(helperFreeRam() + 00);
    //http.setTimeout(15000); // 15 Seconds
    D_println(bigString);
    //http.end(); // 10 Seconds
    //http.setTimeout(10000); // 10 Seconds
    http.begin(wifiSecure, bigString); //Specify request destination
    //bigString = "";  // clear memory


    // define requested header
    const char * headerKeys[] = {"location"} ;
    const size_t numberOfHeaders = 1;
    http.collectHeaders(headerKeys, numberOfHeaders);
    //D_println(helperFreeRam() + 01);
    int httpCode = http.GET();//Send the request  (gram 22K of ram)



    /*** HTTP client errors
        #define HTTPCLIENT_DEFAULT_TCP_TIMEOUT (5000)
      #define HTTPC_ERROR_CONNECTION_FAILED   (-1)
      #define HTTPC_ERROR_SEND_HEADER_FAILED  (-2)
      #define HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)
      #define HTTPC_ERROR_NOT_CONNECTED       (-4)
      #define HTTPC_ERROR_CONNECTION_LOST     (-5)
      #define HTTPC_ERROR_NO_STREAM           (-6)
      #define HTTPC_ERROR_NO_HTTP_SERVER      (-7)
      #define HTTPC_ERROR_TOO_LESS_RAM        (-8)
      #define HTTPC_ERROR_ENCODING            (-9)
      #define HTTPC_ERROR_STREAM_WRITE        (-10)
      #define HTTPC_ERROR_READ_TIMEOUT        (-11)
    */

    D_println(httpCode);
    D_println(helperFreeRam() + 02);
    int antiLoop = 0;
    while (httpCode == 302 && antiLoop++ < 3) {
      //bigString = http.header(headerKeys[0]);
      // google will give answer in relocation
      //D_println(bigString);
      D_println(helperFreeRam() + 031);
      http.end();   //Close connection (got err -7 if not)
      D_println(helperFreeRam() + 041);
      bigString = http.header(headerKeys[0]);
      D_println(http.header(headerKeys[0]));
      http.begin(wifiSecure, bigString); //Specify request new destination
      http.collectHeaders(headerKeys, numberOfHeaders);
      httpCode = http.GET();//Send the request
      //D_println(httpCode);
    }
    bigString = "";
    //    if (httpCode < 0) {
    //      Serial.print(F("cant get an answer :( http.GET()="));
    //      Serial.println(httpCode);
    //      http.end();   //Close connection
    //      return (false);
    //    }

    if (httpCode != 200) {
      Serial.print(F("got an error in http.GET() "));
      D_println(httpCode);
      http.end();   //Close connection
      return (false);
    }

    bigString = http.getString();   //Get the request response payload
    //D_println(helperFreeRam() + 1);
    http.end();   //Close connection (restore 22K of ram)
  } //clear string and http memory
  D_println(helperFreeRam() + 05);
  D_println(bigString);             //Print the response payload
  JSONVar jsonPayload = JSON.parse(bigString);
  //D_println(helperFreeRam() + 06);
  bigString = "";
  if (JSON.typeof(jsonPayload) != F("object")) {
    D_println(JSON.typeof(jsonPayload));
    return (false);
  }

  // super check json data for "status" is a bool true  to avoid foolish data then supose all json data are ok.
  if (!jsonPayload.hasOwnProperty("status") || JSON.typeof(jsonPayload["status"]) != F("boolean") || !jsonPayload["status"]) {
    D_println(JSON.typeof(jsonPayload["status"]));
    return (false);
  }


  JSONVar answer = jsonPayload["answer"];  // cant grab object from the another not new object
  jsonParam = answer;                    // so memory use is temporary duplicated here
  D_println(helperFreeRam() + 001);
  return (true);



}

#define Hex2Char(X) (char)((X) + ((X) <= 9 ? '0' : ('A' - 10)))

// encode optimisé pour le json
String encodeUri(const String aUri) {
  String answer = "";
  String specialChar = F(".-~_{}[],;:\"\\");
  // uri encode maison :)
  for (int N = 0; N < aUri.length(); N++) {
    char aChar = aUri[N];
    //TODO:  should I keep " " to "+" conversion ????  save 2 char but oldy
    if (aChar == ' ') {
      answer += '+';
    } else if ( isAlphaNumeric(aChar) ) {
      answer +=  aChar;
    } else if (specialChar.indexOf(aChar) >= 0) {
      answer +=  aChar;
    } else {
      answer +=  '%';
      answer += Hex2Char( aChar >> 4 );
      answer += Hex2Char( aChar & 0xF);
    } // if alpha
  }
  return (answer);
}
