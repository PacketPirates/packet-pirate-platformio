#include <Arduino.h>

#include "defs.h"

void irScan()
{
  // Reset pattern
  if (g_irPattern)
    delete[] g_irPattern;

  g_irPattern = new bool[g_irLength];

  for (int i = 0; i < g_irLength; i++)
  {
    bool irInput = (bool) digitalRead(MODE_BTN);
    Serial.print("Scanning ir["); Serial.print(i); Serial.print("]: "); Serial.println(irInput);
    g_irPattern[i] = irInput;
    delay(1);
  }

  trimIrScan();
}

void irBroadcast()
{
  if (!g_irPattern || g_irLength == -1)
  {
    Serial.println("ERROR: Unable to broadcast IR without pattern.") ;
    return;
  }

  for (int i = 0; i < g_irLength; i++)
  {
    Serial.print("Broadcasting: "); Serial.println(g_irPattern[i]);
    digitalWrite(IR_LED, (int) g_irPattern[i]);
    delay(1);
  }  

  // End broadcast
  digitalWrite(IR_LED, 0);
}

void trimIrScan()
{
  Serial.println("Trimming IR pattern...");
  int init = -1;
  for (int i = 0; i < g_irLength; i++)
  {
    if (g_irPattern[i])
    {
      init = i;
      break;
    }
  }

  if (init == -1)
  {
    Serial.println("Pattern empty! Resetting array.");
    delete[] g_irPattern;
    g_irLength = -1;
  }

  int end = -1;
  for (int i = g_irLength - 1; i >= 0; i--)
  {
    if (g_irPattern[i])
    {
      end = i;
      break;
    }
  }

  int newLen = end - init + 1;
  bool* tempArr = new bool[newLen];
  for (int i = 0; i < newLen; i++)
  {
    tempArr[i] = g_irPattern[i + init];
  }

  delete[] g_irPattern;
  g_irPattern = tempArr;
  Serial.print("Pattern sliced from size ");Serial.print(g_irLength);Serial.print("to size ");Serial.println(newLen);
  g_irLength = newLen;
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