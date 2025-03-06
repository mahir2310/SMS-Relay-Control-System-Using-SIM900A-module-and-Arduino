//all status is send at once by the status checking // i send it from my gp sim
//12/12/2024 -->6.23AM COLD weather
#include <SoftwareSerial.h>
#include <IRremote.h>

// Sender phone number with country code
const String PHONE = "8801731849660";

int msgCount;

// Global variables for SMS handling
String buff;
String msg;

// Pin definitions
#define rxPin 2
#define txPin 3
SoftwareSerial sim900A(rxPin, txPin);

const byte IR_RECEIVE_PIN = 4;

#define button1_pin 5
#define button2_pin 6
#define button3_pin 7
#define button4_pin 8

#define relay1_pin 9
#define relay2_pin 10
#define relay3_pin 11
#define relay4_pin 12

// Active LOW relay logic
boolean relay1_state = 1;
boolean relay2_state = 1;
boolean relay3_state = 1;
boolean relay4_state = 1;

void setup() {
    // Initialize pins
    pinMode(button1_pin, INPUT_PULLUP);
    pinMode(button2_pin, INPUT_PULLUP);
    pinMode(button3_pin, INPUT_PULLUP);
    pinMode(button4_pin, INPUT_PULLUP);

    pinMode(relay1_pin, OUTPUT);
    pinMode(relay2_pin, OUTPUT);
    pinMode(relay3_pin, OUTPUT);
    pinMode(relay4_pin, OUTPUT);

    // Set initial relay states
    digitalWrite(relay1_pin, relay1_state);
    digitalWrite(relay2_pin, relay2_state);
    digitalWrite(relay3_pin, relay3_state);
    digitalWrite(relay4_pin, relay4_state);

    // Start serial communication
    Serial.begin(115200);
    sim900A.begin(9600);

    // Initialize IR receiver
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    // Initialize GSM module
    initGSM();
    msgCount=1;
}

void initGSM() {
    delay(2000); // Wait for GSM module to stabilize
    
    // Basic AT command to check communication
    sim900A.println("AT");
    delay(1000);
    
    // Set SMS text mode
    sim900A.println("AT+CMGF=1");
    delay(1000);
    
    // Set new message indication
    sim900A.println("AT+CNMI=1,1,0,0,0");
    delay(1000);
    
    // Delete all stored SMS
    sim900A.println("AT+CMGD=1,4");
    delay(1000);
}

void loop() {
    // Check for new SMS
    while (sim900A.available()) {
        String response = sim900A.readStringUntil('\n');
        Serial.println("before trimming response:");
        Serial.println(response);
        response.trim();
        Serial.println("after trimming response: ");
        Serial.println(response);
        Serial.println("GSM Response: " + response); // Debug print
        
        // Check for new message notification
        if (response.indexOf("+CMTI:") != -1) {
            int commaIndex = response.indexOf(",");
            if (commaIndex != -1) {
                String smsIndex = response.substring(commaIndex + 1);
                smsIndex.trim();
                readSMS(smsIndex);
            }
        }
        // Check for message content
        else if (response.indexOf("+CMGR:") != -1) {
            // Wait for the message content in the next line
            delay(100);
            if (sim900A.available()) {
                msg = sim900A.readStringUntil('\n');
                msg.trim();
                Serial.println("Message Content: " + msg); // Debug print
                
                if (msg.length() > 0) {
                    processCommand(msg);
                }
            }
        }
    }

    listen_ir();
    listen_push_buttons();
}

void readSMS(String index) {
    Serial.println("Reading SMS at index: " + index); // Debug print
    sim900A.println("AT+CMGR=" + index);
}

void processCommand(String command) {
    Serial.println("command before making to lowercase");
    Serial.println(command);

    command.toLowerCase();
    Serial.println("command after making lowercase");
    Serial.println(command);

    command.trim();
    Serial.println("Command after trimming");
    Serial.println(command);
    
    Serial.println("Processing command: " + command); // Debug print
    
    if (command == "1") {
        control_relay(1);
    }
    else if (command == "2") {
        control_relay(2);
    } 
    else if (command == "3") {
        control_relay(3);
    } 
    else if (command == "4") {
        control_relay(4);
    } 
    else if (command == "status") {
        // Send status for all relays regardless of which relay's status was requested
        send_all_relay_status();
    }
    else if (command == "del all") {
        delete_all_sms();
    }
}

void listen_ir() {
    if (IrReceiver.decode()) {
        String ir_code = String(IrReceiver.decodedIRData.command, HEX);
        
        if (ir_code == "c") control_relay(1);
        else if (ir_code == "18") control_relay(2);
        else if (ir_code == "5e") control_relay(3);
        else if (ir_code == "8") control_relay(4);

        IrReceiver.resume();
    }
}

void listen_push_buttons() {
    if (digitalRead(button1_pin) == LOW) {
        delay(200);
        control_relay(1);
    }
    else if (digitalRead(button2_pin) == LOW) {
        delay(200);
        control_relay(2);
    }
    else if (digitalRead(button3_pin) == LOW) {
        delay(200);
        control_relay(3);
    }
    else if (digitalRead(button4_pin) == LOW) {
        delay(200);
        control_relay(4);
    }
}

void control_relay(int relay) {
    switch (relay) {
        case 1:
            relay1_state = !relay1_state;
            digitalWrite(relay1_pin, relay1_state);
            break;
        case 2:
            relay2_state = !relay2_state;
            digitalWrite(relay2_pin, relay2_state);
            break;
        case 3:
            relay3_state = !relay3_state;
            digitalWrite(relay3_pin, relay3_state);
            break;
        case 4:
            relay4_state = !relay4_state;
            digitalWrite(relay4_pin, relay4_state);
            break;
    }
    delay(50);
}

// New function to send status of all relays at once
void send_all_relay_status() {
    String sms_text = "Relay Status:\n";
    
    // Add status for each relay
    sms_text += "Relay 1: " + String(relay1_state ? "OFF" : "ON") + "\n";
    sms_text += "Relay 2: " + String(relay2_state ? "OFF" : "ON") + "\n";
    sms_text += "Relay 3: " + String(relay3_state ? "OFF" : "ON") + "\n";
    sms_text += "Relay 4: " + String(relay4_state ? "OFF" : "ON");
    
    Reply(sms_text);
}

void delete_all_sms() {
    sim900A.println("AT+CMGD=1,4");
    Serial.println("delete all sms called,all sms are deleted!!");
    delay(1000);
    //Reply("All SMS deleted");
    
}

void Reply(String text) {
    // Reset SMS mode before sending
    Serial.println("Message count below");
    Serial.println(msgCount++);
    sim900A.println("AT+CMGF=1");
    delay(1000);
    
    // Send SMS command with phone number
    sim900A.println("AT+CMGS=\"" + PHONE + "\"");
    delay(1000);
    
    // Send message content
    sim900A.print(text);
    delay(100);
    
    // Send Ctrl+Z to indicate end of message
    sim900A.write(26);
    delay(1000);
    Serial.println("reply text is below:");
    Serial.println(text);
}