void getREGISTERS()
{
   byte contador;
   for(contador =0; contador<TOTAL_NO_OF_REGISTERS; contador++)
   {
     Serial.print("[");
     Serial.print(contador);
     Serial.print(":");
     Serial.print(regs[contador],16);
     Serial.print("],");
   }
   Serial.println("REGISTERS");
}

void getStatus()
{
  unsigned char contador;
  //Serial.println("******");
  for (contador=0; contador<TOTAL_NO_OF_PACKETS;contador++){
    Serial.print("[");
    Serial.print(contador+1);
    Serial.print(";");
    Serial.print(packets[contador].requests);
    Serial.print(";");
    Serial.print(packets[contador].successful_requests);
    Serial.print(";");
    Serial.print(packets[contador].failed_requests);
    Serial.print(";");
    Serial.print(packets[contador].exception_errors);
    Serial.print("],");
  }
  Serial.println("STATUS");
}

void setDebug()
{
  char *arg;  
  arg = _myMonitor.next();    // Get the next argument from the SerialCommand object buffer
  if (arg != NULL)      // As long as it existed, take it
  {
    if (arg[0] == '1')  set_debug(true);
    else set_debug(false);
  }
  else {
    Serial.print("[DEBUG_ASCII_MODBUS_FRAME:");
    Serial.print(get_debug());
    Serial.println("],MAIN");
  }
}
void changeFlags()
{
  char *arg;  
  arg = _myMonitor.next();    // Get the next argument from the SerialCommand object buffer
  if (arg != NULL)      // As long as it existed, take it
  {
    if (arg[0] == '1')  _autoChangeFlags = true;
    else _autoChangeFlags = false;
  }
  else {
    Serial.print("[SEND_REGISTERS_CHANGES:");
    Serial.print(_autoChangeFlags);
    Serial.println("],MAIN");
  }
}

void setTimeOut()
{
  char *arg;
  arg = _myMonitor.next();    // Get the next argument from the SerialCommand object buffer
  if (arg != NULL)      // As long as it existed, take it
  {
    long value = atoi(arg);
    set_timeout (value);
  }
  else {
    Serial.print("[TIME_OUT_FOR_SLAVE_RESPONSE:");
    Serial.print(get_timeout());
    Serial.println("],MAIN");
  }
}

void setPolling()
{
  char *arg;
  arg = _myMonitor.next();    // Get the next argument from the SerialCommand object buffer
  if (arg != NULL)      // As long as it existed, take it
  {
    long value= atoi(arg);
    set_polling (value);
  }
  else {
    Serial.print("[TIME_POLLING_FOR_SLAVES:");
    Serial.print(get_polling());
    Serial.println("],MAIN");
  }
}
void setRetries()
{
  char *arg;
  arg = _myMonitor.next();    // Get the next argument from the SerialCommand object buffer
  if (arg != NULL)      // As long as it existed, take it
  {
    unsigned char value = atoi(arg);
    set_retry_count(value);
  }
  else {
    Serial.print("[NUMBER_OF_RETRIES_FOR_SLAVE_OUT_POLLING:");
    Serial.print(get_retry_count());
    Serial.println("],MAIN");
  }
}

void miMonitor()
{
  Serial.println("[eMaster:AsciiModbus/openHAB Serial gateway],INFO"); 
}



