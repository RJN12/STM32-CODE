void setup() {
  // Initialize Serial communication at 9600 baud rate
  Serial.begin(9600);
  
  // Introductory message
  Serial.println("Hello from Hardware_Coding!");
  delay(1000);  // Wait 1 second before starting the main message
}

void loop() {
  // Part 1 of the message
  Serial.println("Wish you a"); 
  delay(1000);  // Wait 1 second

  // Part 2 of the message
  Serial.println("Happy Diwali");
  delay(1000);  // Wait 1 second

  // Part 3 of the message
  Serial.println("From Hardware_Coding Team!");
  delay(1000);  // Wait 1 second

  // Concluding message
  Serial.println("Subscribe for more updates and projects!");
  
  // Stop further sending by entering an infinite loop
  while (true) {
    // Optionally, keep the microcontroller idle
  }
}
