#include <Arduino.h>

#include "defs.h"

void irScan()
{
  if (g_irStart == -1)
  {
    g_irStart = g_tick;
    
    // Reset pattern
    if (g_irPattern)
      delete[] g_irPattern;

    g_irPattern = new bool[g_irLength];
  }

  if (g_tick - g_irStart >= g_irLength)
  {
    Serial.println("ERROR: SOMETHING HAS GONE HORRIBLY WRONG AND WE ARE TRYING TO SCAN OUT OF IR SCAN RANGE");
    return;
  }

  bool irInput = (bool) digitalRead(MODE_BTN);

  Serial.print("Scanning ir["); Serial.print(g_tick - g_irStart); Serial.print("]: "); Serial.println(irInput);

  g_irPattern[g_tick - g_irStart] = irInput;

  // End scan but leave pattern in place
  if (g_tick == g_irStart + g_irLength - 1)
    g_irStart = -1;
}

void irBroadcast()
{
  if (g_irStart == -1)
  {
    if (!g_irPattern || g_irLength == -1)
    {
      Serial.println("ERROR: Unable to broadcast IR without pattern.") ;
      return;
    }

    g_irStart = g_tick;
  }

  Serial.print("Broadcasting: "); Serial.println(g_irPattern[g_tick - g_irStart]);
  digitalWrite(IR_LED, (int) g_irPattern[g_tick - g_irStart]);

  // End broadcast
  if (g_tick == g_irStart + g_irLength - 1)
  {
    g_irStart = -1;
    digitalWrite(IR_LED, 0);
  }
}

void setIrPattern(int length, bool* pattern)
{
  if (g_irPattern)
    delete[] g_irPattern;

  g_irLength = length;

  // It is the responsibility of the caller to ensure the memory we are using here is already allocated to the correct length.
  // This will likely be changed to something different once/if a db connection is set up for this
  g_irPattern = pattern;
}