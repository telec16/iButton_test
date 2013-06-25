#include <LiquidCrystal.h>
#include <OneWire.h>

LiquidCrystal lcd(12, 11, 7, 8, 9, 10);

OneWire ds(2);
byte addr[8];
byte addr_pass[8] = {0x01, 0xF0, 0x57, 0xA2, 0x16, 0x00, 0x00, 0xBE};
byte data[8];
byte present = 0;
byte compte = 0;
int i;
int rtc;

void setup() 
{
  Serial.begin(9600);
  lcd.begin(20, 4);
  
  pinMode(3, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
  while(!auth())
  {
    lcd.clear();
    lcd.print("Mauvais identifiant");
    delay(1000);
  }
  lcd.clear();
  lcd.print("Bon identifiant");
  lcd.setCursor(0,1);
  lcd.print("Inserez le DS1904");
  
  delay(2000);
  while(!init_DS1904());
  
  lcd.clear();
  lcd.print("Pour lancer le");
  lcd.setCursor(0,1);
  lcd.print("decompte, appuyez");
  lcd.setCursor(0,2);
  lcd.print("sur le bouton et");
  lcd.setCursor(0,3);
  lcd.print("enlevez le DS1904...");
}

void loop() 
{
  // write!
  while(digitalRead(3) == HIGH && compte == 0);
  if(digitalRead(3) == LOW)
  {
    while (!ds.search(addr));
    lcd.clear();
    lcd.print("Mise a zero...");
    
    present = ds.reset();
    ds.select(addr);
    ds.write(0x99,1);   // write RTC
    //USER  OSC
    //????  11 00
    ds.write(0b00001100);
    ds.write(0x00);
    ds.write(0x00);
    ds.write(0x00);
    ds.write(0x00);
    present = ds.reset();
    compte = 1;
    digitalWrite(13, HIGH);
    
    lcd.setCursor(0,1);
    lcd.print("Attente du signal...");
    
    while(digitalRead(3) == LOW);
    delay(2000);     // unknown if wait needed
  }
  else
  {
    if(compte == 1)
    {
      if(ds.search(addr))
      {
        // read!
        present = ds.reset();
        ds.select(addr);
        ds.write(0x66,1);   // read RTC
        for ( i = 0; i < 5; i++) {
          data[i] = ds.read();
        }
        
        rtc = ((uint32_t)data[1]); //LSB
        rtc |= ((uint32_t)data[2]) << 8;
        rtc |= ((uint32_t)data[3]) << 16;
        rtc |= ((uint32_t)data[4]) << 24;  //MSB
        
        lcd.clear();
        lcd.print("Temps passe : ");
        lcd.setCursor(0,1);
        lcd.print(rtc, DEC);
        lcd.print(" secondes");
        compte = 0;
        digitalWrite(13, 0);
      }
      else
      {
        ds.reset_search();
      }
    }
  }
  
  delay(1000);//no less
  
  //Serial.println();
}

boolean init_DS1990()
{
  while ( !ds.search(addr)) {
      lcd.setCursor(0,2);
      lcd.print("Attente du signal...");
      ds.reset_search();
      delay(1000);  // for readability
  }
  
  if ( OneWire::crc8( addr, 7) != addr[7]) {
      lcd.clear();
      lcd.print("CRC is not valid!\n");
      delay(1000);  // for readability
      return false;
  }
  
  if(addr[0] != 0x01)
  {
    lcd.setCursor(0,2);
    lcd.print("Mauvais signal      ");
    delay(1000);
    return false;
  }
  return true;
}

boolean init_DS1904()
{
  while ( !ds.search(addr)) {
      lcd.clear();
      lcd.print("Attente du signal...");
      ds.reset_search();
      delay(1000);  // for readability
  }
  
  if ( OneWire::crc8( addr, 7) != addr[7]) {
      lcd.clear();
      lcd.print("CRC is not valid!\n");
      delay(1000);  // for readability
      return false;
  }
  
  if(addr[0] != 0x24)
  {
    lcd.clear();
    lcd.print("Mauvais signal");
    delay(1000);
    return false;
  }
  return true;
}

boolean auth()
{
  lcd.clear();
  lcd.print("Veuillez vous");
  lcd.setCursor(0,1);
  lcd.print("identifiez...");
  while(!init_DS1990());
  
  boolean first = true;
  boolean ok = false;
  lcd.setCursor(0,3);
  for( i = 7; i >= 0; i--) 
  {
    lcd.print(addr[i], HEX);
    if (i == 7) lcd.print("-");
    if (i == 1) lcd.print("-");
    if(addr[i] == addr_pass[i] && (first == true || ok == true))
    {
      first = false;
      ok = true;
    }
    else
    {
      ok = false;
    }
    delay(500);
  }
  delay(1000);
  return ok;
}
