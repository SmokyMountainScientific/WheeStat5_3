//  analogRead tab, StellarisWheeStat5_3


//////////////// Read voltage pin ///////////
void readVolts() {
  int inVolt = 0;                 // voltage read
  for (int i =0; i<=3; ++i){  
    inVolt += analogRead(Vread_pin);
  }
  mVread = inVolt/4;
  mVread = mVread*vcc/4096;
  mVread = (vcc/2)-mVread;
  Serial.print(mVread);  
  Serial.print('\t');

}
//////////////////// Read current pin //////////////
void readCurrent(boolean printI) {
  unsigned int iRead = 0;                 // current read
  for (int i =0; i<=15; ++i){  
    iRead += analogRead(Iread_pin);
  }
  mVi = (iRead)*vcc/4098/16;          // digital reading converted to mV
  if (printI == true) {
    Serial.print(mVi);  
    Serial.print('\t');
  }
}
