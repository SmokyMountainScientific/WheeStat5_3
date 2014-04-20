//  DiffPulse tab / StellarisWheeStat sketch
//  In anodic stripping voltammetry, the working electrode is initially held
//    at a voltage negative of that where the analyte is reduced for a 
//    given plating time.  The reduced analyte is precipitated onto the
//    electrode and the amount on the electrode is analyzed by measuring 
//    current passed as the voltage is scanned positive through the potential
//    at which the material is re-oxidized.

void diffPulse()  {

  ////////// Plating /////////////////////////////////////////////////  
  digitalWrite(pulse_pin,LOW);              // set pulse pin to low
  PWMWrite(signal_pin,pwmRes,dInit,pwmClock);  // set signal voltage to 512 + 1/2 Vinit

  /*     digitalWrite(stir_pin,HIGH);
   delay(initial_ms);            // stay at Vinit for initial_delay in sec 
   digitalWrite(stir_pin,LOW); */
  delay(delay1*1000);

  ///////// Stripping ////////////////////////////////////////////////

  for (int dC = 0; dC <= nSteps; ++dC){  
    if (dInit < dFnl){
      dSig = dInit + (dC*pwm_step);      // calculate digital Signal value
    }
    else {
      dSig = dInit - (dC*pwm_step);
    }
    PWMWrite(signal_pin,pwmRes,dSig,pwmClock);
    digitalWrite(pulse_pin,HIGH);       
    delay(stepTime/2);  

    readVolts();
    readCurrent(true);
    digitalWrite(pulse_pin,LOW);       
    delay(stepTime/2);  
    readCurrent(true);
    //readStop();           // if GUI transmits '&', stop read
    Serial.println("");

  }
  openCircuit();    // go to open circuit
 }


